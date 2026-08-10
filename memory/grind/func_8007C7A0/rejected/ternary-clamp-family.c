/* REJECTED (s7, 2026-08-10): the CLAMP-macro/ternary family on the s6
 * per-arm-return chassis. Hypothesis was that GCC 2.7.2's COND_EXPR
 * expansion would materialize target's three-arm $v0 join temp + `move
 * a3,v0` writeback when the assignment target is read inside an arm
 * (safe_from_p forcing a temp). Measured: it does NOT — expr.c uses the
 * assignment target directly (a read inside an arm does not clobber, so
 * safe_from_p passes) and the arms write $a3 directly, same as the if-forms.
 *
 *   V-T1  s16 x = arg0; x = (arg0>=0) ? ((D-1<arg0) ? D-1 : x) : 0;
 *         -> sandbox 5, build 50 (identical asm to the if-form)
 *   V-T2  arg0 = (arg0>=0) ? ((D-1<arg0) ? D-1 : arg0) : 0;
 *         -> sandbox 9, build 50 (worse: param-target expansion shifts regs)
 *
 * Also inert on this chassis (measured s7, same 5/50 asm as candidate):
 *   - in-range-arm self-assign `arg0 = arg0;` (dead-store sanctioned family)
 *     — elided before RTL, produces zero extra insns;
 *   - K&R (old-style) definition of the candidate body — byte-identical
 *     score/asm, re-killing s1's H1 on the new chassis.
 */
