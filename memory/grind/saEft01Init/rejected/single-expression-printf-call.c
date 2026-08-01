/* REJECTED — session 1.  Score 23 (worse than the 18 floor); insn count still
 * 91, so this is a pure scheduling/RA regression.
 *
 * Hypothesis under test: the staged `arg4`/`arg5` locals in the inherited
 * baseline looked like a prior session's hack, and the natural original
 * spelling would be one call expression with the two table lookups written
 * inline as arguments.
 *
 * KILLED.  Collapsing the staging into the call expression makes GCC expand
 * the two lookup chains in the opposite order and re-schedule the whole
 * block, costing 5 points.  The named intermediates are load-bearing: they
 * are what pins the two `lbu` at the head of the block the way target has
 * them.  Keep the staged form (see candidate.c); the open question there is
 * only the ORDER of the two staged assignments, not their existence.
 */
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
    tslTm2LoadImage_2(&D_800161B8);
    debug_printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5],
                 tbl_125c[idx_1494[0]], tbl_125c[idx_1494[1]]);
    cdrom_ClearIrq();
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
