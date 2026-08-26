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
