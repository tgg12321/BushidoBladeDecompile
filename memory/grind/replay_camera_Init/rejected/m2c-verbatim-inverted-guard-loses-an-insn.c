/* s8 REJECTED — a fresh m2c decompile of asm/funcs/replay_camera_Init.s, taken
 * verbatim (INVERTED guard: `if (*pe62 == 0) { ...; return 1; } return 0;`,
 * no named load temps, the re-read folded into the last expression), with the
 * two pointer locals supplying the reload.  sandbox --disable all = 16 / 37.
 * The same shape with candidate.c's internal statement order (r1) = 17 / 37.
 * BOTH lose an instruction relative to candidate.c's 38: inverting the guard
 * moves the body into the fall-through block and local-alloc's block-local
 * pre-assignment changes with it.  m2c also proposes `s16 arg0`, which is
 * already banked dead (rejected/short-a0-narrowing-wrong-home-copy.c).
 * Measured s8: tmp/grind/replay_camera_Init/s8/sweep_results.json (r1, r2). */
s32 replay_camera_Init(s32 a0, s32 a1) {
    s16 *pe62 = &D_80101E62;
    s32 *pe70 = &D_80101E70;
    if (*pe62 == 0) {
        extern u8 SpecialCam;
        s32 sval = ((s32)(a0 << 16)) >> 13;

        D_80101E60 = a0;
        D_80101E6C = *(s32 *)((u8 *)&SpecialCam + sval);
        D_80101E70 = *(s32 *)((u8 *)&D_8008EC38 + sval);
        D_80101E7C = a1;
        D_80101E68 = 0;
        *pe62 = 2;
        D_80101E9E = 0;
        D_80101E78 = (u32)(*pe70 + 0x7FF) >> 11;
        return 1;
    }
    return 0;
}
