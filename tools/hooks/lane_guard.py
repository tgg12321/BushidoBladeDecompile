#!/usr/bin/env python3
"""Lane hook (tools/lanes/lanes.py): one session per lane worktree, lanes stay out of each other's
claims, and a session that owns a lane leaves the main checkout alone.

PreToolUse (default mode), exit 2 = block:
  Edit|Write|MultiEdit|NotebookEdit
    - a file inside a lane worktree (bb2-worktrees/ln-<lane>) the session does not own;
    - a file another live lane has a path: claim on;
    - for a lane owner: a file in the MAIN checkout outside tmp/ (a subagent's cwd is main, so a
      relative path lands there).
  Bash|PowerShell -- the command is split into segments (heredoc / here-string bodies dropped,
  quotes respected) and each segment's real target is resolved, following `cd`/`Set-Location`:
    - a mutating git command, a wave/build tool (python tools/naming_wave.py, ...) or a mutating
      tools/wteng.ps1 call whose target is a lane worktree the session does not own;
    - for a lane owner: the same, targeting main.
    A command that is solely a `tools/lanes/lanes.py` call is allowed (it checks ownership).
PostToolUse (`--post`), for a lane owner's Bash|PowerShell calls: main's tracked-change set is
  snapshotted before the call and compared after; any new change in main is reported (exit 2
  feeds the report back to the agent). This catches writes no command parser can see.

Ownership holds until `lanes.py close` or a takeover, however stale the heartbeat. Sessions that
own no lane are only ever stopped from writing into another session's lane. Fails open (exit 0)
on internal errors, with a note on stderr.
"""
from __future__ import annotations

import fnmatch
import hashlib
import json
import os
import re
import subprocess
import sys
import time
from pathlib import Path

MAIN = Path(__file__).resolve().parents[2]

TOKEN = re.compile(r"""("[^"]*"|'[^']*'|[^\s;&|()<>]+)""")
LANES_ONLY = re.compile(r"""^\s*(?:&\s*)?(?:python3?|py)(?:\.exe)?\s+["']?[^;&|\n"'`$]*"""
                        r"""tools[\\/]+lanes[\\/]+lanes\.py["']?(?:\s+[^;&|\n<>`$]*)?\s*$""",
                        re.IGNORECASE)
CD_VERBS = {"cd", "set-location", "sl", "chdir", "push-location", "pushd"}
GIT_MUTATING = {"commit", "add", "rm", "mv", "reset", "checkout", "switch", "restore", "rebase",
                "merge", "cherry-pick", "revert", "clean", "apply", "am", "pull", "push",
                "update-ref", "update-index", "notes"}
TOOL_WRITERS = re.compile(r"(?:^|[\\/])(naming_wave|data_wave|naming_member_apply|format|"
                          r"naming_wavefollow|wavefollow|build_census|move_tu|grant_rescan)\.py$",
                          re.IGNORECASE)
WTENG_READONLY = (("queue", "status"), ("queue", "next"), ("status",), ("canonical",),
                  ("dossier",), ("diagnose",), ("sandbox",), ("tus-check",), ("layer2", "hash"),
                  ("layer2", "check"))


# ---------------------------------------------------------------- config / paths

def load_cfg():
    cfg = json.loads((MAIN / "tools" / "lanes" / "lanes.json").read_text(encoding="utf-8"))
    reg_p = MAIN / "tmp" / "lanes-state" / "registry.json"
    reg = json.loads(reg_p.read_text(encoding="utf-8")) if reg_p.exists() else {"lanes": {}}
    return cfg, reg.get("lanes", {})


def norm(p: str) -> str:
    return os.path.normcase(os.path.normpath(p.replace("/", os.sep)))


def to_win(p: str) -> str:
    m = re.match(r"^/(?:mnt/)?([a-zA-Z])/(.*)$", p)
    return f"{m.group(1)}:/{m.group(2)}" if m else p


