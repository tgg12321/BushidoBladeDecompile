/* candidate.c — func_8003B9D0 — BEST *LEGAL* FORM (floor 6).
 *
 * READ THIS FIRST.  An earlier session banked a MATCHING body here (the
 * scalar-to-array declaration correction of D_80101EDA).  Layer 1 FAILed that
 * construct and the driver BANNED it, so it is NOT in this file any more; it
 * lives at
 *   rejected/consistent-array-retype-oracle-exact-but-banned-family.c
 * together with the older, per-site-inconsistent variant the driver already
 * banked.  Do not re-propose either without an owner ruling (see the s6
 * synthesis ruling-request in the outcome JSON / hypotheses.md).
 *
 * MEASURED THIS SESSION (s6 synthesis, 2026-08-11), with this body applied to
 * src/code6cac_c2.c and build/ freshly rebuilt from HEAD so the sandbox
 * reference object is canonical:
 *   & tools/wteng.ps1 main sandbox func_8003B9D0 --disable all
 *     -> "score": 6, target_insns 185, build_insns 188, rules_dropped 1.
 *
 * WHAT THIS BODY IS:
 *   D1 — all three of session-1's cheat-asm constructs are DELETED (the
 *        identity-reload barrier on `eda` and the two memory-clobber
 *        scheduling barriers).  Layer 1 ruled D1 clean.
 *   D2 — region B (the 7-instruction shortfall: target's three full
 *        lui/lw/nop/lbu/nop reloads of ((u8 *)D_800A3878)[3]) is closed by
 *        spelling the two flag-selected argument initialisations as if/ELSE
 *        rather than "init to -1, then conditionally overwrite".  An if/else
 *        arm ends in an unconditional jump + BARRIER, so cse_end_of_basic_block
 *        cannot extend the cse basic block past the join label and the flag
 *        word must be re-read.  Layer 1 ruled D2 clean.
 *
 * WHAT IS STILL OPEN (region A, 6 points, 3 sites x 2 insns vs 1):
 *   target keeps the symbol address live in $s0 and addresses the +0x44C
 *   (== 1100 == 0x226 halfwords) accesses as `lh/sh $r,1100($s0)`; this body
 *   re-materialises the symbol at each of those three sites
 *   (`lui` + `%lo(D_80101EDA+1100)`), costing 3 extra instructions
 *   (188 vs target's 185).  Sessions 1-6 established the full mechanism and
 *   the exhaustive partition; see hypotheses.md H6/H9 and evidence.md.
 *
 * INTEGRATION NOTE (operator step, unchanged since s4): regfix.txt:1116 still
 * carries `func_8003B9D0: fill_delay @ 49 <- 52`.  Every honest measurement
 * since s2 drops that rule (rules_dropped 1) and the residual never contains a
 * delay-slot difference, so the rule is dead weight and should be retired.
 * Grind sessions may not edit regfix.txt.
 */

/* declaration (src/code6cac_c2.c:166-167), UNCHANGED FROM HEAD:
 *     extern s16 D_80101EDA;
 *     extern s16 D_80102326;
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
