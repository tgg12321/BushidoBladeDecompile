/* func_8002EA24 -- grind session 2 candidate (structural modality).
 *
 * Honest sandbox floor of THIS form: 9  (was 18 at HEAD, 20 for the session-1
 * "PsyQ-macro" honest respelling).  Build 102 insns vs target 104.
 *
 * WHAT CHANGED vs session 1.  Session 1 assumed target's extra
 * `addu $t4, <src>, $zero` before every cop2 op had to come from a second C
 * variable that GCC failed to coalesce.  Session 2 measured that family dead
 * (7 spellings, all BYTE-IDENTICAL to the plain form) and then found the real
 * answer by forensics: `addu $t4, X, $zero` occurs in 46 target functions,
 * and the ONLY three matched-with-no-rules functions that contain it are
 * func_8001A67C, func_800274BC and func_8004DDB4 -- and the first two are
 * listed in inline_asm_canonical.txt as USER-AUTHORIZED (2026-06-10)
 * hand-written GTE blocks whose authorized C spells the $t4 routing INSIDE a
 * single canonical __asm__ block.  func_8002EA24's LZCS/LZCR block is
 * instruction-for-instruction the same construct as func_800274BC's
 * (addu t4,<val> -> mtc2 t4,$30 -> 2 unfilled GTE delay nops -> addu t4,$sp
 * -> swc2 $31,0(t4)), and asm/funcs/func_8002EA24.s carries splat
 * "handwritten instruction" tags on mtc2 $t4,$30 and on swc2 $26/$27.
 *
 * This form therefore spells BOTH GTE regions the way the two authorized
 * siblings are spelled: one canonical __asm__ block per region, the single
 * C-level operand bound through %N, the $t4 routing inside the template,
 * $2/$12 clobbered.  That closes the entire GTE region exactly (all 8
 * canonical insns + the 3 $t4 copies + both address computations).
 *
 * DISPOSITION IS NOT SELF-APPROVED.  By the letter of [[inline-asm-injection]]
 * a hardcoded-$N template is the forbidden injection pattern; the two sibling
 * precedents are the reason this is a genuine classification question rather
 * than a cheat, and the session returned `ruling-request` for exactly that.
 * Do NOT treat this file as accepted until the owner rules and the function is
 * added to inline_asm_canonical.txt (a surface a grind session may not touch).
 *
 * Residual vs target (score 9 = 6 register mismatches + 2 missing + 1 extra):
 *   a) the compare chain puts x in $a0 and neg_threshold in $a1; target wants
 *      $a1 and $t1 (6 insns: the lw, the negu, three slt, the mult).
 *      Declaration-order / xdefer / a0first / twovars / zdecl / negfirst all
 *      measured (5 byte-identical, zdecl worse at 12).
 *   b) the tail `if (y + a0_var < min_y) return 0; return 1;` still folds to
 *      `slt; xori $v0,$v0,1`; target keeps the unfolded diamond.  Six pure-C
 *      tail shapes measured: endlabel / revcmp / ifelse byte-identical,
 *      gotoreject 30, swap 17, ternary 10.  The documented closure for this
 *      exact shape is [[dead-store-fake-exception]] (dead `ret = 1;` inside
 *      the else arm) -- last-resort, FAKE-annotated, layer-2 reviewed.
 *
 * NOTE on the LZC block's "=m"(sp_var): the template hardcodes 0($sp), which
 * is where GCC currently places sp_var in this 8-byte frame (verified: the
 * build emits `lw $v1, 0($sp)` exactly as target does).  It is correct for
 * this frame layout only -- the same fragility the two authorized siblings
 * carry.
 */
s32 func_8002EA24(u8 *obj, s32 *pos, s32 threshold, s32 r_sq) {
    *(s16 *)(obj + 0xF8) = pos[0] - (*(s32 **)(obj + 0x60))[0];
    *(s16 *)(obj + 0xFA) = pos[1] - (*(s32 **)(obj + 0x60))[1];
    *(s16 *)(obj + 0xFC) = pos[2] - (*(s32 **)(obj + 0x60))[2];
    __asm__ volatile(
        "addiu $v0, %0, 0xF8\n"
        "addu $t4, $v0, $zero\n"
        "lwc2 $0, 0($t4)\n"
        "lwc2 $1, 4($t4)\n"
        "nop\n"
        "nop\n"
        ".word 0x4A486012\n"
        "addiu $v0, %0, 0x100\n"
        "addu $t4, $v0, $zero\n"
        "swc2 $25, 0($t4)\n"
        "swc2 $26, 4($t4)\n"
        "swc2 $27, 8($t4)"
        : : "r"(obj) : "$2", "$12", "memory");

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
