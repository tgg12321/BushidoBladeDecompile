/* REJECTED — session 2.  Score 22 (candidate 18); 93 build insns vs 91.
 *
 * IDEA: target holds BOTH loop-invariant compare constants in $v0 (target idx
 * 38 `lui $v0,(0x3C0000>>16)`) / $v1 (idx 77 `lui $v1,(0x1000000>>16)`), i.e.
 * in registers that were just freed.  The session-2 candidate reuses a fresh
 * scratch local `k` for both constants (which is what defeats loop.c's
 * invariant hoisting — see candidate.c), but `k` gets allocated $a0 and the
 * scheduler then drops `lui a0,0x3c` into the `bnez` delay slot where target
 * keeps a nop.  So: reuse the EXISTING `v0` local as the constant holder
 * instead of introducing `k`, hoping it lands in $v0 like target.
 *
 * RESULT: 22 / 93.  The reuse still defeats the hoisting (no $s4/$s5, the
 * $s0-$s3 map stays correct), but overloading `v0` — which is also the
 * timeout/success flag consumed by `if (v0 != 0)` — lengthens and fragments
 * that pseudo's live range and costs more in the flag block than it saves at
 * the compare.
 *
 * VERDICT: the reuse holder must be a SEPARATE local from the exit flag.
 * Keep `k`.  If the $a0-vs-$v0 placement is to be fixed, it has to come from
 * somewhere other than folding the holder into `v0`.
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

    do {
        v0 = sys_VSync(-1);
        if (D_800F19B8 < v0) {
            goto do_timeout;
        }
        cnt = D_800F19BC;
        D_800F19BC = cnt + 1;
        v0 = 0x3C0000;
        if (!(v0 < cnt)) {
            goto success;
        }

    do_timeout:
        tslTm2LoadImage_2(&D_800161B8);
        {
            s32 arg5, arg4;
            arg5 = tbl_125c[idx_1494[1]];
            arg4 = tbl_125c[idx_1494[0]];
            debug_printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5], arg4, arg5);
        }
        cdrom_ClearIrq();
        v0 = -1;
        goto check;

    success:
        v0 = 0;

    check:
        if (v0 != 0) {
            return -1;
        }
        v0 = 0x1000000;
        if (!(*D_800A14C0 & v0)) {
            return 0;
        }
    } while (a0 == 0);
    return 1;
}
