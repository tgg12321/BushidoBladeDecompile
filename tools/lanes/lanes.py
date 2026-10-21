#!/usr/bin/env python3
"""lanes.py: parallel work lanes for BB2, one agent session per git worktree.

A lane is a named worktree (`../bb2-worktrees/ln-<lane>`, branch `lanes/<lane>`) owned by exactly
one Claude session. Lanes never edit the main checkout. Main is only a landing pad: `land`
replays the lane's commits onto the current main in a private landing worktree, runs the gates
there, and fast-forwards main to the gated commit under the main-reintegration lock
(tools/reintegrate_lock.ps1, same lock file), so main never holds an ungated state.

Conflicts are handled without hand merges where the file allows it:
  - regenerated files (lanes.json "regenerate", e.g. the naming census) take main's side and are
    rebuilt after the gates;
  - row-keyed TSV registries (lanes.json "rowmerge", the backlogs) are merged row by row;
  - a commit whose message carries `Lane-Replay: <shell command>` trailers is re-run on the new
    base instead of cherry-picked (use it only when the commit is exactly that tool output);
  - anything else bounces back to the lane, untouched, with the conflicting files listed.

Claims keep lanes off each other's work: `path:<glob>` (edit and land exclusivity; a shared path
can only be claimed with a trailing `!`, which freezes it for every other lane until released)
and `topic:<text>` (work selection, e.g. `topic:row:m09`, `topic:tu:51268`). The registry lives
in main's `tmp/lanes-state/registry.json` (untracked, so it can never conflict).
tools/hooks/lane_guard.py enforces ownership and claims on Edit/Write/Bash/PowerShell.

Usage (Windows Python, from any cwd):
  python tools/lanes/lanes.py status [--json]
  python tools/lanes/lanes.py open  <lane> [--kind naming|foundation] [--steal]
  python tools/lanes/lanes.py fresh <lane> [--discard]     # --discard backs the tip up first
  python tools/lanes/lanes.py drop  <lane> <sha>            # remove one unlanded commit (a bounce)
  python tools/lanes/lanes.py claim <lane> <token>...       # path:src/main/9F9C.c  topic:row:m09
  python tools/lanes/lanes.py unclaim <lane> [<token>...]   # no tokens = all
  python tools/lanes/lanes.py land  <lane> [--no-push]
  python tools/lanes/lanes.py close <lane> [--remove]
  python tools/lanes/lanes.py path  <lane> [--wsl]
  python tools/lanes/lanes.py recover <lane>                # after an interrupted land
  python tools/lanes/lanes.py push                          # retry a failed push (exit 5)

The session identity is $CLAUDE_CODE_SESSION_ID (override with --session for tests).
Exit codes: 0 ok, 1 refused, 3 land bounced on a conflict, 4 a gate failed, 5 push failed.
"""
from __future__ import annotations

import argparse
import fnmatch
import hashlib
import json
import os
import re
import subprocess
import sys
import time
from pathlib import Path

LANE_RE = re.compile(r"^[a-z0-9][a-z0-9-]{0,30}$")
TRAILER_RE = re.compile(r"^Lane-Replay:\s*(.+?)\s*$", re.MULTILINE)
LOCK_REL = Path("tmp") / ".main_reintegration.lock"
LOCK_STALE_SECS = 5400  # keep in sync with tools/reintegrate_lock.ps1


class Refuse(Exception):
    def __init__(self, msg: str, code: int = 1):
        super().__init__(msg)
        self.code = code


# ---------------------------------------------------------------- basics

def run(cmd, cwd=None, check=True, env=None, input_text=None):
    r = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True, encoding="utf-8",
                       errors="replace", env=env, input=input_text)
    if check and r.returncode != 0:
        raise Refuse(f"command failed ({r.returncode}): {' '.join(map(str, cmd))}\n"
                     f"{(r.stdout or '')[-2000:]}{(r.stderr or '')[-2000:]}")
    return r


def git(*args, cwd, check=True) -> str:
    return run(["git", *args], cwd=cwd, check=check).stdout.strip()


def git_ok(*args, cwd) -> bool:
    return run(["git", *args], cwd=cwd, check=False).returncode == 0


def norm(p) -> str:
    return os.path.normcase(os.path.abspath(str(p)))


def wsl_path(p) -> str:
    s = str(p).replace("\\", "/")
    m = re.match(r"^([A-Za-z]):/(.*)$", s)
    return f"/mnt/{m.group(1).lower()}/{m.group(2)}" if m else s


def find_main(start: Path) -> Path:
    """The MAIN checkout = first entry of `git worktree list` (cwd-independent)."""
    if os.environ.get("BB2_LANES_MAIN"):
        return Path(os.environ["BB2_LANES_MAIN"]).resolve()
    out = git("worktree", "list", "--porcelain", cwd=start)
    for line in out.splitlines():
        if line.startswith("worktree "):
            return Path(line[len("worktree "):].strip()).resolve()
    raise Refuse("cannot resolve the main checkout")


