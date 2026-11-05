#!/usr/bin/env python3
"""orchestrate.py: keep N lane sessions running, one background Claude session per lane.

    python tools/lanes/orchestrate.py 3              run, keeping 3 lanes active (Ctrl-C stops
                                                     the orchestrator; sessions keep running)
    python tools/lanes/orchestrate.py 3 --only naming     naming lanes only (or: foundation)
    python tools/lanes/orchestrate.py 3 --codex      also keep one Codex lane session (see below)
    python tools/lanes/orchestrate.py max 5          change the number while it runs (0 = drain:
                                                     running sessions finish, none start)
    python tools/lanes/orchestrate.py status         what is running, paused, needs attention
    python tools/lanes/orchestrate.py plan           which lanes it would start next
    python tools/lanes/orchestrate.py pause <lane> [--when-away]   never start it (or: only while
                                                     the owner is away); persists across runs
    python tools/lanes/orchestrate.py resume <lane>
    python tools/lanes/orchestrate.py ack <lane|all> clear attention items
    python tools/lanes/orchestrate.py codex on|off   the persisted Codex-lane setting

Each session is `claude --bg` running `/naming-session <lane>` or `/foundation-session <lane>`
from the main checkout. Watch one with `claude attach lane-<lane>`. A session is finished when it
has closed its lane (`lanes.py close`) and gone idle, or has sat idle 5 minutes without ever
opening it (it found no work); the orchestrator then stops it (the
conversation is kept: `claude attach <id>` reopens it) and starts the next lane with work, until the backlogs run
dry. A session idle for 90 minutes with its lane still open is reported as needing attention and keeps
its slot. A foundation lane whose session changed nothing is not restarted this run; naming
waits until the naming backlog changes (a row's id, stage or note, or its workable set).
Attention (the session's last message saved under tmp/lanes-state/attention/, listed by `status`
until `ack`) is raised only for owner questions (an ended session whose last message asks one
or cannot be read),
failed lands (an ended session that left unlanded commits; a failed self-heal land) and stalls
(idle 90 min with its lane open; low disk; a codex lane that stops restarting). Everything
else (finished, no-progress and lost sessions without a question, stale tmp/ scratch) goes to
the one-screen digest tmp/lanes-state/digest.md, rewritten every pass.

Self-heal, every pass: (1) `lanes.py tidy` every 10 min retries what `close --remove` left for
a closed lane (an empty ln-<lane> / lands/<lane> folder, a fully landed lanes/<lane> branch;
a locked packed-refs just fails until the next try) and lists anything else for a look in the
digest; (2) a registration whose Claude owner session is absent from 3 good listings and quiet
30 min is closed (`reap_lane`: claims released, branch and worktree kept) unless the lane has
an unacked attention item (its session may wait on the owner's answer; an owner no slot
tracked whose last message asks something raises that once first); (3) a `blocked`
naming row whose note gained an evidence line (` EVIDENCE [<capture>] (<lane> <date>`) since
it was blocked, or whose ` AFTER: <row id | kind:<kind>> [| ...]` marker names a row / a naming
row of that kind landed since, is moved to `idea` by a detached `heal.py land`, started once per main head
while any row is blocked: it scans first and opens its own lane `heal` only if a row moves
(Lane-Replay commit, normal land). Lane name `heal` is reserved for it (`lanes.py open`
refuses other sessions). No second run starts while one lives (one past 3 h is raised once);
a failure is raised once per distinct (step, rc, rows) until a run succeeds.

Picking work: every start names its row (`/naming-session <lane> --row <id>`), one the session
can start now: no live lane holds `topic:row:<id>` or (foundation) overlaps the row's `claims`,
(naming `reviewed`) no live lane holds a TU its draft edits, and every row its note says it
`lands after` / `Order: after` / `depends on` is landed or dropped. No row is given to two
sessions. Naming lanes (naming-a, naming-b, ...) start one per startable row of
docs/naming/phase3/backlog.tsv (idea/researched/reviewed; reviewed first), less the naming
sessions not yet on a row; foundation lanes in docs/foundation/backlog.tsv order, each with its
first startable `active` row, else its first startable open row (not landed/dropped/blocked;
blocked = waits on an owner ruling). Rows held only by claims keep it polling. Naming and foundation
alternate, the kind with fewer running sessions first. Lanes owned in the registry by any live session are skipped, and so are paused lanes
(tmp/lanes-state/paused.json; `naming` pauses every naming lane; a `--when-away` lane starts only
after 20 min without keyboard/mouse input). `researching` rows wait on a Codex job: they count as
workable once their note cites a Codex bridge run id and every cited run has finished (any
status). `max 0` drains: running sessions finish, then the orchestrator exits.

Codex lane (`--codex` / `codex on`): one extra session `lane-codex`, outside the N slots, that
follows tmp/codex/fake-lane-brief.md. It is restarted when it finishes only while Codex usage is
under 95% (5 h) / 98% (week), the bridge is not BLOCKED, and its last two sessions did not both
land nothing (a main commit whose body has a line starting `Codex run:`); `codex on` re-arms
that. An untracked `lane-<x>` session (from before a restart) is adopted, and keeps lane x busy,
only while it works or, idle, still owns lane x in the registry. With nothing running it keeps polling only while work
waits for the owner to be away, naming waits on a backlog change, or the codex lane waits on
usage; attention: attention/index.json.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
import os
import re
import shutil
import subprocess
import sys
import tempfile
import time
from pathlib import Path

import lanes as lanes_mod
from lanes import (HEARTBEAT_STALE_SECS, Refuse, claims_overlap, gate_pid_alive, pid_namespace,
                   split_token)

# A gate ticket hides a session's stall only this long after it was enqueued: a hung land keeps
# its heartbeat fresh forever, and the lanes queued behind it would never be reported.
QUEUED_LAND_MAX_SECS = 4 * 3600

MAIN = Path(__file__).resolve().parents[2]
# Orchestrator-owned files; BB2_LANES_STATE_DIR moves them (tests). The registry stays lanes.py's.
STATE_DIR = Path(os.environ.get("BB2_LANES_STATE_DIR") or MAIN / "tmp" / "lanes-state")
STATE = STATE_DIR / "orchestrator.json"
MAX_FILE = STATE_DIR / "orchestrator-max"
PAUSED = STATE_DIR / "paused.json"
CODEX_FILE = STATE_DIR / "codex.json"
ATTENTION_DIR = STATE_DIR / "attention"
REGISTRY = MAIN / "tmp" / "lanes-state" / "registry.json"
NAMING_BACKLOG = MAIN / "docs" / "naming" / "phase3" / "backlog.tsv"
FOUNDATION_BACKLOG = MAIN / "docs" / "foundation" / "backlog.tsv"
NAMING_WORKABLE = {"idea", "researched", "reviewed"}
CODEX_RUNS = Path.home() / ".claude" / "codex-bridge" / "runs"
CODEX_RUN_ID = re.compile(r"\b(\d{8}-\d{6}-[A-Za-z0-9][\w.-]*)")  # ids may end in `-`
FOUNDATION_DONE = {"landed", "dropped", "blocked"}  # blocked = waits on an owner ruling
ENDED_STATES = {"exited", "stopped", "failed", "killed", "error"}
STALL_SECS = 90 * 60
NO_OPEN_GRACE_SECS = 5 * 60  # idle this long without ever opening its lane = finished
POLL_SECS = 60
MISS_TICKS = 3      # a session absent from this many good listings in a row has ended
CMD_TIMEOUT = 120
AWAY_SECS = 20 * 60  # owner input idle this long = away (for --when-away lanes)
CODEX_LANE = "codex"
CODEX_PROMPT = "Read tmp/codex/fake-lane-brief.md and follow it exactly."
CODEX_BRIEF = MAIN / "tmp" / "codex" / "fake-lane-brief.md"
CODEX_MAX_5H, CODEX_MAX_WEEK = 95, 98  # owner 2026-10-28: spend all weekly Codex usage
CODEX_RECHECK_SECS = 10 * 60  # how often a quota-blocked Codex lane re-asks the bridge
CODEX_HISTORY = 20


def _stale_secs() -> int:
    cfg = Path(os.environ.get("BB2_LANES_CONFIG") or MAIN / "tools" / "lanes" / "lanes.json")
    try:
        return int(float(json.loads(cfg.read_text(encoding="utf-8")).get("stale_hours", 12)) * 3600)
    except (OSError, ValueError):
        return 12 * 3600


STALE_SECS = _stale_secs()  # same rule as lanes.py is_stale


# ---------------------------------------------------------------- inputs

class StateCorrupt(Exception):
    """orchestrator.json exists but does not parse: never read it as empty (that would drop
    every tracked session and the runner pid)."""


def read_json(p: Path, default):
    try:
        return json.loads(p.read_text(encoding="utf-8"))
    except (FileNotFoundError, json.JSONDecodeError, UnicodeDecodeError):
        return default


def _replace(src: str, dst: Path, tries: int = 6):
    """os.replace, retried: on Windows it fails while another process has dst open."""
    for i in range(tries):
        try:
            os.replace(src, dst)
            return
        except PermissionError:
            if i == tries - 1:
                raise
            time.sleep(0.25 * (i + 1))


def write_json(p: Path, data):
    """Atomic write through a uniquely named temp file in the same directory."""
    p.parent.mkdir(parents=True, exist_ok=True)
    fd, tmp = tempfile.mkstemp(prefix=p.name + ".", suffix=".tmp", dir=p.parent)
    try:
        with os.fdopen(fd, "w", encoding="utf-8", newline="\n") as f:
            f.write(json.dumps(data, indent=2, sort_keys=True) + "\n")
        _replace(tmp, p)
    except BaseException:
        try:
            os.unlink(tmp)
        except OSError:
            pass
        raise


def write_state(st: dict):
    """Only the runner writes orchestrator.json; CLI commands use their own files."""
    st.pop("attention", None)
    write_json(STATE, st)


def attention_index() -> Path:
    return ATTENTION_DIR / "index.json"


def load_attention() -> list[dict]:
    items = read_json(attention_index(), [])
    return items if isinstance(items, list) else []


def update_attention(fn):
    """Read-modify-write of the attention list (its own file): fn(list) -> list."""
    items = fn(load_attention())
    write_json(attention_index(), items)
    return items


def tsv_rows(p: Path) -> list[dict]:
    lines = p.read_text(encoding="utf-8").splitlines() if p.exists() else []
    if not lines:
        return []
    head = lines[0].split("\t")
    return [dict(zip(head, ln.split("\t"))) for ln in lines[1:] if ln.strip()]


def backlogs() -> dict:
    return {"naming": tsv_rows(NAMING_BACKLOG), "foundation": tsv_rows(FOUNDATION_BACKLOG)}


def naming_track_rows(bl: dict) -> list[dict]:
    """Naming rows a naming lane can work: the Codex lane's audit index (kind `audit`) is not one,
    so its note edits neither lift a no-progress hold nor count as a session's progress."""
    return [r for r in bl["naming"] if r.get("kind") != "audit"]


