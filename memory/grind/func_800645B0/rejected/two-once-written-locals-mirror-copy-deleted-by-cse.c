/* REJECTED (session 15, 2026-09-01, forensics) — "P2", the owner-directed
 * Ruling-A named probe, mirror direction: `idx = i + j;` kept at the loop top,
 * and inside the if-arm `wid = idx;` followed by `wid2 = idx2 + wid;`, the
 * three word stores reading `wid2`.  Two distinct fresh locals, each written
 * exactly once; distinct from the banned `val = idx; idx = idx2 + val;`
 * (which re-wrote `idx`) and from the banned `wid` multi-write carrier.
 *
 * MEASURED: `sandbox func_800645B0 --disable all` = 3 / 78, build_insns 78 —
 * identical to P1, and the diff lands on the identical three positions
 * (11, 12, 65: the inner-loop head and the back-edge delay slot).
 *
 * PASS ATTRIBUTION: same cse.c deletion.
 * tmp/grind/func_800645B0/s15/p2.rtl.txt has
 * `(insn 60 57 63 (set (reg/v:SI 79) (reg/v:SI 74)))`; p2.cse.txt has zero
 * occurrences of insn 60 and zero references to pseudo 79, and insn 63 reads
 * `(plus:SI (reg/v:SI 75) (reg/v:SI 74))` directly.
 */
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx;
    s32 idx2;
    s32 mask;
    s32 val;
    s32 last;
    s32 wid;
    s32 wid2;
    D_800F10EC = 1;
    for (i = 0; i < 0xF; i += 4) {
        for (j = 0; j < 4; j++) {
            idx = i + j;
            val = 1;
            mask = val << idx;
            if (!(D_800A3444 & mask)) {
                idx2 = idx << 1;
                last = rand();
                wid = idx;
                wid2 = idx2 + wid;
                *((s32 *)(((s32)(&D_800F0D78)) + (wid2 << 2))) = (((s32 *)D_800A347C)[0] + (last & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + (wid2 << 2))) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + (wid2 << 2))) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
                last = rand();
                val = D_800A3444;
                *((s16 *)(((s32)(&D_800F0BCC)) + idx2)) = last & 7;
                val = val | mask;
                D_800A3444 = val;
                break;
            }
        }
    }
    return 1;
}
