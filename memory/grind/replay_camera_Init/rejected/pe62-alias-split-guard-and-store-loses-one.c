/* q08_guard_symbol_store_ptr — guard reads the SYMBOL, store goes through pe62 */
s32 replay_camera_Init(s32 a0, s32 a1) {
    s32 sval;
    /* FAKE - pointer-alias-fake-exception */
    s16 *pe62 = &D_80101E62;
    /* FAKE - pointer-alias-fake-exception */
    s32 *pe70 = &D_80101E70;
    s32 reloaded;

    if (D_80101E62 != 0) {
        return 0;
    }

    sval = ((s32)(a0 << 16)) >> 13;
    D_80101E60 = a0;
    {
        extern u8 SpecialCam;
        s32 cam_val;
        s32 ec_val;
        cam_val = *(s32 *)((u8 *)&SpecialCam + sval);
        D_80101E6C = cam_val;
        D_80101E7C = a1;
        ec_val = *(s32 *)((u8 *)&D_8008EC38 + sval);
        D_80101E70 = ec_val;
    }
    D_80101E68 = 0;
    *pe62 = 2;
    reloaded = *pe70;
    D_80101E9E = 0;
    D_80101E78 = (u32)(reloaded + 0x7FF) >> 11;
    return 1;
}
