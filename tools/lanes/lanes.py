#!/usr/bin/env python3
"""lanes.py: parallel work lanes for BB2, one agent session per git worktree.

A lane is a named worktree (`../bb2-worktrees/ln-<lane>`, branch `lanes/<lane>`) owned by exactly
one Claude session. Lanes never edit the main checkout. Main is only a landing pad: `land`
replays the lane's commits onto the current main in a private landing worktree, runs the gates
there, and fast-forwards main to the gated commit under the main-reintegration lock
(tools/reintegrate_lock.ps1, same lock file), so main never holds an ungated state. If main
moves while the gates run, the lane is replayed onto the new main; when the paths main changed
since the gated base are disjoint from the lane's and at most one side touched build inputs
(needs_full_gates), that replay gets only the quick checks (lanes.json "quick_gates", then
regenerate), never when build inputs changed on both sides.

Full lands wait FIFO in tmp/lanes-state/registry.json's gate_queue before reading main.
Docs-only lands (lanes.json docs_only_paths) bypass it; quick retries keep the existing ticket.
Scoped tip_checks run once after regeneration and input_checks, including docs-only and quick lands.
With the gate mirror, regenerate and both checks run there after the gates (tools/lanes/
mirror_post.py), also for a docs-only land that has any of them; the landing worktree applies
only the regenerated files and must reach the mirror's tree for every landed commit.
Owners heartbeat while waiting and landing; waiters prune expired heartbeats across PID
namespaces, as well as provably dead local PIDs.

Conflicts are handled without hand merges where the file allows it:
  - regenerated files (lanes.json "regenerate", e.g. the naming census) take main's side and are
    rebuilt after the gates, folded into the lane's last landed commit (no separate commit);
    entries with "inputs" run only when the lane changes an input (including docs-only lands);
  - row-keyed TSV registries (lanes.json "rowmerge", the backlogs) are merged row by row;
  - a commit whose message carries `Lane-Replay: <shell command>` trailers is re-run on the new
    base instead of cherry-picked (use it only when the commit is exactly that tool output);
  - anything else bounces back to the lane, untouched, with the conflicting files listed.

Claims keep lanes off each other's work: `path:<glob>` (edit and land exclusivity; a shared path
can only be claimed with a trailing `!`, which freezes it for every other lane until released)
and `topic:<text>` (work selection, e.g. `topic:row:m09`, `topic:tu:main/51268`). Topics are
exclusive except: `topic:tuedit:<tu>` (a naming wave that renames symbols inside a TU) is shared
with other `tuedit` claims and excludes only `topic:tu:<tu>` (a wave that moves or wholly owns
it); `topic:sym:<key>` (a symbol a wave renames, or a name it introduces, keyed as in its
pairs.txt: `name`, `func:local`, `Type.member`) also excludes the claim on its scope (`func`,
`Type`). `claim --wave <draft dir>` derives a wave's claims (topics_overlap, wave_claims). Two
waves editing one TU then land in sequence through the replay; `land` re-runs
tools/naming_keycheck.py on each naming wave replayed onto a moved main (replay_keychecks) and
bounces the later one if it no longer explains its own diff. The registry lives
in main's `tmp/lanes-state/registry.json` (untracked, so it can never conflict).
tools/hooks/lane_guard.py enforces ownership and claims on Edit/Write/Bash/PowerShell.

Usage (Windows Python, from any cwd):
  python tools/lanes/lanes.py status [--json]
  python tools/lanes/lanes.py open  <lane> [--kind naming|foundation] [--steal]
  python tools/lanes/lanes.py fresh <lane> [--discard]     # --discard backs the tip up first
  python tools/lanes/lanes.py drop  <lane> <sha>            # remove one unlanded commit (a bounce)
  python tools/lanes/lanes.py claim <lane> <token>...       # path:src/main/9F9C.c  topic:row:m09
  python tools/lanes/lanes.py claim <lane> --wave <draft>   # + a naming draft's sym/tu claims
  python tools/lanes/lanes.py unclaim <lane> [<token>...]   # no tokens = all
  python tools/lanes/lanes.py land  <lane> [--no-push] [--no-wait]
  python tools/lanes/lanes.py close <lane> [--remove]
  python tools/lanes/lanes.py path  <lane> [--wsl]
  python tools/lanes/lanes.py recover <lane>                # after an interrupted land
  python tools/lanes/lanes.py push                          # retry a failed push (exit 5)
  python tools/lanes/lanes.py gate-mirror [--reset]         # create / re-clone the WSL gate copy

The session identity is $CLAUDE_CODE_SESSION_ID (override with --session for tests).
Exit codes: 0 ok, 1 refused, 3 land bounced on a conflict, 4 a gate failed, 5 push failed,
6 full-gate queue busy (--no-wait).
"""
from __future__ import annotations

import argparse
import fnmatch
import hashlib
import json
import math
import os
import re
import shlex
import shutil
import subprocess
import sys
import time
import threading
import uuid
from pathlib import Path

LANE_RE = re.compile(r"^[a-z0-9][a-z0-9-]{0,30}$")
TRAILER_RE = re.compile(r"^Lane-Replay:\s*(.+?)\s*$", re.MULTILINE)
LOCK_REL = Path("tmp") / ".main_reintegration.lock"
LOCK_STALE_SECS = 5400  # keep in sync with tools/reintegrate_lock.ps1
LOCK_FUTURE_SLACK_SECS = 300  # clock skew slack; keep in sync with reintegrate_lock.ps1
HEARTBEAT_STALE_SECS = 600  # Expire abandoned gate tickets across Windows/WSL PID namespaces.
HEARTBEAT_INTERVAL_SECS = 30
GATE_POLL_SECS = 5  # a waiter's poll period (reads only; writes only on a queue change)


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
        for attempt in range(5):
            try:
                reg = json.loads(self.reg_path.read_text(encoding="utf-8"))
                break
            except FileNotFoundError:
                reg = {}
                break
            except (PermissionError, json.JSONDecodeError):
                if attempt == 4:
                    raise
                time.sleep(0.05)
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
            deadline = None
            while True:
                try:
                    os.replace(tmp, self.reg_path)
                    break
                except PermissionError:
                    now = time.monotonic()
                    if deadline is None:
                        deadline = now + 5
                    remaining = deadline - now
                    if remaining <= 0:
                        raise
                    # Keep the mutex and the same staged write throughout the retry.
                    time.sleep(min(0.05, remaining))
            return out
        finally:
            try:
                self.mutex.unlink()
            except FileNotFoundError:
                pass


def is_stale(ctx: Ctx, entry: dict) -> bool:
    return time.time() - float(entry.get("heartbeat", 0)) > ctx.stale_secs()


class GateQueue:
    """FIFO tickets in registry.json, protected by Ctx.update's O_EXCL file mutex.

    Only the short queue edits hold the mutex; the head retains its ticket through landing.
    A daemon renews the ticket while waiting and throughout long gate subprocesses.
    Heartbeat expiry works across Windows/WSL namespaces and handles PID reuse.
    """
    def __init__(self, ctx, lane):
        self.ctx = ctx
        self.ticket = {"id": uuid.uuid4().hex, "lane": lane, "session": ctx.session,
                       "pid": os.getpid(), "enqueued": time.time(), "beat": time.time(),
                       "namespace": pid_namespace()}
        self.enqueued = False
        self._stop = threading.Event()
        self._thread = None

    def _refresh(self):
        def refresh(reg):
            for ticket in reg.get("gate_queue", []):
                if ticket["id"] == self.ticket["id"]:
                    ticket["beat"] = time.time()
                    return ticket["beat"]
        beat = self.ctx.update(refresh)
        if beat is not None:
            self.ticket["beat"] = beat

    def _heartbeat(self):
        warned = False
        while not self._stop.wait(HEARTBEAT_INTERVAL_SECS):
            try:
                self._refresh()
                warned = False
            except Exception as error:
                # Keep beating after any refresh failure; never recreate an evicted ticket.
                if not warned:
                    print(f"lanes: gate heartbeat failed for {self.ticket['lane']}: {error}",
                          file=sys.stderr, flush=True)
                    warned = True

    @staticmethod
    def _stale(ticket, now):
        age = now - ticket.get("beat", ticket["enqueued"])
        return age > HEARTBEAT_STALE_SECS or (ticket.get("namespace") == pid_namespace()
                                              and not gate_pid_alive(ticket["pid"]))

    def _enqueue_and_prune(self, reg):
        queue = reg.setdefault("gate_queue", [])
        now = time.time()
        for ticket in list(queue):
            if ticket["id"] != self.ticket["id"] and self._stale(ticket, now):
                queue.remove(ticket)
                age = now - ticket.get("beat", ticket["enqueued"])
                print(f"lanes: dropped stale gate ticket for {ticket['lane']} "
                      f"session={ticket['session']} (beat age {int(age)}s)", flush=True)
        if not any(t["id"] == self.ticket["id"] for t in queue):
            self.ticket["beat"] = time.time()
            queue.append(self.ticket)
        return next(i for i, t in enumerate(queue, 1) if t["id"] == self.ticket["id"])

    def _poll(self):
        """This ticket's queue position. Reads without the mutex; writes (under it) only to
        enqueue, re-enqueue an evicted ticket, or drop a stale one, so waiters do not rewrite
        registry.json every poll (on Windows a replace fails while a reader holds the file)."""
        queue = self.ctx.load().get("gate_queue", [])
        now = time.time()
        mine = any(t["id"] == self.ticket["id"] for t in queue)
        if mine and not any(t["id"] != self.ticket["id"] and self._stale(t, now) for t in queue):
            return next(i for i, t in enumerate(queue, 1) if t["id"] == self.ticket["id"])
        # Set before update: even interruption just after its atomic replace must clean up.
        self.enqueued = True
        return self.ctx.update(self._enqueue_and_prune)

    def wait(self, no_wait=False):
        last_print = last_error = None
        while True:
            try:
                position = self._poll()
            except (OSError, ValueError, Refuse) as error:
                # transient: a replace racing a reader (Windows), a half-read file, a busy
                # mutex; keep the place in the queue and poll again
                now = time.monotonic()
                if last_error is None or now - last_error >= 60:
                    print(f"lanes: gate queue poll failed ({error}); retrying", flush=True)
                    last_error = now
                time.sleep(GATE_POLL_SECS)
                continue
            if self._thread is None:
                self._stop.clear()
                self._thread = threading.Thread(target=self._heartbeat, daemon=True,
                                                name=f"lane-beat-{self.ticket['lane']}")
                self._thread.start()
            if position == 1:
                return
            if no_wait:
                raise Refuse(f"full-gate queue busy: {self.ticket['lane']} is position {position}", 6)
            now = time.monotonic()
            if last_print is None or now - last_print >= 60:
                print(f"lanes: waiting for full-gate queue: position {position} "
                      f"({self.ticket['lane']})", flush=True)
                last_print = now
            time.sleep(GATE_POLL_SECS)

    def close(self):
        """Stop the heartbeat and dequeue. Never raises: a ticket left behind after the retries
        expires by heartbeat age (or its dead PID)."""
        self._stop.set()
        if self._thread is not None:
            self._thread.join()
            self._thread = None
        if not self.enqueued:
            return
        def remove(reg):
            reg["gate_queue"] = [t for t in reg.get("gate_queue", [])
                                 if t["id"] != self.ticket["id"]]
        last = None
        for attempt in range(5):
            try:
                self.ctx.update(remove)
                self.enqueued = False
                return
            except (OSError, ValueError, Refuse) as error:
                last = error
                time.sleep(min(2 ** attempt, 8))
        print(f"lanes: could not dequeue gate ticket for {self.ticket['lane']} ({last}); it "
              f"expires after {HEARTBEAT_STALE_SECS}s", file=sys.stderr, flush=True)


