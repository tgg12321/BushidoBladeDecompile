/*
 * CANDIDATE — func_800401CC (src/text1a_pre.c), session 1 (recon), 2026-08-11.
 * Honest floor with this form: 7/78 (was 20/78 at session start).
 * This form is ALREADY APPLIED in src/text1a_pre.c at end of session 1.
 *
 * What closed 13 of the 20:
 *  1. Removed the `s32 v0;` staging local (old code did `v0 = D_800A36AC;
 *     a2 = v0 & 1;` and `v0 = D_800A3234;` before the second compare).
 *     The multi-block user local went to global-alloc and lost $v0 to the
 *     block-local temps; inlining the global reads makes each read a short
 *     local-alloc temp that takes ascending regs ($2 then $3), matching
 *     target in BOTH the head (insns 1-5) and the second compare (14-21).
 *     This retired the diff regions behind regfix rules
 *     `$2 <-> $3 @ 1-5` and `$2 <-> $3 @ 14-21`.
 *  2. Moved `D_800A3378 = (s32)(pkt + 6);` to AFTER the ot-link statement
 *     (it was between the two packet statements). GCC cannot disambiguate
 *     the `sw D_800A3378` global store vs the `lw ot[0x3FFC]` pointer read,
 *     so SOURCE statement order decides the dependence direction: with the
 *     store last, the ot re-read is an anti-dependence and the scheduler
 *     emits `lw; and; sw D_800A3378; and` exactly like target (insns 70-77).
 *     Before this, the sw was pinned before the lw and the tail had a
 *     2-insn structural mismatch whenever anything else moved.
 *
 * Remaining 7/78: single register-rename swap $6 <-> $7 over the tail
 * (regfix rule `$6 <-> $7 @ 51-68`): the two mask constants after the
 * SetDrawMove call. Target: 0xFFFFFF in $6 (a2), 0xFF000000 in $7 (a3).
 * Ours: reversed. Insn ORDER is byte-perfect (lui $?,0xFF; ori; lui $?,
 * 0xFF00 then identical and/or/sw shapes) — only the two hard-reg names
 * differ across 7 insns. See hypotheses.md for the measured local-alloc
 * mechanism and the killed levers; the next lever must make the 0xFFFFFF
 * qty allocate BEFORE the 0xFF000000 qty in local-alloc's qty_compare_1
 * ordering (priority = floor_log2(refs)*refs*size/(death-birth)).
 */
void func_800401CC(s32 a0, s32 a1, s32 a2) {
    s16 buf[4];
    u16 *tbl;
    s16 u, v;
    s32 *pkt;
    s32 *ot;

    a2 = D_800A36AC & 1;
    if (a2 != D_800A3234) {
        D_800A3378 = (s32)(&D_800A9830 + a2 * 240);
        D_800A3234 = a2;
    }
    if ((s32 *)D_800A3378 != (s32 *)(&D_800A9920 + D_800A3234 * 240)) {
        tbl = &D_80094AF4 + a1 * 6;
        buf[0] = *tbl++;
        buf[1] = *tbl++;
        buf[2] = *tbl++;
        buf[3] = *tbl++;
        u = *tbl++;
        v = *tbl;
        if (a0 != 0) {
            buf[0] = buf[0] + 0x80;
            u = u + 0x80;
        }
        SetDrawMove((s32)(s32 *)D_800A3378, buf, (s16)u, (s16)v);
        pkt = (s32 *)D_800A3378;
        ot = (s32 *)D_800A378C;
        *pkt = (*pkt & 0xFF000000) | (ot[0x3FFC / 4] & 0xFFFFFF);
        ot[0x3FFC / 4] = (ot[0x3FFC / 4] & 0xFF000000) | ((s32)pkt & 0xFFFFFF);
        D_800A3378 = (s32)(pkt + 6);
    }
}
