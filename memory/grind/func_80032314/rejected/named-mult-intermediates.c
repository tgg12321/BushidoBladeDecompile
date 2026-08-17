/* REJECTED — func_80032314, session 1, H3.
 * Why dead: naming the three squared terms as separate locals does NOT change
 * which mflo result gets $a1 vs $a2. GCC coalesces the named temps straight
 * back into the same three pseudos; measured floor 15 (identical to the
 * baseline 15 form), same 3-cycle rotation, same 109 insns.
 * Do not re-propose named intermediates for the dx/dy/dz mult cluster.
 *
 * The only delta from candidate.c was replacing
 *     register u32 dist_sq asm("a0") = (u32)(dx * dx + dy * dy + dz * dz);
 * with
 *     s32 dxs = dx * dx;
 *     s32 dys = dy * dy;
 *     s32 dzs = dz * dz;
 *     register u32 dist_sq asm("a0") = (u32)(dxs + dys + dzs);
 * everything else identical to memory/grind/func_80032314/candidate.c.
 */
