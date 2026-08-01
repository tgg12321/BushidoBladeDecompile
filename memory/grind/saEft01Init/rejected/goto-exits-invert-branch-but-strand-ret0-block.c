/* REJECTED (s3) — 19 at 93 insns (candidate is 18 at 92).
 *
 * WHAT IT PROVES (keep this, it is the useful half): replacing the mask
 * exit's compound body (`ret = 0; break;`) with a BARE `goto` makes
 * jump.c:1764 ("Detect a conditional jump jumping over an unconditional
 * jump") fire, because that transform requires
 * `prev_active_insn (reallabelprev) == insn` — i.e. the unconditional jump
 * must IMMEDIATELY follow the conditional one, and the `ret = 0;` set sat
 * in between.  With the bare goto the build emits target's exact branch
 * SENSE: `beqz $v0, <exit>` instead of our `bnez $v0,<cont> / nop / j <end>`.
 *
 * WHY IT STILL LOSES: the `return 0;` block is then stranded out of line as
 * `(insn 208 (set (reg/i:SI 2 v0) (const_int 0)))` immediately followed by
 * `code_label 218` (the shared return label), and reorg's
 * fill_slots_from_thread REFUSES the own-thread steal for that branch, so
 * we pay `nop` + `j 218` + `nop` + `move v0,zero` (net +1 vs the candidate).
 * The SAME steal succeeds for the flag exit in the same function: the
 * timed_out block is fully consumed (`li v0,-1` lands in the bnez delay
 * slot and its label becomes NOTE_INSN_DELETED_LABEL).  The asymmetry is
 * the whole remaining problem — see hypotheses.md F7.
 *
 * Out-of-loop label ORDER is irrelevant: w4 (aborted before timed_out),
 * w8/w9 (three goto exits with the `return 1` moved out of line too) and
 * w2/w3 (only one of the two exits converted) ALL emit a byte-identical
 * tail at 19/93.  GCC normalises the block order itself.
 */
s32 saEft01Init(s32 a0) {
    s32 v0;
    s32 cnt;
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
            goto timed_out;
        }
        k = 0x1000000;
        if (!(*D_800A14C0 & k)) {
            goto aborted;
        }
    } while (a0 == 0);
    return 1;

timed_out:
    return -1;
aborted:
    return 0;
}
