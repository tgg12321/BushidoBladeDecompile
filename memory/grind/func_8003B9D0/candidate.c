/* candidate.c — func_8003B9D0 — best LEGAL form.  HONEST FLOOR: 6.
 *
 * FLOOR: `sandbox func_8003B9D0 --disable all` == **6**
 *        (target_insns 185, build_insns 188, rules_dropped 1, cheat-asm
 *        stripping ON), re-measured in the forensics session (s4-forensics,
 *        2026-08-11) with this exact body applied to src/code6cac_c2.c.
 *
 * WHY THIS IS THE CANDIDATE AND NOT THE SANDBOX-0 FORM.
 * The previous session's sandbox-0 body added, in place of
 * `saved_44c = eda[0x226];`, the pair
 *
 *     p = (u8 *)&eda[0x226];      <- reuse of the function's later scratch ptr
 *     saved_44c = *(s16 *)p;
 *
 * The layer-1 cheat-reviewer FAILED it (D3) and the driver has BANNED that
 * construct for this function under any spelling.  It is banked, unchanged,
 * at rejected/p-staging-layer1-cheat-banned.c.  The forensics session then
 * proved the reviewer right on mechanism: see hypotheses.md H5 — the
 * construct's entire effect is to keep an otherwise-dead address-computation
 * insn alive through cse1 so that cse2 makes a different decision, and that
 * insn is deleted again by `flow` before any byte is emitted.  "Dead in the
 * emitted output, but its existence in source changed codegen upstream of
 * DCE" is the policy's own definition of a cheat-by-spelling.
 *
 * WHAT THIS BODY ALREADY BUYS (both clean, both keep):
 *   D1 — all three session-1 cheat-asm constructs are gone (the `eda`
 *        identity-reload barrier and the two `__asm__ ("" ::: "memory")`
 *        scheduling barriers).  This body carries ZERO `__asm__`.
 *   D2 — REGION B (the whole original 7-instruction shortfall) is closed by
 *        spelling the two flag-selected argument initialisations as if/else
 *        rather than "init to -1, then conditionally overwrite".
 *
 * WHAT IS LEFT (the 6 points) — REGION A, three `+0x44C` sites where we emit
 * `lui $x,%hi(D_80101EDA+1100)` + `lh|sh $r,%lo(...)($x)` and target emits
 * `lh|sh $r,1100($s0)`.  Named exactly, in hypotheses.md H5:
 *   * the fold is performed by **cse2** (the SECOND cse pass, toplev.c:2926),
 *     in `find_best_addr` (cse.c:2659-2663) via `fold_rtx`'s substitution of
 *     the base pseudo's `qty_const` (cse.c:5171-5179);
 *   * cse1 does NOT fold it — at `.cse` all three sites are still
 *     `(mem (plus (reg eda) (const_int 1100)))`;
 *   * what decides cse2's behaviour is whether an insn setting a pseudo to
 *     `(plus (reg eda) 1100)` still EXISTS when cse2 runs, and that is decided
 *     by `delete_dead_from_cse` (cse.c:8683, called at toplev.c:2867 right
 *     after cse1) which deletes a SET whose destination pseudo has a
 *     WHOLE-FUNCTION reference count of zero.
 * So any dedicated pointer local for the far address is propagated into the
 * MEMs by cse1, drops to refcount 0, is deleted, and the fold comes back —
 * which is why every dedicated-pointer variant measures 188/6.
 *
 * Region B's if/else spelling is unchanged from session 2.
 */

void func_8003B9D0(void) {
    s32 saved_first;
    s32 saved_44c;
    s32 a3_arg;
    s32 a0_arg;
    s32 magic;
    s32 v0;
    u8 *p;
    u8 flags;

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
            s16 *eda = &D_80101EDA;
            saved_first = eda[0];
            saved_44c = eda[0x226];
            if (qf & 0x10) eda[0] = 0x32;
            if (q[3] & 0x20) eda[0x226] = 0x32;
            func_8003AFFC();
            eda[0] = saved_first;
            eda[0x226] = saved_44c;
        }
    }
    if (((u8 *)D_800A3878)[3] & 0x1) a3_arg = D_80101EDA; else a3_arg = -1;
    if (((u8 *)D_800A3878)[3] & 0x2) a0_arg = D_80102326; else a0_arg = -1;
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
