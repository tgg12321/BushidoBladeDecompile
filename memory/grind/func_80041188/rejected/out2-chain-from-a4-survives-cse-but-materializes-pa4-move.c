void func_80041188(s32 a0, u8 *a1, u8 *a2, s32 a3, s32 *a4)
{
    s32 *pa4;
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
    out2 = a4;
    out2 = (s32 *) (((u8 *) out2) + 0x20);
    pa4 = a4;
    saved = base + 0x94;
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
       lever-exhaustion: memory/grind/func_80041188/hypotheses.md, s7..s16 --
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
