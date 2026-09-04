/* s42 REJECTED - 20 / 91 (calibrated lev 20, n=77 vs target's 82).
 * Representative of the 28-form quadrant "fewer than three hoisted table
 * pointers".  s42 compiled the COMPLETE cross-product {2^3 hoist subsets} x
 * {arg3 value local on/off} x {arg4 value local on/off} = 32 forms.  Result:
 * only the all-three-hoisted subset (h7) reaches target's expanded instruction
 * count of 82 at all; every one of the 28 forms that drops at least one hoist
 * lands at n=69..80 and lev 20..38.  Best non-h7 cell is this one (h6 =
 * idx_1494 + tbl_125c hoisted, tbl_11dc read as the global) at 20.
 * The frontier item "register pressure from the number of hoisted callee-saved
 * bases is the last source-side variable" is answered NO on this chassis: the
 * three hoists are load-bearing for the instruction COUNT, not just the map.
 */
s32 CD_datasync(s32 a0) {
    s32 v0;
    s32 cnt;
    u8 *idx_1494;
    s32 *tbl_125c;

    D_800F19B8 = VSync(-1) + 0x3C0;
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
    /* FAKE: do{}while(0) - loop_depth weighting for the table pointers
     * without giving loop.c a loop to hoist the compare constants out of.
     * mechanism: flow.c loop_depth ref-weighting -> global.c allocno_compare.
     * lever-exhaustion: memory/grind/CD_datasync/hypotheses.md (s9 H37/H38). */
    do {
        puts(&g_str_cd_timeout);
        printf(&D_800161C8, D_800F19C0, D_800A11DC[D_800A11D5], tbl_125c[idx_1494[0]], tbl_125c[idx_1494[1]]);
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
