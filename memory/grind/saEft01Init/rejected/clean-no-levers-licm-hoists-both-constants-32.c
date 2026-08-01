/* KILLED as a score improvement (s7, 32/96) but THE most important measured
 * form in the ledger -- this is the HONEST CHEAT-FREE FLOOR of the known-good
 * basin: the proven sessions-2/3 chassis with every codegen-control lever
 * removed and the statements written exactly as the Sony reference has them
 * (literal constants, no `k`, no `cnt = k` staging, no named `arg4`).
 *
 * The inherited 8/91 is therefore NOT a cheat-free 8.  Ladder, each lever
 * added to this file in isolation:
 *     clean   (no levers) ..................... 32 / 96
 *     cleana  (+ named arg4 only) ............. 27 / 96
 *     cleank  (+ k double-set LICM defeat) .... 21 / 90
 *     candidate (k + cnt=k staging + arg4) .....  8 / 91
 *
 * INSTRUMENTED-cc1 FORENSICS (tmp/grind/saEft01Init/s7/clean/cc1.log, ALLOCDBG):
 *     ord=2 pseudo=78 hardreg=16 nrefs=5 livelen=96 pri=1041   tbl_125c -> $s0
 *     ord=3 pseudo=77 hardreg=17 nrefs=5 livelen=98 pri=1020   idx_1494 -> $s1
 *     ord=4 pseudo=72 hardreg=18 nrefs=3 livelen=52 pri= 576   the param -> $s2
 *     ord=5 pseudo=108 hardreg=19 nrefs=3 livelen=92 pri= 326  0x1000000 -> $s3
 *     ord=6 pseudo=85  hardreg=20 nrefs=3 livelen=94 pri= 319  0x3C0000  -> $s4
 *     ord=7 pseudo=76  hardreg=21 nrefs=3 livelen=100 pri=300  tbl_11dc -> $s5
 *
 * The first THREE dispositions are already target's exactly, with no lever of
 * any kind.  The whole allocation defect is that loop.c's LICM creates pseudos
 * 85 and 108 for the two loop-invariant compare constants; they rank 5th and
 * 6th in global_alloc and displace tbl_11dc from target's $s3 to $s5, adding a
 * fifth and sixth callee-save (+5 instructions: 96 vs 91).
 *
 * So the clean-C matching problem for this function reduces to ONE question:
 * a legitimate spelling that denies loop.c those two hoists.  `cleank` proves
 * the payoff is exact -- with the hoists suppressed the map is target's
 * $s0/$s1/$s2/$s3 with four callee-saves and `k` in $a0 -- but it buys it with
 * the [[defeat-licm-hoist-var-reuse]] double-set, which is FAKE-family. */
/* clean = the proven sessions-2/3 chassis with EVERY codegen-control lever
 * removed and the statements written exactly as the Sony reference has them
 * (literal constants, no staging, no named argument intermediate).
 * This is the honest cheat-free-C floor of the known-good basin. */
s32 saEft01Init(s32 a0) {
    s32 v0;
    s32 cnt;
    s32 ret;
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
            ret = -1;
            break;
        }
        ret = *D_800A14C0 & 0x1000000;
        if (ret == 0) {
            break;
        }
        ret = 1;
    } while (a0 == 0);
    return ret;
}
