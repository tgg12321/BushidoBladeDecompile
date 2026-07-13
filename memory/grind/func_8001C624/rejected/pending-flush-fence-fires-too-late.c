/* REJECTED (s3, 2026-07-13) — score 89, 125/127 insns.
 *
 * THIS KILLS FRONTIER AVENUE #2 (sched.c's pending-list flush), by CONFIRMING
 * the mechanism is real and then showing it is unusable for THIS function.
 *
 * THE MECHANISM (sched.c:1754, `if (pending_lists_length > 32)
 * flush_pending_lists (insn)`).  flush_pending_lists makes the flushing insn
 * depend on every pending read and write and sets last_pending_memory_flush,
 * which every subsequent memory op then depends on — a TOTAL memory fence that
 * GCC creates by itself, with no coercion construct.  Its position is determined
 * purely by the COUNT of memory refs since the last flush (calls also flush).
 *
 * THE PROBE (this file).  Source order S0,L,A,B,C,F,E,SV,D,P1,P2,P3 — the F, E
 * and SV groups are hoisted ABOVE the three copies, stacking ~34 memory ops
 * ahead of them so the flush provably fires before copy1.
 *
 * THE RESULT.  It WORKS, exactly as predicted: in the emitted stream all three
 * copies' base registers are now materialised AFTER every store that feeds them
 *   `sw v0,%lo(D_80101FB4)` / `sw zero,%lo(D_80101FB0)` / `sw zero,%lo(D_80101FB8)`
 *   ... then ... `addiu a2,a2,%lo(D_80101FB0)`  <- copy2's base, AFTER all three
 * The copies stop floating.  The fence is genuine and honest.
 *
 * WHY IT IS USELESS ANYWAY.  The fence only exists BECAUSE 33+ memory ops were
 * pushed ahead of the copies — which is precisely the store order the target
 * does NOT have.  In the target's own byte order only 12 memory ops precede
 * copy1 (1 store + 3 loads + 8 stores), less than half the threshold.  So:
 *
 *   the flush can be made to fence the copies ONLY by adopting a store order
 *   that is already wrong.  There is no source order that both (a) puts >32
 *   memory ops before the copies and (b) emits the target's store sequence.
 *
 * Hence score 89 — WORSE than the 61 baseline: the copies are fixed and
 * everything else is destroyed.  The lever is real, measured, and dead.
 *
 * (Corollary worth keeping: the flush DOES fire in the normal form, around the
 * D_80101FDC..FF4 group — memory op ~33.  That is why the setVector stack stores
 * land AFTER those six zero stores in both the target and every build we make.
 * That part of the schedule is the flush doing its job, and it is already right.)
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
    /* --- F, E, SV hoisted here purely to stack >32 memory refs --- */
    D_80101FFC = 0;
    D_80102000 = 0;
    D_80102004 = 0;
    D_8010200C = 0;
    D_80102014 = 0;
    D_80102018 = 0;
    D_8010201A = 0;
    D_80102016 = 0;
    D_80102010 = D_80101F84;
    D_80101FDC = 0;
    D_80101FE0 = 0;
    D_80101FE4 = 0;
    D_80101FEC = 0;
    D_80101FF0 = 0;
    D_80101FF4 = 0;
    local[0] = local[0], local[1] = local[1] - 0x384, local[2] = local[2];
    D_80101FCC = 0;
    D_80101FD0 = 0;
    D_80101FD4 = 0;
    /* --- the flush has now fired; the copies are fenced --- */
    *((Blk16 *)&D_80101F90) = *((Blk16 *)&D_80101F80);
    *((Blk12 *)&D_801020C0) = *((Blk12 *)&D_80101FB0);
    *((Blk16 *)&D_80102114) = *((Blk16 *)&D_80101FCC);
    func_8003FFE0(0);
}
