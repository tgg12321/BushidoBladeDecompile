/* func_800203B4 - s5 (structural) candidate. sandbox --disable all == 0 (65/65,
 * rules_dropped 0, cheat_asm_stripped 25), measured 2026-09-01.
 *
 * CHANGE vs the s4 candidate: the two explicit "nop" lines that padded the cop2 load
 * delay after `lwc2 $1, 8($12)` in the gte_ldv0 island are DELETED. Measured
 * codegen-neutral (0, 65/65 with and without them; source form banked at
 * tmp/grind/func_800203B4/s3/varG_no_explicit_nops.c) - the assembler stage supplies the
 * load-delay nops, so the island is now the LITERAL PsyQ SDK gte_ldv0 macro body with no
 * hand-written filler instructions. This shrinks the authorization surface: nothing in any
 * island is invented; every instruction is an SDK macro instruction.
 *
 * Other measured facts baked into this form (do not "tidy"):
 *  - Local DECLARATION ORDER (mat[8], vec[3], src) is load-bearing: (src, vec, mat) costs
 *    +1 insn with the islands (score 25, build 66) and raises the pure-C bound from 26 to
 *    30 without them - rejected/decl-order-swap-costs-one-insn.c and
 *    rejected/purec-declorder-swap-raises-bound-26-to-30.c.
 *  - STATEMENT ORDER is load-bearing: hoisting the three vec[] stores above the
 *    func_8002EECC call scores 17 (build 63) - rejected/vec-stores-hoisted-above-call.c.
 *  - `arg0 += 0x354;` may equivalently be re-associated into the stlvnl operand as
 *    "r"(arg0 + 0x354): measured 0, 65/65 (tmp/grind/func_800203B4/s3/varD_ptr_expr.c).
 *    Kept as the += form because it mirrors the SDK call shape; the choice is free.
 *  - The s1 block-local `new_var` named intermediate was measured codegen-neutral in s4 and
 *    dropped; no C-side sanctioned-family claim is needed for this body at all.
 *
 * Head = pre-migration pure-C head (commit 83dc0e5d, matched in Wave 16); islands are the
 * func_8002FDB0-authorized single-block spelling (src/code6cac_b.c:1315-1345,
 * inline_asm_canonical.txt:268) - NOTE the 2026-08-17 cluster grant was ruled NOT to reach
 * this function (Judge FAIL, docs/grind/decisions.md:17546); foreclosure record at
 * docs/grind/decisions.md:17550. This body is byte-final and awaits an authorization axis,
 * not codegen work.
 *
 * s4 (permuter modality, 2026-09-01) changed NOTHING in this body and confirmed it again at
 * sandbox --disable all == 0, 65/65 (artifact tmp/grind/func_800203B4/s4/code6cac_sandbox0_s4.o).
 * A ~35k-iteration permuter campaign over the islands-deleted pure-C chassis (base 3000 on
 * asm-differ) produced no valid improvement - best novel find 2960 and semantics-broken
 * (memory/grind/func_800203B4/rejected/permuter-best-find-2960-semantics-broken.c). The
 * permuter axis is spent; see evidence.md facts 28-29.
 *
 * s6 (synthesis, 2026-09-01) MEASURED the minimality of the asm surface: three variants that
 * move the C-expressible parts of the SDK macro bodies out of the islands into C all fail -
 * thin gte_SetRotMatrix (C-loaded matrix words) = 12, thin gte_stlvnl (no `move $12` preamble)
 * = 4, thin gte_ldv0 (VX0/VY0 packing in C) = 8; forms banked in rejected/thin-*.c, evidence.md
 * fact 41. The 25 instructions inside these four islands are byte-forced to be there; do not
 * try to "shrink the asm". Floor re-confirmed 0 (65/65) an eleventh time. */
void func_800203B4(u8 *arg0, s32 arg1, s16 *arg2) {
    s32 mat[8];
    s32 vec[3];
    s32 src;

    *(s16 *)(arg0 + 0x350) = 1;
    *(s16 *)(arg0 + 0x352) = *(u16 *)((u8 *)&D_8008D59E + arg1 * 20);
    src = *(s32 *)((((s32)*(s16 *)(arg0 + 0x352)) << 2) +
                   game_GetPlayerData(*(s16 *)(arg0 + 4)));
    func_8002EECC(src, mat);
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
        :: "r"(mat) : "$12", "$13", "$14", "$15");
    vec[0] = arg2[0];
    vec[1] = arg2[1];
    vec[2] = arg2[2];
    /* PsyQ libgte inline macro gte_ldv0(r) --- pack VX0/VY0 into one word,
     * mtc2 to $0, lwc2 VZ0 into $1, then the 2-cycle GTE load delay. */
    __asm__ volatile(
        "move   $12, %0\n"
        "lhu    $14, 4($12)\n"
        "lhu    $13, 0($12)\n"
        "sll    $14, $14, 16\n"
        "or     $13, $13, $14\n"
        "mtc2   $13, $0\n"
        "lwc2   $1, 8($12)\n"
        :: "r"(vec) : "$12", "$13", "$14");
    /* GTE MVMVA sf=1, mx=rotation, v=V0, cv=none --- cop2 command 0x0486012. */
    __asm__ volatile(".word 0x4A486012");
    arg0 += 0x354;
    /* PsyQ libgte inline macro gte_stlvnl(r) --- store MAC1/MAC2/MAC3
     * ($25/$26/$27) to r. */
    __asm__ volatile(
        "move   $12, %0\n"
        "swc2   $25, 0($12)\n"
        "swc2   $26, 4($12)\n"
        "swc2   $27, 8($12)\n"
        :: "r"(arg0) : "$12");
}
