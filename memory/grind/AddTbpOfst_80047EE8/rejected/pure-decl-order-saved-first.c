/* s2 REJECTED — pure spelling V3: declaration-order swap (saved declared
 * before p), plain statement inits, NO FAKE store, on the floor-10 chassis.
 * sandbox --disable all = 11 (FAKE form = 10). Pseudo numbering is
 * first-USE LUID, not declaration order — same mechanism sibling s9 proved.
 * DEAD.
 *
 * Init chain (rest of body identical to candidate.c minus the FAKE line):
 *   s32 saved;
 *   u32 *p;
 *   ...
 *   p = (u32 *) arg0;
 *   saved = (s32) p;
 */
