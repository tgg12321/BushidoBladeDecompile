/* func_8001C624 - best measured form, session s2 (structural).
 *
 * sandbox --disable all = 0 (127/127 insns), MEASURED THIS SESSION.
 *
 * Delta vs HEAD: the flagged t/u "dead temp" load/store round-trip is GONE,
 * respelled as the era-idiomatic comma-assign setVector expansion. The target
 * bytes genuinely contain the self-copy stores (asm/6CAC.s:5101+5103 lw/sw
 * 0x10($sp); 5102+5117 lw/sw 0x18($sp)) - `local` is address-taken (passed to
 * func_80021D10), so GCC keeps the stores. This spelling reproduces them with
 * no dead temporaries. Score stays 0.
 *
 * STILL PENDING A RULING: the two nested do-while(0) wrappers. Measured
 * load-bearing this session (see evidence.md s2):
 *   - inner wrapper removed   -> 69
 *   - outer wrapper removed   -> 28
 *   - 15 single-level spans   -> best 18, none 0
 *   - the nested pair is the ONLY measured geometry at 0
 * Mechanism: their loop notes fence sched1's basic block. With no fence, GCC's
 * alias analysis (alias.c canon_rtx resolves the block-move's base register
 * back to its SYMBOL_REF) proves the type-punned struct copies independent of
 * the neighbouring per-symbol stores, and hoists each copy ABOVE the stores
 * that feed it. Anchoring the neighbours as offsets off the copied base symbol
 * makes that dependence real, but changes the DAG so the target schedule is
 * unreachable (44 with this geometry, 75 wrapper-free) - see
 * rejected/base-anchored-alias-fix-changes-dag.c.
 *
 * If the owner sanctions the wrappers under do-while-zero-exception (FINAL
 * 2026-07-06), they need inline FAKE annotations and the `0 != 0` normalised
 * to `0`. Do NOT commit this form before that ruling.
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
    do {
        a1 = ((s32 *)&D_80101EC8)[0x36];
        v1 = D_80101FA4;
        a0_val = D_80101FA8;
        D_80101FB4 = -0x384;
        D_80101FB8 = 0;
        D_80101FBC = a1;
        D_80101FC0 = v1 - 0x384;
        D_80101FC4 = a0_val;
        D_80101F80 = a1;
        {
            D_80101F84 = v1;
            D_80101F88 = a0_val;
            do {
                *((Blk16 *)&D_80101F90) = *((Blk16 *)&D_80101F80);
                *((Blk12 *)&D_801020C0) = *((Blk12 *)&D_80101FB0);
                D_80101FCC = 0;
                D_80101FD0 = 0;
            } while (0);
            D_80101FD4 = 0;
            *((Blk16 *)&D_80102114) = *((Blk16 *)&D_80101FCC);
            local[0] = local[0], local[1] = local[1] - 0x384, local[2] = local[2];
            D_80101FDC = 0;
            D_80101FE0 = 0;
        }
        D_80101FE4 = 0;
        D_80101FEC = 0;
        D_80101FF0 = 0;
        D_80101FF4 = 0;
    } while (0 != 0);
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
