/* REJECTED (s9, H39): naming the ADDRESS (`p4 = &tbl_125c[idx_1494[0]];`) and
 * dereferencing it inline in the call, which is the literal reading of target's
 * `addu a0,a0,s0 ... lw a3,0(a0)`, scores 9 / 91 — worse than naming the VALUE
 * (7). Variants: both slots as named pointers 10, pointer+named-value 9.
 */
/* u1: named pointer to the idx[0] slot, dereferenced inline in the call */
/* r1: do{}while(0) around the whole timeout/printf block (depth 2 refs) */
s32 saEft01Init(s32 a0) {
    s32 v0;
    s32 cnt;
    s32 *tbl_11dc;
    u8 *idx_1494;
    s32 *tbl_125c;

    D_800F19B8 = sys_VSync(-1) + 0x3C0;
    tbl_11dc = D_800A11DC;
    idx_1494 = &D_800A1494;
    tbl_125c = D_800A125C;
    D_800F19BC = 0;
    D_800F19C0 = &D_800162C0;

loop:
    v0 = sys_VSync(-1);
    if (D_800F19B8 < v0) {
        goto do_timeout;
    }
    cnt = D_800F19BC;
    D_800F19BC = cnt + 1;
    if (!(0x3C0000 < cnt)) {
        goto success;
    }

do_timeout:
do {
        tslTm2LoadImage_2(&D_800161B8);
        {
            s32 *p4;
            p4 = &tbl_125c[idx_1494[0]];
            debug_printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5], *p4, tbl_125c[idx_1494[1]]);
        }
        cdrom_ClearIrq();
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
