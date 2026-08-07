/* s2 REJECTED — pure spelling V6: statement re-association — second
 * pointer rebound explicitly from saved in two statements:
 *   p = (u32 *) saved;
 *   p = (u32 *) ((s32) p + (s32) ((v_off >> 2) << 2));
 * NO FAKE store, floor-10 chassis. sandbox --disable all = 11 (FAKE form
 * = 10). cse2 folds the rebind copy and still substitutes the canonical
 * register ($a0) into the add. DEAD.
 */
