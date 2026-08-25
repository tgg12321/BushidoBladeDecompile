/* REJECTED — s1 (2026-08-25): s16-typed loop counter to block biv elimination via the
 * giv `v->mode == mode` eligibility check (loop.c:6131).  Measured sandbox 74
 * (build 159 vs target 141) — HImode counter injects sll/sra sign-extensions at every
 * use; MIPS PROMOTE_MODE does not rescue it.  Do not re-propose narrow counter types.
 * Only the declaration differed from candidate.c:
 *     s16 i;            // instead of s32 i, with `} while (i < 2);`
 */
