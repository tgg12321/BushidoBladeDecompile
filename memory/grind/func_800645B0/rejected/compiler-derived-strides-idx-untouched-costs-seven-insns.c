/* REJECTED -- s18 (structural).  The "pure array-subscript" shape: `idx` is never
 * overwritten and BOTH strides are left to the compiler --
 * `(idx * 12)` for the three word stores and `(idx * 2)` for the halfword
 * store, with the `idx2` local deleted entirely.  **44 / 78 at 85 build insns**
 * (+7).  cse.c does NOT unify the `idx << 1` that synth_mult emits inside the
 * *12 expansion with the separate *2 expansion, so the stride arithmetic is
 * materialised twice.  Sibling spellings: n2 (`idx * 12` with `idx2` kept for
 * the halfword store) = 36 / 78 at 82 insns; n3 (`(idx * 3) << 2`) = 14 / 78 at
 * 80 insns.  The target's three-insn chain (sll/addu/sll) is only reachable
 * when the source itself stages the shared `idx << 1`.
 */
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx;
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
                last = rand();
                *((s32 *)(((s32)(&D_800F0D78)) + (idx * 12))) = (((s32 *)D_800A347C)[0] + (last & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + (idx * 12))) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + (idx * 12))) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
                last = rand();
                val = D_800A3444;
                *((s16 *)(((s32)(&D_800F0BCC)) + (idx * 2))) = last & 7;
                val = val | mask;
                D_800A3444 = val;
                break;
            }
        }
    }
    return 1;
}