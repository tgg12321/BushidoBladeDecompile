/* func_8002D320 -- CLOSING FORM -- DISTANCE 0 (re-proven 2026-08-31,
 * post-reset session 1; first proven 2026-09-01 pre-reset).
 *
 * sandbox func_8002D320 --disable all -> {"score": 0, "target_insns": 120,
 * "build_insns": 120, "rules_dropped": 0, "cheat_asm_stripped": 46} with this
 * exact body applied over the INCLUDE_ASM line at src/code6cac_b.c:860
 * (artifact tmp/grind/func_8002D320/s1/sandbox_0.json).
 *
 * PROVENANCE: this is the Judge-passed form from the 2026-08-31 ESCALATE
 * packet (docs/grind/decisions.md:16948), reconstructed and re-measured at 0.
 * It is the session-1 candidate with ONE change: the tail construct.
 *   - The session-1 tail (a hardcoded constant routed through the dead
 *     borrowed local z into the exit value) was layer-1 FAILED and is on
 *     this function's ban list under both its spelling and its misfiled
 *     family citation. It is banked at rejected/layer1-fail-0831-2106.c
 *     and MUST NOT come back in any spelling.
 *   - The passing tail is the func_80078EC0 confirmed-closure shape under
 *     dead-store-fake-exception: a dedicated result variable `ret` set in
 *     BOTH arms, with a dead `ret = 1;` inside the zero arm (two-set arm
 *     breaks jump.c store-flag if-conversion's single-set precondition,
 *     keeping target's unfolded diamond: bnez; move v0,zero delay; addiu
 *     v0,zero,1). FAKE-annotated in the body per the Judge's constraint
 *     (decisions.md:16969: "the FAKE-annotated dead store must remain
 *     annotated verbatim").
 *
 * Tail exhaustion (why the FAKE store is last-resort): five pure-C shapes
 * measured — plain early-return 3/118, result-carrier nest 4/119 (banked),
 * goto-reject 3/118 byte-identical, inverted sense 3/118 byte-identical,
 * combined-&& 8/119 (banked) — plus the twin func_8002EA24's six-shape
 * census on the identical diamond.
 *
 * END STATE: COMPLETED-INLINE-ASM-CANONICAL. The OWNER-CLUSTER canonical-asm
 * grant is EXECUTED (inline_asm_canonical.txt:366, pipeline 2026-08-31; door:
 * tools/grinder/owner_cluster_grants.txt + cop2-addressing-preamble-cluster.md:73).
 * The grant covers ONLY the three cop2 islands; no GPR asm outside them.
 * Islands in the Judge-endorsed narrow form (2026-07-30 twin ruling):
 * addresses computed in C, bound "r"; template = addu $t4 preamble + cop2
 * ops only; "$12" clobber; vector island as TWO blocks (twin [s3]); LZCS
 * island verbatim from authorized sibling func_800274BC.
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
        s32 ret;
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
        if (y + x < min_y) {
            ret = 1; /* FAKE: dead store -- overwritten by `ret = 0;` on the
                      * next statement, never read.  Mechanism: jump.c's
                      * store-flag if-conversion requires SINGLE-SET 0/1 arms;
                      * the two-set arm keeps target's unfolded diamond (bnez;
                      * move v0,zero delay; addiu v0,1) instead of folding the
                      * pair to `slt` + `xori v0,v0,1`.  Family:
                      * dead-store-fake-exception (confirmed closure
                      * func_80078EC0, .claude/rules/dead-store-fake-exception.md:107-128).
                      * Lever-exhaustion: memory/grind/func_8002D320/hypotheses.md
                      * sessions 1-2 (five pure-C tail shapes measured: plain
                      * early-return 3/118, result-carrier nest 4/119,
                      * goto-reject 3/118, inverted sense 3/118, combined-&&
                      * 8/119) + the twin func_8002EA24's six-shape tail census
                      * on the identical diamond. */
            ret = 0;
        } else {
            ret = 1;
        }
        return ret;
    }
}
