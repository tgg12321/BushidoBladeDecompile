/* func_8007CA00 — CANDIDATE-READY (s4, permuter modality, 2026-07-27).
 * sandbox --disable all = 0 (build 44 == target 44) with this body in
 * src/display.c. Layer-1 cheat-reviewer: PASS (independent sandbox re-run,
 * do-while-zero-exception 2026-07-06 final ruling verified on-disk).
 *
 * Final structure (p9 shape + partial ret-funnel + one sanctioned wrap):
 * - Both branches own their `t = 0x400` li; label `sub` sits after case1's
 *   loads = target's exact block geometry.
 * - case1's li (multi-set t, no birthing promotion) floats to block top per
 *   M3 — which is exactly where target wants it: reorg eager fill steals it
 *   into the beqz delay slot past the two lh loads.
 * - case2's li is fenced by a single-level, FAKE-annotated do-while(0)
 *   (sanctioned per do-while-zero-exception, owner ruling 2026-07-06):
 *   the loop notes keep the isolated constant-set below the div chain
 *   (defeats the M3 backward-float), so the li is the last insn before the
 *   j and reorg own-thread fill drops it into the delay slot (M4 satisfied).
 * - `ret:` funnel keeps the ==0 division's last insn a pseudo-dest in its
 *   own BB (M2) — H1 stays byte-exact.
 * - Tail is the direct `t = t - v1; return t - a;` (no block-local c needed:
 *   with the p9 shape the combine-folded return no longer excludes t from
 *   $v0 — both tail forms measured 0; direct is the simpler spelling). */
extern u8 D_8009BE74;
extern u8 D_8009BE77;
s32 func_8007CA00(s16 *arg0) {
    s32 v1, a, t;
    switch (D_8009BE74) {
    case 1:
        if (D_8009BE77 != 0) {
            t = 0x400;
            v1 = arg0[2];
            a = arg0[0];
        sub:
            t = t - v1;
            return t - a;
        }
        t = arg0[0];
        goto ret;
    case 2:
        if (0 != D_8009BE77) {
            v1 = ((s16)(*((u16 *)(arg0 + 2)))) / 2;
            a = arg0[0];
            /* FAKE: wrap keeps the 0x400 load below the div chain so it
               fills the jump delay slot instead of hoisting to block top */
            do { t = 0x400; } while (0);
            goto sub;
        }
        t = ((s32)((s16)(*((u16 *)arg0)))) / 2;
        goto ret;
    default:
        t = arg0[0];
    ret:
        return t;
    }
}
