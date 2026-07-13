/* func_8001C624 — the only form measured at sandbox 0 (127/127).  This is what
 * is COMMITTED ON MAIN today (plus the Judge-approved setVector respell of the
 * t/u round-trip and the `0 != 0` -> `0` normalisation).
 *
 * *** DO NOT COMMIT THIS.  IT IS NOT A CANDIDATE — IT IS THE REGRESSION. ***
 *
 * s2 (2026-07-13) settled what the two do-while(0) wrappers actually do, and the
 * answer closes the ruling the s1 ledger left open:
 *
 *   sched.c:2066-2088 — "If there is a LOOP_{BEG,END} note in the middle of a
 *   basic block, then we must be sure that no instructions are scheduled across
 *   it."  For the insn following a loop note GCC adds an ANTI dep on EVERY
 *   reg_last_uses[i], a dep on EVERY reg_last_sets[i], sets reg_pending_sets_all
 *   = 1, and calls flush_pending_lists() — an all-register + all-memory fence.
 *
 *   The RTL dump of this exact form shows 0 code_labels and 0 barriers (the
 *   function is ONE basic block) with 2 x LOOP_BEG/LOOP_CONT/LOOP_END notes.
 *   So the wrappers contribute nothing but two total sched1 barriers.
 *
 * That is the FORBIDDEN "scheduling barrier" family (CLAUDE.md lists it beside
 * register pins), NOT the sanctioned do-while(0) exception — whose mechanism is
 * NOTE_INSN_LOOP_BEG -> LABEL_OUTSIDE_LOOP_P -> reorg.c relax_delay_slots, and
 * s1 already showed this function is straight-line so that mechanism is absent.
 * The wrappers must NOT be re-submitted for sanction under do-while-zero-exception.
 *
 * Worse: with our current symbol spelling the barriers are load-bearing for the
 * program's MEANING, not just its schedule.  Remove them and GCC hoists the
 * D_801020C0 <- D_80101FB0 copy above the D_80101FB4/FB8 stores that feed it, so
 * the code copies stale data (see rejected/plain-symbols-wrapper-free-baseline.c).
 * That is the proof that the SOURCE is wrong, not that the wrapper is needed.
 *
 * Where to resume: rejected/plain-symbols-wrapper-free-baseline.c is the clean
 * form to sweep STATEMENT ORDER from (sched.c rank_for_schedule breaks ties by
 * INSN_LUID = source order).  Measure with the reloc-normalised diff, not the raw
 * sandbox score.  Full mechanism + the four killed avenues: hypotheses.md.
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
    do {   /* SCHEDULING BARRIER (sched.c:2066 flush_pending_lists) — forbidden */
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
            do {   /* SCHEDULING BARRIER — forbidden */
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
    } while (0);
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
