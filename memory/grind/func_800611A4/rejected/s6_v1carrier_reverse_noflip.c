#include "common_head.h"
/* FORM E: reverse order, stage the LAST (offset-0) load through the EXISTING v1
 * pointer var. Mirrors form A (which flips, sandbox 2) but with an existing
 * carrier instead of invented new_var2 -> tests whether the flip is available
 * via a sanctionable carrier. Still reverse order (residual load-order gap). */
void func_800611A4(s32 *arg0, s32 *arg1) {
    u16 sp[3];
    u16 new_var;
    s32 t;
    s32 *v1 = (s32 *) (&D_800F116C);
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
    t = arg0[2];
    D_800F1148 = t;
    t = arg0[1];
    D_800F1144 = t;
    v1 = (s32 *) arg0[0];
    t = (s32) v1;
    D_800F1140 = t;
    D_800A3464 = 0xFFFFEF;
}
