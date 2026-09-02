#!/usr/bin/env python3
"""Re-adjudication sweep after a family grant.

WHY (post-mortem 2026-09-01): func_80057CC8's closing form sat in rejected/
at score 0 from s8; the F3 grant (2026-08-18) covered it; the ledger's ban and
three reviews kept citing the 2026-07-20 refusal for 18 more sessions. A
grant must reach every ledger it affects the day it lands.

Usage (repo root, Windows or WSL):
  python tools/grinder/grant_rescan.py --term "compound-address duplication" --term "call arg-list"
  python tools/grinder/grant_rescan.py --term ... --apply --family "F3 compound-address duplication" \\
        --ref ".claude/rules/no-new-park-categories.md:377"

Without --apply: prints every ledger whose rejected/* headers, banned
constructs, judge constraints, hypotheses, evidence, or candidate.c mention
any term (case-insensitive substring).
With --apply, for each hit:
  * appends a RE-ADJUDICATE judge constraint naming the grant and the hits;
  * moves matching bans into state.json superseded_bans (audit-preserved) so
    check_banned_constructs no longer auto-discards the resubmission;
  * if the queue item is foreclosed/parked/escalated, returns it to active
    via engine.queue.mark_unparked with the grant as the reason.
The resubmission still passes sandbox 0, self-vet, layer-1, full-build SHA1,
and the Judge -- this widens nothing but the ledger's memory. Review the
hit list BEFORE --apply: a keyword collision would clear an unrelated ban.
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


def _ledger_texts(root: str, func: str) -> list[tuple[str, str]]:
    d = G.ledger_dir(root, func)
    st = G.load_state(root, func) or {}
    out = [(f"banned_constructs[{i}]", str(b)) for i, b in enumerate(st.get("banned_constructs", []))]
    out += [(f"judge_constraints[{i}]", str(c)) for i, c in enumerate(st.get("judge_constraints", []))]
    for name in ("hypotheses.md", "evidence.md", "candidate.c"):
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


def scan(root: str, terms: list[str]) -> dict[str, list[str]]:
    base = os.path.join(root, "memory", "grind")
    hits: dict[str, list[str]] = {}
    if not os.path.isdir(base):
        return hits
    low = [t.lower() for t in terms if t.strip()]
    for func in sorted(os.listdir(base)):
        if not os.path.isfile(os.path.join(base, func, "state.json")):
            continue
        for where, text in _ledger_texts(root, func):
            tl = text.lower()
            for t in low:
                if t in tl:
                    hits.setdefault(func, []).append(f"{where}: '{t}'")
    return hits


def apply(root: str, hits: dict[str, list[str]], family: str, ref: str, date: str) -> None:
    for func, where in hits.items():
        needles = sorted({w.split("'")[1] for w in where if "'" in w})
        n = G.supersede_bans(root, func, needles, f"{family} ({ref}, {date})")
        G.add_judge_constraint(root, func, (
            f"RE-ADJUDICATE (grant rescan {date}): family '{family}' granted at {ref} covers "
            f"terms {needles}. Hits: {'; '.join(where[:6])}. Restore the matching banked form, "
            "re-measure on the current chassis, and if it reaches 0 submit under that family with "
            f"its SCOPE quoted verbatim. {n} superseded ban(s) moved to superseded_bans."))
        try:
            cwd = os.getcwd()
            os.chdir(root)
            try:
                if root not in sys.path:
                    sys.path.insert(0, root)
                from engine import queue as Q
                item = next((i for i in Q.load().get("items", []) if i.get("func") == func), None)
                if item and item.get("status") in ("foreclosed", "parked", "escalated"):
                    Q.mark_unparked(func, reason=f"grant rescan {date}: {family} ({ref})")
            finally:
                os.chdir(cwd)
        except Exception as e:  # engine unavailable (unit tests / Windows without venv)
            print(f"  (queue not updated for {func}: {e})", file=sys.stderr)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--term", action="append", required=True, help="search term (repeatable, case-insensitive)")
    ap.add_argument("--apply", action="store_true")
    ap.add_argument("--family", default="", help="granted family name (required with --apply)")
    ap.add_argument("--ref", default="", help="file:line of the grant (required with --apply)")
    ap.add_argument("--date", default="", help="grant date YYYY-MM-DD (default today)")
    a = ap.parse_args()
    hits = scan(str(ROOT), a.term)
    if not hits:
        print("no ledger mentions any term")
        return 0
    for func, where in hits.items():
        print(f"{func}:")
        for w in where:
            print(f"  {w}")
    if a.apply:
        if not (a.family and a.ref):
            print("--apply requires --family and --ref", file=sys.stderr)
            return 2
        apply(str(ROOT), hits, a.family, a.ref, a.date or datetime.date.today().isoformat())
        print(f"applied to {len(hits)} ledger(s); commit memory/grind + engine/queue.json")
    return 0


if __name__ == "__main__":
    sys.exit(main())
