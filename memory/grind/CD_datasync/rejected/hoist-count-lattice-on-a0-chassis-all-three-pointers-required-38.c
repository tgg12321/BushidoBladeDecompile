/* s45: hoist-count re-audit on the $a0 (p_3245) chassis, all 8 subsets of {tbl_11dc, idx_1494, tbl_125c}.
 * Scores: none 38, 1494 29, 125c 31, 11dc 31, 11dc+1494 32, 11dc+125c 30, 1494+125c 22, all three 12.
 * The s42 m011 hoist-count kill TRANSFERS: all three pointers hoisted is uniquely optimal on the
 * $a0 chassis too, so no hoist-count cell is 'dead on m011 but live on p_3245'. This is the
 * zero-hoist extreme (38). */
s32 CD_datasync(s32 a0) {
    s32 v0;
    s32 cnt;


    D_800F19B8 = VSync(-1) + 0x3C0;

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
        s32 arg3;
        void *arg2;
        s32 arg4;
        s32 arg5;
        puts(&g_str_cd_timeout);
        arg3 = D_800A11DC[D_800A11D5];
        arg2 = D_800F19C0;
        arg4 = D_800A125C[(&D_800A1494)[0]];
        arg5 = D_800A125C[(&D_800A1494)[1]];
        printf(&D_800161C8, arg2, arg3, arg4, arg5);
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
