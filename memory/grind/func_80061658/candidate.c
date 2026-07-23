/* func_80061658 — best NON-CHEAT form. Floor 9 (honest sandbox --disable all).
 * Pure C, zero pins/rules. Byte-match still needs t->$2 / mask->$3 pins (cheat).
 * The honest residual is a pure v0<->v1 swap in the tail: unpinned GCC allocates
 * the load-temp `t` to v1 and the mask const to v0; target wants the reverse.
 * Scheduling/interleaving is byte-identical to target — RA choice only.
 * Apply this body in place of the pinned HEAD body when starting a drill session. */
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
        val = 0x21000C;
        p = &D_800F115C;
        *p = 0;
        D_800F1180 = (s32)p;
        *v1 = val;
        break;
    case 1:
        val = 0x21000D;
        p = &D_800F115C + 1;
        *p = 0;
        D_800F1180 = (s32)p;
        *v1 = val;
        break;
    }
    func_80060A68();
    t = arg0[0]; D_800F1140 = t;
    t = arg0[1]; D_800F1144 = t;
    mask = 0x10FFFF;
    D_800A3464 = mask;
    t = arg0[2]; D_800F1148 = t;
}
