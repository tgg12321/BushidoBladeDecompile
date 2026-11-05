#!/usr/bin/env python3
"""heal.py: move `blocked` naming rows back to `idea` once what they wait on has landed.

    python tools/lanes/heal.py scan     list the rows that would move (changes nothing)
    python tools/lanes/heal.py apply    rewrite docs/naming/phase3/backlog.tsv in this tree
                                        (deterministic; the heal commit's Lane-Replay command)
    python tools/lanes/heal.py land     the orchestrator's flow: open lane `heal`, apply, commit,
                                        `lanes.py land`, close; result in tmp/lanes-state/heal.json

A naming row is `blocked` while it waits on a trace capture or on a wave no row names yet. A
capture unit adds an evidence line to the row's note (` EVIDENCE <capture id> (...)`); a session
that blocks a row on a wave states it in orchestrate.py's dependency spelling, ` Lands after
<alt> [| <alt> ...]` (or `Order: after` / `depends on`), each alternative a naming kind
(`kind:data`, `kind:member`, `kind:func`, ...) or a backlog row id (either backlog). A row moves
back to `idea` when, against the commit that last made it `blocked`, its note at HEAD has more
EVIDENCE lines, or every dependency it states is met and one became met since then: a kind
alternative is met when a naming row of that kind other than the row itself is `landed` now and
was not (or did not exist) at the blocking commit; a row id when that row is landed or dropped
(as the dispatcher counts it). A phrase whose words name no row or kind is prose and is ignored;
so is one already in the note before the row was blocked (notes accumulate: an old phrase must
not release a later block). A row-id phrase reads only its first id, as the dispatcher does.
The rule reads only this tree's history, so a replay onto a moved main gives the same rows.
"""
from __future__ import annotations

import json
import os
import re
import subprocess
import sys
import time
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
LANES_PY = Path(__file__).resolve().with_name("lanes.py")
BACKLOG = "docs/naming/phase3/backlog.tsv"
# an evidence line: EVIDENCE, an optional capture id, then "(<lane> <date>" naming who
# captured it when: ` EVIDENCE hud-x-1 (trace-a 2026-11-03, ...` or ` EVIDENCE (trace-a
# 2026-11-03, row ...): ...`. Prose ("two EVIDENCE lines below (trace-a, landed ...") has no
# date there and does not count; neither does heal.py's own lowercase "evidence line(s)".
EVIDENCE = re.compile(r"(?<![A-Za-z])EVIDENCE\b[^()\n]{0,120}?\(\s*[a-z][\w-]*\s+"
                      r"\d{4}-\d{2}-\d{2}")
# a wave dependency, in orchestrate.py's DEPENDS spelling: `lands after` / `Order: after` /
# `depends on`, then alternatives joined by `|`, each `kind:<naming kind>` or a row id (in
# backticks or not). heal.py's own note prefix never contains these phrases.
_TOKEN = r"`?(?:kind:[a-z]+|[a-z][\w.-]*[a-z0-9])`?"
DEPENDS = re.compile(rf"(?i)\b(?:lands after|order: after|depends on)\s+"
                     rf"({_TOKEN}(?:\s*\|\s*{_TOKEN})*)")
# what `scan` warns about: kind alternatives joined by , / or instead of |
DEPENDS_MORE = re.compile(r"(?i)\s*(?:,|/|\bor\b|\band\b)\s*`?kind:[a-z]+")
DEP_DONE = {"landed", "dropped"}  # a row-id dependency is met as orchestrate.deps_met counts it
FOUNDATION = "docs/foundation/backlog.tsv"
# backlog commits walked back to find where a row was blocked; a row blocked longer ago is
# compared with the oldest version read (for EVIDENCE and dependencies alike)
HISTORY_LIMIT = 400
LANE = "heal"
SUBJECT = "docs: naming backlog -- {ids} blocked -> idea (evidence / dependency landed)"


def git(*a, cwd: Path = ROOT, check: bool = True) -> str:
    r = subprocess.run(["git", "-c", "core.quotepath=off", *a], cwd=cwd, capture_output=True,
                       text=True, encoding="utf-8", errors="replace")
    if check and r.returncode != 0:
        raise RuntimeError(f"git {' '.join(a)}: {r.stderr.strip()}")
    return r.stdout