def unq(tok: str) -> str:
    return tok[1:-1] if len(tok) >= 2 and tok[0] == tok[-1] and tok[0] in "\"'" else tok


def live(cfg, e) -> bool:
    age = time.time() - float(e.get("heartbeat", 0))
    return bool(e.get("owner")) and age <= float(cfg.get("stale_hours", 12)) * 3600


def lane_of_path(cfg, fp: str):
    """(lane, repo-relative path) if fp lies in a lane worktree, else None."""
    root = norm(str((MAIN / cfg["worktree_root"]).resolve()))
    raw = os.path.normpath(to_win(fp).replace("/", os.sep))  # case kept for the claim match
    if not norm(raw).startswith(root + os.sep):
        return None
    first, _, rest = raw[len(root) + 1:].partition(os.sep)
    first = os.path.normcase(first)
    if not first.startswith(cfg["dir_prefix"]):
        return None
    return first[len(cfg["dir_prefix"]):], rest.replace(os.sep, "/")


def resolve(path: str, cur: str) -> str:
    p = to_win(unq(path))
    return p if os.path.isabs(p) else os.path.join(cur, p)


def where(cfg, path: str):
    """('lane', name) | 'main' | 'main-tmp' | 'outside' for an absolute path."""
    hit = lane_of_path(cfg, path)
    if hit:
        return ("lane", hit[0].lower())
    n, m = norm(path), norm(str(MAIN))
    if n == m or n.startswith(m + os.sep):
        return "main-tmp" if (n + os.sep).startswith(norm(str(MAIN / "tmp")) + os.sep) else "main"
    return "outside"


def claimed_by_other(cfg, lanes, lane, rel):
    for other, e in lanes.items():
        if other == lane or not live(cfg, e):
            continue
        for tok in e.get("claims", []):
            kind, _, val = tok.partition(":")
            if kind == "path" and fnmatch.fnmatchcase(rel.lower(), val.rstrip("!").lower()):
                return other, tok
    return None


def owned_lane(lanes, sid):
    """The lane this session owns. Ownership holds until close or takeover, stale or not."""
    for lane, e in lanes.items():
        if sid and e.get("owner") == sid:
            return lane
    return None


def block(msg: str) -> int:
    print("BLOCKED by lane_guard.py: " + msg, file=sys.stderr)
    return 2


# ---------------------------------------------------------------- edits

def check_edit(cfg, lanes, sid, fp, cwd) -> int:
    fp = resolve(fp, cwd)
    hit = lane_of_path(cfg, fp)
    mine = owned_lane(lanes, sid)
    if hit:
        lane, rel = hit[0].lower(), hit[1]
        e = lanes.get(lane, {})
        if e.get("owner") != sid:
            who = e.get("owner") or "nobody (closed)"
            return block(f"{fp} is in lane worktree '{lane}', owned by {who}, not this session. "
                         f"One session per lane: open your own (python tools/lanes/lanes.py open <lane>).")
        other = claimed_by_other(cfg, lanes, lane, rel)
        if other:
            return block(f"{rel} is claimed by lane '{other[0]}' ({other[1]}). Leave it to that "
                         f"lane, or ask its owner to unclaim it.")
        return 0
    if mine and where(cfg, fp) == "main":
        wt = (MAIN / cfg["worktree_root"] / f"{cfg['dir_prefix']}{mine}").resolve()
        return block(f"this session owns lane '{mine}'; edit the file in its worktree {wt}, not "
                     f"in main ({fp}). Main is only a landing pad; scratch goes in main's tmp/.")
    return 0


# ---------------------------------------------------------------- shell

