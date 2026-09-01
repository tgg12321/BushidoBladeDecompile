/* REJECTED s1 (2026-09-01): diagnostic-only form, scored 28 (worse than the
 * d15 candidate) AND the `i+=3; i-=1; i-=1;` shape is the banned `+=2/-=1`
 * respelling class (no-new-park-categories F6 entry sanctions ONLY the exact
 * adjacent `i++; i--;` cancellation pair, and the Judge FAILed the +=2/-=1
 * respelling — decisions.md:1833 lineage). Purpose of the probe was to push
 * loop.c insn_count to 59; result: the 0x91A2B3C5 movable was STILL moved,
 * killing the "threshold is 58 (soft-float)" theory — threshold is 122.
 * Never submit any variant of this form.
 */
/* (body = candidate.c with `i += 1;` replaced by `i += 3; i -= 1; i -= 1;`) */
