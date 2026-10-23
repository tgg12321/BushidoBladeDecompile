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
# Run this under WSL: it reaches engine.score, which shells out to the
# mipsel-linux-gnu cross-toolchain, so the Windows-side interpreter cannot
# complete it regardless.
#
# The deferred-annotation import is still needed: PEP 604 (`str | None`)
# annotations are used below, and the Windows Python on this machine is 3.9,
# which evaluates them eagerly and raises TypeError at def time. Without this,
# a Windows invocation dies with a confusing TypeError inside a type annotation
# instead of reaching the honest "cross-toolchain not found" failure.
from __future__ import annotations

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
from engine import buildstamp, completion, buildconfig, gtemacro  # noqa: E402
from engine import queue as Q  # noqa: E402
from engine import departures  # noqa: E402
sys.path.insert(0, str(REPO / "tools"))
import audit_asm_cheats as AAC  # noqa: E402  (the manual detector's island scanner)
import check_retired_names as CRN  # noqa: E402  (naming waves' retired names in C comments)


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


def _attributed_islands(island_funcs: dict[str, int], src_text: str | None,
                        func: str) -> int:
    """island_funcs[func] plus those of every same-TU `static inline` helper
    func reaches (owner ruling Q126: an inlined helper's code is its caller's,
    and the helper has no symbol of its own for this loop to visit)."""
    helpers = inlineasm.inline_helpers(src_text, func) if src_text else []
    return island_funcs.get(func, 0) + sum(island_funcs.get(h, 0) for h in helpers)


def _helper_issues(src_text: str | None, stem: str, ftab) -> list[str]:
    """Owner ruling Q126 audits a same-TU `static inline` helper only as part
    of the functions that call it, which holds only while every gate can see
    which functions those are. REFUSED, per TU (every file-scope function body
    found in the compiler view, inlineasm.tu_functions):
      (1) a static inline helper GCC also emits out of line (a symbol of its
          own in the object's function table) -- code no caller's review
          covers as its own function;
      (2) a helper with more than one definition (e.g. an `#if 0` copy):
          which body is compiled is ambiguous;
      (3) a helper named in a #define body: a macro is a path to it the
          caller's text does not spell;
      (4) a helper no function in the object reaches: its body is read by
          no gate;
      (5) any other function body with no symbol in the object (e.g.
          `inline` spelled through a macro): no gate reads it at all;
      (6) unsafe TU indexing: lexical view/identifier/directive escapes;
          brace/branch imbalance; unsafe macros/heads; duplicate definitions.
          The walk reads one masked TU view only; headers/#include'd code
          are unseen (see inlineasm._tu_index for the detailed rules)."""
    if not src_text:
        return []
    defs = inlineasm.static_inline_defs(src_text)
    out = []
    if inlineasm.tu_unbalanced(src_text):
        out.append(f"{stem}.c: file-scope braces do not balance or cannot be indexed "
                   f"safely (lexical view/identifier/directive escapes; brace/branch "
                   f"imbalance; unsafe macros/heads; duplicate definitions) -- walk "
                   f"reads one masked TU view only (headers/#include'd code unseen); "
                   f"use explicit, complete definitions")
    for h, spans in defs.items():
        if h in ftab:
            out.append(f"{h} ({stem}.c): static inline helper emitted out of line (in "
                       f"the object's function table) -- refused: Q126 audits a helper "
                       f"only through its callers; make it an ordinary function or keep "
                       f"every use inlined")
        if len(spans) > 1:
            out.append(f"{h} ({stem}.c): static inline helper defined {len(spans)} times "
                       f"(ambiguous) -- keep one definition")
    for h in sorted(inlineasm.macro_named_helpers(src_text)):
        out.append(f"{h} ({stem}.c): static inline helper named in a #define body -- "
                   f"call it directly, so its callers are visible")
    reached: set[str] = set()
    for f in ftab:
        reached.update(inlineasm.inline_helpers(src_text, f))
    for h in defs:
        if h not in reached and h not in ftab:
            out.append(f"{h} ({stem}.c): static inline helper that no function in the "
                       f"object reaches -- no gate reads its body (remove it)")
    for fn in inlineasm.tu_functions(src_text):
        if not fn.static_inline and fn.name not in ftab:
            out.append(f"{fn.name} ({stem}.c): function body with no symbol in the object "
                       f"and not a recognized `static inline` helper -- no gate reads it "
                       f"(spell a helper `static inline`, or remove it)")
    return out


