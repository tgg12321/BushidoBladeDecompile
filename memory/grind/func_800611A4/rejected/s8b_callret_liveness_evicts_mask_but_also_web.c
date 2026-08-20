#include "common_head.h"
/* FORM I (s8b forensics): FORM F's exact post-call shape, but func_80060A68 is
 * declared to RETURN s32 and its result is consumed AFTER the mask's store, so
 * the ABI return register $v0 (hard reg 2) is semantically live ACROSS the mask
 * quantity's span.  Purpose: measure whether hard reg 2 lands in find_free_reg's
 * `used' set for the mask quantity (local-alloc.c:2170 ORs regs_live_at[ins]
 * over born..dead).  If it does, the lowest-free scan must skip $v0 and hand the
 * mask $v1 = target's layout, and the frontier becomes "find a zero-insn way to
 * keep hard reg 2 live".  If it does not, GCC's immediate hard-reg->pseudo copy
 * of the return value makes the regs_live_at path unreachable in pure C. */
extern s32 func_80060A68_r(void);
void func_800611A4(s32 *arg0, s32 *arg1) {
    u16 sp[3];
    u16 new_var;
    s32 t;
    s32 mask;
    s32 r;
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
    r = func_80060A68_r();
    t = arg0[0];
    D_800F1140 = t;
    t = arg0[1];
    mask = 0xFFFFEF;
    D_800F1144 = t;
    t = arg0[2];
    D_800A3464 = mask;
    D_800F1148 = t + r;
}
