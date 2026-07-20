/* s2 probe 1 — REJECTED, score 7 (baseline 2).
 * arg1 (s32) double-set via arm-1 reload alone:
 *     arg1 = D_800A3468;
 *     *(s32 *)(arg1 + 0x14) = (s32)(D_800F1164 + 2);
 *     *(s32 *)arg1 = 0x10016;
 * SCHEDULE became PERFECT (copy at slot 3, every position matches target) but
 * RA broke: local_alloc runs before global_alloc; with the reload removed from
 * arm-1's local pool, the 0x10016 const temp (block-local) grabbed the vacated
 * $v1; global pseudo(arg1) then found v1 blocked -> $a3; 0x10016 in $v1 vs
 * target $a0. 7 = pure register-substitution cluster.
 * ALLOCDBG: 91(D+2 ptr)=10909->v1, arg1=4761->a3.
 * Fix that worked: ALSO globalize the const through the sp20[2] carrier
 * (see candidate.c). Kill lesson: absorbing a block-local temp into a global
 * pseudo always vacates its register to the remaining locals first.
 */
