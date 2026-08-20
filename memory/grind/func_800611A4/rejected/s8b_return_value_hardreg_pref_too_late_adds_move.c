/* FORM K (s8b forensics): FORM F's exact target-interleaved shape, but the
 * function is declared to RETURN the last-loaded value.  The o32 return copy
 * (set (reg 2) (reg <web>)) is the ONE pure-C construct that ties the multi-death
 * load web itself to hard reg 2 -- via combine_regs' copy-suggestion in
 * local-alloc.c:1854-1899 and global.c's set_preference hard-reg copy preference.
 * If the web is preferred into $v0 and hard reg 2 becomes live across the mask's
 * span, find_free_reg's regs_live_at OR (local-alloc.c:2170) excludes $v0 for the
 * mask and hands it $v1 = target's exact layout, at zero instruction cost (a
 * return of a value already in $v0 emits no move).  Purpose: measure whether the
 * return copy is placed before the mask's death (it is emitted at the epilogue,
 * AFTER the mask's sw, so the prediction is that it does not intersect the mask's
 * [born,dead) span and the mask still grabs $v0 in local-alloc, forcing the web to
 * $v1 and a +1 `move $v0,$v1'). */
#include "common_head.h"
/* FORM F: forward-order interleaved mask = target's EXACT instruction shape.
 * loads 0x0/0x4/0x8 into reused t; mask 0xFFFFEF built interleaved (assigned
 * mid-cluster, stored) so it stays alive across the loads. This is the shape
 * whose scheduling matches target. Purpose: dump .lreg/.greg to confirm the
 * routing in target's own instruction shape -- web multi-death -> global,
 * mask single-death -> local -> grabs v0, web forced to v1 (the 9-wall). */
s32 func_800611A4(s32 *arg0, s32 *arg1) {
    u16 sp[3];
    u16 new_var;
    s32 t;
    s32 mask;
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
    t = arg0[0];
    D_800F1140 = t;
    t = arg0[1];
    mask = 0xFFFFEF;
    D_800F1144 = t;
    t = arg0[2];
    D_800A3464 = mask;
    D_800F1148 = t;
    return t;
}
