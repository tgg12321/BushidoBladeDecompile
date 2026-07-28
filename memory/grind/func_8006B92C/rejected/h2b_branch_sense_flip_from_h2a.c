/* s3 REJECTED: h2b_branch_sense_flip_from_h2a
 * Score: 26 (regressed from 6). From h2a base, flipped both case's compare from
 * == to != (else/then arms swap). Intended to force reorg to dead-branch-fill
 * with else-arm's lui $v1 instead of fall-through-fill with then-arm's lui $v0.
 * Actual: GCC jump-threading collapsed the != form to a different bne+j shape
 * that broke the shared do_call and duplicated jal sites, +20 insns of cascade.
 * Consistent with s2's h1c KILLED verdict (branch-sense flip family regresses).
 */
extern u32 D_800A34F8;
extern s32 D_800A350C;
s32 func_8006B92C(s32 *unused, u32 *arg1) {
    s32 sp10;
    s32 ret;
    s32 idx;
    s32 var_v0;
    u32 var_v1;
    s32 var_s0 = 0;
    u32 v;
    u32 a0;
    v = *arg1;
    sp10 = (v & 0xFFFF) | (v >> 16);
    ret = func_800692C0(&sp10, 0, D_800A34FC + 0xC, &D_800A350C);
    ret >>= 16;
    switch (ret) {
    case 1:
        a0 = D_800A34F8;
        if ((a0 & 0xE000) != 0x4000) {
            var_v1 = a0 & 0xFFFF1FFF;
            var_v0 = ((a0 >> 13) & 7) + 1;
            goto complete_store;
        } else {
            D_800A34F8 = a0 & 0xFFFF1FFF;
        }
        goto do_call;
    case 2:
        a0 = D_800A34F8;
        if ((a0 & 0xE000) != 0) {
            var_v1 = a0 & 0xFFFF1FFF;
            var_v0 = ((a0 >> 13) & 7) - 1;
        complete_store:
            var_v1 |= ((var_v0 & 7) << 13);
            D_800A34F8 = var_v1;
        } else {
            D_800A34F8 = (a0 & 0xFFFF1FFF) | 0x4000;
        }
    do_call:
        func_8005C650(0, 0x7F, 0x7F);
        break;
    }
    /* tail identical to candidate.c */
    return var_s0;
}
