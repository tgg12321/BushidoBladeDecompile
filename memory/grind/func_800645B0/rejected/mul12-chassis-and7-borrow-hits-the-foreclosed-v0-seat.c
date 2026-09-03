/* REJECTED -- s18 (structural).  a2 (`idx = idx * 12;`) plus a second REAL write to `idx`
 * (`idx = last & 7;` feeding the halfword store), the mul-chassis analogue of
 * the h form.  **2 / 78** -- the second set defeats the birthing lift and the
 * loop head is recovered, but the residual is the SAME TWO REGISTERS s16 typed
 * FORECLOSED at the RA stage: 55 `andi $s0,$v0,7` and 58 `sh $s0` where the
 * target keeps the value in $v0.  Borrowing the OR result instead
 * (b2_mul12_or_idx: `idx = val | mask; D_800A3444 = idx;`) costs the same two
 * at 59/60 ($s0 vs $v1), and doing BOTH costs four (b4 = 4/78).  Confirms on a
 * third chassis that ANY borrow of `idx` costs exactly two register names:
 * `idx` crosses `jal rand` so it must be callee-saved, and every value the
 * target computes after that call lives in a caller-saved seat.
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
    for (i = 0; i < 0xF; i += 4) {
        for (j = 0; j < 4; j++) {
            idx = i + j;
            val = 1;
            mask = val << idx;
            if (!(D_800A3444 & mask)) {
                idx2 = idx << 1;
                last = rand();
                idx = idx * 12;
                *((s32 *)(((s32)(&D_800F0D78)) + idx)) = (((s32 *)D_800A347C)[0] + (last & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + idx)) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + idx)) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
                last = rand();
                val = D_800A3444;
                idx = last & 7;
                *((s16 *)(((s32)(&D_800F0BCC)) + idx2)) = idx;
                val = val | mask;
                D_800A3444 = val;
                break;
            }
        }
    }
    return 1;
}