/* SUPERSEDED (s2, 2026-07-13) -- the form currently committed on main.
 *
 *   switch ((s16)(*(u16 *)&D_800A36A4 - 2)) { ... }
 *
 * Reaches the target bytes, but the re-audit patrol flagged it as the
 * alias-rename-injection family spelled as a use-site type-pun (a pointer
 * re-view of an s16-declared global purely to force lhu).
 *
 * Superseded by memory/grind/func_80033498/candidate.c, which reaches the
 * IDENTICAL bytes (sandbox --disable all == 0, objdump-confirmed lhu + matching
 * registers) with NO pointer pun -- an ordinary value view plus an s16 index
 * local:  `s16 idx = (u16)D_800A36A4 - 2; switch (idx)`.
 *
 * Kept here because the pointer-pun spelling is strictly worse than the
 * candidate at equal bytes: it takes the address of the global and re-reads it
 * through a different pointer type, which is the specific shape the audit
 * objects to. If the pending ruling sanctions the width-view at all, take the
 * candidate spelling; if it forbids the width-view, BOTH forms die together
 * (the (u16) view is semantically inert in every spelling -- see
 * tmp/grind/func_80033498/s2/identity_proof.py).
 */
