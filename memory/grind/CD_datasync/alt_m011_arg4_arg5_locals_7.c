/* CD_datasync (saEft01Init) - SESSION 42 ALTERNATE FLOOR FORM.  7 / 91,
 * build_insns 91, rules_dropped 0 (live `sandbox CD_datasync --disable all`).
 *
 * This is a SECOND, BYTE-DISTINCT function that ties candidate.c's floor of 7.
 * It is candidate.c's chassis with arg5 ALSO hoisted into a value local after
 * puts() (candidate hoists only arg4).  md5 of the emitted whole function:
 *   candidate.c / m010  -> 8b0d0bf4ec63edd058c7d17b2df537bb
 *   this file  / m011   -> 1bb8643812d2793ab8a96cf8241c5f86
 *
 * WHY IT MATTERS MORE THAN candidate.c FOR THE RESIDUAL: it reproduces more of
 * target's printf-window SPINE.  Aligned against target (s21 cmp.py, expanded
 * slots 41-56), this form is correct at slots 42, 45, 46, 48, 49, 50 - the
 * whole idx1/arg5 address chain (`lbu $v0,1($s1)` / `sll $v0,$v0,2` /
 * `addu $v0,$v0,$s0` / `lw $v1,0($v0)`) AND the arg3 leaf pair
 * (`lui $v0,%hi` / `lbu $v0,%lo`) land in target's exact slots.  candidate.c is
 * correct at only 43, 44, 45, 46 and gets 45/46 partly by coincidence (its
 * arg4 chain happens to spell the same bytes target's arg5 chain does).
 *
 * ITS REMAINING DEFECT IS NOW A SINGLE SWAP: arg4's address chain occupies
 * slots 43/44/47 - exactly the slots target gives to arg2's `lui $a1,%hi(S)` /
 * `lw $a1,%lo(S)($a1)` - while arg2's pair is pushed down to 51/52.  Target
 * wants arg2 at 43/44 and arg4's chain SPLIT (sll at 47, addu at 51, lw at 56).
 * So on this chassis the residual reads "advance arg2's load two slots and
 * split arg4's sll from its addu", which is a sharper statement than the one
 * candidate.c supports.
 *
 * s42 MEASURED AND KILLED on this form: hoisting arg2 (D_800F19C0) into a
 * value local in ANY of the three assignment positions is byte-identical to
 * leaving it inline (forms4 p245 / p425 / p452 == p45), so the arg2 advance is
 * NOT reachable by hoisting it.  Every type-narrowing spelling of the locals
 * and of the table pointers (u32 tables, u32/void* argument locals, u32 index
 * locals, an (s32) cast on the arg3 index, u32 cnt) is byte-identical to this
 * file (forms5 t1-t7, one md5).
 *
 * candidate.c is left as the ledger's candidate (same score, longer provenance);
 * this file is the recommended STARTING CHASSIS for the next session's work on
 * the arg2/arg4 slot swap.
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
        s32 arg4;
        s32 arg5;
        puts(&g_str_cd_timeout);
        arg4 = tbl_125c[idx_1494[0]];
        arg5 = tbl_125c[idx_1494[1]];
        printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5], arg4, arg5);
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
