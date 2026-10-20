#!/usr/bin/env python3
"""PreToolUse hook: one session per lane worktree, and lanes stay out of main and out of each
other's claims (tools/lanes/lanes.py).

Blocks (exit 2):
  Edit|Write|MultiEdit|NotebookEdit
    - a file inside a lane worktree (bb2-worktrees/ln-<lane>) the session does not own;
    - a file another live lane has a path: claim on (inside any lane worktree);
    - a file in the MAIN checkout (outside tmp/) while the session owns a lane: lane work
      happens in the lane worktree (a subagent's cwd is main, so relative paths land there).
  Bash|PowerShell
    - a mutating command that names a lane worktree the session does not own;
    - a mutating git command with no lane-worktree pin while the session owns a lane (it would
      hit main). `tools/lanes/lanes.py` itself is always allowed: it checks ownership itself.

Sessions that own no lane and touch no lane worktree are never affected. Fails open (exit 0)
on any internal error, with a note on stderr.
"""
from __future__ import annotations

import fnmatch
import json
import os
import re
import sys
import time
from pathlib import Path

MAIN = Path(__file__).resolve().parents[2]
MUTATING_GIT = re.compile(
    r"\bgit\b(?:\s+-[cC]\s+\S+|\s+--\S+)*\s+"
    r"(commit|add|rm|mv|reset|checkout|switch|restore|rebase|merge|cherry-pick|revert|stash|"
    r"clean|apply|am|pull|push|tag|branch\s+-[dDfmM]|worktree\s+(?:add|remove|move|prune)|"
    r"update-ref|update-index)\b")
WRITERS = re.compile(
    r"(?<![<>=!])>(?!=)|\b(Set-Content|Out-File|Add-Content|New-Item|Remove-Item|Move-Item|"
    r"Copy-Item|Rename-Item|tee|sed\s+-i|rm|rmdir|mv|cp|mkdir|touch|make|wteng\.ps1|"
    r"naming_wave\.py|data_wave\.py|naming_member_apply\.py|format\.py)\b", re.IGNORECASE)
LANES_CLI = re.compile(r"tools[\\/]+lanes[\\/]+lanes\.py", re.IGNORECASE)


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


def live(cfg, e) -> bool:
    return bool(e.get("owner")) and time.time() - float(e.get("heartbeat", 0)) <= \
        float(cfg.get("stale_hours", 6)) * 3600


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


def claimed_by_other(cfg, lanes, lane, rel):
    for other, e in lanes.items():
        if other == lane or not live(cfg, e):
            continue
        for tok in e.get("claims", []):
            kind, _, val = tok.partition(":")
            val = val.rstrip("!")
            if kind == "path" and (rel == val or fnmatch.fnmatchcase(rel, val)):
                return other, tok
    return None


def owned_lane(cfg, lanes, sid):
    for lane, e in lanes.items():
        if e.get("owner") == sid and live(cfg, e):
            return lane
    return None


def block(msg: str) -> int:
    print("BLOCKED by lane_guard.py: " + msg, file=sys.stderr)
    return 2


def check_edit(cfg, lanes, sid, fp, cwd) -> int:
    if not os.path.isabs(to_win(fp)):
        fp = os.path.join(cwd, fp)
    hit = lane_of_path(cfg, fp)
    mine = owned_lane(cfg, lanes, sid)
    if hit:
        lane, rel = hit
        e = lanes.get(lane, {})
        if e.get("owner") != sid:
            who = e.get("owner") or "nobody (closed)"
            return block(f"{fp} is in lane worktree '{lane}', owned by {who}, not this session. "
                         f"One session per lane: open your own (python tools/lanes/lanes.py open <lane>).")
        other = claimed_by_other(cfg, lanes, lane, rel)
        if other:
            return block(f"{rel} is claimed by lane '{other[0]}' ({other[1]}). Leave it to that lane, "
                         f"or ask its owner to unclaim it.")
        return 0
    if mine:
        p = norm(to_win(fp))
        m = norm(str(MAIN))
        if p.startswith(m + os.sep) and not p.startswith(norm(str(MAIN / "tmp")) + os.sep):
            wt = MAIN / cfg["worktree_root"] / f"{cfg['dir_prefix']}{mine}"
            return block(f"this session owns lane '{mine}'; edit the file in its worktree "
                         f"{wt.resolve()}, not in main ({fp}). Main is only a landing pad.")
    return 0


def check_shell(cfg, lanes, sid, cmd) -> int:
    if LANES_CLI.search(cmd):
        return 0
    root = str((MAIN / cfg["worktree_root"]).resolve())
    mentioned = set()
    for m in re.finditer(re.escape(cfg["dir_prefix"]) + r"([a-z0-9][a-z0-9-]{0,30})", cmd):
        before = cmd[max(0, m.start() - 1):m.start()]
        if before in ("/", "\\"):
            mentioned.add(m.group(1))
    mutating = bool(MUTATING_GIT.search(cmd) or WRITERS.search(cmd))
    for lane in mentioned:
        e = lanes.get(lane, {})
        if e.get("owner") != sid and mutating:
            who = e.get("owner") or "nobody (closed)"
            return block(f"this command writes into lane worktree '{lane}' ({root}), owned by {who}, "
                         f"not this session. Read-only commands are fine; writes need your own lane.")
    mine = owned_lane(cfg, lanes, sid)
    if mine and MUTATING_GIT.search(cmd) and mine not in mentioned:
        return block(f"this session owns lane '{mine}', and this git command has no -C/cd pin to "
                     f"its worktree, so it would hit main. Pin it: git -C \"<lane worktree>\" ... "
                     f"(python tools/lanes/lanes.py path {mine}). Landing is lanes.py land {mine}.")
    return 0


def main() -> int:
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
        if tool in ("Edit", "Write", "MultiEdit", "NotebookEdit"):
            fp = ti.get("file_path") or ti.get("notebook_path") or ""
            return check_edit(cfg, lanes, sid, fp, cwd) if fp else 0
        if tool in ("Bash", "PowerShell"):
            return check_shell(cfg, lanes, sid, ti.get("command") or "")
        return 0
    except Exception as e:
        print(f"lane_guard: fail-open (error: {e})", file=sys.stderr)
        return 0


if __name__ == "__main__":
    sys.exit(main())
