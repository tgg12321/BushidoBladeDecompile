/* NOT an improvement (s7, 21/90) but the PROOF that the LICM hoist is the only
 * allocation-critical defect: `clean` + ONLY the double-set `k` LICM defeat,
 * with no staging and no named arg4.
 *
 * ALLOCDBG (tmp/grind/saEft01Init/s7/cleank/cc1.log):
 *     ord=1 pseudo=76 hardreg=4  nrefs=8 livelen=6   pri=40000   k -> $a0
 *     ord=3 pseudo=79 hardreg=16 nrefs=5 livelen=96  pri=1041    tbl_125c -> $s0
 *     ord=4 pseudo=78 hardreg=17 nrefs=5 livelen=98  pri=1020    idx_1494 -> $s1
 *     ord=5 pseudo=72 hardreg=18 nrefs=3 livelen=52  pri= 576    the param -> $s2
 *     ord=6 pseudo=77 hardreg=19 nrefs=3 livelen=100 pri= 300    tbl_11dc -> $s3
 *
 * Target's exact callee-save map, four callee-saves, both constants
 * materialised inline, and the LICM holder in a caller-saved register -- from
 * ONE lever on top of otherwise fully natural, reference-faithful C.  It lands
 * at 90 instructions, one FEWER than target's 91, so the remaining 21 is
 * scheduling plus that one missing instruction, not allocation.
 *
 * It is banked in rejected/ rather than adopted because 21 > the inherited 8
 * AND because the `k` double-set is the FAKE-family construct the whole
 * cheat-review question hangs on.  If a legitimate LICM suppression is ever
 * found, THIS is the form to graft it onto -- not the 8. */
/* cleank = clean + ONLY the k double-set LICM defeat (no staging, no arg4). */
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
        debug_printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5],
                     tbl_125c[idx_1494[0]], tbl_125c[idx_1494[1]]);
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
        ret = *D_800A14C0 & k;
        if (ret == 0) {
            break;
        }
        ret = 1;
    } while (a0 == 0);
    return ret;
}