def pid_namespace():
    if os.name == "nt":
        return "windows"
    try:
        return Path("/proc/sys/kernel/random/boot_id").read_text().strip() + ":" + os.readlink("/proc/self/ns/pid")
    except OSError:
        return sys.platform


def gate_pid_alive(pid):
    """Unlike recovery's historical probe, treat inaccessible processes as live."""
    if os.name == "nt":
        import ctypes
        from ctypes import wintypes
        kernel = ctypes.WinDLL("kernel32", use_last_error=True)
        kernel.OpenProcess.argtypes = [wintypes.DWORD, wintypes.BOOL, wintypes.DWORD]
        kernel.OpenProcess.restype = wintypes.HANDLE
        kernel.GetExitCodeProcess.argtypes = [wintypes.HANDLE, ctypes.POINTER(wintypes.DWORD)]
        kernel.CloseHandle.argtypes = [wintypes.HANDLE]
        handle = kernel.OpenProcess(0x1000, False, pid)
        if not handle:
            return ctypes.get_last_error() != 87  # only INVALID_PARAMETER proves no such PID
        try:
            code = wintypes.DWORD()
            return not kernel.GetExitCodeProcess(handle, ctypes.byref(code)) or code.value == 259
        finally:
            kernel.CloseHandle(handle)
    try:
        os.kill(pid, 0)
        return True
    except ProcessLookupError:
        return False
    except OSError:
        return True  # permission/probe failures do not prove death


def is_docs_only(ctx, paths) -> bool:
    """Every path under lanes.json docs_only_paths: build_candidate runs no full gates."""
    prefixes = ctx.cfg.get("docs_only_paths", [])
    return all(under_scope(p, prefixes) for p in paths)


def queues_full_gates(ctx, paths):
    # Queue exactly the lands build_candidate gates in full (one shared predicate, so the two
    # cannot drift): an inert_paths land outside docs_only_paths (tools/trace/, .claude/) still
    # runs the full gates, and the queue head landing src/ would invalidate it.
    return not is_docs_only(ctx, paths)


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


def sym_scope(key: str):
    """The scope a scoped pairs.txt key renames inside (`f:l` -> `f`, `T.u.m` -> `T`), else None."""
    for sep in (":", "."):
        if sep in key:
            return key.split(sep, 1)[0]
    return None


def topics_overlap(a: str, b: str) -> bool:
    """Whether two topic claims (values after `topic:`) exclude each other. Equal topics do,
    except `tuedit:<tu>` (shared by waves editing one TU); `tuedit:X` excludes `tu:X`; a scoped
    `sym:f:l` / `sym:T.m` also excludes `sym:f` / `sym:T` (renaming the scope itself)."""
    ka, _, va = a.partition(":")
    kb, _, vb = b.partition(":")
    if {ka, kb} == {"tu", "tuedit"}:
        return va == vb
    if a == b:
        return ka != "tuedit"
    if ka == kb == "sym":
        return sym_scope(va) == vb or sym_scope(vb) == va
    return False


def claims_overlap(kind: str, a: str, b: str) -> bool:
    return topics_overlap(a, b) if kind == "topic" else globs_overlap(a, b)


PAIRS_FILE = "pairs.txt"
_IDENT = r"[A-Za-z_]\w*"
SYM_KEY = re.compile(rf"{_IDENT}(?:[.:]{_IDENT})*")


def wave_sym_claims(pairs_text: str) -> set[str]:
    """`topic:sym:` claims for a wave's pairs.txt (`old=new` per line or comma, `#` comments,
    as naming_keycheck.parse_pairs reads it): each old key, and each new name in the old key's
    scope (two waves may not introduce one name there)."""
    out = set()
    for item in re.split(r"[,\n]", pairs_text):
        item = item.strip()
        if not item or item.startswith("#"):
            continue
        old, sep, new = (x.strip() for x in item.partition("="))
        if not (sep and SYM_KEY.fullmatch(old) and re.fullmatch(_IDENT, new)):
            raise Refuse(f"pairs.txt: bad pair {item!r} (want old=new)")
        scope = old[:max(old.rfind(":"), old.rfind(".")) + 1]  # `f:`, `T.u.`, or empty
        out.update((f"topic:sym:{old}", f"topic:sym:{scope}{new}"))
    return out


# A naming draft's TUs (the orchestrator's dispatch check and `claim --wave`).
DRAFT_TU = re.compile(r"\bsrc/(main/[0-9A-Za-z_/]+)\.c\b")  # TU id = path under src/ w/o .c
DRY_EDITS = re.compile(r"^-- file edits \(\d+\) --$(.*?)(?=^--|\Z)", re.M | re.S)
DRAFT_DECL = re.compile(r"\bDECL\s+(?:\([^)]*\)\s+)?(?:src/main/)?([0-9A-Za-z_]+)\.c:\d")
DRY_COUNTS = re.compile(r"(?<![/\w])([0-9A-Za-z_]+)\.c \d+\b")
DRAFT_BARE = re.compile(r"(?<![/\w])([0-9A-Za-z_]+)\.c\b")


def tu_basenames(main: Path) -> dict[str, str]:
    """`4510C` -> `main/4510C` for every game/library TU whose file name is unique under src/."""
    seen: dict[str, list[str]] = {}
    for p in (main / "src").rglob("*.c") if (main / "src").is_dir() else ():
        tu = p.relative_to(main / "src").with_suffix("").as_posix()
        seen.setdefault(p.stem, []).append(tu)
    return {b: ts[0] for b, ts in seen.items() if len(ts) == 1}


def draft_tus(main: Path, draft: str) -> set[str]:
    """The game TUs a naming draft dir (relative to `main`) edits: dry.txt's edit list, else a
    locals manifest's DECL sites or the dry run's per-file counts, else every TU the draft
    names. Empty when there is no draft."""
    d = (draft or "").strip()
    if not d or d == "-":
        return set()
    root = main / d
    if not root.is_dir():
        return set()
    try:  # the dry run's edit list is exact; prose elsewhere cites TUs the wave never edits
        m = DRY_EDITS.search((root / "dry.txt").read_text(encoding="utf-8", errors="replace"))
    except OSError:
        m = None
    if m and DRAFT_TU.search(m.group(1).replace("\\", "/")):
        return set(DRAFT_TU.findall(m.group(1).replace("\\", "/")))
    texts = []  # no dry-run edit list (locals waves): the TUs the draft cites
    for f in sorted(root.iterdir()):
        if f.is_file() and f.suffix in (".txt", ".csv", ".tsv", ".py"):
            try:
                texts.append(f.read_text(encoding="utf-8", errors="replace").replace("\\", "/"))
            except OSError:
                pass
    text = "\n".join(texts)
    bare = tu_basenames(main)
    # A locals manifest cites each binding's definition as `DECL 4510C.c:1808`: the TU the pair
    # edits. Without one, every TU the draft names (src/ paths and bare `4510C.c` file names).
    decl = {bare[b] for b in DRAFT_DECL.findall(text) if b in bare}
    try:  # a skipped locals dry run still lists its per-file counts: `4510C.c 46, 17AFC.c 1`
        dry = (root / "dry.txt").read_text(encoding="utf-8", errors="replace")
    except OSError:
        dry = ""
    decl |= {bare[b] for b in DRY_COUNTS.findall(dry) if b in bare}
    if decl:
        return decl
    out = set(DRAFT_TU.findall(text))
    out.update(bare[b] for b in DRAFT_BARE.findall(text) if b in bare)
    return out


def wave_claims(main: Path, draft: str) -> set[str]:
    """The claims a naming draft's LAND takes. With a pairs.txt (locals, globals, members):
    its `topic:sym:` claims plus a shared `topic:tuedit:` per TU it edits. Without one
    (file moves, function waves): an exclusive `topic:tu:` per TU, as before."""
    d = (draft or "").strip()
    tus = draft_tus(main, d)
    if d not in ("", "-") and (main / d / PAIRS_FILE).is_file():
        syms = wave_sym_claims((main / d / PAIRS_FILE).read_text(encoding="utf-8",
                                                                 errors="replace"))
        if syms:
            return syms | {f"topic:tuedit:{t}" for t in tus}
    return {f"topic:tu:{t}" for t in tus}


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
        lk = json.loads((ctx.main / LOCK_REL).read_text(encoding="utf-8-sig"))
        return lk if isinstance(lk, dict) else {}
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


