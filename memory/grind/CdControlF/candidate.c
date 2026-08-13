/* CdControlF - session 2 best pure-C form. Honest sandbox distance 9
 * (session 1 ended at 11; HEAD before session 1 was 23 WITH a register
 * asm("s6") pin that is now gone). Structure is byte-for-byte the same 75
 * instructions as target; the whole residual is one register assignment.
 *
 * Two changes vs the session-1 candidate:
 *   1. `a0` is typed `u8` (the Sony libcd `u_char com` shape). This is what
 *      dropped the floor 11 -> 9: the explicit `a0 & 0xFF` masks the s32 form
 *      needed become the parameter's own type, and GCC re-materialises them
 *      itself at exactly the two sites target has them.
 *   2. The forward `extern s32 CdControlF(s32, s32);` at src/system.c:1012 was
 *      updated to `(u8, s32)` to match. Measured codegen-neutral: all five
 *      callers (D_80082050, D_80082320, func_8008241C, CdReadBreak, CdRead)
 *      stay at sandbox distance 0.
 *
 * The remaining 9 is entirely `result` and `elem` sitting in each other's
 * callee-saved register (ours s5=result/s6=elem, target s5=elem/s6=result).
 * See memory/grind/CdControlF/hypotheses.md for the exact allocno-priority
 * arithmetic that has to be moved and by how much.
 *
 * Apply verbatim to src/system.c (and keep the :1012 extern in sync).
 */
s32 CdControlF(u8 a0, s32 a1) {
    s32 result;
    s32 idx;
    s32 saved;
    s32 count;
    s32 *base;
    s32 *elem;

    result = 0;
    idx = a0;
    saved = g_cd_callback_a;
    count = 3;
    base = g_cd_sector_buf;
    elem = base + idx;

loop:
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
    g_cd_callback_a = saved;
    result = -1;
done:
    return result + 1;
}
