/* s13: SAME mechanism as the winning candidate (duplicated arms + a second,
 * single-use row pointer) but the second pointer is a FRESH local `q` instead of a
 * re-assignment of `p`, and the arm condition is `t` instead of `ofs`. Measured on
 * the live chassis: score 22, build_insns 38 == target_insns 38 - the ARRANGEMENT is
 * the target's exactly, but the extra pseudo and the extended live range of `t`
 * shift the whole allocation (t->$v1, i->$a2, ofs->$a1 instead of the target's
 * t->$v0, i->$a1, ofs->$v1). Kept as the measured proof that the mechanism is
 * independent of the register outcome and that reusing `p` (candidate.c) is what
 * restores the target allocation. */
void func_80062020(s32 *arg0) {
    s32 i;
    s32 ofs;
    s32 t;
    s32 *p;
    s32 *q;
    t = *(s32 *)((u8 *)arg0 + 0);
    D_800A32B8 = 0;
    i = 0;
    if ((t & 1) == 0) goto end;
    ofs = 0;
    do {
        t = *(s32 *)((u8 *)arg0 + ofs + 0);
        *(s32 *)((u8 *)&D_800F1198 + ofs) = t;
        t = *(s32 *)((u8 *)arg0 + ofs + 4);
        i = i + 1;
        *(s32 *)((u8 *)&D_800F119C + ofs) = t;
        t = *(s32 *)((u8 *)arg0 + ofs + 8);
        *(s32 *)((u8 *)&D_800F11A0 + ofs) = t;
        ofs = ofs + 12;
        t = *(s32 *)((u8 *)arg0 + ofs + 0);
    } while ((t & 1) != 0);
end:
    ofs = i + i;
    ofs = ofs + i;
    ofs = ofs << 2;
    p = (s32 *)((u8 *)&D_800F1198 + ofs);
    if (t) {
        p[2] = 0;
        p[1] = 0;
    } else {
        p[2] = 0;
        p[1] = 0;
    }
    q = (s32 *)((u8 *)&D_800F1198 + ofs);
    q[0] = 0;
}