def lock_age(lk: dict) -> float:
    """Seconds since ts; unreadable or implausibly future timestamps are stale."""
    raw_ts = lk.get("ts")
    if isinstance(raw_ts, bool):
        return math.inf
    try:
        ts = float(raw_ts)
    except (TypeError, ValueError, OverflowError):
        return math.inf
    now = time.time()
    if not math.isfinite(ts) or ts > now + LOCK_FUTURE_SLACK_SECS:
        return math.inf
    return now - ts


def acquire_lock(ctx: Ctx, label: str):
    deadline = time.time() + 60 * float(ctx.cfg.get("lock_wait_minutes", 45))
    announced = False
    while True:
        if _create_lock(ctx, label):
            return
        lk = read_lock(ctx)
        age = lock_age(lk) if lk is not None else 0
        if lk and lk.get("session") == ctx.session:
            write_lock(ctx, label)
            return
        if lk is None or age > LOCK_STALE_SECS:
            # unreadable (mid-write) or stale: re-check after a pause, then steal by unlink +
            # O_EXCL create, so two stealers cannot both win.
            time.sleep(1)
            lk2 = read_lock(ctx)
            if lk2 == lk or (lk2 is not None and lock_age(lk2) > LOCK_STALE_SECS):
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

# Environment the WSL runner passes on: `wsl bash` starts without the caller's variables
# (no WSLENV), so a capture store chosen on the Windows side would otherwise be ignored by a
# land's gates, which then read the default store. tools/trace/store.py turns a drive path into
# its /mnt form itself.
WSL_FORWARD_ENV = ("BB2_CAPTURE_STORE", "BB2_CAPTURE_STORE_WRITE")


def wsl_env_exports(env=None) -> str:
    """`export NAME=value` lines for the set, non-empty WSL_FORWARD_ENV variables."""
    env = os.environ if env is None else env
    return "".join(f"export {name}={shlex.quote(env[name])}\n"
                   for name in WSL_FORWARD_ENV if env.get(name))


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
                          + wsl_env_exports()
                          + f"cd '{wsl_path(wt)}'\n"
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


# ---------------------------------------------------------------- gate mirror
#
# lanes.json `gate_mirror`: the gates run in a throwaway clone on the WSL ext4 disk
# ($HOME/<path>), fetched read-only from the Windows repo for the exact commit being landed.
# WSL never writes the Windows repo and the clone holds nothing that exists only there
# (`lanes.py gate-mirror --reset` deletes and re-creates it; a missing clone is created on the
# next land). Its untracked toolchain (`copy`) is copied from main once, at creation. One flock
# serialises every use, so two lands never share a checkout.

# Markers live inside .git (never a worktree file, never cleaned). OWNED is written right after
# the clone: --reset (and a land that finds a half-made clone) may delete only a directory
# holding it. DONE is written last: without it the clone is incomplete and is re-made.
MIRROR_OWNED = ".git/lanes-gate-mirror-owned"
MIRROR_DONE = ".git/lanes-gate-mirror"
GATE_LINE = re.compile(r"^@@gate (\d+) rc=(\d+) secs=(\d+)$", re.M)
SHA1_LINE = re.compile(r"^@@sha1 ([0-9a-f]{40}|missing)$", re.M)
OUT_LINE = re.compile(r"^@@out (.+?)\s*$", re.M)
MIRROR_TIMEOUT_SECS = 3 * 3600
# Build outputs the landing worktree's regenerate / checks read (build_census.py: bb2.map),
# copied back from the mirror after the gates.
MIRROR_ARTIFACTS_DEFAULT = ["build/bb2.exe", "build/bb2.map", "build/bb2.elf"]
# The post-gate steps (regenerate, input / tip checks) the mirror runs after the gates.
MIRROR_POST = Path(__file__).with_name("mirror_post.py")
POST_LINE = re.compile(r"^@@post (\w+) (\d+) rc=(\d+) secs=(\d+)$", re.M)
POST_OUT_LINE = re.compile(r"^@@post-out (.+?)\s*$", re.M)


def mirror_cfg(ctx: Ctx) -> dict | None:
    m = ctx.cfg.get("gate_mirror")
    if not m or ctx.cfg.get("runner") == "local" or os.environ.get("BB2_LANES_NO_MIRROR"):
        return None
    return m


def _safe_rel(p: str) -> bool:
    return bool(re.fullmatch(r"[\w./-]+", p)) and ".." not in p and not p.startswith("/")


def _mirror_prelude(ctx: Ctx, m: dict) -> str:
    name = m.get("path", "bb2-gate")
    if not re.fullmatch(r"[\w.-]+", name) or name in (".", ".."):
        raise Refuse(f"gate_mirror.path must be one directory name under $HOME, not {name!r}")
    return ("set -u\n"
            f'M="$HOME/{name}"\n'
            f"SRC={shlex.quote(wsl_path(ctx.main))}\n"
            'exec 9>"$M.lock"\nflock 9\n')


def mirror_create_script(ctx: Ctx, m: dict, reset: bool) -> str:
    """Bash that (re)creates the clone: delete only a directory this tool made (OWNED marker),
    clone main read-only, copy the untracked toolchain, make the venv, then mark it DONE."""
    copies = "".join(
        f'if [ -e "$SRC/{p}" ] && [ ! -e "$M/{p}" ]; then mkdir -p "$(dirname "$M/{p}")" && '
        f'cp -a "$SRC/{p}" "$M/{p}" || exit 93; fi\n'
        f'echo "/{p}" >> .git/info/exclude\n'
        for p in m.get("copy", []) if _safe_rel(p))
    s = _mirror_prelude(ctx, m)
    gone = ("[ -f \"$M/" + MIRROR_DONE + "\" ] || " if not reset else "")
    s += (f'if [ -e "$M" ]; then {gone}{{\n'
          f'  [ -f "$M/{MIRROR_OWNED}" ] || {{ echo "lanes-mirror: $M exists but was not made by '
          f'lanes.py (no {MIRROR_OWNED}); not deleting it"; exit 94; }}\n'
          '  echo "lanes-mirror: removing $M"\n'
          '  rm -rf -- "$M" || exit 94\n'
          '}\nfi\n')
    s += ('if [ ! -e "$M" ]; then\n'
          '  git -c safe.directory="*" clone -q --no-checkout "$SRC" "$M" || exit 93\n'
          f'  date > "$M/{MIRROR_OWNED}"\n'
          '  cd "$M" || exit 93\n'
          '  git remote set-url --push origin no-push-from-the-gate-mirror || exit 93\n'
          '  git checkout -q --detach main || exit 93\n'
          + "".join("  " + ln + "\n" for ln in copies.splitlines()) +
          '  python3 -m venv .venv || exit 93\n'
          '  .venv/bin/pip install -q -r requirements.txt || exit 93\n'
          '  cp requirements.txt .git/lanes-requirements.txt || exit 93\n'
          f'  date > "$M/{MIRROR_DONE}"\n'
          '  echo "lanes-mirror: created $M"\n'
          'fi\n')
    return s


def mirror_gate_script(ctx: Ctx, m: dict, ref: str, sha: str, gates, oracle_exe: str,
                       lane: str = "", post: tuple[str, str] | None = None) -> str:
    """Bash: (re)create the clone if missing or incomplete, fetch `ref` from main, check out
    exactly `sha`, run every gate in order (stop at the first failure), print the built exe's
    SHA1, and stage the build outputs for the landing worktree under $M.out/<lane> (none when
    no gate ran: the mirror's build/ is then an older land's). `post` = (WSL path of
    mirror_post.py, of its plan): after the gates, regenerate and the input / tip checks run
    there with their outputs under $M.post/<lane> (exit 101 when one refuses)."""
    if not re.fullmatch(r"[\w.-]*", lane):
        raise Refuse(f"lane name {lane!r} is not safe for the gate mirror")
    arts = [a for a in m.get("artifacts", MIRROR_ARTIFACTS_DEFAULT) if _safe_rel(a)] \
        if gates else []
    s = mirror_create_script(ctx, m, reset=False)
    s += ('cd "$M" || exit 91\n'
          f'git -c safe.directory="*" fetch -q --no-tags "$SRC" "+{ref}:refs/gate/candidate" '
          '|| exit 91\n'
          f'git checkout -q -f --detach {sha} || exit 91\n'
          'git clean -fdq || exit 91\n'  # untracked, not ignored (build/, .venv stay)
          f'[ "$(git rev-parse HEAD)" = "{sha}" ] || {{ echo "lanes-mirror: HEAD is not {sha}"; '
          'exit 92; }\n'
          'if ! cmp -s requirements.txt .git/lanes-requirements.txt; then\n'
          '  .venv/bin/pip install -q -r requirements.txt || exit 91\n'
          '  cp requirements.txt .git/lanes-requirements.txt\n'
          'fi\n'
          + wsl_env_exports() +
          '. .venv/bin/activate || exit 91\n')
    for i, g in enumerate(gates):
        s += (f'echo "@@gate {i} start"\nT0=$(date +%s)\n( {g} )\nRC=$?\n'
              f'echo "@@gate {i} rc=$RC secs=$(( $(date +%s) - T0 ))"\n'
              f'[ "$RC" = 0 ] || exit 100\n')
    s += (f'if [ -f {shlex.quote(oracle_exe)} ]; then echo "@@sha1 $(sha1sum {shlex.quote(oracle_exe)}'
          ' | cut -c1-40)"; else echo "@@sha1 missing"; fi\n'
          f'O="$M.out/{lane or "_"}"\n'
          'rm -rf -- "$O" && mkdir -p "$O" || exit 95\n'
          + "".join(f'if [ -f {shlex.quote(a)} ]; then mkdir -p "$O/$(dirname {shlex.quote(a)})" '
                    f'&& cp {shlex.quote(a)} "$O/{a}" || exit 95; fi\n' for a in arts) +
          'echo "@@out $(wslpath -w "$O")"\n')
    if post:
        # the capture store sits beside main (tools/trace/store.py store_root); read it there
        store = wsl_path(ctx.main.parent / "bb2-captures")
        s += (f'P="$M.post/{lane or "_"}"\n'
              'rm -rf -- "$P" && mkdir -p "$P" || exit 95\n'
              f'[ -n "${{BB2_CAPTURE_STORE:-}}" ] || export BB2_CAPTURE_STORE={shlex.quote(store)}\n'
              f'python3 {shlex.quote(post[0])} {shlex.quote(post[1])} "$P"\n'
              'RC=$?\n'
              'echo "@@post-out $(wslpath -w "$P")"\n'
              '[ "$RC" = 0 ] || exit 101\n')
    return s


