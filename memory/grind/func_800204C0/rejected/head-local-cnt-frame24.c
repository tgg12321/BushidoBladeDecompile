/* REJECTED s1 (2026-09-08): head spelled `cnt = *p + 1; *p = cnt; if ((cnt & 7) == 2)` with an
 * s32 cnt local. Measured sandbox --disable all = 10 (121/122): frame vars=24 vs target 32 and
 * the `move $v1,$v0` beqz delay-slot copy is missing. The `*p += 1` + fresh re-read spelling
 * (candidate.c) closes both to 0; islands identical in both forms. */
void func_800204C0(u8 *arg0) {
    s32 mac[3];
    s16 out[3];
    s32 pid;
    s32 cnt;
    s32 src;
    s32 mul;
    s32 tx, ty, tz;

    pid = *(s16 *)(arg0 + 4);
    if (*(s16 *)(arg0 + 0x350) != 0) {
        cnt = *(s16 *)(arg0 + 0x350) + 1;
        *(s16 *)(arg0 + 0x350) = cnt;
        if ((cnt & 7) == 2) {
            src = *(s32 *)((((s32)*(s16 *)(arg0 + 0x352)) << 2) +
                           game_GetPlayerData(pid));
            /* PsyQ libgte inline macro gte_SetRotMatrix(r) --- loads the 5 packed
             * rotation-matrix words at r into cop2 control regs $0..$4.  The SDK
             * macro body hardcodes $12-$15 and copies the operand into $12. */
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
                :: "r"(src) : "$12", "$13", "$14", "$15");
            /* PsyQ libgte inline macro gte_ldv0(r) --- pack VX0/VY0 into one word,
             * mtc2 to $0, lwc2 VZ0 into $1, then the 2-cycle GTE load delay carried
             * as explicit nops (maspsx does NOT supply them in the full-build
             * context â€” measured on func_800203B4, 2026-09-01). */
            __asm__ volatile(
                "move   $12, %0\n"
                "lhu    $14, 4($12)\n"
                "lhu    $13, 0($12)\n"
                "sll    $14, $14, 16\n"
                "or     $13, $13, $14\n"
                "mtc2   $13, $0\n"
                "lwc2   $1, 8($12)\n"
                "nop\n"
                "nop\n"
                :: "r"(arg0 + 0x354) : "$12", "$13", "$14");
            /* GTE MVMVA sf=1, mx=rotation, v=V0, cv=none --- cop2 command 0x0486012. */
            __asm__ volatile(".word 0x4A486012");
            mul = ((0x96 - *(s16 *)(arg0 + 0x350)) << 12) / 150;
            /* PsyQ libgte inline macro gte_stlvnl(r) --- store MAC1/MAC2/MAC3
             * ($25/$26/$27) to r. */
            __asm__ volatile(
                "move   $12, %0\n"
                "swc2   $25, 0($12)\n"
                "swc2   $26, 4($12)\n"
                "swc2   $27, 8($12)\n"
                :: "r"(mac) : "$12");
            tx = (mac[0] * mul) / 0x1000;
            out[0] = tx;
            ty = (mac[1] * mul) / 0x1000;
            out[1] = ty;
            tz = (mac[2] * mul) / 0x1000;
            out[2] = tz;
            if ((s16)ty >= 0x801) {
                out[0] = -tx;
                out[1] = -ty;
                out[2] = -tz;
            }
            func_80032854(pid, 4, (u8 *)0x1F8000A8 + pid * 0x108 + *(s16 *)(arg0 + 0x352) * 0xC, out);
        }
        if (*(s16 *)(arg0 + 0x350) >= 0x96) {
            *(s16 *)(arg0 + 0x350) = 0;
        }
        *(s16 *)(arg0 + 0x350) = 0;
    }
}
