/* REJECTED (s25). `s32 *p4 = &tbl_125c[idx_1494[0]];` + `*p4` in the printf call.
 * lev 9. Structurally the closest non-candidate window ever measured: arg4's VALUE
 * load moves to the register-load point (target's `lw $a3,0($a0)` last), which no
 * value-local form achieves. But the pointer spelling emits `addu $v1,$s0,$v1`
 * (BASE first) where target has `addu $a0,$a0,$s0` (INDEX first). Six spellings
 * of the same idea (p2/q1/q5/q7/q8/qa: `&tbl[i]`, `i + tbl`, `p4[0]`, with/without
 * an arg5 value local, with an aliased index pointer) all emit BYTE-IDENTICALLY.
 * The operand flip is not order-dependent, so this family is excluded by operand
 * order alone, independent of any scheduling question. */
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
        goto do_timeout;
    }
    cnt = D_800F19BC;
    D_800F19BC = cnt + 1;
    if (!(0x3C0000 < cnt)) {
        goto success;
    }

do_timeout:
    /* FAKE: do{}while(0) */
    do {
        s32 *p4;
        puts(&g_str_cd_timeout);
        p4 = &tbl_125c[idx_1494[0]];
        printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5], *p4,
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
