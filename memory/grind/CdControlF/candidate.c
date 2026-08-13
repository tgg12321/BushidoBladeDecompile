/* CdControlF - session 3 CLOSING form. Honest sandbox distance 0
 * (`sandbox CdControlF --disable all` -> score 0, target_insns 75,
 * build_insns 75, rules_dropped 0). Floor history 23 (s1 start) -> 11 (s1)
 * -> 9 (s2) -> 0 (s3).
 *
 * Three things close it, on top of the s1/s2 form:
 *   1. The `u8 a0` parameter (Sony libcd `u_char com`) from session 2, with the
 *      forward `extern s32 CdControlF(u8, s32);` at src/system.c:1014 kept in
 *      sync. Measured codegen-neutral for all five callers.
 *   2. The `do { ... } while (0);` region around the retry-loop body. This is
 *      the sanctioned do-while(0) match device (.claude/rules/
 *      do-while-zero-exception.md), FAKE-annotated at the construct site.
 *      MECHANISM (measured, not guessed): flow.c's life analysis accumulates
 *      `reg_n_refs[regno] += loop_depth` (flow.c:2081/2329/2515/2725) and
 *      loop_depth comes from NOTE_INSN_LOOP_BEG/END, so every reference inside
 *      a loop-note region counts TWICE while reg_live_length is untouched.
 *      Sizing the region to the whole loop body multiplies exactly the right
 *      rows of global.c:635 allocno_compare and lands the entire seven-allocno
 *      table on target's order in one step:
 *        count 7/35=4000 s0 | a1 7/37=3783 s1 | idx 4/34=2352 s2 |
 *        a0 4/37=2162 s3 | saved 4/37=2162 s4 | elem 3/31=967 s5 |
 *        result 3/39=769 s6      (= target's s0..s6 exactly)
 *      Region sizing is load-bearing: body-only (decrement outside) scores 16,
 *      label-inside-region scores 16, whole-loop-body scores 4.
 *   3. Init-statement order `idx, saved, count, base, elem, result` — i.e.
 *      `result = 0;` LAST. With the allocation now pinned by the region, the
 *      remaining 4 was purely the prologue schedule: target emits
 *      `sw s6 / move s6,zero` at prologue positions 17-18, ours emitted them at
 *      5-6 because `result = 0;` was the first init statement. Session 2's
 *      "result must be initialised first-or-early" was true only on the OLD
 *      allocation basis, where init order WAS the allocation lever.
 *
 * Apply verbatim to src/system.c and keep the :1014 extern at (u8, s32).
 */
s32 CdControlF(u8 a0, s32 a1) {
    s32 result;
    s32 idx;
    s32 saved;
    s32 count;
    s32 *base;
    s32 *elem;

    idx = a0;
    saved = g_cd_callback_a;
    count = 3;
    base = g_cd_sector_buf;
    elem = base + idx;
    result = 0;

loop:
    /* FAKE: loop-note ref weighting seats elem in s5 and result in s6,
       mechanism: flow.c life analysis (reg_n_refs += loop_depth) feeding
       global.c allocno_compare, lever-exhaustion: memory/grind/CdControlF/
       hypotheses.md (s1 60+120 perms, s2 240 init orders + batches A-E,
       s3 batches A/B) */
    do {
    g_cd_callback_a = 0;

    if (idx != 1) {
        if (g_cd_mode & 0x10) {
            CD_cw(1, 0, 0, 0);
        }
    }
    if (a1 != 0) {
        if ((*elem) != 0) {
            if (CD_cw(2, a1, 0, 0) != 0) {
                goto next;
            }
        }
    }
    g_cd_callback_a = saved;
    if (CD_cw(a0, a1, 0, 1) == 0) {
        goto done;
    }
next:
    count--;

    if (count != (-1)) {
        goto loop;
    }
    } while (0);
    g_cd_callback_a = saved;
    result = -1;
done:
    return result + 1;
}