def progress_key(bl: dict, lane: str) -> str:
    """A digest of the backlog rows a lane works from; unchanged after a session = no progress."""
    rows = naming_track_rows(bl) if lane.startswith("naming") else \
        [r for r in bl["foundation"] if r.get("lane") == lane]
    blob = "\n".join(f"{r.get('id')}\t{r.get('stage')}\t{r.get('note')}" for r in rows)
    return hashlib.sha1(blob.encode()).hexdigest()


def claude_exe() -> str:
    exe = shutil.which("claude")
    if not exe:
        sys.exit("orchestrate: `claude` is not on PATH")
    return exe


def sh(cmd: list, timeout: int = CMD_TIMEOUT, cwd=None, merge: bool = True) -> tuple[int | None, str]:
    """Run a command, output through a temp file (a `claude --bg` child may keep inherited pipe
    handles open, so pipes could never reach EOF). rc None = timed out. merge=False drops stderr."""
    with tempfile.TemporaryFile() as out, tempfile.TemporaryFile() as err:
        try:
            rc = subprocess.run(cmd, cwd=cwd, stdin=subprocess.DEVNULL, stdout=out,
                                stderr=subprocess.STDOUT if merge else err,
                                timeout=timeout).returncode
        except subprocess.TimeoutExpired:
            rc = None
        out.seek(0)
        return rc, out.read().decode("utf-8", errors="replace")


def agents() -> dict | None:
    """Background sessions by short id; None if the listing failed (never read as "all gone")."""
    _ALIVE["ok"] = False
    rc, out = sh([claude_exe(), "agents", "--json", "--all"], merge=False)
    if rc != 0:
        return None
    rows = None
    for m in re.finditer(r"\[", out):  # the first '[' that starts a JSON list (skips noise)
        try:
            rows, _ = json.JSONDecoder().raw_decode(out, m.start())
        except json.JSONDecodeError:
            continue
        if isinstance(rows, list) and all(isinstance(a, dict) for a in rows):
            break
        rows = None
    if rows is None:
        return None
    _ALIVE.update(ok=True, sessions={a["sessionId"] for a in rows
                                     if a.get("sessionId") and not is_ended(a)})
    return {a["id"]: a for a in rows if a.get("kind") == "background" and a.get("id")}


def idle_secs() -> float | None:
    """Seconds since the owner's last keyboard/mouse input (Windows); None if unknown."""
    if os.name != "nt":
        return None
    try:
        import ctypes
        from ctypes import wintypes

        class LASTINPUTINFO(ctypes.Structure):
            _fields_ = [("cbSize", wintypes.UINT), ("dwTime", wintypes.DWORD)]
        lii = LASTINPUTINFO()
        lii.cbSize = ctypes.sizeof(LASTINPUTINFO)
        if not ctypes.windll.user32.GetLastInputInfo(ctypes.byref(lii)):
            return None
        ticks = ctypes.windll.kernel32.GetTickCount() & 0xFFFFFFFF
        return ((ticks - lii.dwTime) & 0xFFFFFFFF) / 1000.0
    except (OSError, AttributeError, ValueError):
        return None


def owner_away() -> bool:
    idle = idle_secs()
    return idle is not None and idle >= AWAY_SECS


# ---------------------------------------------------------------- transcripts

def project_slug(cwd: str) -> str:
    """Claude Code's project directory name for a cwd (every non-alphanumeric becomes '-')."""
    return re.sub(r"[^A-Za-z0-9]", "-", cwd)


def projects_root() -> Path:
    return Path(os.environ.get("CLAUDE_CONFIG_DIR") or Path.home() / ".claude") / "projects"


def transcript_path(session: str, cwd: str | None, root: Path | None = None) -> Path | None:
    if not session or not re.fullmatch(r"[0-9A-Za-z-]+", session):
        return None
    root = root or projects_root()
    for c in (cwd, str(MAIN)):
        if c:
            p = root / project_slug(c) / f"{session}.jsonl"
            if p.is_file():
                return p
    try:
        return next(iter(root.glob(f"*/{session}.jsonl")), None)
    except OSError:
        return None


def _message_text(rec: dict) -> str:
    if rec.get("type") != "assistant" or rec.get("isSidechain"):
        return ""
    msg = rec.get("message")
    content = msg.get("content") if isinstance(msg, dict) else None
    if isinstance(content, str):
        return content.strip()
    if isinstance(content, list):
        return "\n\n".join(b.get("text", "").strip() for b in content
                           if isinstance(b, dict) and b.get("type") == "text"
                           and isinstance(b.get("text"), str) and b["text"].strip())
    return ""


def last_assistant_text(path: Path | None, tail_bytes: int = 4 << 20) -> str:
    """The final main-thread assistant text message of a transcript; "" if none/unreadable."""
    if not path:
        return ""
    try:
        size = path.stat().st_size
        for limit in ((tail_bytes, size) if size > tail_bytes else (size,)):
            with open(path, "rb") as f:
                f.seek(max(0, size - limit))
                lines = f.read().decode("utf-8", errors="replace").splitlines()
            for ln in reversed(lines):
                try:
                    rec = json.loads(ln)
                except ValueError:
                    continue
                text = _message_text(rec) if isinstance(rec, dict) else ""
                if text:
                    return text
    except OSError:
        pass
    return ""


DISK_MIN_GB = 5.0           # below this, start no session (owner 2026-10-25, after C: hit 0.4 GB)
TMP_STALE_DAYS = 3          # tmp/ scratch untouched this long is reported (sessions clean their own)
TMP_SCAN_SECS = 60 * 60
# long-lived tooling state under tmp/ (not scratch): lanes, Codex prompts, trace runners, emulator
# build and portable profile (gdbtrace --portable_dir), Grinder log/lock, manual lane, cc1psx workdir
# tool work dirs: ra_solver_work (tools/ra_solver/extract.py), cc1build (tools/build_oracle_cc1.sh),
# scan (engine/diagnose.py, engine/orchestrator.py defaults); p3s0 is 2026-10-07 probe scratch
# (named in an engine/test_engine.py docstring), kept by owner request
TMP_KEEP = {"lanes-state", "codex", "trace-instances", "audit", "grind", "redux-portable",
            "pcsx-redux", "manual", "cc1psx", "decomp_me_corpus",
            "ra_solver_work", "cc1build", "p3s0", "scan"}


def disk_free_gb(path: Path = MAIN) -> float | None:
    try:
        return shutil.disk_usage(path).free / 2**30
    except OSError:
        return None


def stale_tmp_dirs(root: Path, now: float, days: float = TMP_STALE_DAYS) -> list[str]:
    """Top-level tmp/ directories (not tooling state) whose own mtime and every direct child's
    mtime are older than `days`: abandoned scratch. Never follows links; cheap (one level)."""
    out = []
    try:
        entries = sorted(os.scandir(root), key=lambda e: e.name)
    except OSError:
        return out
    for e in entries:
        if e.name in TMP_KEEP or not e.is_dir(follow_symlinks=False):
            continue
        try:
            newest = e.stat(follow_symlinks=False).st_mtime
            with os.scandir(e.path) as kids:
                for k in kids:
                    newest = max(newest, k.stat(follow_symlinks=False).st_mtime)
        except OSError:
            continue
        if now - newest > days * 86400:
            out.append(e.name)
    return out


def note_attention(key: str, reason: str, text: str, now: float) -> dict:
    """An attention item that is not about a session (disk, stale scratch)."""
    out = ATTENTION_DIR / f"{key}.md"
    try:
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_text(f"# {key}: {reason}\n\n{text}\n", encoding="utf-8", newline="\n")
    except OSError as e:
        log(f"could not write {out}: {e}")
    log(f"ATTENTION {key}: {reason}; full text: {out}")
    item = {"lane": key, "sid": "-", "session": "", "time": now, "reason": reason,
            "path": str(out), "summary": reason}
    try:
        update_attention(lambda items: [i for i in items if i.get("lane") != key] + [item])
    except OSError as e:  # a full disk must not turn the guard off
        log(f"could not record attention item {key}: {e}")
    return item


def check_housekeeping(st: dict, now: float) -> bool:
    """Stale-scratch report, then the disk guard. True when disk is too low to start sessions."""
    if now - float(st.get("tmp_scan_at", 0)) >= TMP_SCAN_SECS:
        st["tmp_scan_at"] = now
        # listed in the digest only (not an attention item: no owner question, land or stall)
        st["tmp_stale"] = stale_tmp_dirs(MAIN / "tmp", now)
    free = disk_free_gb()
    if free is not None and free < DISK_MIN_GB:
        first = not st.get("disk_low")
        st["disk_low"] = True
        if first:
            note_attention("disk", f"C: has {free:.1f} GB free (< {DISK_MIN_GB:g} GB)",
                           "No session is started until free space is back above the limit. "
                           "Running sessions continue. Stale tmp/ folders, if any, are in the "
                           "`tmp-scratch` attention item.", now)
        return True
    if st.pop("disk_low", None):
        log(f"disk recovered ({free:.1f} GB free); starting sessions again")
    return False


# ---------------------------------------------------------------- self-heal

TIDY_SECS = 10 * 60          # how often leftover branches / empty lane folders are retried
ZOMBIE_IDLE_SECS = 30 * 60   # an absent owner's transcript (or heartbeat) quiet this long = gone
HEAL_LOG_MAX = 20
ENDED_OWNERS_MAX = 64        # session ids whose end a tracked slot already handled
DIGEST = STATE_DIR / "digest.md"
HEAL_RESULT = STATE_DIR / "heal.json"
HEAL_LANE = "heal"
CLAUDE_SESSION = re.compile(r"[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}")
_ALIVE: dict = {"ok": False, "sessions": set()}  # every live session id (any kind), last listing


def heal_note(st: dict, msg: str, now: float):
    log(f"self-heal: {msg}")
    st["heal_log"] = (st.get("heal_log", []) + [{"t": now, "msg": msg}])[-HEAL_LOG_MAX:]


def lanes_ctx():
    return lanes_mod.Ctx(MAIN, "orchestrator")


def tidy_step(st: dict, now: float):
    """Retry what `close --remove` left behind (a locked packed-refs, a folder Windows held):
    quiet, recorded in the digest; `look` items need the owner and are listed there."""
    if now - float(st.get("tidy_at", 0)) < TIDY_SECS:
        return
    st["tidy_at"] = now
    got = lanes_mod.tidy_leftovers(lanes_ctx())
    for status, what, detail in got:
        if status == "done":
            heal_note(st, f"removed leftover {what}", now)
    st["leftovers"] = [f"{what}: {detail or status}" for status, what, detail in got
                       if status in ("look", "failed")]


