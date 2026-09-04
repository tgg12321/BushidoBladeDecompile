/* REJECTED (s49, forensics) - 10 / 92 est (engine-equivalent estimator
 * re-validated against the sandbox this session: candidate=7, r_ik_iv_w0=8).
 *
 * WHY DEAD: the s48 frontier's first item asked for arg4 in {ip,ikp} crossed
 * with arg5 in {ip,ikp} - two named element-address POINTER locals - on the
 * theory that two long address quantities across [20,36) would give
 * find_free_reg the second occupant the $a0 seat needs.  All 62 forms of the
 * cross (arg4 in {ip,ikp,dp} x arg5 in {ip,ikp,dp} x every statement
 * interleaving) emit a BYTE-IDENTICAL block-3 quantity table
 *   q0 [8,16) r12 -> $v0 | q1 [14,36) r12 -> $v1 | q2 [16,20) r4 -> $v0
 *   q3 [24,34) r12 -> $v0 | Sq4 [32,38) r4 -> $a1
 * and a byte-identical window.  Exactly ONE quantity (q1) spans [20,36), not
 * two, so the precondition is not met; the seat stays $v1.  The whole
 * pointer-cross is a single equivalence class: chain length (dp/ip/ikp) and
 * statement order are both erased.  It also LOSES F1 (idx1's lbu is emitted
 * first), which is why it is 10 rather than 7.
 *
 * Kept as the representative of the class (pp_ip_ip_w0); the other 61 forms
 * are in tmp/grind/CD_datasync/s49/forms/.
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
        s32 *p5;
        s32 i4;
        s32 *p4;
        puts(&g_str_cd_timeout);
        i5 = idx_1494[1];
        p5 = &tbl_125c[i5];
        i4 = idx_1494[0];
        p4 = &tbl_125c[i4];
        printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5], *p4, *p5);
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
