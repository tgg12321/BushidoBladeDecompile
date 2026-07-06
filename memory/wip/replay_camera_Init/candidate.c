/* WIP candidate for replay_camera_Init — src/code6cac_b2_post.c
 * Apply this body in place of the current replay_camera_Init definition.
 * Measured: sandbox --disable all score 17 (target 39 insns, build 36 insns),
 * 0 register-asm pins, 0 __asm__ blocks, 0 pointer aliases, 1 residual regfix
 * rule (replay_camera_Init: fill_delay @ 26 <- 15 — cannot retire until score 0).
 *
 * cheat-reviewer (2026-07-05) FAILed an earlier version of this candidate for
 * an unannotated `s16 *s0 = &D_80101E62;` pointer alias (pointer-alias-fake-exception
 * requires lever-exhaustion + named mechanism + /* FAKE */ annotation, none present).
 * Removed the alias entirely (direct D_80101E62 accesses below) — confirmed the
 * honest distance is UNCHANGED at 17, so the alias was not load-bearing.
 */
s32 replay_camera_Init(s32 a0, s32 a1) {
    s32 sval;

    if (D_80101E62 != 0) {
        return 0;
    }

    sval = ((s32)(a0 << 16)) >> 13;
    D_80101E60 = a0;
    {
        extern u8 SpecialCam;
        s32 cam_val;
        s32 ec_val;
        cam_val = *(s32 *)((u8 *)&SpecialCam + sval);
        D_80101E6C = cam_val;
        D_80101E7C = a1;
        ec_val = *(s32 *)((u8 *)&D_8008EC38 + sval);
        D_80101E70 = ec_val;
    }
    D_80101E68 = 0;
    D_80101E62 = 2;
    D_80101E9E = 0;
    D_80101E78 = (u32)(D_80101E70 + 0x7FF) >> 11;
    return 1;
}
