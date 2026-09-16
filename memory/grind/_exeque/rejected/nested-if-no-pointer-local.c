/* REJECTED s11 (rederive) — nested if/if structure (matches candidate.c's
 * two-level control flow) but with the pointer local `s32 *p = &D_8009BE7C;`
 * removed, reading/writing D_8009BE7C directly:
 *
 *   if (D_8009BF78 == D_8009BF7C && !(*D_8009BF54 & 0x01000000)) {
 *       if (D_8009BE7C != 0 && D_8009BE80 != 0) {
 *           D_8009BE7C = 0;
 *           ((s32 (*)(void))D_8009BE80)();
 *       }
 *   }
 *
 * Measured on the s4-s10 do-while(0)-wrapped floor-2/187 chassis, this
 * final block substituted for the pointer-local form: sandbox score
 * 5/187 (WORSE than the banked 2/187). Confirms the pointer-local `p`
 * indirection (S2's H5b) is itself load-bearing for the floor-2 result,
 * not an inert stylistic choice. KILLED instance.
 */
