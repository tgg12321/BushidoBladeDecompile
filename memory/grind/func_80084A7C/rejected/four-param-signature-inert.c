/* REJECTED (session 3) — declaring the signature the three call sites actually
 * use.  main.c:451,503,547 call this through a (void (*)(s16,s16,u8,u8 *)) cast
 * because it is one slot of the D_800F3340 dispatch table, so the honest
 * question was whether declaring the extra two parameters (making incoming
 * $a2/$a3 live-in) changes the allocation.
 *
 * Measured: BYTE-IDENTICAL to the 2-parameter form (cc1-only golddiff 45 == the
 * 2-param baseline, 124 insns, same `move $5,$7` join copy, same $a2/$a3 swap).
 * Unused parameters never reach a live range here, so the prototype question is
 * codegen-INERT for this function.  Confirms session 1's note that the
 * prototype contradiction is not part of the residual — do not spend a session
 * on it.
 */
void func_80084A7C(s16 a0, s16 a1, u8 a2, u8 *a3) { /* body as floor24 */ }
