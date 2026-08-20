/* func_80072CD4 - CLEAN candidate, `sandbox func_80072CD4 --disable all` = 4, build_insns 79 ==
 * target_insns 79, rules_dropped 0. Re-measured on the current chassis by grind session s5
 * (rederive modality, 2026-08-20); artifact tmp/grind/func_80072CD4/s5r/base.dis.
 *
 * This file deliberately holds the CLEAN floor-4 body, NOT the sandbox-0 per-arm-triple body.
 * The per-arm body scores 0 (re-measured 0/79 this session) but is a BANNED CONSTRUCT for this
 * function: the layer-1 cheat-reviewer FAILed it twice (2026-08-20 05:53 and 06:20) as a
 * respelling of the Judge-FAILed @4/@0xC duplicated-into-arms store-schedule construct, and the
 * driver's banned_constructs list names the whole body. It is banked, with its measurement, in
 * rejected/layer1-fail-0820-0553.c and
 * rejected/rederive_polyg4_struct_perarm_score0_banned_family.c. Do NOT re-apply either as a
 * candidate; the open question about them is a ruling question (see the s5-rederive
 * ruling-request in the session outcome), never a submission.
 *
 * Pure C: one local `int fc_const`. No asm/pins/volatile/barrier/do-while/dead store/duplication.
 *
 * WHAT THE RESIDUAL 4 IS (s5-rederive, established from EMITTED BYTES alone - no cc1 dump needed):
 *  target merge block  : sb v1,4 | sb v1,0xC | sb v0,0xE | li 0xFC | sb 0x14 | ... | sb zero,0x16 | sb v0,0x1E
 *  this body's merge   : sb v0,0xE | li 0xFC | sb 0x14 | ... | sb v1,4 | sb v1,0xC | sb zero,0x16 | sb v0,0x1E
 *  i.e. the two `sb v1` stores sit at the merge TAIL instead of the merge HEAD. 4 differing insns.
 *
 * THE TWO CODEGEN LAWS THAT PIN IT (both demonstrated by controls measured this session):
 *  L1 - PRODUCER-LESS MERGE-BLOCK STORES SINK TO THE MERGE TAIL. A store whose value register is
 *       defined in a PREDECESSOR block has no in-block dependence, is ready in sched2's first
 *       bottom-up round, and being picked first bottom-up puts it LAST in the emitted block.
 *       Control A (this body): `sb v1,4`/`sb v1,0xC` (v1 = fc_const, predecessor-defined) sink.
 *       Control B (rejected/rederive_merge_literals_no_fcholder_6_77.c): drop the fc_const local
 *       and write literal 0xFC in the merge block - the stores acquire an IN-BLOCK `li` producer
 *       and immediately move to the merge head (`li v0,252 / sb v0,4 / sb v0,12 / sb v0,20`),
 *       score 6 / 77 insns. Same stores, same block, opposite placement, only the producer moved.
 *       TARGET ITSELF OBEYS L1: its one producer-less merge-block store, `sb zero,0x16`, is at the
 *       merge TAIL (0x80072D94), out of ascending field order, exactly where L1 puts it.
 *  L2 - A jump2 COMMON TAIL IS SPLICED AT THE JOIN LABEL, i.e. AHEAD OF EVERYTHING sched2 EMITTED
 *       for the merge block (toplev.c pass order: sched2 -> jump_optimize(cross_jump=1)). So a
 *       cross-jumped store can occupy merge position 0 and a merge-block source store never can.
 *       Control (this body): `sb v0,0xE`, written per-arm in source and cross-jumped, IS at merge
 *       position 0 - ahead of the merge-block stores that L1 sank.
 *
 * THE CONSEQUENCE (a reconstruction proof, not a preference): in target, `sb v1,4` and
 * `sb v1,0xC` are at merge positions 0 and 1 with v1 defined in a predecessor, and `sb v0,0xE`
 * follows at position 2 with v0 defined at the arm tails. By L1 they cannot be merge-block source
 * statements (they would have sunk, like `sb zero,0x16` does). By L2 nothing a merge-block source
 * statement produces can precede a cross-jumped insn, so they cannot be merge-block statements
 * sitting ahead of a cross-jumped `sb v0,0xE` either. The only remaining producer is L2 itself:
 * all THREE are one 3-insn jump2 common tail, i.e. the original C wrote @4, @0xC and @0xE inside
 * BOTH inner arms. That is precisely the body the layer-1 reviewer banned.
 *
 * AXES MEASURED DEAD BY s5-rederive (all on the current chassis, scores in the filenames):
 *  - cross-block `var_v0` (xblock) in four fresh spellings: var-assignment first in the arm,
 *    s32-typed var, fc_const hoisted above the OUTER if (sibling func_80072BC4's house style):
 *    13/78, 13/78, 14/78. All lose the same way - sched1 hoists the producer-less `li var_v0` to
 *    the arm TOP, which makes the two arm tails identical and lets jump2 cross-jump `sb v0,0xD`
 *    out of them (78 insns, one short of target's 79, which keeps `sb v0,0xD` in both arms).
 *  - the 0x14/0x15/0x16/0x1C/0x1D/0x1E group moved ABOVE the inner if: 33/78.
 *  - defeating L1 by memory-aliasing instead of by moving the producer:
 *    rejected/rederive_walkptr_alias_serialize_6_79.c uses a second base pointer
 *    (`u8 *q = (u8 *)arg1 + 4;  q[0] = fc_const;  q[8] = fc_const;`) so GCC 2.7.2's
 *    memrefs_conflict_p cannot disambiguate q-based from s1-based MEMs and serializes them.
 *    It WORKS as a mechanism - the two stores leave the merge tail and land immediately after the
 *    cross-jumped `sb v0,0xE` - but it scores 6/79, worse than 4, because L2 still owns merge
 *    position 0. This is the closest any non-per-arm form has come and it confirms L2 is the
 *    binding constraint, not L1.
 */
