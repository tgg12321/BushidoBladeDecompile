/* REJECTED (session 2, 2026-08-12) — declaration order is NOT the lever for
 * the `addu $s0,$s1,$s0` operand order.
 *
 * Hypothesis H7: GCC 2.7.2 assigns local pseudos at expand_decl time, so
 * pseudo numbers follow declaration order (verified in the t.i.combine dump:
 * i=72, j=73, idx=74, idx2=75, mask=76, val=77). If cse.c's commutative
 * canonicalisation ordered PLUS operands by pseudo number, declaring `idx2`
 * ahead of `idx` would emit `addu $s0,$s1,$s0` as the target does.
 *
 * MEASURED: score 3, build_insns 78 — unchanged, still `addu $s0,$s0,$s1`.
 * Three spellings all failed identically:
 *   Q_declswap       idx2 declared immediately before idx
 *   R_declswap_mul3  same, with the sum written as `idx * 3`
 *   S_idx2_first     idx2 declared first of all locals
 * Sweep: tmp/grind/func_800645B0/s1/sweep4.py
 *
 * The SOTN-sanctioned "named-intermediate declaration order" family is
 * therefore SPENT on this residual — do not re-propose it.
 */
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx2;   /* <-- declared ahead of idx; no effect */
    s32 idx;
    s32 mask;
    s32 val;
    s32 last;
    D_800F10EC = 1;
    i = 0;
    do {
        j = 0;
        do {
            idx = i + j;
            val = 1;
            mask = val << idx;
            j += 1;
            if (!(D_800A3444 & mask)) {
                idx2 = idx << 1;
                idx = idx2 + idx;
                *((s32 *)(((s32)(&D_800F0D78)) + (idx << 2))) = (((s32 *)D_800A347C)[0] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + (idx << 2))) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + (idx << 2))) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
                last = rand();
                val = D_800A3444;
                *((s16 *)(((s32)(&D_800F0BCC)) + idx2)) = last & 7;
                val = val | mask;
                D_800A3444 = val;
                break;
            }
        } while (j < 4);
        i += 4;
    } while (i < 0xF);
    return 1;
}