def strip_bodies(cmd: str) -> str:
    """Drop heredoc and PowerShell here-string bodies (their lines are data, not commands)."""
    cmd = re.sub(r"@'\r?\n.*?\r?\n'@", "@''@", cmd, flags=re.DOTALL)
    cmd = re.sub(r'@"\r?\n.*?\r?\n"@', '@""@', cmd, flags=re.DOTALL)
    out, lines, i = [], cmd.split("\n"), 0
    while i < len(lines):
        ln = lines[i]
        out.append(ln)
        m = re.search(r"<<-?\s*(['\"]?)([A-Za-z_][A-Za-z0-9_]*)\1", ln)
        i += 1
        if m:
            word = m.group(2)
            while i < len(lines) and lines[i].strip() != word:
                i += 1
            i += 1  # the terminator
    return "\n".join(out)


def segments(cmd: str):
    """Split at ; && || | newline outside quotes."""
    blank = re.sub(r""""[^"]*"|'[^']*'""", lambda m: "Q" * len(m.group(0)), cmd)
    cuts, out, start = [m.span() for m in re.finditer(r"&&|\|\||[;|\n]", blank)], [], 0
    for a, b in cuts:
        out.append(cmd[start:a])
        start = b
    out.append(cmd[start:])
    return [s for s in out if s.strip()]


def words(seg: str):
    w = [m.group(1) for m in TOKEN.finditer(seg)]
    while w and (w[0] == "&" or re.match(r"^\$env:\w+=|^\w+=", w[0])):
        w = w[1:]
    return w


def git_target(w, cur):
    """(target dir, mutating?) for a git command word list (w[0] is git)."""
    i, target = 1, cur
    while i < len(w):
        a = unq(w[i])
        if a == "-C" and i + 1 < len(w):
            target = resolve(w[i + 1], target)
            i += 2
        elif a == "-c" and i + 1 < len(w):
            i += 2
        elif a.startswith("--work-tree=") or a.startswith("--git-dir="):
            target = resolve(a.split("=", 1)[1], target)
            i += 1
        elif a.startswith("-"):
            i += 1
        else:
            break
    if i >= len(w):
        return target, False
    sub, rest = unq(w[i]).lower(), [unq(x) for x in w[i + 1:]]
    if sub in GIT_MUTATING:
        return target, True
    if sub == "branch":
        return target, any(x in ("-d", "-D", "-f", "-m", "-M", "--delete", "--force", "--move")
                           for x in rest)
    if sub == "worktree":
        return target, bool(rest) and rest[0] in ("add", "remove", "move", "prune")
    if sub == "stash":
        return target, not (rest and rest[0] in ("list", "show"))
    if sub == "tag":
        return target, bool(rest) and not any(x in ("-l", "--list") for x in rest)
    return target, False


def shell_targets(cmd: str, cwd: str):
    """Yield (target dir, what) for every mutating git / wave-tool / wteng segment."""
    cur = cwd
    for seg in segments(strip_bodies(cmd)):
        w = words(seg)
        if not w:
            continue
        verb = unq(w[0]).lower()
        base = os.path.basename(verb)
        if verb in CD_VERBS:
            args = [x for x in w[1:] if not unq(x).startswith("-")]
            if args:
                cur = resolve(args[0], cur)
            continue
        if base in ("git", "git.exe"):
            t, mut = git_target(w, cur)
            if mut:
                yield t, "git " + " ".join(unq(x) for x in w[1:4])
            continue
        flat = [unq(x) for x in w]
        wt_i = next((i for i, x in enumerate(flat) if x.lower().endswith("wteng.ps1")), None)
        if wt_i is not None and wt_i + 1 < len(flat):
            tgt, sub = flat[wt_i + 1], tuple(x.lower() for x in flat[wt_i + 2:wt_i + 4])
            if any(sub[:len(ro)] == ro for ro in WTENG_READONLY):
                continue
            yield (str(MAIN) if tgt.lower() == "main" else resolve(tgt, cur)), "wteng " + " ".join(sub)
            continue
        if base.startswith("python") or base in ("py", "py.exe", "python3"):
            script = next((x for x in flat[1:] if not x.startswith("-")), "")
            if TOOL_WRITERS.search(script):
                yield cur, os.path.basename(script)