def owner_quiet_secs(owner: str, e: dict, now: float) -> float:
    """Seconds since the owner session last wrote its transcript (else since the lane's
    registry heartbeat)."""
    p = transcript_path(owner, None)
    try:
        last = p.stat().st_mtime if p else float(e.get("heartbeat", 0))
    except OSError:
        last = float(e.get("heartbeat", 0))
    return now - last


def zombie_candidates(reg: dict, alive: set[str], tracked: set[str]) -> dict[str, str]:
    """lane -> owner for registrations owned by a Claude session no listing shows alive and no
    tracked slot is running. Non-Claude owners (heal-<pid>, scripts) are never candidates."""
    out = {}
    for lane, e in reg.get("lanes", {}).items():
        owner = e.get("owner")
        if (isinstance(owner, str) and CLAUDE_SESSION.fullmatch(owner)
                and owner not in alive and owner not in tracked):
            out[lane] = owner
    return out


def reap_step(st: dict, reg: dict, now: float):
    """Close registrations whose owner session is gone: absent from MISS_TICKS good listings
    in a row and quiet ZOMBIE_IDLE_SECS. Their claims stop blocking other lanes at once
    (instead of after stale_hours); branch and worktree stay for the next session. A lane with
    an unacked attention item is held: its ended session may be waiting on the owner's answer
    (`claude attach` resumes it), so nothing may take the lane over until `ack`."""
    if not _ALIVE["ok"]:
        return
    tracked = {s.get("session") for s in st["sessions"].values() if s.get("session")}
    cands = zombie_candidates(reg, _ALIVE["sessions"], tracked)
    held = {i.get("lane") for i in load_attention()}
    seen = st.get("zombie", {})
    nxt = {}
    for lane, owner in cands.items():
        prev = seen.get(lane, {}) if seen.get(lane, {}).get("owner") == owner else {}
        n = prev.get("n", 0) + 1
        nxt[lane] = {"owner": owner, "n": n, **({"asked": True} if prev.get("asked") else {})}
        e = reg["lanes"].get(lane) or {}
        if (n < MISS_TICKS or lane in held
                or owner_quiet_secs(owner, e, now) < ZOMBIE_IDLE_SECS):
            continue
        # an owner no slot tracks (started by hand, or before a restart) had no end handling:
        # if its last message asks the owner something, raise that once and hold the lane
        # until `ack` instead of freeing it under a pending question
        if not prev.get("asked") and owner not in st.get("ended_owners", []):
            nxt[lane]["asked"] = True
            if asks_owner(last_assistant_text(transcript_path(owner, None))):
                raise_attention(lane, owner[:8], owner, None,
                                "untracked session ended asking the owner", now)
                continue
        if lanes_mod.reap_lane(lanes_ctx(), lane, owner, "owner session gone"):
            claims = " ".join(e.get("claims") or []) or "no claims"
            heal_note(st, f"closed {lane}: owner {owner[:8]} is gone ({claims} released; "
                          f"branch and worktree kept)", now)
            nxt.pop(lane, None)
    st["zombie"] = nxt


def lane_unlanded(lane: str) -> int | None:
    """Unlanded commits on lanes/<lane> (0 if no branch); None if git failed."""
    try:
        ctx = lanes_ctx()
        br = ctx.branch(lane)
        if not lanes_mod.git_ok("rev-parse", "--verify", "-q", f"refs/heads/{br}", cwd=MAIN):
            return 0
        return len(lanes_mod.unlanded(ctx, lane, br))
    except Exception:  # noqa: BLE001
        return None


HEAL_MAX_SECS = 3 * 3600  # a heal run still alive after this is reported (never killed)


def heal_running(st: dict, now: float | None = None) -> bool:
    """The detached `heal.py land` is still alive. No age cap: a run past HEAL_MAX_SECS (a long
    gate queue, a hang, a reused pid) keeps blocking a second run in ln-heal; heal_finish raises
    it once for the owner instead of starting another run over it."""
    h = st.get("heal")
    return bool(h) and pid_alive(int(h.get("pid", 0)))


def heal_finish(st: dict, now: float):
    """Read the result of a finished `heal.py land`; a failed land is an attention item, raised
    once per distinct failure (step, rc, rows) until a run succeeds, so an acked failure that
    repeats on every main move stays acked."""
    h = st.get("heal")
    if not h:
        return
    if heal_running(st, now):
        if now - float(h.get("started", 0)) > HEAL_MAX_SECS and not h.get("warned"):
            h["warned"] = True
            note_attention("heal", "self-heal run still going after 3 h",
                           f"`tools/lanes/heal.py land` (pid {h.get('pid')}) started "
                           f"{_hm(float(h.get('started', 0)))} and is still alive; no other heal "
                           f"run starts while it lives. Log: {HEAL_RESULT.parent / 'logs' / 'heal.log'}. "
                           f"If it hangs, end that pid; the next tick reads its result.", now)
        return
    st.pop("heal")
    res = read_json(HEAL_RESULT, {})
    if not isinstance(res, dict) or float(res.get("ended") or 0) < float(h["started"]):
        res = {"rc": None, "step": "died without a result", "log": ""}
    if res.get("rc") == 0:
        st.pop("heal_fail_key", None)
        if res.get("step") == "landed":
            heal_note(st, "unblocked " + ", ".join(res.get("rows") or []) + " (landed)", now)
        return
    st["heal_failed_head"] = h.get("head", "")
    owner = ((read_json(REGISTRY, {"lanes": {}}).get("lanes") or {}).get(HEAL_LANE) or {}).get("owner")
    if isinstance(owner, str) and owner.startswith("heal-"):  # it died holding the lane
        lanes_mod.reap_lane(lanes_ctx(), HEAL_LANE, owner, "heal.py ended without closing it")
    key = f"{res.get('step')}|{res.get('rc')}|{','.join(sorted(h.get('rows') or []))}"
    if st.get("heal_fail_key") == key:
        heal_note(st, f"self-heal land failed again at {res.get('step')} (rc {res.get('rc')}; "
                      f"already raised)", now)
        return
    st["heal_fail_key"] = key
    note_attention("heal", f"self-heal land failed at {res.get('step')} (rc {res.get('rc')})",
                   f"`tools/lanes/heal.py land` tried to move {', '.join(h.get('rows') or [])} "
                   f"from blocked to idea. Log: {res.get('log') or '(none)'}. It retries when "
                   f"main moves; lane `{HEAL_LANE}` may hold the unlanded commit "
                   f"(`lanes.py status`).", now)


def unblock_step(st: dict, bl: dict, reg: dict, now: float):
    """Blocked naming rows whose evidence / AFTER: wave has landed: start `heal.py land`
    (detached) once per main head. It opens lane `heal`, commits the backlog flip with a
    Lane-Replay trailer, lands and closes; heal_finish reads its result on a later tick."""
    heal_finish(st, now)
    if st.get("heal") or not any(r.get("stage") == "blocked" for r in bl.get("naming", [])):
        return
    if lane_owned(reg, HEAL_LANE, now):
        return
    head = (git_main("rev-parse", "main") or "").strip()
    if not head or head in (st.get("unblock_head"), st.get("heal_failed_head")):
        return
    st["unblock_head"] = head
    # the history scan (a git show per backlog version) runs in the detached process, which
    # opens the heal lane only if a row moves; the tick never waits on it
    rows = [r["id"] for r in bl.get("naming", []) if r.get("stage") == "blocked"]
    flags = (subprocess.CREATE_NEW_PROCESS_GROUP | subprocess.CREATE_NO_WINDOW) if os.name == "nt" else 0
    p = subprocess.Popen([sys.executable, str(Path(__file__).with_name("heal.py")), "land"],
                         cwd=MAIN, stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL,
                         stderr=subprocess.DEVNULL, creationflags=flags)
    st["heal"] = {"pid": p.pid, "started": now, "rows": rows, "head": head}
    log(f"self-heal: checking blocked {', '.join(rows)} for landed evidence / AFTER: (pid {p.pid})")


def self_heal(st: dict, bl: dict, reg: dict, now: float):
    _guarded("tidying lane leftovers", tidy_step, st, now)
    _guarded("closing zombie registrations", reap_step, st, reg, now)
    _guarded("unblocking naming rows", unblock_step, st, bl, reg, now)


# ---------------------------------------------------------------- attention policy

QUESTION = re.compile(r"\?|\b(?:should I|do you want|if you want|can you|could you|please|"
                      r"your (?:decision|call|ruling|answer|approval)|owner(?:'s)? (?:ruling|"
                      r"decision|question|input|answer)|needs? (?:the owner|an owner|a ruling|"
                      r"you)|ask(?:s|ing)? the owner|waits? (?:on|for) (?:the )?owner|"
                      r"you answer|once you|let me know|tell me|decision needed|needs? a "
                      r"decision|waiting (?:on|for) you|awaiting your|owner action|owner "
                      r"sign-off|until the owner)\b", re.I)


def asks_owner(text: str) -> bool:
    """The session's last message asks the owner something: a question mark or owner-request
    wording anywhere in it (sessions lead with the ask, then status; wide on purpose: a false
    hit costs one `ack`, a miss hides a question)."""
    text = text.strip()
    return bool(text) and bool(QUESTION.search(text))


def recent_end(st: dict, lane: str, sid: str, what: str, text: str, now: float):
    summary = " ".join(text.split())[:140]
    st["recent_ends"] = (st.get("recent_ends", []) + [
        {"t": now, "lane": lane, "sid": sid, "what": what, "summary": summary}])[-8:]


def raise_attention(lane: str, sid: str, session: str, cwd: str | None, reason: str,
                    now: float | None = None) -> dict:
    """Save the session's last message for the owner, log it, and list it in the state."""
    now = now or time.time()
    path = transcript_path(session, cwd)
    text = last_assistant_text(path) or (
        "(no final assistant message found: transcript "
        + (f"{path} has none or is unreadable" if path else "not found") + ")")
    when = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(now))
    out = ATTENTION_DIR / f"{lane}-{sid}.md"
    try:
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_text(f"# {lane}: {reason}\n\n- lane: {lane}\n- session: {sid} ({session or '?'})\n"
                       f"- time: {when}\n- reason: {reason}\n- transcript: {path or '(not found)'}\n"
                       f"- reopen: claude attach {sid}\n\n{text}\n", encoding="utf-8", newline="\n")
    except OSError as e:
        log(f"could not write {out}: {e}")
    summary = " ".join(text.split())[:150]
    log(f"ATTENTION {lane}: {summary}; full text: {out}")
    item = {"lane": lane, "sid": sid, "session": session, "time": now, "reason": reason,
            "path": str(out), "summary": summary}
    update_attention(lambda items: [i for i in items
                                    if (i.get("lane"), i.get("sid")) != (lane, sid)] + [item])
    return item


# ---------------------------------------------------------------- the decision (pure)

