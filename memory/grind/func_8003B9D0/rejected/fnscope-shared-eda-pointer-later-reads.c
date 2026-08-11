/* REJECTED — func_8003B9D0 — session 5 (forensics), sandbox 16 (build_insns 186).
 *
 * WHAT THIS WAS: the session-4-forensics frontier F1 probe, written out in full.
 * The idea was that region A's `(plus (reg eda) 1100)` address needs a pseudo
 * with a NON-ZERO whole-function reference count so that its set survives
 * `delete_dead_from_cse`, and that the honest way to get one is to let ONE
 * pointer genuinely serve both the `qf & 0x30` save/restore block AND the two
 * later reads of the same object — which the function really does perform
 * (`a3_arg = D_80101EDA` reads the halfword at +0, `a0_arg = D_80102326` reads
 * the halfword at +0x44C, and D_80102326 == &D_80101EDA[0x226]).
 * `eda` is therefore declared at FUNCTION scope and assigned UNCONDITIONALLY at
 * the top (the block-scoped variant that permuter output-200-1 produced is
 * undefined behaviour — a pointer assigned only inside `if (qf & 0x30)` and read
 * afterwards — which is why that form was rejected in session 4).
 *
 * MEASURED: `sandbox func_8003B9D0 --disable all` == 16 (target_insns 185,
 * build_insns 186), versus the floor of 6.  KILLED.
 *
 * WHY IT FAILS, from the objdump of tmp/sandbox/func_8003B9D0/code6cac_c2.o:
 * the three IN-BLOCK displaced sites still FOLD —
 *     lui s1,0x0 / lh s1,1100(s1)
 *     lui at,0x0 / sh v0,1100(at)
 *     lui at,0x0 / sh s1,1100(at)
 * — while the LATER read comes out as `lh a0,1100(s2)`, i.e. target's
 * register+displacement shape appears at the ONE site where target does not
 * want it (target re-materialises `lui a0,%hi(D_80102326)` there).  The reason
 * is exactly the cse quantity mechanism: the region-B `if/else` join labels end
 * the cse basic block, so by the later read `eda`'s quantity carries no
 * `qty_const` and `find_best_addr` cannot fold; inside the `qf & 0x30` block
 * `eda`'s constant is still live, so it folds.  Hoisting the pointer's
 * definition also moves the `la` out of target's position (tgt[55..56], inside
 * the block) and turns the two later symbol loads into pointer loads, which is
 * where the extra 10 points come from.
 *
 * GENERAL LESSON BANKED: a non-zero whole-function reference count is NOT
 * sufficient — the reference has to be one that keeps the address expression in
 * the cse HASH TABLE at the point the displaced MEM is processed.  A reference
 * that lives in a later cse basic block does nothing for region A.
 */

void func_8003B9D0(void) {
    s32 saved_first;
    s32 saved_44c;
    s32 a3_arg;
    s32 a0_arg;
    s32 magic;
    s32 v0;
    u8 *p;
    s16 *eda;
    u8 flags;

    eda = &D_80101EDA;
    magic = 0x80190800;
    func_8001DA2C();
    game_Cleanup();
    if (D_800A3768 != 0x14) gpu_InitDisplay();
    if (D_800A3768 != 0xFF) gpu_DisableDisplay();
    func_800174F4();
    gpu_EnableDisplay();
    func_80020D38();
    disp_SetFramebufferMode(1, 0, 0, 0);
    if (((u8 *)D_800A3878)[3] & 0x80) {
        func_80020CDC();
        magic = 0x80118800;
    }
    {
        u8 *q = (u8 *)D_800A3878;
        u8 qf = q[3];
        if (qf & 0x30) {
            saved_first = eda[0];
            saved_44c = eda[0x226];
            if (qf & 0x10) eda[0] = 0x32;
            if (q[3] & 0x20) eda[0x226] = 0x32;
            func_8003AFFC();
            eda[0] = saved_first;
            eda[0x226] = saved_44c;
        }
    }
    if (((u8 *)D_800A3878)[3] & 0x1) a3_arg = eda[0]; else a3_arg = -1;
    if (((u8 *)D_800A3878)[3] & 0x2) a0_arg = eda[0x226]; else a0_arg = -1;
    p = (u8 *)D_800A3878;
    flags = p[3];
    if (flags & 0x10) a3_arg = 0x32;
    if (flags & 0x20) a0_arg = 0x32;
    D_800A390F = 0;
    func_80054884(D_800A376C, p[0], 0, a3_arg, a0_arg, -1, -1, magic);
    func_80041688(0, 0);
    func_80041688(1, 0);
    if (((u8 *)D_800A3878)[3] & 0x40) func_8004659C(-1);
    if (D_8010277D == 0xE || D_8010277D == 0x1D) {
        func_80041BF4(D_800A37B4, D_800A37B5, D_800A37B6);
    }
    func_8001DBE4();
    D_800A3768 = 0xFF;
    D_800A36A8 = 0;
    func_80035FA8();
    v0 = func_80036EA8(5, ((u8 *)D_800A3878)[1]);
    func_80036FD4(v0, ((u8 *)D_800A3878)[2]);
    func_80037260();
    D_800A37B8 = 0;
    D_800A3834 = 7;
    gpu_DisableDisplay();
}
