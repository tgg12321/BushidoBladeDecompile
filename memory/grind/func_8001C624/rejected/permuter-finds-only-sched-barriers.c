/* REJECTED — s4, 2026-07-13.  THE PERMUTER BASIN OVER THE PLAIN CHASSIS IS A
 * SCHEDULING-BARRIER BASIN.  Do not re-run a naive permuter campaign on this
 * function expecting an honest closer; it converges on the banned construct.
 *
 * CAMPAIGN (tools/permuter_campaign.py, telemetry in metrics/events.jsonl):
 *   workspace  tmp/grind/func_8001C624/s4/perm_a   (label "plain-scalars")
 *   chassis    the plain-scalar wrapper-free baseline (= s4 candidate.c)
 *   base score 5615      iterations 27,651      finds 1,517      best 875
 *
 * The workspace is a MINIMAL TU whose codegen is verified byte-identical (objdump
 * -dr, relocs included) to the full code6cac.c TU — see mkws_mini.sh's
 * context-equivalence gate.  target.o is assembled straight from asm/6CAC.s, so the
 * relocs match the plain chassis and the score is honest (no base+offset pollution).
 *
 * ============================ THE RESULT ============================
 * Triaged all 1,517 finds by construct (tmp/grind/func_8001C624/s4/triage.py):
 *
 *   best overall             875   do-while(0) BARRIER
 *   ... every one of the top ~14 finds is a do-while(0) barrier ...
 *   best CLEAN find         3035   (wrapper-free, no volatile/asm/register-pin,
 *                                   and both calls left in their mandated positions)
 *
 * A 3.5x gap.  The barrier family MONOPOLISES the entire top of the distribution.
 *
 * TWO SPELLINGS OF THE SAME CHEAT.  Permuter's improvements come from exactly two
 * moves, and they are the same move:
 *   1. do{...}while(0) around a statement group.  In this STRAIGHT-LINE function a
 *      loop note is nothing but a sched.c:2066 total barrier (ANTI dep on every
 *      reg_last_uses[i], dep on every reg_last_sets[i], reg_pending_sets_all=1,
 *      flush_pending_lists()).  s2 killed this for sanction: the do-while-zero
 *      exception's mechanism is reorg.c/LABEL_OUTSIDE_LOOP_P, which is ABSENT here.
 *   2. RELOCATING A CALL (moving `func_80021D10(1, local, ...)` down, or hoisting
 *      `func_8003FFE0(0)` up into the body).  A call is also a flush_pending_lists
 *      barrier — and moving it is a SPEC change, not a respelling.  The naive
 *      "wrapper-free" filter misses this; triage.py's semantic filter catches it.
 *
 * WHY THIS MATTERS: this is exactly how the committed regression got made.  The s1
 * permuter session took the top find, and the top find is always a barrier.  The
 * search space genuinely contains nothing else — which is itself the evidence that
 * no pure-C STATEMENT-level rearrangement fixes the schedule, and that the problem
 * lives in the DECLARATIONS (where s4 proved the honest space is empty — see
 * rejected/aggregate-any-spelling-loses-offset0-atform.c).
 */

/* The best find (score 875) — a do-while(0) barrier.  Recorded, NOT proposed. */
void func_8001C624(void) {
    typedef struct { s32 a, b, c, d; } Blk16;
    typedef struct { s32 a, b, c; } Blk12;
    s32 local[3];
    s32 x, y, z, i;

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
    /* CHEAT — sched.c:2066 total scheduling barrier wearing a do-while(0) costume. */
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
