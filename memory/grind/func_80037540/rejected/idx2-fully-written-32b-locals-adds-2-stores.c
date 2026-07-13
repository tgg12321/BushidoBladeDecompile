/* REJECTED — `s32 argv[6]; s32 idx[2];`  sandbox --disable all = 2  (45 build insns vs 43 target)
 *
 * WHY IT WAS TRIED (s3, structural): this is the ONLY form that makes all 32
 * bytes of the target's locals region semantically real with ZERO dead
 * declarations — the two func_80036EA8() results get a genuine home in idx[],
 * both elements are written AND read. get_frame_size() = 24 + 8 = 32, so the
 * prologue comes out at the correct -0x48 and all 43 target instructions match
 * in shape.
 *
 * WHY IT IS DEAD: the two extra instructions are exactly the two `sw` stores
 * into idx[0] (0x28) and idx[1] (0x2C). The TARGET CONTAINS NO SUCH STORES —
 * it has exactly six stores, at 0x10/0x14/0x18/0x1C/0x20/0x24.
 *
 * THIS IS THE PROOF, not just a failed variant. Combined with GCC 2.7.2's
 * compute_frame_size (mips.c): total = MIPS_STACK_ALIGN(get_frame_size())
 * + MIPS_STACK_ALIGN(outgoing_args=16) + MIPS_STACK_ALIGN(gp_reg_size=24),
 * so frame 0x48 REQUIRES get_frame_size() in [25,32]. Any C that WRITES those
 * extra bytes emits stores the target does not have (this file: +2). Any C that
 * does NOT write them declares an unwritten tail. There is no third option.
 * => The ORIGINAL SOURCE declared a locals object larger than what it wrote.
 *
 * Superseded by the s3 ruling-request. Do not re-propose.
 */
void func_80037540(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4) {
    s32 argv[6];
    s32 idx[2];
    s32 v0;

    idx[0] = func_80036EA8(6, a2);
    argv[0] = (s32)&SpecialCam + idx[0] * 8;
    argv[1] = a3;
    argv[2] = a0;
    argv[3] = a1;
    idx[1] = func_80036EA8(6, 2);
    argv[4] = (s32)&SpecialCam + idx[1] * 8;
    argv[5] = a4;
    v0 = func_800392B8();
    marionation_camera_Init_80037468(6, argv, v0 + 0x7FC);
}
