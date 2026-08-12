/* REJECTED (session 2, 2026-08-12) — writing the byte offset as `idx * 12`
 * costs an extra instruction (79 vs the 78-insn target).
 *
 * Hypothesis H8: the three s32 stores are a 12-byte-strided triple, so the
 * natural original spelling might be a `* 12` byte offset (or `* 3` word
 * index), and expmed.c's synth_mult expands `x * 3` as
 * (plus (ashift x 1) x) -- shift result FIRST -- which is exactly the target's
 * `addu $s0,$s1,$s0` operand order.
 *
 * MEASURED (tmp/grind/func_800645B0/s1/sweep3.py):
 *   L_mul3_mul2    idx2 = idx * 2;  idx = idx * 3;    score 3, 78 insns
 *   N_mul3_shift2  idx2 = idx << 1; idx = idx * 3;    score 3, 78 insns
 *   M_mul12_mul2   idx2 = idx * 2;  idx = idx * 12;   score 12, 79 insns
 *   O_mul12_shift2 idx2 = idx << 1; idx = idx * 12;   score 12, 79 insns
 *   P_mul12_only   idx2 = idx;      idx = idx * 12;   score 21, 81 insns
 *
 * Conclusions: `* 3` is byte-identical to the explicit `idx2 + idx` (so
 * synth_mult does NOT preserve its operand order past cse.c's commutative
 * canonicalisation), and `* 12` regresses because the `idx * 2` value stops
 * being shared with the D_800F0BCC s16 store, forcing a second shift.
 */
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx;
    s32 idx2;
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
                idx2 = idx * 2;
                idx = idx * 12;   /* <-- 79 insns: idx*2 no longer shared */
                *((s32 *)(((s32)(&D_800F0D78)) + idx)) = (((s32 *)D_800A347C)[0] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + idx)) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + idx)) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
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