def parse_mirror_output(out: str, gates) -> tuple[list[tuple[str, int, int]], str | None]:
    """[(gate, rc, secs)] for every gate that ran, and the exe SHA1 line (None if absent)."""
    ran = [(gates[int(i)], int(rc), int(secs)) for i, rc, secs in GATE_LINE.findall(out)
           if int(i) < len(gates)]
    m = SHA1_LINE.search(out)
    return ran, (m.group(1) if m else None)


def run_wsl_script(ctx: Ctx, name: str, text: str) -> subprocess.CompletedProcess:
    d = ctx.state_dir / "gate-mirror"
    d.mkdir(parents=True, exist_ok=True)
    script = d / f"{name}.sh"
    script.write_text(text, encoding="utf-8", newline="\n")
    try:
        return subprocess.run(["wsl", "bash", wsl_path(script)], capture_output=True, text=True,
                              encoding="utf-8", errors="replace", timeout=MIRROR_TIMEOUT_SECS)
    except FileNotFoundError:
        raise Refuse("`wsl` is not available: the gate mirror needs WSL "
                     "(BB2_LANES_NO_MIRROR=1 gates in the landing worktree)", 4)
    except subprocess.TimeoutExpired:
        raise Refuse(f"the gate mirror script ran over {MIRROR_TIMEOUT_SECS // 3600} h; "
                     f"killed (log in {d})", 4)


def run_gates_mirror(ctx: Ctx, lane: str, ld: Path, gates, log, post=None) -> str | None:
    """Run `gates` in the mirror on ld's HEAD; returns the built exe's SHA1 (None when no SHA1
    line was printed). Raises Refuse(4) on a failed gate or a mirror error. `post`
    (mirror_post_plan) also runs the post-gate steps there; their result lands in
    ctx.mirror_post for apply_mirror_post."""
    m = mirror_cfg(ctx)
    sha = git("rev-parse", "HEAD", cwd=ld)
    ref = f"refs/lanes-gate/{lane}"
    ctx.mirror_post = None
    post_args = None
    if post is not None:
        d = ctx.state_dir / "gate-mirror"
        d.mkdir(parents=True, exist_ok=True)
        plan, prog = d / f"post-{lane}.json", d / f"post-{lane}.py"
        plan.write_text(json.dumps(dict(post, gated=sha), indent=1) + "\n", encoding="utf-8",
                        newline="\n")
        prog.write_bytes(MIRROR_POST.read_bytes())  # this checkout's copy, not the candidate's
        post_args = (wsl_path(prog), wsl_path(plan))
    git("update-ref", ref, sha, cwd=ctx.main)  # the Windows side's only write
    print(f"lanes: gates in the WSL mirror ~/{m.get('path', 'bb2-gate')} at {sha[:9]}"
          + (" (then regenerate and checks there)" if post is not None else ""), flush=True)
    try:
        r = run_wsl_script(ctx, f"gate-{lane}",
                           mirror_gate_script(ctx, m, ref, sha, gates, ctx.cfg["oracle_exe"],
                                              lane, post=post_args))
    finally:
        git("update-ref", "-d", ref, cwd=ctx.main, check=False)
    logname = getattr(log, "name", "?")
    out = (r.stdout or "") + (r.stderr or "")
    log.write(f"\n$ [gate mirror {sha}]\n{out}\n[exit {r.returncode}]\n")
    ran, sha1 = parse_mirror_output(out, gates)
    for g, rc, secs in ran:
        print(f"lanes:   $ {g}  ({secs}s{'' if rc == 0 else f', exit {rc}'})", flush=True)
        log.write(f"[gate {g}: {secs}s]\n")
    for kind, i, rc, secs in POST_LINE.findall(out):
        print(f"lanes:   mirror {kind} {i}: {secs}s{'' if rc == '0' else f', exit {rc}'}",
              flush=True)
    log.flush()
    if post is not None and r.returncode in (0, 101) and len(ran) == len(gates):
        res = read_mirror_post(out)
        if res.get("failed"):
            raise Refuse(mirror_post_refusal(res["failed"], logname), 4)
        if r.returncode == 0:
            ctx.mirror_post = res
    if r.returncode != 0:
        tail = "\n".join("lanes:     " + t for t in out.strip().splitlines()[-8:])
        if tail:
            print(tail, flush=True)
        failed = next((g for g, rc, _ in ran if rc != 0), None)
        raise Refuse(f"gate failed: {failed}  (log {logname})" if failed else
                     f"gate mirror error (exit {r.returncode}; log {logname}); "
                     f"`lanes.py gate-mirror --reset` rebuilds it", 4)
    if len(ran) != len(gates):
        raise Refuse(f"gate mirror ran {len(ran)} of {len(gates)} gates (log {logname})", 4)
    copy_mirror_artifacts(out, ld)
    ctx.mirror_gated = sha
    return sha1


def copy_mirror_artifacts(out: str, ld: Path):
    """Windows side: copy the build outputs the mirror staged (`@@out <\\wsl path>`) into the
    landing worktree's build/, so regenerate (the census reads build/bb2.map) and the checks
    there see the tree that was gated, never an older land's build."""
    m = OUT_LINE.search(out)
    if not m:
        raise Refuse("the gate mirror staged no build outputs (no @@out line)", 4)
    src = Path(m.group(1))
    if not src.is_dir():
        raise Refuse(f"the gate mirror's build outputs are not readable at {src}", 4)
    for f in src.rglob("*"):
        if f.is_file():
            dst = ld / f.relative_to(src)
            dst.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(f, dst)


def mirror_post_plan(ctx: Ctx, ld: Path, base: str, docs_only: bool) -> dict:
    """What mirror_post.py runs after the gates: regenerate()'s entries for this candidate and
    the input / tip checks (it scopes those itself, per commit, like input_checks/tip_checks)."""
    return {"base": base,
            "regenerate": [{"path": g["path"], "cmd": g["cmd"]}
                           for g in regen_entries(ctx, docs_only)],
            "input_checks": ctx.cfg.get("input_checks", []),
            "tip_checks": ctx.cfg.get("tip_checks", []),
            "cand_paths": sorted(ctx.cand_paths)}


def mirror_post_needed(ctx: Ctx, ld: Path, post: dict) -> bool:
    """Whether a gate-less (docs-only) land has post-gate work, so must go through the mirror:
    a regenerate entry, or a check whose inputs a commit, the candidate or an output touches
    (an upper bound of what mirror_post.py will run)."""
    if post["regenerate"]:
        return True  # so no output changes below: only the commits' and candidate's paths
    if any(any(matches_input_prefix(p, c["inputs"]) for p in post["cand_paths"])
           for c in post["tip_checks"]):
        return True
    if not post["input_checks"]:
        return False
    paths = set()
    for c in git("rev-list", "--reverse", f"{post['base']}..HEAD", cwd=ld).split():
        paths |= set(git("diff-tree", "--no-commit-id", "--name-only", "-z", "-r",
                         "--no-renames", c, cwd=ld).split("\0")) - {""}
    return any(any(matches_input_prefix(p, c["inputs"]) for p in paths)
               for c in post["input_checks"])


def read_mirror_post(out: str) -> dict:
    """mirror_post.py's post.json and regen.patch (`@@post-out <\\wsl path>`)."""
    m = POST_OUT_LINE.search(out)
    if not m:
        raise Refuse("the gate mirror ran no post-gate steps (no @@post-out line)", 4)
    src = Path(m.group(1))
    try:
        res = json.loads((src / "post.json").read_text(encoding="utf-8"))
        res["patch"] = (src / "regen.patch").read_bytes() if res.get("changed") else b""
    except (OSError, ValueError) as e:
        raise Refuse(f"the gate mirror's post-gate result is not readable at {src}: {e}", 4)
    return res


def mirror_post_refusal(failed: dict, logname: str) -> str:
    """The refusal regenerate() / input_checks() / tip_checks() would have raised."""
    kind, cmd = failed.get("kind"), failed.get("cmd", "")
    return {"regenerate": f"regenerate failed: {cmd}  (log {logname})",
            "input": f"input check failed: {cmd}  (log {logname})",
            "tip": f"tip check failed: {cmd}  (log {logname})",
            "dirty": f"the gates left tracked changes in the gate mirror: {cmd}"}.get(
        kind, f"gate mirror post-gate error: {cmd}  (log {logname}); "
              f"`lanes.py gate-mirror --reset` rebuilds it")


def apply_mirror_post(ctx: Ctx, lane: str, ld: Path, base: str, log) -> list:
    """Bring the mirror's regenerated outputs into the landing worktree: apply its patch on the
    gated commit, fold it like regenerate() does, and require the mirror's tree for every
    commit base..HEAD (so the checks it ran judged exactly what lands). Returns changed paths."""
    res = ctx.mirror_post
    if res is None:
        raise Refuse("the gate mirror returned no post-gate result", 4)
    changed = list(res.get("changed", []))
    if changed:
        pf = ctx.state_dir / "gate-mirror" / f"regen-{lane}.patch"
        pf.parent.mkdir(parents=True, exist_ok=True)
        pf.write_bytes(res["patch"])
        r = run(["git", "apply", "--index", "--binary", "--whitespace=nowarn", str(pf)], cwd=ld,
                check=False)
        if r.returncode != 0:
            raise Refuse(f"could not apply the mirror's regenerated files:\n"
                         f"{(r.stderr or '')[-1500:]}", 4)
        staged = set(git("diff", "--cached", "--name-only", "--no-renames", cwd=ld).split())
        if staged != set(changed):
            raise Refuse(f"the mirror's patch staged {sorted(staged)}, not {sorted(changed)}", 4)
    entries = {g["path"]: g for g in ctx.cfg.get("regenerate", [])}
    fold_regenerated(ctx, lane, ld, [entries.get(p, {"path": p, "subject": "docs: refresh"})
                                     for p in changed])
    trees = [git("rev-parse", c + "^{tree}", cwd=ld)
             for c in git("rev-list", "--reverse", f"{base}..HEAD", cwd=ld).split()]
    if trees != res.get("trees"):
        raise Refuse(f"the landing worktree's trees after the regenerate fold ({len(trees)} "
                     f"commit(s)) differ from the mirror's ({len(res.get('trees') or [])}); "
                     f"main was not changed (log {getattr(log, 'name', '?')})", 4)
    return changed


