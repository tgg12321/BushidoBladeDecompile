typedef signed char s8;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
typedef int s32;
typedef unsigned int u32;

extern s32 ratan2(s32, s32);

s32 func_8002FC80(s32 *a0, s32 *a1, s32 *a2) {
    s32 v1, v2;
    s32 *p;
    s32 ret;

    /* Compute (a1 - a0) into scratchpad SCR[0x60..0x68] and (a2 - a0)
     * into SCR[0x70..0x78] - same layout as func_8002FDB0. */
    v1 = *a1;
    v2 = *a0;
    *(s32 *)0x1F800360 = v1 - v2;

    v1 = *(a1 + 1);
    v2 = *(a0 + 1);
    *(s32 *)0x1F800364 = v1 - v2;

    v1 = *(a1 + 2);
    v2 = *(a0 + 2);
    *(s32 *)0x1F800368 = v1 - v2;

    v1 = *a2;
    v2 = *a0;
    *(s32 *)0x1F800370 = v1 - v2;

    v1 = *(a2 + 1);
    v2 = *(a0 + 1);
    *(s32 *)0x1F800374 = v1 - v2;

    v1 = *(a2 + 2);
    v2 = *(a0 + 2);
    *(s32 *)0x1F800378 = v1 - v2;

    /* PsyQ libgte inline macro gte_SetRotMatrix(r) - loads the 3 packed
     * rotation-matrix words at r into cop2 control regs R11R12/R13R21/R22R23.
     * The SDK macro body hardcodes $12-$15 and copies the operand into $12. */
    __asm__ volatile(
        "move   $12, %0\n"
        "lw     $13, 0($12)\n"
        "lw     $14, 4($12)\n"
        "ctc2   $13, $0\n"
        "lw     $15, 8($12)\n"
        "ctc2   $14, $2\n"
        "ctc2   $15, $4\n"
        :: "r"((s32 *)0x1F800360) : "$12", "$13", "$14", "$15");
    /* PsyQ libgte inline macro gte_ldlvl(r) - load long vector at r into
     * IR1/IR2/IR3 ($9/$10/$11), IR3 first, then the 2-cycle GTE load delay. */
    __asm__ volatile(
        "move   $12, %0\n"
        "lwc2   $11, 8($12)\n"
        "lwc2   $9, 0($12)\n"
        "lwc2   $10, 4($12)\n"
        "nop\n"
        "nop\n"
        :: "r"((s32 *)0x1F800370) : "$12");
    /* GTE OP (outer/cross product of the IR vector with the rotation matrix
     * diagonal), sf=0 - cop2 command 0x0170000C. */
    __asm__ volatile(".word 0x4B70000C");
    /* PsyQ libgte inline macro gte_stlvnl(r) - store MAC1/MAC2/MAC3
     * ($25/$26/$27) to r. */
    p = (s32 *)0x1F800380;
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        :: "r"(p) : "$12");
    /* Angle of the cross product in the XZ-ish plane, +0x800 (180 deg)
     * when MAC2 is positive. */
    ret = ratan2(p[0], p[2]);
    if (p[1] > 0) {
        ret += 0x800;
    }
    return ret;
}
