/* motion_SetMotion (code6cac_c_mid.c) — WIP NOTE, not a full body.
 *
 * The HEAD body is large (402 insns) and CORRECT in structure. The ONLY
 * change toward COMPLETED-C is removing the two forbidden register pins:
 *
 *   -    register s32 result asm("s3") = 0;
 *   -    register s32 sel2   asm("s2") = -1;
 *   +    s32 result = 0;
 *   +    s32 sel2   = -1;
 *
 * Keep the rest of the HEAD body verbatim (do NOT change case 9/11's
 * `sel = 0xC` — see notes; the regfix subst is an anti-CROSS-JUMP fix, not a
 * value correction; setting it to 0xD breaks the full build).
 *
 * With pins removed, GCC flips the allocation to result->s2, sel2->s3 (target
 * wants result->s3, sel2->s2). That swap is MASKED by the sandbox (scores 0
 * for it) so only the full-build SHA1 sees it — `retire` rolls back.
 *
 * STILL BLOCKING (both must be solved in pure C, coupled):
 *   1. RA: force result->$s3, sel2->$s2 without pins.
 *   2. cross-jump: retire the regfix subst (anti-cross-jump li 12->13 @149).
 */
