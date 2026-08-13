/* REJECTED (session 2). Writing the retry loop as a REAL loop instead of the
 * goto chain regresses the honest floor from 9/11 to 18-22, in every spelling:
 *
 *   do { ... continue; ... } while (--count != -1);      -> 18
 *   do { ... goto next; ... next: count--; } while (...) -> 18
 *   for (count = 3; count != -1; count--) { ... }        -> 22
 *   while (1) { ... next: count--; if (...) break; }     -> 18
 *
 * (measured on the session-1 s32-param base; artifact
 * tmp/grind/CdControlF/s2/variants_a.json). The loop-note / loop-rotation
 * machinery reshapes the prologue and the back edge; target's shape is the
 * flat goto chain. Do not re-derive: the goto-chain loop body is fixed.
 */
s32 CdControlF(s32 a0, s32 a1) {
    s32 result;
    s32 idx;
    s32 saved;
    s32 count;
    s32 *base;
    s32 *elem;

    result = 0;
    idx = a0 & 0xFF;
    saved = g_cd_callback_a;
    count = 3;
    base = g_cd_sector_buf;
    elem = base + idx;

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
                    continue;
                }
            }
        }
        g_cd_callback_a = saved;
        if (CD_cw(a0 & 0xFF, a1, 0, 1) == 0) {
            goto done;
        }
    } while (--count != (-1));

    g_cd_callback_a = saved;
    result = -1;
done:
    return result + 1;
}
