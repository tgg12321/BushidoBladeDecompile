/* REJECTED s1: no "memory" clobber on the SetRotMatrix island (v1). Measured sandbox 4 stripped
 * (build 79 vs target 81): GCC CSEs arg0[1] across the first island so the target reload
 * `lw $t1,4($a0); nop` at 0x8001934C is missing; local s32 out[6] also made the frame 32 (target 24). */
void func_80019310(s16 *arg0, s32 *arg1) {
    VECTOR out;
    s32 i;
    s32 *dst;
    s16 count;

    /* PsyQ libgte inline macro gte_SetRotMatrix(r) --- SDK macro body
     * hardcodes $12-$15 and copies the operand into $12. */
    __asm__ volatile(
        "move   $12, %0\n"
        "lw     $13, 0($12)\n"
        "lw     $14, 4($12)\n"
        "ctc2   $13, $0\n"
        "ctc2   $14, $1\n"
        "lw     $13, 8($12)\n"
        "lw     $14, 12($12)\n"
        "lw     $15, 16($12)\n"
        "ctc2   $13, $2\n"
        "ctc2   $14, $3\n"
        "ctc2   $15, $4\n"
        :: "r"(*(s32 *)(arg0 + 2)) : "$12", "$13", "$14", "$15");
    /* PsyQ libgte inline macro gte_SetTransMatrix(r) --- translation
     * vector words 20/24/28 into cop2 control regs $5..$7. */
    __asm__ volatile(
        "move   $12, %0\n"
        "lw     $13, 20($12)\n"
        "lw     $14, 24($12)\n"
        "ctc2   $13, $5\n"
        "lw     $15, 28($12)\n"
        "ctc2   $14, $6\n"
        "ctc2   $15, $7\n"
        :: "r"(*(s32 *)(arg0 + 2)) : "$12", "$13", "$14", "$15");

    count = ((s16 *)arg1)[2];
    dst = (s32 *)arg1[3];
    for (i = 0; i < count; i++) {
        /* PsyQ libgte inline macro gte_ldv0(r) --- lwc2 VXY0/VZ0 + 2-cycle
         * GTE load delay carried as explicit nops. */
        __asm__ volatile(
            "move   $12, %0\n"
            "lwc2   $0, 0($12)\n"
            "lwc2   $1, 4($12)\n"
            "nop\n"
            "nop\n"
            :: "r"((s32 *)(arg1[0] + i * 8)) : "$12");
        /* GTE MVMVA sf=1, mx=rotation, v=V0, cv=TR --- cop2 command 0x0480012. */
        __asm__ volatile(".word 0x4A480012");
        /* PsyQ libgte inline macro gte_stlvnl(r) --- store MAC1/MAC2/MAC3. */
        __asm__ volatile(
            "move   $12, %0\n"
            "swc2   $25, 0($12)\n"
            "swc2   $26, 4($12)\n"
            "swc2   $27, 8($12)\n"
            :: "r"(&out) : "$12");
        dst[0] = out.vx << 7;
        dst[1] = out.vy << 7;
        dst[2] = out.vz << 7;
        dst[3] = 0;
        dst[4] = 0;
        dst[5] = 0;
        dst = (s32 *)((u8 *)dst + 0x40);
        count = ((s16 *)arg1)[2];
    }
    *(MATRIX *)(arg1 + 5) = **(MATRIX **)(arg0 + 2);
}
