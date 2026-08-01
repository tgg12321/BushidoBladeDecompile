/* KILLED (s7, 16/91, from the candidate's 8/91).  Sony declares the two
 * objects this function touches as `volatile Alarm_t Alarm;` and
 * `static volatile CD_intr Intr = {0};`, so applying `volatile` to BB2's
 * D_800F19B8 / D_800F19BC / D_800F19C0 / D_800A1494 extern declarations is a
 * legitimate header-type correction, NOT a coercion -- and it was the one
 * remaining lever the session-6 frontier wanted (a DAG change that could
 * separate the two isomorphic argument chains).
 *
 * MEASURED DEAD.  On the candidate chassis it costs 8 points (8 -> 16); on the
 * reference chassis (r0) it changes nothing at all (35 -> 35).  BB2 links PsyQ
 * 4.0 where sotn's reference is 3.5-era, so the shipped object evidently was
 * NOT built with these volatile-qualified.  Do not re-propose volatile here.
 *
 * The harness that applies the declaration rewrite to every extern block in
 * the TU is tmp/grind/saEft01Init/s7/vscore.py (`--novol` disables it). */
s32 saEft01Init(s32 a0) {
    s32 v0;
    s32 cnt;
    s32 ret;
    s32 k;
    s32 *tbl_11dc;
    volatile u8 *idx_1494;
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
            s32 arg4;
            arg4 = tbl_125c[idx_1494[0]];
            debug_printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5], arg4, tbl_125c[idx_1494[1]]);
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
