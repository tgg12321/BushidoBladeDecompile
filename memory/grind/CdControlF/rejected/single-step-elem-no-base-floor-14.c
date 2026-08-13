/* REJECTED (s1): the one-step `elem = &g_cd_sector_buf[idx];` spelling floors
 * at 14 across ALL 60 legal init orders and ALL 120 declaration orders. It
 * emits `sll v1,s2,2 / lui v0 / addiu v0 / addu elem,v1,v0`, i.e. the shift
 * and the symbol land in the OPPOSITE scratch registers from target. The
 * base two-step fixes v0/v1 and takes the floor to 11.
 * Measured: sweep_results.json (best 14), sweep2_results.json (all 14). */
s32 CdControlF(s32 a0, s32 a1) {
    s32 result;
    s32 idx;
    s32 saved;
    s32 count;
    s32 *elem;

    result = 0;
    idx = a0 & 0xFF;
    saved = g_cd_callback_a;
    count = 3;
    elem = &g_cd_sector_buf[idx];

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
