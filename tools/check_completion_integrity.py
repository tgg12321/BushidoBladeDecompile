#!/usr/bin/env python3
"""Standing completion-integrity guard.

Every function in the codebase is one of three categories:

  INCOMPLETE                     — in engine/queue.json. Carries a regfix/asmfix
                                   rule, a cheat construct (cheat-asm,
                                   register hint, volatile coercion), OR a
                                   non-zero honest pure-C distance. Stays in
                                   the queue until it reaches a COMPLETED state.
  COMPLETED-C                    — zero rules, zero cheat constructs in source,
                                   byte-matches as-built. NOT in queue.json. NOT
                                   listed in inline_asm_canonical.txt.
  COMPLETED-INLINE-ASM-CANONICAL — zero rules, canonical inline asm (GTE/cop2/
                                   BIOS/HW) OR whole-body `__asm__("glabel...")`
                                   that is its accepted finished form. Listed in
                                   inline_asm_canonical.txt. NOT in queue.json.

This tool audits that the invariant holds: every function with rules > 0 or
cheat constructs > 0 is either in the queue (INCOMPLETE) or authorized in
inline_asm_canonical.txt (COMPLETED-INLINE-ASM-CANONICAL). Anything else is a
cheated "completion" that snuck past the gate.

The two PREVENTION mechanisms are:
  - engine.queue.mark_done() refuses to record a function as done if it carries
    rules or non-canonical cheat constructs — and on success REMOVES it from the queue
    (queue presence = INCOMPLETE);
  - engine.queue.generate() recomputes status from scratch (only `parked` is
    sticky), so a cheated state that landed via manual edit is re-opened.

This tool DETECTS any cheated completion that slipped past those gates (manual
edit to the queue, a missed cheat pattern, etc.). Exit code 1 on any violation;
run it manually, in CI, or from a hook.
"""
import json
import os
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO))
# Engine modules use relative paths (regfix.txt, asmfix.txt, src/<stem>.c,
# build/src/<stem>.o) keyed off the working directory — chdir to REPO so they
# resolve correctly regardless of where this tool was invoked from.
os.chdir(REPO)
from engine import cheats, inlineasm, pipeline as P, score  # noqa: E402

QUEUE = REPO / "engine" / "queue.json"


def main() -> int:
    in_queue: set[str] = set()
    if QUEUE.exists():
        q = json.loads(QUEUE.read_text())
        in_queue = {it["func"] for it in q.get("items", [])}
    canon = cheats.canonical_asm_funcs()

    violations: list[str] = []
    total_completed_c = 0
    total_completed_canon = 0

    # Walk every C source file and check every function defined in it.
    # Use RELATIVE paths — score._o_func_table shells out via objdump, and an
    # absolute path containing spaces ("Bushido Blade 2 Decompile") gets split
    # by the shell. We chdir'd to REPO above, so relative paths work fine.
    for stem in sorted(P.c_stems()):
        ref_o = f"build/src/{stem}.o"
        if not Path(ref_o).exists():
            continue
        for func in score._o_func_table(ref_o):
            if func in in_queue:
                continue  # INCOMPLETE — the queue covers it
            # Function is not in the queue: must be one of the COMPLETED states.
            rules = (len(cheats.func_rule_lines(func, cheats.REGFIX))
                     + len(cheats.func_rule_lines(func, cheats.REGFIX2))
                     + len(cheats.func_rule_lines(func, cheats.ASMFIX)))
            cheat_count = inlineasm.file_func_cheat_asm_count(stem, func)
            prologue = cheats.func_prologue_count(func)
            is_canon = func in canon

            if is_canon:
                total_completed_canon += 1
                if rules > 0:
                    violations.append(
                        f"{func} ({stem}.c): COMPLETED-INLINE-ASM-CANONICAL "
                        f"but carries {rules} regfix/asmfix rule(s)")
                if prologue > 0:
                    violations.append(
                        f"{func} ({stem}.c): COMPLETED-INLINE-ASM-CANONICAL but carries "
                        f"{prologue} prologue_fix entry(ies)")
                continue

            # COMPLETED-C invariants: no rules, no cheat constructs, no prologue_fix.
            if rules > 0:
                violations.append(
                    f"{func} ({stem}.c): NOT in queue and NOT canonical, but carries "
                    f"{rules} regfix/asmfix rule(s) — should be INCOMPLETE")
            elif prologue > 0:
                violations.append(
                    f"{func} ({stem}.c): NOT in queue and NOT canonical, but carries "
                    f"{prologue} prologue_fix entry(ies) — should be INCOMPLETE "
                    f"(prologue_fix reorders cc1's prologue; audit 2026-06-15)")
            elif cheat_count > 0:
                violations.append(
                    f"{func} ({stem}.c): NOT in queue and NOT canonical, but has "
                    f"{cheat_count} cheat construct(s) in source — should be INCOMPLETE")
            else:
                total_completed_c += 1

    print(f"COMPLETED-C:                    {total_completed_c} functions")
    print(f"COMPLETED-INLINE-ASM-CANONICAL: {total_completed_canon} functions")
    print(f"INCOMPLETE (in queue):          {len(in_queue)} functions")

    if violations:
        print(f"\nCOMPLETION-INTEGRITY VIOLATIONS ({len(violations)}):")
        for v in violations:
            print(f"  ** {v}")
        print("\nFix: either un-complete (drop into queue and strip the cheat down "
              "to pure C), or, only if the function is genuinely canonical-asm, "
              "authorize it in inline_asm_canonical.txt with evidence.")
        return 1
    print("\nOK: all completed functions satisfy their category's invariants.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