def parse_rows(text: str) -> dict[str, dict]:
    lines = text.splitlines()
    if not lines:
        return {}
    head = lines[0].rstrip("\r").split("\t")
    out = {}
    for ln in lines[1:]:
        f = ln.split("\t")
        if len(f) >= len(head) and f[0]:
            out[f[0]] = dict(zip(head, f))
    return out


def evidence_count(note: str) -> int:
    return len(EVIDENCE.findall(note or ""))


def _phrase(m: re.Match) -> str:
    return " ".join(m.group(0).lower().split())


def _alternatives(m: re.Match) -> list[str]:
    """A phrase's alternatives, as the dispatcher reads them: `kind:` alternatives joined by `|`
    (the dispatcher reads `kind`, names no row and ignores them), or else the first row id alone
    (orchestrate.DEPENDS captures only it)."""
    alts = [t.strip().strip("`").lower() for t in m.group(1).split("|")]
    if alts[0].startswith("kind:"):
        return [a for a in alts if a.startswith("kind:")]
    return alts[:1]


def dependencies(note: str, before: str | None = None) -> list[list[str]]:
    """The alternatives of every dependency phrase in a note, in order. With `before` (the note
    just before the row was blocked), only phrases the blocking edit or a later one wrote: a
    phrase counts as many times as it occurs now beyond its occurrences in `before`, so a stale
    one left in an accumulated note never releases a later block."""
    seen = Counter(_phrase(m) for m in DEPENDS.finditer(before or ""))
    out = []
    for m in DEPENDS.finditer(note or ""):
        if seen[_phrase(m)] > 0:
            seen[_phrase(m)] -= 1
            continue
        out.append(_alternatives(m))
    return out


def deps_newly_met(rid: str, deps: list[list[str]], cur: dict, then: dict, other_cur: dict,
                   other_then: dict) -> list[str]:
    """`<alt> (<why>)` for the dependency alternatives of row `rid` that became met since the
    blocking version, or [] unless every recognized dependency is met now. Naming rows `cur` /
    `then` (now / at the blocking version), foundation rows `other_cur` / `other_then` likewise.
    An alternative naming no row or kind is ignored; a phrase with none recognized too."""
    def stage(rows, i):
        return (rows.get(i) or {}).get("stage")
    newly: list[str] = []
    known = 0
    for alts in deps:
        recognized = met = False
        for a in alts:
            if a.startswith("kind:"):
                if not any(r.get("kind") == a[5:] for r in cur.values()):
                    continue
                recognized = True
                hit = [i for i, r in cur.items() if i != rid and r.get("kind") == a[5:]
                       and r.get("stage") == "landed" and stage(then, i) != "landed"]
                if hit:
                    met = True
                    newly.append(f"{a} ({', '.join(hit)})")
                continue
            rows_now, rows_then = (cur, then) if a in cur else (other_cur, other_then)
            if a == rid or a not in rows_now:
                continue
            recognized = True
            if stage(rows_now, a) in DEP_DONE:
                met = True
                if stage(rows_then, a) not in DEP_DONE:
                    newly.append(f"{a} ({stage(rows_now, a)})")
        if recognized:
            known += 1
            if not met:
                return []
    return newly if known else []


