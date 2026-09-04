/* ALTERNATE FORM (equal floor, different family): the timeout block DUPLICATED into both condition arms instead of a shared label, with NO do-while(0) wrapper. Emits asm BYTE-IDENTICAL to the wrapper chassis (only .L label numbering differs) and measures 7 / 91, rules_dropped 0. Proves the pointer ref-lift that the wrapper buys via flow.c loop_depth is equally reachable via literal duplication + jump.c cross-jump re-merge (sanctioned family: duplicated-statement-into-arms). Does NOT lower the floor and still needs a FAKE annotation under that family's rule, so it is a lateral, not a win.
 * measured session 21 (2026-09-04, rederive), chassis floor 7/91.
 */
s32 CD_datasync(s32 a0) {
    s32 v0;
    s32 cnt;
    s32 *tbl_11dc;
    u8 *idx_1494;
    s32 *tbl_125c;

    D_800F19B8 = VSync(-1) + 0x3C0;
    tbl_11dc = D_800A11DC;
    idx_1494 = &D_800A1494;
    tbl_125c = D_800A125C;
    D_800F19BC = 0;
    D_800F19C0 = &D_800162C0;

loop:
    v0 = VSync(-1);
    if (D_800F19B8 < v0) {
        {
            s32 arg4;
            puts(&g_str_cd_timeout);
            arg4 = tbl_125c[idx_1494[0]];
            printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5], arg4,
                   tbl_125c[idx_1494[1]]);
            CD_flush();
        }
        v0 = -1;
        goto check;
    }
    cnt = D_800F19BC;
    D_800F19BC = cnt + 1;
    if (0x3C0000 < cnt) {
        {
            s32 arg4;
            puts(&g_str_cd_timeout);
            arg4 = tbl_125c[idx_1494[0]];
            printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5], arg4,
                   tbl_125c[idx_1494[1]]);
            CD_flush();
        }
        v0 = -1;
        goto check;
    }
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
