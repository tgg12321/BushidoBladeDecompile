/* func_80041188 - s18 (rederive) form "V7".  sandbox --disable all == 8,
 * 132 target / 133 build insns.  ALL-TARGET callee-saved seats AND target's
 * block-2 insn shape (`addiu $s3,$s7,0x20`) at the same time - the first form
 * in 18 sessions to have both.  NO FAKE chain-extender: `stptr` is the honest
 * single statement `stptr = base + 0xFC;`.
 *
 * MECHANISM (the s18 lever).  flow.c:2081 counts `reg_n_refs[regno] +=
 * loop_depth`, and `loop_depth` is the NOTE nesting depth (flow.c:440-471 sets
 * basic_block_loop_depth from NOTE_INSN_LOOP_BEG/END; flow.c:1453 aborts at 0,
 * so the no-note baseline is 1).  A `do { ... } while (0);` wrap emits
 * LOOP_BEG/LOOP_END notes without a back edge, so every reference INSIDE the
 * wrap is counted TWICE by flow while loop.c leaves the region alone (no giv
 * rewrite - contrast the real do-while in rejected/loop1-real-dowhile-...).
 * This is the ONLY construct in this compiler that produces an ODD reference
 * delta: every other byte-free lift is a combine-deleted set+use chain, which
 * is quantised to +2 (evidence.md E-s18-3).  It makes s15's Z0 inverse-solver
 * vector #1 (tbl 4->5 AND out2 3->4) spellable for the first time.
 *
 * MEASURED ALLOCDBG table (instrumented cc1, s17/dump17.sh V7) - every seat is
 * target's:  88 stptr 6/41 = 2926 -> $s3 . 91 stptr2 6/48 = 2500 -> $s0 .
 * 78 i 8/97 = 2474 -> $s4 . 79 tbl 5/47 = 2127 -> $s5 . 86 out2 4/42 = 1904 ->
 * $s6 . 77 pa4 7/95 = 1473 -> $s7 . 75 a3 4/99 = 808 -> $fp . 87 out3 3/47 ->
 * $s3 . 85 saved spilled (target spills it too).
 *
 * THE ONE RESIDUAL: 133 insns, not 132.  The extra insn is a load-delay `nop`
 * in the a2 half of loop1: with the out2 wrap in place the scheduler hoists
 * `offset = offset + (s32) a2;` ABOVE the first jal (target keeps it after),
 * and `i++` then lands before `lhu $v0,0($s0)` instead of filling its delay
 * slot.  Wrap placement is therefore a scheduling constraint as well as a
 * reference-count dial - see evidence.md E-s18-5 for the placements measured
 * (T1/T3 cost 0 insns, T2 costs 2, U6 costs 0 but breaks the a1/a2 tie, U8
 * costs 3).  Closing that single nop is the s19 frontier.
 */
void func_80041188(s32 a0, u8 *a1, u8 *a2, s32 a3, s32 *a4)
{
    s32 *pa4 = a4;
    s32 i = 1;
    s32 *tbl = D_80094CFC;
    s32 base = D_800A9A10[a0];
    s16 buf[3];
    s32 saved;
    s32 *out2;
    s32 *out3;
    s32 stptr;
    s32 offset;
    u16 *p;
    s32 stptr2;
    saved = base + 0x94;
    out2 = (s32 *) (((u8 *) pa4) + 0x20);
    stptr = base + 0xFC;
        loop1:
    do { offset = (*tbl) * 6; } while (0); /* FAKE: loop-note ref weighting, +1 flow reference on tbl (4 -> 5) */
    p = (u16 *) (offset + (s32) a1);
    buf[0] = p[0];
    buf[1] = -p[1];
    buf[2] = -p[2];
    func_8004A348(buf, pa4);
    tbl++;
    offset = offset + (s32) a2;
    p = (u16 *) offset;
    i++;
    buf[0] = p[0];
    buf[1] = -p[1];
    buf[2] = -p[2];
    do { func_8004A348(buf, out2); } while (0); /* FAKE: loop-note ref weighting, +1 flow reference on out2 (3 -> 4) */
    func_800523E0(pa4, out2, a3, stptr + 0x38);
    do { *((s16 *) (stptr + 6)) = 2; } while (0); /* FAKE: loop-note ref weighting, +1 flow reference on stptr (5 -> 6) */
    stptr += 0x68;
    if (i < 0x12) {
        goto loop1;
    }
    a1 += 0x6C;
    a2 += 0x6C;
    i = 0x12;
    stptr2 = saved + 0x750;
    out3 = (s32 *) (((u8 *) pa4) + 0x20);
    loop2:
    func_80044DE4((s16 *) a1, (s16 *) a2, a3, stptr2 + 0x4C);
    a1 += 6;
    a2 += 6;
    buf[0] = *((u16 *) a1);
    a1 += 2;
    buf[1] = -(*((u16 *) a1));
    a1 += 2;
    buf[2] = -(*((u16 *) a1));
    a1 += 2;
    func_8004A348(buf, pa4);
    buf[0] = *((u16 *) a2);
    a2 += 2;
    buf[1] = -(*((u16 *) a2));
    a2 += 2;
    buf[2] = -(*((u16 *) a2));
    a2 += 2;
    func_8004A348(buf, out3);
    func_800523E0(pa4, out3, a3, stptr2 + 0x38);
    *((s16 *) (stptr2 + 6)) = 1;
    stptr2 += 0x68;
    i++;
    if (i < 0x14) {
        goto loop2;
    }
}
