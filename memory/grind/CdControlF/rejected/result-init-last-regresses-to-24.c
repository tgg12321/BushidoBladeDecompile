/* REJECTED (s1): initialising `result` LAST (after elem) scores 24 vs 11 for
 * result-first. Init-statement ORDER is the live lever; result must be the
 * first (or near-first) initialised local. Measured: sweep_results.json,
 * sweep3_results.json variant E_result_after_elem. */
s32 CdControlF(s32 a0, s32 a1) {
    s32 idx;
    s32 saved;
    s32 count;
    s32 *base;
    s32 *elem;
    s32 result;

    idx = a0 & 0xFF;
    saved = g_cd_callback_a;
    count = 3;
    base = g_cd_sector_buf;
    elem = base + idx;
    result = 0;

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
    if (CD_cw(a0 & 0xFF, a1, 0, 1) == 0) {
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
