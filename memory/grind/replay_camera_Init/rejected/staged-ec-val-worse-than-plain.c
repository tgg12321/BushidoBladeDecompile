/* permuter find ws2/output-625-1 (base 860 -> 625), de-permuted.
 * Delta vs v_a: the D_8008EC38 load is staged through `reloaded` (the same
 * local that later holds the re-read of D_80101E70) before being assigned to
 * ec_val -- staged-value-reused-variable. */
s32 replay_camera_Init(s32 a0, s32 a1) {
    s32 sval;
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
        reloaded = *(s32 *)((u8 *)&D_8008EC38 + sval);
        D_80101E7C = a1;
        ec_val = reloaded;
        D_80101E70 = ec_val;
    }
    D_80101E68 = 0;
    D_80101E62 = 2;
    reloaded = *pe70;
    D_80101E9E = 0;
    D_80101E78 = (u32)(reloaded + 0x7FF) >> 11;
    return 1;
}