def lane_owned(reg: dict, lane: str, now: float) -> bool:
    e = reg.get("lanes", {}).get(lane) or {}
    return bool(e.get("owner")) and now - float(e.get("heartbeat", 0)) <= STALE_SECS


def is_ended(ag: dict | None) -> bool:
    # An exited background session is still listed (state "done") but has no pid.
    return ag is None or ag.get("state") in ENDED_STATES or not ag.get("pid")


def is_idle(ag: dict | None) -> bool:
    return ag is not None and (ag.get("state") == "done" or ag.get("status") == "idle")


def is_codex(slot: dict) -> bool:
    return slot.get("kind") == "codex"


def gate_queue_position(reg: dict, lane: str, now: float, session: str | None = None) -> int | None:
    """Pruned position of a fresh, valid, under-cap ticket for this session, or None."""
    queue = reg.get("gate_queue")
    if not isinstance(queue, list):
        return None
    # Count only tickets lanes.py would keep after pruning; the orchestrator's 4 h
    # stall cap applies to the matching ticket, never to tickets ahead of it.
    position = 0
    for ticket in queue:
        if not isinstance(ticket, dict):
            continue
        beat = ticket.get("beat", ticket.get("enqueued"))
        if (isinstance(beat, bool) or not isinstance(beat, (int, float))
                or not math.isfinite(beat) or now - beat > HEARTBEAT_STALE_SECS):
            continue
        if ticket.get("namespace") == pid_namespace():
            pid = ticket.get("pid")
            if isinstance(pid, bool) or not isinstance(pid, int):
                continue
            try:
                alive = gate_pid_alive(pid)
            except Exception:
                alive = False
            if not alive:
                continue
        position += 1
        if (not isinstance(ticket.get("lane"), str) or not ticket["lane"]
                or ticket["lane"] != lane
                or (isinstance(session, str) and session
                    and ticket.get("session") != session)):
            continue
        enqueued = ticket.get("enqueued")
        if (isinstance(enqueued, bool) or not isinstance(enqueued, (int, float))
                or not math.isfinite(enqueued)
                or now - enqueued >= QUEUED_LAND_MAX_SECS):
            continue
        return position
    return None


RELEASED_GRACE_SECS = 600  # closed its lane this long ago: finished even if not idle


def session_phase(slot: dict, ag: dict | None, reg: dict, now: float) -> str:
    """running | finished | stalled | lost for one tracked session.

    A session has released its lane once it opened the lane (seen owning it, or the lane was
    closed after the session started) and no longer owns it. Finished = released and idle (or
    exited), or released with the lane closed RELEASED_GRACE_SECS ago even if still
    busy. A session idle for NO_OPEN_GRACE_SECS that never opened its lane (it found no
    work) is finished too. An idle session that owns its lane usually just waits on a
    background `lanes.py land`; it is only reported as stalled after STALL_SECS unless
    its lane has a fresh full-gate queue ticket enqueued under QUEUED_LAND_MAX_SECS ago."""
    e = reg.get("lanes", {}).get(slot["lane"]) or {}
    owns = bool(slot.get("session")) and e.get("owner") == slot["session"]
    released = not owns and (slot.get("seen_open") or float(e.get("closed") or 0) >= slot["started"])
    if is_ended(ag):
        return "finished" if released else "lost"
    closed = float(e.get("closed") or 0)
    if released and closed and now - closed >= RELEASED_GRACE_SECS:
        # its work is done; a leftover background agent can keep a session busy for hours
        return "finished"
    if is_idle(ag):
        if released:
            return "finished"
        idle_for = now - slot.get("idle_since", now)
        if (slot.get("session") and not owns and not slot.get("seen_open")
                and idle_for >= NO_OPEN_GRACE_SECS):
            return "finished"  # decided there was no work and never opened its lane
        if (idle_for >= STALL_SECS
                and gate_queue_position(reg, slot["lane"], now, slot.get("session")) is None):
            return "stalled"
    return "running"


def pause_blocks(paused: dict, lane: str, away: bool) -> bool:
    """A lane is held if it (or its group, `naming` for naming-*) is paused; a when-away pause
    holds it only while the owner is present."""
    for name in {lane, "naming" if lane.startswith("naming") else lane}:
        p = paused.get(name)
        if p is not None and not (p.get("when_away") and away):
            return True
    return False


def live_lanes(ag: dict, reg: dict) -> dict[str, str]:
    """lane -> short id of every background session named `lane-<lane>` that is still at work:
    not ended, and either not idle or idle while it owns its lane in the registry (waiting on a
    land). An idle session keeps its process long after it finished; that one does not count."""
    out = {}
    for sid, a in sorted(ag.items()):
        name = a.get("name") or ""
        if not name.startswith("lane-") or len(name) <= 5 or is_ended(a):
            continue
        lane = name[5:]
        if is_idle(a):
            owner = (reg.get("lanes", {}).get(lane) or {}).get("owner")
            if not owner or owner != a.get("sessionId"):
                continue
        out.setdefault(lane, sid)
    return out


def naming_row_workable(r: dict) -> bool:
    """A naming row a session can take now. A `researching` row waits on its Codex run; once
    every bridge run its note cites (by run id, at least one found) has finished, it is
    workable: the session harvests the result and moves the row on."""
    stage = r.get("stage")
    if stage in NAMING_WORKABLE:
        return True
    if stage != "researching":
        return False
    found = False
    for rid in CODEX_RUN_ID.findall(r.get("note") or ""):
        meta = CODEX_RUNS / rid.rstrip(".") / "meta.json"  # a sentence's full stop
        if not meta.is_file():
            continue
        try:
            m = json.loads(meta.read_text(encoding="utf-8"))
        except (OSError, ValueError):
            return False
        if not m.get("finished"):
            return False
        found = True
    return found


def draft_tu_claims(r: dict) -> set[str]:
    """`topic:tu:` tokens for every game TU a row's tracked draft dir edits (lanes.draft_tus:
    dry.txt's edit list, else DECL sites / per-file counts, else every TU it names). Empty when
    the row has no draft yet."""
    return {f"topic:tu:{t}" for t in lanes_mod.draft_tus(MAIN, r.get("draft") or "")}


def draft_claims(r: dict) -> set[str]:
    """The claims a row's LAND takes (`lanes.py claim --wave <draft>`): a pairs.txt draft's
    `topic:sym:` claims plus shared `topic:tuedit:` TUs, else exclusive `topic:tu:` TUs."""
    try:
        return lanes_mod.wave_claims(MAIN, r.get("draft") or "")
    except Refuse:  # a malformed pairs.txt: fall back to whole-TU claims (its LAND will refuse)
        return draft_tu_claims(r)


def tu_basenames() -> dict[str, str]:
    """`4510C` -> `main/4510C` for every game/library TU whose file name is unique under src/."""
    return lanes_mod.tu_basenames(MAIN)


def held_claims(reg: dict, now: float) -> dict[str, str]:
    """Every claim token held by a live (owned, fresh) lane -> that lane."""
    out = {}
    for lane, e in reg.get("lanes", {}).items():
        if lane_owned(reg, lane, now):
            for tok in e.get("claims") or []:
                out.setdefault(tok, lane)
    return out


def startable_naming_rows(bl: dict, reg: dict, now: float,
                          held: dict[str, str] | None = None) -> list[dict]:
    """Workable naming rows a NEW session could take now: no live lane claims the row, and a
    `reviewed` row's draft claims (symbols, TUs) are not held by another lane (its LAND would be
    refused). Counting
    these instead of every workable row stops sessions that start, find every row blocked by
    another lane's claims, and quit."""
    held = held_claims(reg, now) if held is None else held
    idx = row_index(bl)
    out = []  # member pairs (`Type.member`) claim topic:sym like any pairs.txt draft
    for r in bl["naming"]:
        if not naming_row_workable(r) or f"topic:row:{r.get('id')}" in held:
            continue
        if r.get("stage") == "reviewed" and claims_held(_tokens(" ".join(draft_claims(r))),
                                                        held) is not None:
            continue
        if not deps_met(r, idx):
            continue
        out.append(r)
    # the skill's pick order: LAND (reviewed) first, then researched / researching, then idea
    rank = {"reviewed": 0, "researched": 1, "researching": 1}
    return sorted(out, key=lambda r: rank.get(r.get("stage"), 2))


# `lands after X` / `Order: after X` / `depends on X` in a note: the row starts only once row X
# (any backlog) is landed or dropped. A word that names no row is ignored.
DEPENDS = re.compile(r"(?i)\b(?:lands after|order: after|depends on)\s+`?([a-z][\w.-]*[a-z0-9])")
DEP_DONE = {"landed", "dropped"}


def row_index(bl: dict) -> dict[str, dict]:
    out: dict[str, dict] = {}
    for kind in ("naming", "foundation"):
        for r in bl.get(kind, []):
            out.setdefault(str(r.get("id")), r)
    return out


def deps_met(r: dict, idx: dict[str, dict]) -> bool:
    for dep in DEPENDS.findall(r.get("note") or ""):
        d = idx.get(dep)
        if d is not None and d is not r and d.get("stage") not in DEP_DONE:
            return False
    return True


def _tokens(text: str) -> list[tuple[str, str]]:
    out = []
    for tok in (text or "").split():
        try:
            kind, val, _ = split_token(tok.strip("[]"))
        except Refuse:
            continue
        out.append((kind, val.lower() if kind == "path" else val))
    return out


def claims_held(tokens: list[tuple[str, str]], held: dict[str, str]) -> str | None:
    """The lane holding a claim that overlaps one of `tokens` (as `lanes.py claim` would refuse:
    overlapping topics (lanes.topics_overlap) or path globs), else None."""
    for htok, lane in held.items():
        try:
            hk, hv, _ = split_token(htok)
        except Refuse:
            continue
        hv = hv.lower() if hk == "path" else hv
        for k, v in tokens:
            if k == hk and claims_overlap(k, v, hv):
                return lane
    return None


def row_claims(r: dict) -> set[str]:
    """The claim tokens a session working row `r` will take: its row, plus a foundation row's
    `claims` column or a `reviewed` naming row's draft claims (its LAND)."""
    out = {f"topic:row:{r.get('id')}"}
    if "lane" in r:
        out.update(f"{k}:{v}" for k, v in _tokens(r.get("claims") or ""))
    elif r.get("stage") == "reviewed":
        out |= draft_claims(r)
    return out


def reserve(held: dict[str, str], lane: str, r: dict):
    """Count row `r`'s claims as held (by `lane`) while planning, before its session claims."""
    for tok in row_claims(r):
        held.setdefault(tok, lane)


def foundation_row_startable(r: dict, held: dict[str, str], idx: dict[str, dict]) -> bool:
    if r.get("stage") in FOUNDATION_DONE or not deps_met(r, idx):
        return False
    toks = _tokens(r.get("claims") or "") + [("topic", f"row:{r.get('id')}")]
    return claims_held(toks, held) is None


