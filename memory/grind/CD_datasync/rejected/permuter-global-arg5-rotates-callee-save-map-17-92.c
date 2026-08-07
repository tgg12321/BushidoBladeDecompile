/* REJECTED (grind session 13, permuter).  sandbox 17 / 92 — a REGRESSION of
 * ten over the candidate's 7 / 91, even though decomp-permuter scored it
 * BETTER than the candidate (423 vs the candidate's 435).
 *
 * The one and only change is arg5: `tbl_125c[idx_1494[1]]` respelled as a
 * direct index of the GLOBAL, `D_800A125C[idx_1494[1]]`.  Semantically
 * identical (tbl_125c is initialised to D_800A125C and never re-assigned), and
 * it is the first form in 65 measured argument spellings that reads the array
 * through its own symbol instead of the hoisted base — so it was worth the
 * measurement.  What it actually does:
 *
 *   * the second reference to the symbol costs a fresh `lui at` + `addu at,v1`
 *     + `lw v1,0(at)` address chain, so the function grows to 92 instructions;
 *   * dropping tbl_125c to ONE use in the block collapses its allocno priority,
 *     and the whole callee-save map rotates ($s0/$s1/$s2 take D_800A1494 /
 *     the param / D_800A125C instead of target's D_800A125C / D_800A1494 /
 *     param), which is what the extra ten positional diffs are;
 *   * the argument block itself lands FURTHER from target than the candidate's
 *     (`lbu v0,0(s0)` / `lbu v1,1(s0)` against target's `lbu a0,0(s1)` /
 *     `lbu v0,1(s1)`, and `lw a3` moves to build idx 56).
 *
 * This is the concrete instance behind session 13's headline kill: the permuter
 * charges 5 per renamed register and 60 per reordered instruction, so a
 * complete callee-save rotation (16 renames = 80) is CHEAPER to it than the
 * candidate's seven correctly-allocated-but-misordered instructions (7 x 60 =
 * 420).  The honest sandbox metric is position-locked and weights both at 1.
 * Do not re-propose global-symbol respellings of the hoisted bases, and do not
 * trust a permuter score on this function without re-scoring in the sandbox.
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
    /* FAKE: do{}while(0) — loop_depth weighting for the three table pointers
     * without giving loop.c a loop to hoist the compare constants out of. */
    do {
        s32 arg4;
        tslTm2LoadImage_2(&D_800161B8);
        arg4 = tbl_125c[idx_1494[0]];
        debug_printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5], arg4,
                     D_800A125C[idx_1494[1]]);
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
