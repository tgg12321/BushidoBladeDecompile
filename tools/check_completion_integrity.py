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
from engine import buildstamp, completion, buildconfig  # noqa: E402
from engine import queue as Q  # noqa: E402
sys.path.insert(0, str(REPO / "tools"))
import audit_asm_cheats as AAC  # noqa: E402  (the manual detector's island scanner)


def _island_funcs(src_text: str | None, stem: str) -> dict[str, int]:
    """func -> number of multi-insn in-body __asm__ blocks that carry at least
    one instruction outside the cop2 whitelist (the hardcoded-GPR addressing
    preamble of a GTE island, or smuggled work). Mirrors the
    'Multi-instruction __asm__ inside C function bodies' section of
    tools/audit_asm_cheats.py so this guard and that report cannot disagree."""
    out: dict[str, int] = {}
    if src_text is None:
        return out
    for _f, _line, _n, fname, _insns in AAC.scan_c_body_smuggled_work(src_text, f"{stem}.c"):
        if fname:
            out[fname] = out.get(fname, 0) + 1
    return out

QUEUE = REPO / "engine" / "queue.json"


def main() -> int:
    in_queue: set[str] = set()
    if QUEUE.exists():
        q = json.loads(QUEUE.read_text())
        in_queue = {it["func"] for it in q.get("items", [])}
    canon = cheats.canonical_asm_funcs()

    violations: list[str] = []
    freshness = buildstamp.check()
    if not freshness['fresh']:
        violations.append(f'Build is not certified from current inputs: {freshness}')
    gate_notes: list[str] = []
    data_as_code: list[str] = []
    total_completed_c = 0
    total_completed_canon = 0

    # Walk every C source file and check every function defined in it.
    # Use RELATIVE paths — score._o_func_table shells out via objdump, and an
    # absolute path containing spaces ("Bushido Blade 2 Decompile") gets split
    # by the shell. We chdir'd to REPO above, so relative paths work fine.
    for stem in sorted(P.c_stems()):
        ref_o = f"build/src/{stem}.o"
        if not Path(ref_o).exists():
            violations.append(f'{ref_o}: required object missing')
            continue
        src_text = inlineasm._read_src_cached(stem)
        island_funcs = _island_funcs(src_text, stem)
        for func in score._o_func_table(ref_o):
            if func in in_queue:
                continue  # INCOMPLETE — the queue covers it
            # Data-as-code: a symbol in the object's function table that is not
            # a C-level function of the file at all (`.include`d asm body,
            # `.aent` alternate entry, instruction-less `glabel` marker). Owner
            # ruling 2026-08-07: these are NOT completions and are excluded
            # from the COMPLETED-C count project-wide (same structural test the
            # queue generator uses; tools/spotcheck applies it too). Surfaced,
            # not silently dropped.
            if src_text is not None and func not in canon \
                    and Q.not_a_c_function_text(src_text, func):
                data_as_code.append(f"{func} ({stem}.c)")
                continue
            # Function is not in the queue: must be one of the COMPLETED states.
            rules = (len(cheats.func_rule_lines(func, cheats.REGFIX))
                     + len(cheats.func_rule_lines(func, cheats.REGFIX2))
                     + len(cheats.func_rule_lines(func, cheats.ASMFIX)))
            cheat_count = inlineasm.file_func_cheat_asm_count(stem, func)
            prologue = cheats.func_prologue_count(func)
            gates = cheats.maspsx_gate_entries(func)
            is_canon = func in canon
            for issue in completion.source_issues(stem, func, is_canon):
                violations.append(f'{func} ({stem}.c): {issue}')

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
                for path, _cls in gates:
                    gate_notes.append(
                        f"{func} ({stem}.c): canonical-asm body gated in {path} — "
                        f"vestigial (glabel bodies never set maspsx current_func); "
                        f"delete the entry at the next idle rebuild window")
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
            elif func in island_funcs:
                # The engine's cheat count treats a canonical cop2 island (GTE
                # mnemonics + its hardcoded-GPR addressing preamble) as
                # non-cheat, so `queue done` records the function COMPLETED-C.
                # The completion standard says that bucket is pure C; a body
                # carrying an inline-asm island is COMPLETED-INLINE-ASM-CANONICAL
                # and must be listed. Five functions slipped through this gap
                # 2026-07-28..2026-09-01 (Judge note, docs/grind/decisions.md
                # 2026-09-01 func_8002EA24 final call); this keeps the two
                # detectors (this tool + tools/audit_asm_cheats.py) in agreement.
                violations.append(
                    f"{func} ({stem}.c): NOT canonical but carries a multi-insn "
                    f"inline-asm island with non-cop2 instructions "
                    f"({island_funcs[func]} block(s)) — either the island is "
                    f"canonical hand-asm (authorize it in inline_asm_canonical.txt "
                    f"with evidence; honest bucket COMPLETED-INLINE-ASM-CANONICAL) "
                    f"or it is injection (should be INCOMPLETE)")
            else:
                total_completed_c += 1
                # COMPLETED-C + maspsx gate: cheat-pathway gates are violations
                # (a pure-C spelling exists — the completion leaned on config);
                # fidelity gates are legitimate but must stay VISIBLE
                # (.claude/rules/maspsx-gate-lists.md, adjudicated 2026-07-13).
                for path, cls in gates:
                    if cls == "cheat-pathway":
                        violations.append(
                            f"{func} ({stem}.c): COMPLETED-C but gated in {path} "
                            f"(cheat-pathway: a pure-C spelling exists for that "
                            f"effect) — should be INCOMPLETE until the C is fixed "
                            f"and the entry deleted")
                    else:
                        gate_notes.append(
                            f"{func} ({stem}.c): COMPLETED-C depends on {path} "
                            f"(fidelity-class assembler gate)")

    # Standalone linked canonical assembly is not in a C object's inventory.
    for func in buildconfig.LINKED_ASM_FUNCS:
        obj = Path(f'build/asm/funcs/{func}.o')
        if not obj.exists():
            violations.append(f'{obj}: required standalone object missing')
        elif func not in in_queue:
            if func not in canon:
                violations.append(f'{func}: standalone assembly is neither queued nor canonical')
            else:
                total_completed_canon += 1

    print(f"COMPLETED-C:                    {total_completed_c} functions")
    print(f"COMPLETED-INLINE-ASM-CANONICAL: {total_completed_canon} functions")
    print(f"INCOMPLETE (in queue):          {len(in_queue)} functions")
    if data_as_code:
        print(f"\ndata-as-code symbols ({len(data_as_code)}) — in the objects' "
              f"function tables but structurally not C functions; excluded from "
              f"the COMPLETED-C count (owner ruling 2026-08-07):")
        for d in sorted(data_as_code):
            print(f"  -- {d}")

    if gate_notes:
        print(f"\nmaspsx gate-dependent completions ({len(gate_notes)}) — "
              f"legitimate, tracked for transparency:")
        for n in gate_notes:
            print(f"  -- {n}")

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
