# SELF-VET — func_800283D0 (grind s27, 2026-08-27)

Diff = `INCLUDE_ASM("asm/funcs", func_800283D0);` in src/code6cac_b.c replaced by the
pure-C body in memory/grind/func_800283D0/candidate.c. Nothing else in the tree is
touched: no regfix.txt / asmfix.txt / .claude/rules / engine / tools / Makefile / *.ld
edit, no inline `__asm__` of any kind, no `register ... asm("$N")` pin, no volatile,
no alias rename, no scheduling barrier. `sandbox func_800283D0 --disable all` prints
`"score": 0, "target_insns": 215, "build_insns": 215` with these edits in place.

CONSTRUCTS: do-while(0) wrap around the two func_80032854 calls in the `temp_v0_3 == temp_v1_3` arm; duplicated store `*(s16 *)(arg0 + 0x286) = 0x19;` at label `sel19`; duplicated store `*(s16 *)(arg0 + 0x286) = var_v0_4;` in the `temp_v1_3 < temp_v0_3` arm; named intermediate `s32 sel`; named intermediates `s32 idx0` / `s32 idx1`; ordinary-C items that claim no family and are listed here for completeness: pointer local `tail` (one write, three reads), `goto ret_one;` / `goto block_15;` / `goto block_49;` / `goto do_calls;` / `goto set_0xB;` / `goto block_20;` mixed with inline `return ret;` and `return 1;` exits, the m2c-shaped value locals `d_val`, `temp_a1`, `temp_a1_2`, `temp_s4`, `temp_s5`, `temp_v0`, `temp_v0_3`, `temp_v1`, `temp_v1_2`, `temp_v1_3`, `temp_v1_4`, `temp_v1_5`, `temp_a0_2`, `var_s1`, `var_a1`, `var_v0`, `var_v0_2`, `var_v0_3`, `var_v0_4`, `ret`, and the sub-word reads `*(u16 *)(arg0 + 0x6A)` / `*(s16 *)(arg0 + 4)` on real game-state structure fields.

## T1 semantic purpose
- **do-while(0) wrap**: no observable effect on the function's output; the two calls
  execute exactly once either way. It has NO semantic purpose — it is a declared
  match device, which is precisely what the do-while-zero family sanctions, and it
  carries the mandatory FAKE annotation naming the mechanism.
- **`sel19` store / `<`-arm store**: both have full semantic purpose — they are real
  stores of a real value to a real field, on control-flow arms that genuinely need
  the store. Target's own bytes contain three `sh $v0, 0x286($s0)` instructions
  (asm/funcs/func_800283D0.s, emitted indices 83, 147, 202) and the C's four store
  statements cross-jump-merge down to exactly those three. What is match-motivated is
  only the CHOICE to write the store inside each arm rather than sharing one copy
  through a label, so they are FAKE-annotated under the duplicated-statement family.
- **`sel`**: holds a real, consumed value (the selected constant 0xB / 0x19 that is
  stored at `block_48`). A simpler form (`var_v0_2 = (var_s1 == 0) ? 0xB : 0x19;`)
  produces different bytes — banked as
  rejected/tern-no-intermediate-canonical-order-remerges.c — so the intermediate is
  match-motivated and is FAKE-annotated.
- **`idx0` / `idx1`**: hold real, consumed values — the two `Judge[]` table indices.
  The `negu` / `addiu 0x400` / `andi 0xFFF` chain they name appears verbatim in
  target's bytes. What is match-motivated is only that they are NAMED and declared
  before `tail`, so they are FAKE-annotated.
- **`tail`**: real pointer to the word triple the tail arithmetic indexes, read three
  times (`tail[0x45]`, `tail[0x46]`, `tail[0x47]`). Full semantic purpose; all three
  loads are in target's bytes.
- **Everything else**: each local carries a value that is loaded, computed or stored
  in target's 215 instructions. There is no dead local, no constant holder, no
  written-never-read array, no self-assign, no dead-parameter store, no `(void)x`
  discard, and no address-of-a-dead-local anywhere in the body.

## T2 human-programmer
- The overall shape is what a human writing this routine would write: read the two
  fighters' state words, take five early exits, run a membership test over a set of
  move IDs, then a decision tree that stores a reaction ID and fires two
  `func_80032854` notifications. The new s27 shape (`if (temp_v1 == 4) goto ret_one;`
  / `if (temp_v1 == 0x14) return ret;` as flat early guards instead of two nested
  `if (x != k) { ... }` blocks) reads BETTER than the s26 form and is the ordinary way
  a C programmer writes a chain of early-out guards.
