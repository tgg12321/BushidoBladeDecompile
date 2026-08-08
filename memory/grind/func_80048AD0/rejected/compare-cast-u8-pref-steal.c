/* REJECTED — s2-permuter find output-35-1 (141 s into campaign s2-body-only)
 * plus the two hand-probe combinations, all measured 2026-08-07.
 *
 * Mutation: `if ((u8)sound == 0xFF)` (compare-site truncation cast) on the
 * honest score-1 reuse chassis. Effect measured with the workspace pipeline
 * (47/47 insns, zero ins/del):
 *   - The caller-side `andi $a0,$a0,0xff` DISAPPEARS: CSE unifies the
 *     call-site implicit truncation with the compare's (u8) temp, and the
 *     `and P 0xff` folds to nothing because P is lbu-loaded
 *     (nonzero_bits = 0xFF).
 *   - BUT the snd_LoadBgm argument copy now sources the QImode CSE temp, not
 *     `sound`'s pseudo, so set_preference/expand_preferences hand {$a0} to
 *     the dying temp instead of the reused counter. Result: counter=$a2,
 *     delta=$a0 — the classic separate-counter allocation, 7 register diffs
 *     (permuter weighted 35): move a2,zero / subu a0 / sh a2 / addiu a2 /
 *     slti v0,a2 / addiu a0,a0,1768 all flipped vs target.
 *
 * The andi-fold and the preference-loss are COUPLED through the same CSE
 * temp: any compare-site truncation spelling that folds the andi necessarily
 * re-routes the {$a0} pref away from the counter. Probe matrix:
 *   compare-cast only            -> 7 reg diffs (this file)
 *   call-cast `(u8)sound` only   -> andi remains (score-1-equivalent; no
 *                                   CSE partner at the compare)
 *   both casts                   -> identical to compare-cast only
 *
 * ALSO: both casts are semantically redundant (`sound` is lbu-loaded, its
 * value is always < 0x100, and the u8 prototype already truncates) — the
 * "redundant width casts (F2)" FORBIDDEN family. So even a variant of this
 * shape that somehow preserved the pref would be unusable without an owner
 * ruling. Dead on both axes. */
typedef int s32;
typedef unsigned char u8;
/* body = candidate.c score-1 form with the compare line replaced by:
 *   if ((u8)sound == 0xFF) return 0;
 * full permuter source: tmp/grind/func_80048AD0/s2/find_score35_compare_cast.c */
