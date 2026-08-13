/* CdControlF — session 1 best pure-C form. Honest sandbox distance 11
 * (baseline 23, and 23 was WITH the register asm("s6") pin that this form
 * deletes). Structure is byte-for-byte the same 75 instructions as target;
 * the whole residual is register assignment. Apply verbatim to src/system.c.
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