- The `goto ret_one;` edges into the shared `return 1;` block are exactly what
  target's own asm shows the original author wrote: three of target's branches
  (emitted 15, 22, 24) jump to `.L80028488`, which is the same block the range chain
  falls through to. This is not a device; it is the original control flow, read out of
  the target rather than invented.
- A reader WOULD ask "why is this here?" about four things: the `do { } while (0);`
  wrap, the two duplicated stores, `sel`, and `idx0`/`idx1`. Those four are exactly
  the four that carry FAKE annotations and sanctioned-family claims below. I claim
  nothing else passes silently.

## T3 GCC-internals justification
Yes, and stated openly rather than hidden: the do-while(0) wrap works through
flow.c's `REG_N_REFS += loop_depth` weighting feeding global.c `allocno_compare`;
the duplicated stores work through jump2 cross-jump tail merging (`find_cross_jump`)
choosing which copy survives inline; `sel` works through expand/cse LUID ordering;
`idx0`/`idx1` work through local-alloc quantity BIRTH order feeding
`floor_log2(refs) * refs * 10000 / span` in global.c. Each mechanism is named in the
construct's own `/* FAKE: ... */` comment. Per the policy this is the cheat SIGNAL,
which is why all four are declared as sanctioned-family constructs with FAKE
annotations and lever-exhaustion pointers rather than presented as ordinary C. No
OTHER construct in the body has a GCC-internals justification: the rest is program
logic. In particular the s27 change that closed the last two points is NOT a
GCC-internals construct — it is the control flow target's own branch targets show.

## T4 permuter/search provenance
None of these constructs came from decomp-permuter output or from any automated
search. s22 derived the do-while(0) wrap from a measured `reg_n_refs` inequality
(evidence.md s21/s22); s13/s16/s21 derived the duplicated stores from target's own
store topology; s24/s25 derived `sel`, `idx0`, `idx1` from a closed-form local-alloc
priority model checked against BB2_QTY_DEBUG dumps. s27's own change (the flattened
early guards + `ret_one`) was derived by reading target's branch targets out of
asm/funcs/func_800283D0.s and locating `.L80028488` at emitted index 46 — target's
asm is the source of the shape, not a search. Nothing here passes detectors "only
because the detectors don't catch this spelling"; every match-motivated construct is
declared above.

## T5 family check
Checked against the forbidden-family catalog line by line. Not present in this diff:
register-asm pins; hardcoded-`$N` `__asm__`; lost-codegen inserts; scheduling
barriers; INLINE_MOVE_ALIASING; volatile coercion in any spelling (there is no
`volatile` in the body at all); unused-local-array frame coercion; dead-param assign;
dead-conditional store; empty-body `if (cond) { }`; `if (1) { ... }`; dead-goto label
pad; DImode scheduling chain; `s32 one = 1;` opaque single-bit defeater; lowercase
`asm(...)`; `asm("sym")` alias renames; redundant width casts; `bb2.ld` rodata
reorders; combine-foldable chain extenders. `ret = 1;` is NOT an opaque constant
holder: `ret` is the function's return value, read at four `return ret;` sites, and it
materialises in target as `addiu $s6, $zero, 0x1` (emitted index 16) plus four
`addu $v0, $s6, $zero` exits. The four match-motivated constructs map to three
sanctioned families, claimed with scope + precedent below.

