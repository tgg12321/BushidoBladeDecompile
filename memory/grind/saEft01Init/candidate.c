/* saEft01Init — best form as of grind session 3 (structural).
 *
 * Honest pure-C distance (sandbox --disable all): 18   [floor UNCHANGED
 * across sessions 1-3]  ·  92 build insns vs 91 target.
 * Body is byte-for-byte the session-2 candidate; s3 re-measured 13 further
 * structural variants and none beat it.  What changed is the DIAGNOSIS.
 *
 * CLUSTER A IS SOLVED (session 2).  This form produces target's exact
 * callee-save map using only $s0-$s3:
 *     $s0 = D_800A125C (tbl_125c)   $s1 = &D_800A1494 (idx_1494)
 *     $s2 = the a0 param            $s3 = D_800A11DC  (tbl_11dc)
 * via (1) a REAL do/while loop, which emits NOTE_INSN_LOOP_BEG so flow.c
 * loop-depth-weights the in-loop refs and global.c:allocno_compare ranks
 * the two table pointers above the 1-use param, and (2) ONE reused scratch
 * local `k` holding both loop-invariant compare constants, which denies
 * loop.c the hoist that the plain real-loop form suffers (29/98).
 *
 * *** SESSION-3 CAVEAT ON `k` — READ BEFORE ANY COMPLETION CLAIM ***
 * The single reused `k` CANNOT be the original spelling.  Target
 * materialises the two constants in TWO DIFFERENT hard registers —
 * `lui $v0,(0x3C0000>>16)` at target idx 41 and `lui $v1,(0x1000000>>16)`
 * at idx 83 — and one C variable is one pseudo, hence one hard register.
 * Our build puts both in $a0.  So `k` is a synthetic LICM defeat standing
 * in for whatever the original did, and it has still not been through
 * cheat-reviewer.  Session 3 found the actual loop.c gate (see below) and
 * a natural spelling that works for ONE of the two constants.
 *
 * THE LICM GATE (tools/gcc-2.7.2/loop.c:695, measured s3):
 *   a movable is NOT built only when all three fail:
 *     (A) ! maybe_never && ! loop_reg_used_before_p (...)
 *     (B) ! REG_USERVAR_P (dest) && ! REG_LOOP_TEST_P (dest)
 *     (C) reg_in_basic_block_p (p, dest)
 *   A user local whose live range crosses a branch, set where maybe_never
 *   is already 1, therefore stays inline in its OWN pseudo — no double-set
 *   needed.  This works for the 0x1000000 mask constant (set at `check:`,
 *   spanning the `if (v0 != 0)` branch — 97 insns) but NOT for 0x3C0000,
 *   whose set is too early in the loop body for maybe_never to be 1 yet
 *   (98 insns, i.e. hoisted).  loop.c:702 (n_times_set != 1 and
 *   ! consec_sets_invariant_p) is the ONLY route found so far that keeps
 *   BOTH inline — and it costs the two-register shape.
 *
 * THE REMAINING RESIDUAL (all of it is the tail; RA is already correct):
 *   +2 : our mask exit is `bnez v0,<cont> / nop / j <end> / move v0,zero`
 *        where target has `beqz $v0,.L80081CFC / addu $v0,$zero,$zero`.
 *        s3 CONFIRMED the mechanism: jump.c:1764 inverts a conditional
 *        jump over an unconditional one only when the unconditional jump
 *        IMMEDIATELY follows (prev_active_insn (reallabelprev) == insn);
 *        the `ret = 0;` set in the if-body blocks it.  Writing the exit as
 *        a bare `goto` DOES produce target's branch sense, but then the
 *        `return 0;` block is stranded out of line and reorg refuses to
 *        steal it into the delay slot (19/93).  See rejected/
 *        goto-exits-invert-branch-but-strand-ret0-block.c and F7.
 *   -1 : `k` in $a0 lets the scheduler hoist `lui a0,0x3c` into the bnez
 *        delay slot at build idx 31 where target keeps a nop.
 *
 * Do NOT re-try (all banked in rejected/): inlining the three table globals
 * at their use sites (40); one-expression printf call (23); plain real loop
 * without the reuse (29/98); v0 as the holder for both constants (22/93);
 * inline `return -1/0/1` instead of break+ret (19/93); `for(;;)` with three
 * inline returns (19/93); if/else and `continue` spellings of the mask test
 * (both collapse to this exact output, 18/92); goto exits in every label
 * order (19/93); two branch-spanning const locals (27/97); v0-timeout plus
 * spanning-mask local (22/94).
 */
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
            ret = -1;
            break;
        }
        k = 0x1000000;
        if (!(*D_800A14C0 & k)) {
            ret = 0;
            break;
        }
        ret = 1;
    } while (a0 == 0);
    return ret;
}
