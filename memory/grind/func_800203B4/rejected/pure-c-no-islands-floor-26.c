/* REJECTED (s4 structural, measured 2026-09-01): the candidate body with ALL four inline-asm
 * islands deleted - i.e. the maximal PURE-C form of this function. sandbox --disable all
 * = 26 (build_insns 39 vs target 65, rules_dropped 0). The score equals the exact
 * instruction-count DEFICIT (65-39=26), i.e. every one of the 39 C-emitted instructions
 * matches the target; the entire residual is instructions no C construct can emit
 * (5x ctc2, mtc2, lwc2, 3x swc2, the MVMVA .word, plus their addressing preamble and the
 * two unfilled cop2 load-delay nops). GCC 2.7.2 emits cop2 ONLY from inline asm, so no
 * structural lever (decl order, block splits, type narrowing, re-association) can move 26.
 * This is the number the structural modality is bounded by. */
void func_800203B4(u8 *arg0, s32 arg1, s16 *arg2) {
    s32 mat[8];
    s32 vec[3];
    s32 src;

    *(s16 *)(arg0 + 0x350) = 1;
    *(s16 *)(arg0 + 0x352) = *(u16 *)((u8 *)&D_8008D59E + arg1 * 20);
    {
        s32 new_var;
        new_var = game_GetPlayerData(*(s16 *)(arg0 + 4));
        src = *(s32 *)((((s32)*(s16 *)(arg0 + 0x352)) << 2) + new_var);
    }
    func_8002EECC(src, mat);
    /* PsyQ libgte inline macro gte_SetRotMatrix(r)  loads the 5 packed
     * rotation-matrix words at r into cop2 control regs $0..$4.  The SDK
     * macro body hardcodes $12-$15 and copies the operand into $12. */
    vec[0] = arg2[0];
    vec[1] = arg2[1];
    vec[2] = arg2[2];
    /* PsyQ libgte inline macro gte_ldv0(r)  pack VX0/VY0 into one word,
     * mtc2 to $0, lwc2 VZ0 into $1, then the 2-cycle GTE load delay. */
    /* GTE MVMVA sf=1, mx=rotation, v=V0, cv=none  cop2 command 0x0486012. */
    arg0 += 0x354;
    /* PsyQ libgte inline macro gte_stlvnl(r)  store MAC1/MAC2/MAC3
     * ($25/$26/$27) to r. */
}
