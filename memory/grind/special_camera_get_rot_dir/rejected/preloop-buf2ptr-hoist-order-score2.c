typedef struct { s32 w_q[4]; } Quad;
typedef struct { s32 w_t[3]; } Triple;
typedef struct { s32 rot[15]; } CamRot;
extern void CdControl(s32, s32, s32);
extern void CdIntToPos(s32, s32);
void special_camera_get_rot_dir(s32 *dest) {
    extern u8 SpecialCam;
    u8 sp_buf[0x800];
    u8 sp_buf2[8];
    s32 index;
    s32 cam_base;
    s32 v0;
    s32 constant_80;
    s32 *buf2_ptr;

    constant_80 = 0x80;
    index = func_80036EA8(6, 0) << 3;
    cam_base = (s32)&SpecialCam;
    buf2_ptr = (s32 *)sp_buf2;

    for (;;) {
        CdControl(2, index + cam_base, 0);
        v0 = ((s32 (*)())func_800372F4)(0x800, (s32)sp_buf, constant_80);
        if (v0 != 0) continue;

        *(CamRot *)dest = *(CamRot *)&sp_buf[0x10];

        v0 = CdPosToInt(index + cam_base);
        CdIntToPos(v0 + 1, (s32)buf2_ptr);
        CdControl(2, (s32)buf2_ptr, 0);
        v0 = ((s32 (*)())func_800372F4)(dest[3], dest[2], constant_80);
        if (v0 == 0) break;
    }
}
