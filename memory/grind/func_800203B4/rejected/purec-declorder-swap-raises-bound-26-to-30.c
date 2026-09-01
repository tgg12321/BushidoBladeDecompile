/* REJECTED (s5, 2026-09-01): the pure-C form (all four inline-asm islands deleted) with the
 * locals reordered to (src, vec[3], mat[8]) scores 30 with build_insns 39 - i.e. FOUR of the
 * 39 C-emitted instructions now mismatch, where the canonical order (mat, vec, src) has all
 * 39 matching (score 26 == the exact 65-39 deficit). Proves the pure-C bound of 26 is a
 * MINIMUM that the canonical declaration order already attains, and that structural
 * perturbation only moves it UP. Do not reorder the locals. */
void func_800203B4(u8 *arg0, s32 arg1, s16 *arg2) {
    s32 src;
    s32 vec[3];
    s32 mat[8];

    *(s16 *)(arg0 + 0x350) = 1;
    *(s16 *)(arg0 + 0x352) = *(u16 *)((u8 *)&D_8008D59E + arg1 * 20);
    {
        s32 new_var;
        new_var = game_GetPlayerData(*(s16 *)(arg0 + 4));
        src = *(s32 *)((((s32)*(s16 *)(arg0 + 0x352)) << 2) + new_var);
    }
    func_8002EECC(src, mat);
    /* PsyQ libgte inline macro gte_SetRotMatrix(r) --- loads the 5 packed
     * rotation-matrix words at r into cop2 control regs $0..$4.  The SDK
     * macro body hardcodes $12-$15 and copies the operand into $12. */
    vec[0] = arg2[0];
    vec[1] = arg2[1];
    vec[2] = arg2[2];
    /* PsyQ libgte inline macro gte_ldv0(r) --- pack VX0/VY0 into one word,
     * mtc2 to $0, lwc2 VZ0 into $1, then the 2-cycle GTE load delay. */
    /* GTE MVMVA sf=1, mx=rotation, v=V0, cv=none --- cop2 command 0x0486012. */
    arg0 += 0x354;
    /* PsyQ libgte inline macro gte_stlvnl(r) --- store MAC1/MAC2/MAC3
     * ($25/$26/$27) to r. */
}
