/* REJECTED (s4) — the ws4 permuter campaign's BEST find (permuter score 235,
 * from ~23,100 iterations seeded on the 39-instruction v_f chassis).
 *
 * sandbox --disable all -> 21 / build_insns 38: the WORST score measured
 * anywhere in this grind, against a floor of 13.  This is the concrete proof
 * of s4 H12 — the permuter's weighted objective (registers x 5, reorderings
 * x 60, ins/del x 100) is ANTI-CORRELATED with the engine sandbox on this
 * function, because the entire remaining residue is register naming, which the
 * permuter prices at 5 points and the sandbox at full weight.  The permuter
 * rates candidate.c (the score-13 floor) at 650.
 *
 * It is also not legal C: `pe70` is READ while uninitialised (the permuter sank
 * its initialiser below the read).  Kept purely as the measured data point.
 */
s32 replay_camera_Init(s32 a0, s32 a1) {
    s32 sval;
    s16 *pe62 = &D_80101E62;
    s32 *pe70;
    s32 reloaded;
    s32 new_var;

    if (*pe62 != 0) {
        return 0;
    }

    new_var = ((s32)(a0 << 16)) >> 13;
    sval = new_var;
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
    pe70 = &D_80101E70;
    new_var = (u32)(reloaded + 0x7FF) >> 11;
    sval = 0;
    *pe62 = 2;
    D_80101E9E = sval;
    D_80101E78 = new_var;
    return 1;
}
