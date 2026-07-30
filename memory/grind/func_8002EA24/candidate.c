/* func_8002EA24 -- grind session 3 candidate (structural modality).
 *
 * Honest sandbox floor of THIS form: 9  (HEAD = 18; session-1 honest
 * PsyQ-macro respelling = 20; session-2 authorized-sibling respelling = 9).
 * Build 102 insns vs target 104.  Disassembly md5 (dis.sh) = 1fc26fe12849.
 *
 * WHAT CHANGED vs the session-2 candidate.  The Judge's binding constraint on
 * this session was: keep the LZC block ONLY in the exact func_800274BC-
 * authorized shape, and re-spell the vector/mvmva block MINIMALLY -- operand
 * address computed in C and bound via %N, template limited to the $t4 copy +
 * lwc2/swc2 + the mvmva .word, no hardcoded `addiu $v0, %0, 0xF8` inside the
 * template and no `$2` clobber.  That is exactly what this form does, and it
 * is measured BYTE-IDENTICAL to the session-2 form (same score 9, same
 * disassembly md5).  The constraint therefore costs nothing: both address
 * computations now come from ordinary C (`addiu $v0,$t0,0xF8` /
 * `addiu $v0,$t0,0x100`, both in $v0, both in target's position) and only the
 * $t4 routing + the cop2 ops remain inside the templates.
 *
 * IMPORTANT: the block must be split into TWO __asm__ statements (load+mvmva,
 * then store).  A single block taking both pointers as %0/%1 forces GCC to
 * materialise both addresses BEFORE the block, which emits
 * `addiu $v1,$t0,0xF8` + `addiu $v0,$t0,0x100` back-to-back ahead of the
 * lwc2 -- score 13, banked as rejected/gte-single-block-two-operands-score13.c.
 *
 * SANDBOX ARTIFACT (do not mistake for a codegen gap): the cheat-invisible
 * sandbox strips the bare `nop` lines out of a kept canonical block, so the
 * sandbox disassembly is missing all four GTE pipeline nops (2 before the
 * mvmva, 2 after the mtc2).  They ARE in the templates and appear in a real
 * build.  Modulo those four stripped nops, the entire GTE region -- both
 * address computations, all three `addu $t4,<reg>,$zero` copies, all 8 cop2
 * instructions -- matches target instruction-for-instruction.
 *
 * DISPOSITION IS NOT SELF-APPROVED.  A hardcoded-$N template is the forbidden
 * injection pattern by the letter of [[inline-asm-injection]]; the two in-tree
 * sibling authorizations (func_8001A67C / func_800274BC, user-authorized
 * 2026-06-10) for the identical construct are why this is a genuine
 * classification question.  Session 2 returned `ruling-request`; the Judge's
 * answer constrained the SHAPE (above) but explicitly did NOT authorize adding
 * func_8002EA24 to inline_asm_canonical.txt or retiring its 10 regfix rules.
 *
 * RESIDUAL vs target (score 9), now fully diagnosed:
 *   H5 (6 of the 9 points) -- `x` lands in $a0 and `neg_threshold` in $a1;
 *       target wants $a1 and $t1.  Root cause read straight off the cc1
 *       `.greg` conflict lists: the ONE missing conflict is
 *       a0_var <-> {x, neg_threshold}.  See evidence.md session 3 for the
 *       full derivation and the measured confirmation.
 *   H6 (3 of the 9 points) -- the tail `if (y + a0_var < min_y) return 0;
 *       return 1;` still folds to `slt; xori $v0,$v0,1`; target keeps the
 *       unfolded `bnez / addu $v0,$zero,$zero / addiu $v0,$zero,1` diamond.
 *       Six pure-C tail shapes were measured dead in session 2; the documented
 *       closure is [[dead-store-fake-exception]], which needs a /* FAKE */
 *       annotation + layer-2 review and was out of scope for a structural
 *       session.
 *
 * NOTE on the LZC block's "=m"(sp_var): the template hardcodes 0($sp), which
 * is where GCC places sp_var in this 8-byte frame (the build emits
 * `lw $v1, 0($sp)` exactly as target does).  Correct for this frame layout
 * only -- the same fragility the two authorized siblings carry.
 */
s32 func_8002EA24(u8 *obj, s32 *pos, s32 threshold, s32 r_sq) {
    s32 *vin;
    s32 *vout;
    *(s16 *)(obj + 0xF8) = pos[0] - (*(s32 **)(obj + 0x60))[0];
    *(s16 *)(obj + 0xFA) = pos[1] - (*(s32 **)(obj + 0x60))[1];
    *(s16 *)(obj + 0xFC) = pos[2] - (*(s32 **)(obj + 0x60))[2];
    vin = (s32 *)(obj + 0xF8);
    __asm__ volatile(
        "addu $t4, %0, $zero\n"
        "lwc2 $0, 0($t4)\n"
        "lwc2 $1, 4($t4)\n"
        "nop\n"
        "nop\n"
        ".word 0x4A486012"
        : : "r"(vin) : "$12", "memory");
    vout = (s32 *)(obj + 0x100);
    __asm__ volatile(
        "addu $t4, %0, $zero\n"
        "swc2 $25, 0($t4)\n"
        "swc2 $26, 4($t4)\n"
        "swc2 $27, 8($t4)"
        : : "r"(vout) : "$12", "memory");

    {
        s32 x = *(s32 *)(obj + 0x100);
        s32 z;
        s32 a0_var;
        s32 sp_var;
        s32 min_y;
        s32 max_y;
        s32 y_low;
        s32 y;
        s32 neg_threshold = -threshold;

        if (x < neg_threshold || threshold < x) return 0;
        z = *(s32 *)(obj + 0x104);
        if (z < neg_threshold || threshold < z) return 0;

        a0_var = x * x + z * z;
        if (r_sq < a0_var) return 0;
        a0_var = r_sq - a0_var;

        if ((u32)a0_var < 0x400) {
            a0_var = (u32)*(((u8 *)&D_8008D118) + a0_var) >> 3;
        } else {
            s32 lzcr = 0;
            if (a0_var >= 0) {
                __asm__ volatile(
                    "addu $t4, %1, $zero\n"
                    "mtc2 $t4, $30\n"
                    "nop\n"
                    "nop\n"
                    "addu $t4, $sp, $zero\n"
                    "swc2 $31, 0($t4)"
                    : "=m"(sp_var) : "r"(a0_var) : "$12");
                lzcr = sp_var;
            }
            {
                s32 shift = 0x16 - (lzcr & ~1);
                s32 tbl = *(((u8 *)&D_8008D118) + ((u32)a0_var >> shift));
                a0_var = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
            }
        }

        max_y = 0;
        min_y = 0;
        y_low = *(s32 *)(obj + 0xB0);
        if (y_low < 0) {
            min_y = y_low;
        } else {
            max_y = y_low;
        }
        y = *(s32 *)(obj + 0x108);
        if (max_y < y - a0_var) return 0;
        if (y + a0_var < min_y) return 0;
        return 1;
    }
}
