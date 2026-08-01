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
            s32 *p4;
            p4 = &tbl_125c[idx_1494[0]];
            debug_printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5], *p4, tbl_125c[idx_1494[1]]);
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

/* ============================================================================
 * WHY THIS IS BANKED (grind session 7, forensics) — 10 / 91, NOT a regression
 * to be re-derived, but a STRUCTURALLY NEW BASIN worth more work.
 *
 * Spelling: the fourth argument's ADDRESS is a named pointer local
 * (`p4 = &tbl_125c[idx_1494[0]];`) and the DEREFERENCE is written inline in
 * the call (`*p4`).  Semantically identical to `arg4 = tbl_125c[idx_1494[0]]`.
 *
 * It is the FIRST measured form that DECOUPLES the head and the load of the
 * fourth argument's chain — the thing every earlier session failed to do:
 *
 *   form      arg4 head pos   arg4 load pos   score
 *   cand       1               9              8
 *   n0/n2/a5n  9-12           16             14
 *   n4        11              16             13
 *   p4         2              13             10
 *   TARGET     1              16              -
 *
 * Mechanism (measured with the instrumented cc1, see evidence.md s7): naming
 * the ADDRESS puts the lbu/sll/addu at the statement's position (low LUIDs)
 * while `expand_call` still emits the load itself as `(set (reg a3) (mem ...))`
 * — the LAST insn of the argument sequence, target's expand shape.  Splitting
 * the chain that way is what lets sched1's reverse pass stretch it.
 *
 * Residual vs target in this basin (block positions):
 *   * the two `lbu` are swapped — p4 emits idx_1494[1] first, target
 *     idx_1494[0] first;
 *   * p4's `addu` is emitted with the operands commuted
 *     (`addu $v1,$s0,$v1` vs target's `addu $a0,$a0,$s0`);
 *   * the tail order is `sw(10) / lw a3(13) / lw a2(16)` where target has
 *     `sw(14) / lw a2(15) / lw a3(16)`.
 *
 * Neighbours measured this session, all in the same basin, none better:
 *   p4d (declaration-with-initialiser)              10  — byte-identical to p4
 *   p45  (both addresses named, arg4 first)         11
 *   p45r (both addresses named, arg5 first)         11
 *   q1   (arg5 VALUE named first + p4)              10
 *   q2   (p4 first + arg5 VALUE named)              10
 *   q3   (`p4 = tbl_125c + idx_1494[0]`)            10
 *   q4   (p4 + arg2's address named, p4 first)      11
 *   q5   (p4 + arg2's address named, arg2 first)    14
 * ==========================================================================*/
