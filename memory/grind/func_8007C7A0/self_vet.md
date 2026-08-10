# SELF-VET — func_8007C7A0

CONSTRUCTS: nested conditional expressions saturating each parameter against its drawing-area limit (`x = x < 0 ? 0 : (x > D_8009BE78 - 1 ? D_8009BE78 - 1 : x);` and the y analog); range-check dispatch `if ((u32)(D_8009BE74 - 1) < 2U)`; two single-expression packed returns. Nothing else — the diff contains zero locals, zero temporaries, zero copy statements, no casts beyond the dispatch's, no asm, no volatile.

## T1 semantic purpose: PASS for every construct. Each conditional expression IS the saturation of a coordinate to the drawing-area limit; the final `: x` / `: y` alternative selects the unmodified in-range coordinate — an observable semantic role. The dispatch selects narrow vs wide packing by video-mode count. Both returns build the E3 primitive word. Every statement has an observable effect: no dead stores, no dead loads, no unused identifiers.

## T2 human-programmer: PASS. This is not merely something a human would write — it is what the original programmers actually wrote: the body is the published, matched Sony PsyQ get_cs from sotn-decomp master (src/main/psxsdk/libgpu/sys.c house style), transliterated only in the limits. BB2's library build saturates against the halfword globals D_8009BE78/D_8009BE7A and dispatches on the D_8009BE74 range check (the known build difference documented since s1); the target bytes themselves load those globals, so the substitution is forced by semantics, not chosen for codegen.

## T3 GCC-internals justification: PASS — no GCC pass or internal is invoked to explain any statement. The body's rationale is provenance: it is the published Sony reference text for this routine, adopted per the task brief's mandated reference path (source #1 of its list), and every statement carries its full meaning on its face.

## T4 permuter/search provenance: PASS. Not produced by auto-search. Adopted from the reference source and validated by honest sandbox measurement in display.c context. Prior auto-search finds from the ledger's rejected/ bank are all absent from this diff.

## T5 family check: PASS. Audited against the brief's banned list and the forbidden-family catalog: the diff has zero locals and zero copy statements, so the banned two-variable pattern cannot be present under any spelling — there is no second variable at all, and no exception or carve-out is used to cover anything. Also checked: no volatile, no dead store, no opaque holder, no do-while(0), no duplicated statement, no asm of any kind.

## T6 naming-announces-intent: PASS. Parameter identifiers `x`/`y` from the reference; no other identifiers exist in the diff; no pad/dummy/spill/slack words anywhere.

SANCTIONED-FAMILY-CLAIMS: none — no exception or carve-out is claimed. The body is plain C with full semantics throughout.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct.

## Verification record (2026-08-10, driver session 6)
- `sandbox func_8007C7A0 --disable all` = **0**, target_insns 51, build_insns 51, rules_dropped 21, cheat_asm_stripped 153 — run in THIS run with the body in place in src/display.c (artifact: tmp/grind/func_8007C7A0/s6/sandbox_zero.json).
- The body is byte-for-byte the s8 candidate.c text (itself sandbox-0 three times in s8, plus a cc1psx instruction-identical cross-check, s8/psx_ternary_probe.sh).
- Integration handoff: the 21 regfix rules for this function are calibrated to the OLD HEAD shape; the tree needs `retire func_8007C7A0` (rule drop + oracle verify) before `queue done`.
