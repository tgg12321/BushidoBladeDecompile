/* !!! SESSION 24 CORRECTION (2026-09-04) !!!
 * The "8 / 91" in the header below is WRONG.  Session 24 re-measured this exact
 * file with the live sandbox (`sandbox CD_datasync --disable all`, HEAD chassis)
 * and it scores **10 / 91**, not 8.  The fast harness agrees (lev 10, nop 3, and
 * the s21/s22 calibration sandbox = lev + nop - 3 was verified exactly on two
 * points this session: candidate.c 7 == 7 and this file 10 == 10).
 * It is also STRUCTURALLY FARTHER from target than candidate.c, not closer:
 * aligned against the target window, candidate.c has 6 displaced insns and this
 * file has 8, and this file's arg4 chain completes at window slot 9 where target
 * completes it LAST (slot 16).  The s23 "geometry-correct basin" reading and the
 * whole s23 frontier built on it (the "one displacement" / "arg2 second-consumer"
 * programme) are therefore VOID - see evidence.md [s24].
 * Keep the file as a measured data point; do NOT use it as a chassis.
 */
/* CD_datasync (saEft01Init) - SESSION 23 ARTIFACT: the first form in 23
 * sessions that reproduces the TARGET'S EXACT BLOCK-3 GEOMETRY.
 * sandbox 8 / 91 (lev 8, nop 3) - one WORSE than the 7-floor base form, but
 * structurally the closest thing measured: it is the only geometry class that
 * matches target on all three of the discriminants s22 named:
 *   (1) idx_1494[0]'s lbu is emitted FIRST (build 41) while its chain
 *       COMPLETES LAST (the lw into $a3 is the last insn of the block),
 *   (2) idx_1494[1]'s chain completes first,
 *   (3) arg5's loaded VALUE lands in a register DISTINCT from its own address
 *       register (the anti-coalescing condition of the s22 frontier).
 * The whole residual of this form is ONE displacement: arg2's
 * `lui $a1 / lw $a1,%%lo(D_800F19C0)` sinks from target's build 43-44 to 54-55,
 * because local-alloc hands $a1 to arg5's VALUE (the seat target gives to
 * $v1).  Ten arg2 spellings x placements (inline, named `s32 *` local at five
 * statement positions, u32-cast local, read before puts) are BYTE-IDENTICAL to
 * this file; hoisting arg2 to function scope regresses to 28+ because the
 * pseudo then takes a callee-saved seat.  See hypotheses.md [s23].
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
    /* FAKE: do{}while(0) — loop_depth weighting for the three table pointers
     * without giving loop.c a loop to hoist the compare constants out of.
     * mechanism: flow.c loop_depth ref-weighting -> global.c allocno_compare.
     * lever-exhaustion: memory/grind/CD_datasync/hypotheses.md (s9 H37/H38). */
    do {
        s32 *m2;
        s32 k0;
        s32 i1;
        s32 arg4;
        s32 arg5;
        puts(&g_str_cd_timeout);
        m2 = D_800F19C0;
        i1 = idx_1494[1];
        k0 = idx_1494[0] * 4;
        arg5 = tbl_125c[i1];
        arg4 = *(s32 *)((u8 *)tbl_125c + k0);
        printf(&D_800161C8, m2, tbl_11dc[D_800A11D5], arg4,
               arg5);
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