def foundation_lane_row(bl: dict, lane: str, held: dict[str, str],
                        idx: dict[str, dict] | None = None) -> str | None:
    """The row a new session on `lane` takes: its first startable `active` row, else its first
    startable other open row (backlog order). None when every open row waits on a claim or a
    dependency."""
    idx = row_index(bl) if idx is None else idx
    rows = [r for r in bl["foundation"] if r.get("lane") == lane]
    for want_active in (True, False):
        for r in rows:
            if (r.get("stage") == "active") == want_active and foundation_row_startable(r, held,
                                                                                     idx):
                return str(r.get("id"))
    return None


def claim_blocked_foundation(bl: dict, reg: dict, now: float) -> list[str]:
    """Foundation lanes with open rows none of which can start now because of live claims."""
    held = held_claims(reg, now)
    idx = row_index(bl)
    out = []
    for r in bl["foundation"]:
        lane = r.get("lane", "")
        if (lane and lane not in out and r.get("stage") not in FOUNDATION_DONE
                and not lane_owned(reg, lane, now) and foundation_lane_row(bl, lane, held, idx) is None
                and any(foundation_row_startable(x, {}, idx) for x in bl["foundation"]
                        if x.get("lane") == lane)):
            out.append(lane)
    return out


def claim_blocked_naming_ids(bl: dict, reg: dict, now: float) -> list[str]:
    """Workable rows no new session can take because of live claims. They clear when the holder
    lands or its claims go stale (STALE_SECS), so the orchestrator keeps polling for them."""
    idx = row_index(bl)  # a row waiting on a dependency is not claim-blocked
    return sorted({i for i in workable_naming_ids(bl) if deps_met(idx[i], idx)}
                  - {r["id"] for r in startable_naming_rows(bl, reg, now)})


def workable_naming_ids(bl: dict) -> list[str]:
    """Stable snapshot of row identities, independent of backlog order."""
    return sorted({r["id"] for r in bl["naming"] if naming_row_workable(r)})


def naming_hold(bl: dict) -> dict:
    """What a naming no-progress hold waits on: the workable row ids (a Codex run finishing
    changes them without a backlog edit) and a digest of every naming row's id / stage / note,
    row order ignored, so a note-only edit (PROMPT READY) or a stage move between workable
    stages lifts it too."""
    rows = sorted((str(r.get("id")), str(r.get("stage")), str(r.get("note")))
                  for r in naming_track_rows(bl))
    blob = "\n".join("\t".join(t) for t in rows)
    return {"key": hashlib.sha1(blob.encode()).hexdigest(),
            "workable": workable_naming_ids(bl)}


def candidates(bl: dict, reg: dict, st: dict, now: float, only: str | None,
               paused: dict | None = None, away: bool = False,
               live: set | frozenset = frozenset()) -> list[str]:
    """Lanes to start next, best first (see plan_starts)."""
    return [lane for lane, _ in plan_starts(bl, reg, st, now, only, paused, away, live)]


def plan_starts(bl: dict, reg: dict, st: dict, now: float, only: str | None,
                paused: dict | None = None, away: bool = False,
                live: set | frozenset = frozenset()) -> list[tuple[str, str | None]]:
    """(lane, row id) pairs to start next, best first (naming and foundation alternate). Every
    start names the row the session works: a startable row (no live lane holds its row or the
    claims its work needs, its dependencies landed) that no running session was given. `live`:
    lanes with a running `lane-<lane>` session, tracked or not, which count as busy."""
    paused = paused or {}
    busy = {s["lane"] for s in st["sessions"].values()} | set(live)
    given = {s["lane"]: s["row"] for s in st["sessions"].values() if s.get("row")}
    skip = set(st["no_progress"])
    hold = naming_hold(bl)
    workable_ids = hold["workable"]
    if ("no_progress_naming" in st
            and st["no_progress_naming"] != hold):
        del st["no_progress_naming"]
    held = held_claims(reg, now)
    idx = row_index(bl)
    # Rows already given to a running session (not yet claimed) and rows planned in this pass
    # reserve the claims their work will take, so two starts never head for one TU or path.
    for lane, row in given.items():
        if row in idx:
            reserve(held, lane, idx[row])
    naming: list[tuple[str, str | None]] = []
    if only in (None, "naming") and "no_progress_naming" not in st:
        rows, taken = [], dict(held)
        for r in startable_naming_rows(bl, reg, now, held):
            if (str(r.get("id")) in given.values()
                    or claims_held(_tokens(" ".join(row_claims(r))), taken) is not None):
                continue
            rows.append(r)
            reserve(taken, "", r)
        owned = {l for l in reg.get("lanes", {}) if lane_owned(reg, l, now)}
        on_row = {f"topic:row:{i}" for i in workable_ids}
        held_rows = {lane for tok, lane in held.items() if tok in on_row}
        # naming lanes on a row (claimed, or given at start) are excluded with that row; a lane
        # with neither (an adopted pre-dispatch session, or one between rows) may pick any row
        active = sum(1 for lane in busy | owned
                     if lane.startswith("naming") and lane not in held_rows and lane not in given)
        free = [f"naming-{c}" for c in "abcdefghijklmnopqrstuvwxyz"
                if f"naming-{c}" not in busy and not lane_owned(reg, f"naming-{c}", now)
                and not pause_blocks(paused, f"naming-{c}", away)]
        rows = rows[:max(0, len(rows) - active)]
        naming = [(lane, str(r.get("id"))) for lane, r in zip(free, rows)]
        for lane, r in zip(free, rows):
            reserve(held, lane, r)
    found: list[tuple[str, str | None]] = []
    if only in (None, "foundation"):
        seen: set[str] = set()
        for r in bl["foundation"]:
            lane = r.get("lane", "")
            if (lane and r.get("stage") not in FOUNDATION_DONE and lane not in seen
                    and lane not in busy and lane not in skip and not lane_owned(reg, lane, now)
                    and not pause_blocks(paused, lane, away)):
                seen.add(lane)
                row = foundation_lane_row(bl, lane, held, idx)
                if row is not None:
                    found.append((lane, row))
                    reserve(held, lane, idx[row])
    # The kind with fewer running sessions goes first, so a freed slot does not always go back
    # to naming while foundation lanes wait (ties: naming first).
    lanes_running = [s["lane"] for s in st["sessions"].values() if not is_codex(s)]
    lanes_running += [l for l in live if l not in lanes_running and l != CODEX_LANE]
    n_naming = sum(1 for l in lanes_running if l.startswith("naming"))
    first, second = ((found, naming) if len(lanes_running) - n_naming < n_naming
                     else (naming, found))
    out: list[tuple[str, str | None]] = []
    for i in range(max(len(naming), len(found))):
        out += first[i:i + 1] + second[i:i + 1]
    return out


def update_slots(st: dict, ag: dict, reg: dict, bl: dict, now: float) -> list[tuple[str, str]]:
    """Advance every tracked session; returns (event, short id) pairs. Mutates st. `ag` is a
    successful listing; a session missing from it counts as ended only after MISS_TICKS."""
    events = []
    for sid, slot in list(st["sessions"].items()):
        a = ag.get(sid)
        if a is None:
            slot["missing"] = slot.get("missing", 0) + 1
            if slot["missing"] < MISS_TICKS:
                continue
        else:
            slot.pop("missing", None)
            if not slot.get("session") and a.get("sessionId"):
                slot["session"] = a["sessionId"]
        e = reg.get("lanes", {}).get(slot["lane"]) or {}
        if slot.get("session") and e.get("owner") == slot["session"]:
            slot["seen_open"] = True
        if is_idle(a):
            slot.setdefault("idle_since", now)
        else:
            slot.pop("idle_since", None)
        phase = session_phase(slot, a, reg, now)
        if phase in ("finished", "lost"):
            del st["sessions"][sid]
            if slot.get("session"):  # its end was handled here: reap_step won't re-ask
                st["ended_owners"] = (st.get("ended_owners", []) + [slot["session"]])[-ENDED_OWNERS_MAX:]
            group = "naming" if slot["lane"].startswith("naming") else slot["lane"]
            if (phase == "finished" and not is_codex(slot)
                    and progress_key(bl, slot["lane"]) == slot.get("progress_at_start")):
                if group == "naming":
                    st["no_progress_naming"] = naming_hold(bl)
                else:
                    st["no_progress"] = sorted(set(st["no_progress"]) | {group})
                phase = "no-progress"
            events.append((phase, sid))
        elif phase == "stalled" and not slot.get("warned"):
            slot["warned"] = True
            events.append(("stalled", sid))
    return events


def codex_landed(commits: list[tuple[str, str]]) -> list[str]:
    """Subjects of the (subject, body) main commits that are the Codex lane's own landings:
    a body with a line starting `Codex run:` (the codex-worker skill requires it). Subjects
    and bodies that merely mention the Codex lane (the owner's tooling commits) do not count."""
    return [s for s, b in commits
            if any(re.match(r"Codex run: \S+", ln) for ln in b.splitlines())]


def codex_gate(cfg: dict, paused: dict, away: bool, history: list[dict]) -> str | None:
    """Why the Codex lane must not (re)start, from local facts; None = ask the bridge."""
    if not cfg.get("enabled"):
        return "codex lane is off"
    if pause_blocks(paused, CODEX_LANE, away):
        return "codex lane is paused" + (" until the owner is away"
                                         if paused.get(CODEX_LANE, {}).get("when_away") else "")
    recent = [h for h in history if h.get("ended", 0) >= cfg.get("armed", 0)][-2:]
    if len(recent) == 2 and not any(h.get("landed") for h in recent):
        return ("its last two sessions landed nothing; `orchestrate.py codex on` re-arms it")
    return None


USAGE_BLOCKER = re.compile(r"window at \d+%|limit reached|ordinary usage is not allowed")


def codex_usage_block(bridge: dict | None) -> bool:
    """True when Codex is held only by usage (a window percentage or a reported limit), which
    clears by itself; False for a missing, failing or otherwise BLOCKED bridge."""
    if not isinstance(bridge, dict) or not isinstance(bridge.get("usage"), dict):
        return False
    u = bridge["usage"]
    p5 = (u.get("primary") or {}).get("usedPercent")
    pw = (u.get("secondary") or {}).get("usedPercent")
    if p5 is None or pw is None:
        return False
    if bridge.get("clear", False):
        return p5 >= CODEX_MAX_5H or pw >= CODEX_MAX_WEEK
    blockers = bridge.get("blockers") or []
    return bool(blockers) and all(USAGE_BLOCKER.search(b) for b in blockers)


