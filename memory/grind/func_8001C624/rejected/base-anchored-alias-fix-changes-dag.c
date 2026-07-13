/* REJECTED (s2) - base-symbol anchoring: fixes the FALSE independence, but
 * changes the dependence DAG so the target schedule becomes unreachable.
 *
 * Idea: the six neighbour symbols (D_80101FB4/FB8, D_80101F84/F88,
 * D_80101FD0/FD4) are written as offsets off the base symbol that the adjacent
 * struct copy reads, so GCC sees a REAL true-dependence (overlapping offsets
 * from one SYMBOL_REF) instead of two unrelated symbols. This provably works:
 * with anchoring the copies no longer hoist above the stores that feed them,
 * and NO wrapper is needed to hold them down. Instruction SET and shapes are
 * exactly right (127/127; at-form field stores, correct block-copy idiom).
 *
 * WHY IT IS DEAD: the new edges also re-shape the whole block DAG, and the
 * scheduler's canonical order for it is not the target's.
 *   wrapper-free (this file)          -> 75
 *   + the committed nested geometry   -> 44
 *   + 12 other wrapper placements     -> best 47
 *   + 12 statement-order permutations -> 11 of 12 identical at 75 (order is INERT)
 * The copies P2 (FB0->1020C0) and P3 (FCC->102114) have no dependence on the
 * load-fed F80/FBC groups, so no C-level edge can order them after those
 * groups; only a block fence can. Do not re-propose anchoring as a wrapper
 * replacement.
 */
void func_8001C624(void) {
    typedef struct { s32 a, b, c, d; } Blk16;
    typedef struct { s32 a, b, c; } Blk12;
    s32 local[3];
    s32 a1;
    s32 v1;
    s32 a0_val;

    v1 = 0x36;
    func_80021D10(0, &((s32 *)&D_80101EC8)[v1], (s32)D_800A38E0);
    func_80021D10(1, local, (s32)D_800A38E0);
    D_80101FB0 = 0;
    a1 = ((s32 *)&D_80101EC8)[0x36];
    v1 = D_80101FA4;
    a0_val = D_80101FA8;
    ((s32 *)&D_80101FB0)[1] = -0x384;
    ((s32 *)&D_80101FB0)[2] = 0;
    D_80101FBC = a1;
    D_80101FC0 = v1 - 0x384;
    D_80101FC4 = a0_val;
    D_80101F80 = a1;
    ((s32 *)&D_80101F80)[1] = v1;
    ((s32 *)&D_80101F80)[2] = a0_val;
    *((Blk16 *)&D_80101F90) = *((Blk16 *)&D_80101F80);
    *((Blk12 *)&D_801020C0) = *((Blk12 *)&D_80101FB0);
    D_80101FCC = 0;
    ((s32 *)&D_80101FCC)[1] = 0;
    ((s32 *)&D_80101FCC)[2] = 0;
    *((Blk16 *)&D_80102114) = *((Blk16 *)&D_80101FCC);
    local[0] = local[0], local[1] = local[1] - 0x384, local[2] = local[2];
    D_80101FDC = 0;
    D_80101FE0 = 0;
    D_80101FE4 = 0;
    D_80101FEC = 0;
    D_80101FF0 = 0;
    D_80101FF4 = 0;
    D_80101FFC = 0;
    D_80102000 = 0;
    D_80102004 = 0;
    D_8010200C = 0;
    D_80102014 = 0;
    D_80102018 = 0;
    D_8010201A = 0;
    D_80102016 = 0;
    D_80102010 = ((s32 *)&D_80101F80)[1];
    func_8003FFE0(0);
}