def unblockable(history, other=None) -> list[tuple[str, str, str]]:
    """(row id, sha that blocked it, why) for every row to move.

    history yields (sha, rows at sha) newest first, the current version first. A row's blocking
    version is the oldest of the unbroken run of versions, from the current one back, where it
    is blocked (the run's end if the walk stops first). The walk stops once every row blocked
    now has found its blocking version, so with no blocked row it reads nothing more.
    other(sha) gives the foundation backlog's rows at one of those versions, other(None) its
    current rows (not the naming version's: a foundation row lands in commits that leave the
    naming backlog alone); without `other`, dependency row ids are looked up in the naming
    backlog only."""
    it = iter(history)
    first = next(it, None)
    if first is None:
        return []
    cur = first[1]
    pending = {rid for rid, r in cur.items() if r.get("stage") == "blocked"}
    found: dict[str, tuple[str, dict]] = {}
    before: dict[str, str | None] = {}  # the row's note in the version just before its block
    prev = first
    while pending:
        nxt = next(it, None)
        if nxt is None:
            break
        for rid in list(pending):
            if (nxt[1].get(rid) or {}).get("stage") != "blocked":
                found[rid] = prev
                before[rid] = (nxt[1].get(rid) or {}).get("note", "")
                pending.discard(rid)
        prev = nxt
    for rid in pending:  # blocked as far back as the walk went
        found[rid] = prev
        before[rid] = None
    cache: dict[str, dict] = {}

    def other_at(sha):
        if sha not in cache:
            cache[sha] = other(sha) if other else {}
        return cache[sha]

    out = []
    for rid in sorted(found, key=list(cur).index):
        sha, rows = found[rid]
        note = cur[rid].get("note", "")
        why = []
        new = evidence_count(note) - evidence_count(rows[rid].get("note", ""))
        if new > 0:
            why.append(f"{new} new evidence line(s)")
        deps = dependencies(note, before[rid])
        if deps:
            met = deps_newly_met(rid, deps, cur, rows, other_at(None), other_at(sha))
            if met:
                why.append("dependency met: " + "; ".join(met))
        if why:
            out.append((rid, sha, ", ".join(why)))
    return out


def load_history(root: Path = ROOT, limit: int = HISTORY_LIMIT):
    """Yields (sha, rows) for the working tree's backlog (only if it differs from HEAD's), then
    each commit that touched it, newest first (first parent: the landed order); lazily, one
    `git show` per version actually read."""
    work = parse_rows((root / BACKLOG).read_text(encoding="utf-8"))
    shas = git("log", "--first-parent", f"-n{limit}", "--format=%H", "--", BACKLOG, cwd=root).split()
    versions = iter(shas)

    def at(sha):
        try:
            return parse_rows(git("show", f"{sha}:{BACKLOG}", cwd=root))
        except RuntimeError:
            return {}  # the file did not exist there

    head_sha = next(versions, None)
    head = at(head_sha) if head_sha else {}
    if head_sha is None or work != head:
        yield "WORKTREE", work
    if head_sha is not None:
        yield head_sha, head
    for sha in versions:
        yield sha, at(sha)


def other_history(root: Path = ROOT):
    """unblockable's `other`: the foundation backlog's rows at a version load_history yields
    (None or "WORKTREE": the working tree's file; {} where the file does not exist)."""
    def at(sha):
        if sha in (None, "WORKTREE"):
            p = root / FOUNDATION
            return parse_rows(p.read_text(encoding="utf-8")) if p.exists() else {}
        try:
            return parse_rows(git("show", f"{sha}:{FOUNDATION}", cwd=root))
        except RuntimeError as e:
            # only a version without the file reads as empty: any other git failure would make
            # every landed foundation row look newly landed
            if "does not exist in" in str(e) or "exists on disk, but not in" in str(e):
                return {}
            raise
    return at


def moves_in(root: Path = ROOT) -> list[tuple[str, str, str]]:
    return unblockable(load_history(root), other_history(root))


def rewrite(text: str, moves: list[tuple[str, str, str]]) -> str:
    by_id = {rid: (sha, why) for rid, sha, why in moves}
    lines = text.split("\n")
    head = lines[0].rstrip("\r").split("\t")
    si, ni = head.index("stage"), head.index("note")
    for k, ln in enumerate(lines[1:], 1):
        f = ln.split("\t")
        if len(f) > max(si, ni) and f[0] in by_id and f[si] == "blocked":
            sha, why = by_id[f[0]]
            f[si] = "idea"
            f[ni] = f"Unblocked (heal.py): {why} since {sha[:9]} blocked it. " + f[ni]
            lines[k] = "\t".join(f)
    return "\n".join(lines)


