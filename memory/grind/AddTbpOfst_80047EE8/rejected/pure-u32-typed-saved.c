/* s2 REJECTED — pure spelling V5: type-narrow/re-type lever — saved typed
 * u32 instead of s32 (saved = (u32)p; second pointer p = (u32*)(saved +
 * ((v_off>>2)<<2)) in unsigned arithmetic). NO FAKE store, floor-10
 * chassis. sandbox --disable all = 11 (FAKE form = 10). SImode is SImode;
 * signedness does not perturb the cse2 equivalence class or the RA.
 * DEAD.
 */