def mirror_tip_extra(ctx: Ctx, gated: str, tip: str, cwd: Path) -> list[str]:
    """Paths where `tip`'s tree differs from the gated commit's, outside the regenerated files.
    Trees, not ancestry: regenerate amends the gated commit (or drops it when it empties)."""
    regen = {r["path"] for r in ctx.cfg.get("regenerate", [])}
    return sorted(set(git("diff", "--name-only", "--no-renames", gated, tip, cwd=cwd).split())
                  - regen)


def cmd_gate_mirror(ctx: Ctx, a):
    m = ctx.cfg.get("gate_mirror")
    if not m or ctx.cfg.get("runner") == "local":
        raise Refuse("no gate_mirror in lanes.json (or the local runner): gates run in the "
                     "landing worktree")
    r = run_wsl_script(ctx, "create", mirror_create_script(ctx, m, reset=a.reset))  # flocked
    out = ((r.stdout or "") + (r.stderr or "")).strip()
    if out:
        print(out)
    if r.returncode != 0:
        raise Refuse(f"gate mirror setup failed (exit {r.returncode})", 1)
    print(f"lanes: gate mirror ~/{m.get('path', 'bb2-gate')} ready")
    return 0


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
        print(json.dumps({"lanes": reg["lanes"], "main_lock": lk,
                          "gate_queue": reg.get("gate_queue", [])}, indent=2))
        return 0
    main_head = git("rev-parse", "--short", "main", cwd=ctx.main)
    age = lock_age(lk) if lk is not None else 0
    print(f"main {main_head}   lock: " + (f"HELD by {lk.get('label') or lk.get('session')} "
                                          + (f"({int(age)}s)" if math.isfinite(age)
                                             else "(malformed ts: stale)")
                                          + (" STALE" if age > LOCK_STALE_SECS else "")
                                          if lk is not None else "free"))
    print("full-gate queue:" + (" empty" if not reg.get("gate_queue") else ""))
    for position, ticket in enumerate(reg.get("gate_queue", []), 1):
        since = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(ticket["enqueued"]))
        age = int(time.time() - ticket.get("beat", ticket["enqueued"]))
        print(f"  {position}. {ticket['lane']} waiting since {since}; beat age {age}s")
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
        new = []
        for tok in tokens:
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
                    if claims_overlap(kind, oval, val):
                        raise Refuse(f"'{tok}' overlaps lane '{other}' claim '{otok}'")
            canon = f"{kind}:{val}{'!' if freeze else ''}"
            if canon not in e["claims"] and canon not in new:
                new.append(canon)
        e["claims"].extend(new)
        return list(e["claims"])

    tokens = list(a.tokens)
    for d in getattr(a, "wave", None) or []:
        got = wave_claims(ctx.main, d)
        if not got:
            raise Refuse(f"--wave {d}: no draft there, or it names no TU")
        tokens += sorted(got)
    if not tokens:
        raise Refuse("claim: give tokens or --wave <draft dir>")
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
        br = ctx.branch(a.lane)
        if git_ok("rev-parse", "--verify", "-q", f"refs/heads/{br}", cwd=ctx.main):
            left = unlanded(ctx, a.lane, br)
            if left:
                print(f"lanes: kept branch {br}: {len(left)} unlanded commit(s)")
            else:
                git("branch", "-D", br, cwd=ctx.main)  # every commit is in main (unlanded())
                print(f"lanes: deleted branch {br} (all of it is on main)")
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


INERT_DEFAULT = ("docs/", "memory/", ".claude/")


def needs_full_gates(lane_paths, main_paths, inert=INERT_DEFAULT, docs_only=("docs/", "memory/"),
                     mergeable=()):
    """Decide whether a candidate that passed the full gates on main M1 must be re-gated in full
    after main moved to M2 (pure: two path lists in, (bool, reason) out).

    `lane_paths`: what the lane changes; `main_paths`: what changed on main M1..M2. A path is a
    BUILD INPUT unless it is under an `inert` prefix (lanes.json inert_paths: docs/, memory/,
    .claude/, tools/trace/, tools/lanes/): everything else (src/, include/, asm/, Makefile, *.ld,
    engine/, other tools/, root registries, AGENTS.md...) counts.
    `mergeable` paths (regenerated outputs, row-merged registries) may change on both sides: the
    replay merges them mechanically and the regenerate step rebuilds them on the final tree.
    `docs_only` is the prefix set under which a lane skips gates and regeneration entries
    without declared inputs.

    A cheap re-check (clean replay + quick gates + regenerate) suffices only when the sides are
    disjoint (outside `mergeable`) and at most one side touched build inputs. A lane that is not
    docs-only also needs main's side to be inert: its regenerate step reads the previous build."""
    def low(ps):
        return {p.replace("\\", "/").lower() for p in ps}  # NTFS is case-insensitive

    def all_under(ps, prefixes):
        return all(under_scope(p, prefixes) for p in ps)

    merge = low(mergeable)
    lane, moved = low(lane_paths) - merge, low(main_paths) - merge
    if not moved:
        return False, "main changed nothing the lane's gates depend on"
    both = sorted(lane & moved)
    if both:
        return True, "main changed paths the lane also changes: " + ", ".join(both[:5])
    lane_build, main_build = not all_under(lane, inert), not all_under(moved, inert)
    if lane_build and main_build:
        return True, "build inputs changed on both sides"
    if main_build and not all_under(low(lane_paths), docs_only):
        return True, "main changed build inputs under a gated lane"
    return False, ("main changed only " + ("docs" if not main_build else "build inputs the "
                                           "docs-only lane cannot affect"))


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


class NeedFullGates(Exception):
    """build_candidate's quick re-check found the replay was not mechanical."""


# the full gates' integrity check reads "inert" files (layer2.jsonl ledgers, naming manifests,
# the census), so it reruns on every quick re-check; it reuses the gated build/ (same inputs)
QUICK_GATES_DEFAULT = ("python3 -m engine.cli tus-check", "python3 tools/check_completion_integrity.py")


def mergeable_paths(ctx: Ctx) -> set:
    """Paths both sides may change without a hand merge: regenerated outputs and row-merged
    registries (see needs_full_gates)."""
    return {g["path"] for g in ctx.cfg.get("regenerate", [])} | set(ctx.cfg.get("rowmerge", []))


WAVE_SUBJECT = re.compile(r"^naming: ([\w.+-]+) -- ")
WAVE_PENDING = "docs/naming/phase3/pending/{}/" + PAIRS_FILE


def wave_pairs(ld: Path, rev: str) -> str | None:
    """The pairs a naming wave commit applies (subject `naming: <id>[+<id>...] -- ...`, each
    id's pairs.txt in that commit's tree), or None if it is not such a commit."""
    m = WAVE_SUBJECT.match(git("log", "-1", "--format=%s", rev, cwd=ld))
    if not m:
        return None
    texts = []
    for rid in m.group(1).split("+"):
        r = run(["git", "show", f"{rev}:{WAVE_PENDING.format(rid)}"], cwd=ld, check=False)
        if r.returncode != 0:
            return None  # a file move or function wave: no pairs to check against
        texts.append(r.stdout)
    return "\n".join(texts)


KEYCHECK_OK = re.compile(r"^OK: every change is explained by the pairs\s*$", re.M)
KEYCHECK_FAIL = re.compile(r"^FAIL: \d+ change\(s\) the pairs do not explain\s*$", re.M)


def _keycheck(ctx: Ctx, ld: Path, rev: str, pairs: str, log) -> str:
    """'ok' | 'fail' (naming_keycheck's own verdict) | 'error' (no verdict: a crash, a missing
    toolchain, a git failure; never read as either)."""
    d = ld / "tmp" / "lanes"
    d.mkdir(parents=True, exist_ok=True)
    pf, of = d / f"keycheck-{rev[:12]}.txt", d / f"keycheck-{rev[:12]}.out"
    pf.write_text(pairs, encoding="utf-8", newline="\n")
    of.unlink(missing_ok=True)
    try:
        rc = run_shell(ctx, ld, "rc=0; python3 tools/naming_keycheck.py --pairs-file "
                       f"tmp/lanes/{pf.name} --base {rev}^ --new {rev} --census-regenerated "
                       f"> tmp/lanes/{of.name} 2>&1 || rc=$?; cat tmp/lanes/{of.name}; exit $rc",
                       log)
        out = of.read_text(encoding="utf-8", errors="replace") if of.is_file() else ""
    finally:
        pf.unlink(missing_ok=True)
        of.unlink(missing_ok=True)
    if rc == 0 and KEYCHECK_OK.search(out):
        return "ok"
    if rc == 1 and KEYCHECK_FAIL.search(out) and "toolchain missing" not in out:
        return "fail"
    return "error"


