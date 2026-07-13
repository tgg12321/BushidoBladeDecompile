/* REJECTED (s2) — the ONLY tail-free form: s32 sp[6], every element written.
 *
 * sandbox --disable all == 15.  target_insns 43, build_insns 43.
 *
 * WHY IT IS DEAD:
 * The instruction COUNT is already correct (43 == 43) and the six stores land in
 * the right ORDER. All 15 diffs are sp-relative offset/immediate shifts caused by
 * the frame being 0x40 instead of 0x48:
 *     locals 24B -> frame 16+24+24 = 0x40   (saves s0..s4,ra at 0x28-0x3C,
 *                                            incoming a4 lw at 0x50)
 *     target     -> frame          = 0x48   (saves at 0x30-0x44, a4 lw at 0x58)
 *
 * This is the load-bearing negative of the whole function: it proves the C is
 * OTHERWISE CORRECT and that the locals-region SIZE is the entire and only gap.
 * There is nothing else left to restructure.
 *
 * Keep this form. If the ruling goes against any unwritten-tail reservation,
 * this is the clean body and the residual 15 is the frame, not the code.
 */
void func_80037540(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4) {
    s32 sp[6];
    s32 v0;

    v0 = func_80036EA8(6, a2);
    sp[0] = (s32)&SpecialCam + v0 * 8;
    sp[1] = a3;
    sp[2] = a0;
    sp[3] = a1;
    v0 = func_80036EA8(6, 2);
    sp[4] = (s32)&SpecialCam + v0 * 8;
    sp[5] = a4;
    v0 = func_800392B8();
    marionation_camera_Init_80037468(6, sp, v0 + 0x7FC);
}
