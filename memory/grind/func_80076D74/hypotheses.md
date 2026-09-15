# Hypothesis ledger — func_80076D74

## s1 (recon) — measured
- H1 KILLED (instance): table byte via pointer arithmetic / u8[] extern / byte-offset cast — 53 each; symbol forced+hoisted (expr.c force_operand + loop.c).
- H2 CONFIRMED: u8 pair table `extern u8 D_8009BCF8[][2]` + `D_8009BCF8[idx][1]` -> in-loop lui/addu/lbu form. 53 -> 49.
- H3 CONFIRMED: record struct with cells[2][5][2] + bitfields (pad10, f10:2, f12:2, f14:1, f15:2) at 0x14. 49 -> 39.
- H4 CONFIRMED: `(j << 1)` instead of `j * 2` in the two D_800A36A0-side halfword indexes flips plus operand order -> local-alloc tie -> all loop seats match. 39 -> 5.
- H5 KILLED (instance): tail MEM_IN_STRUCT flag (s32 arg0 + cast offsets) — 5, unchanged.

## Live frontier (floor 5 = epilogue only)
- F1: the return copy `move $v0,$s3` must be scheduled after `sw $v0,24($s1)` in sched1. Candidate RTL shapes to spell in ordinary C and measure: (a) two-copy chain (return value copied through a second pseudo so the v0 set trails by one cycle — check whether any natural spelling of the return/flag produces it); (b) a basic-block boundary before the return (a label between the increment and `return ret` — only natural if the original control flow puts a join there; the current if-block joins at the draw code top, so this needs a different block structure; measure before believing); (c) run tools/sched_solver on the tail BB (insns 391..402 of the .sched dump) to get the exact dependence set that yields the target order, then find the C that carries it.
- F2: if F1 is closed the function should be bytes-proven; run verify-oracle only then.

## [s1] Table byte spelled as (&D_8009BCF9)[idx*2], as extern u8 D_8009BCF9[] indexed [idx*2], or as *(u8*)((u8*)&D_8009BCF9 + idx*2) reproduces the target's in-loop lui $at/addu $at/lbu %lo form
- mechanism: expr.c force_operand materialises the symbol in a register when the address is PLUS(symbol, MULT); loop.c hoists that constant set out of the loop
- probe: sandbox --disable all on the three spellings (body_v1, body_v2A, body_v2B)
- result: 53 / 53 / 53 â€” lui/addiu $t3 hoisted before the loop, addu $v0,$v0,$t3; lbu $v0,0($v0) inside; rtl dump insn 228 (set (reg 177) (symbol_ref D_8009BCF9)) in all three
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main chassis (-mel -msoft-float), raw-mask word ops, no FAKE constructs

## [s1] Declaring the table as a u8 pair table extern u8 D_8009BCF8[][2] and reading D_8009BCF8[idx][1] produces the target's in-loop lui $at/addu $at/lbu %lo(D_8009BCF9) form
- mechanism: expr.c ARRAY_REF -> get_inner_reference: offset idx*2 computed into a register, address plus(symbol, reg), bitpos +1 folded into the symbol constant; no separate symbol insn for loop.c to hoist
- probe: sandbox on body_v2C (only this change vs body_v1)
- result: 53 -> 49; table access lines now byte-equal; linked lo16 of D_8009BCF8+1 == D_8009BCF9
- verdict: CONFIRMED

## [s1] The record at *(u8**)D_800A36A0 is a struct { u8 cells[2][5][2]; u32 bitfields pad:10,f10:2,f12:2,f14:1,f15:2 } and spelling it so reproduces the bitfield block schedule and the store addressing
- mechanism: MEM_IN_STRUCT_P on bitfield stores lets sched.c true_dependence (sched.c:817) exempt the fixed-address gp reload; get_inner_reference emits offset j*2+i*10 so no hdr+i*10 invariant exists for loop.c to hoist
- probe: sandbox on body_v3D
- result: 49 -> 39; lw $a1,gp hoisted above the f12 sw, and $a2,$v1,$a2 before the f14 sw, addu $a0,$v1,$a1; addu $a0,$a3,$a0 all match
- verdict: CONFIRMED

## [s1] Writing the two D_800A36A0-side halfword indexes with (j << 1) instead of j * 2 makes j2 keep $v1 and fixes every loop register seat
- mechanism: expr.c PLUS_EXPR under EXPAND_SUM orders a MULT term first; local-alloc.c ties the address temp to the first dying operand, extending j2's qty past the j+1 temp; a plain-register term flips the order so the tie goes to the base sum instead
- probe: sandbox on body_v4E; .sched/.lreg dumps of body_v3D show qty {141,143,198} vs 201 overlap
- result: 39 -> 5; only the epilogue differs
- verdict: CONFIRMED

