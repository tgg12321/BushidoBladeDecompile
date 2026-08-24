/* func_80041188 / hirahira_w_ctrl - s7 (rederive) candidate. sandbox
 * --disable all == 1, 132/132 insns, frame 72 == target 0x48.
 *
 * ORDINARY C. No register pin, no inline asm, no volatile, no dead code, no
 * unused local, NO /* FAKE *â€‹/ construct, no variable reuse, no do-while(0)
 * wrap. Every local is once-declared, written where a human would write it,
 * and read for its real value. This REPLACES the s3/s5/s6 "merged stptr"
 * chassis (loop2's pointer reusing the dead loop1 walker via an out2 read),
 * which scored the same 1 but only with a FAKE variable-reuse annotation.
 *
 * WHAT s7 CHANGED (the rederive). Every ledger session up to s6 assumed the
 * committed rule-era shape: ONE `out2` local, reused/re-initialised before
 * loop2. s7 read the target instead and found that target holds a4+0x20 in
 * TWO DIFFERENT callee-saved registers - $s6 across loop1, $s3 across loop2 -
 * which GCC 2.7.2 cannot do with one pseudo (it has no live-range splitting).
 * Therefore the original source has TWO SEPARATE LOCALS, and the second one
 * (`out3`) is defined in the between-loops block. Measured corollary (s7):
 * GCC does NOT hoist `a4 + 8` out of these goto-loops at all (no loop notes),
 * so both `addiu ...,$s7,0x20` in target are source-level statements, not
 * LICM output - see rejected/inline-a4plus8-no-licm-hoist.c.
 *
 * WHY `out3 = out2;` AND NOT `out3 = a4 + 8;`. This is the entire residual.
 * The GCC 2.7.2 global.c allocno priority is
 *     pri = floor_log2(reg_n_refs) * reg_n_refs / reg_live_length * 10000
 * and the four contested callee-saved seats need the order
 *     tbl > out2 > a4 > a3   (-> $s5, $s6, $s7, $fp).
 * Measured this session from .lreg/.greg with this exact body:
 *     73/74 a1,a2   16 refs / 99  = 6464.6   -> $s1,$s2
 *     88    stptr    7 refs / 41  = 3414.6   -> $s3 (loop1 only)
 *     91    stptr2   6 refs / 48  = 2553.2   -> $s0
 *     78    i        8 refs / 97  = 2474.2   -> $s4
 *     79    tbl      4 refs / 47  = 1702.1   -> $s5
 *     86    out2     4 refs / 47  = 1702.1   -> $s6   (EXACT TIE with tbl,
 *                                                      broken by allocno
 *                                                      number 79 < 86)
 *     77    a4       6 refs / 95  = 1263.2   -> $s7
 *     75    a3       4 refs / 99  =  808.1   -> $fp
 *     87    out3     3 refs / 47  =  638.3   -> $s3 (loop2, no conflict)
 * With `out3 = (s32 *)((u8 *)a4 + 0x20);` instead, out2 drops to 3 refs and
 * live 42 (dead at loop1's exit) -> pri 714.3, which lands it BELOW a4 and a3;
 * the whole {out2, a4, a3} triple then permutes to {$fp, $s6, $s7} and the
 * score goes 1 -> 15 (rejected/two-locals-out3-from-a4-seat-permutation.c).
 * out2's 4th reference AND its live length 47 both come from exactly one
 * thing: being read in the between-loops block, as its LAST statement.
 * Position is load-bearing - moving `out3 = out2;` earlier in that block
 * shortens out2's live range and re-breaks the tie the wrong way
 * (measured: last=1, 3rd=10, 2nd=11, 1st/0th=12).
 *
 * RESIDUAL (slot 72, the ONLY non-formatting diff in 132 insns):
 *     OURS  move  $s3, $s6            TGT  addiu $s3, $s7, 0x20
 * i.e. target re-derives loop2's pointer from a4 while STILL giving out2 a
 * 4th flow-counted reference. Since flow.c fixes reg_n_refs before combine
 * runs, the original's 4th out2 reference must be an insn in the
 * between-loops block that combine deleted. Finding a byte-free spelling of
 * that reference is the whole remaining problem - see hypotheses.md s7.
 *
 * s9 ADDENDUM (structural, 2026-08-23). This body still scores 1 and, newly
 * measured this session with a mechanical seat checker, it already holds
 * ALL-TARGET callee-saved seats - so its single residual insn is a FORM
 * difference, not an allocation difference. s9 also produced the first
 * all-target-seats form on the I2 chassis (rejected/m1-i2-symk-ilate-...c,
 * sandbox 4) whose entire residual is the preamble emission ORDER, and proved
 * that order and tbl's live length are the same variable (sched.c
 * rank_for_schedule's INSN_LUID fallback + flow.c:1685). See evidence.md s9.
 *
 * s10 ADDENDUM (structural, 2026-08-23). Still floor 1, still ALL-TARGET seats.
 * s10 closed the whole "make i outrank tbl in the i-FIRST order" axis by a
 * measured identity: with i defined before tbl, reg_live_length(i) is always
 * reg_live_length(tbl) + 49, so an 8-reference i needs tbl live >= 50, while
 * sixteen distinct block-0 mutations bound tbl's live length at 48. The escape
 * is a NINTH flow-counted reference to i: measured, it gives ALL-TARGET seats
 * on the i-first + separated-restore chassis at sandbox 3 / 133 insns, one insn
 * over target, that insn being the reference itself. A byte-free ninth
 * reference (surviving cse1 into flow, deleted by combine) is distance 0.
 * See evidence.md s10 and hypotheses.md s10 frontier 1.
 *
 * s11 ADDENDUM (escalation, 2026-08-23). Still floor 1 / 132 insns / ALL-TARGET
 * seats (re-measured this session at the start and again at the end). s11 closed
 * the block-0 side of the residual, the mirror of s8's between-block kill:
 * on the only chassis that emits target's `addiu $s3,$s7,0x20` (out3 taken from
 * pa4), out2's live length CANNOT exceed 43 - measured with its definition made
 * the first statement of block 0 - while target's seating needs out2's priority
 * inside (1458, 1702), i.e. live 47..55 at 4 references. The window is empty.
 * The F1 combine-foldable chain-extender (the last unspent sanctioned byte-free
 * reg_n_refs family) does buy out2 a 4th reference in block 0, but every spelling
 * must subtract a pa4-derived term to cancel, which lifts pa4 (8 refs / 2424)
 * above out2 (1739), AND it materialises two insns (134 vs 132), failing F1's own
 * zero-bytes prerequisite. See evidence.md E-s11-1/E-s11-2.
 *
 * s11 CORRECTION to this header's own claims. `stptr = base; stptr += 0xFC;`
 * IS a combine-foldable chain-extender: target emits one `addiu $s3,$v0,0xFC`,
 * the split is byte-neutral (132 insns either way), and its only surviving effect
 * is stptr's reg_n_refs count - un-splitting it drops stptr from 7/41=3414 to
 * 5/41=2439, below stptr2, and the floor from 1 to 15. So the "ORDINARY C ... NO
 * FAKE construct" claim above is WRONG for that one statement. A future
 * candidate must either carry a FAKE: F1 annotation on it (family:
 * .claude/rules/dead-store-fake-exception.md:32) or replace it; shipped
 * un-annotated it is a layer-1 FAIL. See evidence.md E-s11-3.
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
    stptr = base;
    stptr += 0xFC; /* FAKE: F1 combine-foldable chain-extender (see header s11
                      correction) — byte-neutral split whose only effect is
                      lifting stptr's reg_n_refs 5->7 (floor 15->1); target
                      emits the single addiu. NOT committable un-annotated. */
    loop1:
    offset = (*tbl) * 6;
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
    func_8004A348(buf, out2);
    func_800523E0(pa4, out2, a3, stptr + 0x38);
    *((s16 *) (stptr + 6)) = 2;
    stptr += 0x68;
    if (i < 0x12) {
        goto loop1;
    }
    a1 += 0x6C;
    a2 += 0x6C;
    i = 0x12;
    stptr2 = saved + 0x750;
    out3 = out2;
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
