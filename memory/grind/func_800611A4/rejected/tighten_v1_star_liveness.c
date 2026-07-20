/* REJECTED s2 — Variant D: keep alias, move `*v1 = 0x21001A;` immediately
 * after `D_800A3468 = (s32) v1;` (tighten liveness).
 * Sandbox --disable all = 20 (much worse). Same 43 insns, pure ordering
 * shift, but reorders the pre-call cluster so the sp[0]/sp[1]/sp[2] and
 * the arg1 halfword-load chain interleave badly. Killed: source ordering
 * around the alias write is load-bearing for pre-call schedule; the target
 * shape needs the *v1 store immediately BEFORE the call, not tightly
 * paired with the D_800A3468 assignment. */
void func_800611A4(s32 *arg0, s32 *arg1) {
    u16 sp[3];
    u16 new_var;
    s32 t;
    s32 mask;
    s32 *v1 = (s32 *) (&D_800F116C);
    sp[0] = *((u16 *) (((s32) arg1) + 0));
    sp[1] = *((u16 *) (((s32) arg1) + 2));
    D_800A3468 = (s32) v1;
    *v1 = 0x21001A;
    new_var = *((u16 *) (((s32) arg1) + 4));
    D_800F117C = (s32) (&sp[0]);
    D_800F1178 = (s32) arg0;
    D_800F1180 = (s32) (&D_800F116A);
    sp[2] = new_var;
    func_80060A68();
    t = arg0[0];
    D_800F1140 = t;
    t = arg0[1];
    D_800F1144 = t;
    mask = 0xFFFFEF;
    D_800A3464 = mask;
    t = arg0[2];
    D_800F1148 = t;
}
