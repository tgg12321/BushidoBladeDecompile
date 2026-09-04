/* CD_datasync — REJECTED (s48).  harness lev=9 nop=3 (engine >= 9).
 *
 * The one form family in s48 that DID move the block-3 quantity table.
 * Spelling arg4's element ADDRESS as a named pointer local (i4 = idx[0];
 * p4 = &tbl_125c[i4]; ... printf(..., *p4, val5)) creates the long spanning
 * quantity the ra_solver vector asks for:
 *     q0 [8,14)  refs12 -> $v0
 *     q1 [14,20) refs4  -> $v0   (arg5 value)
 *     q2 [16,36) refs12 -> $v1   (arg4 pointer — now spans the whole region)
 *     q3 [24,34) refs12 -> $v0   (arg3 chain)
 * The arg4 quantity is born at 16 instead of 20 (solver atom "qty2 born
 * earlier", cost 2) and still lands in $v1, because only ONE other quantity
 * (q3, in $v0) overlaps its span; find_free_reg's ascending scan stops at
 * $v1.  Reaching $a0 needs TWO quantities alive across [20,36) and allocated
 * ahead of it — and the second one has to be the arg5 value, which sched1
 * shortens to 2 luids (see the sibling rejected form).  39 forms swept
 * (arg4 in {ikp,ip} x arg5 in {v,iv,ik} x all interleavings); best 9.
 * Unlike the value-local lattice, statement ORDER is live here: w0 gives a
 * different table from w1..w9, so single-use pointer locals are not
 * copy-propagated into the call the way single-use value locals are.
 */
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
    /* FAKE: do{}while(0) - loop_depth weighting for the three table pointers
     * without giving loop.c a loop to hoist the compare constants out of.
     * mechanism: flow.c loop_depth ref-weighting -> global.c allocno_compare.
     * lever-exhaustion: memory/grind/CD_datasync/hypotheses.md (s9 H37/H38). */
    do {
        s32 i5;
        s32 val5;
        s32 i4;
        s32 *p4;
        puts(&g_str_cd_timeout);
        i5 = idx_1494[1];
        val5 = tbl_125c[i5];
        i4 = idx_1494[0];
        p4 = &tbl_125c[i4];
        printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5], *p4, val5);
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