s32 func_80072CD4(s32 arg0, GameObj *arg1) {
    int fc_const;

    SetPolyG4(arg1);
    SetSemiTrans(arg1, 0);
    if (arg0 < 4) {
        fc_const = 0xFC;
        if (*(s32 *)((s32)(D_800A35C4) + 8) & 4) {
            *(u8 *)((s32)(arg1) + 5) = 0xC3;
            *(u8 *)((s32)(arg1) + 6) = 0x1E;
            *(u8 *)((s32)(arg1) + 0xD) = 0xC8;
            *(u8 *)((s32)(arg1) + 0xE) = 0x32;
        } else {
            *(u8 *)((s32)(arg1) + 5) = 0xC3;
            *(u8 *)((s32)(arg1) + 6) = 0x50;
            *(u8 *)((s32)(arg1) + 0xD) = 0xDC;
            *(u8 *)((s32)(arg1) + 0xE) = 0x46;
        }
        *(u8 *)((s32)(arg1) + 4) = fc_const;
        *(u8 *)((s32)(arg1) + 0xC) = fc_const;
        *(u8 *)((s32)(arg1) + 0x14) = 0xFC;
        *(u8 *)((s32)(arg1) + 0x15) = 0x82;
        *(u8 *)((s32)(arg1) + 0x1C) = 0x32;
        *(u8 *)((s32)(arg1) + 0x1D) = 0x28;
        *(u8 *)((s32)(arg1) + 0x16) = 0;
        *(u8 *)((s32)(arg1) + 0x1E) = 0xA;
    } else {
        *(u8 *)((s32)(arg1) + 4) = 0x10;
        *(u8 *)((s32)(arg1) + 5) = 0x30;
        *(u8 *)((s32)(arg1) + 6) = 0x60;
        *(u8 *)((s32)(arg1) + 0xC) = 0x18;
        *(u8 *)((s32)(arg1) + 0xD) = 0;
        *(u8 *)((s32)(arg1) + 0xE) = 0x40;
        *(u8 *)((s32)(arg1) + 0x14) = 0x30;
        *(u8 *)((s32)(arg1) + 0x15) = 0;
        *(u8 *)((s32)(arg1) + 0x16) = 0x60;
        *(u8 *)((s32)(arg1) + 0x1C) = 0;
        *(u8 *)((s32)(arg1) + 0x1D) = 0;
        *(u8 *)((s32)(arg1) + 0x1E) = 0;
    }
    AddPrim(D_800A374C + 0x60, arg1);
    return (s32)((u8 *)arg1 + 0x24);
}
