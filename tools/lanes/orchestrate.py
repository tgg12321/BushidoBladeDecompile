#!/usr/bin/env python3
"""orchestrate.py: keep N lane sessions running, one background Claude session per lane.

    python tools/lanes/orchestrate.py 3              run, keeping 3 lanes active (Ctrl-C stops
                                                     the orchestrator; sessions keep running)
    python tools/lanes/orchestrate.py 3 --only naming     naming lanes only (or: foundation)
    python tools/lanes/orchestrate.py max 5          change the number while it runs (0 = drain:
                                                     running sessions finish, none start)
    python tools/lanes/orchestrate.py status         what is running
    python tools/lanes/orchestrate.py plan           which lanes it would start next

Each session is `claude --bg` running `/naming-session <lane>` or `/foundation-session <lane>`
from the main checkout. Watch one with `claude attach lane-<lane>`. A session is finished when it
has closed its lane (`lanes.py close`) and gone idle; the orchestrator then stops it (the
conversation is kept: `claude attach <id>` reopens it) and starts the next lane with work, until the backlogs run
dry. A session idle for 90 minutes with its lane still open is reported as needing attention and keeps
its slot. A lane whose session finished without changing its backlog is not started again this run.

Picking work: naming lanes (naming-a, naming-b, ...) while docs/naming/phase3/backlog.tsv has more
workable rows (idea/researched/reviewed) than naming sessions; foundation lanes in
docs/foundation/backlog.tsv order while a lane has rows not landed/dropped. Naming and foundation
alternate. Lanes owned in the registry by any live session are skipped. `researching` rows wait on
a Codex job, so they do not count as workable. `max 0` drains: running sessions finish, then the
orchestrator exits.
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
STATE = MAIN / "tmp" / "lanes-state" / "orchestrator.json"
MAX_FILE = MAIN / "tmp" / "lanes-state" / "orchestrator-max"
REGISTRY = MAIN / "tmp" / "lanes-state" / "registry.json"
NAMING_BACKLOG = MAIN / "docs" / "naming" / "phase3" / "backlog.tsv"
FOUNDATION_BACKLOG = MAIN / "docs" / "foundation" / "backlog.tsv"
NAMING_WORKABLE = {"idea", "researched", "reviewed"}
FOUNDATION_DONE = {"landed", "dropped"}
ENDED_STATES = {"exited", "stopped", "failed", "killed", "error"}
STALL_SECS = 90 * 60
POLL_SECS = 60
MISS_TICKS = 3      # a session absent from this many good listings in a row has ended
CMD_TIMEOUT = 120


def _stale_secs() -> int:
    cfg = Path(os.environ.get("BB2_LANES_CONFIG") or MAIN / "tools" / "lanes" / "lanes.json")
    try:
        return int(float(json.loads(cfg.read_text(encoding="utf-8")).get("stale_hours", 12)) * 3600)
    except (OSError, ValueError):
        return 12 * 3600


STALE_SECS = _stale_secs()  # same rule as lanes.py is_stale


# ---------------------------------------------------------------- inputs

def read_json(p: Path, default):
    try:
        return json.loads(p.read_text(encoding="utf-8"))
    except (FileNotFoundError, json.JSONDecodeError):
        return default


def write_state(st: dict):
    STATE.parent.mkdir(parents=True, exist_ok=True)
    tmp = STATE.with_suffix(".tmp")
    tmp.write_text(json.dumps(st, indent=2, sort_keys=True) + "\n", encoding="utf-8", newline="\n")
    tmp.replace(STATE)


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


# ---------------------------------------------------------------- the decision (pure)

def lane_owned(reg: dict, lane: str, now: float) -> bool:
    e = reg.get("lanes", {}).get(lane) or {}
    return bool(e.get("owner")) and now - float(e.get("heartbeat", 0)) <= STALE_SECS


def is_ended(ag: dict | None) -> bool:
    # An exited background session is still listed (state "done") but has no pid.
    return ag is None or ag.get("state") in ENDED_STATES or not ag.get("pid")


def is_idle(ag: dict | None) -> bool:
    return ag is not None and (ag.get("state") == "done" or ag.get("status") == "idle")


def session_phase(slot: dict, ag: dict | None, reg: dict, now: float) -> str:
    """running | finished | stalled | lost for one tracked session.

    A session has released its lane once it opened the lane (seen owning it, or the lane was
    closed after the session started) and no longer owns it. Finished = released and idle (or
    exited). An idle session usually just waits on a background `lanes.py land`; it is only
    reported as stalled after STALL_SECS."""
    e = reg.get("lanes", {}).get(slot["lane"]) or {}
    owns = bool(slot.get("session")) and e.get("owner") == slot["session"]
    released = not owns and (slot.get("seen_open") or float(e.get("closed") or 0) >= slot["started"])
    if is_ended(ag):
        return "finished" if released else "lost"
    if is_idle(ag):
        if released:
            return "finished"
        if now - slot.get("idle_since", now) >= STALL_SECS:
            return "stalled"
    return "running"


def candidates(bl: dict, reg: dict, st: dict, now: float, only: str | None) -> list[str]:
    """Lanes to start next, best first (naming and foundation alternate)."""
    busy = {s["lane"] for s in st["sessions"].values()}
    skip = set(st["no_progress"])
    naming: list[str] = []
    if only in (None, "naming") and "naming" not in skip:
        workable = sum(1 for r in bl["naming"] if r.get("stage") in NAMING_WORKABLE)
        owned = {l for l in reg.get("lanes", {}) if lane_owned(reg, l, now)}
        active = sum(1 for lane in busy | owned if lane.startswith("naming"))
        free = [f"naming-{c}" for c in "abcdefghijklmnopqrstuvwxyz"
                if f"naming-{c}" not in busy and not lane_owned(reg, f"naming-{c}", now)]
        naming = free[:max(0, workable - active)]
    found: list[str] = []
    if only in (None, "foundation"):
        for r in bl["foundation"]:
            lane = r.get("lane", "")
            if (lane and r.get("stage") not in FOUNDATION_DONE and lane not in found
                    and lane not in busy and lane not in skip and not lane_owned(reg, lane, now)):
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
            if phase == "finished" and progress_key(bl, slot["lane"]) == slot.get("progress_at_start"):
                st["no_progress"] = sorted(set(st["no_progress"]) | {group})
                phase = "no-progress"
            events.append((phase, sid))
        elif phase == "stalled" and not slot.get("warned"):
            slot["warned"] = True
            events.append(("stalled", sid))
    return events


# ---------------------------------------------------------------- actions

def log(msg: str):
    print(time.strftime("%H:%M:%S ") + msg, flush=True)


def start_session(lane: str, args) -> tuple[str, dict] | None:
    skill = "naming-session" if lane.startswith("naming") else "foundation-session"
    cmd = [claude_exe(), "--bg", "-n", f"lane-{lane}", "--permission-mode", args.permission_mode]
    if args.model:
        cmd += ["--model", args.model]
    cmd.append(f"/{skill} {lane}")
    started = time.time()
    rc, out = sh(cmd, cwd=MAIN)
    m = re.search(r"backgrounded\W+([0-9a-f]{8})\b", out)
    if not m:
        log(f"could not start {lane}: rc={rc} {out.strip()[:300]}")
        return None
    sid = m.group(1)
    log(f"started {lane} ({skill}) as {sid}; watch it: claude attach {sid}")
    # The full sessionId (the registry owner) is filled in from the next listing.
    return sid, {"lane": lane, "session": "", "started": started}


def stop_session(sid: str) -> bool:
    rc, out = sh([claude_exe(), "stop", sid])
    if rc != 0:
        log(f"`claude stop {sid}` failed (rc={rc}): {out.strip()[:200]}")
    return rc == 0


# ---------------------------------------------------------------- state

def load_state() -> dict:
    st = read_json(STATE, {})
    st.setdefault("sessions", {})
    st.setdefault("no_progress", [])
    return st


def read_max() -> int:
    try:
        return int(MAX_FILE.read_text(encoding="utf-8").strip())
    except (FileNotFoundError, ValueError):
        return 0


def write_max(n: int):
    MAX_FILE.parent.mkdir(parents=True, exist_ok=True)
    MAX_FILE.write_text(f"{n}\n", encoding="utf-8", newline="\n")


def pid_alive(pid: int) -> bool:
    # os.kill(pid, 0) terminates the process on Windows; ask tasklist instead.
    # Windows reuses pids, so also require a python image.
    rc, out = sh(["tasklist", "/FI", f"PID eq {pid}", "/NH", "/FO", "CSV"], timeout=30)
    return any(f'"{pid}"' in ln and ln.lower().startswith(('"python', '"py.exe'))
               for ln in out.splitlines())


# ---------------------------------------------------------------- commands

def tick(args) -> bool:
    """One pass; False when nothing is left to run or wait on."""
    now = time.time()
    st = load_state()
    reg = read_json(REGISTRY, {"lanes": {}})
    bl = backlogs()
    ag = agents()
    if ag is None:
        log("`claude agents --json` failed; skipping this pass")
        return True
    lanes_by_sid = {sid: s["lane"] for sid, s in st["sessions"].items()}
    for ev, sid in update_slots(st, ag, reg, bl, now):
        lane = lanes_by_sid[sid]
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
    write_state(st)
    limit = read_max()
    failed = 0
    for lane in candidates(bl, reg, st, now, args.only):
        if len(st["sessions"]) >= limit:
            break
        got = start_session(lane, args)
        if not got:
            failed += 1
            continue
        sid, slot = got
        slot["progress_at_start"] = progress_key(bl, lane)
        st["sessions"][sid] = slot
        write_state(st)
    if st["sessions"]:
        return True
    if failed:
        log(f"could not start any session ({failed} attempt(s) failed, see above); stopping")
    else:
        log("drained: no sessions running" if limit == 0 else "no lane has work left; done")
    return False


def cmd_run(args):
    st = load_state()
    if st.get("runner_pid") and st["runner_pid"] != os.getpid() and pid_alive(st["runner_pid"]):
        sys.exit(f"orchestrate: already running (pid {st['runner_pid']}); "
                 f"change the count with: python tools/lanes/orchestrate.py max {args.n}")
    st["runner_pid"] = os.getpid()
    st["no_progress"] = []
    write_state(st)
    write_max(args.n)
    log(f"keeping {args.n} lane session(s) active" + (f" ({args.only} only)" if args.only else "")
        + "; Ctrl-C stops the orchestrator only, sessions keep running")
    try:
        while tick(args):
            time.sleep(POLL_SECS)
    except KeyboardInterrupt:
        log("orchestrator stopped; sessions keep running (rerun to adopt them)")
    finally:
        st = load_state()
        st.pop("runner_pid", None)
        write_state(st)


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
    print(f"orchestrator {'running (pid %d)' % runner if runner and pid_alive(runner) else 'not running'}"
          f", max {read_max()}, sessions {len(st['sessions'])}"
          + (f", not restarting: {', '.join(st['no_progress'])}" if st["no_progress"] else ""))
    now = time.time()
    for sid, slot in sorted(st["sessions"].items(), key=lambda kv: kv[1]["lane"]):
        mins = int((now - slot["started"]) / 60)
        print(f"  {slot['lane']:<14} {sid}  {session_phase(slot, ag.get(sid), reg, now):<8} "
              f"{mins} min   claude attach {sid}")


def cmd_plan(args):
    st = load_state()
    lanes = candidates(backlogs(), read_json(REGISTRY, {"lanes": {}}), st, time.time(), args.only)
    print("next lanes, in order: " + (" ".join(lanes) or "(none: no work left)"))


def main(argv=None):
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
    m = sub.add_parser("max")
    m.add_argument("n", type=int)
    sub.add_parser("status")
    d = sub.add_parser("plan", help="print the lanes a run would start next, start nothing")
    d.add_argument("--only", choices=["naming", "foundation"])
    a = p.parse_args(argv)
    {"run": cmd_run, "max": cmd_max, "status": cmd_status, "plan": cmd_plan}[a.cmd](a)


if __name__ == "__main__":
    main()
