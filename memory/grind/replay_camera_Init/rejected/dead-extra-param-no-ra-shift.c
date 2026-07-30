/* REJECTED (s1, 2026-07-30) — KILLED as a lever for the missing
 * `addu $a3,$a1,$zero` parameter copy.
 *
 * Idea: our .greg gives the three global allocnos $a0/$a1/$a2; target uses
 * $a0/$a3/$t0 — a uniform +2 shift, which means hard regs $a1 and $a2 were
 * excluded (conflicting) for the a1-valued allocno in the original compile.
 * Declaring additional parameters would make more incoming argument registers
 * live at function entry, and any allocno born there would conflict with them.
 *
 * MEASURED: sandbox --disable all -> score 17, build_insns 36. Completely
 * unchanged from the two-parameter form.
 *
 * WHY IT CANNOT WORK: a declared-but-unused parameter's copy insn is dead, and
 * `flow` deletes it before global.c builds the conflict graph — so the hard reg
 * is never in any live set and contributes no conflict. Adding a *used* extra
 * parameter is not available either: the in-file caller
 * special_camera_check_pos_outside_ground_80036E34 passes exactly two arguments.
 *
 * Still-open (NOT killed by this): C shapes that keep the incoming $a1 hard
 * register live PAST the birth of the pseudo that holds its value. That is a
 * different mechanism from merely declaring more parameters.
 */
s32 replay_camera_Init(s32 a0, s32 a1, s32 a2) {
    s32 sval;

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
    D_80101E62 = 2;
    D_80101E9E = 0;
    D_80101E78 = (u32)(D_80101E70 + 0x7FF) >> 11;
    return 1;
}
