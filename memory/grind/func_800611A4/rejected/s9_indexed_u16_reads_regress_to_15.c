/* REJECTED s9 -- score 15 (was 0 with the byte-offset cast spelling).
 * Same 43 insns, same walking-pointer tail; the ONLY difference is the
 * pre-call param reads spelled `((u16 *) arg1)[i]` and the buffer address
 * spelled `(s32) svec` instead of `*((u16 *)(((s32) arg1) + N))` /
 * `(s32) (&svec[0])`. Semantically identical C; 15 diffs of pre-call
 * scheduling/register naming. Lesson: the pre-call region's byte-identity
 * depends on the cast spelling of the halfword reads -- do not "clean up"
 * those casts. */
void func_800611A4(s32 *arg0, s32 *arg1) {
    u16 svec[3];
    s32 *p;
    s32 *v1 = (s32 *) (&D_800F116C);
    svec[0] = ((u16 *) arg1)[0];
    svec[1] = ((u16 *) arg1)[1];
    D_800A3468 = (s32) v1;
    svec[2] = ((u16 *) arg1)[2];
    D_800F117C = (s32) svec;
    D_800F1178 = (s32) arg0;
    D_800F1180 = (s32) (&D_800F116A);
    *v1 = 0x21001A;
    func_80060A68();
    p = arg0;
    D_800F1140 = *p++;
    D_800F1144 = *p++;
    D_800F1148 = *p;
    D_800A3464 = 0xFFFFEF;
}