def dep_warnings(root: Path = ROOT) -> list[tuple[str, str]]:
    """(row id, problem) for dependencies the tools cannot honour as written: on a blocked row, a
    `kind:<kind>` no other naming row has, kind alternatives joined by `,` / `/` / `or` / `and`
    instead of `|`, or a phrase mixing a row id with kinds; on an open (not blocked) row, any
    `kind:` dependency (the dispatcher ignores it and starts the row now)."""
    rows = parse_rows((root / BACKLOG).read_text(encoding="utf-8"))
    out = []
    for rid, r in rows.items():
        note, stage = r.get("note", ""), r.get("stage")
        if stage in ("landed", "dropped"):
            continue
        for m in DEPENDS.finditer(note):
            raw = [t.strip().strip("`").lower() for t in m.group(1).split("|")]
            kinds = [a for a in raw if a.startswith("kind:")]
            if not kinds:
                continue
            if stage != "blocked":
                out.append((rid, f"`{m.group(0)}` on a `{stage}` row: the dispatcher ignores "
                                 f"kind dependencies; stage it `blocked`"))
                continue
            if DEPENDS_MORE.match(note, m.end()):
                out.append((rid, f"`{m.group(0)}` is followed by `,` / `/` / `or` / `and`: join "
                                 f"alternatives with `|`"))
            if len(kinds) != len(raw):
                out.append((rid, f"`{m.group(0)}` mixes row ids with kinds: state a row id in "
                                 f"its own `Lands after`"))
            for a in kinds:
                if not any(i != rid and x.get("kind") == a[5:] for i, x in rows.items()):
                    out.append((rid, f"{a} names no other naming row's kind (never met)"))
    return out


def cmd_scan(root: Path = ROOT) -> list[tuple[str, str, str]]:
    moves = moves_in(root)
    for rid, sha, why in moves:
        print(f"{rid}: blocked at {sha[:9]}, {why} since -> idea")
    if not moves:
        print("no blocked naming row has new evidence or a newly met dependency")
    for rid, problem in dep_warnings(root):
        print(f"warning: {rid}: {problem}")
    return moves


def cmd_apply(root: Path = ROOT) -> list[tuple[str, str, str]]:
    moves = moves_in(root)
    if moves:
        p = root / BACKLOG
        p.write_text(rewrite(p.read_text(encoding="utf-8"), moves), encoding="utf-8", newline="\n")
    for rid, sha, why in moves:
        print(f"heal: {rid} blocked -> idea ({why} since {sha[:9]})")
    return moves


# ---------------------------------------------------------------- the orchestrator's land flow

def main_root() -> Path:
    """The main checkout, as lanes.py finds it ($BB2_LANES_MAIN, else this file's tree)."""
    return Path(os.environ.get("BB2_LANES_MAIN") or ROOT).resolve()


def state_dir() -> Path:
    return Path(os.environ.get("BB2_LANES_STATE_DIR") or main_root() / "tmp" / "lanes-state")


def lanes_py(session: str, *a, log) -> int:
    cmd = [sys.executable, str(LANES_PY), "--session", session, *a]
    log(f"$ lanes.py {' '.join(a)}")
    r = subprocess.run(cmd, cwd=main_root(), stdin=subprocess.DEVNULL, stdout=subprocess.PIPE,
                       stderr=subprocess.STDOUT, text=True, encoding="utf-8", errors="replace")
    log(r.stdout.rstrip())
    return r.returncode


