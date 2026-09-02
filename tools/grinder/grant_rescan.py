#!/usr/bin/env python3
"""Re-adjudication sweep after a family grant.

WHY (post-mortem 2026-09-01): func_80057CC8's closing form sat in rejected/
at score 0 from s8; the F3 grant (2026-08-18) covered it; the ledger's ban and
three reviews kept citing the 2026-07-20 refusal for 18 more sessions. A
grant must reach every ledger it affects the day it lands.

Run from the REPO ROOT (`root` is the repo root: the ledgers are read from
`<root>/memory/grind`, the queue from `<root>/engine/queue.json`).

Usage:
  # 1. dry run -- ALWAYS read the WOULD SUPERSEDE lines before applying
  python tools/grinder/grant_rescan.py --term "compound-address duplication" --bans-only
  # 2. apply, narrowed to the ledgers you actually vetted
  python tools/grinder/grant_rescan.py --term "compound-address duplication" --apply \\
        --only func_8002D518 --family "F3 compound-address duplication" \\
        --ref ".claude/rules/no-new-park-categories.md:377"

Without --apply: prints every ledger whose rejected/* headers, banned
constructs, judge constraints, hypotheses, evidence, or candidate.c mention
any term (case-insensitive substring), plus, per ledger, every ban the run
WOULD supersede.
With --apply, for each selected hit:
  * appends a RE-ADJUDICATE judge constraint naming the grant and the hits;
  * moves matching bans into state.json superseded_bans (audit-preserved) so
    check_banned_constructs no longer auto-discards the resubmission;
  * if the queue item is foreclosed/parked/escalated, returns it to active
    via engine.queue.mark_unparked with the grant as the reason.

Blast-radius guards (--apply only; a dry run is never gated):
  * the GRINDER MUST BE STOPPED -- this writes ledgers and engine/queue.json,
    and queue dirt discards a live grind session. --apply refuses while
    tmp/grind/grind.lock names a live pid; run `pwsh tools/grinder/grind.ps1
    -Stop` and confirm the pidfile clears first.
  * --only <func> (repeatable) restricts application to named ledgers.
  * --max-hits N (default 3) refuses to modify more ledgers than N unless --yes.
  * --bans-only drops ledgers whose only hits are prose (candidate.c /
    hypotheses.md / evidence.md) with no ban, constraint or rejected/ hit.
Needles are the matched TERMS themselves -- never re-parsed out of a formatted
line, which an apostrophe in a term used to truncate ("caller's frame" ->
"caller") and so cleared unrelated bans.

The resubmission still passes sandbox 0, self-vet, layer-1, full-build SHA1,
and the Judge -- this widens nothing but the ledger's memory.
"""
from __future__ import annotations

import argparse
import datetime
import os
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))
from tools.grinder import grindlib as G  # noqa: E402

SNIPPET = 80
# Hits here are the Judge's own record (a ban, a constraint, a banked rejected
# form). A mention in candidate.c / hypotheses.md / evidence.md is prose -- it
# can be the whole point (the banked form's rationale) or an incidental word,
# so --bans-only exists to drop the latter.
_SOFT = ("candidate.c", "hypotheses.md", "evidence.md")


def _alive(pid: int) -> bool:
    """True if `pid` names a live process. PermissionError == alive (the
    process exists, we just do not own it)."""
    try:
        os.kill(pid, 0)
        return True
    except PermissionError:
        return True
    except (OSError, ValueError, OverflowError):
        return False


def grinder_lock_pid(root: str) -> int | None:
    """The live grinder pid from tmp/grind/grind.lock, else None (absent lock,
    unreadable lock, or a stale pid left by a crashed driver)."""
    p = os.path.join(root, "tmp", "grind", "grind.lock")
    try:
        with open(p, encoding="utf-8", errors="replace") as f:
            pid = int(f.read().strip())
    except (OSError, ValueError):
        return None
    return pid if _alive(pid) else None


