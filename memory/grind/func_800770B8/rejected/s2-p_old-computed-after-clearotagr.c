/* REJECTED (s2, 2026-09-01) — floor 13 from a 12 chassis (net worse, but INFORMATIVE).
 * Moving `p_old = (s32 *)(arg0 + 0x58);` from before ClearOTagR to after it DOES
 * sink the `addiu $17,$16,88` out of prologue slot 8 (to slot 11, target has it at 12) —
 * i.e. sched1's block-0 placement of that insn follows C statement order / LUID.
 * But it costs more than it buys: the freed slot is taken by `lui $a0,%hi(D_800A374C)`
 * instead of target's `sw $ra`, so the two prologue register saves (sw $ra / sw $s1)
 * and the ClearOTagR arg order (target evaluates a1=0x1008 BEFORE the a0 global load;
 * ours does a0 first) drift further apart — the positional mismatch grows from 4 rows
 * to 6. The real class-A residual is the a1-first arg evaluation, not the addiu slot.
 */
    sp[0] = 0;
    sp[1] = 0;
    ClearOTagR(D_800A374C, 0x1008);
    p_old = (s32 *)(arg0 + 0x58);
