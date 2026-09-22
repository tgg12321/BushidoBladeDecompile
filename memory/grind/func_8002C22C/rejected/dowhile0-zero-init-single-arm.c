/* REJECTED (s2, 2026-09-22) — wrapping ONE arm's 6-word scratchpad zero-init
 * in `do { ... } while (0);` (with a /* FAKE */ annotation per
 * do-while-zero-exception, hoping the loop-note boundary would break cse1's
 * address-pseudo sharing across the six stores) measured WORSE than the plain
 * per-arm duplication: sandbox --disable all = 200 (build_insns 246, same
 * count as without the wrap) vs 199 for the un-wrapped per-arm duplication.
 * Zero net benefit, one extra construct — reverted. Kept here only as a
 * negative data point; do not re-propose this exact wrap on this arm without
 * new evidence.
 *
 * (Only the changed region is reproduced; the rest of the function is
 * identical to memory/grind/func_8002C22C/candidate.c.)
 */
    if (D_800A3824 & 1) {
        do { /* FAKE: loop-note boundary defeats cse1's block-extension address merge, mechanism: cse.c cse_end_of_basic_block backward scan breaks on NOTE_INSN_LOOP_END, lever-exhaustion: plain-store spelling (s1, 211) and per-arm duplication (s2a, 199) both left the scratchpad address pseudo shared across all six stores in this arm */
        *(s32 *)0x1F800360 = 0;
        *(s32 *)0x1F800364 = 0;
        *(s32 *)0x1F800368 = 0;
        *(s32 *)0x1F800370 = 0;
        *(s32 *)0x1F800374 = 0;
        *(s32 *)0x1F800378 = 0;
        } while (0);
        v1 = *(s32 *)0x1F80004C;
        /* ... rest of arm unchanged ... */
