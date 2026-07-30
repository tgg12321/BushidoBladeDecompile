/* REJECTED (s2, 2026-07-30) — "target statement order" and the whole
 * statement-ordering / declaration-order / type-narrowing family.
 *
 * This form writes the statements in exactly the order target's assembly
 * executes them (both loads, then D_80101E6C, then D_80101E70, then
 * D_80101E7C, then the tail). It is the most natural-looking spelling and
 * it is measurably WORSE than candidate.c in BOTH regimes:
 *
 *   form                      --disable all      --disable all --keep-cheat-asm
 *   candidate.c (E7C between
 *   the two loads)            17 / 36 insns      14 / 38 insns   <-- floor
 *   this form (target order)  18 / 37 insns      23 / 40 insns
 *
 * Seven sibling orderings were measured in the same sweep
 * (tmp/grind/replay_camera_Init/s2/sweep_results*.json); none beat 17/14:
 *   v02 reload-into-local            18 / 37   |  23 / 40
 *   v03 D_80101E60 store after loads 22 / 38   |  24 / 40
 *   v04 inverted branch (if ==0 {})  23 / 37   |  27 / 39
 *   v05 pointer locals for the loads 18 / 37   |  23 / 40
 *   v06 == candidate.c               17 / 36   |  14 / 38
 *   v07 D_80101E7C store last        19 / 37   |  23 / 40
 *   v08 (s16) narrowing for sval     18 / 37   |  23 / 40
 *
 * Do not re-derive: the ordering axis is measured dead. candidate.c's
 * placement of `D_80101E7C = a1;` between the SpecialCam load and the
 * D_8008EC38 load is the local optimum and every neighbour is worse.
 */
s32 replay_camera_Init(s32 a0, s32 a1) {
    extern u8 SpecialCam;
    s32 sval;
    s32 cam_val;
    s32 ec_val;

    if (D_80101E62 != 0) {
        return 0;
    }

    sval = ((s32)(a0 << 16)) >> 13;
    D_80101E60 = a0;
    cam_val = *(s32 *)((u8 *)&SpecialCam + sval);
    ec_val = *(s32 *)((u8 *)&D_8008EC38 + sval);
    D_80101E6C = cam_val;
    D_80101E70 = ec_val;
    D_80101E7C = a1;
    D_80101E68 = 0;
    D_80101E62 = 2;
    D_80101E9E = 0;
    D_80101E78 = (u32)(D_80101E70 + 0x7FF) >> 11;
    return 1;
}
