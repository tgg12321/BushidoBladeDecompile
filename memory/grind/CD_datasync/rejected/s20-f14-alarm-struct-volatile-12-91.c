//DROP:extern s32 D_800F19B8;
//DROP:extern s32 D_800F19BC;
//DROP:extern void *D_800F19C0;
typedef struct { s32 timeout; s32 count; void *arg; } CD_alarm;
extern volatile CD_alarm D_800F19B8;
s32 CD_datasync(s32 a0) {
    s32 v0;
    s32 cnt;
    s32 *tbl_11dc;
    u8 *idx_1494;
    s32 *tbl_125c;

    D_800F19B8.timeout = VSync(-1) + 0x3C0;
    tbl_11dc = D_800A11DC;
    idx_1494 = &D_800A1494;
    tbl_125c = D_800A125C;
    D_800F19B8.count = 0;
    D_800F19B8.arg = &D_800162C0;

loop:
    v0 = VSync(-1);
    if (D_800F19B8.timeout < v0) {
        goto do_timeout;
    }
    cnt = D_800F19B8.count;
    D_800F19B8.count = cnt + 1;
    if (!(0x3C0000 < cnt)) {
        goto success;
    }

do_timeout:
    /* FAKE: do{}while(0) — loop_depth weighting for the three table pointers
     * without giving loop.c a loop to hoist the compare constants out of.
     * mechanism: flow.c loop_depth ref-weighting -> global.c allocno_compare.
     * lever-exhaustion: memory/grind/CD_datasync/hypotheses.md (s9 H37/H38). */
    do {
        s32 arg4;
        puts(&g_str_cd_timeout);
        arg4 = tbl_125c[idx_1494[0]];
        printf(&D_800161C8, D_800F19B8.arg, tbl_11dc[D_800A11D5], arg4,
               tbl_125c[idx_1494[1]]);
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
