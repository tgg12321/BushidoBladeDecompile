/* s9 REJECTED — "the third declared parameter is HONEST because some caller
 * really passes three arguments."  REFUTED BY A NEGATIVE CALL-SITE CENSUS.
 *
 * s7 measured that a consumed third parameter is the only shape that reproduces
 * BOTH of target's residual register names at once (`addu $a3,$a1,$zero` in the
 * bnez delay slot AND the D_80101E62 address in $t0), and rejected it on two
 * grounds, the first of which was ASSERTED rather than measured: "the asm call
 * sites pass two arguments".  This session measured it.
 *
 * CENSUS (all 13 `jal replay_camera_Init` sites in the shipped assembly, six
 * instructions of argument setup read at each):
 *   func_80016A8C, func_80020DDC, func_8005B7C4, func_8005B8B8 (x2),
 *   func_8005BA8C, func_8005FBC8, func_800602AC, func_80060CB8, func_8006E10C,
 *   func_8006E950, obj_InitTaskCamera, DispPracticeMenuTex_B,
 *   special_camera_check_pos_outside_ground_80036E34.
 *
 * RESULT: every single site sets up $a0 and $a1 only.  Not one writes $a2 or
 * $a3 before the call.  The most telling site is
 * special_camera_check_pos_outside_ground_80036E34, which MOVES its own
 * incoming $a2/$a3 into $s1/$s2 *before* the jal — i.e. it treats those
 * registers as its own state to preserve across the call, not as arguments it
 * is forwarding.  A three-parameter replay_camera_Init would be reading
 * uninitialised registers at every one of its 13 call sites.
 *
 * CONSEQUENCE: the third parameter is a dead-value coercion by construction,
 * exactly as s7's ground (2) already showed it does not pay (14/39 basin best
 * vs the 13/38 floor at the time).  Both grounds are now measured.  This closes
 * the last open register-naming avenue in the ledger.
 *
 * SUPERSEDED IN PRACTICE: the function reached honest distance 0 this session
 * by an entirely different route — the `struct CamPair` 8-byte record copy —
 * which produces target's $a3 home copy and $t0 address for free, with a
 * two-parameter signature.  See memory/grind/replay_camera_Init/candidate.c.
 * This file is banked so no future session re-opens the third-parameter idea.
 */
s32 replay_camera_Init(s32 a0, s32 a1, s32 a2) {
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
    *pe62 = 2;
    D_80101E7C = a1;
    D_80101E68 = a2;   /* the fabricated consumer — no caller supplies this */
    D_80101E9E = 0;
    D_80101E78 = (u32)(reloaded + 0x7FF) >> 11;
    return 1;
}