def _header_function_issues(root: Path = REPO) -> list[str]:
    """Function definitions in project headers (include/**/*.h, src/**/*.h;
    the vendored PsyQ headers excepted): a body there is outside every
    per-TU gate (Q126 attributes same-TU helpers only)."""
    vendored = set(gtemacro.VENDORED)
    out = []
    for pat in ("include/**/*.h", "src/**/*.h"):
        for p in sorted(root.glob(pat)):
            rel = p.relative_to(root).as_posix()
            if rel in vendored:
                continue
            text = p.read_text(encoding="utf-8", errors="replace")
            out += [f"{rel}: function definition {fn.name} in a project header -- "
                    f"define it in the .c file that uses it"
                    for fn in inlineasm.tu_functions(text)]
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
    # Code-size split (inline-asm audit A2, 2026-10-01): bytes per bucket, so
    # the report can state SOTN-comparable coverage ("bytes converted to C",
    # sotn-decomp README) beside the completion counts.
    size = {"c": 0, "islands": 0, "whole": 0, "incomplete": 0}

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
        whole_asm = inlineasm.whole_body_asm_funcs(src_text) if src_text else set()
        ftab = score._o_func_table(ref_o)
        violations.extend(_helper_issues(src_text, stem, ftab))
        for func in ftab:
            fsize = ftab[func][1]
            if func in in_queue:
                size["incomplete"] += fsize
                continue  # INCOMPLETE — the queue covers it
            # Data-as-code: a symbol in the object's function table that is not
            # a C-level function of the file at all (an `alabel` / `.aent` of an
            # included asm body, `.aent` alternate entry, instruction-less
            # `glabel` marker). Owner ruling 2026-08-07: these are NOT
            # completions and are excluded from the COMPLETED-C count
            # project-wide (same structural test the queue generator uses;
            # tools/spotcheck applies it too). Surfaced, not silently dropped.
            if src_text is not None and func not in canon \
                    and Q.not_a_c_function_text(src_text, func, Q.read_asm_file):
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
                size["whole" if func in whole_asm else "islands"] += fsize
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
            elif _attributed_islands(island_funcs, src_text, func):
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
                    f"({_attributed_islands(island_funcs, src_text, func)} block(s), "
                    f"its static inline helpers' included) — either the island is "
                    f"canonical hand-asm (authorize it in inline_asm_canonical.txt "
                    f"with evidence; honest bucket COMPLETED-INLINE-ASM-CANONICAL) "
                    f"or it is injection (should be INCOMPLETE)")
            else:
                total_completed_c += 1
                size["c"] += fsize
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

    # Owner ruling Q39: a function that left engine/queue.json after the gate
    # existed must hold a layer-2 PASS on its current body (engine/departures.py
    # — catches a completion that bypassed `queue done`, e.g. a hand-edited
    # queue.json). Git or history read failures are violations too.
    violations.extend(departures.unreviewed_departures())

    # Canonical-asm region grants (tools/canonical_asm_regions.json) match the
    # islands they reviewed, queued functions included: the loop above skips
    # those, and naming waves rename operands inside their islands.
    violations.extend(completion.grant_issues(set(P.c_stems())))

    # Vendored PsyQ GTE headers (engine/gtemacro.py): each is Sony's pinned
    # text, and no other header defines a macro expanding to inline asm (the
    # gates read unexpanded source, so such a macro would be invisible).
    violations.extend(f"vendored header: {i}" for i in gtemacro.check_vendored())
    violations.extend(gtemacro.header_registry_issues())
    # Q126: helpers are attributed within one .c file; no project header may
    # carry a function body.
    violations.extend(_header_function_issues())
    # No TU uses `##`: a token paste can assemble asm or a vendored name that
    # no gate sees by name (counted and stripped as a cheat construct).
    violations.extend(f"src/{stem}.c: `##` token paste (cheat construct)"
                      for stem in sorted(P.c_stems())
                      if gtemacro.paste_spans(inlineasm._read_src_cached(stem) or ""))

    # Naming waves (.claude/rules/naming-bar.md): no C comment keeps a name a
    # wave retired (completion-bar item 3: comments assert nothing false).
    violations.extend(f"{path}:{line}: comment keeps retired name {name}"
                      for path, line, name in CRN.scan(REPO)[2])

    print(f"COMPLETED-C:                    {total_completed_c} functions")
    print(f"COMPLETED-INLINE-ASM-CANONICAL: {total_completed_canon} functions")
    print(f"INCOMPLETE (in queue):          {len(in_queue)} functions")
    total_bytes = sum(size.values())
    if total_bytes:
        def pct(n):
            return f"{100.0 * n / total_bytes:5.1f}%"
        print(f"\ncode size by bucket ({total_bytes:,} bytes of C-object .text; "
              f"data-as-code excluded):")
        print(f"  COMPLETED-C                           {size['c']:>8,}  {pct(size['c'])}")
        print(f"  canonical: C body + asm islands       {size['islands']:>8,}  {pct(size['islands'])}")
        print(f"  canonical: whole-body hand-written asm{size['whole']:>8,}  {pct(size['whole'])}")
        print(f"  INCOMPLETE (queue)                    {size['incomplete']:>8,}  {pct(size['incomplete'])}")
        conv = size['c'] + size['islands']
        print(f"  => in C (SOTN 'code coverage' sense: COMPLETED-C + C-with-islands): "
              f"{conv:,} bytes, {pct(conv).strip()}")
        print(f"  => completed incl. canonical hand-written asm: "
              f"{conv + size['whole']:,} bytes, {pct(conv + size['whole']).strip()}")
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
