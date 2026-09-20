# SELF-VET — func_8006ECF4
CONSTRUCTS: named intermediate `b2 = b * 2` (staged sub-expression); struct type `Rect_8006ECF4` (4x s16, matches existing project-wide `Rect` shape in src/ings.c); `switch`/`default`/`goto skip_load` control flow replacing a fallthrough case set

## T1 semantic purpose
`b2` holds the real value `b*2` that is consumed at its use site (`D_8009BC40[b2 + c*12]`) — same bytes contribute to the final index either way, this only changes association/emit order, not program meaning. Real semantic purpose, not dead.
`Rect_8006ECF4` is the real type of the copied object (a screen-rect passed to `LoadImage`) — same struct shape as `Rect` already declared and used identically in src/ings.c for the exact same `LoadImage` idiom. Real, non-dead type declaration.
`switch`/`goto skip_load` encodes the ACTUAL control-flow topology read directly off the disassembly (asm/funcs/func_8006ECF4.s lines 84-138): the jump table's non-explicit-case entries and the `sel>=15` path both go straight to the shared fallback and never enter the `if (i != 0)` guard. The goto is not decorative — it is the only way to express "some switch arms fall through to guarded code, the default arm does not" in C without duplicating the guard's condition or code.

## T2 human-programmer
Yes to all three. A programmer transcribing this disassembly would naturally: (a) name the sub-expression `b*2` if writing out the index calc step by step, (b) reuse the project's existing `Rect` struct shape for a screen-rect copy+LoadImage call (it's already the established idiom in this codebase for this exact API), (c) write a switch with a default arm that goto-skips guarded code that only applies to the recognized cases — this is ordinary readable C for "recognized selector → extra validated side effect; unrecognized selector → just use a formula."

## T3 GCC-internals justification
None of the three constructs are justified by naming a GCC pass as the MECHANISM that makes them "work" by hiding/defeating something. `b2`'s effect (matching op order) is incidental to a real value; the struct type controls real ABI/codegen (alignment-driven aligned-vs-unaligned copy) as an ordinary type-correctness fix, not a coercion; the switch/goto is ordinary control-flow structure recovery, not a scheduling/allocation trick. No FAKE annotation is claimed for any of them because none needs one — they are exactly the code a human would write from the spec/disassembly, and each measured drop follows from expressing REAL program logic more precisely, not from suppressing something in the emitted output.

## T4 permuter/search provenance
None of the three came from an auto-search tool. `b2` and the switch/goto rewrite were derived by directly reading asm/funcs/func_8006ECF4.s and matching its literal instruction sequence/control flow. The struct-type fix was derived by finding the identical idiom already matched in src/ings.c (func_80016A8C, COMPLETED-C on main) and applying the same struct shape here. All three are principled derivations, not "whatever the detector didn't catch."

## T5 family check
`b2`: SOTN-accepted "named-intermediate declaration order" family (.claude/rules/no-new-park-categories.md) — once-written, real value, byte-neutral (target_insns==build_insns unaffected — both sides still 209 vs 21x insns; this term specifically closed 4 hunks from source-level to operand-only), fresh local. No annotation required per that family's own precedent (SOTN ships this shape un-annotated as ordinary style; the FAKE-annotation requirement in [[no-new-park-categories]] governs the constructs that have NO semantic purpose — this one does have one, board test T1).
`Rect_8006ECF4`: not a "sanctioned family" construct at all — it's an ordinary type declaration matching a real, already-established, already-matched project shape. No family entry needed; this is normal decomp type-correctness work, same category as [[header-type-correction-from-use-sites]] in spirit (correcting an invented aggregate's shape from use-site/sibling evidence) but even more directly: literally reusing a sibling TU's proven-matching struct layout.
`switch`/`goto skip_load`: SOTN-accepted "mixed exit forms" family (.claude/rules/no-new-park-categories.md, citing SsVabOpenHeadWithMode) — ordinary goto used to mix exit forms / skip a code region, real control flow, not label-sharing dead code.

## T6 naming-announces-intent
`b2`, `Rect_8006ECF4`, `skip_load` are all descriptive of their real role (a doubled value, a rect type, a control-flow target) — none of them are named `pad`/`dummy`/`spill`/`unused`/`tmp` or otherwise announce coercion intent. `skip_load` literally states what the goto does (skip the LoadImage attempt), consistent with the real control flow, not a hidden purpose.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: named-intermediate declaration order
  SCOPE: "Named-intermediate declaration order (narrow-byte-args-packed-call hi/lo sub-trick): declare a sub-expression as a separately-named local to bias LUID. SOTN's `randy` chain in `src/weapon/w_037.c` is the same mechanism."
  PRECEDENT: .claude/rules/no-new-park-categories.md:96

  FAMILY: mixed exit forms
  SCOPE: "Mixed exit forms ([[cross-jump-store-tail-merge]]): deliberately mix `goto endK` with inline `return` to defeat `find_cross_jump`. SOTN ships this verbatim in `SsVabOpenHeadWithMode` (`src/main/psxsdk/libsnd/vs_vh.c`)."
  PRECEDENT: .claude/rules/no-new-park-categories.md:94

ANNOTATION-CONFORMANCE: n/a — no FAKE construct (all three constructs have a truthful semantic reading with a real, observably-necessary purpose; none is a no-semantic-purpose coercion requiring the `/* FAKE */` prerequisites)
