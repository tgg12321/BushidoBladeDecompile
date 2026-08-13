/* func_800645B0 — REJECTED FAMILY (grind session 10, escalation modality).
 *
 * WHAT WAS TESTED.  The ledger's top live frontier item after session 9c:
 * "what semantic C construct denies sched.c's birthing_insn_p lift on the
 * loop-top `addu idx,i,j` at ZERO instruction cost?"  The lift fires because
 * reg_n_sets[idx] == 1, so the lever is a SECOND set of `idx`; every second
 * set INSIDE the if-body is already enumerated (it must be one of the three
 * values the target keeps in $s0, and all three are measured — sum = the SB
 * expand_binop wall, byte offset = DA 12/78, i+j again = the entry-copy fold).
 * The un-measured surface was second sets placed OUTSIDE the if-body.  Nine
 * placements were measured on the JD chassis (whose ENTIRE residual is the
 * three loop-top points), harness tmp/grind/func_800645B0/s10/sweep33.py:
 *
 *   YA  JD control                                              3 / 78
 *   YB  `idx = 1; D_800F10EC = idx;` before the loops (KD)      4 / 78
 *   YC  ordinary declaration initialiser `s32 idx = 0;`         3 / 78
 *   YD  `idx = 0;` as a statement before the loops              3 / 78
 *   YE  `idx = i;` at the outer-loop body TOP                   3 / 78
 *   YF  `idx = i;` at the outer-loop body TAIL                  3 / 78
 *   YG  outer for INIT clause `for (i = 0, idx = 0; ...)`       3 / 78
 *   YH  outer for UPDATE clause `i += 4, idx = i`               3 / 78
 *   YI  inner for UPDATE clause `j++, idx = j`                  3 / 78
 *   YJ  `idx = D_800A3444;` before the loops (a real load)      3 / 78
 *
 * WHY IT IS DEAD — a two-branch closure, both branches measured.
 *   (1) If the second set is not READ before `idx = i + j;` overwrites it, it
 *       is a dead store and flow.c deletes it before reg_n_sets is taken.  All
 *       eight such placements (YC/YD/YE/YF/YG/YH/YI/YJ) are BYTE-IDENTICAL to
 *       the control — not one instruction moved — including YJ, whose RHS is a
 *       genuine memory load and therefore cannot be constant-folded.  Deadness,
 *       not foldability, is the operative rule outside the if-body.
 *   (2) If the second set IS read (YB, the session-8 KD form), it survives and
 *       the loop top becomes EXACT — but `idx` is a multi-block pseudo in $s0
 *       and GCC 2.7.2 has no live-range splitting, so the value is materialised
 *       in $s0 before the loops, where the target writes $s0 not at all.  That
 *       costs the prologue points: 4 / 78, one WORSE than the control.
 *
 * There is therefore no zero-cost second set of `idx` outside the if-body, and
 * the inside-the-if-body enumeration was already complete.  Frontier item 1 is
 * KILLED.
 *
 * The body below is YB (the only variant that survives the fold), banked as
 * the representative of the family.  Do not re-propose any of the ten.
 */
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx;
    s32 mask;
    s32 val;
    s32 last;
    idx = 1;
    D_800F10EC = idx;
    for (i = 0; i < 0xF; i += 4) {
        for (j = 0; j < 4; j++) {
            idx = i + j;
            val = 1;
            mask = val << idx;
            if (!(D_800A3444 & mask)) {
                last = rand();
                *((s32 *)(((s32)(&D_800F0D78)) + ((((idx << 1) + idx)) << 2))) = (((s32 *)D_800A347C)[0] + (last & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + ((((idx << 1) + idx)) << 2))) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + ((((idx << 1) + idx)) << 2))) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
                last = rand();
                val = D_800A3444;
                *((s16 *)(((s32)(&D_800F0BCC)) + (idx << 1))) = last & 7;
                val = val | mask;
                D_800A3444 = val;
                break;
            }
        }
    }
    return 1;
}
