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
its slot. A lane whose session finished without changing its backlog is not started again this run.
For a no-progress, stalled or lost session the session's last message (usually its question) is
saved under tmp/lanes-state/attention/ and listed by `status` until `ack`.

Picking work: naming lanes (naming-a, naming-b, ...) while docs/naming/phase3/backlog.tsv has more
workable rows (idea/researched/reviewed) than naming sessions; foundation lanes in
docs/foundation/backlog.tsv order while a lane has rows not landed/dropped/blocked
(blocked = waits on an owner ruling). Naming and foundation
alternate. Lanes owned in the registry by any live session are skipped, and so are paused lanes
(tmp/lanes-state/paused.json; `naming` pauses every naming lane; a `--when-away` lane starts only
after 20 min without keyboard/mouse input). `researching` rows wait on a Codex job, so they do not
count as workable. `max 0` drains: running sessions finish, then the orchestrator exits.

Codex lane (`--codex` / `codex on`): one extra session `lane-codex`, outside the N slots, that
follows tmp/codex/fake-lane-brief.md. It is restarted when it finishes only while Codex usage is
under 90% (5 h) / 80% (week), the bridge is not BLOCKED, and its last two sessions did not both
land nothing (a main commit whose body has a line starting `Codex run:`); `codex on` re-arms
that. An untracked `lane-<x>` session (from before a restart) is adopted, and keeps lane x busy,
only while it works or, idle, still owns lane x in the registry. With nothing running it keeps polling only while work
waits for the owner to be away or the codex lane waits on usage; attention: attention/index.json.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
import time
from pathlib import Path

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
CODEX_MAX_5H, CODEX_MAX_WEEK = 90, 80
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


def progress_key(bl: dict, lane: str) -> str:
    """A digest of the backlog rows a lane works from; unchanged after a session = no progress."""
    rows = bl["naming"] if lane.startswith("naming") else \
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


def session_phase(slot: dict, ag: dict | None, reg: dict, now: float) -> str:
    """running | finished | stalled | lost for one tracked session.

    A session has released its lane once it opened the lane (seen owning it, or the lane was
    closed after the session started) and no longer owns it. Finished = released and idle (or
    exited). A session idle for NO_OPEN_GRACE_SECS that never opened its lane (it found no
    work) is finished too. An idle session that owns its lane usually just waits on a
    background `lanes.py land`; it is only reported as stalled after STALL_SECS."""
    e = reg.get("lanes", {}).get(slot["lane"]) or {}
    owns = bool(slot.get("session")) and e.get("owner") == slot["session"]
    released = not owns and (slot.get("seen_open") or float(e.get("closed") or 0) >= slot["started"])
    if is_ended(ag):
        return "finished" if released else "lost"
    if is_idle(ag):
        if released:
            return "finished"
        idle_for = now - slot.get("idle_since", now)
        if (slot.get("session") and not owns and not slot.get("seen_open")
                and idle_for >= NO_OPEN_GRACE_SECS):
            return "finished"  # decided there was no work and never opened its lane
        if idle_for >= STALL_SECS:
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


def candidates(bl: dict, reg: dict, st: dict, now: float, only: str | None,
               paused: dict | None = None, away: bool = False,
               live: set | frozenset = frozenset()) -> list[str]:
    """Lanes to start next, best first (naming and foundation alternate). `live`: lanes with a
    running `lane-<lane>` session, tracked or not, which count as busy."""
    paused = paused or {}
    busy = {s["lane"] for s in st["sessions"].values()} | set(live)
    skip = set(st["no_progress"])
    naming: list[str] = []
    if only in (None, "naming") and "naming" not in skip:
        workable = sum(1 for r in bl["naming"] if r.get("stage") in NAMING_WORKABLE)
        owned = {l for l in reg.get("lanes", {}) if lane_owned(reg, l, now)}
        active = sum(1 for lane in busy | owned if lane.startswith("naming"))
        free = [f"naming-{c}" for c in "abcdefghijklmnopqrstuvwxyz"
                if f"naming-{c}" not in busy and not lane_owned(reg, f"naming-{c}", now)
                and not pause_blocks(paused, f"naming-{c}", away)]
        naming = free[:max(0, workable - active)]
    found: list[str] = []
    if only in (None, "foundation"):
        for r in bl["foundation"]:
            lane = r.get("lane", "")
            if (lane and r.get("stage") not in FOUNDATION_DONE and lane not in found
                    and lane not in busy and lane not in skip and not lane_owned(reg, lane, now)
                    and not pause_blocks(paused, lane, away)):
                found.append(lane)
    out: list[str] = []
    for i in range(max(len(naming), len(found))):
        out += naming[i:i + 1] + found[i:i + 1]
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
            group = "naming" if slot["lane"].startswith("naming") else slot["lane"]
            if (phase == "finished" and not is_codex(slot)
                    and progress_key(bl, slot["lane"]) == slot.get("progress_at_start")):
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


