/* REJECTED spelling — measured FLAT (still 35) in s1 2026-08-25.
 *
 * Hypothesis was that `&s0[s3]` (address-of-index) would expand the scaled
 * index addition offset-first (`addu dest, offset, base`) as target has at the
 * four rule sites. Measured: GCC 2.7.2 expands &a[i] with the POINTER as
 * operand 0 — identical base-first output to the rule-era
 * `(s32 *)((u8 *)s0 + (s3 << 2))` cast spelling. Do not re-propose.
 *
 * The spelling that DOES emit offset-first (confirmed, in candidate.c):
 *     (s32 *)((s3 << 2) + (s32)s0)
 * Note rvalue indexing `p[s3]` (a load, not an address into a variable)
 * already emits offset-first and needs no change.
 */
s32 *a0_ptr = &s0[s3];              /* -> addu a0, s0, a1  (base-first, WRONG) */