def codex_quota_block(bridge: dict | None) -> str | None:
    """Why Codex usage forbids a new session (codex_bridge.py status --json); None = clear."""
    if not isinstance(bridge, dict):
        return "codex bridge status unavailable"
    if not bridge.get("clear", False):
        return "codex bridge BLOCKED: " + ("; ".join(bridge.get("blockers") or []) or "?")
    u = bridge.get("usage") or {}
    p5 = (u.get("primary") or {}).get("usedPercent")
    pw = (u.get("secondary") or {}).get("usedPercent")
    if p5 is None or pw is None:
        return "codex usage unknown"
    if p5 >= CODEX_MAX_5H:
        return f"codex 5-hour usage {p5}% (>= {CODEX_MAX_5H}%)"
    if pw >= CODEX_MAX_WEEK:
        return f"codex weekly usage {pw}% (>= {CODEX_MAX_WEEK}%)"
    return None


# ---------------------------------------------------------------- actions

def log(msg: str):
    print(time.strftime("%H:%M:%S ") + msg, flush=True)


def start_bg(name: str, prompt: str, args) -> str | None:
    cmd = [claude_exe(), "--bg", "-n", name, "--permission-mode", args.permission_mode]
    if args.model:
        cmd += ["--model", args.model]
    cmd.append(prompt)
    rc, out = sh(cmd, cwd=MAIN)
    m = re.search(r"backgrounded\W+([0-9a-f]{8})\b", out)
    if not m:
        log(f"could not start {name}: rc={rc} {out.strip()[:300]}")
        return None
    return m.group(1)


def session_prompt(lane: str, row: str | None) -> str:
    skill = "naming-session" if lane.startswith("naming") else "foundation-session"
    return f"/{skill} {lane}" + (f" --row {row}" if row else "")


def start_session(lane: str, args, row: str | None = None) -> tuple[str, dict] | None:
    started = time.time()
    prompt = session_prompt(lane, row)
    sid = start_bg(f"lane-{lane}", prompt, args)
    if not sid:
        return None
    log(f"started {lane} ({prompt}) as {sid}; watch it: claude attach {sid}")
    # The full sessionId (the registry owner) is filled in from the next listing.
    return sid, {"lane": lane, "session": "", "started": started, "row": row}


def stop_session(sid: str) -> bool:
    rc, out = sh([claude_exe(), "stop", sid])
    if rc != 0:
        log(f"`claude stop {sid}` failed (rc={rc}): {out.strip()[:200]}")
    return rc == 0


def git_main(*a) -> str | None:
    rc, out = sh(["git", "-C", str(MAIN), "-c", "i18n.logOutputEncoding=utf-8", *a],
                 timeout=60, merge=False)
    return out if rc == 0 else None


def parse_log(out: str, since_ts: float | None = None) -> list[tuple[str, str]]:
    """Records of `git log --format=%ct%x1f%s%x1f%b%x1e` -> (subject, body), optionally only
    those committed at or after since_ts."""
    commits = []
    for rec in out.split("\x1e"):
        parts = rec.strip("\r\n").split("\x1f")
        if len(parts) != 3:
            continue
        ct, subj, body = parts
        if since_ts is None or (ct.strip().isdigit() and int(ct) >= since_ts):
            commits.append((subj.strip(), body))
    return commits


def main_commits(since_sha: str, since_ts: float) -> list[tuple[str, str]] | None:
    """(subject, body) of commits main gained since the session started; None if git failed."""
    fmt = "--format=%ct%x1f%s%x1f%b%x1e"
    if since_sha:
        out = git_main("log", fmt, f"{since_sha}..main")
        if out is not None:
            return parse_log(out)
    out = git_main("log", "main", "-n", "500", fmt)
    return None if out is None else parse_log(out, since_ts)


def bridge_status() -> dict | None:
    home = Path(os.environ.get("USERPROFILE") or Path.home())
    script = home / ".claude" / "skills" / "codex" / "codex_bridge.py"
    rc, out = sh([sys.executable, str(script), "status", "--json"], merge=False)
    try:
        return json.loads(out[out.index("{"):]) if rc == 0 else None
    except ValueError:
        return None


# ---------------------------------------------------------------- state

def load_state() -> dict:
    """The runner's state; {} only when the file is absent. Raises StateCorrupt if it exists
    but does not parse."""
    try:
        st = json.loads(STATE.read_text(encoding="utf-8"))
    except FileNotFoundError:
        st = {}
    except (OSError, ValueError) as e:
        raise StateCorrupt(f"{STATE}: {e}") from e
    if not isinstance(st, dict):
        raise StateCorrupt(f"{STATE}: not a JSON object")
    st.setdefault("sessions", {})
    st.setdefault("no_progress", [])
    if "naming" in st["no_progress"]:
        st["no_progress_naming"] = naming_hold(backlogs())
        st["no_progress"] = [lane for lane in st["no_progress"] if lane != "naming"]
    st.setdefault("codex_history", [])
    st.pop("attention", None)  # lives in attention/index.json now
    return st


def read_max() -> int:
    try:
        return int(MAX_FILE.read_text(encoding="utf-8").strip())
    except (FileNotFoundError, ValueError):
        return 0


def write_max(n: int):
    MAX_FILE.parent.mkdir(parents=True, exist_ok=True)
    fd, tmp = tempfile.mkstemp(dir=str(MAX_FILE.parent), prefix=MAX_FILE.name + ".", suffix=".tmp")
    with os.fdopen(fd, "w", encoding="utf-8", newline="\n") as f:
        f.write(f"{n}\n")
    _replace(tmp, MAX_FILE)


def load_paused() -> dict:
    p = read_json(PAUSED, {})
    return p if isinstance(p, dict) else {}


def read_codex() -> dict:
    c = read_json(CODEX_FILE, {})
    return c if isinstance(c, dict) else {}


def set_codex(on: bool) -> dict:
    """Turn the Codex lane on/off; turning it on re-arms the two-strike rule."""
    c = read_codex()
    if on:
        c["armed"] = time.time()
    c["enabled"] = on
    write_json(CODEX_FILE, c)
    return c


def pid_alive(pid: int) -> bool:
    # os.kill(pid, 0) terminates the process on Windows; ask tasklist instead.
    # Windows reuses pids, so also require a python image.
    rc, out = sh(["tasklist", "/FI", f"PID eq {pid}", "/NH", "/FO", "CSV"], timeout=30)
    return any(f'"{pid}"' in ln and ln.lower().startswith(('"python', '"py.exe'))
               for ln in out.splitlines())


# ---------------------------------------------------------------- codex lane

def adopt_sessions(st: dict, ag: dict, bl: dict, reg: dict):
    """Track every live `lane-<x>` session the state does not know: one started before a
    restart, or one whose start was never written (write_state failed). Otherwise the next
    tick could start lane x a second time before the first session opens it. A finished
    (idle, lane not owned) session is not adopted: see live_lanes."""
    tracked_lanes = {s["lane"] for s in st["sessions"].values()}
    known_codex = {h.get("sid") for h in st["codex_history"]}
    for lane, sid in live_lanes(ag, reg).items():
        if sid in st["sessions"] or lane in tracked_lanes:
            continue
        a = ag[sid]
        started = float(a.get("startedAt") or 0) / 1000 or time.time()
        slot = {"lane": lane, "session": a.get("sessionId", ""), "started": started,
                "adopted": True}
        if lane == CODEX_LANE:
            if sid in known_codex:  # already recorded as ended (its stop failed)
                continue
            slot.update(kind="codex", main_at_start="")
        else:
            slot["progress_at_start"] = progress_key(bl, lane)
        st["sessions"][sid] = slot
        tracked_lanes.add(lane)
        log(f"adopted running {lane} session {sid}; watch it: claude attach {sid}")


def codex_ended(st: dict, sid: str, slot: dict, ev: str, a: dict, now: float):
    commits = main_commits(slot.get("main_at_start", ""), slot["started"])
    landed = codex_landed(commits or [])
    st["codex_history"] = (st["codex_history"] + [{
        "sid": sid, "session": slot.get("session", ""), "started": slot["started"], "ended": now,
        "event": ev, "landed": landed, "git_ok": commits is not None}])[-CODEX_HISTORY:]
    if landed:
        log(f"codex lane session {sid} landed {len(landed)}: " + "; ".join(landed)[:300])
        return
    why = "could not read main's git log" if commits is None else "landed nothing on main"
    reason = f"codex lane session {why}" + (" (ended with its lane open)" if ev == "lost" else "")
    block = codex_gate(read_codex(), {}, False, st["codex_history"])
    if block:
        reason += f"; not restarting: {block}"
    log(f"codex: {reason}")
    session = slot.get("session") or a.get("sessionId", "")
    text = last_assistant_text(transcript_path(session, a.get("cwd")))
    # attention only when it stops the lane, the git log is unreadable, it left unlanded
    # commits (a failed land), or it asks the owner
    left = lane_unlanded(CODEX_LANE)
    if left:
        reason += f"; {left} unlanded commit(s) on lanes/{CODEX_LANE} (land failed?)"
    if block or commits is None or left or left is None or not text.strip() or asks_owner(text):
        raise_attention(CODEX_LANE, sid, session, a.get("cwd"), reason, now)
    else:
        recent_end(st, CODEX_LANE, sid, reason, text, now)


def maybe_start_codex(st: dict, args, paused: dict, away: bool, now: float):
    if any(is_codex(s) for s in st["sessions"].values()):
        return
    cfg = read_codex()
    block = codex_gate(cfg, paused, away, st["codex_history"])
    if block is None and not CODEX_BRIEF.is_file():
        block = f"brief {CODEX_BRIEF} is missing"
    if block:
        st.pop("codex_quota", None)
        st.pop("codex_usage_wait", None)
    else:
        if st.get("codex_quota") and now - st.get("codex_checked", 0) < CODEX_RECHECK_SECS:
            return  # quota-blocked recently; don't ask the bridge every minute
        if now - st.get("codex_start_failed", 0) < CODEX_RECHECK_SECS:
            return  # `claude --bg` failed recently; back off the same way
        st["codex_checked"] = now
        bridge = bridge_status()
        block = codex_quota_block(bridge)
        st["codex_quota"] = bool(block)
        st["codex_usage_wait"] = bool(block) and codex_usage_block(bridge)
    if block:
        if block != st.get("codex_blocked") and cfg.get("enabled"):
            log(f"codex lane not started: {block}")
        st["codex_blocked"] = block
        return
    st.pop("codex_blocked", None)
    head = (git_main("rev-parse", "main") or "").strip()
    sid = start_bg(f"lane-{CODEX_LANE}", CODEX_PROMPT, args)
    if not sid:
        st["codex_start_failed"] = now
        log(f"codex lane: start failed; retrying in {CODEX_RECHECK_SECS // 60} min")
        return
    st.pop("codex_start_failed", None)
    log(f"started codex lane as {sid}; watch it: claude attach {sid}")
    st["sessions"][sid] = {"lane": CODEX_LANE, "kind": "codex", "session": "", "started": now,
                           "main_at_start": head}


