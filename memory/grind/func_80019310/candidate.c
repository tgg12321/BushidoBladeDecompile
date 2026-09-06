/* func_80019310 - GTE rotate-and-scale of an SVECTOR array into a 0x40-stride VECTOR
 * table, then a 32-byte MATRIX copy into the descriptor. Pure-C body plus four PsyQ SDK
 * GTE macro islands - gte_SetRotMatrix, gte_SetTransMatrix, gte_ldv0 (lwc2 form),
 * cop2 MVMVA sf=1/mx=rot/v=V0/cv=TR (.word 0x4A480012), gte_stlvnl - in the same
 * `move $12, %0` macro-body spelling as func_800203B4 (owner grant 2026-09-01, widened
 * cop2 materialize-then-copy anchor; func_80019310 is named in that grant record,
 * docs/grind/decisions.md 2026-09-01 entry). Every island carries the macro's own
 * clobber list including "memory"; the SetRotMatrix island's clobber is what makes GCC
 * re-read the MATRIX pointer before the SetTransMatrix island (target 0x8001934C), and
 * the gte_stlvnl island writes out[] which the C below reads. Honest bucket is
 * COMPLETED-INLINE-ASM-CANONICAL (allowlist line required). Full ledger:
 * memory/grind/func_80019310/ (s1: sandbox --disable all == 0, 81/81). */
void func_80019310(s16 *arg0, s32 *arg1) {
    s32 out[3];
    s32 i;
    s32 *dst;

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
        :: "r"(*(s32 *)(arg0 + 2)) : "$12", "$13", "$14", "$15", "memory");
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
        :: "r"(*(s32 *)(arg0 + 2)) : "$12", "$13", "$14", "$15", "memory");

    dst = (s32 *)arg1[3];
    for (i = 0; i < ((s16 *)arg1)[2]; i++) {
        /* PsyQ libgte inline macro gte_ldv0(r) --- lwc2 VXY0/VZ0 + 2-cycle
         * GTE load delay carried as explicit nops. */
        __asm__ volatile(
            "move   $12, %0\n"
            "lwc2   $0, 0($12)\n"
            "lwc2   $1, 4($12)\n"
            "nop\n"
            "nop\n"
            :: "r"((s32 *)(arg1[0] + i * 8)) : "$12", "memory");
        /* GTE MVMVA sf=1, mx=rotation, v=V0, cv=TR --- cop2 command 0x0480012. */
        __asm__ volatile(".word 0x4A480012");
        /* PsyQ libgte inline macro gte_stlvnl(r) --- store MAC1/MAC2/MAC3. */
        __asm__ volatile(
            "move   $12, %0\n"
            "swc2   $25, 0($12)\n"
            "swc2   $26, 4($12)\n"
            "swc2   $27, 8($12)\n"
            :: "r"(out) : "$12", "memory");
        dst[0] = out[0] << 7;
        dst[1] = out[1] << 7;
        dst[2] = out[2] << 7;
        dst[3] = 0;
        dst[4] = 0;
        dst[5] = 0;
        dst = (s32 *)((u8 *)dst + 0x40);
    }
    *(MATRIX *)(arg1 + 5) = **(MATRIX **)(arg0 + 2);
}
