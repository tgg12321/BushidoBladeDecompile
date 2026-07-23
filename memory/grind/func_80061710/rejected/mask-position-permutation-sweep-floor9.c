/* func_80061710 — grind s3 (STRUCTURAL modality) — mask-position permutation sweep.
 *
 * KILLED: four genuinely-untried mask-position permutations, ALL sandbox 9
 * (pin-free, 46/46, 0 rules). None in the s2 rejected bank or the WIP list.
 * Together with s2 (V7 leading-block = 6, middle-load block-local split = 7)
 * this EXHAUSTIVELY maps the mask-position permutation space for 710.
 *
 * MECHANISM (confirmed decisively this session): mask must DIE BEFORE load1 for
 * the mask pseudo and the 3-load web to reuse $v0 (V7 = floor 6). Any mask
 * position at/after load1 leaves the interleaved mask live-across a load, so
 * GCC 2.7.2 local-alloc gives the longer-lived pseudo the lower reg ($v0),
 * pushing the loads to $v1 = the full v0<->v1 rename wall = 9. Interleave
 * (target schedule) and mask=$v1 are mutually exclusive in pure structure.
 *
 * The four forms measured (each replaces the tail after func_80060A68()):
 *
 *   [C] mask split across load2 (lui-position-mimic):          -> 9
 *       t=arg0[0]; D_800F1140=t; mask=0x10FF10;
 *       t=arg0[1]; D_800F1144=t; D_800A3464=mask; t=arg0[2]; D_800F1148=t;
 *
 *   [A] mask atomic block after load1:                          -> 9
 *       t=arg0[0]; D_800F1140=t; mask=0x10FF10; D_800A3464=mask;
 *       t=arg0[1]; D_800F1144=t; t=arg0[2]; D_800F1148=t;
 *
 *   [E] compute-early / store-late (mask live across all loads):-> 9
 *       mask=0x10FF10; t=arg0[0]; D_800F1140=t; t=arg0[1]; D_800F1144=t;
 *       D_800A3464=mask; t=arg0[2]; D_800F1148=t;
 *
 *   [HEAD-posn] mask atomic after load2 (baseline pin-free):    -> 9
 *
 * The ONLY sub-9 sanctioned form remains V7 (mask leading block, = 6,
 * rejected/v7-mask-atomic-first-floor6.c). No structural form reaches a
 * committable 0; the sole 0-basin is the constant-staging copy-preference
 * coercion (candidate.c, POLICY-BLOCKED, owner-gated under func_80061658's
 * SOTN census which explicitly names 710).
 *
 * Representative form [C] shown below (all four are one-line reorderings).
 */
void func_80061710(s32 *arg0, s32 arg1) {
    s32 *v1 = (s32 *)&D_800F116C;
    s32 t;
    s32 mask;
    u8 *p;
    s32 val;
    D_800A3468 = (s32)v1;
    D_800F1178 = (s32)arg0;
    switch (arg1) {
    case 0:
        val = 0x21000E;
        p = &D_800F115C + 2;
        *p = 0;
        D_800F1180 = (s32)p;
        *v1 = val;
        break;
    case 1:
        val = 0x21000F;
        p = &D_800F115C + 3;
        *p = 0;
        D_800F1180 = (s32)p;
        *v1 = val;
        break;
    }
    func_80060A68();
    t = arg0[0]; D_800F1140 = t;
    mask = 0x10FF10;
    t = arg0[1]; D_800F1144 = t;
    D_800A3464 = mask;
    t = arg0[2]; D_800F1148 = t;
}
