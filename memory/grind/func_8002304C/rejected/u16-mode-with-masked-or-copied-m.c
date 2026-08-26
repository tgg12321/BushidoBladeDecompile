/* REJECTED SPELLINGS for the mode/m block (the andi residual) — s1, 2026-08-26.
 * Both measured on the otherwise-floor-1 chassis (216-insn target, everything
 * else matched). Both score 3 (215/216 insns): the mask/copy insn VANISHES
 * entirely (combine folds zero_extend of the u16-typed load through to every
 * consumer — one register serves both m and mode, so target's separate
 * $v1 copy AND its consumers' register assignment go wrong).
 *
 * Variant A (score 3):
 *   u16 mode = *((u16 *) (obj + 0x6A));
 *   s32 m = mode & 0xFFFF;
 *
 * Variant B (score 3):
 *   u16 mode = *((u16 *) (obj + 0x6A));
 *   u16 m = mode;
 *
 * KEPT (floor 1) — the only spelling of the three that materializes a distinct
 * $v1 insn (an addu copy; target has andi):
 *   s32 mode = *((u16 *) (obj + 0x6A));
 *   u16 m = mode;
 *
 * Variant C (score 25, 218/216 insns — address-taken local homed to stack,
 * adds sw + reload):
 *   s32 mode = *((u16 *) (obj + 0x6A));
 *   u16 m = *(u16 *)&mode;
 *
 * Do NOT re-propose A/B/C. The wide->narrow (s32 mode, u16 m) direction is the
 * one that keeps two pseudos alive; the narrow->wide direction lets combine
 * collapse them; address-taking spills.
 */

/* ---------------------------------------------------------------------------
 * s2 ADDENDUM (2026-08-26) — this file's closing advice was WRONG about the
 * remaining quadrant, and the function is now MATCHED.
 *
 * The four s1 variants span only three of the four (mode-width x m-width)
 * quadrants: A = (u16, s32-masked), B = (u16, u16), KEPT = (s32, u16),
 * C = (s32, address-taken). The fourth — **(s32 mode, s32 m = mode & 0xFFFF)**,
 * i.e. BOTH operands wide with an explicit mask — was never measured. It
 * scores **0** (216/216 insns, full verify-oracle SHA1 match) and is the
 * original author's own idiom: func_80023E40 in this same TU
 * (src/code6cac.c:2545-2546, COMPLETED-C at 6d255e79) reads the identical
 * field obj+0x6A with exactly that two-line spelling.
 *
 * Mechanism (why the wide/wide quadrant is the one that works): combine
 * refuses to substitute `reg = zero_extend:SI(mem:HI)` into
 * `reg2 = and:SI(reg, 65535)` while `reg` stays live afterwards (`mode - 0x17`
 * reads it), so the standalone andsi3 survives as `andi $v1,$a0,0xffff`.
 * Narrowing EITHER side (variants A/B) creates a truncate/extend pair that an
 * earlier pass collapses before liveness can protect it. A/B/C remain
 * correctly rejected; only the "wide->narrow is the right direction"
 * conclusion in the note above is superseded.
 * ------------------------------------------------------------------------- */