def waiting_reason(st: dict, cands_now: list[str], cands_if_away: list[str],
                   codex_cfg: dict) -> str | None:
    """Why an orchestrator with nothing running should keep polling rather than exit: lanes
    that only wait for the owner to be away, or a codex lane held only by Codex usage."""
    why = []
    held = [l for l in cands_if_away if l not in cands_now]
    if held:
        why.append(f"{', '.join(held)} start when the owner is away "
                   f"({AWAY_SECS // 60} min without input)")
    if codex_cfg.get("enabled") and st.get("codex_usage_wait") and st.get("codex_blocked"):
        why.append(f"codex lane: {st['codex_blocked']}")
    if st.get("heal"):
        why.append("self-heal is checking " + ", ".join(st["heal"].get("rows") or [])
                   + " (blocked -> idea once evidence landed)")
    return "; ".join(why) or None


def _guarded(what: str, fn, *a):
    """Run one step of a tick; an exception is logged and the tick goes on."""
    try:
        return fn(*a)
    except Exception as e:  # noqa: BLE001 - one bad event must not kill the loop
        log(f"error while {what}: {type(e).__name__}: {e}")
        return None


# ---------------------------------------------------------------- commands

def tick(args) -> bool:
    """One pass; False when nothing is left to run or wait on."""
    now = time.time()
    try:
        st = load_state()
    except StateCorrupt as e:
        return state_corrupt(e)
    _LAST_GOOD.update(st=st, bad=0)
    reg = read_json(REGISTRY, {"lanes": {}})
    bl = backlogs()
    ag = agents()
    if ag is None:
        log("`claude agents --json` failed; skipping this pass")
        return True
    adopt_sessions(st, ag, bl, reg)
    slots = dict(st["sessions"])  # the same slot dicts: update_slots fills in, then drops them
    for ev, sid in update_slots(st, ag, reg, bl, now):
        _guarded(f"handling {ev} of session {sid}", handle_event, st, ev, sid, slots[sid], ag,
                 now)
    disk_low = _guarded("housekeeping", check_housekeeping, st, now)
    self_heal(st, bl, reg, now)
    _TICK["ag"] = ag
    write_state(st)
    # live lane sessions, minus the ones this pass just found finished (still listed until gone)
    live = {lane for lane, sid in live_lanes(ag, reg).items() if sid not in slots or sid in st["sessions"]}
    limit = read_max()
    paused = load_paused()
    away = owner_away() if any(p.get("when_away") for p in paused.values()) else False
    lane_count = sum(1 for s in st["sessions"].values() if not is_codex(s))
    failed = 0
    starts = plan_starts(bl, reg, st, now, args.only, paused, away, live)
    cands = [lane for lane, _ in starts]
    for lane, row in starts:
        if lane_count >= limit or disk_low:  # low disk: start nothing, keep tracking/draining
            break
        got = _guarded(f"starting {lane}", start_session, lane, args, row)
        if not got:
            failed += 1
            continue
        sid, slot = got
        slot["progress_at_start"] = progress_key(bl, lane)
        st["sessions"][sid] = slot
        lane_count += 1
        write_state(st)
    if limit > 0 and not disk_low:
        _guarded("starting the codex lane", maybe_start_codex, st, args, paused, away, now)
    if disk_low and limit > 0 and not st["sessions"]:
        write_state(st)
        return True  # nothing running; wait for space instead of declaring the work done
    if st["sessions"]:
        st.pop("waiting", None)
        write_state(st)
        return True
    if failed:
        write_state(st)
        log(f"could not start any session ({failed} attempt(s) failed, see above); stopping")
        return False
    if limit == 0:
        write_state(st)
        log("drained: no sessions running")
        return False
    cx = read_codex()
    wait = waiting_reason(st, cands, cands if away else
                          candidates(bl, reg, st, now, args.only, paused, True, live), cx)
    if "no_progress_naming" in st:
        unheld = dict(st)
        del unheld["no_progress_naming"]
        if candidates(bl, reg, unheld, now, args.only, paused, True, live):
            naming_wait = "naming waits for the naming backlog to change"
            wait = f"{wait}; {naming_wait}" if wait else naming_wait
    if args.only in (None, "naming") and not (wait and "naming waits" in wait) and any(
            not pause_blocks(paused, f"naming-{c}", True) for c in "abcdefghijklmnopqrstuvwxyz"):
        blocked = claim_blocked_naming_ids(bl, reg, now)
        if blocked and not candidates(bl, reg, st, now, args.only, paused, True, live):
            claim_wait = f"naming rows {', '.join(blocked)} wait on other lanes' claims"
            wait = f"{wait}; {claim_wait}" if wait else claim_wait
    if args.only in (None, "foundation"):
        blocked_f = [l for l in claim_blocked_foundation(bl, reg, now)
                     if l not in st["no_progress"] and not pause_blocks(paused, l, True)]
        if blocked_f and not candidates(bl, reg, st, now, args.only, paused, True, live):
            claim_wait = f"foundation lanes {', '.join(blocked_f)} wait on other lanes' claims"
            wait = f"{wait}; {claim_wait}" if wait else claim_wait
    if wait:
        if wait != st.get("waiting"):
            log(f"waiting: {wait}")
        st["waiting"] = wait
        write_state(st)
        return True
    write_state(st)
    held = candidates(bl, reg, st, now, args.only, live=live)
    if held:
        log(f"no unpaused lane has work left; the remaining work is paused ({', '.join(held)}: "
            f"`orchestrate.py resume <lane>`); done")
    else:
        log("no lane has work left; done")
    return False


def _hm(t) -> str:
    return time.strftime("%H:%M", time.localtime(float(t or 0)))


def digest_text(st: dict, ag: dict, reg: dict, att: list[dict], now: float) -> str:
    """One screen: sessions, the gate queue, attention, self-heal actions, leftovers to look at,
    recent session ends."""
    out = [f"# lanes digest {time.strftime('%Y-%m-%d %H:%M', time.localtime(now))}", ""]
    out.append(f"max {read_max()}, sessions {len(st['sessions'])}"
               + (f"; waiting: {st['waiting']}" if st.get("waiting") else "")
               + ("; DISK LOW: no new sessions" if st.get("disk_low") else ""))
    for sid, s in sorted(st["sessions"].items(), key=lambda kv: kv[1]["lane"])[:12]:
        phase = session_phase(s, ag.get(sid), reg, now)
        pos = gate_queue_position(reg, s["lane"], now, s.get("session"))
        if phase == "running" and pos is not None:
            phase = f"queued to land #{pos}"
        out.append(f"- {s['lane']} {sid} {phase}, {int((now - s['started']) / 60)} min"
                   + (f", row {s['row']}" if s.get("row") else ""))
    q = [t for t in reg.get("gate_queue") or [] if isinstance(t, dict)]
    if q:
        out.append("gate queue: " + ", ".join(f"{t.get('lane')} (since {_hm(t.get('enqueued'))})"
                                              for t in q[:6]))
    out += ["", f"## attention ({len(att)})" + ("; clear: orchestrate.py ack <lane|all>"
                                               if att else ": none")]
    out += [f"- {i.get('lane')} {_hm(i.get('time'))} {i.get('reason')}: {i.get('path')}"
            for i in att[-8:]]
    heal = st.get("heal_log", [])[-8:]
    if heal or st.get("heal"):
        out += ["", "## self-heal"]
        if st.get("heal"):
            out.append(f"- running: heal.py land, checking {', '.join(st['heal'].get('rows') or [])}")
        out += [f"- {_hm(h['t'])} {h['msg']}" for h in heal]
    look = (st.get("leftovers") or []) + [f"tmp/{n}: untouched {TMP_STALE_DAYS}+ days"
                                          for n in st.get("tmp_stale") or []]
    if look:
        out += ["", "## left for a look"] + [f"- {x}" for x in look[:8]]
        if len(look) > 8:
            out.append(f"- ... {len(look) - 8} more")
    ends = st.get("recent_ends", [])[-6:]
    if ends:
        out += ["", "## recent ends"] + [f"- {_hm(e['t'])} {e['lane']} {e['sid']} {e['what']}: "
                                         f"{e['summary']}" for e in ends]
    return "\n".join(out) + "\n"


def write_digest(st: dict, ag: dict, reg: dict, now: float):
    text = digest_text(st, ag, reg, load_attention(), now)
    try:
        if DIGEST.read_text(encoding="utf-8") == text:
            return
    except OSError:
        pass
    DIGEST.parent.mkdir(parents=True, exist_ok=True)
    fd, tmp = tempfile.mkstemp(dir=str(DIGEST.parent), prefix="digest.", suffix=".tmp")
    with os.fdopen(fd, "w", encoding="utf-8", newline="\n") as f:
        f.write(text)
    _replace(tmp, DIGEST)


_TICK: dict = {"ag": None}  # the last good agents listing, for the digest


def digest_after_tick():
    if _TICK["ag"] is None:
        return
    write_digest(load_state(), _TICK["ag"], read_json(REGISTRY, {"lanes": {}}), time.time())


_LAST_GOOD: dict = {"st": None, "bad": 0}
CORRUPT_TICKS = 3


def state_corrupt(e: Exception) -> bool:
    """orchestrator.json does not parse: skip the tick; after CORRUPT_TICKS in a row, set the
    bad file aside and restore the last state this runner read."""
    _LAST_GOOD["bad"] += 1
    log(f"state file unreadable ({e}); skipping this pass")
    if _LAST_GOOD["bad"] >= CORRUPT_TICKS and _LAST_GOOD["st"] is not None:
        aside = STATE.with_name(f"{STATE.name}.corrupt-{int(time.time())}")
        try:
            os.replace(STATE, aside)
        except OSError:
            pass
        write_state(_LAST_GOOD["st"])
        _LAST_GOOD["bad"] = 0
        log(f"restored the last good state; the unreadable file is kept as {aside}")
    return True


