/* s2 REJECTED — pure spelling V4: split-init reversal binding p FROM saved:
 *   saved = arg0;
 *   p = (u32 *) saved;
 * NO FAKE store, floor-10 chassis. sandbox --disable all = 11 (FAKE form
 * = 10). cse2 places {arg0, saved, p} in one equivalence class regardless
 * of which C name the copies flow through; the second-pointer add still
 * binds $a0. Same mechanism sibling killed on its body (split-init probe).
 * DEAD.
 */