## [s1] Spelling the packet-list argument as s32 arg0 with *(s32*)(arg0+0x18) cast offsets (clearing MEM_IN_STRUCT on the tail lw/sw) moves the return copy after the store
- mechanism: MEM_IN_STRUCT_P is set only for INDIRECT_REF of a PLUS_EXPR (expr.c:4570); a cast inserts a NOP_EXPR and clears it, changing sched.c true_dependence outcomes
- probe: sandbox on body_v5F
- result: 5, byte-identical residual to body_v4E â€” the tail diff does not depend on the in-struct flag
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main chassis, body_v4E form otherwise, no FAKE constructs

## s2 (structural) - measured
- H6 KILLED (instance): `u8 ret` two-copy return chain - 5, unchanged; copies stay at priority 8 (anti-dep on the call only), hoisted into the load-delay slot.
- H7 CONFIRMED: single-level `do { arg0[6] += 0xC; } while (0);` (FAKE, do-while-zero family) - 5 -> 0. sched.c loop_notes make the return copy depend on the sw.
- H8 CONFIRMED: empty `do { } while (0);` between the increment and `return ret;` - also 0 (same mechanism); not banked as the candidate (H7 preferred, wraps the controlled statement).

## [s2] Declaring ret as u8 so `return ret` expands to a promoted-subreg two-copy chain schedules the return copy after the sw
- mechanism: expr.c convert_move on a SUBREG_PROMOTED_VAR_P source emits copy-to-pseudo then copy-to-v0; if either copy gained priority >= 9 it would sort after the sw
- probe: sandbox on body_vA_u8ret.c
- result: 5 - residual byte-identical to the s32 form; both copies depend only on the call (priority 8) and are hoisted before the addiu
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main chassis (-mel -msoft-float), s1 candidate body otherwise, no FAKE constructs

## [s2] Wrapping the final `arg0[6] += 0xC;` in a single-level do { } while (0) pins the return copy after the sw
- mechanism: sched.c:2287 loop_notes - the first insn after NOTE_INSN_LOOP_END (the return copy) gets dependences on every earlier set/use in the block, so it is not ready while the sw is unscheduled; the increment temp then takes $v0
- probe: sandbox on body_vC_wrap_incr.c; fresh .sched dump of the form
- result: 0 - tail lw/nop/addiu/sw/move v0,s3 matches; new dump T-2 ready list = the copy alone
- verdict: CONFIRMED

## Live frontier
- none: sandbox 0 with the FAKE-annotated single-level wrap. If the Judge FAILs the wrap, the remaining ordinary-C axis is a block structure that places a label between the increment and the return copy (no natural geometry found in s2: the if-join sits before the draw code, the tail has no branches).

## s2 (permuter, 2026-09-15 re-run; the first s2 pass was DISCARDED for a scope violation - its measurements are re-taken here) - measured
- H9 CONFIRMED (re-measured this session): 2-byte record table `typedef struct { u8 unk0; u8 unk1; } Unk8009BCF8Record; extern Unk8009BCF8Record D_8009BCF8[20];` read as `D_8009BCF8[idx].unk1` reproduces the in-loop table read: 5 without the wrap, 0 with the do-while(0) wrap (codegen identical to the BANNED `u8 [][2]` + `[idx][1]` spelling). Decl placed TU-local in text1b.c ONLY for measurement (grind surface excludes include/); canonical placement = memory/grind/func_80076D74/record_table_decl.patch (include/game.h).
- H10 KILLED (instance): permuter campaign on the no-FAKE floor-5 record-table chassis (s32 ret seed), 34,745 iterations / 942 s (discarded first pass, log kept at tmp/grind/func_80076D74/s2/perm_record-table-nofake-floor5/campaign.log): no ordinary-C tail find; both finds are do-while(0) forms.
- H11 KILLED (instance): permuter campaign seeded on the `u8 ret` (two-copy chain) record-table chassis, label u8ret-record-table-floor5 (tmp/perm_76D74_s2b) - result in the H11 block below.

