/* REJECTED (floor 9, no improvement). Hypothesis: place `mask = 0x10FFFF;`
 * BEFORE the func_80060A68() call so mask's live range crosses the call; the
 * call clobbers v0, which would force mask's allocno off v0 -> into v1, freeing
 * v0 for the load-temp t (= target allocation).
 *
 * WHY IT FAILED (measured, disasm-confirmed): 0x10FFFF is a pure constant, so
 * GCC does NOT keep it live across the call — it rematerializes (lui+ori) the
 * constant AFTER the call, right before its store. mask never actually crosses
 * the call, so the allocation is unchanged: tail still `lw v1,0(s0)` (t->v1),
 * mask->v0. Floor stays 9. Corollary: any source reordering of a *constant*
 * mask assignment is inert (the scheduler pins the const's live range to just
 * before its single use). This is why the prior WIP's 3 mask-move variants were
 * all 9 too. The lever must act on `t`'s live range, not the mask's placement. */
void func_80061658(s32 *arg0, s32 arg1) {
    s32 *v1 = (s32 *)&D_800F116C;
    s32 t;
    s32 mask;
    u8 *p;
    s32 val;
    D_800A3468 = (s32)v1;
    D_800F1178 = (s32)arg0;
    switch (arg1) {
    case 0:
        val = 0x21000C; p = &D_800F115C; *p = 0;
        D_800F1180 = (s32)p; *v1 = val; break;
    case 1:
        val = 0x21000D; p = &D_800F115C + 1; *p = 0;
        D_800F1180 = (s32)p; *v1 = val; break;
    }
    mask = 0x10FFFF;      /* moved before the call — inert, GCC remats after */
    func_80060A68();
    t = arg0[0]; D_800F1140 = t;
    t = arg0[1]; D_800F1144 = t;
    D_800A3464 = mask;
    t = arg0[2]; D_800F1148 = t;
}
