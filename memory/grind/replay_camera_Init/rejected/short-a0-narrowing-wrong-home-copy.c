/* REJECTED (s4 H16) — `short a0` parameter narrowing (from ws5 permuter find
 * output-380-1), applied as a single-token delta to candidate.c.
 *
 * sandbox --disable all -> 14 / build_insns 39 (target 39).
 *
 * It is the closest thing to a clean 39-instruction form found anywhere: it
 * reaches target's instruction count, keeps the D_80101E70 re-read AFTER the
 * store (unlike the v_f/h8 family, which is semantically divergent), and costs
 * only +1 over the 13 floor.  It is still dead:
 *
 *  - The 39th instruction is `move v1,a0` — a TRUNCATION home copy of the
 *    FIRST parameter — not target's `move a3,a1` parameter home copy.  Wrong
 *    instruction in the right slot.
 *  - Target's own assembly disproves the narrowing: it stores the parameter
 *    with `sh a0,0(at)`, straight from the incoming argument register with no
 *    truncating copy at all.  That is precisely what an un-narrowed `s32`
 *    parameter emits; a `short` parameter cannot produce it.
 *
 * Do not re-propose parameter narrowing for this function.
 */
s32 replay_camera_Init(short a0, s32 a1) {
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