def check_shell(cfg, lanes, sid, cmd, cwd) -> int:
    if LANES_ONLY.match(cmd):
        return 0
    mine = owned_lane(lanes, sid)
    for target, what in shell_targets(cmd, cwd):
        loc = where(cfg, target)
        if isinstance(loc, tuple):
            e = lanes.get(loc[1], {})
            if e.get("owner") != sid:
                who = e.get("owner") or "nobody (closed)"
                return block(f"`{what}` would run in lane worktree '{loc[1]}', owned by {who}, not "
                             f"this session. Reads are fine; writes need your own lane.")
        elif mine and loc in ("main", "main-tmp"):
            wt = (MAIN / cfg["worktree_root"] / f"{cfg['dir_prefix']}{mine}").resolve()
            return block(f"this session owns lane '{mine}', and `{what}` would run against main "
                         f"(a subagent's cwd is main). Pin it to the lane worktree {wt}: "
                         f"git -C \"<wt>\" ..., cd \"<wt>\" && ..., tools/wteng.ps1 \"<wt>\" ... "
                         f"Landing is python tools/lanes/lanes.py land {mine}.")
    return 0


# ---------------------------------------------------------------- post check

def main_dirty() -> set:
    r = subprocess.run(["git", "-C", str(MAIN), "status", "--porcelain", "--untracked-files=no"],
                       capture_output=True, text=True, encoding="utf-8", errors="replace")
    return set(r.stdout.splitlines()) if r.returncode == 0 else set()


def snap_path(sid: str) -> Path:
    h = hashlib.sha1(sid.encode()).hexdigest()[:16]
    return MAIN / "tmp" / "lanes-state" / f"snap-{h}.json"


def pre_snapshot(sid):
    p = snap_path(sid)
    p.parent.mkdir(parents=True, exist_ok=True)
    p.write_text(json.dumps(sorted(main_dirty())), encoding="utf-8")


def post_check(cfg, lanes, sid) -> int:
    mine = owned_lane(lanes, sid)
    p = snap_path(sid)
    if not mine or not p.exists():
        return 0
    before = set(json.loads(p.read_text(encoding="utf-8")))
    new = sorted(main_dirty() - before)
    if not new:
        return 0
    wt = (MAIN / cfg["worktree_root"] / f"{cfg['dir_prefix']}{mine}").resolve()
    print("lane_guard: main has NEW uncommitted changes after this command, and this session owns "
          f"lane '{mine}':\n  " + "\n  ".join(new[:15]) +
          f"\nIf this command made them: redo the change in the lane worktree {wt}, then restore "
          f"main's copies (git -C \"{MAIN}\" restore -- <paths>). Main must stay clean: every "
          f"lane's land refuses while it is dirty.", file=sys.stderr)
    return 2


# ---------------------------------------------------------------- entry

def main() -> int:
    post = "--post" in sys.argv[1:]
    try:
        payload = json.load(sys.stdin)
    except Exception as e:
        print(f"lane_guard: fail-open (bad payload: {e})", file=sys.stderr)
        return 0
    try:
        cfg, lanes = load_cfg()
        sid = str(payload.get("session_id") or "")
        tool = payload.get("tool_name")
        ti = payload.get("tool_input") or {}
        cwd = payload.get("cwd") or os.getcwd()
        if post:
            return post_check(cfg, lanes, sid) if tool in ("Bash", "PowerShell") else 0
        if tool in ("Edit", "Write", "MultiEdit", "NotebookEdit"):
            fp = ti.get("file_path") or ti.get("notebook_path") or ""
            return check_edit(cfg, lanes, sid, fp, cwd) if fp else 0
        if tool in ("Bash", "PowerShell"):
            rc = check_shell(cfg, lanes, sid, ti.get("command") or "", cwd)
            if rc == 0 and owned_lane(lanes, sid):
                pre_snapshot(sid)
            return rc
        return 0
    except Exception as e:
        print(f"lane_guard: fail-open (error: {e})", file=sys.stderr)
        return 0


if __name__ == "__main__":
    sys.exit(main())
