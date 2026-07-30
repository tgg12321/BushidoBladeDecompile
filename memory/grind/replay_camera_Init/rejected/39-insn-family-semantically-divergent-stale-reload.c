/* REJECTED (s4) — best member of the 39-instruction family, and the proof that
 * the whole family is a dead end.
 *
 * sandbox --disable all -> 17 / build_insns 39 (target 39).  Four points WORSE
 * than the 13 / 38 floor in candidate.c.
 *
 * Two independent reasons it is dead:
 *
 * 1. SEMANTIC DIVERGENCE (s4 H15).  The emitted object performs the
 *    `reloaded = *pe70;` re-read BEFORE the `D_80101E70 = ec_val;` store it is
 *    supposed to observe: ... lw v1 / lw a1 / sw v1(E6C) / lui a0; lw a0,0(a0)
 *    <- THE RE-READ / li v1,2 / sh v1,0(a3) / ... / lui at; sw a1,0(at) <- the
 *    E70 store, at the very end.  So D_80101E78 is computed from the STALE
 *    value of D_80101E70.  Target stores first and re-reads after.  GCC 2.7.2
 *    may sink the store past the load because (mem (reg)) and
 *    (mem (symbol_ref)) do not alias to its disambiguator — the very property
 *    that makes the re-read survive.  candidate.c does NOT have this defect.
 *
 * 2. It costs more than it buys.  The 39th instruction (the a1 parameter-home
 *    copy) appears because the two loads are ADJACENT (s4 H14), but the extra
 *    live value shifts the register allocation further from target, not closer.
 *
 * What it DID teach: issuing the D_8008EC38 load BEFORE the SpecialCam load
 * puts cam_val in $v1, matching target's `lw v1,%lo(SpecialCam)` — worth -2
 * inside this family and possibly reusable in the 38-instruction family.
 */
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
        ec_val = *(s32 *)((u8 *)&D_8008EC38 + sval);
        cam_val = *(s32 *)((u8 *)&SpecialCam + sval);
        D_80101E6C = cam_val;
        D_80101E70 = ec_val;
    }
    reloaded = *pe70;
    *pe62 = 2;
    D_80101E7C = a1;
    D_80101E68 = 0;
    D_80101E9E = 0;
    D_80101E78 = (u32)(reloaded + 0x7FF) >> 11;
    return 1;
}
