#!/usr/bin/env python3
"""lanes.py: parallel work lanes for BB2, one agent session per git worktree.

A lane is a named worktree (`../bb2-worktrees/ln-<lane>`, branch `lanes/<lane>`) owned by exactly
one Claude session. Lanes never edit the main checkout. Main is only a landing pad: `land`
replays the lane's commits onto the current main in the lane's own worktree, runs the gates
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
  python tools/lanes/lanes.py fresh <lane> [--discard]
  python tools/lanes/lanes.py claim <lane> <token>...       # path:src/main/9F9C.c  topic:row:m09
  python tools/lanes/lanes.py unclaim <lane> [<token>...]   # no tokens = all
  python tools/lanes/lanes.py land  <lane> [--no-push]
  python tools/lanes/lanes.py close <lane> [--remove]
  python tools/lanes/lanes.py path  <lane> [--wsl]

The session identity is $CLAUDE_CODE_SESSION_ID (override with --session for tests).
Exit codes: 0 ok, 1 refused, 3 land bounced on a conflict, 4 a gate failed.
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
            ahead = git("rev-list", "--count", f"main..{ctx.branch(lane)}", cwd=ctx.main, check=False) or "?"
        owner = e.get("owner") or "(closed)"
        tag = " STALE" if e.get("owner") and is_stale(ctx, e) else ""
        mine = " (you)" if e.get("owner") == ctx.session else ""
        print(f"  {lane:<16} {e.get('kind', '?'):<10} owner={owner[:8]}{mine}{tag}  "
              f"unlanded={ahead}  claims={' '.join(e.get('claims', [])) or '-'}")
    return 0


def ensure_worktree(ctx: Ctx, lane: str) -> Path:
    d = ctx.lane_dir(lane)
    br = ctx.branch(lane)
    if d.exists():
        if not (d / ".git").exists():
            raise Refuse(f"{d} exists but is not a worktree; inspect it by hand")
        return d
    if d.name.startswith("bb2-work-"):
        raise Refuse("bb2-work-* names re-arm the worktree-era guards")
    ctx.wt_root.mkdir(parents=True, exist_ok=True)
    if git_ok("rev-parse", "--verify", "-q", f"refs/heads/{br}", cwd=ctx.main):
        if not git_ok("merge-base", "--is-ancestor", br, "main", cwd=ctx.main):
            raise Refuse(f"branch {br} exists with unlanded commits but no worktree; "
                         f"inspect it (git log main..{br})")
        git("branch", "-f", br, "main", cwd=ctx.main)
        git("worktree", "add", str(d), br, cwd=ctx.main)
    else:
        git("worktree", "add", "-b", br, str(d), "main", cwd=ctx.main)
    # WSL git cannot read a Windows `gitdir: C:/...` line: make it relative.
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
            raise Refuse(f"bootstrap failed in {d}; remove it with "
                         f"pwsh -NoProfile -File tools/safe_remove_worktree.ps1 {d} -Force")
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
    ahead = git("rev-list", "--count", f"main..{ctx.branch(lane)}", cwd=ctx.main)
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
    ahead = int(git("rev-list", "--count", "main..HEAD", cwd=d))
    if ahead and not a.discard:
        raise Refuse(f"lane has {ahead} unlanded commit(s); land them, or pass --discard")
    git("checkout", "-q", "-B", ctx.branch(a.lane), "main", cwd=d)
    print(f"lanes: {a.lane} reset to main {git('rev-parse', '--short', 'HEAD', cwd=d)}")
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
            ahead = int(git("rev-list", "--count", f"main..{ctx.branch(a.lane)}", cwd=ctx.main))
            if ahead and not a.force:
                raise Refuse(f"lane has {ahead} unlanded commit(s); land them or pass --force "
                             f"(the branch is kept either way)")
        e["owner"] = None
        e["claims"] = []
        e["closed"] = time.time()

    ctx.update(do)
    print(f"lanes: closed {a.lane}")
    if a.remove and d.exists():
        if git("status", "--porcelain", "--untracked-files=no", cwd=d):
            raise Refuse(f"{d} is dirty; not removing it")
        r = subprocess.run(["pwsh", "-NoProfile", "-File",
                            str(ctx.main / "tools" / "safe_remove_worktree.ps1"), str(d), "-Force"])
        if r.returncode != 0:
            raise Refuse(f"safe_remove_worktree.ps1 failed for {d}")
        print(f"lanes: removed worktree {d}")
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
    r = run(["git", "cherry-pick", "--allow-empty", "--keep-redundant-commits", sha], cwd=wt, check=False)
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
                     f"  The lane must redo it on fresh main (lanes.py fresh --discard), or mark a "
                     f"pure tool-output commit with Lane-Replay: trailers.", 3)
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


def cmd_land(ctx: Ctx, a):
    lane = a.lane
    ctx.update(lambda reg: require_owner(ctx, reg, lane))
    wt = ctx.lane_dir(lane)
    br, lbr = ctx.branch(lane), ctx.land_branch(lane)
    if git("rev-parse", "--abbrev-ref", "HEAD", cwd=wt) != br:
        raise Refuse(f"{wt} is not on {br}")
    if git("status", "--porcelain", "--untracked-files=no", cwd=wt):
        raise Refuse(f"{wt} has uncommitted changes; commit them first")
    commits = git("rev-list", "--reverse", "main..HEAD", cwd=wt).split()
    if not commits:
        print("lanes: nothing to land")
        return 0
    if git("rev-list", "--merges", "main..HEAD", cwd=wt):
        raise Refuse("the lane holds merge commits; replay cannot carry them. Rebuild the lane as "
                     "linear commits on fresh main (lanes.py fresh --discard).")

    touched = sorted(set(git("diff", "--name-only", "main...HEAD", cwd=wt).split()))
    reg = ctx.load()
    for p in touched:
        hit = claimed_by_other(ctx, reg, lane, p)
        if hit:
            raise Refuse(f"{p} is claimed by lane '{hit[0]}' ({hit[1]}{'!' if hit[2] else ''}); "
                         f"it cannot land until that claim is released")

    logp = ctx.state_dir / "logs" / f"{lane}-{time.strftime('%Y%m%d-%H%M%S')}.log"
    logp.parent.mkdir(parents=True, exist_ok=True)
    log = open(logp, "w", encoding="utf-8", newline="\n")
    acquire_lock(ctx, f"lanes land {lane}")
    land_ok = False
    try:
        check_main_idle(ctx)
        remote = ctx.cfg.get("remote")
        if remote and not a.no_push:
            git("fetch", "-q", remote, "main", cwd=ctx.main)
            rmain = f"{remote}/main"
            if not git_ok("merge-base", "--is-ancestor", rmain, "main", cwd=ctx.main):
                if git_ok("merge-base", "--is-ancestor", "main", rmain, cwd=ctx.main):
                    git("merge", "-q", "--ff-only", rmain, cwd=ctx.main)
                else:
                    raise Refuse(f"main and {rmain} have diverged; resolve by hand")
        base = git("rev-parse", "main", cwd=ctx.main)
        print(f"lanes: landing {len(commits)} commit(s) of {lane} onto main {base[:9]}", flush=True)

        git("checkout", "-q", "-B", lbr, base, cwd=wt)
        for sha in commits:
            how = replay_commit(ctx, wt, sha, log)
            print(f"lanes:   {sha[:9]} {how}", flush=True)
            write_lock(ctx, f"lanes land {lane}")  # heartbeat

        new_paths = set(git("diff", "--name-only", f"{base}..HEAD", cwd=wt).split())
        docs_only = all(any(p.startswith(d) for d in ctx.cfg.get("docs_only_paths", [])) for p in new_paths)
        kind = ctx.load()["lanes"][lane].get("kind", "foundation")
        gates = [] if docs_only else ctx.cfg["gates"].get(kind, ctx.cfg["gates"]["foundation"])
        if docs_only:
            print("lanes: docs-only change; build gates skipped")
        for g in gates:
            write_lock(ctx, f"lanes land {lane}")
            if run_shell(ctx, wt, g, log) != 0:
                raise Refuse(f"gate failed: {g}  (log {logp})", 4)
        if gates and ctx.cfg.get("oracle_sha1"):
            exe = wt / ctx.cfg["oracle_exe"]
            got = hashlib.sha1(exe.read_bytes()).hexdigest() if exe.exists() else "missing"
            if got != ctx.cfg["oracle_sha1"]:
                raise Refuse(f"{exe} SHA1 {got} != oracle {ctx.cfg['oracle_sha1']}", 4)
            print(f"lanes: SHA1 == oracle ({got[:12]})")

        if not docs_only:
            for g in ctx.cfg.get("regenerate", []):
                if run_shell(ctx, wt, g["cmd"], log) != 0:
                    raise Refuse(f"regenerate failed: {g['cmd']}", 4)
                git("add", "--", g["path"], cwd=wt)
                if not git_ok("diff", "--cached", "--quiet", "--", g["path"], cwd=wt):
                    msgfile = wt / "tmp" / "lanes" / "regen-msg.txt"
                    msgfile.parent.mkdir(parents=True, exist_ok=True)
                    msgfile.write_text(f"{g['subject']} after lane {lane}\n", encoding="utf-8", newline="\n")
                    git("commit", "-q", "-F", str(msgfile), cwd=wt)
                    print(f"lanes:   regenerated {g['path']}")
        if git("status", "--porcelain", "--untracked-files=no", cwd=wt):
            raise Refuse("the gates left tracked changes in the lane worktree: "
                         + git("status", "--porcelain", "--untracked-files=no", cwd=wt), 4)

        write_lock(ctx, f"lanes land {lane}")  # still ours?
        if git("rev-parse", "main", cwd=ctx.main) != base:
            raise Refuse("main moved during the land despite the lock; run land again")
        check_main_idle(ctx)  # again: the gates took minutes
        tip = git("rev-parse", "HEAD", cwd=wt)
        git("merge", "-q", "--ff-only", tip, cwd=ctx.main)
        land_ok = True
        if remote and not a.no_push:
            r = run(["git", "push", "-q", remote, "main"], cwd=ctx.main, check=False)
            if r.returncode != 0:
                raise Refuse(f"main -> {tip[:9]} locally, but the PUSH FAILED: "
                             f"{(r.stderr or '').strip()[-400:]}\n  Push main by hand "
                             f"(git -C <main> push {remote} main) before the next land.", 5)
            print(f"lanes: main -> {tip[:9]} (pushed)")
        else:
            print(f"lanes: main -> {tip[:9]} (not pushed: --no-push)")
    finally:
        # Always put the lane back on its branch; on success the branch becomes the new main.
        run(["git", "cherry-pick", "--abort"], cwd=wt, check=False)
        if land_ok:
            git("checkout", "-q", "-B", br, "main", cwd=wt)
        else:
            run(["git", "reset", "-q", "--hard"], cwd=wt, check=False)
            run(["git", "checkout", "-q", br], cwd=wt, check=False)
        run(["git", "branch", "-D", lbr], cwd=wt, check=False)
        release_lock(ctx)
        log.close()
    return 0


# ---------------------------------------------------------------- cli

def main(argv=None) -> int:
    p = argparse.ArgumentParser(prog="lanes.py", description=__doc__.split("\n")[0])
    p.add_argument("--session", default=os.environ.get("CLAUDE_CODE_SESSION_ID", ""))
    sub = p.add_subparsers(dest="cmd", required=True)
    s = sub.add_parser("status"); s.add_argument("--json", action="store_true")
    s = sub.add_parser("open"); s.add_argument("lane"); s.add_argument("--kind", choices=["naming", "foundation"])
    s.add_argument("--steal", action="store_true"); s.add_argument("--allow-multi", action="store_true")
    s = sub.add_parser("fresh"); s.add_argument("lane"); s.add_argument("--discard", action="store_true")
    s = sub.add_parser("claim"); s.add_argument("lane"); s.add_argument("tokens", nargs="+")
    s = sub.add_parser("unclaim"); s.add_argument("lane"); s.add_argument("tokens", nargs="*")
    s = sub.add_parser("land"); s.add_argument("lane"); s.add_argument("--no-push", action="store_true")
    s = sub.add_parser("close"); s.add_argument("lane"); s.add_argument("--remove", action="store_true")
    s.add_argument("--force", action="store_true")
    s = sub.add_parser("path"); s.add_argument("lane"); s.add_argument("--wsl", action="store_true")
    a = p.parse_args(argv)
    try:
        if a.cmd not in ("status", "path") and not a.session:
            raise Refuse("no session id: $CLAUDE_CODE_SESSION_ID is unset (pass --session)")
        ctx = Ctx(find_main(Path(__file__).resolve().parent), a.session)
        return {"status": cmd_status, "open": cmd_open, "fresh": cmd_fresh, "claim": cmd_claim,
                "unclaim": cmd_unclaim, "land": cmd_land, "close": cmd_close,
                "path": cmd_path}[a.cmd](ctx, a)
    except Refuse as e:
        print(f"lanes: REFUSED: {e}", file=sys.stderr)
        return e.code


if __name__ == "__main__":
    sys.exit(main())
