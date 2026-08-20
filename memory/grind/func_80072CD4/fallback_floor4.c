/* func_80072CD4 - CLEAN candidate (reviewer-passable): `sandbox --disable all` = 4,
 * build_insns 79 == target. Re-measured chassis-current s5-forensics (2026-08-20).
 *
 * This is the SAME body previously banked as fallback_floor4.c. It is restored as candidate.c
 * because the sandbox-0 body that briefly occupied this file (the per-arm rgb0/rgb1 triple form)
 * was FAILed by the layer-1 cheat-reviewer as a respelling of the Judge-FAILed @4/@0xC
 * duplicated-into-arms store-schedule construct, and is now a BANNED construct for this function.
 * Do not re-apply it as a candidate; the open question about it is a ruling question, not a
 * submission (see the s5-forensics ruling-request in the ledger).
 *
 * Pure C: one local `int fc_const`. No asm/pins/volatile/barrier/do-while/dead store/duplication.
 *
 * WHAT s5-FORENSICS ESTABLISHED ABOUT THE RESIDUAL 4 (dump-level, not hypothesis):
 *  - The residual is the merge block store order. Ours: `sb v0,0xE` (cross-jumped from the arms)
 *    at the merge head, the li/sb chains next, and `sb v1,4 / sb v1,0xC` DEFERRED to the block tail.
 *    Target: `sb v1,4 / sb v1,0xC / sb v0,0xE` at the merge head.
 *  - Pass attribution (BB2_SCHED_DEBUG on the instrumented cc1, tools/gcc-2.7.2/cc1;
 *    tmp/grind/func_80072CD4/s5/scheddbg_pass{1,2}.txt): the deferral is done by **sched2**
 *    (schedule_insns pass 2), not by jump2 and not by sched1. sched.c schedule_block walks the
 *    block BOTTOM-UP (pick order is the exact reverse of emit order - verified against the PICK
 *    trace). A store whose value register is defined in a PREDECESSOR block has ZERO dependence
 *    predecessors inside the block, so it is ready at the first bottom-up round and wins the
 *    equal-priority (pri=1) schedule_select potential-hazard tiebreak (sched.c:2660-2745, unit=0
 *    memory insns beat unit=-1 ones); being picked first bottom-up puts it LAST in the emitted
 *    block. The li->sb pairs are pulled the other way by adjust_priority birth boost
 *    (pri 0x7F000001) the moment their consumer is scheduled.
 *  - Internal control inside THIS function own build: `sb v0,0xE`, which the source writes
 *    per-arm and jump2 cross-jumps, DOES sit at the merge head; `sb v1,4`/`sb v1,0xC`, which the
 *    source writes in the merge block, sink to the tail. Same block, same pass, opposite placement,
 *    and the only difference is which block the source wrote them in.
 *  - Pass ORDER (toplev.c:3117 sched2 -> :3142 jump_optimize(cross_jump=1) -> :3167 dbr):
 *    cross-jump runs AFTER sched2, so a common tail spliced at the join label is never
 *    re-scheduled. That is why the cross-jumped store keeps the head slot the sunk stores
 *    cannot reach.
 *  => Consequence: no source form that writes @4/@0xC inside the merge block can place them at the
 *    merge head, for any statement order (s4b 15.8k-iteration directed PERM_LINESWAP over exactly
 *    those stores found nothing below base - source order is irrelevant, sched2 rebuilds the order
 *    from a dependence graph that every permutation shares).
 *
 * The cross-block chassis (rejected/xblock_sched1_hoist.c, 13/78) fails for the mirror-image reason:
 * sched1 hoists the arms `li var_v0,0x32/0x46` to the arm TOP because it has unit=-1 and no in-block
 * consumer, so it loses every equal-priority tiebreak and is picked last bottom-up
 * (scheddbg_pass1.txt block=2/3: SELBEST picks 54/49/44 over 57 at clock 2/4/6, insn 57 picked at
 * clock 8 = block top). The hoist then makes the arm tails identical, cross-jumps `sb v0,0xD` out of
 * the arms (78 insns, one short of target) and rotates RA (fc_const -> $a0, var_v0 -> $v1).
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
