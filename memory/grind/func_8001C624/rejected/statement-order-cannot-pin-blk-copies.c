/* REJECTED (s3, 2026-07-13) — best of 14 statement orders swept over the PLAIN
 * (separate-scalar-symbol) spelling.  Score 58 vs the 61 baseline.  This form is
 * still SEMANTICALLY WRONG (the copies read stale data), so it is not a
 * candidate; it is recorded as the empirical high-water mark of the avenue.
 *
 * THIS KILLS FRONTIER AVENUE #1 (the s2 ledger's headline "un-swept lever").
 *
 * The s2 frontier reasoned: sched.c's rank_for_schedule breaks INSN_PRIORITY
 * ties by INSN_LUID = SOURCE ORDER, therefore source order selects the emitted
 * order.  That is true but IRRELEVANT here, and s3 measured why.
 *
 * MEASUREMENTS (all `sandbox --disable all`, reloc-normalised diff, target=127):
 *   baseline (= the target's own statement order)        61
 *   P2 -> source end            (o1, THIS FILE)          58
 *   P2 -> immediately after S0  (o2)                     61
 *   all three copies -> end     (o3)                     61
 *   loads first                 (z1)                     61
 *   FB0 group contiguous        (z2)                     61
 *   B/C swapped                 (z3)                     67
 *   P2 after D                  (z4)                     61
 *   D before the copies         (z5)                     61
 *   C+P1 hoisted                (z6)                     67
 *   SV/E swapped                (z7)                     61
 *   D first                     (z8)                     61
 *   S0 moved down before P2     (y2)                     61
 *   FB0 group adjacent          (y3)                     61
 *   => range 58-67.  The copies NEVER land correctly in any order.
 *
 * WHY ORDER CANNOT WORK — the mechanism, stated exactly:
 * Under the plain spelling each block copy has EXACTLY ONE dependence: the store
 * to its own base symbol (D_80101FB0 for copy2, D_80101FCC for copy3, D_80101F80
 * for copy1).  The stores at offsets +4/+8 are DIFFERENT symbol_refs, and
 * memrefs_conflict_p's CONSTANT_P branch requires rtx_equal_for_memref_p, so GCC
 * sees no dependence on them at all.  Statement order is only a TIE-BREAKER
 * (rank_for_schedule: INSN_PRIORITY -> class-vs-last_scheduled_insn -> INSN_LUID).
 * A tie-breaker cannot CREATE a dependence.  No permutation of statements can
 * pin an insn that the scheduler believes is unconstrained.
 *
 * The o1 probe is the decisive one: moving copy2 to the very END of the source
 * still emitted it IMMEDIATELY after `sw zero,%lo(D_80101FB0)` — and dragged
 * that store to the TOP of the block with it.  The (store, copy) pair is glued
 * together and floats as a unit, wherever the source puts either half.
 *
 * CONSEQUENCE: the fix must ADD A DEPENDENCE, which means giving the copy and
 * its feeding stores a SHARED BASE SYMBOL.  See candidate.c (aggregate typing —
 * semantically correct, 125/127) and rejected/region-cast-alias-review-is-cheat.c
 * (the cast spelling — byte-exact, but a cheat).
 */
void func_8001C624(void) {
    typedef struct { s32 a, b, c, d; } Blk16;
    typedef struct { s32 a, b, c; } Blk12;
    s32 local[3];
    s32 x;
    s32 y;
    s32 z;
    s32 i;

    i = 0x36;
    func_80021D10(0, &((s32 *)&D_80101EC8)[i], (s32)D_800A38E0);
    func_80021D10(1, local, (s32)D_800A38E0);
    D_80101FB0 = 0;
    x = ((s32 *)&D_80101EC8)[0x36];
    y = D_80101FA4;
    z = D_80101FA8;
    D_80101FB4 = -0x384;
    D_80101FB8 = 0;
    D_80101FBC = x;
    D_80101FC0 = y - 0x384;
    D_80101FC4 = z;
    D_80101F80 = x;
    D_80101F84 = y;
    D_80101F88 = z;
    *((Blk16 *)&D_80101F90) = *((Blk16 *)&D_80101F80);
    D_80101FCC = 0;
    D_80101FD0 = 0;
    D_80101FD4 = 0;
    *((Blk16 *)&D_80102114) = *((Blk16 *)&D_80101FCC);
    local[0] = local[0], local[1] = local[1] - 0x384, local[2] = local[2];
    D_80101FDC = 0;
    D_80101FE0 = 0;
    D_80101FE4 = 0;
    D_80101FEC = 0;
    D_80101FF0 = 0;
    D_80101FF4 = 0;
    *((Blk12 *)&D_801020C0) = *((Blk12 *)&D_80101FB0);   /* moved to source end */
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
