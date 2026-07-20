/* s3 V1 — REJECTED score=20 build_insns=43. Dropping the `new_var` local
 * and inlining `sp[2] = *(u16*)(arg1+4);` before the call badly perturbs
 * pre-call scheduling: the arg1[2] halfword load no longer lands in $a0,
 * the JAL delay slot cannot be filled with the sh, and the whole pre-call
 * cluster reorders. Baseline pin-free = 9; this = 20. */
void func_800611A4_v1(s32 *arg0, s32 *arg1) {
    u16 sp[3];
    s32 t;
    s32 mask;
    s32 *v1 = (s32 *) (&D_800F116C);
    sp[0] = *((u16 *) (((s32) arg1) + 0));
    sp[1] = *((u16 *) (((s32) arg1) + 2));
    D_800A3468 = (s32) v1;
    D_800F117C = (s32) (&sp[0]);
    D_800F1178 = (s32) arg0;
    D_800F1180 = (s32) (&D_800F116A);
    *v1 = 0x21001A;
    sp[2] = *((u16 *) (((s32) arg1) + 4));   /* the change */
    func_80060A68();
    /* ... same tail as candidate ... */
}