def replay_keychecks(ctx: Ctx, lane: str, ld: Path, log):
    """Re-prove every naming wave the replay moved onto a different parent: its replayed diff
    must still be explained by its own pairs (tools/naming_keycheck.py --new), now that waves
    claim symbols rather than whole TUs and another wave may have landed in the same TU.
    Its FAIL verdict bounces the commit (exit 3); no verdict at all fails the gate (exit 4)."""
    for sha, new in ctx.replayed.items():
        if ctx.replay_how.get(sha) == "empty":
            continue  # already in main: nothing of it lands
        if new == sha or git("rev-parse", f"{new}^", cwd=ld) == git("rev-parse", f"{sha}^", cwd=ld):
            continue  # not moved: the pre-land keycheck still holds
        pairs = wave_pairs(ld, new)
        if pairs is None:
            continue
        print(f"lanes: keycheck on replayed naming wave {sha[:9]} -> {new[:9]}", flush=True)
        verdict = _keycheck(ctx, ld, new, pairs, log)
        if verdict == "ok":
            continue
        if verdict == "error":
            raise Refuse(f"naming_keycheck gave no verdict on replayed wave {sha[:9]} (a crash "
                         f"or a broken toolchain; log {log.name}); main was not changed", 4)
        raise Refuse(f"naming wave {sha[:9]} is no longer explained by its pairs after the "
                     f"replay onto main (naming_keycheck; log {log.name}): another wave changed "
                     f"the same code\n  Drop just this commit (later commits are kept) and redo "
                     f"it on the new main: python tools/lanes/lanes.py drop {lane} {sha[:9]}", 3)


def build_candidate(ctx: Ctx, lane: str, ld: Path, commits, base: str, log,
                    quick_paths=None) -> str:
    """Replay the lane's commits onto `base`, gate, regenerate, then check the final tip once.
    No reintegration lock is held: nothing here touches main or the lane worktree. Returns the gated tip
    (equal to `base` if every commit was already in main).

    `quick_paths` (the lane's paths from an earlier full pass; cmd_land passes it only when
    needs_full_gates() said no) runs the quick gates instead of the full ones, provided the replay
    stays mechanical: no Lane-Replay re-run and no path outside that set. Otherwise it raises
    NeedFullGates and the caller decides (within its full-run budget) whether to run them. Sets ctx.gated to "full" | "quick" | "docs" and ctx.cand_paths to the
    candidate's changed paths (before regeneration)."""
    run(["git", "cherry-pick", "--abort"], cwd=ld, check=False)
    git("checkout", "-q", "--detach", "-f", base, cwd=ld)
    git("reset", "-q", "--hard", base, cwd=ld)
    ctx.replayed = {}  # lane sha -> the candidate commit holding it (base's tip if it was empty)
    ctx.replay_how = {}  # lane sha -> replay_commit's result
    ctx.gated, ctx.cand_paths = "full", set()
    ctx.mirror_gated = ctx.mirror_tip = None
    hows = []
    for sha in commits:
        try:
            how = replay_commit(ctx, ld, sha, log)
        except Refuse as e:
            if e.code == 3:  # every bounce names the commit and the way out
                raise Refuse(f"{e}\n  Drop just this commit (later commits are kept) and redo it "
                             f"on the new main: python tools/lanes/lanes.py drop {lane} {sha[:9]}", 3)
            raise
        hows.append(how)
        ctx.replayed[sha] = git("rev-parse", "HEAD", cwd=ld)
        ctx.replay_how[sha] = how
        print(f"lanes:   {sha[:9]} {how}", flush=True)
    replay_keychecks(ctx, lane, ld, log)
    new_paths = set(git("diff", "--name-only", "--no-renames", f"{base}..HEAD", cwd=ld).split())
    ctx.cand_paths = new_paths
    if not new_paths:
        input_checks(ctx, ld, base, log)
        return git("rev-parse", "HEAD", cwd=ld)
    docs_only = is_docs_only(ctx, new_paths)
    quick = quick_paths is not None and not docs_only
    if quick:
        allowed = {p.lower() for p in quick_paths} | {p.lower() for p in mergeable_paths(ctx)}
        extra = sorted(p for p in new_paths if p.lower() not in allowed)
        if "replayed" in hows or extra:
            raise NeedFullGates("the replay onto the new main was not mechanical ("
                                + ("a Lane-Replay re-ran" if "replayed" in hows else
                                   "new paths: " + ", ".join(extra[:5])) + ")")
    kind = ctx.load()["lanes"][lane].get("kind", "foundation")
    if docs_only:
        gates = []
        ctx.gated = "docs"
        print("lanes: docs-only change; build gates skipped")
    elif quick:
        gates = list(ctx.cfg.get("quick_gates", QUICK_GATES_DEFAULT))
        ctx.gated = "quick"
        print("lanes: main moved only in files disjoint from this lane (see needs_full_gates); "
              "quick re-check instead of the full gates", flush=True)
    else:
        gates = scope_gates(ctx, ld, base, new_paths,
                            ctx.cfg["gates"].get(kind, ctx.cfg["gates"]["foundation"]))
    check_untracked(ld)
    post = mirror_post_plan(ctx, ld, base, docs_only) if mirror_cfg(ctx) is not None else None
    # the mirror runs the gates and the post-gate steps; a docs-only land with nothing to
    # regenerate or check never waits on the mirror's lock
    mirrored = post is not None and (bool(gates) or mirror_post_needed(ctx, ld, post))
    if mirrored and quick and not any("verify-oracle" in g for g in gates):
        gates = ["python3 -m engine.cli verify-oracle --rebuild"] + gates
    if mirrored:
        mirror_sha1 = run_gates_mirror(ctx, lane, ld, gates, log, post=post)
    else:
        for g in gates:
            t0 = time.time()
            rc = run_shell(ctx, ld, g, log)
            log.write(f"[gate {g}: {time.time() - t0:.0f}s]\n")
            if rc != 0:
                raise Refuse(f"gate failed: {g}  (log {log.name})", 4)
    if gates and (not quick or mirrored) and ctx.cfg.get("oracle_sha1"):
        if mirrored:
            exe, got = f"~/{mirror_cfg(ctx).get('path', 'bb2-gate')}/{ctx.cfg['oracle_exe']}", \
                mirror_sha1 or "missing"
        else:
            exe = ld / ctx.cfg["oracle_exe"]
            got = hashlib.sha1(exe.read_bytes()).hexdigest() if exe.exists() else "missing"
        if got != ctx.cfg["oracle_sha1"]:
            raise Refuse(f"{exe} SHA1 {got} != oracle {ctx.cfg['oracle_sha1']}", 4)
        print(f"lanes: SHA1 == oracle ({got[:12]})")
    gated_sha = git("rev-parse", "HEAD", cwd=ld)
    if mirrored:  # regenerated and checked in the mirror: apply its outputs, same trees
        apply_mirror_post(ctx, lane, ld, base, log)
    else:
        regenerated = regenerate(ctx, lane, ld, log, docs_only=docs_only)
    if mirrored:  # what lands is the gated commit plus only the regenerated files
        extra = mirror_tip_extra(ctx, gated_sha, "HEAD", ld)
        if ctx.mirror_gated != gated_sha or extra:
            raise Refuse(f"the gated commit {str(ctx.mirror_gated)[:9]} is not the candidate "
                         f"{gated_sha[:9]}" + (f" (after it: {', '.join(extra[:5])})"
                                               if extra else ""), 4)
        ctx.mirror_tip = gated_sha  # cmd_land re-checks the tip it fast-forwards
    else:
        input_checks(ctx, ld, base, log)
        tip_checks(ctx, ld, log, regenerated)
    if git("status", "--porcelain", "--untracked-files=no", cwd=ld):
        raise Refuse("the gates left tracked changes in the landing worktree: "
                     + git("status", "--porcelain", "--untracked-files=no", cwd=ld), 4)
    return git("rev-parse", "HEAD", cwd=ld)


def under_scope(path, entries):
    """`entries` ending in "/" are prefixes, entries holding `*` are fnmatch globs (`*` spans
    `/`), others name one file; case-insensitive (NTFS)."""
    p = path.replace("\\", "/").casefold()
    return any(p.startswith(e.casefold()) if e.endswith("/") else
               fnmatch.fnmatchcase(p, e.casefold()) if "*" in e else p == e.casefold()
               for e in entries)


# What a token-level rename may never touch: C keywords (a swap is structure, not a name) and
# MIPS register / directive words (asm/ operands are not symbols).
_RENAME_FIXED = set("auto break case char const continue default do double else enum extern "
                    "float for goto if int long register return short signed sizeof static "
                    "struct switch typedef union unsigned void volatile while asm __asm__ "
                    "inline __inline__ __attribute__ typeof __typeof__ define include ifdef "
                    "ifndef endif undef elif pragma INCLUDE_ASM INCLUDE_RODATA glabel".split())
_RENAME_REG = re.compile(r"(?:zero|at|v[01]|a[0-3]|t\d|s[0-8]|k[01]|gp|sp|fp|ra|f\d+|\d+)$")
_RENAME_TOKEN = re.compile(r"[A-Za-z_]\w*|\d\w*|\S")


def token_only_rename(ld: Path, base: str, paths, unread_by=()) -> tuple[bool, str]:
    """Prove base..HEAD changed `paths` by a token-level identifier substitution only
    (whitespace ignored, comments and strings included): equal token sequences except at
    identifier positions, one old->new map over every path that merges no two names within
    the changed files (no target is also an unchanged name there), no keyword or register in
    it. An old name may survive elsewhere (a scoped local rename). It is the claim
    tools/naming_keycheck.py proves from a wave's pairs, derived from the diff so the lander
    needs no manifest; it does not prove semantic identity (a typedef or mnemonic swap passes),
    which the always-run oracle rebuild covers. Fails closed on an added, deleted or unreadable
    path and on any other token change; text is decoded with errors="replace", so a binary
    or line-ending change is the oracle's to catch, not this check's.

    `unread_by`: git pathspecs (engine/, tools/, minus scenario data) whose code and tests
    may hard-code symbol names; an old name occurring there as a word in `base` is no proof (the skipped gate may
    read it from the live tree)."""
    fwd, back = {}, {}
    for p in sorted(paths):
        try:
            old = git("show", f"{base}:{p}", cwd=ld)
            new = git("show", f"HEAD:{p}", cwd=ld)
        except Refuse:  # added, deleted or unreadable: not a rename
            return False, f"{p}: added or deleted"
        a, b = _RENAME_TOKEN.findall(old), _RENAME_TOKEN.findall(new)
        if len(a) != len(b):
            return False, f"{p}: token count changed ({len(a)} -> {len(b)})"
        for x, y in zip(a, b):
            if x == y:
                # an unchanged name may not also be a rename target (two names merged into
                # one); it may still be renamed elsewhere (a scoped/local rename)
                if re.match(r"[A-Za-z_]", x) and back.setdefault(x, x) != x:
                    return False, f"{p}: {back[x]!r} renamed onto the kept name {x!r}"
                continue
            if not (re.match(r"[A-Za-z_]", x) and re.match(r"[A-Za-z_]", y)) \
                    or {x, y} & _RENAME_FIXED or _RENAME_REG.match(x) or _RENAME_REG.match(y):
                return False, f"{p}: non-identifier change {x!r} -> {y!r}"
            if fwd.setdefault(x, y) != y or back.setdefault(y, x) != x:
                return False, f"{p}: inconsistent rename {x!r} -> {y!r}"
    if fwd and unread_by:
        # old and new names: a new name a reader already uses could shadow what it looks up
        names = sorted(set(fwd) | set(fwd.values()))
        hit = run(["git", "grep", "-l", "-w", "-F", "-f", "-", base, "--", *unread_by], cwd=ld,
                  check=False, input_text="\n".join(names) + "\n")  # stdin: no argv limit
        if hit.returncode not in (0, 1):
            return False, "git grep failed: " + (hit.stderr or "").strip()[:200]
        if hit.stdout.strip():
            files = [ln.split(":", 1)[-1] for ln in hit.stdout.split()]
            return False, "a renamed name occurs in " + ", ".join(files[:5])
    return True, f"{len(fwd)} identifier(s) renamed, nothing else changed"


