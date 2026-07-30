/* s8 REJECTED — the honest array-declaration trick that WORKS for D_80101E62
 * does NOT work for D_80101E70.  With src/code6cac_b2_post.c:45 changed to
 * `extern s32 D_80101E70[];` and both the store and the re-read written as
 * `D_80101E70[0]`, the score is 17 / 36 — the reload is GONE.  An index-0 array
 * access folds to the same `(mem (symbol_ref))` rtx as the scalar, so cse.c's
 * store-to-load forwarding matches it and eats the load.  Only a `(mem (reg))`
 * read — i.e. the /* FAKE *\/ `s32 *pe70` pointer local — defeats it.
 * Measured s8: tmp/grind/replay_camera_Init/s8/arraytest_results2.json (a2). */
s32 replay_camera_Init(s32 a0, s32 a1) {
    s32 sval;
    s32 reloaded;

    if (D_80101E62[0] != 0) {
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
        D_80101E70[0] = ec_val;
    }
    D_80101E68 = 0;
    D_80101E62[0] = 2;
    reloaded = D_80101E70[0];
    D_80101E9E = 0;
    D_80101E78 = (u32)(reloaded + 0x7FF) >> 11;
    return 1;
}