def _ledger_texts(root: str, func: str) -> list[tuple[str, str]]:
    d = G.ledger_dir(root, func)
    st = G.load_state(root, func) or {}
    out = [(f"banned_constructs[{i}]", str(b)) for i, b in enumerate(st.get("banned_constructs", []))]
    out += [(f"judge_constraints[{i}]", str(c)) for i, c in enumerate(st.get("judge_constraints", []))]
    for name in _SOFT:
        p = os.path.join(d, name)
        if os.path.isfile(p):
            try:
                with open(p, encoding="utf-8", errors="replace") as f:
                    out.append((name, f.read()))
            except OSError:
                pass
    rj = os.path.join(d, "rejected")
    if os.path.isdir(rj):
        for fn in sorted(os.listdir(rj)):
            try:
                with open(os.path.join(rj, fn), encoding="utf-8", errors="replace") as f:
                    out.append((f"rejected/{fn}", "".join(f.readlines()[:80])))
            except OSError:
                pass
    return out


def _snippet(text: str, at: int, term_len: int) -> str:
    pad = max(0, (SNIPPET - term_len) // 2)
    lo = max(0, at - pad)
    s = " ".join(text[lo:lo + SNIPPET].split())
    return s[:SNIPPET]


def scan(root: str, terms: list[str]) -> dict[str, list[dict]]:
    """{func: [{"where", "term", "text_snippet"}, ...]} for every ledger under
    <root>/memory/grind mentioning a term (case-insensitive substring).
    One hit per (location, term) pair -- the first occurrence."""
    base = os.path.join(root, "memory", "grind")
    hits: dict[str, list[dict]] = {}
    if not os.path.isdir(base):
        return hits
    wanted = [t for t in terms if t.strip()]
    for func in sorted(os.listdir(base)):
        if not os.path.isfile(os.path.join(base, func, "state.json")):
            continue
        for where, text in _ledger_texts(root, func):
            tl = text.lower()
            for t in wanted:
                at = tl.find(t.lower())
                if at >= 0:
                    hits.setdefault(func, []).append(
                        {"where": where, "term": t,
                         "text_snippet": _snippet(text, at, len(t))})
    return hits


def needles(hitlist: list[dict]) -> list[str]:
    """The matched TERMS -- the only safe needle source (see module docstring)."""
    return sorted({h["term"] for h in hitlist})


def filter_bans_only(hits: dict[str, list[dict]]) -> dict[str, list[dict]]:
    """Drop ledgers whose hits are all prose (no ban, constraint or rejected/)."""
    return {f: hl for f, hl in hits.items()
            if any(h["where"] not in _SOFT for h in hl)}


def format_hits(hits: dict[str, list[dict]]) -> str:
    lines = []
    for func, hl in hits.items():
        lines.append(f"{func}:")
        for h in hl:
            lines.append(f"  {h['where']}: {h['term']!r} -- {h['text_snippet']}")
    return "\n".join(lines)


def _queue_update(root: str, func: str, reason: str) -> str:
    """Return a one-line report of the queue change (empty if none).
    Sets engine.queue.QUEUE_PATH explicitly rather than chdir-ing: the module
    reads/writes that one attribute for load/save/lock, and mutating cwd under
    a concurrent caller is the worse of the two hazards."""
    if root not in sys.path:
        sys.path.insert(0, root)
    from engine import queue as Q
    prev = Q.QUEUE_PATH
    Q.QUEUE_PATH = os.path.join(root, "engine", "queue.json")
    try:
        item = next((i for i in Q.load().get("items", []) if i.get("func") == func), None)
        if item is None:
            return ""
        was = item.get("status")
        if was not in ("foreclosed", "parked", "escalated"):
            return ""
        r = Q.mark_unparked(func, reason=reason)
        if not r.get("ok"):
            raise RuntimeError(r.get("reason", "mark_unparked refused"))
        return f"queue: {func} {was} -> active"
    finally:
        Q.QUEUE_PATH = prev


def apply(root: str, hits: dict[str, list[dict]], family: str, ref: str, date: str) -> int:
    """Apply the grant to every ledger in `hits`. Returns the number of
    functions whose QUEUE update failed (ledger writes are reported per line);
    a non-zero return is the caller's exit code."""
    failures = 0
    for func, hl in hits.items():
        ns = needles(hl)
        moved = G.supersede_bans(root, func, ns, f"{family} ({ref}, {date})")
        if moved:
            print(f"{func}: superseded {len(moved)} ban(s): "
                  f"{[m['text'][:60] for m in moved]}")
        else:
            print(f"{func}: no bans affected")
        where = [f"{h['where']}: {h['term']!r}" for h in hl]
        G.add_judge_constraint(root, func, (
            f"RE-ADJUDICATE (grant rescan {date}): family '{family}' granted at {ref} covers "
            f"terms {ns}. Hits: {'; '.join(where[:3])}. Restore the matching banked form, "
            "re-measure on the current chassis, and if it reaches 0 submit under that family with "
            f"its SCOPE quoted verbatim. {len(moved)} superseded ban(s) moved to superseded_bans."
        )[:400])
        print(f"{func}: RE-ADJUDICATE constraint added")
        try:
            line = _queue_update(root, func, f"grant rescan {date}: {family} ({ref})")
            if line:
                print(line)
        except Exception as e:  # engine unavailable / queue refused the update
            failures += 1
            print(f"  (queue NOT updated for {func}: {e})", file=sys.stderr)
    return failures


def _parser() -> argparse.ArgumentParser:
    ap = argparse.ArgumentParser(
        prog="grant_rescan.py", description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--term", action="append", required=True,
                    help="search term (repeatable, case-insensitive)")
    ap.add_argument("--apply", action="store_true")
    ap.add_argument("--only", action="append", default=[],
                    help="restrict application to this func (repeatable)")
    ap.add_argument("--max-hits", type=int, default=3,
                    help="refuse --apply above this many ledgers unless --yes (default 3)")
    ap.add_argument("--yes", action="store_true", help="confirm a wide --apply")
    ap.add_argument("--bans-only", action="store_true",
                    help="skip ledgers whose only hits are prose (candidate.c/hypotheses.md/evidence.md)")
    ap.add_argument("--family", default="", help="granted family name (required with --apply)")
    ap.add_argument("--ref", default="", help="file:line of the grant (required with --apply)")
    ap.add_argument("--date", default="", help="grant date YYYY-MM-DD (default today)")
    return ap


def run(argv: list[str] | None = None, root: str | None = None) -> int:
    a = _parser().parse_args(argv)
    root = str(root or ROOT)
    hits = scan(root, a.term)
    if a.bans_only:
        hits = filter_bans_only(hits)
    if not hits:
        print("no ledger mentions any term")
        return 0
    for func, hl in hits.items():
        print(f"{func}:")
        for h in hl:
            print(f"  {h['where']}: {h['term']!r} -- {h['text_snippet']}")
        pv = G.preview_superseded_bans(root, func, needles(hl))
        for b in pv:
            print(f"  WOULD SUPERSEDE: {b}")
        if not pv:
            print("  (no bans affected)")
    if not a.apply:
        return 0
    if not (a.family and a.ref):
        print("--apply requires --family and --ref", file=sys.stderr)
        return 2
    date = a.date or datetime.date.today().isoformat()
    try:
        datetime.date.fromisoformat(date)
    except ValueError:
        print(f"--date must be YYYY-MM-DD, got {date!r}", file=sys.stderr)
        return 2
    pid = grinder_lock_pid(root)
    if pid is not None:
        print(f"REFUSING --apply: the grinder is running (tmp/grind/grind.lock pid {pid}). "
              "This writes ledgers + engine/queue.json and queue dirt discards a live "
              "session. Run `pwsh tools/grinder/grind.ps1 -Stop`, confirm the pidfile "
              "clears, then re-run.", file=sys.stderr)
        return 2
    if a.only:
        missing = [f for f in a.only if f not in hits]
        if missing:
            print(f"--only names ledgers with no hits: {missing}", file=sys.stderr)
            return 2
        hits = {f: hl for f, hl in hits.items() if f in a.only}
    if len(hits) > a.max_hits and not a.yes:
        print(f"REFUSING --apply: {len(hits)} ledgers would be modified (--max-hits "
              f"{a.max_hits}). Narrow with --only/--bans-only/a more specific --term, "
              "or pass --yes if the hit list above is genuinely all covered by the grant.",
              file=sys.stderr)
        return 2
    failures = apply(root, hits, a.family, a.ref, date)
    print(f"applied to {len(hits)} ledger(s); commit memory/grind + engine/queue.json")
    return 1 if failures else 0


def main() -> int:
    return run()


if __name__ == "__main__":
    sys.exit(main())
