#include "common_head.h"
/* FORM J (s8b forensics): as FORM I but the call returns a 64-bit value, whose
 * o32 ABI home is the $v0/$v1 PAIR (hard regs 2 AND 3).  This is the only
 * construct in the o32 ABI that makes hard reg 3 a value producer at all, and
 * s8 named it as the theoretical source of a $v1 copy-suggestion.  Purpose:
 * measure (a) whether hard regs 2/3 appear in the mask quantity's find_free_reg
 * `used' set, and (b) whether a $v1 copy-suggestion materialises anywhere. */
typedef long long s64;
extern s64 func_80060A68_d(void);
void func_800611A4(s32 *arg0, s32 *arg1) {
    u16 sp[3];
    u16 new_var;
    s32 t;
    s32 mask;
    s64 r;
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
    r = func_80060A68_d();
    t = arg0[0];
    D_800F1140 = t;
    t = arg0[1];
    mask = 0xFFFFEF;
    D_800F1144 = t;
    t = arg0[2];
    D_800A3464 = mask;
    D_800F1148 = t + (s32) r + (s32) (r >> 32);
}
