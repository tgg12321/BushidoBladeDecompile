/* REJECTED (s8, 2026-08-25) — measured DEAD at honest floor 1, superseded by
 * the matched candidate.c.
 *
 * These are the three "almost right" heads: they use the scaled index, but the
 * pointer add is expanded in ASSIGNMENT context (or through a named shift
 * local), so GCC 2.7.2's index-first canonicalization never fires and idx25
 * stays base-first (`addu $18,$16,$2` vs target `addu $18,$2,$16`).
 *
 *   B2  s2 = (s32 *)((u8 *)a0 + s1 * 2);            -> score 1
 *   D   s2 = (s32 *)&((s16 *)a0)[s1];               -> score 1
 *   E   v0 = s1 << 1;
 *       s3 = *(s16 *)((u8 *)a0 + v0 + 0x332);       -> score 1
 *
 * Why: expr.c PLUS_EXPR only reaches the `both_summands` canonicalization
 * (expr.c:5289, "put a multiplication first") when the `+` is expanded with
 * modifier EXPAND_SUM — i.e. when it is a MEM address, not an assignment RHS
 * (expr.c:5238 `goto binop`) — AND when the index operand is still a MULT rtx
 * (expr.c:5362 MULT_EXPR indexed-address path), which a named `v0 = s1 << 1`
 * local destroys. B2/D fail the first condition, E the second.
 *
 * The matched form (candidate.c) satisfies both:
 *     s3 = *(s16 *)((u8 *)a0 + s1 * 2 + 0x332);
 *
 * Full measurement table: tmp/grind/func_800307D0/s8/variant_matrix.md.
 */
