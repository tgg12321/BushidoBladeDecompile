/* REJECTED s2: score 17 (baseline was 14 pre-B5). Loading the subtrahend
 * (arg0[1]) first perturbs the load schedule, not just the dest reg.
 *   s32 y0 = arg0[1];
 *   s32 y1 = arg1[1];
 *   s32 dy = y1 - y0;
 * Neutral variants (score unchanged): minuend-first named temps (y1,y0),
 * split-init dy = arg1[1]; dy -= arg0[1];  — named-temp splits along existing
 * expression boundaries are RTL-neutral (expand already makes those temps). */
