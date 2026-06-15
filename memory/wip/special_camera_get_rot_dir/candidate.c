/* special_camera_get_rot_dir (code6cac_b2_post.c) — clean candidate, ZERO cheat-asm.
 * Removed the 5 register pins (index s2, cam_base s3, constant_80 s4, copy_end s5).
 * The masked sandbox stays 12 either way (it masks reg names); the real gap is a
 * 4-register ROTATION vs target — only the full SHA1 sees it (not byte-identical).
 * Resume: paste over the function; the structure/count match, only the s2-s5
 * allocation rotates. */
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
    copy_end = (Quad *)&sp_buf[0x40];
    buf2_ptr = (s32 *)sp_buf2;

retry:
    func_80080258(2, index + cam_base, 0);
    v0 = ((s32 (*)())func_800372F4)(0x800, (s32)sp_buf, constant_80);
    if (v0 != 0) goto retry;

    {
        Quad *dst_q = (Quad *)dest;
        Quad *src = (Quad *)&sp_buf[0x10];
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
