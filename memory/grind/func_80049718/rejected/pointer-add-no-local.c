/* REJECTED — func_80049718 prologue address computation.
 * Bare pointer-add spelling WITHOUT a named-intermediate local:
 *     p_anim = D_800EF980 + arg0;
 * Measured floor 11 (WORSE than &D_800EF980[arg0]'s floor, and far worse than
 * the floor-0 named-local form). Do NOT re-propose.
 *
 * WHY it's worse: without staging the base into a named pseudo, GCC's RTL still
 * births the index computation first; the pointer-add form additionally
 * perturbs downstream register allocation. The WINNING form stages the base:
 *     s16 *tbl = D_800EF980;
 *     p_anim = tbl + arg0;         // floor 0 (base pseudo born first)
 * See candidate.c / hypotheses.md Cluster A.
 *
 * Also rejected (prior session): (s16*)((s32)&D_800EF980[0] + arg0*2) -> no
 * change (stayed at floor 6; addu operand order did not flip). */
