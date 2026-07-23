#include "common_head.h"
/* FORM H: express the 3 stores as an AGGREGATE (struct) copy instead of the
 * reused-t load/store idiom. Genuinely new pseudo topology (not in the rejected
 * bank). Question: does GCC 2.7.2's block-move produce a single-death LOCAL
 * load-web that could win v0, and does it interleave with the mask? */
struct S3 { s32 a, b, c; };
void func_800611A4(s32 *arg0, s32 *arg1) {
    u16 sp[3];
    u16 new_var;
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
    *((struct S3 *) (&D_800F1140)) = *((struct S3 *) arg0);
    D_800A3464 = 0xFFFFEF;
}
