/* func_8002D320 -- grind session 1 (recon, 2026-08-31) -- DISTANCE 0.
 *
 * sandbox func_8002D320 --disable all  ->  {"score": 0, "target_insns": 120,
 * "build_insns": 120, "rules_dropped": 0}  measured THIS session with this
 * exact body applied over the INCLUDE_ASM line at src/code6cac_b.c:860.
 * Tail diamond verified by disassembly (bnez; move v0,zero delay; li v0,1).
 *
 * DERIVATION (one session, riding the twin func_8002EA24's 17-session ledger):
 *   - Body = the retired-chassis body (retired-chassis-2026-08/body.c) minus
 *     ALL cheats (1 memory barrier, 5 register pins, 3 hardcoded-$N templates,
 *     placeholder moves, stripped .word swc2 spellings), with:
 *   - the brief's LEVER 1: `else if (min_y < y_low) max_y = y_low;` (target's
 *     slt $v0,$a2,$v1 -- NOT a compare against 0);
 *   - the brief's LEVER 2: straight early-return tail (no result carrier);
 *   - both GTE islands in the Judge-endorsed NARROW form (2026-07-30 twin
 *     ruling): addresses computed in C, bound "r"(ptr), template = addu $t4
 *     preamble + cop2 ops only, "$12" clobber; vector island as TWO blocks
 *     (twin [s3]: the two-statement split is required); LZCS island verbatim
 *     from the authorized sibling func_800274BC (src/code6cac_b.c:292);
 *   - ONE variable chain x -> dist_sq -> disc -> sqrt (target's $a0 does
 *     exactly that);
 *   - ONE FAKE construct: the tail 0/1 diamond staged-z closure (twin's L1,
 *     [[staged-value-reused-variable]]) -- see annotation in the body.
 *     Without it the build folds the final pair to slt+xori (score 3, 118
 *     insns; measured v1). Honest tail shapes measured dead this session:
 *     result-carrier nest (4/119), goto-reject (3/118, byte-identical to v1),
 *     inverted sense (3/118).
 *
 * The unlike-the-twin register geometry is WHY this fell in one session: the
 * extra leading `flag` arg shifts obj->$a1/pos->$a2, so every register seat
 * target uses (x/dist/disc/sqrt $a0, neg+min_y $a2, max_y $a3, y $a1) is a
 * naturally-freed argument register -- no allocator fight exists.
 *
 * END-STATE NOTE: contains canonical cop2 islands => the function completes as
 * COMPLETED-INLINE-ASM-CANONICAL via the pipeline grant path (judge-sole-gate
 * rule 3; cluster membership .claude/rules/cop2-addressing-preamble-cluster.md:73;
 * the func_8002FDB0 grant inline_asm_canonical.txt:268 extends to this sibling
 * subject to the mechanical check -- all four conditions hold here: sandbox 0,
 * zero pins/barriers/aliasing, in-island GPR = addressing preamble only,
 * layer-2 + verify-oracle --rebuild still owed at integration).
 */
s32 func_8002D320(s32 flag, u8 *obj, s32 *pos, s32 threshold, s32 r_sq) {
    if (flag == 0) {
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
    }
    {
        s32 x;
        s32 z;
        s32 sp_var;
        s32 min_y;
        s32 max_y;
        s32 y_low;
        s32 y_high;
        s32 y;
        s32 neg_threshold = -threshold;

        x = *(s32 *)(obj + 0x100);
        if (x < neg_threshold || threshold < x) return 0;
        z = *(s32 *)(obj + 0x104);
        if (z < neg_threshold || threshold < z) return 0;

        x = x * x + z * z;
        if (r_sq < x) return 0;
        x = r_sq - x;

        if ((u32)x < 0x400) {
            x = (u32)*(((u8 *)&D_8008D118) + x) >> 3;
        } else {
            s32 lzcr = 0;
            if (x >= 0) {
                __asm__ volatile(
                    "addu $t4, %1, $zero\n"
                    "mtc2 $t4, $30\n"
                    "nop\n"
                    "nop\n"
                    "addu $t4, $sp, $zero\n"
                    "swc2 $31, 0($t4)"
                    : "=m"(sp_var) : "r"(x) : "$12");
                lzcr = sp_var;
            }
            {
                s32 shift = 0x16 - (lzcr & ~1);
                s32 tbl = *(((u8 *)&D_8008D118) + ((u32)x >> shift));
                x = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
            }
        }

        max_y = 0;
        min_y = 0;
        y_low = *(s32 *)(obj + 0xB0);
        if (y_low < 0) {
            min_y = y_low;
        } else if (min_y < y_low) {
            max_y = y_low;
        }
        y_high = *(s32 *)(obj + 0xC0);
        if (y_high < min_y) {
            min_y = y_high;
        } else if (max_y < y_high) {
            max_y = y_high;
        }
        y = *(s32 *)(obj + 0x108);
        if (max_y < y - x) return 0;
        /* FAKE: stages the returned 0 through z -- a real value, read by the
         * return on the next statement -- whose own value (the rotated Z) was
         * last read at the `z * z` above and is dead here.  Mechanism: jump.c's
         * store-flag if-conversion requires a SINGLE-SET arm; the two-statement
         * arm keeps target's unfolded 0/1 diamond (bnez; move v0,zero delay;
         * addiu v0,1) instead of folding it to `slt` + `xori v0,v0,1`.
         * Family: [[staged-value-reused-variable]].  Lever-exhaustion:
         * memory/grind/func_8002D320/hypotheses.md session 1 (four pure-C tail
         * shapes measured: plain early-return, result-carrier nest, goto-reject,
         * inverted sense) + the twin func_8002EA24's session-2 six-shape tail
         * census on the identical diamond. */
        if (y + x < min_y) { z = 0; return z; }
        return 1;
    }
}
