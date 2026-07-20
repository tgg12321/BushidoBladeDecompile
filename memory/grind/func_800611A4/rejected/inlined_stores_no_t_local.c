/* REJECTED s2 — Variant B: no `t` local, direct inlined stores (m2c shape).
 * Sandbox --disable all = 11 (worse than pin-free floor 9). Same 43 insns.
 * Pure RA shift: inlined single-use expression pseudos are HIGHER-priority
 * to the RA than the mask, sending mask to $v1 and displacing at least one
 * more insn's register pair (11 diffs vs 9). Killed the hypothesis that
 * "anonymous single-use pseudos would relax the tiebreak surface." They
 * make it worse: fewer refs on the load side means mask no longer wins v0. */
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
    D_800F1140 = arg0[0];
    D_800F1144 = arg0[1];
    D_800A3464 = 0xFFFFEF;
    D_800F1148 = arg0[2];
}
