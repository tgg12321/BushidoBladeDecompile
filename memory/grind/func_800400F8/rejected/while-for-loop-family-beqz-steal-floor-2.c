/* REJECTED-as-match s2 (2026-07-14): the natural while-loop spelling (and
 * every for/while ordering variant: s1-first, s0-first, for, comma-for x2 —
 * ALL score exactly 2 with 28 insns). This family DOES reproduce the target's
 * 0x28 frame (the loop condition's folded variable-compare leaves the
 * combine USE pseudo -> phantom slot sp+20) AND the correct register
 * allocation ($s0=counter in the blez delay slot, $s1=pointer).
 * The irreducible 2: reorg steals `move s1,s2` into the beqz delay slot
 * (target: nop there, `addu s1,s2` after blez). Mechanism (measured via
 * cc1 -da dumps, tmp/grind/func_800400F8/s2/cc1/): GCC 2.7.2 jump1
 * duplicate_loop_exit_test inserts the guard AFTER all pre-loop inits; the
 * guard lh is may_trap_p (only safe because beqz guards s2!=0), so reorg's
 * fall-through scan stops at it — but any trap-free init move sitting before
 * the lh IS stolen. Every while/for spelling must init s1 before the loop
 * => before the duplicated guard => stolen. Structurally unreachable; not a
 * cheat issue.
 */
void func_800400F8(s32 *a0) {
    s16 *s2;
    s16 *s1;
    s32 s0;
    s2 = (s16 *)a0[9];
    if (s2 != 0) {
        s1 = s2;
        s0 = 0;
        while (s0 < s2[0]) {
            obj_Clear(s1[4]);
            s1 = (s16 *)((s32)s1 + 0xD0);
            s0++;
        }
    }
}
