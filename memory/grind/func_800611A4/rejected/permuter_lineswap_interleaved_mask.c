/* Rejected — permuter output-50-1 (best of 10784 iters, permuter_score=50)
 * Sandbox --disable all == 9 (WORSE than V7 baseline 6).
 *
 * Structural move: mask (D_800A3464 = 0xFFFFEF) shifted from atomic-first
 * (V7 shape, sandbox=6) to interleaved between load 1 and load 2.
 * Also adds `new_var2 = &D_800F116A;` intermediate for D_800F116A address.
 *
 * Why it fails: this is the same "mask interleaved with loads" shape that
 * s1/s2/s3 already banked as hitting the 9-wall (v0<->v1 rename swap) for
 * every mask-interleave position other than V4/V7's atomic-first.
 * The 4 other lowest-scoring permuter finds (output-50-2, 55-1, 60-1, 64-1)
 * all share the same interleave move with alias variations.
 *
 * Kill: PERM_LINESWAP + PERM_RANDOMIZE random-mutation basin from V7 has
 * no gradient down to sandbox<6 in 10784 iters (best find is sandbox=9).
 */
extern u8 D_800F116A;
extern s32 D_800F116C;
extern s32 D_800A3464;
extern s32 D_800A3468;
void func_800611A4(s32 *arg0, s32 *arg1) {
    u16 sp[3];
    u16 new_var;
    s32 t;
    u8 *new_var2;
    s32 *v1 = (s32 *) (&D_800F116C);
    sp[0] = *((u16 *) (((s32) arg1) + 0));
    sp[1] = *((u16 *) (((s32) arg1) + 2));
    D_800A3468 = (s32) v1;
    new_var2 = &D_800F116A;
    new_var = *((u16 *) (((s32) arg1) + 4));
    D_800F117C = (s32) (&sp[0]);
    D_800F1178 = (s32) arg0;
    D_800F1180 = (s32) new_var2;
    *v1 = 0x21001A;
    sp[2] = new_var;
    func_80060A68();
    t = arg0[0];
    D_800F1140 = t;
    D_800A3464 = 0xFFFFEF;
    t = arg0[1];
    D_800F1144 = t;
    t = arg0[2];
    D_800F1148 = t;
}
