#!/usr/bin/env python3
"""PreToolUse hook: one session per lane worktree, and lanes stay out of main and out of each
other's claims (tools/lanes/lanes.py).

Blocks (exit 2):
  Edit|Write|MultiEdit|NotebookEdit
    - a file inside a lane worktree (bb2-worktrees/ln-<lane>) the session does not own;
    - a file another live lane has a path: claim on (inside any lane worktree);
    - a file in the MAIN checkout (outside tmp/) while the session owns a lane: lane work
      happens in the lane worktree (a subagent's cwd is main, so relative paths land there).
  Bash|PowerShell (quoted strings are blanked before matching, so a quoted path can't hide a
  subcommand)
    - a possibly-writing command (git mutation, writer verb, redirect, python, wave/build tool)
      that names a lane worktree the session does not own; reads are fine;
    - for a session that owns a lane: a git mutation or wave/build tool (wteng.ps1,
      naming_wave.py, ...) not pinned to its worktree, or pinned to main; a writer verb or
      redirect whose target is in main outside tmp/.
    A command that is solely a `tools/lanes/lanes.py` call is allowed (it checks ownership itself).

Ownership holds until `lanes.py close` or a takeover, however stale the heartbeat. Sessions that
own no lane and write into no lane worktree are never affected. Fails open (exit 0) on any
internal error, with a note on stderr.
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
ARG = r"""("[^"]*"|'[^']*'|[^\s;&|()<>]+)"""
# matched against the command with quoted strings blanked (so a quoted path or message can't hide
# or fake a subcommand)
MUTATING_GIT = re.compile(
    r"\bgit(?:\s+(?:-[cC]\s+\S+|--\S+))*\s+"
    r"(commit|add|rm|mv|reset|checkout|switch|restore|rebase|merge|cherry-pick|revert|stash|"
    r"clean|apply|am|pull|push|tag|branch\s+-[dDfmM]|worktree\s+(?:add|remove|move|prune)|"
    r"update-ref|update-index)\b")
SEP = r"(?:^|[;&|(\n{]|&&|\|\|)\s*(?:&\s*)?"
WRITER_VERB = re.compile(
    SEP + r"(Set-Content|Out-File|Add-Content|New-Item|Remove-Item|Move-Item|Copy-Item|"
    r"Rename-Item|tee|sed|rm|rmdir|mv|cp|mkdir|touch|make|ni|del|erase|ren|rd|md)\b", re.IGNORECASE)
PYTHON_CMD = re.compile(SEP + r"(?:python3?|py)(?:\.exe)?\b", re.IGNORECASE)
REDIRECT = re.compile(r"(?<![\d>&=\-])>>?(?!\s*(?:\$null|&\d|/dev/null|nul\b))")
REDIRECT_TARGET = re.compile(r"(?<![\d>&=\-])>>?\s*" + ARG)
# matched against the raw command (these names are usually inside a quoted path)
TOOL_WRITERS = re.compile(r"(wteng\.ps1|naming_wave\.py|data_wave\.py|naming_member_apply\.py|"
                          r"format\.py|wavefollow\.py|build_census\.py|reintegrate_lock\.ps1)",
                          re.IGNORECASE)
LANES_ONLY = re.compile(r"""^\s*(?:&\s*)?(?:python3?|py)(?:\.exe)?\s+["']?[^;&|\n"']*"""
                        r"""tools[\\/]+lanes[\\/]+lanes\.py["']?(?:\s+[^;&|\n<>`$]*)?\s*$""",
                        re.IGNORECASE)
PIN = re.compile(r"(?:\s-C\s+|(?i:\b(?:cd|set-location|push-location|pushd|sl)\s+(?:-LiteralPath\s+|-Path\s+)?))" + ARG)


def blank_quotes(cmd: str) -> str:
    return re.sub(r""""[^"]*"|'[^']*'""", "Q", cmd)


def unq(tok: str) -> str:
    return tok[1:-1] if len(tok) >= 2 and tok[0] == tok[-1] and tok[0] in "\"'" else tok


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
    return bool(e.get("owner")) and time.time() - float(e.get("heartbeat", 0)) <=         float(cfg.get("stale_hours", 12)) * 3600


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
            if kind == "path" and fnmatch.fnmatchcase(rel.lower(), val.lower()):
                return other, tok
    return None


