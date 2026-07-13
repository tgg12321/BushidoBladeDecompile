/* REJECTED (s2) - single do-while(0) level is INSUFFICIENT (best of 15 spans).
 *
 * This is the best single-wrapper form found: one do-while(0) spanning the two
 * struct copies through the local round-trip (P1..RT). Score 18, not 0.
 * Full sweep on the matching body (pun symbols + comma round-trip):
 *   nested pair (committed geometry)  ->  0   <-- only geometry at 0
 *   P1..RT (this file)                -> 18
 *   P1..Z2                            -> 18
 *   inner only (P1..K2)               -> 28
 *   P1..K3 -> 28 | P1..P3 -> 33 | F2..K2 -> 49 | F1..K2 -> 55 | P2..K2 -> 59
 *   F1..Z6 -> 60 | L1..K2 -> 65 | outer only (L1..Z6) -> 69 | B1..K2 -> 70
 *   L1..RT -> 72 | L1..Z2 -> 74 | L1..P3 -> 76
 * This is the "single-level-insufficient" evidence the do-while-zero-exception
 * rule demands before a NESTED wrapper may be considered.
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
    D_80101FB4 = -0x384;
    D_80101FB8 = 0;
    D_80101FBC = a1;
    D_80101FC0 = v1 - 0x384;
    D_80101FC4 = a0_val;
    D_80101F80 = a1;
    D_80101F84 = v1;
    D_80101F88 = a0_val;
    do {
        *((Blk16 *)&D_80101F90) = *((Blk16 *)&D_80101F80);
        *((Blk12 *)&D_801020C0) = *((Blk12 *)&D_80101FB0);
        D_80101FCC = 0;
        D_80101FD0 = 0;
        D_80101FD4 = 0;
        *((Blk16 *)&D_80102114) = *((Blk16 *)&D_80101FCC);
        local[0] = local[0], local[1] = local[1] - 0x384, local[2] = local[2];
    } while (0);
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
    D_80102010 = D_80101F84;
    func_8003FFE0(0);
}
