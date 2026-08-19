/* REJECTED FAMILY (s4, 2026-08-08): permuter's dominant basin across all three
 * s4 campaigns (weighted 55-100). EVERY member stages x (or a synthetic local)
 * in ONE dispatch arm and reads it UNINITIALIZED in the other:
 *
 *   narrow: lo = (lo = x) & 0x3FF;   wide: lo = lo & 0xFFF;        (65/85-weighted)
 *   narrow: pkt = x; lo = pkt & 0x3FF;   wide: lo = pkt & 0xFFF;   (75/95-weighted)
 *   narrow: new_var = x; ...             wide: lo = new_var & 0xFFF;
 *
 * FORBIDDEN: undefined behavior (uninit read) + dead-read coercion family —
 * same class as the s0-era rejected score-175-1 / round-13 S2. Never honest-
 * measured (cheat status obvious).
 *
 * DIAGNOSTIC VALUE (why it keeps winning the weighted metric): the uninit read
 * makes the staging pseudo live INTO the wide arm, i.e. live across the
 * dispatch branch — the conflict that pushes the remaining renames toward
 * target. Legit spellings of the same intent all measured and all fail:
 *   duplicated pkt=x into BOTH arms (sanctioned dup-into-arms shape)  -> 13
 *   unconditional lo = x pre-dispatch, arms mask lo                   -> 20 (52 insns)
 *   lo = x; lo = lo & 0x3FF; split-assign inside narrow arm           -> 14 (inert)
 * So the "second x-pseudo live across dispatch" conflict is NOT reachable by
 * any measured legitimate spelling; the campaign found no other basin.
 */
