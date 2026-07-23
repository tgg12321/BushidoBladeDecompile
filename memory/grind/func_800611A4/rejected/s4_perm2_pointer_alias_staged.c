/* REJECTED — s4 permuter (interleaved-V0 seed) output-40-1. sandbox=7, 43/43.
 * WORSE than the V7 floor of 6 AND cheat-shaped.
 *
 * WHY DEAD: two fresh single-use codegen-coercion locals with no semantic
 * purpose: `s32 *new_var2 = &arg0[1]; t = *new_var2;` (a pointer alias into
 * the middle of the array to compute one element via a separate base) and
 * `s32 new_var3 = arg0[0]; t = new_var3;` (a staged intermediate). A human
 * would write `t = arg0[1];` / `t = arg0[0];` directly. Fails no-new-park-
 * categories tests 2/3/4. Mechanistically it flips ONLY load 1 to $v0 while
 * loads 2/3 stay $v1 and mask stays $v0 — strictly worse than V7. Not vetted
 * to reviewer (self-rejected: worse-than-floor + obvious coercion).
 */
void func_800611A4(s32 *arg0, s32 *arg1) {
    u16 sp[3];
    s32 new_var3;
    u16 new_var;
    s32 t;
    s32 *v1 = (s32 *) (&D_800F116C);
    s32 *new_var2;
    sp[0] = *((u16 *) (((s32) arg1) + 0));
    sp[1] = *((u16 *) (((s32) arg1) + 2));
    D_800A3468 = (s32) v1;
    new_var = *((u16 *) (((s32) arg1) + 4));
    D_800F117C = (s32) (&sp[0]);
    D_800F1178 = (s32) arg0;
    D_800F1180 = (s32) (&D_800F116A);
    *v1 = 0x21001A;
    sp[2] = new_var;
    func_80060A68();
    new_var2 = &arg0[1];
    new_var3 = arg0[0];
    t = new_var3;
    D_800F1140 = t;
    t = *new_var2;
    D_800F1144 = t;
    D_800A3464 = 0xFFFFEF;
    t = arg0[2];
    D_800F1148 = t;
}
