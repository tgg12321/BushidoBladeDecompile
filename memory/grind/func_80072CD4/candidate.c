/* func_80072CD4 - CLEAN candidate (reviewer-passable): `sandbox func_80072CD4 --disable all` = 4,
 * build_insns 79 == target_insns 79, rules_dropped 0. Re-measured on the CURRENT chassis by
 * grind session s5-structural (2026-08-20); this file is byte-identical to fallback_floor4.c.
 *
 * WHY THIS FILE IS THE FLOOR-4 BODY AND NOT THE SANDBOX-0 BODY.
 * The sandbox-0 form (per-arm complete rgb0/rgb1 POLY_G4 triples, i.e. @4=0xFC and @0xC=0xFC
 * written inside BOTH inner arms) is a BANNED construct for this function: the layer-1
 * cheat-reviewer FAILed it three times (2026-08-20 05:53, 06:20, 07:02) as a respelling of the
 * Judge-FAILed dup4_0xc_into_arms store-schedule cheat, and state.json lists it in
 * banned_constructs. It is banked at
 * rejected/rederive_polyg4_struct_perarm_score0_banned_family.c and must NOT be re-submitted in
 * any spelling. src/text1b.c carries `INCLUDE_ASM("asm/funcs", func_80072CD4);` per
 * asm-until-matched.
 *
 * Pure C: one local `int fc_const`. No asm/pins/volatile/barrier/do-while/dead store/duplication.
 *
 * THE RESIDUAL 4 (established by s5-forensics at cc1-dump level, re-confirmed s5-structural):
 * the merge block store order. Ours: cross-jumped `sb v0,0xE` at merge position 0, the li/sb
 * chains next, `sb v1,4 / sb v1,0xC` DEFERRED to the block tail by sched2. Target: `sb v1,4 /
 * sb v1,0xC / sb v0,0xE` at the merge head, with `addiu v1,$zero,0xFC` in the inner beqz delay
 * slot (which this body already reproduces exactly).
 *
 * TWO CODEGEN LAWS BOUND EVERY NON-PER-ARM FORM (evidence.md "L1"/"L2"):
 *  L1 - a merge-block store whose value register is defined in a PREDECESSOR block has zero
 *       in-block dependence predecessors, is ready in sched2 first bottom-up round, and is
 *       therefore emitted LAST. Target obeys L1 itself (`sb zero,0x16` sits at its merge tail).
 *  L2 - pass order is sched2 -> jump_optimize(cross_jump=1) -> dbr, so a jump2 common tail is
 *       spliced at the new join label AHEAD of everything sched2 emitted, and is never
 *       re-scheduled. Nothing a merge-block source statement can do reaches position 0.
 *
 * s5-structural closed the last untried structural combination (alias serialisation composed
 * with the cross-block chassis) - see hypotheses.md [s5-structural]. All three measurements are
 * WORSE than 4 and each fails for a cost reason, not a scheduling reason: an alias base pointer
 * always materialises its own register copy (+1 insn target does not have), and removing the
 * per-arm `@0xE` store to free the merge head re-triggers sched1 hoist of the cross-block value
 * `li`, which makes the arm tails identical and cross-jumps `sb v0,0xD` out of them (-1 insn).
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