def handle_event(st: dict, ev: str, sid: str, slot: dict, ag: dict, now: float):
    """Log, stop and report one session event from update_slots."""
    lane, a = slot["lane"], ag.get(sid) or {}
    session = slot.get("session") or a.get("sessionId", "")
    if ev in ("finished", "no-progress"):
        stopped = "stopped" if is_ended(ag.get(sid)) or stop_session(sid) else "NOT stopped"
        log(f"{lane} finished; session {sid} {stopped} (claude attach {sid} reopens it)")
        if ev == "no-progress":
            wait = ("again unless the naming backlog changes" if lane.startswith("naming")
                    else "again this run")
            log(f"{lane} changed nothing in its backlog; not starting it {wait}")
    elif ev == "lost":
        log(f"{lane}: session {sid} ended without closing its lane; see "
            f"`python tools/lanes/lanes.py status` and `claude logs {sid}`")
    elif ev == "stalled":
        log(f"{lane}: session {sid} idle for {STALL_SECS // 60} min with its lane open; "
            f"look: claude attach {sid}")
    if is_codex(slot) and ev != "stalled":
        codex_ended(st, sid, slot, ev, a, now)
        return
    if ev == "stalled":
        raise_attention(lane, sid, session, a.get("cwd"),
                        f"idle {STALL_SECS // 60} min with its lane open", now)
        return
    # An ended session needs the owner only if it asks something or left work unlanded (its
    # land failed); anything else goes to the digest.
    text = last_assistant_text(transcript_path(session, a.get("cwd")))
    left = lane_unlanded(lane)
    what = {"finished": "finished", "no-progress": "finished without changing its backlog",
            "lost": "session ended without closing its lane"}[ev]
    if left:
        raise_attention(lane, sid, session, a.get("cwd"),
                        f"{what}; {left} unlanded commit(s) on its lane (land failed?)", now)
    elif left is None:
        raise_attention(lane, sid, session, a.get("cwd"),
                        f"{what}; could not read its lane branch (unlanded work?)", now)
    elif not text.strip():
        raise_attention(lane, sid, session, a.get("cwd"),
                        f"{what}; its last message is unreadable (it may ask something)", now)
    elif asks_owner(text):
        raise_attention(lane, sid, session, a.get("cwd"), f"{what}; it asks the owner", now)
    else:
        recent_end(st, lane, sid, what, text, now)


def cmd_run(args):
    try:
        st = load_state()
    except StateCorrupt as e:
        sys.exit(f"orchestrate: {e}\nFix or move the file aside; it is not overwritten.")
    if st.get("runner_pid") and st["runner_pid"] != os.getpid() and pid_alive(st["runner_pid"]):
        sys.exit(f"orchestrate: already running (pid {st['runner_pid']}); "
                 f"change the count with: python tools/lanes/orchestrate.py max {args.n}")
    st["runner_pid"] = os.getpid()
    st["no_progress"] = []
    st.pop("no_progress_naming", None)
    write_state(st)
    write_max(args.n)
    if args.codex:
        set_codex(True)
    log(f"keeping {args.n} lane session(s) active" + (f" ({args.only} only)" if args.only else "")
        + (" plus the codex lane" if read_codex().get("enabled") else "")
        + "; Ctrl-C stops the orchestrator only, sessions keep running")
    paused = load_paused()
    if paused:
        log("paused (not started): " + ", ".join(
            l + (" (when away)" if p.get("when_away") else "") for l, p in sorted(paused.items())))
    try:
        while True:
            try:
                more = tick(args)
                _guarded("writing the digest", digest_after_tick)
                if not more:
                    break
            except Exception as e:  # noqa: BLE001 - keep the loop alive; the next pass retries
                log(f"error in this pass: {type(e).__name__}: {e}")
            time.sleep(POLL_SECS)
    except KeyboardInterrupt:
        log("orchestrator stopped; sessions keep running (rerun to adopt them)")
    finally:
        try:
            st = load_state()
            st.pop("runner_pid", None)
            write_state(st)
        except (StateCorrupt, OSError) as e:
            log(f"could not clear runner_pid: {e}")


def cmd_max(args):
    write_max(args.n)
    print(f"max lane sessions: {args.n}" + (" (drain: running sessions finish, none start)"
                                             if args.n == 0 else ""))


def cmd_status(_args):
    st = load_state()
    ag = agents()
    if ag is None:
        print("(`claude agents --json` failed; session states unknown)")
        ag = {}
    reg = read_json(REGISTRY, {"lanes": {}})
    runner = st.get("runner_pid")
    running = bool(runner and pid_alive(runner))
    cx = read_codex()
    no_progress = list(st["no_progress"])
    if "no_progress_naming" in st:
        no_progress.append("naming (until the naming backlog changes)")
    print(f"orchestrator {'running (pid %d)' % runner if running else 'not running'}"
          f", max {read_max()}, sessions {len(st['sessions'])}"
          + f", codex lane {'on' if cx.get('enabled') else 'off'}"
          + (f", not restarting: {', '.join(no_progress)}" if no_progress else ""))
    if running and st.get("waiting"):
        print(f"waiting: {st['waiting']}")
    now = time.time()
    for sid, slot in sorted(st["sessions"].items(), key=lambda kv: kv[1]["lane"]):
        mins = int((now - slot["started"]) / 60)
        phase = session_phase(slot, ag.get(sid), reg, now)
        position = gate_queue_position(reg, slot["lane"], now, slot.get("session"))
        if phase == "running" and position is not None:
            phase = f"queued to land (position {position})"
        print(f"  {slot['lane']:<14} {sid}  {phase:<8} "
              f"{mins} min   claude attach {sid}"
              + (f"   row {slot['row']}" if slot.get("row") else ""))
    if cx.get("enabled") and st.get("codex_blocked"):
        print(f"codex lane waiting: {st['codex_blocked']}")
    hist = st["codex_history"][-3:]
    if hist:
        print("codex lane recent: " + "; ".join(
            f"{h.get('sid')} {len(h.get('landed') or [])} landed" for h in hist))
    paused = load_paused()
    if paused:
        idle = idle_secs()
        print("paused: " + ", ".join(
            l + (" (when away)" if p.get("when_away") else "") for l, p in sorted(paused.items()))
              + ("" if not any(p.get("when_away") for p in paused.values()) else
                 f"   [owner idle {int(idle // 60)} min; away at {AWAY_SECS // 60}]" if idle is not None
                 else "   [owner idle time unknown: when-away lanes stay paused]"))
    if st.get("heal"):
        print(f"self-heal: checking {', '.join(st['heal'].get('rows') or [])} (blocked -> idea)")
    for h in st.get("heal_log", [])[-3:]:
        print(f"self-heal {time.strftime('%m-%d %H:%M', time.localtime(h['t']))}: {h['msg']}")
    if DIGEST.is_file():
        print(f"digest: {DIGEST}")
    att = load_attention()
    if att:
        print(f"attention ({len(att)}; clear with: orchestrate.py ack <lane|all>):")
        for i in att:
            print(f"  {i.get('lane'):<14} {i.get('sid')}  "
                  f"{time.strftime('%m-%d %H:%M', time.localtime(i.get('time', 0)))}  "
                  f"{i.get('reason')}\n      {i.get('summary', '')[:150]}\n      {i.get('path')}")


def cmd_plan(args):
    st = load_state()
    paused = load_paused()
    away = owner_away() if any(p.get("when_away") for p in paused.values()) else False
    ag = agents() or {}
    reg = read_json(REGISTRY, {"lanes": {}})
    starts = plan_starts(backlogs(), reg, st, time.time(), args.only,
                         paused, away, set(live_lanes(ag, reg)))
    print("next lanes, in order: " + (" ".join(f"{l} (row {r})" for l, r in starts)
                                      or "(none: no startable row)"))
    if paused:
        print("paused: " + ", ".join(sorted(paused)))


def cmd_pause(args):
    p = load_paused()
    p[args.lane] = {"when_away": bool(args.when_away), "since": time.time()}
    write_json(PAUSED, p)
    try:
        sessions = load_state()["sessions"]
    except StateCorrupt:
        sessions = {}
    running = [sid for sid, s in sessions.items()
               if s["lane"] == args.lane or (args.lane == "naming" and s["lane"].startswith("naming"))]
    print(f"paused {args.lane}" + (f": started only when the owner is away (no input for "
                                   f"{AWAY_SECS // 60} min)" if args.when_away else ": never started")
          + "; persists across runs until `resume`.")
    print("Pausing does not stop a running session" + (
        f": {', '.join(running)} keeps going (claude attach <id> to look)." if running else "."))


def cmd_resume(args):
    p = load_paused()
    if p.pop(args.lane, None) is None:
        print(f"{args.lane} was not paused" + (f" (paused: {', '.join(sorted(p))})" if p else ""))
        return
    write_json(PAUSED, p)
    print(f"resumed {args.lane}")


def cmd_ack(args):
    gone = []

    def drop(items):
        keep = [i for i in items if not (args.lane == "all" or i.get("lane") == args.lane)]
        gone.extend(i for i in items if i not in keep)
        return keep
    left = update_attention(drop)
    print(f"cleared {len(gone)} attention item(s); {len(left)} left"
          + (" (files kept under tmp/lanes-state/attention/)" if gone else ""))


def cmd_codex(args):
    c = set_codex(args.state == "on")
    print(f"codex lane {'on' if c['enabled'] else 'off'}" + (
        " (re-armed; a running orchestrator starts it within a minute when Codex usage allows)"
        if c["enabled"] else " (a running lane-codex session is not stopped; it just won't restart)"))


def utf8_stdio():
    """A cp1252 console or redirected file must not crash on non-ASCII (commit subjects,
    session messages)."""
    for stream in (sys.stdout, sys.stderr):
        try:
            stream.reconfigure(encoding="utf-8", errors="backslashreplace")
        except (AttributeError, ValueError, OSError):
            pass


def main(argv=None):
    utf8_stdio()
    argv = list(sys.argv[1:] if argv is None else argv)
    if argv and argv[0].isdigit():
        argv.insert(0, "run")
    p = argparse.ArgumentParser(prog="orchestrate.py", description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = p.add_subparsers(dest="cmd", required=True)
    r = sub.add_parser("run")
    r.add_argument("n", type=int)
    r.add_argument("--only", choices=["naming", "foundation"])
    r.add_argument("--model")
    r.add_argument("--permission-mode", default="bypassPermissions")
    r.add_argument("--codex", action="store_true", help="also keep the codex lane (persists: codex off)")
    m = sub.add_parser("max")
    m.add_argument("n", type=int)
    sub.add_parser("status")
    d = sub.add_parser("plan", help="print the lanes a run would start next, start nothing")
    d.add_argument("--only", choices=["naming", "foundation"])
    pz = sub.add_parser("pause", help="never start a lane (persists); does not stop a running one")
    pz.add_argument("lane")
    pz.add_argument("--when-away", action="store_true",
                    help=f"start it only when the owner has been idle {AWAY_SECS // 60}+ min")
    rs = sub.add_parser("resume")
    rs.add_argument("lane")
    ak = sub.add_parser("ack", help="clear attention items")
    ak.add_argument("lane")
    cx = sub.add_parser("codex", help="turn the codex lane on/off (persists)")
    cx.add_argument("state", choices=["on", "off"])
    a = p.parse_args(argv)
    try:
        {"run": cmd_run, "max": cmd_max, "status": cmd_status, "plan": cmd_plan,
         "pause": cmd_pause, "resume": cmd_resume, "ack": cmd_ack, "codex": cmd_codex}[a.cmd](a)
    except StateCorrupt as e:
        sys.exit(f"orchestrate: {e}")


if __name__ == "__main__":
    main()
