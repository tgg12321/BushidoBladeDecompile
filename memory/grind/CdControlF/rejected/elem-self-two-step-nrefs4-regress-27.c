/* REJECTED (session 2). Reusing `elem` for its own base (dropping the separate
 * `base` local) raises elem's reg_n_refs from 2 to 4, which is far too much:
 * floor_log2(4)*4 = 8 vs floor_log2(2)*2 = 2, so elem's allocno priority jumps
 * 645 -> 2352 and it takes s0 outright. Score 27 (floor is 9).
 *
 * Measured ALLOCDBG (tmp/grind/CdControlF/s2/variants_c.json, C3):
 *   ord=0 pseudo=79(elem) hardreg=16 nrefs=4 livelen=34 pri=2352
 *
 * The lesson for the frontier: elem needs EXACTLY 3 references (not 4), and at
 * a live length near 38, to land in the (769, 810) priority window. A fourth
 * reference overshoots by a factor of ~3.
 */
s32 CdControlF(u8 a0, s32 a1) {
    s32 result;
    s32 idx;
    s32 saved;
    s32 count;
    s32 *elem;

    result = 0;
    idx = a0;
    saved = g_cd_callback_a;
    count = 3;
    elem = g_cd_sector_buf;
    elem = elem + idx;

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
