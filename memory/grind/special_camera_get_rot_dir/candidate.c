/* special_camera_get_rot_dir (code6cac_b2_post.c) — BEST FORM, floor=9 (down from 12).
 * ZERO cheat-asm, ZERO pins. Apply over the function to start from the score-9 floor.
 *
 * s1 recon finding: the honest floor is a pure 4-register rotation of
 * {index, cam_base, constant_80, copy_end} over callee-saved s2/s3/s4/s5.
 * The masked sandbox IS a valid gradient here (score == count of rotation
 * diffs; the WIP's "sandbox is blind" claim was WRONG — it only strips PINS,
 * not real C-structure changes).
 *
 * This form defines copy_end block-locally (line "copy_end = ..." moved INTO
 * the copy block, just before the do-loop). That correctly lands
 * index->s2, cam_base->s3, constant_80->s4 (all THREE now match target).
 * The ONLY residual: copy_end lands in caller-saved t0 (score 9, 70 insns)
 * because its live range no longer crosses a call; target needs it in
 * callee-saved s5 (72 insns, +2 save/restore). See hypotheses.md for the
 * remaining lever (get copy_end callee-saved-but-lowest-priority).
 */
void special_camera_get_rot_dir(s32 *dest) {
    u8 sp_buf[0x800];
    u8 sp_buf2[8];
    s32 index;
    s32 cam_base;
    s32 v0;
    s32 constant_80;
    Quad *copy_end;
    s32 *buf2_ptr;

    constant_80 = 0x80;
    index = func_80036EA8(6, 0) << 3;
    cam_base = (s32)&SpecialCam;
    buf2_ptr = (s32 *)sp_buf2;

retry:
    func_80080258(2, index + cam_base, 0);
    v0 = ((s32 (*)())func_800372F4)(0x800, (s32)sp_buf, constant_80);
    if (v0 != 0) goto retry;

    {
        Quad *dst_q = (Quad *)dest;
        Quad *src = (Quad *)&sp_buf[0x10];
        copy_end = (Quad *)&sp_buf[0x40];
        do {
            *dst_q = *src;
            src++;
            dst_q++;
        } while (src != copy_end);
        *(Triple *)dst_q = *(Triple *)src;
    }

    v0 = cdrom_BcdToFrames(index + cam_base);
    cdrom_FramesToBcd(v0 + 1, (s32)buf2_ptr);
    func_80080258(2, (s32)buf2_ptr, 0);
    v0 = ((s32 (*)())func_800372F4)(dest[3], dest[2], constant_80);
    if (v0 != 0) goto retry;
}
