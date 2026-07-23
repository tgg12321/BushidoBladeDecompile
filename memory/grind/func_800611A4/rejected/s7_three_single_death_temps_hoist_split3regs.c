#include "common_head.h"
/* FORM G: 3 separate single-use temps (frontier-1's "single-death load pseudos").
 * Each load goes to its OWN pseudo (t0/t1/t2), so each is single-death -> LOCAL.
 * Purpose: dump .sched to show sched1 HOISTS the three loads together (the
 * interleave breaks) because there is no reused-pseudo WAR anti-dependence and
 * GCC disambiguates the D_800F114x stores from the arg0[] loads. Proves the
 * frontier-1 tension: single-death (local) and interleave-preserving are
 * mutually exclusive without pseudo-reuse (which forces multi-death -> global). */
void func_800611A4(s32 *arg0, s32 *arg1) {
    u16 sp[3];
    u16 new_var;
    s32 t0, t1, t2;
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
    t0 = arg0[0];
    D_800F1140 = t0;
    t1 = arg0[1];
    D_800F1144 = t1;
    D_800A3464 = 0xFFFFEF;
    t2 = arg0[2];
    D_800F1148 = t2;
}
