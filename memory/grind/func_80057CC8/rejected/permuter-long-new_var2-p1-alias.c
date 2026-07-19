/* REJECTED — permuter-found score-0 close for func_80057CC8 (s4, 2026-07-19).
 *
 * Lever the permuter found: introduce `long new_var2` as a NAMED
 * INTERMEDIATE carrying the p1 pointer address expression, then cast
 * to s16* at the assignment to `p`. This shifts the p1 addu dest pseudo's
 * copy-pref / live-range boundary so RA picks v0 (matching target) instead
 * of v1.
 *
 * WHY REJECTED — same cheat class as rejected/block-scope-alias-p1.c
 * (2026-06-16 layer-1 FAIL, 2026-06-22 layer-2 FAIL): a local whose sole
 * purpose is to shape the pointer's pseudo lifetime so RA coalesces
 * differently. Fails the cheat-by-any-spelling checklist:
 *
 *   1. No semantic purpose: `new_var2` (permuter-generated name, no role)
 *      is declared as `long` (integer holder for a pointer address);
 *      the direct `p = (s16*)(<expr>)` is the natural spelling.
 *   2. Justification is GCC-internals only: the ONLY effect of the
 *      intermediate is to change the p1 addu dest pseudo's copy-pref
 *      class (per [[hoist-shared-arm-computation-defeats-copy-pref]]);
 *      no observable behaviour change.
 *   3. A human programmer writing this function from a spec would not
 *      introduce a `long` holder for an s16* address computation.
 *   4. The named-local-fake-exception carve-out (2026-07-01) covers
 *      constant-holder / dead SCALAR locals with FAKE annotation +
 *      layer-2. This form is a pointer-address alias for lifetime
 *      shaping, not a constant holder; the annotation would be a
 *      lifetime-shaping justification, precisely what [[no-new-park-
 *      categories]] "cheats by any spelling" forbids.
 *
 * Companion permuter finding: score 15 = declaration reorder only (no
 * improvement over floor 3); score 25/35 = pointer-alias/cx-alias
 * variants (same cheat class as this find); score 30 = `scale = (s32)table`
 * variable-reuse (would need FAKE + layer-2 under named-local-fake-
 * exception, and still masked-15).
 *
 * The permuter's random+PERM_* exploration from the candidate-floor-3
 * baseline yielded ONE score-0 form, and it is a cheat-class alias. That
 * eliminates "directed permuter finds a novel legitimate lever" from the
 * frontier — the RA basin can be closed only via lifetime shaping in the
 * search space the permuter enumerates.
 */
void func_80057CC8(u8 *arg0, s32 arg1, s16 *arg2, s16 *arg3)
{
  unsigned short prev_idx;
  unsigned short next_idx;
  s32 ang_prev;
  s32 ang_next;
  s32 ang_mid;
  long new_var2;  /* CHEAT: pointer-address holder for p1 RA shaping */
  s32 scale;
  s32 base;
  s32 half;
  u16 cx;
  s16 new_var;
  s16 *p;
  u16 cy;
  s16 *table;
  prev_idx = arg1 - 1;
  table = *((s16 **) (arg0 + 4));
  cx = *((u16 *) ((((s32) table) + (arg1 * 4)) + 0));
  cy = *((u16 *) ((((s32) table) + (arg1 * 4)) + 2));
  if (((s16) prev_idx) < 0) {
    prev_idx = arg0[3] - 1;
  }
  {
    s32 tmp = arg1 + 1;
    next_idx = tmp;
    if (((s16) tmp) >= ((s32) arg0[3])) {
      next_idx = 0;
    }
  }
  new_var2 = ((((s32) (prev_idx << 16)) >> 16) << 2) + ((s32) table);
  p = (s16 *) new_var2;
  ang_prev = single_game_getEnemyCharId(p[0] - ((s16) cx), p[1] - ((s16) cy)) & 0xFFF;
  p = (s16 *) (((((s32) (next_idx << 16)) >> 16) << 2) + ((s32) (*((s16 **) (arg0 + 4)))));
  ang_next = single_game_getEnemyCharId(p[0] - ((s16) cx), p[1] - ((s16) cy)) & 0xFFF;
  if (ang_next < ang_prev) {
    base = ang_prev + 0x800;
    half = ((s32) (ang_prev - ang_next)) / 2;
    ang_mid = base - half;
  } else {
    ang_mid = (((s32) (ang_next - ang_prev)) / 2) + ang_prev;
  }
  scale = arg0[2] * 40;
  *arg2 = cx + ((scale * ((s32) (*((&Judge) + (ang_mid & 0xFFF))))) >> 12);
  *arg3 = cy + ((scale * ((s32) (new_var = *((&Judge) + ((((s16) ang_mid) + 0x400) & 0xFFF))))) >> 12);
}
