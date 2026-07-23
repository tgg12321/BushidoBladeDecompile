/* REJECTED — arg0[i] indexing form. Honest pin-free floor = 9 (the inherited
 * "wall"). All 9 diffs are the v0/v1 register-name swap: this form allocates the
 * load temp to $v1 and the mask to $v0, the reverse of target. HEAD "matched"
 * only by pinning `register s32 t asm("$2"); register s32 mask asm("$3");`
 * (forbidden cheat-asm). Superseded by the walking-pointer candidate, which
 * reaches 0 without pins — so the swap is NOT unreachable in pure C, it was only
 * unreachable from THIS indexing shape. */
void func_800617C8(s32 *arg0) {
    s32 *v1 = (s32 *)&D_800F116C;
    s32 t, mask;   /* HEAD pinned these to $2/$3 — cheat-asm; still floor 9 unpinned */
    D_800A3468 = (s32)v1;
    D_800F1178 = (s32)arg0;
    /* ... prologue/branch body identical to candidate ... */
end:
    func_80060A68();
    t = arg0[0]; D_800F1140 = t;
    t = arg0[1]; D_800F1144 = t;
    mask = 0xC06013;
    D_800A3464 = mask;
    t = arg0[2]; D_800F1148 = t;
}
