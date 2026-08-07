#!/usr/bin/env python3
"""Canonical-conversion PAYOFF check (promoted from tmp/w6_canon_check.py,
task #19 — tmp/ is unbacked and the check recurs for every future
canonical-authorization wave).

For a canonical-authorized function the conversion's success criterion is the
opposite of an R6-style still-INCOMPLETE check: it must now reach a COMPLETED
state. All four must hold:
  1. it is listed in inline_asm_canonical.txt
  2. ZERO regfix/asmfix rules remain keyed to it
  3. a read-only regen DROPS it from the queue. Per engine/queue.py that IS
     the canonical completion — "regen must drop them too, or they sit in the
     queue forever". Before conversion its live asmfix rule kept it as a
     CANON-EXTRACT/authorize item.
  4. a read-only mark_done simulation SUCCEEDS, reaching
     COMPLETED-INLINE-ASM-CANONICAL. This is run against the COMMITTED queue,
     which still lists the function; running it against the regen would ask
     mark_done about an item regen has already (correctly) dropped and get
     "not in queue" for every one of them.

Read-only contract:
  * engine/queue.json is never written (QUEUE_PATH points at a temp copy).
  * THE BUILD IS NEVER TRIGGERED. The original tmp/ version violated this:
    Q.mark_done() calls oracle.verify(rebuild=False), and oracle.verify runs
    pipeline.build_all() whenever build/bb2.exe is MISSING — so a "check"
    script could silently seize the build lock and kick a minutes-long full
    build (a hazard while the Grinder holds the lane). This version refuses
    up front when build/bb2.exe is absent instead of building it.
    NOTE: Q.generate() still compiles per-file CHEAT-STRIPPED objects into a
    temp workdir (that is what regen is); it does not touch build/ or the
    full-link oracle path.

usage: canon_payoff_check.py <func> [<func> ...]
       canon_payoff_check.py <batchfile.txt>
"""
import json
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from engine import cheats, queue as Q

funcs = sys.argv[1:]
if len(funcs) == 1 and funcs[0].endswith(".txt"):
    funcs = [l.split("#")[0].strip()
             for l in Path(funcs[0]).read_text().splitlines()]
    funcs = [f for f in funcs if f]
if not funcs:
    sys.exit("usage: canon_payoff_check.py <func> ... | <batchfile.txt>")

# Build-trigger guard (the promoted fix): mark_done -> oracle.verify(rebuild
# =False) -> pipeline.build_all() fires when build/bb2.exe is missing. A check
# script must never be the thing that starts a build.
if not Path("build/bb2.exe").exists():
    sys.exit("build/bb2.exe is missing — refusing to run: the mark_done "
             "simulation would trigger a FULL build via oracle.verify(). "
             "Run `verify-oracle --rebuild` (or `build`) first, then re-run "
             "this check against the fresh build/.")

canon = cheats.canonical_asm_funcs()
committed = json.loads(Path("engine/queue.json").read_text())

with tempfile.TemporaryDirectory() as td:
    qp = Path(td) / "queue.json"
    orig = Q.QUEUE_PATH
    Q.QUEUE_PATH = str(qp)
    try:
        qp.write_text(Path("engine/queue.json").read_text())
        regen = Q.generate(workdir=f"{td}/wd")
        regen_items = {it["func"]: it for it in regen["items"]}
        done = {}
        for f in funcs:
            # Restore the COMMITTED queue each time: it still lists these
            # functions, so mark_done is exercised on the gate rather than on
            # regen's already-correct drop.
            qp.write_text(Path("engine/queue.json").read_text())
            done[f] = Q.mark_done(f)
    finally:
        Q.QUEUE_PATH = orig

print(f"regen: {len(regen_items)} items (committed queue has "
      f"{committed['counts']['total']}); engine/queue.json untouched\n")

fail = 0
for f in funcs:
    rules = Q._rule_count(f)
    it = regen_items.get(f)
    dn = done[f]

    c1 = f in canon
    c2 = rules == 0
    c3 = it is None  # dropped by regen == canonical completion
    state = dn.get("completion") or dn.get("completion_state")
    c4 = dn.get("ok") is True and state == "COMPLETED-INLINE-ASM-CANONICAL"
    ok = c1 and c2 and c3 and c4
    fail += 0 if ok else 1
    print(f"{'PASS' if ok else 'FAIL'}  {f}")
    print(f"      canonical-authorized={c1}   [{'ok' if c1 else 'BAD'}]")
    print(f"      rules={rules}   [{'ok' if c2 else 'BAD'}]")
    print(f"      regen     {('status=' + it['status'] + ' verdict=' + str(it.get('verdict'))) if it else 'ABSENT (dropped = completed)'}   [{'ok' if c3 else 'BAD'}]")
    print(f"      mark_done ok={dn.get('ok')} state={state}"
          f"{'' if dn.get('ok') else ' reason=' + str(dn.get('reason'))[:120]}   [{'ok' if c4 else 'BAD'}]")

print(f"\n{len(funcs) - fail}/{len(funcs)} passed")
sys.exit(1 if fail else 0)