def owned_lane(cfg, lanes, sid):
    """The lane this session owns. Ownership holds until close or takeover, stale or not."""
    for lane, e in lanes.items():
        if sid and e.get("owner") == sid:
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


def where(cfg, path: str, cwd: str):
    """'main' (main outside tmp/), 'tmp' (main's tmp/), ('lane', name), or 'outside'."""
    p = unq(path)
    if not p or p.startswith("$") or p.startswith("-"):
        return "outside"
    p = to_win(p)
    if not os.path.isabs(p):
        p = os.path.join(cwd, p)
    hit = lane_of_path(cfg, p)
    if hit:
        return ("lane", hit[0])
    n, m = norm(p), norm(str(MAIN))
    if n == m or n.startswith(m + os.sep):
        return "tmp" if (n + os.sep).startswith(norm(str(MAIN / "tmp")) + os.sep) else "main"
    return "outside"


def write_targets(cmd: str):
    """Arguments of writer verbs, and redirect targets (best effort: every path-like argument of
    a writer counts, sources included, which errs toward blocking)."""
    out = [m.group(1) for m in REDIRECT_TARGET.finditer(cmd)]
    for seg in re.split(r"&&|\|\||[;|\n]", cmd):
        mv = WRITER_VERB.search(blank_quotes(seg))
        if mv:
            at = seg.lower().find(mv.group(1).lower()) + len(mv.group(1))
            out += [t for t in re.findall(ARG, seg[at:]) if not unq(t).startswith("-")]
    return out


def check_shell(cfg, lanes, sid, cmd, cwd) -> int:
    if LANES_ONLY.match(cmd):
        return 0  # lanes.py enforces ownership itself
    bq = blank_quotes(cmd)
    mentioned = {m.group(1).lower() for m in
                 re.finditer(r"[\\/]" + re.escape(cfg["dir_prefix"]) + r"([a-z0-9][a-z0-9-]{0,30})",
                             cmd, re.IGNORECASE)}
    git_mut = bool(MUTATING_GIT.search(bq))
    verb = bool(WRITER_VERB.search(bq) or REDIRECT.search(bq))
    tool = bool(TOOL_WRITERS.search(cmd))
    py = bool(PYTHON_CMD.search(bq))
    mine = owned_lane(cfg, lanes, sid)

    for lane in mentioned:
        e = lanes.get(lane, {})
        if e.get("owner") != sid and (git_mut or verb or tool or py):
            who = e.get("owner") or "nobody (closed)"
            return block(f"this command may write into lane worktree '{lane}', owned by {who}, not "
                         f"this session. Read-only commands are fine; writes need your own lane.")
    if not mine:
        return 0

    pins = [where(cfg, m.group(1), cwd) for m in PIN.finditer(" " + cmd)]
    if (git_mut or tool) and "main" in pins:
        return block(f"this session owns lane '{mine}', and this command is pinned to main. "
                     f"Work in the lane worktree; landing is lanes.py land {mine}.")
    if (git_mut or tool) and mine not in mentioned:
        return block(f"this session owns lane '{mine}', and this command has no pin to its "
                     f"worktree, so it would run against main (a subagent's cwd is main). Pin it: "
                     f"git -C \"<wt>\" ..., cd \"<wt>\" && ..., or tools/wteng.ps1 \"<wt>\" ... "
                     f"(python tools/lanes/lanes.py path {mine}).")
    if verb:
        # a relative target after a `cd <lane>` resolves in the lane, not main
        lane_pinned = any(isinstance(p, tuple) for p in pins)
        for t in write_targets(cmd):
            if lane_pinned and not os.path.isabs(to_win(unq(t))):
                continue
            if where(cfg, t, cwd) == "main":
                return block(f"this session owns lane '{mine}', and this command writes into main "
                             f"({unq(t)}). Main is only a landing pad; write in the lane worktree, "
                             f"or under main's tmp/ for scratch.")
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
            return check_shell(cfg, lanes, sid, ti.get("command") or "", cwd)
        return 0
    except Exception as e:
        print(f"lane_guard: fail-open (error: {e})", file=sys.stderr)
        return 0


if __name__ == "__main__":
    sys.exit(main())
