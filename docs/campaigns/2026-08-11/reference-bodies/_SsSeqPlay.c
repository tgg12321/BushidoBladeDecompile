/* ***  MATCH-READY  ***  src/main.c - _SsSeqPlay (LIBSND/MIDIREAD)
 * Reference: sotn-decomp src/main/psxsdk/libsnd/seqread.c:28-51 (_SsSeqPlay).
 *
 * MEASURED: sandbox --disable all == 0   (baseline 28, with 15 regfix rules).
 * PROVEN:   `engine retire _SsSeqPlay` dropped all 15 regfix.txt rules and the full
 *           clean build SHA1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa (the oracle).
 *           (Verified in isolation, with no other src edit in the tree.)
 *
 * This is a COMPLETED-C candidate: zero rules, zero cheat-asm, byte-identical.
 * It was NOT committed - this was a measurement campaign. To land it: apply this
 * body, run `retire _SsSeqPlay`, then the layer-2 cheat-reviewer, then `queue done`.
 *
 * Adaptation from the reference: SOTN's `struct SeqStruct* score = &_ss_score[a0][a1]`
 * becomes BB2's byte-offset base pointer (D_80106F28 table, 0xB0 stride); SOTN's
 * field names map as delta_value -> +0x90, unk70 -> +0x54, unk6E -> +0x52; and
 * _SsGetSeqData() is BB2's func_80084CC0().
 *
 * The lever the reference supplied was the CONTROL FLOW, not the arithmetic: BB2's
 * previous body spelled the tail as a goto-loop with a shared `store_result` label,
 * which is what the 15 regfix rules were compensating for. SOTN's nested
 * do/while -- `do { do { GetSeqData(); } while (delta == 0); acc += delta; }
 * while (acc < unk70);` -- plus the if/else-if/else ladder in the first arm is the
 * shape GCC 2.7.2 actually emits for the target.
 */

void _SsSeqPlay(s16 a0, s16 a1) {
    u8 *base;
    s32 var_s0;

    base = (u8 *)(*(s32 *)((u8 *)&D_80106F28 + ((s32)(a0 << 16) >> 14)) + (s16)a1 * 0xB0);

    if (*(s32 *)(base + 0x90) - *(s16 *)(base + 0x54) > 0) {
        if (*(s16 *)(base + 0x52) > 0) {
            (*(s16 *)(base + 0x52))--;
        } else if (*(s16 *)(base + 0x52) == 0) {
            *(s16 *)(base + 0x52) = *(s16 *)(base + 0x54);
            (*(s32 *)(base + 0x90))--;
        } else {
            *(s32 *)(base + 0x90) -= *(s16 *)(base + 0x54);
        }
    } else if (*(s32 *)(base + 0x90) <= *(s16 *)(base + 0x54)) {
        var_s0 = *(s32 *)(base + 0x90);
        do {
            do {
                func_80084CC0(a0, a1);
            } while (*(s32 *)(base + 0x90) == 0);
            var_s0 += *(s32 *)(base + 0x90);
        } while (var_s0 < *(s16 *)(base + 0x54));
        *(s32 *)(base + 0x90) = var_s0 - *(s16 *)(base + 0x54);
    }
}
