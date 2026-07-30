/* s8 REJECTED — sibling transplant.  func_80036FD4 (same TU, src/code6cac_b2_post.c:325)
 * reads the SpecialCam table as `s32 *entry = (s32 *)(&SpecialCam + idx); entry[0];
 * entry[1];`, exploiting that SpecialCam and D_8008EC38 are adjacent words of one
 * 8-byte table entry.  Transplanted here it scores 24 / 38 (vs candidate.c 13 / 38):
 * one shared address pseudo for both loads conflicts with everything else live,
 * and target does NOT share — it re-materialises `lui $at; addu $at,$at,$v0` for
 * each of the two loads, i.e. two independent symbol+index addresses.
 * Measured s8: sweep_results.json (r4). */
s32 replay_camera_Init(s32 a0, s32 a1) {
    s16 *pe62 = &D_80101E62;
    s32 *pe70 = &D_80101E70;
    s32 sval;
    s32 reloaded;

    if (*pe62 != 0) {
        return 0;
    }

    sval = ((s32)(a0 << 16)) >> 13;
    D_80101E60 = a0;
    {
        extern u8 SpecialCam;
        s32 *entry = (s32 *)((u8 *)&SpecialCam + sval);
        D_80101E6C = entry[0];
        D_80101E7C = a1;
        D_80101E70 = entry[1];
    }
    D_80101E68 = 0;
    *pe62 = 2;
    reloaded = *pe70;
    D_80101E9E = 0;
    D_80101E78 = (u32)(reloaded + 0x7FF) >> 11;
    return 1;
}
