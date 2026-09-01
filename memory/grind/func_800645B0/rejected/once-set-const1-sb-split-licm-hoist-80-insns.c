/* SESSION 16 (2026-09-01, forensics) - KILLED, mechanism dump-proven.
 *
 * FAMILY-LEVEL KILL: making the const-1 carrier `val` a ONCE-SET local -- by any
 * spelling -- is fatal on this function, because reg_n_sets[val] == 1 is
 * simultaneously (a) the precondition for sched.c birthing_insn_p's max-priority
 * lift (the thing we WANT, so the const-1 `li` ties with the loop-top `addu` and
 * INSN_LUID/statement order decides the loop head) and (b) the precondition for
 * loop.c's invariant hoist of that same set (`n_times_set == 1` + invariant).
 * GCC 2.7.2 reads both from the same counter, so the lift cannot be bought
 * without the hoist.  Dump proof (tmp/grind/func_800645B0/s15b/e.loop.txt):
 *   "Insn 41: regno 78 (life 1), move-insn savings 1  moved to 188"   (inner loop)
 *   "Insn 188: regno 78 (life 52), ... halved since already moved  moved to 190"
 *   and insn 190 `(set (reg/v:SI 78) (const_int 1))` sits BEFORE the outer
 *   NOTE_INSN_LOOP_BEG (insn 15) -- i.e. hoisted out of BOTH loops.
 * Cost: the hoisted constant is then live across the inner loop's `jal rand`,
 * so RA must give it a CALLEE-SAVED register ($20), adding `sw $20,32($sp)` +
 * `lw $20,32($sp)` = +2 insns (.frame regs=6 vs the target's 5).  The target
 * keeps `addiu $v1,$zero,0x1` INSIDE the inner loop (asm/funcs/func_800645B0.s:16).
 * This is why the SB chassis' reuse of `val` for the D_800A3444 RMW is
 * load-bearing: it is a LICM defeat, not only a scheduling choice.
 *
 * Measured this session (honest `sandbox func_800645B0 --disable all`, floor
 * re-confirmed 1/78 on the SB chassis the same day):
 *   A  SB chassis + fresh `flags` local for the RMW ....... 13 / 78 @ 80 insns
 *   B  WD chassis + fresh `flags` local for the RMW ....... 12 / 78 @ 80 insns
 *   C  WD chassis + `mask` reused as the RMW carrier ...... 14 / 78 @ 77 insns
 *   E  WD chassis + `idx` reused as the RMW carrier ....... 15 / 78 @ 80 insns
 * (E is the shape that also restores the target's loop-head/delay-slot
 * structure -- `bne $2,$0,.L563 ; addu $16,$19,$3` -- and still loses on the
 * +2 hoist cost.  C is the only form ever measured at 77 insns, one SHORT of
 * the target's 78.)
 */
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx;
    s32 idx2;
    s32 mask;
    s32 val;
    s32 flags;
    s32 last;
    D_800F10EC = 1;
    for (i = 0; i < 0xF; i += 4) {
        for (j = 0; j < 4; j++) {
            idx = i + j;
            val = 1;
            mask = val << idx;
            if (!(D_800A3444 & mask)) {
                idx2 = idx << 1;
                last = rand();
                idx = idx2 + idx;
                *((s32 *)(((s32)(&D_800F0D78)) + (idx << 2))) = (((s32 *)D_800A347C)[0] + (last & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + (idx << 2))) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + (idx << 2))) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
                last = rand();
                flags = D_800A3444;
                *((s16 *)(((s32)(&D_800F0BCC)) + idx2)) = last & 7;
                flags = flags | mask;
                D_800A3444 = flags;
                break;
            }
        }
    }
    return 1;
}
