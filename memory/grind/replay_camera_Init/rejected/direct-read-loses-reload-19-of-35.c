/* p725's statement ordering, but the final read of D_80101E70 is DIRECT
 * (no pointer).  Isolates the reordering from the pointer-mediated reload. */
s32 replay_camera_Init(s32 a0, s32 a1) {
    s32 sval;
    s32 new_var2;
    s32 *new_var3;
    extern u8 SpecialCam;

    if (D_80101E62 != 0) {
        return 0;
    }

    sval = ((s32)(a0 << 16)) >> 13;
    D_80101E60 = a0;
    {
        s32 cam_val;
        s32 ec_val;
        cam_val = *(s32 *)((u8 *)&SpecialCam + sval);
        D_80101E6C = cam_val;
        D_80101E68 = 0;
        new_var3 = (s32 *)((u8 *)&D_8008EC38 + sval);
        D_80101E7C = a1;
        ec_val = *new_var3;
        D_80101E70 = ec_val;
    }
    D_80101E62 = 2;
    new_var2 = D_80101E70;
    D_80101E9E = 0;
    D_80101E78 = (u32)(new_var2 + 0x7FF) >> 11;
    return 1;
}
