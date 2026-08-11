/* candidate.c — func_8003B9D0 — best form after session 1 (recon).
 *
 * FLOOR: sandbox --disable all == 21 (build_insns 178 vs target 185).
 * This is the UNCHANGED HEAD form of src/code6cac_c2.c:183-249. No probe this
 * session improved on it, so it is banked verbatim as the baseline to resume from.
 *
 * NOTE FOR THE NEXT SESSION: the three __asm__ constructs below are CHEAT-ASM and
 * are stripped by the sandbox before scoring (they cannot and do not move the 21).
 * They are shown here only because they mark exactly where the two residual regions
 * are — the identity-reload barrier on `eda` marks region A, the two memory barriers
 * mark region B. A COMPLETED-C form must delete all three AND the regfix rule
 * `func_8003B9D0: fill_delay @ 49 <- 52`.
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
            __asm__ __volatile__("" : "=r"(eda) : "0"(eda));   /* CHEAT — region A marker */
            saved_first = eda[0];
            saved_44c = eda[0x226];
            if (qf & 0x10) eda[0] = 0x32;
            if (q[3] & 0x20) eda[0x226] = 0x32;
            func_8003AFFC();
            eda[0] = saved_first;
            eda[0x226] = saved_44c;
        }
    }
    a3_arg = -1;
    if (((u8 *)D_800A3878)[3] & 0x1) a3_arg = D_80101EDA;
    __asm__ __volatile__("" ::: "memory");                     /* CHEAT — region B marker */
    a0_arg = -1;
    if (((u8 *)D_800A3878)[3] & 0x2) a0_arg = D_80102326;
    __asm__ __volatile__("" ::: "memory");                     /* CHEAT — region B marker */
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