def cmd_land() -> int:
    """Open lane `heal` under its own session id, commit the apply output with a Lane-Replay
    trailer, land it and close. Writes {rc, rows, step, log, ended} to heal.json."""
    sd = state_dir()
    sd.mkdir(parents=True, exist_ok=True)
    logp = sd / "logs" / "heal.log"
    logp.parent.mkdir(parents=True, exist_ok=True)
    session = f"heal-{os.getpid()}-{int(time.time())}"  # not a Claude session: never reaped
    result = {"rc": 1, "rows": [], "step": "start", "log": str(logp), "started": time.time()}

    with open(logp, "w", encoding="utf-8", newline="\n") as lf:
        def log(s):
            lf.write(s + "\n")
            lf.flush()

        def done(rc, step):
            result.update(rc=rc, step=step, ended=time.time())
            out = sd / "heal.json"
            tmp = out.with_suffix(".tmp")
            tmp.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8", newline="\n")
            os.replace(tmp, out)
            return rc

        # scan main first (here, not in the orchestrator's tick): open nothing if nothing moves
        try:
            for rid, problem in dep_warnings(main_root()):
                log(f"warning: {rid}: {problem}")
            if not moves_in(main_root()):
                log("nothing to unblock on main")
                return done(0, "nothing")
        except Exception as e:  # noqa: BLE001
            log(f"scan failed: {type(e).__name__}: {e}")
            return done(1, "scan")
        if lanes_py(session, "open", LANE, "--kind", "foundation", log=log):
            return done(2, "open")
        wt = lane_dir(log)
        if wt is None:
            lanes_py(session, "close", LANE, "--force", log=log)
            return done(2, "path")

        def fail(rc, step):
            # heal owns lane `heal` outright: never leave it dirty for the next run's `fresh`
            reset_lane(wt, log)
            lanes_py(session, "close", LANE, "--force", log=log)
            return done(rc, step)

        reset_lane(wt, log)  # a crashed earlier run may have left edits behind
        if lanes_py(session, "fresh", LANE, "--discard", log=log):
            return fail(2, "fresh")
        try:
            moves = cmd_apply(wt)
            result["rows"] = [m[0] for m in moves]
            if not moves:
                log("nothing to unblock on the current main")
                lanes_py(session, "close", LANE, "--remove", log=log)
                return done(0, "nothing")
            msg = sd / "heal-msg.txt"
            ids = ", ".join(result["rows"])
            body = "\n".join(f"- {rid}: {why} since {sha[:9]} set it blocked"
                             for rid, sha, why in moves)
            msg.write_text(f"{SUBJECT.format(ids=ids)}\n\nThe orchestrator's self-heal "
                           f"(tools/lanes/heal.py): a blocked row waits on a trace capture (an "
                           f"EVIDENCE line in its note) or on a wave (a `lands after` dependency), and "
                           f"that has landed.\n\n{body}\n\n"
                           f"Lane-Replay: python3 tools/lanes/heal.py apply\n",
                           encoding="utf-8", newline="\n")
            git("add", BACKLOG, cwd=wt)
            r = subprocess.run(["git", "-C", str(wt), "commit", "-q", "-F", str(msg)],
                               capture_output=True, text=True, encoding="utf-8", errors="replace")
            log(r.stdout + r.stderr)
            if r.returncode:
                return fail(3, "commit")
        except Exception as e:  # noqa: BLE001
            log(f"{type(e).__name__}: {e}")
            return fail(3, "apply")
        rc = lanes_py(session, "land", LANE, log=log)
        if rc:
            return fail(10 + rc, "land")
        lanes_py(session, "close", LANE, "--remove", log=log)
        return done(0, "landed")


def lane_dir(log) -> Path | None:
    """`lanes.py path heal`, checked: an absolute ln-heal directory that is not main."""
    r = subprocess.run([sys.executable, str(LANES_PY), "path", LANE], cwd=main_root(),
                       capture_output=True, text=True, encoding="utf-8", errors="replace")
    s = r.stdout.strip()
    p = Path(s) if s else None
    if (r.returncode or p is None or not p.is_absolute() or p.name != lane_dir_name()
            or p.resolve() == main_root()):
        log(f"`lanes.py path {LANE}` gave {s!r} (rc {r.returncode}); refusing to work there")
        return None
    return p


def lane_dir_name() -> str:
    """ln-heal: the configured dir_prefix (lanes.json in main) + LANE."""
    try:
        cfg = json.loads((main_root() / "tools" / "lanes" / "lanes.json").read_text(encoding="utf-8"))
        return cfg.get("dir_prefix", "ln-") + LANE
    except (OSError, ValueError):
        return "ln-" + LANE


def reset_lane(wt: Path, log):
    """Discard uncommitted edits in the heal lane's own worktree (only that one)."""
    if (wt / ".git").exists() and wt.resolve() != main_root():
        r = subprocess.run(["git", "-C", str(wt), "reset", "-q", "--hard", "HEAD"],
                           capture_output=True, text=True, encoding="utf-8", errors="replace")
        if r.returncode:
            log(f"reset of {wt} failed: {r.stderr.strip()}")


def main(argv=None) -> int:
    for s in (sys.stdout, sys.stderr):
        try:
            s.reconfigure(encoding="utf-8", errors="backslashreplace")
        except (AttributeError, ValueError, OSError):
            pass
    argv = list(sys.argv[1:] if argv is None else argv)
    cmd = argv[0] if argv else "scan"
    if cmd == "scan":
        cmd_scan()
        return 0
    if cmd == "apply":
        cmd_apply()
        return 0
    if cmd == "land":
        return cmd_land()
    print(__doc__)
    return 2


if __name__ == "__main__":
    sys.exit(main())