def scope_gates(ctx: Ctx, ld: Path, base: str, paths, gates) -> list:
    """Size the full gates to the candidate's diff (lanes.json gate_scope). A gate with a
    scope rule is skipped when every changed path is under its `light` entries, or when the
    rest are all under its `renames` entries and token_only_rename() (with the rule's
    `unread_by` git pathspecs) proves them a token-level rename. Gates without a rule
    (verify-oracle, tus-check, the label census) always run."""
    scope = ctx.cfg.get("gate_scope", {})
    proofs = {}
    out = []
    for g in gates:
        rule = scope.get(g)
        if not rule:
            out.append(g)
            continue
        rest = [p for p in paths if not under_scope(p, rule.get("light", []))]
        why = None
        if not rest:
            why = "every changed path is under " + ", ".join(rule.get("light", []))
        elif rule.get("renames") and all(under_scope(p, rule["renames"]) for p in rest):
            key = (tuple(sorted(rest)), tuple(rule.get("unread_by", [])))
            if key not in proofs:
                proofs[key] = token_only_rename(ld, base, rest, rule.get("unread_by", []))
            proof = proofs[key]
            if proof[0]:
                why = "token-only rename: " + proof[1]
            else:
                print(f"lanes: not a token-only rename ({proof[1]})", flush=True)
        if why:
            print(f"lanes: gate skipped: {g}  ({why})", flush=True)
            continue
        out.append(g)
    return out


def matches_input_prefix(path, prefixes):
    """Match configured input prefixes with Windows' case-insensitive spelling."""
    return any(path.casefold().startswith(prefix.casefold()) for prefix in prefixes)


def tip_checks(ctx: Ctx, ld: Path, log, regenerated):
    """Check HEAD once for inputs touched by the candidate or regenerated outputs."""
    paths = set(ctx.cand_paths) | set(regenerated)
    for check in ctx.cfg.get("tip_checks", []):
        if not any(matches_input_prefix(p, check["inputs"]) for p in paths):
            continue
        cmd = check["cmd"]
        if run_shell(ctx, ld, cmd, log) != 0:
            raise Refuse(f"tip check failed: {cmd}  (log {log.name})", 4)


def input_checks(ctx: Ctx, ld: Path, base: str, log):
    """Gate every landed tree, including intermediate commits and amended tips.

    Scope each command by that commit's inputs, so docs-only and quick lands
    receive the same checks and a later fix cannot conceal an invalid commit.
    """
    checks = ctx.cfg.get("input_checks", [])
    if not checks:
        return
    commits = git("rev-list", "--reverse", f"{base}..HEAD", cwd=ld).splitlines()
    for commit in commits:
        paths = git("diff-tree", "--no-commit-id", "--name-only", "-z", "-r",
                    "--no-renames", commit, cwd=ld).split("\0")
        for check in checks:
            if not any(matches_input_prefix(p, check["inputs"]) for p in paths):
                continue
            cmd = check["cmd"] + " --rev " + commit
            if run_shell(ctx, ld, cmd, log) != 0:
                raise Refuse(f"input check failed: {cmd}  (log {log.name})", 4)


def regenerate(ctx: Ctx, lane: str, ld: Path, log, docs_only=False):
    """Rebuild the lanes.json "regenerate" outputs and fold any change into the candidate's last
    lane commit (amend), so a land adds no separate refresh commit and every landed commit holds
    a census that matches its tree. The amend keeps message and author and runs the commit-msg
    chain like any commit (its staged diff is only the regenerated files). ctx.replayed entries
    that pointed at the amended commit are moved to the new one.

    Entries with inputs run only when the candidate's pre-regeneration paths touch an input
    prefix (excluding the output itself), including on docs-only lands. Thus a failing runtime
    events build refuses only lands touching its capture inputs. Entries without inputs keep
    running on every non-docs land and never on docs-only lands. Returns changed output paths."""
    changed = []
    for g in regen_entries(ctx, docs_only):
        if run_shell(ctx, ld, g["cmd"], log) != 0:
            raise Refuse(f"regenerate failed: {g['cmd']}  (log {log.name})", 4)
        git("add", "--", g["path"], cwd=ld)
        if not git_ok("diff", "--cached", "--quiet", "--", g["path"], cwd=ld):
            changed.append(g)
    return fold_regenerated(ctx, lane, ld, changed)


def regen_entries(ctx: Ctx, docs_only=False) -> list:
    """The lanes.json "regenerate" entries this candidate runs (regenerate()'s scoping)."""
    out = []
    for g in ctx.cfg.get("regenerate", []):
        if "inputs" in g:
            # the output itself is excluded exactly (as before); only the input prefix test
            # casefolds, so the step can only run more often
            if not any(p != g["path"]
                       and matches_input_prefix(p, g["inputs"])
                       for p in ctx.cand_paths):
                continue
        elif docs_only:
            continue
        out.append(g)
    return out


def fold_regenerated(ctx: Ctx, lane: str, ld: Path, changed) -> list:
    """Fold the staged, changed regenerate outputs (`changed`: entries) into the candidate."""
    if not changed:
        return []
    old = git("rev-parse", "HEAD", cwd=ld)
    if old in ctx.replayed.values() and git_ok("diff", "--cached", "--quiet", "HEAD~1", cwd=ld):
        # the commit changed only regenerated files, and regeneration undid that: drop it (its
        # lane commits count as landed in its parent)
        git("reset", "-q", "--hard", "HEAD~1", cwd=ld)
        new = git("rev-parse", "HEAD", cwd=ld)
        ctx.replayed = {k: (new if v == old else v) for k, v in ctx.replayed.items()}
        how = f"{old[:9]} held only regenerated files and was dropped"
    elif old in ctx.replayed.values():
        env = dict(os.environ, GIT_EDITOR="true")
        r = run(["git", "commit", "-q", "--amend", "--no-edit"], cwd=ld, check=False, env=env)
        if r.returncode != 0:
            raise Refuse(f"could not fold the regenerated files into {old[:9]}:\n"
                         f"{(r.stdout or '')[-1500:]}{(r.stderr or '')[-1500:]}", 4)
        new = git("rev-parse", "HEAD", cwd=ld)
        ctx.replayed = {k: (new if v == old else v) for k, v in ctx.replayed.items()}
        how = f"folded into {git('log', '-1', '--format=%h %s', 'HEAD', cwd=ld)[:72]}"
    else:  # HEAD is not a lane commit (cannot happen with a non-empty diff): own commit
        msgfile = ctx.state_dir / f"regen-msg-{lane}.txt"
        msgfile.write_text(f"{changed[0]['subject']} after lane {lane}\n", encoding="utf-8",
                           newline="\n")
        git("commit", "-q", "-F", str(msgfile), cwd=ld)
        how = "own commit"
    for g in changed:
        print(f"lanes:   regenerated {g['path']} ({how})")
    return [g["path"] for g in changed]


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
    attached = git("symbolic-ref", "-q", "HEAD", cwd=wt, check=False)
    busy = [p for p in ("rebase-merge", "rebase-apply", "MERGE_HEAD", "CHERRY_PICK_HEAD",
                        "REVERT_HEAD") if Path(wt, git("rev-parse", "--git-path", p, cwd=wt)).exists()]
    if attached != f"refs/heads/{br}" or busy:
        print(f"lanes: note: {br} was left as is ("
              + (f"a {busy[0]} is in progress" if busy else "HEAD is not on the branch")
              + "); the next `land` skips the already-landed commits")
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
        old = git("rev-parse", "--verify", "-q", f"{br}@{{1}}", cwd=wt, check=False)
        if old and old != head and unlanded(ctx, lane, old, base=m):
            ref = f"refs/lanes-backup/{lane}/raced-{old[:9]}"
            git("update-ref", ref, old, cwd=wt)
            back = run(["git", "reset", "-q", "--keep", old], cwd=wt, check=False).returncode == 0
            print(f"lanes: note: a commit arrived while {br} was being moved; "
                  + (f"{br} was put back on it" if back else f"{br} could not be put back on it")
                  + f" (saved as {ref})")
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


LAND_ATTEMPTS = 6       # replays onto a moved main, cheap re-checks included
FULL_GATE_ATTEMPTS = 3  # of which full gate runs (~25 min each with three lanes live)