## T6 naming-announces-intent
No identifier in the body is `pad`, `_pad`, `dummy`, `unused`, `spill`, `sp_*`,
`_buf`, `tail_pad`, `slack` or `_frame_pad`. `tail` is a semantic name (the pointer to
the word triple the tail arithmetic indexes) and is READ three times, so it is not a
discard/unused/address-of-only local and not the coercion shape the test targets.
`sel`, `idx0`, `idx1`, `ret`, `d_val` name what they hold. The `var_*` / `temp_*`
names are m2c's decompilation convention, used consistently across this file. No local
in the body is unused, discarded, or only address-taken.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: do-while(0) match device
  SCOPE: "`do { <any body> } while (0);` — including empty bodies — is a sanctioned pure-C match device for ANY codegen effect, including register allocation."
  PRECEDENT: .claude/rules/do-while-zero-exception.md:23

  FAMILY: duplicated statement into arms
  SCOPE: "Writing the SAME real statement in two or more control-flow arms — instead of sharing one copy via a label/goto — is a legitimate matching technique, **including** when: GCC's jump2 cross-jump re-merges the copies so the final bytes are identical to the shared-label form, and the duplication's surviving effect is the extra `reg_n_refs` count flow.c records (allocno-priority lift for global RA), and the label placement among the copies is chosen to steer the merge DIRECTION (which copy survives inline vs becomes the jump)."
  PRECEDENT: .claude/rules/duplicated-statement-into-arms.md:13

  FAMILY: named-intermediate declaration order
  SCOPE: "A fresh named intermediate therefore qualifies under this entry **whatever GCC pass it acts through** (LUID bias, cse.c re-materialization, allocno priority), provided ALL of: (1) once-written, once-read — multi-write carriers are NOT this entry (the `y1` FAIL, decisions.md:1833, stands); (2) real value — the intermediate holds a computation that appears in the target's own bytes and only relocates where the value is named; pure no-op copies stay with the dead-store family and its prerequisites; (3) byte-neutral — `build_insns == target_insns`, the compiler folds the copy; (4) fresh local, not a borrow — [[staged-value-reused-variable]] keeps its own bounds; (5) destination not live-pre-initialized (the `x/tx` FAIL, decisions.md:4251, stands); (6) standard prerequisites: dump-proven named mechanism, documented lever exhaustion, `/* FAKE: ... */` annotation, layer-1 + layer-2 review."
  PRECEDENT: .claude/rules/no-new-park-categories.md:199
  SIX-PRONG CHECK for `sel`, `idx0`, `idx1`:
    (1) once-written, once-read — `sel` written once, read once at `var_v0_2 = sel;`;
        `idx0` written once, read once at `(&Judge)[idx0]`; `idx1` written once, read
        once at `(&Judge)[idx1]`. None is re-assigned anywhere.
    (2) real value — the ternary's selected constant is stored at `block_48` and is in
        target's bytes; the two index chains (`negu`, `addiu $v0,$v0,0x400`,
        `andi $v0,$v0,0xFFF`) are in target's bytes.
    (3) byte-neutral — `build_insns` 215 == `target_insns` 215, score 0.
    (4) fresh locals, not borrows — all three are new declarations in the innermost
        scope; no existing local is reused to carry them.
    (5) destinations not live-pre-initialized — `var_v0_2` is assigned on every arm
        that reaches `block_48`, and the two `(&Judge)[...]` reads have no destination
        local at all.
    (6) prerequisites — mechanism named from local-alloc.c/global.c and checked in the
        s24/s25 BB2_QTY_DEBUG dumps; lever exhaustion in
        memory/grind/func_800283D0/hypotheses.md (s24 sites 96/161 ladder, s25 V1-V4);
        FAKE annotations present (below); layer-1 + layer-2 review pending.

  FAMILY: mixed exit forms (`goto endK` + inline `return`) — ordinary C, no FAKE required
  SCOPE: "target has more `sw GLOBAL` stores (or more `j SAME_LABEL` error tails) than your build: GCC 2.7.2 jump2 cross_jump merged N identical `[sw GLOBAL; j END]` tails into one block. FIX: give the error paths a MIX of exit forms (distinct `goto endK; ... endK: return G;` labels + one inline `return G;`) so the block ENDINGS differ -> suffixes not rtx_equal -> no merge."
  PRECEDENT: .claude/rules/cross-jump-store-tail-merge.md:5
  (Sub-word reads of real structure fields — `*(u16 *)(arg0 + 0x6A)`,
  `*(s16 *)(arg0 + 4)` — are plain typed loads from a game-state record, not the
  `*(u16 *)&local` stack-param trick, and claim no family.)

ANNOTATION-CONFORMANCE:
  /* FAKE: do-while(0) loop-note ref weighting, mechanism: flow.c REG_N_REFS += loop_depth feeding global.c allocno_compare, lever-exhaustion: memory/grind/func_800283D0/hypotheses.md */
  /* FAKE: store duplicated into this arm instead of sharing block_48's copy, mechanism: jump2 cross-jump tail merge (jump.c find_cross_jump) chooses which copy survives inline, lever-exhaustion: memory/grind/func_800283D0/hypotheses.md s13/s21/s22 */
  /* FAKE: store duplicated into the `<` arm instead of sharing do_store_calls's copy, mechanism: jump2 cross-jump tail merge (jump.c find_cross_jump), lever-exhaustion: memory/grind/func_800283D0/hypotheses.md s13/s16/s21 */
  /* FAKE: named intermediate for the selected constant, mechanism: cse.c/expand LUID ordering keeps the two constants materialised in target's order, lever-exhaustion: memory/grind/func_800283D0/hypotheses.md s16 (rejected/tern-no-intermediate-canonical-order-remerges.c) */
  /* FAKE: idx0/idx1 named intermediates declared before `tail`, mechanism: local-alloc quantity BIRTH order (local-alloc.c qty_births feeding global.c allocno_compare priority floor_log2(refs)*refs*10000/span), lever-exhaustion: memory/grind/func_800283D0/hypotheses.md s24/s25 */
  All five carry what + mechanism + lever-exhaustion. Every construct that claims a
  FAKE-mandating family carries one; no construct outside those three families is
  annotated, because none is match-motivated.
