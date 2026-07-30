/* s8 REJECTED — shared-end-label / goto shape (`ret = 0; goto end; ... ret = 1;
 * end: return ret;`), mirroring target's `j .L80036E2C` into a common `jr $ra`.
 * It is the only s8 shape that reaches target's 39 instructions from the
 * 38-instruction basin, but scores 25 / 39: the `ret` pseudo is what the extra
 * instruction pays for, and it disorders the whole tail.  The two-return spelling
 * in candidate.c already produces target's `j`/`addu $v0,$zero,$zero` epilogue.
 * Measured s8: sweep_results.json (r5). */
s32 replay_camera_Init(s32 a0, s32 a1) {
    s16 *pe62 = &D_80101E62;
    s32 *pe70 = &D_80101E70;
    s32 sval;
    s32 reloaded;
    s32 ret;

    if (*pe62 != 0) {
        ret = 0;
        goto end;
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
    ret = 1;
end:
    return ret;
}