## [s2-permuter] A record-table declaration (typedef struct { u8 unk0; u8 unk1; } Unk8009BCF8Record; extern Unk8009BCF8Record D_8009BCF8[20];) read as D_8009BCF8[idx].unk1 reproduces the target's in-loop lui $at/addu $at/lbu %lo(D_8009BCF9) form exactly like the banned [..][1] pair-table spelling
- mechanism: expr.c COMPONENT_REF over a variable-index ARRAY_REF goes through get_inner_reference: the offset idx*2 is forced into a register, the address is plus(symbol, reg) and the .unk1 byte position folds into the symbol constant -> `lbu $v0,%lo(D_8009BCF8+1)($at)` == %lo(D_8009BCF9); there is no separate `reg = symbol` insn for loop.c to hoist (the 1-D `D_8009BCF9[idx*2]` spelling builds *(&array + idx*2) via the expr.c ARRAY_REF nonconstant-index path, which force_operand splits and loop.c hoists - s1 H1, 53)
- probe: sandbox --disable all on body_p5_record.c (no wrap) and body_p0_record.c (single-level do-while(0) wrap), TU-local decl via tmp/grind/func_80076D74/s2/decl.py, HEAD chassis
- result: 5 / 0 - identical to the s1/s2 numbers for the [..][1] spelling; the linked byte is the same %lo(D_8009BCF9)
- verdict: CONFIRMED

## [s2-permuter] A random-mutation permuter campaign seeded on the no-FAKE floor-5 record-table chassis (s32 ret) finds an ordinary-C spelling that schedules the return copy after the arg0[6] store
- mechanism: sched.c priority(): the copy needs a dependence on the lw/addiu/sw chain or a block boundary; the permuter's reorder / temp-split / cast / self-assign / do-while passes could expose one
- probe: tools/permuter_campaign.py launch/harvest, label record-table-nofake-floor5, -j 8, 34,745 iterations, 942 s (first s2 pass; log + finds preserved under tmp/grind/func_80076D74/s2/perm_record-table-nofake-floor5/ and perm_find_output-*.c)
- result: two finds only - output-10-1 (permuter score 10) = an empty `do { } while (0);` inserted between the increment and `return ret;` (the s2 H8 form, sanctioned family); output-130-1 (130) = the whole draw block wrapped in do-while(0) plus a dead `j = 0` (worse; the dead store is not a sanctioned shape for that use). No non-do-while find in 34.7k iterations.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main chassis (-mel -msoft-float), record-table decl, s32 ret, no FAKE constructs in the seed

## [s2-permuter] A random-mutation permuter campaign seeded on the `u8 ret` (promoted-subreg two-copy return chain) record-table chassis finds an ordinary-C spelling that schedules the return copy after the arg0[6] store
- mechanism: same sched.c priority() gap as H10; the two-copy chain gives the permuter a second copy insn to perturb (temp splits / reorders around the return), a structurally different tail from campaign 1's single-copy seed
- probe: tools/permuter_campaign.py launch/wait/harvest, label u8ret-record-table-floor5, workspace tmp/perm_76D74_s2b (base.c = campaign 1 base with `u8 ret;` at the func_80076D74 decl only), -j 8, 43,339 iterations, 1,169 s, two in-turn wait windows (398 s + 530 s), harvest --stop
- result: three finds, all the do-while(0) attractor: output-10-1 (score 10) = empty `do { } while (0);` before `return ret;` plus permuter noise (`D_8009BCF8 = D_8009BCF8;` dead global self-assign, `(0, arg0[5])` comma, decl reorder); output-170-1 (170) = draw block from `p+5` on wrapped in do-while(0) plus `j = 5; arg0[j]`; output-70-1 (70) = tail four statements wrapped in do-while(0) plus a `new_var = &arg0[6]` pointer temp. No find without a do-while(0). Diffs saved as tmp/grind/func_80076D74/s2/perm_u8ret_find_output-{10,170,70}-1.diff.txt, log perm_u8ret-record-table-floor5.campaign.log
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main chassis (-mel -msoft-float), record-table decl, u8 ret two-copy tail, no FAKE constructs in the seed

