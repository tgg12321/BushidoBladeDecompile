/* s45: frontier item 1c - initialise a table base pointer INSIDE the do_timeout block rather than at
 * function scope, the one hoist position s44's set A never tested. All 14 cells regress: on m011
 * 20/24/24/28/30/24/35, on p_3245 22/30/32/29/31/38/31, and n collapses 82 -> 69-80 because the base is
 * re-materialised per use instead of living in a callee-saved seat. A block-scope pointer does not become
 * call-clobbered scratch; it stops being a register value at all. */
s32 CD_datasync(s32 a0) {
    s32 v0;
    s32 cnt;
    s32 *tbl_11dc;
    u8 *idx_1494;

    D_800F19B8 = VSync(-1) + 0x3C0;
    tbl_11dc = D_800A11DC;
    idx_1494 = &D_800A1494;
    D_800F19BC = 0;
    D_800F19C0 = &D_800162C0;

loop:
    v0 = VSync(-1);
    if (D_800F19B8 < v0) {
        goto do_timeout;
    }
    cnt = D_800F19BC;
    D_800F19BC = cnt + 1;
    if (!(0x3C0000 < cnt)) {
        goto success;
    }

do_timeout:
    /* FAKE: do{}while(0) - loop_depth weighting for the three table pointers
     * without giving loop.c a loop to hoist the compare constants out of.
     * mechanism: flow.c loop_depth ref-weighting -> global.c allocno_compare.
     * lever-exhaustion: memory/grind/CD_datasync/hypotheses.md (s9 H37/H38). */
    do {
        s32 *tbl_125c;
        s32 arg4;
        puts(&g_str_cd_timeout);
        tbl_125c = D_800A125C;
        arg4 = tbl_125c[idx_1494[0]];
        printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5], arg4, tbl_125c[idx_1494[1]]);
        CD_flush();
    } while (0);
    v0 = -1;
    goto check;

success:
    v0 = 0;

check:
    if (v0 != 0) {
        return -1;
    }
    if (*D_800A14C0 & 0x1000000) {
        if (a0 == 0) {
            goto loop;
        }
        return 1;
    }
    return 0;
}