class Ctx:
    def __init__(self, main: Path, session: str):
        self.main = main
        self.session = session
        cfg_path = Path(os.environ.get("BB2_LANES_CONFIG") or main / "tools" / "lanes" / "lanes.json")
        self.cfg = json.loads(cfg_path.read_text(encoding="utf-8"))
        self.wt_root = (main / self.cfg["worktree_root"]).resolve()
        self.state_dir = main / "tmp" / "lanes-state"
        self.state_dir.mkdir(parents=True, exist_ok=True)
        self.reg_path = self.state_dir / "registry.json"
        self.mutex = self.state_dir / ".registry.mutex"

    def lane_dir(self, lane: str) -> Path:
        return self.wt_root / f"{self.cfg['dir_prefix']}{lane}"

    def branch(self, lane: str) -> str:
        return f"{self.cfg['branch_prefix']}{lane}"

    def land_branch(self, lane: str) -> str:
        return f"{self.cfg['branch_prefix']}{lane}-land"

    def land_dir(self, lane: str) -> Path:
        return self.wt_root / "lands" / lane

    def stale_secs(self) -> int:
        return int(float(self.cfg.get("stale_hours", 12)) * 3600)

    # ---- registry (short critical sections under an O_EXCL mutex file)
    def _acquire_mutex(self):
        deadline = time.time() + 30
        while True:
            try:
                fd = os.open(self.mutex, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
                os.write(fd, str(os.getpid()).encode())
                os.close(fd)
                return
            except FileExistsError:
                try:
                    if time.time() - self.mutex.stat().st_mtime > 120:
                        self.mutex.unlink()  # a crashed holder; the section is seconds long
                        continue
                except FileNotFoundError:
                    continue
                if time.time() > deadline:
                    raise Refuse(f"registry mutex busy: {self.mutex}")
                time.sleep(0.2)

    def load(self) -> dict:
        try:
            reg = json.loads(self.reg_path.read_text(encoding="utf-8"))
        except FileNotFoundError:
            reg = {}
        reg.setdefault("lanes", {})
        return reg

    def update(self, fn):
        """Read-modify-write the registry atomically; fn(reg) may raise Refuse."""
        self._acquire_mutex()
        try:
            reg = self.load()
            out = fn(reg)
            tmp = self.reg_path.with_suffix(".tmp")
            tmp.write_text(json.dumps(reg, indent=2, sort_keys=True) + "\n", encoding="utf-8",
                           newline="\n")
            os.replace(tmp, self.reg_path)
            return out
        finally:
            try:
                self.mutex.unlink()
            except FileNotFoundError:
                pass


def is_stale(ctx: Ctx, entry: dict) -> bool:
    return time.time() - float(entry.get("heartbeat", 0)) > ctx.stale_secs()


def require_owner(ctx: Ctx, reg: dict, lane: str) -> dict:
    e = reg["lanes"].get(lane)
    if not e or not e.get("owner"):
        raise Refuse(f"lane '{lane}' is not open; run: lanes.py open {lane}")
    if e["owner"] != ctx.session:
        raise Refuse(f"lane '{lane}' is owned by session {e['owner']} (not you, {ctx.session}).")
    e["heartbeat"] = time.time()
    return e


# ---------------------------------------------------------------- claims

def split_token(tok: str):
    if ":" not in tok:
        raise Refuse(f"claim '{tok}' needs a kind prefix: path:<glob> or topic:<text>")
    kind, val = tok.split(":", 1)
    if kind not in ("path", "topic") or not val:
        raise Refuse(f"claim '{tok}': kind must be path or topic")
    freeze = kind == "path" and val.endswith("!")
    if freeze:
        val = val[:-1]
    return kind, val.replace("\\", "/"), freeze


def _literal_prefix(glob: str) -> str:
    m = re.search(r"[*?\[]", glob)
    return glob if not m else glob[:m.start()]


def globs_overlap(a: str, b: str) -> bool:
    if a == b or fnmatch.fnmatchcase(a, b) or fnmatch.fnmatchcase(b, a):
        return True
    pa, pb = _literal_prefix(a), _literal_prefix(b)
    wa, wb = pa != a, pb != b
    if wa and wb:
        return pa.startswith(pb) or pb.startswith(pa)
    if wa:
        return pb.startswith(pa) and fnmatch.fnmatchcase(b, a)
    if wb:
        return pa.startswith(pb) and fnmatch.fnmatchcase(a, b)
    return False


def path_claims(entry: dict):
    for tok in entry.get("claims", []):
        kind, val, freeze = split_token(tok)
        if kind == "path":
            yield val, freeze


def claimed_by_other(ctx: Ctx, reg: dict, lane: str, rel: str):
    """(other_lane, glob, frozen) if another live lane claims path `rel`, else None."""
    rel = rel.replace("\\", "/").lower()  # NTFS is case-insensitive
    for other, e in reg["lanes"].items():
        if other == lane or not e.get("owner") or is_stale(ctx, e):
            continue
        for glob, freeze in path_claims(e):
            if fnmatch.fnmatchcase(rel, glob.lower()):
                return other, glob, freeze
    return None


def is_shared(ctx: Ctx, glob: str) -> bool:
    return any(globs_overlap(glob, s) for s in ctx.cfg.get("shared_paths", []))


# ---------------------------------------------------------------- main lock

def read_lock(ctx: Ctx):
    try:
        return json.loads((ctx.main / LOCK_REL).read_text(encoding="utf-8-sig"))
    except Exception:
        return None


def _lock_json(ctx: Ctx, label: str) -> bytes:
    head = git("rev-parse", "--short", "HEAD", cwd=ctx.main)
    obj = {"session": ctx.session, "ts": int(time.time()), "head": head, "label": label}
    return json.dumps(obj, separators=(",", ":")).encode("utf-8")


def _create_lock(ctx: Ctx, label: str) -> bool:
    """Atomically create the lock file; False if it already exists."""
    data = _lock_json(ctx, label)
    p = ctx.main / LOCK_REL
    p.parent.mkdir(parents=True, exist_ok=True)
    try:
        fd = os.open(p, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
    except FileExistsError:
        return False
    with os.fdopen(fd, "wb") as f:
        f.write(data)
    return True


def write_lock(ctx: Ctx, label: str):
    """Heartbeat: rewrite the lock only while it is still ours."""
    lk = read_lock(ctx)
    if not lk or lk.get("session") != ctx.session:
        raise Refuse("lost the main lock mid-land (another session took it); main was not "
                     "changed by this land", 1)
    p = ctx.main / LOCK_REL
    tmp = p.with_name(p.name + f".{os.getpid()}.tmp")
    tmp.write_bytes(_lock_json(ctx, label))
    os.replace(tmp, p)


def acquire_lock(ctx: Ctx, label: str):
    deadline = time.time() + 60 * float(ctx.cfg.get("lock_wait_minutes", 45))
    announced = False
    while True:
        if _create_lock(ctx, label):
            return
        lk = read_lock(ctx)
        age = time.time() - float(lk.get("ts", 0)) if lk else 0
        if lk and lk.get("session") == ctx.session:
            write_lock(ctx, label)
            return
        if lk is None or age > LOCK_STALE_SECS:
            # unreadable (mid-write) or stale: re-check after a pause, then steal by unlink +
            # O_EXCL create, so two stealers cannot both win.
            time.sleep(1)
            lk2 = read_lock(ctx)
            if lk2 == lk or (lk2 and time.time() - float(lk2.get("ts", 0)) > LOCK_STALE_SECS):
                try:
                    (ctx.main / LOCK_REL).unlink()
                except FileNotFoundError:
                    pass
            continue
        if time.time() > deadline:
            raise Refuse(f"main lock held by {lk.get('session')} ({lk.get('label')}) "
                         f"for {int(age)}s; gave up waiting")
        if not announced:
            print(f"lanes: waiting for the main lock (held by {lk.get('label') or lk.get('session')})",
                  flush=True)
            announced = True
        time.sleep(15)


def release_lock(ctx: Ctx):
    lk = read_lock(ctx)
    if lk and lk.get("session") == ctx.session:
        try:
            (ctx.main / LOCK_REL).unlink()
        except FileNotFoundError:
            pass


# ---------------------------------------------------------------- runner

def run_shell(ctx: Ctx, wt: Path, cmd: str, log) -> int:
    """Run one shell line at the root of worktree `wt` (WSL + venv, or local for tests)."""
    print(f"lanes:   $ {cmd}", flush=True)
    if ctx.cfg.get("runner") == "local":
        r = subprocess.run(cmd, cwd=wt, shell=True, capture_output=True, text=True,
                           encoding="utf-8", errors="replace")
    else:
        script_dir = wt / "tmp" / "lanes"
        script_dir.mkdir(parents=True, exist_ok=True)
        script = script_dir / "run.sh"
        script.write_text("set -e\n"
                          f"cd '{wsl_path(wt)}'\n"
                          ". .venv/bin/activate\n"
                          f"{cmd}\n", encoding="utf-8", newline="\n")
        r = subprocess.run(["wsl", "bash", wsl_path(script)], capture_output=True, text=True,
                           encoding="utf-8", errors="replace")
    out = (r.stdout or "") + (r.stderr or "")
    log.write(f"\n$ {cmd}\n{out}\n[exit {r.returncode}]\n")
    log.flush()
    tail = "\n".join(out.strip().splitlines()[-6:])
    if tail:
        print("\n".join("lanes:     " + t for t in tail.splitlines()), flush=True)
    return r.returncode


# ---------------------------------------------------------------- row merge

def _rows(text: str):
    """(header, key order, key->line), or None if the text is not a clean row-keyed table
    (missing, blank lines, duplicate keys): those merge only by hand."""
    lines = text.splitlines()
    if not lines:
        return None
    header, body = lines[0], lines[1:]
    order, rows = [], {}
    for ln in body:
        key = ln.split("\t", 1)[0]
        if not ln.strip() or key in rows:
            return None
        order.append(key)
        rows[key] = ln
    return header, order, rows


def _same_relative_order(a, b) -> bool:
    common = set(a) & set(b)
    return [k for k in a if k in common] == [k for k in b if k in common]


def rowmerge(base: str, ours: str, theirs: str):
    """Three-way merge of a row-keyed TSV (first column = key). Returns text, or None when it must
    be resolved by hand: the same row changed differently on both sides, the header changed on
    both, the lane reordered rows, or a side is missing/not a clean table."""
    pb, po, pt = _rows(base), _rows(ours), _rows(theirs)
    if pb is None or po is None or pt is None:
        return None
    (hb, ob, rb), (ho, oo, ro), (ht, ot, rt) = pb, po, pt
    if ht == hb:
        header = ho
    elif ho == hb or ho == ht:
        header = ht
    else:
        return None
    if not _same_relative_order(ob, ot):  # the lane moved rows: not a row edit
        return None
    result = dict(ro)
    out_order = list(oo)
    for key in set(rb) | set(ro) | set(rt):
        b, o, t = rb.get(key), ro.get(key), rt.get(key)
        if t == b:
            continue
        if o == b:
            if t is None:
                result.pop(key, None)
            else:
                result[key] = t
        elif o != t:
            return None
    # rows new in theirs go right after their nearest predecessor (in theirs) that we keep
    for i, key in enumerate(ot):
        if key in out_order or key not in result:
            continue
        pos = 0
        for prev in reversed(ot[:i]):
            if prev in out_order:
                pos = out_order.index(prev) + 1
                break
        out_order.insert(pos, key)
    lines = [header] + [result[k] for k in out_order if k in result]
    nl = "\n" if (ours.endswith("\n") or theirs.endswith("\n")) else ""
    return "\n".join(lines) + nl


# ---------------------------------------------------------------- commands

def cmd_status(ctx: Ctx, a):
    reg = ctx.load()
    lk = read_lock(ctx)
    if a.json:
        print(json.dumps({"lanes": reg["lanes"], "main_lock": lk}, indent=2))
        return 0
    main_head = git("rev-parse", "--short", "main", cwd=ctx.main)
    print(f"main {main_head}   lock: " + (f"HELD by {lk.get('label') or lk.get('session')} "
                                          f"({int(time.time() - float(lk.get('ts', 0)))}s)"
                                          if lk else "free"))
    if not reg["lanes"]:
        print("no lanes registered")
    for lane, e in sorted(reg["lanes"].items()):
        d = ctx.lane_dir(lane)
        ahead = "-"
        if d.exists():
            try:
                ahead = len(unlanded(ctx, lane, ctx.branch(lane), reg))
            except Exception:
                ahead = "?"
        owner = e.get("owner") or "(closed)"
        tag = " STALE" if e.get("owner") and is_stale(ctx, e) else ""
        mine = " (you)" if e.get("owner") == ctx.session else ""
        print(f"  {lane:<16} {e.get('kind', '?'):<10} owner={owner[:8]}{mine}{tag}  "
              f"unlanded={ahead}  claims={' '.join(e.get('claims', [])) or '-'}")
    return 0


def unlanded(ctx: Ctx, lane: str, rev: str, reg=None, base: str = "main") -> list:
    """The lane commits up to `rev` that have not landed in `base`, oldest first. A commit has
    landed if it is in `base`, or if the main commit a land replayed it as (registry
    `landed_map`, lane sha -> main sha) is in `base`. A lane branch that could not be moved after
    a land still holds the originals, and replaying them would re-apply landed work over newer
    main changes; checked per commit, so a reset of main un-lands exactly what it removed."""
    lmap = (reg or ctx.load())["lanes"].get(lane, {}).get("landed_map", {})
    out = []
    for c in git("rev-list", "--reverse", rev, f"^{base}", cwd=ctx.main).split():
        m = lmap.get(c)
        if m and git_ok("merge-base", "--is-ancestor", m, base, cwd=ctx.main):
            continue
        out.append(c)
    return out


def finish_worktree(ctx: Ctx, d: Path):
    """Relative gitdir (WSL git cannot read a Windows `gitdir: C:/...` line) + toolchain bootstrap
    (private build/, junctioned toolchain)."""
    gd = git("rev-parse", "--git-dir", cwd=d)
    gd_abs = gd if os.path.isabs(gd) else str((d / gd).resolve())
    with open(d / ".git", "r+", encoding="utf-8", newline="\n") as f:
        f.write(f"gitdir: {os.path.relpath(gd_abs, d).replace(os.sep, '/')}\n")
        f.truncate()
    boot = ctx.cfg.get("bootstrap")
    if boot:
        env = dict(os.environ, WORKTREE_STALE_POLICY="fail")
        r = subprocess.run([*boot[:-1], str(d / boot[-1])] if boot[-1].endswith(".ps1") else boot,
                           cwd=d, env=env)
        if r.returncode != 0:
            raise Refuse(f"bootstrap failed in {d}; the next open/land retries it, or remove it "
                         f"with pwsh -NoProfile -File tools/safe_remove_worktree.ps1 {d} -Force")
    boot_marker(d).write_text(f"{time.time()}\n", encoding="utf-8")


def boot_marker(d: Path) -> Path:
    """Written (inside the worktree's git dir, so the gates never see it) once the bootstrap
    succeeded; a tree without it is re-bootstrapped instead of reused half set up."""
    gd = git("rev-parse", "--git-dir", cwd=d)
    return Path(gd if os.path.isabs(gd) else d / gd) / "lanes-bootstrapped"


def ensure_land_tree(ctx: Ctx, lane: str) -> Path:
    """The lane's private landing worktree (detached; only `land` uses it), so a land never
    touches the lane worktree the agent keeps working in."""
    d = ctx.land_dir(lane)
    if d.exists():
        if not (d / ".git").exists():
            raise Refuse(f"{d} exists but is not a worktree; inspect it by hand")
        if not boot_marker(d).exists():  # an earlier bootstrap failed: redo it at main
            run(["git", "cherry-pick", "--abort"], cwd=d, check=False)
            git("checkout", "-q", "--detach", "-f", "main", cwd=d)
            finish_worktree(ctx, d)
        return d
    d.parent.mkdir(parents=True, exist_ok=True)
    git("worktree", "add", "-q", "--detach", str(d), "main", cwd=ctx.main)
    finish_worktree(ctx, d)
    return d


def ensure_worktree(ctx: Ctx, lane: str) -> Path:
    d = ctx.lane_dir(lane)
    br = ctx.branch(lane)
    if d.exists():
        if not (d / ".git").exists():
            raise Refuse(f"{d} exists but is not a worktree; inspect it by hand")
        if not boot_marker(d).exists():  # an earlier bootstrap failed
            if git("status", "--porcelain", "--untracked-files=no", cwd=d) or \
                    unlanded(ctx, lane, br):
                raise Refuse(f"{d} was never bootstrapped and holds work; bootstrap it by hand "
                             f"(tools/setup_worker_worktree.ps1) or remove it with "
                             f"tools/safe_remove_worktree.ps1")
            git("checkout", "-q", "-B", br, "main", cwd=d)
            finish_worktree(ctx, d)
        return d
    if d.name.startswith("bb2-work-"):
        raise Refuse("bb2-work-* names re-arm the worktree-era guards")
    ctx.wt_root.mkdir(parents=True, exist_ok=True)
    if git_ok("rev-parse", "--verify", "-q", f"refs/heads/{br}", cwd=ctx.main):
        if unlanded(ctx, lane, br):
            raise Refuse(f"branch {br} exists with unlanded commits but no worktree; "
                         f"inspect it (git log main..{br})")
        git("branch", "-f", br, "main", cwd=ctx.main)
        git("worktree", "add", str(d), br, cwd=ctx.main)
    else:
        git("worktree", "add", "-b", br, str(d), "main", cwd=ctx.main)
    finish_worktree(ctx, d)
    return d


def cmd_open(ctx: Ctx, a):
    lane = a.lane
    if not LANE_RE.match(lane):
        raise Refuse(f"bad lane name '{lane}' (lowercase letters, digits, dashes)")
    kind = a.kind or ("naming" if lane.startswith("naming") else "foundation")

    def take(reg):
        for other, e in reg["lanes"].items():
            if other != lane and e.get("owner") == ctx.session and not a.allow_multi:
                raise Refuse(f"this session already owns lane '{other}'; close it first "
                             f"(one session, one lane)")
        e = reg["lanes"].get(lane, {})
        if e.get("owner") and e["owner"] != ctx.session and not is_stale(ctx, e) and not a.steal:
            raise Refuse(f"lane '{lane}' is owned by live session {e['owner']}; pick another lane")
        prev = e.get("owner")
        e.update({"owner": ctx.session, "kind": kind, "heartbeat": time.time(),
                  "opened": e.get("opened") if prev == ctx.session else time.time()})
        if prev and prev != ctx.session:
            e["stolen_from"] = prev
        e.setdefault("claims", [])
        reg["lanes"][lane] = e
        return prev

    take_prev = ctx.update(take)
    d = ensure_worktree(ctx, lane)
    dirty = git("status", "--porcelain", "--untracked-files=no", cwd=d)
    ahead = len(unlanded(ctx, lane, ctx.branch(lane)))
    behind = git("rev-list", "--count", f"{ctx.branch(lane)}..main", cwd=ctx.main)
    print(f"lanes: lane '{lane}' ({kind}) owned by {ctx.session}"
          + (f" (taken over from {take_prev})" if take_prev and take_prev != ctx.session else ""))
    print(f"lanes: worktree {d}\nlanes: wsl      {wsl_path(d)}\nlanes: branch   {ctx.branch(lane)}"
          f"  unlanded={ahead} behind-main={behind}" + ("  DIRTY" if dirty else ""))
    return 0


def cmd_fresh(ctx: Ctx, a):
    ctx.update(lambda reg: require_owner(ctx, reg, a.lane))
    d = ctx.lane_dir(a.lane)
    if git("status", "--porcelain", "--untracked-files=no", cwd=d):
        raise Refuse(f"{d} has uncommitted changes; commit or discard them first")
    ahead = len(unlanded(ctx, a.lane, git("rev-parse", "HEAD", cwd=d)))
    if ahead and not a.discard:
        raise Refuse(f"lane has {ahead} unlanded commit(s); land them, drop one with "
                     f"`lanes.py drop {a.lane} <sha>`, or pass --discard")
    if ahead:
        backup_lane(ctx, a.lane, d, "discarded")
    git("checkout", "-q", "-B", ctx.branch(a.lane), "main", cwd=d)
    print(f"lanes: {a.lane} reset to main {git('rev-parse', '--short', 'HEAD', cwd=d)}")
    return 0


def backup_lane(ctx: Ctx, lane: str, d: Path, what: str):
    """Save the lane tip under refs/lanes-backup/ and list the unlanded commits being rewritten."""
    ref = (f"refs/lanes-backup/{lane}/{time.strftime('%Y%m%d-%H%M%S')}-"
           f"{git('rev-parse', '--short', 'HEAD', cwd=d)}")
    git("update-ref", ref, "HEAD", cwd=d)
    print(f"lanes: {what} from {ctx.branch(lane)} (tip saved as {ref}):")
    for sha in reversed(unlanded(ctx, lane, git("rev-parse", "HEAD", cwd=d))):
        print(f"  {git('log', '-1', '--oneline', sha, cwd=d)}")


def cmd_drop(ctx: Ctx, a):
    """Remove one unlanded commit (typically the one a land bounced on) and keep the rest."""
    ctx.update(lambda reg: require_owner(ctx, reg, a.lane))
    d, br = ctx.lane_dir(a.lane), ctx.branch(a.lane)
    if land_running(ctx, a.lane):
        raise Refuse(f"a land of '{a.lane}' is running; wait for it to finish")
    if git("status", "--porcelain", "--untracked-files=no", cwd=d):
        raise Refuse(f"{d} has uncommitted changes; commit or discard them first")
    sha = git("rev-parse", "--verify", "-q", f"{a.sha}^{{commit}}", cwd=d, check=False)
    if not sha or sha not in unlanded(ctx, a.lane, br):
        raise Refuse(f"{a.sha} is not an unlanded commit of {br}")
    if git("rev-list", "--merges", f"{sha}~1..{br}", cwd=d):
        raise Refuse("merge commits after it; rebuild the lane with fresh --discard")
    backup_lane(ctx, a.lane, d, f"dropping {sha[:9]}")
    r = run(["git", "rebase", "-q", "--onto", f"{sha}~1", sha, br], cwd=d, check=False)
    if r.returncode != 0:
        run(["git", "rebase", "--abort"], cwd=d, check=False)
        raise Refuse(f"could not drop {sha[:9]} (rebase failed, most likely a later commit depends "
                     f"on it; lane unchanged):\n"
                     f"{r.stdout}{r.stderr}")
    print(f"lanes: dropped {sha[:9]}; {br} keeps "
          f"{len(unlanded(ctx, a.lane, br))} unlanded commit(s)")
    return 0


def cmd_claim(ctx: Ctx, a):
    def do(reg):
        e = require_owner(ctx, reg, a.lane)
        for tok in a.tokens:
            kind, val, freeze = split_token(tok)
            if kind == "path" and is_shared(ctx, val) and not freeze:
                raise Refuse(f"{val} is a shared path; claim it only as a freeze "
                             f"(path:{val}!), which blocks every other lane's landing on it")
            for other, oe in reg["lanes"].items():
                if other == a.lane or not oe.get("owner") or is_stale(ctx, oe):
                    continue
                for otok in oe.get("claims", []):
                    ok_, oval, _ = split_token(otok)
                    if ok_ != kind:
                        continue
                    if (kind == "topic" and oval == val) or (kind == "path" and globs_overlap(oval, val)):
                        raise Refuse(f"'{tok}' overlaps lane '{other}' claim '{otok}'")
            canon = f"{kind}:{val}{'!' if freeze else ''}"
            if canon not in e["claims"]:
                e["claims"].append(canon)
        return list(e["claims"])

    claims = ctx.update(do)
    print(f"lanes: {a.lane} claims: {' '.join(claims)}")
    return 0


def cmd_unclaim(ctx: Ctx, a):
    def do(reg):
        e = require_owner(ctx, reg, a.lane)
        if not a.tokens:
            e["claims"] = []
        else:
            drop = {f"{k}:{v}{'!' if f else ''}" for k, v, f in map(split_token, a.tokens)}
            e["claims"] = [c for c in e["claims"] if c not in drop]
        return list(e["claims"])

    print(f"lanes: {a.lane} claims: {' '.join(ctx.update(do)) or '-'}")
    return 0


def cmd_close(ctx: Ctx, a):
    d = ctx.lane_dir(a.lane)

    def do(reg):
        e = require_owner(ctx, reg, a.lane)
        if d.exists():
            ahead = len(unlanded(ctx, a.lane, ctx.branch(a.lane), reg))
            if ahead and not a.force:
                raise Refuse(f"lane has {ahead} unlanded commit(s); land them or pass --force "
                             f"(the branch is kept either way)")
        e["owner"] = None
        e["claims"] = []
        e["closed"] = time.time()

    ctx.update(do)
    print(f"lanes: closed {a.lane}")
    if a.remove:
        if d.exists() and git("status", "--porcelain", "--untracked-files=no", cwd=d):
            raise Refuse(f"{d} is dirty; not removing it")
        for tree in (d, ctx.land_dir(a.lane)):  # the lane and its private landing worktree
            if not tree.exists():
                continue
            r = subprocess.run(["pwsh", "-NoProfile", "-File",
                                str(ctx.main / "tools" / "safe_remove_worktree.ps1"), str(tree), "-Force"])
            if r.returncode != 0:
                raise Refuse(f"safe_remove_worktree.ps1 failed for {tree}")
            print(f"lanes: removed worktree {tree}")
    return 0


def cmd_path(ctx: Ctx, a):
    d = ctx.lane_dir(a.lane)
    print(wsl_path(d) if a.wsl else str(d))
    return 0


# ---------------------------------------------------------------- land

def conflicted(wt: Path):
    out = git("diff", "--name-only", "--diff-filter=U", cwd=wt, check=False)
    return [p for p in out.splitlines() if p]


def show_stage(wt: Path, stage: int, path: str) -> str:
    r = run(["git", "show", f":{stage}:{path}"], cwd=wt, check=False)
    return r.stdout if r.returncode == 0 else ""


def picking(wt: Path) -> bool:
    return git_ok("rev-parse", "-q", "--verify", "CHERRY_PICK_HEAD", cwd=wt)


def index_matches_head(wt: Path) -> bool:
    return git_ok("diff", "--cached", "--quiet", "HEAD", cwd=wt)


def replay_commit(ctx: Ctx, wt: Path, sha: str, log) -> str:
    """Apply one lane commit on HEAD. Returns 'picked' | 'resolved' | 'replayed' | 'empty'.
    Raises Refuse(code=3) on a conflict it may not resolve."""
    r = run(["git", "cherry-pick", "--allow-empty", sha], cwd=wt, check=False)
    if r.returncode == 0:
        return "picked"
    files = conflicted(wt)
    if not files:
        # Only a cherry-pick that stopped because the change is already in main may be skipped.
        # Anything else (a refusal, an error) must never silently drop the commit.
        if picking(wt) and index_matches_head(wt):
            run(["git", "cherry-pick", "--skip"], cwd=wt, check=False)
            return "empty"
        run(["git", "cherry-pick", "--abort"], cwd=wt, check=False)
        raise Refuse(f"cherry-pick of {sha[:9]} failed without a conflict:\n"
                     f"{(r.stdout or '')[-1500:]}{(r.stderr or '')[-1500:]}", 3)
    regen_paths = {g["path"] for g in ctx.cfg.get("regenerate", [])}
    rowmerge_paths = set(ctx.cfg.get("rowmerge", []))
    unresolved = []
    for f in files:
        if f in regen_paths:
            ok = run(["git", "checkout", "--ours", "--", f], cwd=wt, check=False).returncode == 0
            if not ok:
                unresolved.append(f)
                continue
            git("add", "--", f, cwd=wt)  # main's copy; rebuilt after the gates
        elif f in rowmerge_paths:
            merged = rowmerge(show_stage(wt, 1, f), show_stage(wt, 2, f), show_stage(wt, 3, f))
            if merged is None:
                unresolved.append(f)
                continue
            (wt / f).write_text(merged, encoding="utf-8", newline="\n")
            git("add", "--", f, cwd=wt)
        else:
            unresolved.append(f)
    if not unresolved:
        if index_matches_head(wt):
            run(["git", "cherry-pick", "--skip"], cwd=wt, check=False)
            return "empty"
        env = dict(os.environ, GIT_EDITOR="true")
        rr = run(["git", "cherry-pick", "--continue"], cwd=wt, check=False, env=env)
        if rr.returncode != 0:
            raise Refuse(f"cherry-pick --continue failed for {sha[:9]}:\n{rr.stdout}{rr.stderr}", 3)
        return "resolved"
    msg = git("log", "-1", "--format=%B", sha, cwd=wt)
    cmds = TRAILER_RE.findall(msg)
    run(["git", "cherry-pick", "--abort"], cwd=wt, check=False)
    if not cmds:
        raise Refuse(f"commit {sha[:9]} conflicts with main in: {', '.join(unresolved)}\n"
                     f"  (A pure tool-output commit can carry Lane-Replay: trailers instead.)", 3)
    print(f"lanes:   {sha[:9]} conflicts in {', '.join(unresolved)}; replaying its Lane-Replay commands")
    before = set(git("status", "--porcelain", cwd=wt).splitlines())
    for c in cmds:
        if run_shell(ctx, wt, c, log) != 0:
            git("reset", "-q", "--hard", "HEAD", cwd=wt)
            raise Refuse(f"Lane-Replay command failed for {sha[:9]}: {c}", 3)
    # Stage only what the original commit touched; anything else the commands left behind is an
    # error (it would otherwise land unreviewed).
    paths = git("diff-tree", "--no-commit-id", "--name-only", "-r", sha, cwd=wt).splitlines()
    if paths:
        git("add", "-A", "--", *paths, cwd=wt)
    stray = [ln for ln in git("status", "--porcelain", cwd=wt).splitlines()
             if ln not in before and ln[3:] not in paths]
    if stray:
        git("reset", "-q", "--hard", "HEAD", cwd=wt)
        raise Refuse(f"Lane-Replay for {sha[:9]} changed paths its commit did not: "
                     f"{', '.join(x[3:] for x in stray[:10])}", 3)
    if index_matches_head(wt):
        return "empty"
    msgfile = wt / "tmp" / "lanes" / "replay-msg.txt"
    msgfile.parent.mkdir(parents=True, exist_ok=True)
    msgfile.write_text(msg if msg.endswith("\n") else msg + "\n", encoding="utf-8", newline="\n")
    author = git("log", "-1", "--format=%an <%ae>", sha, cwd=wt)
    git("commit", "-q", "-F", str(msgfile), f"--author={author}", cwd=wt)
    return "replayed"


def pid_alive(pid: int) -> bool:
    if os.name == "nt":  # os.kill(pid, 0) would TerminateProcess on Windows
        import ctypes
        h = ctypes.windll.kernel32.OpenProcess(0x1000, False, pid)  # QUERY_LIMITED_INFORMATION
        if not h:
            return False
        code = ctypes.c_ulong()
        ctypes.windll.kernel32.GetExitCodeProcess(h, ctypes.byref(code))
        ctypes.windll.kernel32.CloseHandle(h)
        return code.value == 259  # STILL_ACTIVE
    try:
        os.kill(pid, 0)
        return True
    except OSError:
        return False


def check_main_idle(ctx: Ctx):
    """Main must be on main, have no tracked edits, and no Grinder run may be live there (its
    sessions keep candidate edits in main and its scope check discards a moved HEAD)."""
    if git("rev-parse", "--abbrev-ref", "HEAD", cwd=ctx.main) != "main":
        raise Refuse("the main checkout is not on branch main")
    lockf = ctx.main / "tmp" / "grind" / "grind.lock"
    if lockf.exists():
        try:
            pid = int(lockf.read_text(encoding="utf-8-sig").split()[0])
        except Exception:
            pid = 0
        if pid and pid_alive(pid):
            raise Refuse(f"the Grinder is running in main (pid {pid}); lanes land between Grinder "
                         f"runs (pwsh tools/grinder/grind.ps1 -Stop)")
    if git("status", "--porcelain", "--untracked-files=no", cwd=ctx.main):
        raise Refuse("the main checkout has uncommitted tracked changes; under the lane model "
                     "nobody edits main. Commit or move them first.")


UNTRACKED_OK = {".venv"}  # the toolchain link setup_worker_worktree.ps1 creates


def check_untracked(wt: Path):
    """Untracked (non-ignored) files would feed the gates but never reach main."""
    out = git("status", "--porcelain", "--untracked-files=all", cwd=wt)
    extra = [ln[3:] for ln in out.splitlines()
             if ln.startswith("?? ") and ln[3:].rstrip("/") not in UNTRACKED_OK]
    if extra:
        raise Refuse("the landing worktree has untracked files the gates would see: "
                     + ", ".join(extra[:10]) + ("..." if len(extra) > 10 else ""), 4)


def paths_of(ctx: Ctx, commits) -> list:
    out = set()
    for c in commits:
        out.update(git("diff-tree", "--no-commit-id", "--name-only", "-r", c, cwd=ctx.main).split())
    return sorted(out)


def mark_landed(ctx: Ctx, lane: str, replayed: dict):
    """Record which main commit each lane commit landed as (see unlanded); entries the lane
    branch no longer reaches are pruned."""
    live = set(git("rev-list", ctx.branch(lane), "^main", cwd=ctx.main).split()) | set(replayed)

    def do(reg):
        e = reg["lanes"].setdefault(lane, {})
        m = {k: v for k, v in e.get("landed_map", {}).items() if k in live}
        m.update(replayed)
        e["landed_map"] = m
        e.pop("landed", None); e.pop("landed_main", None)
    ctx.update(do)


def check_claims(ctx: Ctx, lane: str, touched):
    reg = ctx.load()
    for p in touched:
        hit = claimed_by_other(ctx, reg, lane, p)
        if hit:
            raise Refuse(f"{p} is claimed by lane '{hit[0]}' ({hit[1]}{'!' if hit[2] else ''}); "
                         f"it cannot land until that claim is released")


def land_pidfile(ctx: Ctx, lane: str) -> Path:
    return ctx.state_dir / f"land-{lane}.pid"


def land_running(ctx: Ctx, lane: str) -> int:
    """pid of a live land of this lane, else 0."""
    try:
        pid = int(land_pidfile(ctx, lane).read_text(encoding="utf-8").split()[0])
    except Exception:
        return 0
    return pid if pid != os.getpid() and pid_alive(pid) else 0


def recover_lane(ctx: Ctx, lane: str, quiet=False) -> bool:
    """Undo a land that died midway. The lane worktree and branch were never touched by it; this
    resets the private landing worktree and releases a main lock that land left behind (held by
    this session, or by the session this lane was taken over from). Refuses while a land of this
    lane is still running. Returns True if anything was recovered."""
    pid = land_running(ctx, lane)
    if pid:
        raise Refuse(f"a land of '{lane}' is still running (pid {pid}); wait for it to finish")
    did = False
    ld = ctx.land_dir(lane)
    if ld.exists():
        if picking(ld):
            run(["git", "cherry-pick", "--abort"], cwd=ld, check=False)
            did = True
        if git("status", "--porcelain", "--untracked-files=no", cwd=ld):
            run(["git", "reset", "-q", "--hard"], cwd=ld, check=False)
            did = True
    lk = read_lock(ctx)
    e = ctx.load()["lanes"].get(lane, {})
    if lk and lk.get("label") == f"lanes land {lane}" and \
            lk.get("session") in (ctx.session, e.get("stolen_from")):
        try:
            (ctx.main / LOCK_REL).unlink()
        except FileNotFoundError:
            pass
        did = True
    try:
        land_pidfile(ctx, lane).unlink()
    except FileNotFoundError:
        pass
    if did and not quiet:
        print(f"lanes: recovered {lane} from an interrupted land (lane branch untouched)")
    return did


def cmd_recover(ctx: Ctx, a):
    ctx.update(lambda reg: require_owner(ctx, reg, a.lane))
    if not recover_lane(ctx, a.lane):
        print(f"lanes: {a.lane}: nothing to recover")
    return 0


def build_candidate(ctx: Ctx, lane: str, ld: Path, commits, base: str, log) -> str:
    """Replay the lane's commits onto `base` in the private landing worktree, gate, regenerate.
    No lock is held: nothing here touches main or the lane worktree. Returns the gated tip
    (equal to `base` if every commit was already in main)."""
    run(["git", "cherry-pick", "--abort"], cwd=ld, check=False)
    git("checkout", "-q", "--detach", "-f", base, cwd=ld)
    git("reset", "-q", "--hard", base, cwd=ld)
    ctx.replayed = {}  # lane sha -> the candidate commit holding it (base's tip if it was empty)
    for sha in commits:
        try:
            how = replay_commit(ctx, ld, sha, log)
        except Refuse as e:
            if e.code == 3:  # every bounce names the commit and the way out
                raise Refuse(f"{e}\n  Drop just this commit (later commits are kept) and redo it "
                             f"on the new main: python tools/lanes/lanes.py drop {lane} {sha[:9]}", 3)
            raise
        ctx.replayed[sha] = git("rev-parse", "HEAD", cwd=ld)
        print(f"lanes:   {sha[:9]} {how}", flush=True)
    new_paths = set(git("diff", "--name-only", "--no-renames", f"{base}..HEAD", cwd=ld).split())
    if not new_paths:
        return git("rev-parse", "HEAD", cwd=ld)
    docs_only = all(any(p.startswith(d) for d in ctx.cfg.get("docs_only_paths", [])) for p in new_paths)
    kind = ctx.load()["lanes"][lane].get("kind", "foundation")
    gates = [] if docs_only else ctx.cfg["gates"].get(kind, ctx.cfg["gates"]["foundation"])
    if docs_only:
        print("lanes: docs-only change; build gates skipped")
    check_untracked(ld)
    for g in gates:
        if run_shell(ctx, ld, g, log) != 0:
            raise Refuse(f"gate failed: {g}  (log {log.name})", 4)
    if gates and ctx.cfg.get("oracle_sha1"):
        exe = ld / ctx.cfg["oracle_exe"]
        got = hashlib.sha1(exe.read_bytes()).hexdigest() if exe.exists() else "missing"
        if got != ctx.cfg["oracle_sha1"]:
            raise Refuse(f"{exe} SHA1 {got} != oracle {ctx.cfg['oracle_sha1']}", 4)
        print(f"lanes: SHA1 == oracle ({got[:12]})")
    if not docs_only:
        for g in ctx.cfg.get("regenerate", []):
            if run_shell(ctx, ld, g["cmd"], log) != 0:
                raise Refuse(f"regenerate failed: {g['cmd']}", 4)
            git("add", "--", g["path"], cwd=ld)
            if not git_ok("diff", "--cached", "--quiet", "--", g["path"], cwd=ld):
                msgfile = ctx.state_dir / f"regen-msg-{lane}.txt"
                msgfile.write_text(f"{g['subject']} after lane {lane}\n", encoding="utf-8", newline="\n")
                git("commit", "-q", "-F", str(msgfile), cwd=ld)
                print(f"lanes:   regenerated {g['path']}")
    if git("status", "--porcelain", "--untracked-files=no", cwd=ld):
        raise Refuse("the gates left tracked changes in the landing worktree: "
                     + git("status", "--porcelain", "--untracked-files=no", cwd=ld), 4)
    return git("rev-parse", "HEAD", cwd=ld)


def sync_main_with_remote(ctx: Ctx, remote: str):
    """Under the lock: main must not hold commits origin lacks (they would be pushed unreviewed
    with ours); fast-forward main if origin is ahead."""
    git("fetch", "-q", remote, "main", cwd=ctx.main)
    rmain = f"{remote}/main"
    local_only = git("rev-list", "--oneline", f"{rmain}..main", cwd=ctx.main)
    if local_only:
        raise Refuse(f"main has commits that are not on {rmain}; a land would push them along "
                     f"unreviewed. If they are an earlier land whose push failed, run "
                     f"`lanes.py push`; otherwise investigate:\n  "
                     + "\n  ".join(local_only.splitlines()[:8]))
    if git("rev-parse", "main", cwd=ctx.main) != git("rev-parse", rmain, cwd=ctx.main):
        check_main_idle(ctx)
        git("merge", "-q", "--ff-only", rmain, cwd=ctx.main)


def advance_lane(ctx: Ctx, lane: str, landed_from: str):
    """After a land, move the lane branch onto the new main without touching uncommitted work:
    commits made during the land are rebased onto main; with local edits they stay as they are
    (the next land skips the already-landed ones: see unlanded())."""
    wt, br = ctx.lane_dir(lane), ctx.branch(lane)
    m = git("rev-parse", "main", cwd=ctx.main)  # one value throughout: main may move meanwhile
    left = unlanded(ctx, lane, landed_from, base=m)
    if left:  # main changed under the land (e.g. a reset): never move work off the branch
        print(f"lanes: note: {len(left)} commit(s) of {br} are not in main (it changed during "
              f"the land); {br} was left as is, land again")
        return
    run(["git", "update-index", "-q", "--refresh"], cwd=wt, check=False)  # stale stat info
    # (a rewritten-but-unchanged file) makes reset --keep refuse
    head = git("rev-parse", "HEAD", cwd=wt)  # read last, right before the move
    if head == landed_from:
        r = run(["git", "reset", "-q", "--keep", m], cwd=wt, check=False)
        if r.returncode != 0:
            print(f"lanes: note: {br} was left as is to keep your uncommitted edits; nothing to "
                  f"do: the next `land` skips the already-landed commits")
            return
        # a commit made between the HEAD read and the reset is in the reflog entry the reset
        # wrote; put the branch back on it rather than strand it
        old = git("rev-parse", f"{br}@{{1}}", cwd=wt, check=False)
        if old and old != head and unlanded(ctx, lane, old, base=m):
            git("update-ref", f"refs/lanes-backup/{lane}/raced-{old[:9]}", old, cwd=wt)
            run(["git", "reset", "-q", "--keep", old], cwd=wt, check=False)
            print(f"lanes: note: a commit arrived while {br} was being moved; it was kept "
                  f"({br} at {git('rev-parse', '--short', 'HEAD', cwd=wt)}, also saved as "
                  f"refs/lanes-backup/{lane}/raced-{old[:9]})")
        return
    if git("status", "--porcelain", "--untracked-files=no", cwd=wt):
        print(f"lanes: note: {br} gained commits during the land and has uncommitted edits; it "
              f"was left as is (the next land skips the already-landed commits)")
        return
    git("update-ref", f"refs/lanes-backup/{lane}/advance-{head[:9]}", head, cwd=wt)
    r = run(["git", "rebase", "-q", "--onto", m, landed_from, br], cwd=wt, check=False)
    if r.returncode != 0:
        run(["git", "rebase", "--abort"], cwd=wt, check=False)
        print(f"lanes: note: the commits made during the land did not rebase cleanly onto main; "
              f"{br} was left as is")
    else:
        print(f"lanes: {br}: commits made during the land were rebased onto the new main")


def cmd_land(ctx: Ctx, a):
    lane = a.lane
    ctx.update(lambda reg: require_owner(ctx, reg, lane))
    wt, br = ctx.lane_dir(lane), ctx.branch(lane)
    recover_lane(ctx, lane)  # refuses if a land of this lane is still running
    tipsrc = git("rev-parse", br, cwd=ctx.main)  # land exactly this; later lane commits wait
    commits = unlanded(ctx, lane, tipsrc)
    if not commits:
        print("lanes: nothing to land")
        if wt.exists() and tipsrc != git("rev-parse", "main", cwd=ctx.main):
            advance_lane(ctx, lane, tipsrc)  # a branch an earlier land had to leave as is
        return 0
    if any(len(git("rev-list", "--parents", "-n1", c, cwd=ctx.main).split()) > 2 for c in commits):
        raise Refuse("the lane holds merge commits; replay cannot carry them. Rebuild the lane as "
                     "linear commits on fresh main (lanes.py fresh --discard).")
    check_claims(ctx, lane, paths_of(ctx, commits))
    remote = None if a.no_push else ctx.cfg.get("remote")
    land_pidfile(ctx, lane).write_text(str(os.getpid()), encoding="utf-8")
    ld = ensure_land_tree(ctx, lane)

    logp = ctx.state_dir / "logs" / f"{lane}-{time.strftime('%Y%m%d-%H%M%S')}.log"
    logp.parent.mkdir(parents=True, exist_ok=True)
    log = open(logp, "w", encoding="utf-8", newline="\n")
    lk = read_lock(ctx)
    held_before = bool(lk and lk.get("session") == ctx.session)  # e.g. a codex-worker landing
    try:
        for attempt in range(3):
            base = git("rev-parse", "main", cwd=ctx.main)
            # against this attempt's base: main may have moved, or been reset, since the last look
            commits = unlanded(ctx, lane, tipsrc, base=base)
            if any(len(git("rev-list", "--parents", "-n1", c, cwd=ctx.main).split()) > 2
                   for c in commits):
                raise Refuse("the lane holds merge commits; rebuild it as linear commits")
            print(f"lanes: landing {len(commits)} commit(s) of {lane} onto main {base[:9]}"
                  + (f" (retry {attempt}: main moved)" if attempt else ""), flush=True)
            tip = build_candidate(ctx, lane, ld, commits, base, log)  # no lock held
            if tip == base:
                print("lanes: every lane commit is already in main; nothing new to land")
                mark_landed(ctx, lane, ctx.replayed)
                advance_lane(ctx, lane, tipsrc)
                return 0
            # Short critical section: verify nothing moved, then fast-forward and push.
            if not held_before:
                acquire_lock(ctx, f"lanes land {lane}")
            try:
                check_claims(ctx, lane,  # a freeze may have arrived meanwhile
                             git("diff", "--name-only", "--no-renames", f"{base}..{tip}",
                                 cwd=ctx.main).split())
                check_main_idle(ctx)
                if remote:
                    sync_main_with_remote(ctx, remote)
                if git("rev-parse", "main", cwd=ctx.main) != base:
                    continue  # main moved: rebuild on the new main (finally releases the lock)
                # recorded first: a mapping is trusted only once its commit is in main, so this
                # is harmless if the fast-forward fails, and a failure here cannot leave main
                # holding lane work it does not know has landed
                mark_landed(ctx, lane, ctx.replayed)
                git("merge", "-q", "--ff-only", tip, cwd=ctx.main)
                pushed = push_main(ctx, remote) if remote else "not pushed: --no-push"
            finally:
                if not held_before:
                    release_lock(ctx)
            print(f"lanes: main -> {tip[:9]} ({pushed})")
            advance_lane(ctx, lane, tipsrc)
            return 0
        raise Refuse("main kept moving during three land attempts; try again", 1)
    finally:
        run(["git", "cherry-pick", "--abort"], cwd=ld, check=False)
        try:
            land_pidfile(ctx, lane).unlink()
        except FileNotFoundError:
            pass
        log.close()


def push_main(ctx: Ctx, remote: str) -> str:
    r = run(["git", "push", "-q", remote, "main"], cwd=ctx.main, check=False)
    if r.returncode != 0:
        raise Refuse(f"main moved locally, but the PUSH FAILED: {(r.stderr or '').strip()[-400:]}"
                     f"\n  Retry with `python tools/lanes/lanes.py push` before the next land.", 5)
    return "pushed"


def cmd_push(ctx: Ctx, a):
    """Retry the push after a land whose push failed (exit 5). Pushes only a fast-forward."""
    remote = ctx.cfg.get("remote") or "origin"
    acquire_lock(ctx, "lanes push")
    try:
        git("fetch", "-q", remote, "main", cwd=ctx.main)
        if not git_ok("merge-base", "--is-ancestor", f"{remote}/main", "main", cwd=ctx.main):
            raise Refuse(f"main is not a fast-forward of {remote}/main; resolve by hand")
        ahead = git("rev-list", "--oneline", f"{remote}/main..main", cwd=ctx.main)
        if not ahead:
            print("lanes: main is already pushed")
            return 0
        print("lanes: pushing:\n  " + "\n  ".join(ahead.splitlines()[:10]))
        push_main(ctx, remote)
        print("lanes: pushed")
        return 0
    finally:
        release_lock(ctx)


# ---------------------------------------------------------------- cli

def main(argv=None) -> int:
    p = argparse.ArgumentParser(prog="lanes.py", description=__doc__.split("\n")[0])
    p.add_argument("--session", default=os.environ.get("CLAUDE_CODE_SESSION_ID", ""))
    sub = p.add_subparsers(dest="cmd", required=True)
    s = sub.add_parser("status"); s.add_argument("--json", action="store_true")
    s = sub.add_parser("open"); s.add_argument("lane"); s.add_argument("--kind", choices=["naming", "foundation"])
    s.add_argument("--steal", action="store_true"); s.add_argument("--allow-multi", action="store_true")
    s = sub.add_parser("fresh"); s.add_argument("lane"); s.add_argument("--discard", action="store_true")
    s = sub.add_parser("drop"); s.add_argument("lane"); s.add_argument("sha")
    s = sub.add_parser("claim"); s.add_argument("lane"); s.add_argument("tokens", nargs="+")
    s = sub.add_parser("unclaim"); s.add_argument("lane"); s.add_argument("tokens", nargs="*")
    s = sub.add_parser("land"); s.add_argument("lane"); s.add_argument("--no-push", action="store_true")
    s = sub.add_parser("close"); s.add_argument("lane"); s.add_argument("--remove", action="store_true")
    s.add_argument("--force", action="store_true")
    s = sub.add_parser("path"); s.add_argument("lane"); s.add_argument("--wsl", action="store_true")
    s = sub.add_parser("recover"); s.add_argument("lane")
    sub.add_parser("push")
    a = p.parse_args(argv)
    try:
        if a.cmd not in ("status", "path") and not a.session:
            raise Refuse("no session id: $CLAUDE_CODE_SESSION_ID is unset (pass --session)")
        ctx = Ctx(find_main(Path(__file__).resolve().parent), a.session)
        return {"status": cmd_status, "open": cmd_open, "fresh": cmd_fresh, "drop": cmd_drop, "claim": cmd_claim,
                "unclaim": cmd_unclaim, "land": cmd_land, "close": cmd_close,
                "path": cmd_path, "recover": cmd_recover, "push": cmd_push}[a.cmd](ctx, a)
    except Refuse as e:
        print(f"lanes: REFUSED: {e}", file=sys.stderr)
        return e.code


if __name__ == "__main__":
    sys.exit(main())
