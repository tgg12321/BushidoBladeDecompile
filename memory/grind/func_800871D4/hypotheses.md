# Hypothesis ledger — func_800871D4

## [s1] The m2c-reconstructed shape (explicit `var_v1 = D_8010280A & 0xFFFF` twice, no in-place -=0x10) matches oracle semantics but our cc1 folds both masks and produces a worse layout.
- mechanism: cc1 nonzero_bits tracking proves the u16 lhu result is 16-bit-clean, so `& 0xFFFF` reduces to identity in combine.c; the shape reshuffles RA vs the in-place-modify shape.
- probe: Applied m2c-shape (var_v1 = D_8010280A & 0xFFFF; ...; var_v1 = D_8010280A & 0xFFFF in else) — sandbox --disable all
- result: score=12 (WORSE than direct-form floor=10)
- verdict: KILLED

## [s1] The 5 other BB2 functions containing the exact `lhu $r,GLOBAL; nop; andi $r2,$r,0xFFFF` pattern are ALL still INCOMPLETE (2 parked ASM-STRUCTURAL, 2 ASM-SUSPECT, 1 active). ZERO COMPLETED-C sibling in the tree produces this shape.
- mechanism: Cross-tree census (tmp/grind/func_800871D4/s1/find_sibling2.py) — the shape is a shared-failure family, not a per-function accident.
- probe: Regex scan of asm/funcs/*.s + queue.json status join.
- result: 6 matching funcs; 5 non-target (func_8002304C active-C, func_8003DE14 ASM-SUSPECT, saTan4FireDisp_80048864 ASM-SUSPECT, func_8008C464 parked ASM-STRUCTURAL, func_80023F08 parked ASM-STRUCTURAL). None matched.
- verdict: CONFIRMED

## [s2] Declaring var_v1 as u16 (narrow type) will steer GCC toward the oracle's $a0/$v1 unmasked/masked split.
- mechanism: u16 lvalue triggers extra codegen at store points; may bias RA to keep an unmasked mirror.
- probe: Applied `u16 var_v1;` in src/main.c; sandbox --disable all.
- result: score=11, build_insns=52 (== target 52 but 11 mismatched positions); WORSE than u32 floor=10.
- verdict: KILLED

## [s2] Gate 1 (canonical-asm STRONG signals via scan_hand_coded) passes for func_800871D4.
- mechanism: S1 multu pacing / S2 empty branches / S6 BIOS jumptable are the strong tier per hand-coded-asm-recognition.
- probe: python3 tools/scan_hand_coded.py --single func_800871D4 (tmp/grind/func_800871D4/s2/scan_hand_coded.txt).
- result: tier=LOW, score 0/8, zero strong signals; the function is a trivial bit-flag setter with linear control flow, zero call sites, no GTE/BIOS/HW ops.
- verdict: KILLED

## [s2] Gate 2 (in-hand SOTN or in-tree COMPLETED-C precedent for the lhu+andi-0xFFFF-from-u16-global coercion shape) passes.
- mechanism: Standing 2026-07-27 auto-ruling requires a CITED SOTN file+line or a COMPLETED-C sibling precedent for the coercion family.
- probe: s1 cross-tree BB2 sibling census (tmp/grind/func_800871D4/s1/find_sibling2.py); no SOTN checkout in-repo.
- result: 5 BB2 siblings share the exact shape, ALL INCOMPLETE (2 parked ASM-STRUCTURAL, 2 ASM-SUSPECT, 1 active-C); zero COMPLETED-C sibling; no in-hand SOTN cite. Per the standing ruling, negative census = FAILED gate, not open question.
- verdict: KILLED

## [s3] The redundant `andi $v1,$a0,0xFFFF` survives combine.c when the pseudo holding the raw `lhu` result has MORE THAN ONE USE, because can_combine_p refuses to substitute a load into an AND whose source register is still live afterwards - so nonzero_bits never gets to prove the value 16-bit-clean and fold the mask.
- mechanism: combine.c can_combine_p / try_combine require the i2 destination to be dead after the combination. With two uses of the load result the load-into-AND combination is rejected outright, and simplify_and_const_int (which is only reached inside try_combine) never runs on it. The AND is then emitted verbatim as `andi`.
- probe: `temp_a0 = D_8010280A; var_v1 = temp_a0 & 0xFFFF;` up front plus the semantically-required restore `var_v1 = temp_a0 & 0xFFFF;` at the end of the else arm (kept LIVE by the in-place `var_v1 -= 0x10;`), giving temp_a0 two uses. sandbox --disable all.
- result: score 10 -> 6; build_insns 50 -> 52 == target_insns 52; BOTH `andi $v1,$a0,0xFFFF` present at the target's positions; instruction stream structurally exact (objdump-verified, and inverse_compose classify reports register-blanked IDENTICAL).
- verdict: CONFIRMED

## [s3] Once both andis are present, the whole remaining residual is a global.c allocno-priority tie between the two symmetric mask locals, and it is REACHABLE from C.
- mechanism: global.c sorts allocnos by floor_log2(n_refs)*n_refs*size/live_length and assigns strictly ascending from the first free hard register. Pseudo 75 (then-arm mask, 3 refs / 19 insns) outranks pseudo 76 (else-arm mask, 3 refs / 21 insns), so 75 takes $a1 and 76 takes $a2; the target wants the opposite. No preference lever exists: $a1/$a2 never appear as hard regs in this function's pre-RA RTL, so set_preference cannot record one (8 preference atoms reported FORECLOSED).
- probe: tools/ra_solver/extract.py func_800871D4 main; tools/ra_solver/inverse.py global <model> --swap 75,76 --depth 2.
- result: REACHABLE, minimal solution size 1 atom, 18 distinct vectors across four families - refs_down 75, live_extend 75, live_shrink 76, refs_up 76.
- verdict: CONFIRMED

## [s3] The live_extend-75 atom is realisable in ordinary C by hoisting the else-arm's `var_a2 = 0` to the declaration, and it produces the target's register seats.
- mechanism: giving pseudo 75 a definition in block 0 extends its live range from 19 to >21 insns, dropping its priority below pseudo 76's and flipping the global.c order from `74 73 75 76` to `74 73 76 75`.
- probe: `s32 var_a2 = 0;` at the declaration, `var_a2 = 0;` removed from the else arm; sandbox --disable all + re-extraction of the RA model.
- result: dispositions 75->$a2, 76->$a1 - EXACTLY target. sandbox score = 3 (new floor), build_insns = 53.
- verdict: CONFIRMED

## [s3] The +1 instruction that the block-0 hoist costs can be recovered by moving the zero-store to a different position inside block 0.
- mechanism: if the zero-store were not the last schedulable insn before the branch, reorg.c would fill the `beqz` delay slot from the arm instead (with `li v0,1`, as the target does), keeping the constant 1 materialised once for both arms.
- probe: two spellings measured - declaration initialiser (`s32 var_a2 = 0;`) and plain statement immediately before the `if`.
- result: byte-IDENTICAL output for both (score 3, build_insns 53). sched2 ranks the zero-store last inside block 0 unconditionally because it has no successors there, so its source position is inert; reorg.c always gets it.
- verdict: KILLED

## [s3] Declaration order of the two mask locals steers the seat assignment on the new (52-insn, both-andi) chassis.
- mechanism: pseudo numbering follows declaration order at expand time and could break the priority tie.
- probe: var_a1 declared before var_a2 on the score-6 chassis; sandbox --disable all.
- result: score stays 6, no change in dispositions. The sort is driven by the priority quotient, and the two pseudos are not tied on it (.158 vs .143), so birth order never becomes the tie-break.
- verdict: KILLED

## [s3] Reordering statements inside the arms can flip the seats without cost.
- mechanism: emission order changes birth LUIDs and hence live_length.
- probe: then-arm `var_a1 = 0;` moved before the shift; sandbox --disable all.
- result: score=8, build_insns=53 - the `j` delay slot is lost because the shift is no longer the arm's last instruction. Strictly worse than the score-6 chassis.
- verdict: KILLED

## [s3] The redundant `andi $v1,$a0,0xFFFF` survives combine.c whenever the pseudo holding the raw `lhu` result has MORE THAN ONE USE, so the long-standing 'our cc1 always folds the mask, cc1psx does not' conclusion described a single-use spelling, not a fork divergence.
- mechanism: combine.c's can_combine_p / try_combine require the i2 destination to be dead after the combination. With two uses of the load result, the load-into-AND combination is rejected outright, and simplify_and_const_int (reachable only inside try_combine) never runs on the AND - so nonzero_bits never proves the value 16-bit-clean and the mask is emitted verbatim as `andi`.
- probe: Cache the load in `temp_a0` and mask it twice: `temp_a0 = D_8010280A; var_v1 = temp_a0 & 0xFFFF;` up front, plus the semantically-required restore `var_v1 = temp_a0 & 0xFFFF;` at the end of the else arm, kept LIVE by the in-place `var_v1 -= 0x10`. sandbox --disable all; objdump of tmp/sandbox/func_800871D4/main.o.
- result: score 10 -> 6; build_insns 50 -> 52 == target_insns 52; both `andi $v1,$a0,0xFFFF` present at the target's positions; instruction stream structurally exact (inverse_compose classify reports register-blanked IDENTICAL).
- verdict: CONFIRMED

## [s3] With both andis present, the entire remaining residual is a global.c allocno-priority tie between the two symmetric mask locals, and it is REACHABLE from C at a single atom.
- mechanism: global.c sorts allocnos by floor_log2(n_refs)*n_refs*size/live_length and assigns strictly ascending from the first free hard reg. Pseudo 75 (then-arm mask, 3 refs / 19 insns, pri .158) outranks pseudo 76 (else-arm mask, 3 refs / 21, pri .143), taking $a1; the target wants the opposite. No preference lever exists - $a1/$a2 never appear as hard regs in this function's pre-RA RTL, so set_preference cannot record one.
- probe: tools/ra_solver/extract.py func_800871D4 main; tools/ra_solver/inverse.py global tmp/ra_solver_work/func_800871D4.model.json --swap 75,76 --depth 2 --top 20.
- result: REACHABLE, minimal solution size 1 atom, 18 distinct vectors in four families: refs_down 75 (3->2, 3->1), live_extend 75 (19->23, 19->27), live_shrink 76 (21->17, 21->13), refs_up 76 (3->4 .. 3->15). 8 preference atoms reported FORECLOSED.
- verdict: CONFIRMED

## [s3] The live_extend-75 atom is realisable in ordinary C by hoisting the else arm's `var_a2 = 0` to the declaration, and it produces the target's register seats exactly.
- mechanism: A definition of pseudo 75 in block 0 extends its live range past 21 insns, dropping its priority below pseudo 76's and flipping the global.c order from `74 73 75 76` to `74 73 76 75`.
- probe: `s32 var_a2 = 0;` at the declaration with the else-arm store removed; sandbox --disable all plus re-extraction of the RA model.
- result: dispositions 75->$a2, 76->$a1 - exactly target. sandbox score 3 (new floor), build_insns 53. Objdump confirms the mask registers now match target throughout.
- verdict: CONFIRMED

## [s3] The +1 instruction the block-0 hoist costs can be recovered by repositioning the zero-store inside block 0 so reorg.c fills the `beqz` delay slot from the arm (with `li v0,1`) as the target does.
- mechanism: reorg.c's fill_simple_delay_slots takes the last schedulable insn before the branch; if the zero-store were not last, the constant 1 would stay shared between both arms instead of being materialised twice.
- probe: Two spellings measured - declaration initialiser (`s32 var_a2 = 0;`) and a plain statement immediately before the `if`.
- result: byte-IDENTICAL output for both (score 3, build_insns 53). sched2 ranks the zero-store last inside block 0 unconditionally (no successors in that block), so its source position is inert and reorg.c always claims it.
- verdict: KILLED

## [s3] Declaration order of the two mask locals steers the seat assignment on the new 52-instruction both-andi chassis (re-measure of the s0 finding on a changed chassis).
- mechanism: Pseudo numbering follows declaration order at expand time and could break the priority tie.
- probe: var_a1 declared before var_a2 on the score-6 chassis; sandbox --disable all.
- result: score stays 6, dispositions unchanged. The two pseudos are not tied on the priority quotient (.158 vs .143), so birth order never becomes the tie-break.
- verdict: KILLED

## [s3] Reordering statements inside the branch arms flips the seats at no instruction cost.
- mechanism: Emission order changes birth LUIDs and hence live_length.
- probe: then-arm `var_a1 = 0;` moved before the shift; sandbox --disable all.
- result: score 8, build_insns 53 - the `j` delay slot is lost because the shift is no longer the arm's last instruction. Strictly worse than the score-6 chassis.
- verdict: KILLED

## [s4] The `live_shrink pseudo 76` atom that s3 declared unreachable is in fact reachable, because the target's instruction ORDER constrains the emitted stream, not the source statement order.
- mechanism: global.c allocno_compare sorts on floor_log2(n_refs)*n_refs*size/live_length. The else-arm mask's live_length (21) is dominated by its block-3 contribution: it stays live from the join down to `or $a0,$a0,$a1`, 18 RTL insns in, only because the source completes the entire D_801078D8 read-modify-write group before starting the D_801078DA one. Interleaving the two groups at source level moves its last reference ~5 RTL insns earlier; sched1 (which runs before local/global alloc, so live_length is measured on ITS output) and sched2 are then free to re-emit the target's order. Live_length drops below the then-arm mask's 19, the sort flips, and the else-arm mask takes $a1 - the target's seat - with no block-0 store and therefore no extra instruction.
- probe: On the s3 score-6 chassis (rejected/dualuse-52insn-a1a2-seats-swapped.c), replace the tail with: both key-off loads, both key-on loads, both ORs, both stores, both masked write-backs. sandbox --disable all, then verify-oracle.
- result: score 6 -> 0, build_insns == target_insns == 52, rules_dropped 0. Full build SHA1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa (build_matches true). THE FUNCTION IS MATCHED.
- verdict: CONFIRMED

## [s4] A PARTIAL interleave suffices (moving only the D_801078DA `or` up).
- mechanism: if the death point were set by the first post-join reference, hoisting just the OR would shorten the range.
- probe: Two spellings - DA `or` moved next to the D8 `or`; DA `or` moved above the D_800F1B10 load. sandbox --disable all.
- result: score=6, build_insns=52 for BOTH. live_length is set by the LAST reference, so the DA store and the D_800F1B12 write-back must move up as well.
- verdict: KILLED

## [s4] The two `& 0xFFFF` masks can be replaced by a type-level narrowing (a `u16` local), which would remove the width-redundancy question entirely.
- mechanism: GCC 2.7.2 defines PROMOTE_MODE, so HImode locals live in SImode registers with an explicit zero-extension after each assignment - in principle the same `andi $v1,$a0,0xFFFF` the target needs, but emitted by the type system rather than by an explicit mask.
- probe: three spellings on the interleaved chassis - `u16 raw` + `u16 vc` with no mask; `u32 raw` + `u16 vc`; `u32 raw` + `u32 vc` with no mask (plain copies). sandbox --disable all.
- result: 55 insns / score 14; 54 insns / score 7; 53 insns / score 6 respectively. All strictly worse than 52. The extension GCC inserts after the in-place `vc -= 0x10` is the extra instruction in the first two; the third emits a copy instead of either `andi`. The explicit masks are load-bearing and no narrow-type spelling substitutes for them.
- verdict: KILLED

## [s4] A permuter campaign on the s3 score-3 chassis can find the seat flip.
- mechanism: randomized C perturbation over the whole body (perm_reorder_stmts, perm_temp_for_expr, perm_refer_to_var etc.) exploring exactly the statement-order neighbourhood the RA residual lives in.
- probe: hand-built workspace tools/decomp-permuter/nonmatchings/func_800871D4_s4 (full-TU cpp + cc1 -mel + maspsx + regfix + asmfix), 6 jobs, --stop-on-zero, PERM_RANDOMIZE over the entire body; ~2550 iterations.
- result: base_score 160; best novel find score 40 at 103s, then 130 and 145; never approached 0. The analytic route (reading s3's ra_solver live-range accounting and reasoning about which atom family costs no instruction) reached 0 first. Harvested with --stop.
- verdict: KILLED (as a route for THIS function; recorded as a negative campaign datum, log banked)

## [s5] The target's two redundant-looking `andi $v1,$a0,0xFFFF` are not redundant at all: they are GCC 2.7.2 PROMOTE_MODE zero-extensions of an ordinary `u16` LOCAL, and no mask of any kind is needed to obtain them.
- mechanism: PROMOTE_MODE puts a HImode local in an SImode pseudo without guaranteeing the upper bits are clean, so each `int`-context use of the local emits `zero_extend`. .flow shows `(set (reg/v:HI 75) (mem:HI D_8010280A))` then `(set (reg:SI 79) (zero_extend:SI (reg/v:HI 75)))` for the `sltiu`, and a second `zero_extend` of the same HI pseudo re-materialised at the end of the else arm once `voice - 16` has consumed the first extended copy. Both land at the target's exact positions, including the one inside the else arm that the then-path's `j` skips.
- probe: Declare the local as `u16 voice;` (the type psyz's Sony source uses) and delete every `& 0xFFFF` from the body; sandbox --disable all; read tmp/grind/func_800871D4/dumps/main.flow and main.s.
- result: both `andi`s present at the target positions, 52 insns, and the residual is purely emission order + the known $a1/$a2 seat swap. The s1/s2 "our cc1 folds the mask, cc1psx does not" finding and the s3/s4 dual-use-of-the-raw-load workaround were both artefacts of writing the local as `u32` with an explicit mask.
- verdict: CONFIRMED

## [s5] psyz's PsyQ 4.0 decomp of LIBSND/VM_NOWOF (`_SsVmKeyOffNow`) is a directly usable seed for this function, not merely a name identification.
- mechanism: BB2 links an interim 4.0-lineage libsnd; the psyz body differs from BB2 only in the `_svm_voice[]` stride (52 vs BB2's 54, the documented SpuVoice growth) per memory/grind/note2pitch/psyz-sweep-2026-08-18.md.
- probe: Transplant the psyz body verbatim with BB2 symbol names and `idx = voice * 54`; sandbox --disable all.
- result: score 8, build_insns 52, ALL registers already correct. `voice * 54` reproduces the target's `sll 3 / subu / sll 2 / subu / sll 1` strength reduction exactly. Only the emission position of the two key-off loads diverges. Banked rejected/s5-psyz-verbatim-order-score8.c.
- verdict: CONFIRMED

## [s5] GCC 2.7.2's schedulers treat a store whose address is `(plus (reg) (symbol_ref))` as an absolute memory barrier, so the target's "key-off loads above the voice clears, key-off stores below them" order is unreachable from read-modify-write source.
- mechanism: the 2.7.2 alias check cannot disambiguate `_svm_voice[voice].field` from a fixed global, so no load or store crosses it. Two plain `(symbol_ref)` references with different symbols ARE disambiguated and reorder freely (the key-on loads hoist over the key-off stores in every measured form).
- probe: Move the two `_svm_okof* |=` read-modify-writes above the two `_svm_voice[voice]` clears; sandbox --disable all; compare emitted memory order with source order.
- result: score 12, 52 insns; emitted memory order follows source EXACTLY - both the loads and the stores stayed above the clears. Banked rejected/s5-okof-rmw-before-clears-score12.c. Corollary: the target's order forces source in which the key-off LOADS are separated from the key-off STORES by the two clears.
- verdict: CONFIRMED

## [s5] With the key-off loads separated from the stores but the write-backs still done one group at a time, the emission order is exact and the last residual is the known global.c allocno-priority tie between the two mask locals.
- mechanism: global.c allocno_compare sorts on floor_log2(n_refs)*n_refs*size/live_length; bitsLower is 3 refs / 19 (pri 1578) and bitsUpper 3 refs / 21 (pri 1428), so bitsLower sorts first and takes $a1 while the target wants bitsUpper there.
- probe: `old1 = D_801078D8; old2 = D_801078DA;` above the clears, then `okof1 = old1 | bitsLower; D_801078D8 = okof1; D_800F1B10 &= ~okof1;` and the DA group after them; sandbox --disable all + tools/ra_solver/extract.py + inverse.py global --swap 73,74.
- result: score 6, 52 insns, instruction-exact stream. inverse.py: REACHABLE at 1 atom; cheapest are `live_shrink 73: 21->19` (a TIE suffices - the lower allocno number wins it) and `live_extend 74: 19->21`. Banked rejected/s5-grouped-writeback-score6-a1a2-swap.c.
- verdict: CONFIRMED

## [s5] Source-level reordering of the pure-register operations inside the grouped-write-back shape can reach that atom.
- mechanism: emission order of the `or`s changes the pseudos' death points and hence live_length.
- probe: four spellings, all measured with sandbox --disable all and (for the first) a re-extracted RA model: compute `okof2` before `okof1`; hoist both key-on loads into locals ahead of both ORs; hoist only `on1`; the OR swap plus the `on1` hoist together (rejected/s5-or-swap-plus-keyon-hoist-score6.c).
- result: ALL score 6 / 52 insns; the re-extracted model is unchanged (74 livelen 19, 73 livelen 21, dispositions identical). sched1 sinks each `or` to sit immediately before its own store regardless of source order, so bitsUpper's live range always exceeds bitsLower's by exactly the intervening key-on-1 group.
- verdict: KILLED

## [s5] Reusing bitsLower as the key-off accumulator (`bitsLower |= old1;`) is a usable live_extend lever.
- mechanism: it extends bitsLower's live range to its store and the following `nor`, which is the `live_extend 74: 19->21` atom.
- probe: `bitsLower |= old1; D_801078D8 = bitsLower; D_800F1B10 &= ~bitsLower;`; sandbox --disable all.
- result: score 11, 52 insns. The lever works on live_length but retargets the `or`'s destination to the mask's register, emitting `or $a2,$v1,$a2` where target has `or $v1,$v1,$a2`. Banked rejected/s5-reuse-bitslower-accumulator-11.c.
- verdict: KILLED

## [s5] Advancing the two 24-voice halves in lockstep reaches the `live_shrink 73` atom at zero instruction cost and closes the function.
- mechanism: reading both key-off words, clearing the voice slot, reading both key-on words, applying both masks, then storing both key-off words and both key-on words moves bitsUpper's last reference above the entire key-on-1 group. Its live_length drops to <= bitsLower's, so global.c allocno_compare sorts bitsUpper first and it takes $a1 - the target's seat - with no added instruction. The key-on stores may sit below the key-off stores in source because plain `(symbol_ref)` memory references with different symbols are disambiguated and reorder freely; only the `(plus reg symbol_ref)` voice-slot stores are barriers.
- probe: the body now in src/main.c and memory/grind/func_800871D4/candidate.c; sandbox --disable all, then verify-oracle.
- result: **score 0**, build_insns == target_insns == 52, rules_dropped 0; verify-oracle ok=true, build_sha1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa, build_matches true. Zero regfix/asmfix rules, zero inline asm, zero `/* FAKE */`, zero mask, no claimed coercion family. Self-vet: memory/grind/func_800871D4/self_vet.md.
- verdict: CONFIRMED
