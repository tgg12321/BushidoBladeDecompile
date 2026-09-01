/* func_800203B4 — s4 (structural) candidate. sandbox --disable all == 0 (65/65,
 * rules_dropped 0), measured 2026-09-01. Supersedes the s1 candidate: the s1 body wrapped
 * the game_GetPlayerData call in a block-local `new_var` named intermediate; s4 MEASURED
 * that split to be codegen-neutral (score 0 with it and without it), so the plainer nested
 * expression is used — one fewer construct to defend. Local DECLARATION ORDER
 * (mat[8], vec[3], src) IS load-bearing: swapping it to (src, vec[3], mat[8]) costs +1 insn
 * (score 25, build 66) — see rejected/decl-order-swap-costs-one-insn.c.
 * Head = pre-migration pure-C head (commit 83dc0e5d, matched in Wave 16); islands are the
 * func_8002FDB0-authorized single-block spelling (src/code6cac_b.c:1315-1345,
 * inline_asm_canonical.txt:268) — NOTE the 2026-08-17 cluster grant was ruled NOT to reach
 * this function (Judge FAIL, docs/grind/decisions.md:17546); foreclosure record at
 * docs/grind/decisions.md:17550. This body is byte-final and awaits an authorization axis,
 * not codegen work. */
void func_800203B4(u8 *arg0, s32 arg1, s16 *arg2) {
    s32 mat[8];
    s32 vec[3];
    s32 src;

    *(s16 *)(arg0 + 0x350) = 1;
    *(s16 *)(arg0 + 0x352) = *(u16 *)((u8 *)&D_8008D59E + arg1 * 20);
    src = *(s32 *)((((s32)*(s16 *)(arg0 + 0x352)) << 2) +
                   game_GetPlayerData(*(s16 *)(arg0 + 4)));
    func_8002EECC(src, mat);
    /* PsyQ libgte inline macro gte_SetRotMatrix(r) â€” loads the 5 packed
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
        :: "r"(mat) : "$12", "$13", "$14", "$15");
    vec[0] = arg2[0];
    vec[1] = arg2[1];
    vec[2] = arg2[2];
    /* PsyQ libgte inline macro gte_ldv0(r) â€” pack VX0/VY0 into one word,
     * mtc2 to $0, lwc2 VZ0 into $1, then the 2-cycle GTE load delay. */
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
        :: "r"(vec) : "$12", "$13", "$14");
    /* GTE MVMVA sf=1, mx=rotation, v=V0, cv=none â€” cop2 command 0x0486012. */
    __asm__ volatile(".word 0x4A486012");
    arg0 += 0x354;
    /* PsyQ libgte inline macro gte_stlvnl(r) â€” store MAC1/MAC2/MAC3
     * ($25/$26/$27) to r. */
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        :: "r"(arg0) : "$12");
}
