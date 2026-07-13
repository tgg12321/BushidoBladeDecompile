/* func_80037540 — REJECTED (s4, structural, 2026-07-13). sandbox --disable all = 15.
 *
 * WHY IT IS DEAD — and why the negative is worth more than the form:
 *
 * The goal was to find the CHEAPEST member of the "fully-written locals" branch of
 * s3's trichotomy. s3 measured `s32 argv[6]; s32 idx[2];` (32B locals, every byte
 * written) -> score 2: frame correct at -0x48, but two `sw` into 0x28/0x2C that the
 * target does not contain. The obvious next question s3 never asked: can a locals
 * object of 25-28 bytes cost only ONE extra store? That would be `argv[6]` (24B)
 * plus a single 4-byte object written exactly once — this file.
 *
 * MEASURED: score 15, frame -0x40, and objdump shows **NO STORE INTO idx[0] AT ALL**.
 * GCC 2.7.2 SCALAR-PROMOTED the single-element array into a register (it survives in
 * $v0/$s0 across two jal's; six stores only, at 0x10..0x24). get_frame_size = 24, not
 * 28. The form degenerates EXACTLY to `s32 sp[6]` (same 43 insns, same score 15, all
 * diffs sp-relative offset shifts).
 * Artifact: tmp/grind/func_80037540/s2/idx1_promoted.objdump
 *
 * => THE "+1 EXTRA STORE" FORM DOES NOT EXIST. Any extra object small enough to be
 * written by a single `sw` (i.e. one 4-byte element, constant index, address never
 * taken) is scalar-promoted and contributes ZERO frame bytes. To occupy frame bytes
 * an object must defeat promotion — which needs >=2 elements (or an address escape,
 * and this function has no callee that takes a pointer: func_80036EA8(int,int),
 * func_800392B8(void), and the argv pointer is already argv). >=2 elements written
 * => >=2 stores. o32 MIPS-I has no 8-byte store, so no single-instruction way to
 * fully write an 8-byte object either.
 *
 * THEREFORE s3's +2 is not one sampled point — it is the PROVEN MINIMUM of the
 * fully-written branch. The trichotomy is now closed at both ends by measurement:
 *     s32 sp[6]                (24B, tail-free)              -> 15  (frame 0x40)
 *     s32 argv[6]; s32 idx[1]  (promoted; degenerates to 24B) -> 15  (frame 0x40)  <- this file
 *     s32 argv[6]; s32 idx[2]  (32B, every byte written)      ->  2  (MINIMUM of the branch)
 *     s32 sp[7] / s32 sp[8]    (28B/32B, unwritten tail)      ->  0
 * Reaching frame 0x48 requires get_frame_size in [25,32] (H7, from
 * mips.c:compute_frame_size; outgoing_args=16 and gp_reg_size=24 are both pinned by
 * the target bytes). Every such object either writes its extra bytes (>= +2 insns the
 * target lacks, PROVEN minimum) or does not (an unwritten tail). No third option.
 * => The ORIGINAL SOURCE declared a locals object strictly larger than what it wrote.
 */
extern s32 func_800392B8(void);
extern void marionation_camera_Init_80037468(s32, s32 *, s32);

void func_80037540(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4) {
    s32 argv[6];
    s32 idx[1];
    s32 v0;

    idx[0] = func_80036EA8(6, a2);
    argv[0] = (s32)&SpecialCam + idx[0] * 8;
    argv[1] = a3;
    argv[2] = a0;
    argv[3] = a1;
    v0 = func_80036EA8(6, 2);
    argv[4] = (s32)&SpecialCam + v0 * 8;
    argv[5] = a4;
    v0 = func_800392B8();
    marionation_camera_Init_80037468(6, argv, v0 + 0x7FC);
}