def cheap_recheck_paths(ctx: Ctx, full_pass, base: str, commits):
    """The lane paths to hand build_candidate for a quick re-check onto `base`, or None when the
    full gates must run: no earlier full pass, a different commit set, `base` not a descendant of
    the gated base (main was reset), or needs_full_gates() over the gated base..`base` delta."""
    if full_pass is None:
        return None
    gbase, gcommits, lane_paths = full_pass
    if list(commits) != gcommits or gbase == base:
        return None
    if not git_ok("merge-base", "--is-ancestor", gbase, base, cwd=ctx.main):
        print("lanes: main was rewritten since the gated base; full gates", flush=True)
        return None
    moved = git("diff", "--name-only", "--no-renames", gbase, base, cwd=ctx.main).split()
    inert = ctx.cfg.get("inert_paths", list(INERT_DEFAULT))
    full, why = needs_full_gates(lane_paths, moved, inert, ctx.cfg.get("docs_only_paths", []),
                                 mergeable_paths(ctx))
    print(f"lanes: main moved {gbase[:9]}..{base[:9]} ({len(moved)} path(s)): {why}"
          + ("; full gates" if full else ""), flush=True)
    return None if full else sorted(lane_paths)


def cmd_land(ctx: Ctx, a):
    lane = a.lane
    ctx.update(lambda reg: require_owner(ctx, reg, lane))
    wt, br = ctx.lane_dir(lane), ctx.branch(lane)
    recover_lane(ctx, lane)  # refuses if a land of this lane is still running
    tipsrc = git("rev-parse", br, cwd=ctx.main)  # land exactly this; later lane commits wait
    commits = unlanded(ctx, lane, tipsrc)
    if not commits:
        print("lanes: nothing to land")
        if wt.exists() and git("rev-list", "-n1", tipsrc, "^main", cwd=ctx.main):
            advance_lane(ctx, lane, tipsrc)  # a branch an earlier land had to leave as is
        return 0
    if any(len(git("rev-list", "--parents", "-n1", c, cwd=ctx.main).split()) > 2 for c in commits):
        raise Refuse("the lane holds merge commits; replay cannot carry them. Rebuild the lane as "
                     "linear commits on fresh main (lanes.py fresh --discard).")
    check_claims(ctx, lane, paths_of(ctx, commits))
    remote = None if a.no_push else ctx.cfg.get("remote")
    full_pass = None  # (base, commits, lane paths) of the last full gate pass
    full_runs = attempts = 0
    queue = GateQueue(ctx, lane)
    ld = log = None
    body_failed = main_fast_forwarded = False
    try:
        # Refuse --no-wait before bootstrapping a landing tree; refresh main only after waiting.
        land_pidfile(ctx, lane).write_text(str(os.getpid()), encoding="utf-8")
        lk = read_lock(ctx)
        held_before = bool(lk and lk.get("session") == ctx.session
                           and lock_age(lk) < LOCK_STALE_SECS)
        if held_before:
            write_lock(ctx, lk.get("label") or f"lanes land {lane}")
        # The lane's own paths: build_candidate later decides on the replayed diff, which can
        # differ; that changes only who waits, never which gates run. A session that already
        # holds the main lock does not queue: no one can fast-forward main ahead of it, and a
        # head waiting on its lock would otherwise wait on it while it waits on the head.
        if queues_full_gates(ctx, paths_of(ctx, commits)):
            if held_before:
                print("lanes: this session holds the main lock; not queueing for the full gates",
                      flush=True)
            else:
                queue.wait(getattr(a, "no_wait", False))
        ld = ensure_land_tree(ctx, lane)
        logp = ctx.state_dir / "logs" / f"{lane}-{time.strftime('%Y%m%d-%H%M%S')}.log"
        logp.parent.mkdir(parents=True, exist_ok=True)
        log = open(logp, "w", encoding="utf-8", newline="\n")
        for attempt in range(LAND_ATTEMPTS):
            base = git("rev-parse", "main", cwd=ctx.main)
            # against this attempt's base: main may have moved, or been reset, since the last look
            commits = unlanded(ctx, lane, tipsrc, base=base)
            if any(len(git("rev-list", "--parents", "-n1", c, cwd=ctx.main).split()) > 2
                   for c in commits):
                raise Refuse("the lane holds merge commits; rebuild it as linear commits")
            quick_paths = cheap_recheck_paths(ctx, full_pass, base, commits)
            if quick_paths is None and full_runs >= FULL_GATE_ATTEMPTS:
                break
            attempts += 1
            print(f"lanes: landing {len(commits)} commit(s) of {lane} onto main {base[:9]}"
                  + (f" (retry {attempt}: main moved)" if attempt else ""), flush=True)
            tip = None
            if quick_paths is not None:
                try:
                    tip = build_candidate(ctx, lane, ld, commits, base, log,  # no lock held
                                          quick_paths=quick_paths)
                except NeedFullGates as e:
                    print(f"lanes: {e}; full gates", flush=True)
                    if full_runs >= FULL_GATE_ATTEMPTS:
                        break
            if tip is None:
                full_runs += 1
                tip = build_candidate(ctx, lane, ld, commits, base, log)  # no lock held
            if getattr(ctx, "gated", "full") != "quick":
                full_pass = (base, list(commits),
                             set(getattr(ctx, "cand_paths", ())) | set(paths_of(ctx, commits)))
            if tip == base:
                print("lanes: every lane commit is already in main; nothing new to land")
                mark_landed(ctx, lane, ctx.replayed)
                advance_lane(ctx, lane, tipsrc)
                return 0
            # Short critical section: verify nothing moved, then fast-forward and push.
            if not held_before:
                acquire_lock(ctx, f"lanes land {lane}")
            try:
                # Ours before anything here can move main (sync fast-forwards it); a lock held
                # from before the land gets a fresh ts (keeping its holder's label).
                lk = read_lock(ctx)
                if not lk or lk.get("session") != ctx.session:
                    raise Refuse("lost the main lock mid-land (another session took it); main was "
                                 "not changed by this land", 1)
                if held_before:
                    write_lock(ctx, lk.get("label") or f"lanes land {lane}")
                check_claims(ctx, lane,  # a freeze may have arrived meanwhile
                             git("diff", "--name-only", "--no-renames", f"{base}..{tip}",
                                 cwd=ctx.main).split())
                check_main_idle(ctx)
                if remote:
                    sync_main_with_remote(ctx, remote)
                if git("rev-parse", "main", cwd=ctx.main) != base:
                    continue  # main moved: rebuild on the new main (finally releases the lock)
                lk = read_lock(ctx)  # and still ours right before the fast-forward
                if not lk or lk.get("session") != ctx.session:
                    raise Refuse("lost the main lock mid-land (another session took it); main was "
                                 "not changed by this land", 1)
                # recorded first: a mapping is trusted only once its commit is in main, so this
                # is harmless if the fast-forward fails, and a failure here cannot leave main
                # holding lane work it does not know has landed
                gated = getattr(ctx, "mirror_tip", None)
                if gated and mirror_tip_extra(ctx, gated, tip, ctx.main):
                    raise Refuse(f"the tip {tip[:9]} is not the mirror-gated commit "
                                 f"{gated[:9]} plus regenerated files; main was not changed", 4)
                mark_landed(ctx, lane, ctx.replayed)
                git("merge", "-q", "--ff-only", tip, cwd=ctx.main)
                main_fast_forwarded = True
                pushed = push_main(ctx, remote) if remote else "not pushed: --no-push"
            finally:
                if not held_before:
                    release_lock(ctx)
            print(f"lanes: main -> {tip[:9]} ({pushed})")
            advance_lane(ctx, lane, tipsrc)
            return 0
        raise Refuse(f"main kept moving during {attempts} land attempts ({full_runs} full gate "
                     f"runs); try again", 1)
    except BaseException:
        body_failed = True
        raise
    finally:
        cleanup_error = None
        cleanup_interrupt = None
        for cleanup in (
            lambda: run(["git", "cherry-pick", "--abort"], cwd=ld, check=False)
            if ld is not None else None,
            lambda: land_pidfile(ctx, lane).unlink(missing_ok=True),
            lambda: log.close() if log is not None else None,
            queue.close,  # last, after all landing resources have been cleaned up
        ):
            try:
                cleanup()
            except BaseException as e:
                if not isinstance(e, Exception) and cleanup_interrupt is None:
                    cleanup_interrupt = e
                elif cleanup_error is None:
                    cleanup_error = e
        if cleanup_interrupt is not None:
            raise cleanup_interrupt
        if cleanup_error is not None and not body_failed:
            if main_fast_forwarded:
                print(f"lanes: warning: cleanup failed after main fast-forwarded: {cleanup_error}",
                      file=sys.stderr)
            else:
                raise cleanup_error


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
    s = sub.add_parser("claim"); s.add_argument("lane"); s.add_argument("tokens", nargs="*")
    s.add_argument("--wave", action="append", metavar="DRAFT",
                   help="also claim a naming draft's symbols/TUs (wave_claims); repeatable")
    s = sub.add_parser("unclaim"); s.add_argument("lane"); s.add_argument("tokens", nargs="*")
    s = sub.add_parser("land"); s.add_argument("lane"); s.add_argument("--no-push", action="store_true")
    s.add_argument("--no-wait", action="store_true", help="refuse with exit 6 if the full-gate queue is busy")
    s = sub.add_parser("close"); s.add_argument("lane"); s.add_argument("--remove", action="store_true")
    s.add_argument("--force", action="store_true")
    s = sub.add_parser("path"); s.add_argument("lane"); s.add_argument("--wsl", action="store_true")
    s = sub.add_parser("recover"); s.add_argument("lane")
    sub.add_parser("push")
    s = sub.add_parser("gate-mirror"); s.add_argument("--reset", action="store_true")
    a = p.parse_args(argv)
    try:
        if a.cmd not in ("status", "path", "gate-mirror") and not a.session:
            raise Refuse("no session id: $CLAUDE_CODE_SESSION_ID is unset (pass --session)")
        ctx = Ctx(find_main(Path(__file__).resolve().parent), a.session)
        return {"status": cmd_status, "open": cmd_open, "fresh": cmd_fresh, "drop": cmd_drop, "claim": cmd_claim,
                "unclaim": cmd_unclaim, "land": cmd_land, "close": cmd_close,
                "path": cmd_path, "recover": cmd_recover, "push": cmd_push,
                "gate-mirror": cmd_gate_mirror}[a.cmd](ctx, a)
    except Refuse as e:
        print(f"lanes: REFUSED: {e}", file=sys.stderr)
        return e.code


if __name__ == "__main__":
    sys.exit(main())
