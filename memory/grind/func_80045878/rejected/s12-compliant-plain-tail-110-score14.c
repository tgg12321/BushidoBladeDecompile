/* DEAD (s12, structural).  score 14 at 110 insns.  RE-MEASURED on today's
 * chassis (s10 first measured this shape); kept because s12 disassembled it
 * and the disassembly is the direct confirmation of the allocation mechanism.
 *
 * SHAPE: candidate.c with the fresh carrier `c` DELETED entirely -- the third
 * if's condition is the ordinary `s1[3] != -2`, and the two tail values are
 * written inline (`v0[11] = a0 + 3;` / `*(s32 *)((s32)v0 + 0x18) = 0x8000;`).
 * This is the fully Judge-compliant "what a human would write" tail, so its
 * score IS the compliant floor for the plain-tail chassis.
 *
 * MEASURED TAIL (objdump of tmp/sandbox/func_80045878/text1a_c.o):
 *     move  a0,s1          <- base copy seated $a0, not $v0
 *     addiu v0,s2,3        <- scratch 1 seated $v0
 *     sh    v0,22(a0)
 *     move  v0,s2          <- EXTRA insn (+1): a0's value re-materialised
 *     sh    v0,4(a0) ; sh v0,20(a0) ; sh v0,16(a0)
 *     li    v0,0x8000      <- scratch 2 also seated $v0
 *     sh    s5,8(a0)
 *     sw    v0,24(a0)
 * vs target:
 *     addu  v0,s1,zero ; addiu v1,s2,3 ; sh v1,0x16(v0) ; ori v1,zero,0x8000
 *     sh s2,4(v0) ; sh s5,8(v0) ; sh s2,0x14(v0) ; sh s2,0x10(v0) ; sw v1,0x18(v0)
 *
 * WHY KILLED / WHAT IT PROVES: both tail scratches are block-local, so
 * local_alloc (which runs before global_alloc, and scans hard regs in
 * ascending regno order because mips.h defines no REG_ALLOC_ORDER) hands
 * BOTH of them $v0; the base copy is a multi-block allocno and therefore only
 * reaches global_alloc, by which time $v0 is conflicted for the base's entire
 * range and it is pushed to $a0.  This is the exact configuration the carrier
 * `c` exists to prevent, observed end-to-end in the emitted code.
 */
void func_80045878(s32 a0, s32 a1, s32 a2) {
    s32 s3;
    s16 *v0;
    s16 *s1;
    s32 s0;
    v0 = (s16 *) func_8004574C(a0);
    if (v0 != 0) {
        s1 = (s16 *) ((s32 *) v0)[1];
    } else {
        s1 = (s16 *) func_800455AC(a0);
        func_80045600(a0, 0x1A88 + ((s32) s1));
        func_80045230(0);
        func_80045694(a0, (s32) (&func_80045AA4));
        s1[4] = -1;
        s1[3] = 0;
    }
    s3 = a0 + 3;
    if (func_8004574C(s3) != 0) {
        func_800400F8((s32) s1);
    }
    if (((func_8004574C(s3) != 0) && (s1[4] == a1)) && (s1[3] != (-2))) {
        s1[3] = 0;
    } else {
        *((s32 *) (((s32) s1) + 0x20)) = a2;
        s0 = (s32) func_800455AC(s3);
        *((s32 *) (((s32) s1) + 0x1C)) = s0;
        if (a2 != 0) {
            func_80044ED8(a1, a2);
        } else {
            func_80044ED8(a1, s0);
            s0 = s0 + ((((u32) ((s32 *) s0)[*((s32 *) s0)]) >> 2) << 2);
            func_80045230(s0);
        }
        func_80045600(s3, s0);
        func_80045694(s3, (s32) (&func_80045AA4));
        s1[3] = 1;
        *((s32 *) (((s32) s1) + 0x24)) = 0;
        *((s32 *) s1) = 0;
    }
    v0 = s1;
    v0[11] = a0 + 3;
    v0[2] = a0;
    v0[4] = a1;
    v0[10] = a0;
    v0[8] = a0;
    *((s32 *) (((s32) v0) + 0x18)) = 0x8000;
}
