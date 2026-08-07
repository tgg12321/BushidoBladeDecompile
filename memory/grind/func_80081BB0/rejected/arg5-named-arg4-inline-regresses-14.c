/* REJECTED (session 5, H21) — 14 / 91, a 6-point regression.
 *
 * Hypothesis: target names only the FIFTH argument (it homes it through a
 * register into the stack slot, `lw v1,0(v0) ... sw v1,16(sp)`) and writes
 * the fourth inline in the call, which is why target's `lw a3,0(a0)` is the
 * LAST memory reference of the block.  Measured: 14 / 91 in all three
 * spellings (block-scope decl, decl-with-initialiser, function-scope decl —
 * byte-identical to each other).  The mirror image (arg4 named, arg5 inline)
 * is 8 / 91 and is the banked candidate.  Whatever defers target's `lw a3`,
 * it is not an inline fourth argument.
 */
s32 saEft01Init(s32 a0) {
    s32 v0;
    s32 cnt;
    s32 ret;
    s32 k;
    s32 *tbl_11dc;
    u8 *idx_1494;
    s32 *tbl_125c;

    D_800F19B8 = sys_VSync(-1) + 0x3C0;
    tbl_11dc = D_800A11DC;
    idx_1494 = &D_800A1494;
    tbl_125c = D_800A125C;
    D_800F19BC = 0;
    D_800F19C0 = &D_800162C0;

    do {
        v0 = sys_VSync(-1);
        if (D_800F19B8 < v0) {
            goto do_timeout;
        }
        cnt = D_800F19BC;
        D_800F19BC = cnt + 1;
        k = 0x3C0000;
        if (!(k < cnt)) {
            goto success;
        }

    do_timeout:
        tslTm2LoadImage_2(&D_800161B8);
        {
            s32 arg5;
            arg5 = tbl_125c[idx_1494[1]];
            debug_printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5], tbl_125c[idx_1494[0]], arg5);
        }
        cdrom_ClearIrq();
        v0 = -1;
        goto check;

    success:
        v0 = 0;

    check:
        if (v0 != 0) {
            ret = -1;
            break;
        }
        k = 0x1000000;
        cnt = k;
        ret = *D_800A14C0 & cnt;
        if (ret == 0) {
            break;
        }
        ret = 1;
    } while (a0 == 0);
    return ret;
}
