void special_camera_get_rot_dir(s32 *dest) {
    u8 sp_buf[0x800];
    u8 sp_buf2[8];
    register s32 index asm("s2");
    register s32 cam_base asm("s3");
    s32 v0;
    register s32 constant_80 asm("s4");
    register Quad *copy_end asm("s5");
    s32 *buf2_ptr;

    constant_80 = 0x80;
    index = func_80036EA8(6, 0) << 3;
    cam_base = (s32)&SpecialCam;
    copy_end = (Quad *)&sp_buf[0x40];
    buf2_ptr = (s32 *)sp_buf2;

retry:
    CdControl(2, index + cam_base, 0);
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

    v0 = CdPosToInt(index + cam_base);
    CdIntToPos(v0 + 1, (s32)buf2_ptr);
    CdControl(2, (s32)buf2_ptr, 0);
    v0 = ((s32 (*)())func_800372F4)(dest[3], dest[2], constant_80);
    if (v0 != 0) goto retry;
}
