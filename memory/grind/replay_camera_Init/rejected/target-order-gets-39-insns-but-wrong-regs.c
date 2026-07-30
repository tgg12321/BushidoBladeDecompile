/* v_d + target's own statement order, now that the pointer-mediated reload
 * exists: both loads, then both stores, then the re-read, and only THEN
 * `D_80101E7C = a1;` (which is what keeps the incoming a1 live to the end and
 * should force its home copy out of $a1). */
s32 replay_camera_Init(s32 a0, s32 a1) {
    s32 sval;
    s16 *pe62 = &D_80101E62;
    s32 *pe70 = &D_80101E70;
    s32 reloaded;

    if (*pe62 != 0) {
        return 0;
    }

    sval = ((s32)(a0 << 16)) >> 13;
    D_80101E60 = a0;
    {
        extern u8 SpecialCam;
        s32 cam_val;
        s32 ec_val;
        cam_val = *(s32 *)((u8 *)&SpecialCam + sval);
        ec_val = *(s32 *)((u8 *)&D_8008EC38 + sval);
        D_80101E6C = cam_val;
        D_80101E70 = ec_val;
    }
    reloaded = *pe70;
    D_80101E7C = a1;
    D_80101E68 = 0;
    *pe62 = 2;
    D_80101E9E = 0;
    D_80101E78 = (u32)(reloaded + 0x7FF) >> 11;
    return 1;
}