def start_session(lane: str, args) -> tuple[str, dict] | None:
    skill = "naming-session" if lane.startswith("naming") else "foundation-session"
    started = time.time()
    sid = start_bg(f"lane-{lane}", f"/{skill} {lane}", args)
    if not sid:
        return None
    log(f"started {lane} ({skill}) as {sid}; watch it: claude attach {sid}")
    # The full sessionId (the registry owner) is filled in from the next listing.
    return sid, {"lane": lane, "session": "", "started": started}


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
    raise_attention(CODEX_LANE, sid, slot.get("session") or a.get("sessionId", ""), a.get("cwd"),
                    reason, now)


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
    write_state(st)
    # live lane sessions, minus the ones this pass just found finished (still listed until gone)
    live = {lane for lane, sid in live_lanes(ag, reg).items() if sid not in slots or sid in st["sessions"]}
    limit = read_max()
    paused = load_paused()
    away = owner_away() if any(p.get("when_away") for p in paused.values()) else False
    lane_count = sum(1 for s in st["sessions"].values() if not is_codex(s))
    failed = 0
    cands = candidates(bl, reg, st, now, args.only, paused, away, live)
    for lane in cands:
        if lane_count >= limit:
            break
        got = _guarded(f"starting {lane}", start_session, lane, args)
        if not got:
            failed += 1
            continue
        sid, slot = got
        slot["progress_at_start"] = progress_key(bl, lane)
        st["sessions"][sid] = slot
        lane_count += 1
        write_state(st)
    if limit > 0:
        _guarded("starting the codex lane", maybe_start_codex, st, args, paused, away, now)
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
            log(f"{lane} changed nothing in its backlog; not starting it again this run")
    elif ev == "lost":
        log(f"{lane}: session {sid} ended without closing its lane; see "
            f"`python tools/lanes/lanes.py status` and `claude logs {sid}`")
    elif ev == "stalled":
        log(f"{lane}: session {sid} idle for {STALL_SECS // 60} min with its lane open; "
            f"look: claude attach {sid}")
    if is_codex(slot) and ev != "stalled":
        codex_ended(st, sid, slot, ev, a, now)
    elif ev != "finished":
        reason = {"no-progress": "finished without changing its backlog",
                  "lost": "session ended without closing its lane",
                  "stalled": f"idle {STALL_SECS // 60} min with its lane open"}[ev]
        raise_attention(lane, sid, session, a.get("cwd"), reason, now)


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
                if not tick(args):
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
    cx = read_codex()
    print(f"orchestrator {'running (pid %d)' % runner if runner and pid_alive(runner) else 'not running'}"
          f", max {read_max()}, sessions {len(st['sessions'])}"
          + f", codex lane {'on' if cx.get('enabled') else 'off'}"
          + (f", not restarting: {', '.join(st['no_progress'])}" if st["no_progress"] else ""))
    now = time.time()
    for sid, slot in sorted(st["sessions"].items(), key=lambda kv: kv[1]["lane"]):
        mins = int((now - slot["started"]) / 60)
        print(f"  {slot['lane']:<14} {sid}  {session_phase(slot, ag.get(sid), reg, now):<8} "
              f"{mins} min   claude attach {sid}")
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
    lanes = candidates(backlogs(), reg, st, time.time(), args.only,
                       paused, away, set(live_lanes(ag, reg)))
    print("next lanes, in order: " + (" ".join(lanes) or "(none: no work left)"))
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
