# Hypothesis ledger — hirahira_w_frie

## s2 (2026-07-17, structural)

1. KILLED — "sched1 orders a1's callee-save copy first naturally because the *offsets load feeds the entry branch (critical path)". Measured: fully clean form = 6 (a0 pair still first). Both chains tie at priority 4; rank_for_schedule (sched.c:2398) tie-breaks by INSN_LUID → expand_function_start's a0-first emission wins. The critical-path argument fails because the a0 chain (move+lw+addu) is exactly as deep as the a1 chain (move+lh+beq).

2. KILLED — "initializer/statement-order geometry can bias the copy-pair order". Measured: v1-as-first-decl-init = 6 (unchanged); inits-as-statements-after-read = 13 (worse — decl-position initializers for slots/count/dest are load-bearing for the s5/s6 prologue-region scheduling). Statement geometry cannot move the entry copies at all.

3. KILLED — "a semantically-true offsets walking cursor (single rename, forward role) flips the pair". Measured: 6, unchanged. The flip requires relocating the BASE (a0) copy, not the offsets copy.

4. CONFIRMED — "combine relocates a single-use param's entry copy to its rename-init position, flipping the pair without the reversed-pair shape". Measured: base-only rename `s32 *b = base;` = 2 (prologue pair fixed); + block-local `stop = -2` = 0. Mechanism: combine merges (move pA<-a0) + (move pb<-pA) placing the result at the later insn; a1's untouched entry copy then has lower LUID.

5. CONFIRMED — "the preheader pair (li s7,-2 before move s1,s5) requires an explicit constant statement before the walker init". Measured: clean+stop = 4 (exactly the 2-insn cluster closed vs clean 6). loop.c hoists the literal after the walker stmt; leaf-priority tie keeps emission order.

## s2b (2026-07-17, structural, post-Judge-ban sweep)

6. KILLED — "K&R old-style definition with the param-decl block reversed flips expand_function_start's copy emission order without a rename". Measured: 4, unchanged. c-decl.c store_parm_decls orders DECL_ARGUMENTS by the IDENTIFIER LIST (ABI-fixed a0=base), not by decl-block order. This was the last conceivable no-rename emission-order lever.

7. KILLED — "a do-while(0) loop-note fence near the entry read can touch the arg-copy pair order". Measured: 19 (worse, 58/59 insns — the notes perturb the real loop's reorg). Also inert by construction: both copies are emitted adjacently BEFORE any statement, so no statement-level wrapper can sit between them.

8. KILLED — "residual declaration-geometry axes (uninitialized-decl order; initialized-decl permutation with dest first) bias the pair". Measured: 4 and 4, unchanged. sched1 renormalizes entry-block emission order; local pseudo numbering never touches the two copies' LUIDs.

CONCLUSION: with H1-H8 the sanctioned pure-C space is measured-closed at floor 4 (= exactly the 2 swapped prologue pairs). The only closing lever (H4, single-use-param alias via combine) is Judge-banned pending the owner's tombstone ruling; escalation filed in docs/grind/decisions.md 2026-07-17.

## [s2] K&R old-style definition with the param-decl block reversed (offsets declared before base) flips expand_function_start's entry-copy emission order without a literal rename
- mechanism: c-decl.c store_parm_decls chaining DECL_ARGUMENTS in decl-block order would give a1's copy the lower LUID, winning sched1's rank_for_schedule tie-break
- probe: rewrote the definition K&R-style with s16 *offsets; declared first; sandbox hirahira_w_frie --disable all
- result: score 4, unchanged (59/59) — store_parm_decls orders DECL_ARGUMENTS by the identifier list (ABI-fixed a0=base), decl-block order is inert
- verdict: KILLED

## [s2] a sanctioned do-while(0) wrapper around the entry v1=*offsets read can fence sched1's entry region in a way that touches the arg-copy pair order
- mechanism: NOTE_INSN_LOOP_BEG/END emitted by the wrapper acting as a scheduling fence near the copies
- probe: wrapped the entry read+increment in do{...}while(0) with FAKE annotation; sandbox --disable all
- result: score 19, build 58 vs target 59 — strictly worse (injected loop notes perturb the real loop's reorg/preheader); also inert for the pair by construction since both copies are emitted adjacently before any statement
- verdict: KILLED

## s3 (2026-07-17, structural)

9. KILLED — "the owner has ruled on the s2b escalation (either branch)". Probe: read docs/grind/decisions.md tail. Result: file ends at the s2b OWNER-ESCALATION entry; no owner disposition. Function remains owner-gated; floor 4 stands.

10. KILLED — "param TYPE participates in the entry-copy scheduling (type-narrowing lever, param side)". Probe: `u32 *base` on the clean+stop base. Measured: 4, 59/59 — invariant floor, type-inert as the credited mechanism predicts.

11. KILLED — "the a1-chain consumer local can be narrowed (`s16 v1`) to perturb the tie". Measured: 21, build 66 vs 59 — +7 sign-extension insns, strictly worse; not byte-neutral. Type-narrowing axis is dead on both entry chains.

## [s2] residual declaration-geometry axes (fully reversed uninitialized-decl order; initialized-decl permutation with dest's initializer first) bias the entry-copy pair via pseudo numbering or emission order
- mechanism: local pseudo regno / RTL emission order of the derived-local defs feeding sched1 tie-breaks
- probe: two variants on the clean+stop base, each measured with sandbox --disable all
- result: score 4 and 4, unchanged — sched1 renormalizes the entry block; the two arg copies' LUIDs are untouched by any local declaration geometry
- verdict: KILLED

## [s3] The owner has ruled on the s2b tombstone escalation (branch a or b)
- mechanism: docs/grind/decisions.md is the owner's signed-disposition surface; frontier next-probe was to check it
- probe: Read the decisions.md tail past the s2b OWNER-ESCALATION entry (lines 627-636)
- result: File ends at the escalation entry; no owner disposition follows. Both branches remain open.
- verdict: KILLED

## [s3] Param TYPE participates in entry-copy scheduling — retyping base could perturb the prologue-pair order (type-narrowing lever, param side, absent from the s2/s2b grid)
- mechanism: If type changed the a0 chain's RTL shape or priority, the 4=4 tie could break; credited s2 mechanism predicts inert (copies emitted pre-statement in param order, a0 chain move/lw/addu type-invariant at 32-bit width)
- probe: u32 *base on the clean+stop floor-4 base; sandbox --disable all
- result: score 4, 59/59 — invariant floor, type-inert exactly as predicted; banked rejected/param-type-widen-u32-base-floor4.c
- verdict: KILLED

## [s3] The a1-chain consumer local can be narrowed (s16 v1) to re-tie the entry-chain priorities
- mechanism: Narrowing v1 changes its arithmetic uses' RTL; but GCC must insert sll/sra sign-extensions, so byte-neutrality was the open question
- probe: s16 v1 on the clean+stop base; sandbox --disable all
- result: score 21, build 66 vs target 59 (+7 sign-extension insns) — strictly worse, not byte-neutral; banked rejected/entry-read-local-s16-floor21.c
- verdict: KILLED
