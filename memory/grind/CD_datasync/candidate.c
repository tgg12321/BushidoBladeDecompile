/* s61 NOTE (rederive): this file is UNCHANGED and still scores 2 (the order-lost
 * two-step form).  The informative body of s61 is
 * progress/s61-dmaptr-carrier-window-and-flag-byte-exact-3.c - score 3, with the
 * do_timeout window AND the flag block byte-exact and the whole residual moved
 * into three tail instructions.  Read that header and the s61 block of
 * evidence.md before starting from anything here.
 */
/* CD_datasync - SESSION 60 (rederive, forced by the CD_ready sibling trigger).
 * SCORE OF THIS BODY: 2 / 91 (ties the 59-session floor) - but the session's
 * REAL result is a new chassis, and the next session should probably start
 * from progress/s60-cdready-transplant-v0-carrier-window-byte-exact-3.c
 * instead of from this file.  Read this header before anything else.
 *
 * WHAT HAPPENED.  CD_ready (marionation_Exec) reached COMPLETED-C on main
 * 2026-09-06 and its do_timeout window - byte-shape-identical to ours - is
 * the matched spelling (src/system.c:379-435).  s60 transplanted that exact
 * spelling onto the CD_datasync chassis.  The transplant changed the shape of
 * the residual for the first time since s2.
 *
 * 1. THE 59-SESSION ORDER RESIDUAL IS SOLVED.  Every session since s2 has
 *    been fighting "arg4's index sll is scheduled two slots too early"
 *    (target 80081C78-84: `sll v0,v0,2 ; addu v0,v0,s0 ; sll a0,a0,2 ;
 *    lw v1,0(v0)`).  s57 proved by solver arithmetic that no statement
 *    REORDER inside the old chassis could reach it, and s59 proved every
 *    added-luid spelling re-prices five quantities at once.  Both remain true
 *    OF THE OLD CHASSIS.  CD_ready's structure reaches the order directly:
 *      - `puts(...)` is emitted OUTSIDE (before) the do{}while(0) wrap;
 *      - the chain-A raw byte read `t0 = idx_1494[0];` is ALSO outside the
 *        wrap, at loop depth 1, so its ref weight is 7 not 8;
 *      - inside the wrap, chain B is computed FIRST (tb -> pB), chain A's
 *        address SECOND, and `arg5 = *pB` third.
 *    With a multi-set carrier for chain A's address (see 2), the emitted
 *    window at tmp/grind/CD_datasync/s60/dis_P3.txt is BYTE-EXACT against
 *    asm/funcs/CD_datasync.s:45-63 - all fifteen instructions, seats included.
 *
 * 2. THE CARRIER IS THE WHOLE GAME.  Chain A's address must be staged through
 *    a pseudo that is MULTI-SET WITH AT LEAST ONE SET OUTSIDE THE WINDOW.
 *    Measured, all on this chassis:
 *      v0 (the VSync-result / -1 / 0 flag local) ... 3   <- window byte-exact
 *      cnt (the poll counter)                     ... 4   <- window byte-exact
 *      fresh fn-scope local, single set           ... 8
 *      fresh local, split two-step (aA=t0<<2; aA+=base) 2 <- order LOST again
 *      tbl_125c (re-set after its last read)      ... 2   <- order LOST again
 *      t0 self-reassigned / tb / pB re-used       ... 8 / 8 / 4
 *      fresh local + a dead store before the loop ... 8   (dead store inert)
 *      fresh local + a dead store after the wrap  ... 8   (dead store inert)
 *      the puts() argument pointer as carrier     ... 8
 *    A dead store does NOT buy the multi-set-ness: only sets that carry a
 *    real value on a reaching path move the allocation.
 *
 * 3. THE NEW RESIDUAL IS THREE INSTRUCTIONS AND ONE PSEUDO.  On the v0-carrier
 *    form (score 3) the ONLY divergence left in the whole function is that the
 *    -1/0 flag lives in $a0 instead of $v0:
 *        ours   j .. ; li a0,-1  /  move a0,zero  /  bnez a0,.. ; li v0,-1
 *        target j .. ; li v0,-1  /  move v0,zero  /  bnez v0,.. ; li v0,-1
 *    Cause: GCC 2.7.2 has no web splitting, so the carrier def and the flag
 *    defs are one pseudo -> one quantity -> one hard reg, and the chain-A
 *    address correctly prefers $a0 (target holds it there too).  The flag is
 *    dragged along.  Splitting the flag into its own local restores $v0 for
 *    the flag but makes the carrier single-set again and the ORDER is lost
 *    (rejected/s60-flag-split-from-v0-carrier-8.c = 8).  Four escapes were
 *    measured and all cost more: `return v0` instead of `return -1` (4),
 *    staging the flag through cnt (3, identical residual), copying the
 *    carrier into a fresh pointer before the printf (3, identical residual),
 *    putting the carrier on chain B instead of chain A (8).
 *
 * 4. WHAT THIS BODY IS.  The same transplant with a fresh block-scope local
 *    split two-step (aA = t0 << 2; aA += (s32)tbl_125c).  It scores 2 with
 *    the OLD 2-insn order residual back - i.e. it ties the floor without
 *    telling you anything new.  It is here only because the contract asks for
 *    the lowest-scoring form; the 3-scoring v0-carrier form in progress/ is
 *    the one with information in it.
 *
 * FRONTIER FOR s61: find a third pseudo in this function that is (a)
 * multi-set with a set outside the do_timeout window, (b) carries a real
 * value on a reaching path, and (c) tolerates $a0 for all of its uses.  The
 * function only has v0, cnt, a0, tbl_11dc, tbl_125c and idx_1494 at function
 * scope; v0 (3) and cnt (4) are the only two that reach the order at all, and
 * both pay for it in their own block.  If no such pseudo exists, the question
 * to ask is whether CD_datasync's original source had a statement AFTER the
 * timeout window that CD_ready has too (CD_ready's carrier `src` is the copy
 * loop's source pointer, a genuinely later-used local) - i.e. whether our
 * control-flow tail is the wrong shape rather than our window.
 */
s32 CD_datasync(s32 a0) {
    s32 v0;
    s32 cnt;
    s32 *tbl_11dc;
    volatile u8 *idx_1494;
    s32 *tbl_125c;

    D_800F19B8 = VSync(-1) + 0x3C0;
    tbl_11dc = D_800A11DC;
    idx_1494 = &g_cd_status_a;
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
    puts(&g_str_cd_timeout);
    {
        s32 arg5;
        s32 t0;
        s32 *pB;
        s32 tb;
        void **pp;
        s32 aA;
        t0 = idx_1494[0];
        do {
            tb = idx_1494[1];
            pB = (s32 *)((tb << 2) + (s32)tbl_125c);
            aA = t0 << 2;
            aA += (s32)tbl_125c;
            arg5 = *pB;
            pp = &D_800F19C0;
            printf(&D_800161C8, *pp, tbl_11dc[D_800A11D5], *(s32 *)aA, arg5);
            CD_flush();
        } while (0);
    }
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
