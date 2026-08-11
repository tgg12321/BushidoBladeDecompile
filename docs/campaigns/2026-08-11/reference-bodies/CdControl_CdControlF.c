/* BEST ADAPTED BODY — src/system.c
 * Reference: sotn-decomp src/main/psxsdk/libcd/sys.c:87-112 (static inline cd_cw
 * + CdControl/CdControlF wrappers).
 * Adaptation: CD_cbsync -> g_cd_callback_a (D_800A11B4); D_80032A24[] ->
 * g_cd_sector_buf[] (D_800A112C); CdStatus() & CdlStatShellOpen -> the inlined
 * byte read g_cd_mode & 0x10 (D_800A11C4, what the target asm actually does);
 * CdlNop -> 1, CdlSetloc -> 2; BB2's s32 params kept (target masks with andi).
 * Return spelling: BB2 target computes `ret + 1` (addiu $v0,$s6,1) rather than
 * SOTN's `== 0`, so the wrappers use +1.
 *
 * MEASURED: CdControlF 19 (was 23), CdControl 17 (was 25). Both 0 net insn delta.
 * Residual: register allocation + one LICM constant hoist (build parks the
 * literal 1 of the `idx != 1` compare in a 9th callee-save $s7 across the loop;
 * target re-materialises `li $v0,1` inside the loop each iteration).
 */

static inline cd_cw(s32 com, s32 param, s32 result, s32 arg3) {
    s32 old = g_cd_callback_a;
    s32 count = 4;

    while (count--) {
        g_cd_callback_a = 0;
        if ((com & 0xFF) != 1 && (g_cd_mode & 0x10)) {
            CD_cw(1, 0, 0, 0);
        }
        if (param == 0 || g_cd_sector_buf[com & 0xFF] == 0 ||
            !CD_cw(2, (void *)param, (void *)result, 0)) {
            g_cd_callback_a = old;
            if (!CD_cw(com & 0xFF, (void *)param, (void *)result, arg3)) {
                return 0;
            }
        }
    }

    g_cd_callback_a = old;
    return -1;
}

s32 CdControl(s32 a0, s32 a1, s32 a2) {
    return cd_cw(a0, a1, a2, 0) + 1;
}
s32 CdControlF(s32 a0, s32 a1) {
    return cd_cw(a0, a1, 0, 1) + 1;
}