## Live frontier (after s2-permuter re-run)
- RULING NEEDED (this session's outcome): does the record-table declaration `typedef struct { u8 unk0; u8 unk1; } Unk8009BCF8Record; extern Unk8009BCF8Record D_8009BCF8[20];` (include/game.h) with the evidence in evidence.md s2 satisfy prongs (a)-(e) of the per-word-splat -> aggregate merge family and supersede the 2026-09-15 12:46 layer-1 ban on the `u8 [][2]` pair table? If yes, body_p0_record.c (sandbox 0) + record_table_decl.patch is the candidate; the header/text1b_b.c/undefined_syms_auto.txt edits are outside the grind surface and need an operator/driver integration step.
- Ordinary-C tail fix (would remove the FAKE wrap): two permuter basins (34.7k + 43.3k iterations, s32 and u8 ret seeds) yield only do-while(0). Remaining untried ordinary-C axis: a block structure with a second predecessor of the tail (a label before the return copy); no natural geometry identified in s1-s2 - the if-join sits before the draw code and the tail has no branches.

## s3 (permuter, 2026-09-15) - measured
- H12 CONFIRMED: the Judge-cleared body + TU-local record-table decl gives sandbox 0 AND a full-link SHA1 == oracle on HEAD (bytes proven on main; declaration placement is byte-neutral).
- H13 KILLED (instance): pointer-to-slot tail `s32 *dm = &arg0[6]; ... *dm += 0xC;` - 5, combine folds it back to the plain form.
- H14 KILLED (instance): typed packet pointer round trip `u8 *q = (u8 *)arg0[6]; ... arg0[6] = (s32)(q + 0xC)` - 21, pointer held across the calls.
- H15 KILLED (instance): branch-on-ret tail `if (ret) { return 1; } return 0;` - 5, folded back to `move v0,s3`, same hoisted copy.
- H16 KILLED (instance): permuter campaign seeded on the branch-on-ret chassis (31,554 iterations) - only do-while(0) forms and semantic noise.

## [s3-permuter] The Judge-cleared candidate body with the Unk8009BCF8Record[20] declaration TU-local in text1b.c compiles and links to the oracle executable (bytes proven on main, declaration placement byte-neutral)
- mechanism: the record-table extern + typedef produce the same RTL wherever the declaration sits (expr.c get_inner_reference path, evidence.md s1/s2); the linker resolves D_8009BCF8 from undefined_syms_auto.txt:1252; the stale D_8009BCF9 row is unreferenced once asm/funcs/func_80076D74.s is not assembled
- probe: decl.py + apply2.py candidate.c on src/text1b.c, sandbox --disable all, then tmp-only full build (fullbuild.py: copy of build/, rebuild text1b.o via engine.pipeline.build_c_object, link via a tmp copy of bb2.ld, objcopy, make_psexe, SHA1)
- result: sandbox 0; SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == ORACLE (tmp/grind/func_80076D74/s3/fullbuild.log)
- verdict: CONFIRMED

## [s3-permuter] Spelling the drawmode-packet tail through a pointer local to the slot (`s32 *dm = &arg0[6]; SetDrawMode(*dm,..); AddPrim(.., *dm); *dm += 0xC;`) changes the tail RTL enough to schedule the return copy after the store
- mechanism: a separate address pseudo could give the lw/addiu/sw chain or the copy a different dependence shape in sched1
- probe: sandbox --disable all on body_v1_dmptr.c
- result: 5 - residual byte-identical to the plain `arg0[6] += 0xC` tail; combine folds the address pseudo into the MEMs before sched1 runs
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main chassis (-mel -msoft-float), record-table decl TU-local, s32 ret, no FAKE constructs

## [s3-permuter] Loading the drawmode packet pointer once (`u8 *q = (u8 *)arg0[6];`), passing it to both calls and storing `arg0[6] = (s32)(q + 0xC)` reproduces the target tail
- mechanism: a single load with the increment applied to the loaded value would make the store depend on a value computed before the calls, possibly re-ordering the tail
- probe: sandbox --disable all on body_v2_qptr.c
- result: 21 - q is held in a callee-saved register across SetDrawMode/AddPrim; the target reloads lw 24($s1) after the calls. Worse than the plain tail
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main chassis (-mel -msoft-float), record-table decl TU-local, s32 ret, no FAKE constructs

## [s3-permuter] Spelling the return as a branch (`if (ret) { return 1; } return 0;`) after `arg0[6] = arg0[6] + 0xC;` places a basic-block boundary before the return copy
- mechanism: a conditional return creates extra blocks; if the copy of ret into v0 lands in a block after the store, sched1 cannot hoist it into the load-delay slot
- probe: sandbox --disable all on body_v3_retbranch.c
- result: 5 - jump.c / cse fold the branch to the same `move v0,s3` in the same block; residual byte-identical to the plain tail
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main chassis (-mel -msoft-float), record-table decl TU-local, s32 ret, no FAKE constructs

## [s3-permuter] A random-mutation permuter campaign seeded on the branch-on-ret tail chassis (structurally different from the s2 s32-ret and u8-ret seeds) finds an ordinary-C spelling that schedules the return copy after the arg0[6] store
- mechanism: same sched.c priority() gap as H10/H11; the conditional-return seed exposes if/return statement mutations (reorder, temp-for-expr, split) the single-return seeds did not
- probe: tools/permuter_campaign.py launch/wait/harvest --stop, label retbranch-record-table-floor5, workspace tmp/grind/func_80076D74/s3/perm_ws_retbranch, -j 8, 31,554 iterations / 865 s, base permuter score 190
- result: four finds, none an ordinary-C tail fix: output-115-1 semantically broken (constant return / fall-off), output-70-1 do-while(0) around the last five statements, output-10-1 empty do-while(0) before the return (= s2 H8), output-190-1 base-score noise. No novel find in the last 545 s
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD main chassis (-mel -msoft-float), record-table decl TU-local, branch-on-ret tail, no FAKE constructs in the seed

## Live frontier (after s3)
- INTEGRATION HANDOFF (docs/grind/decisions.md 2026-09-15 s3 entry): bytes are proven on main; the only remaining delta is the operator-applied declaration placement (include/game.h) + two byte-neutral deletions (src/text1b_b.c:237-238, undefined_syms_auto.txt:79) per the 13:47 ruling. Once applied, the next session's candidate-ready is candidate.c verbatim (Judge-cleared body hash ffd478b35c7a6afd) with the scalar externs at text1b.c:2227-2228 removed.
- Ordinary-C removal of the do-while(0) wrap: three permuter basins (109.6k iterations) and six hand tails (s32/u8 ret, cast offsets, slot pointer, packet-pointer round trip, branch-on-ret) all leave the return copy at sched.c priority 8; still-untried: a tail spelling where the store to arg0[6] and the return value share a pseudo through a real data dependence (no natural form identified - the function's semantics give the return value no relation to the packet cursor).

## [s2-rerun-permuter] The Judge-cleared candidate body (hash ffd478b35c7a6afd) with the Unk8009BCF8Record[20] declaration TU-local re-measures sandbox 0 and links to the oracle executable from clean HEAD 493ad9e97
- mechanism: same as the s3 H12 entry - the record-table extern/typedef produce the same RTL wherever the declaration sits; the linker resolves D_8009BCF8 from undefined_syms_auto.txt:1252
- probe: s2/decl.py + s2/apply2.py candidate.c; `sandbox func_80076D74 --disable all`; s2/handoff/fullbuild.py tmp-only full link
- result: sandbox score 0 (161/161, rules_dropped 0); SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle (tmp/grind/func_80076D74/s2/handoff/fullbuild.log)
- verdict: CONFIRMED

## Live frontier (after s2-rerun)
- INTEGRATION HANDOFF filed with the validator-required title (docs/grind/decisions.md `2026-09-15 - func_80076D74 (src/text1b.c) - OWNER-ESCALATION - INTEGRATION HANDOFF`). Expected driver path: Judge ESCALATE(integration-handoff, scope_paths=[include/game.h, src/text1b_b.c, undefined_syms_auto.txt]) -> scope_allow.txt grant -> function stays ACTIVE -> next session applies record_table_decl.patch (corrected: game.h add, text1b_b.c delete, undefined_syms_auto.txt:79 delete), deletes text1b.c:2227-2228, applies candidate.c VERBATIM via s2/apply2.py, re-measures 0/161, returns candidate-ready (layer-1 skipped under the 13:47 clearance; driver bytes + FINAL CALL).
- Ordinary-C removal of the do-while(0) wrap: unchanged from the s3 frontier (three basins + six hand tails all leave the return copy at sched.c priority 8; no natural data dependence between the return value and the packet cursor has been identified).

## [s2] The Judge-cleared candidate body (hash ffd478b35c7a6afd) with the Unk8009BCF8Record[20] declaration TU-local re-measures sandbox 0 and links to the oracle executable from clean HEAD 493ad9e97
- mechanism: record-table extern/typedef produce the same RTL wherever the declaration sits; linker resolves D_8009BCF8 from undefined_syms_auto.txt:1252; the D_8009BCF9 row is unreferenced once asm/funcs/func_80076D74.s is not assembled
- probe: s2/decl.py + s2/apply2.py candidate.c; sandbox func_80076D74 --disable all; s2/handoff/fullbuild.py tmp-only full link
- result: sandbox score 0, target_insns 161, build_insns 161, rules_dropped 0; SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle (tmp/grind/func_80076D74/s2/handoff/fullbuild.log)
- verdict: CONFIRMED
