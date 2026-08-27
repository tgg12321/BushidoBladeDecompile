/* s30 (structural). The ENTIRE pa4-free chassis family (s21 N1/CANDF, s22/s23 A1,
s21 Q1) written in its honest form: no `pa4` carrier local -- the fifth (stack)
parameter `a4` is used directly as the matrix pointer -- and block 2's second
pointer defined honestly as `out3 = (s32 *)((u8 *)a4 + 0x20)` so out2 keeps only the
3 references target's own bytes host.

MEASURED (this session, tmp/grind/func_80041188/s30/A1/): ALLOCDBG
  a1 16/99=6464 $s1 | a2 16/99=6464 $s2 | stptr 7/41=3414 $s3 | stptr2 6/48=2500 $s0
  i 8/97=2474 $s4 | tbl 4/47=1702 $s5 | a3 4/99=808 $s6 | a4 7/190=736 $s7
  | out2 3/42=714 $fp
i.e. target's seating (out2 $s6, a4 $s7, a3 $fp) inverted in all three seats.

WHY DEAD -- now a closed form, not a sweep. `a4` is the FIFTH parameter, so its pseudo
is set from a stack MEM and carries a REG_EQUIV note to that MEM (verified in
s30/A1/red.i.sched insn 12). local-alloc.c:1064 doubles reg_live_length for ANY pseudo
carrying a REG_EQUIV note -- `reg_live_length[regno] *= 2;` -- which is the entire
explanation of the 30-session-unexplained "a4 live 190 vs a3 live 99" (sched1's own
recount is 95: s30/A1/sll_76.err). So on this chassis
    pri(a4) = 2*7*10000 / (2*L4raw) = 70000/L4raw
    pri(a3) = 2*4*10000 / L3raw     = 80000/L3raw
and L4raw (95) <= L3raw (99) because a3's live range starts one insn EARLIER in block 0
and both die at the same func_800523E0 in loop2. Therefore pri(a4) < pri(a3) ALWAYS:
a3 is seated first and takes $s6, the seat target gives out2. Measured exchange rate
(form A1I, one extra real store added inside loop2): a4 190->192 (+2, doubled),
a3 99->100 (+1), out2 42 (unchanged, dead in loop2) -- so every added loop2 insn moves
a4 DOWN twice as fast as a3, and a4 > a3 requires y < -67 insns. s21's Q1/N1 inverted
the pair only by giving a4 an EIGHTH reference (the do-while(0) wrap), which crosses
floor_log2's 7->8 step to 3 and yields 1263 -- one more reference than target's bytes
host (census E-s29-1: target hosts a4/pa4 at 7).

CONSEQUENCE (structural fact about the ORIGINAL): the original source DOES carry a
carrier local for the fifth parameter (candidate.c's `pa4`), because cse1 merges the
carrier and the parameter into one pseudo whose SET is a REG copy, not a MEM load, so
no REG_EQUIV note is attached and the live length is NOT doubled (95, pri 1473). */
void func_80041188(s32 a0, u8 *a1, u8 *a2, s32 a3, s32 *a4)
{
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
    out2 = (s32 *) (((u8 *) a4) + 0x20);
    stptr = base;
    stptr += 0xFC;
    /* FAKE: byte-neutral split of the single statement `stptr = base + 0xFC;`
       into a register copy plus a constant add, whose ONLY effect is lifting
       stptr's reg_n_refs from 5 to 7 (allocno priority 2439 -> 3414), which is
       what seats stptr in $s3 above `i`; target emits the single addiu.
       mechanism: flow.c:2081 counts reg_n_refs BEFORE combine and nothing ever
       recomputes it (combine.c:55-56 says so in its own header comment), while
       reg_live_length IS recomputed post-combine by sched1 (sched.c:5106 from
       sched.c:3165). combine.c then folds the copy+add back into one addiu, so
       the extra reference is counted by flow, spent by global.c's
       allocno_compare (global.c:635-656), and costs neither a byte nor a unit
       of live range. s16 additionally PROVED the original source carried such a
       reference: target's block-0 and block-2 emissions are insn-for-insn
       identical in order to ours, so target's own stptr live length is 41 and
       its i priority is 2474, yet target seats stptr above i -- which at live 41
       needs >= 6 references while target's bytes show only 5 stptr insns.
       lever-exhaustion: memory/grind/func_80041188/hypotheses.md, s7..s17 --
       stptr live 41 -> 40 killed by measurement in s16 (E-s16-2, def already the
       last insn of scheduled block 0, loop-carried across all 40 loop1 insns);
       every non-chain reference-lift spelling measured (s16 E-s16-4: use-only
       fold-back on tbl and out2, copy-then-modify on out2, compare-split on i)
       folds before flow and lifts nothing; s14 Y1/Y2/Y3 killed every statement
       -order route to satisfying stptr > i from the other side. */
    loop1:
    offset = (*tbl) * 6;
    p = (u16 *) (offset + (s32) a1);
    buf[0] = p[0];
    buf[1] = -p[1];
    buf[2] = -p[2];
    func_8004A348(buf, a4);
    tbl++;
    offset = offset + (s32) a2;
    p = (u16 *) offset;
    i++;
    buf[0] = p[0];
    buf[1] = -p[1];
    buf[2] = -p[2];
    func_8004A348(buf, out2);
    func_800523E0(a4, out2, a3, stptr + 0x38);
    *((s16 *) (stptr + 6)) = 2;
    stptr += 0x68;
    if (i < 0x12) {
        goto loop1;
    }
    a1 += 0x6C;
    a2 += 0x6C;
    i = 0x12;
    stptr2 = saved + 0x750;
    out3 = (s32 *) (((u8 *) a4) + 0x20);
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
    func_8004A348(buf, a4);
    buf[0] = *((u16 *) a2);
    a2 += 2;
    buf[1] = -(*((u16 *) a2));
    a2 += 2;
    buf[2] = -(*((u16 *) a2));
    a2 += 2;
    func_8004A348(buf, out3);
    func_800523E0(a4, out3, a3, stptr2 + 0x38);
    *((s16 *) (stptr2 + 6)) = 1;
    stptr2 += 0x68;
    i++;
    if (i < 0x14) {
        goto loop2;
    }
}
