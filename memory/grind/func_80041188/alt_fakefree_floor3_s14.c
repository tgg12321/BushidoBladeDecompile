/* func_80041188 / hirahira_w_ctrl - s14 (synthesis) FAKE-FREE alternative.
 * sandbox --disable all == 3, 132/132 insns, ALL-TARGET callee-saved seats.
 *
 * This is NOT the lowest-floor form (memory/grind/func_80041188/candidate.c is, at 1).
 * It is the first form in the whole ledger that reaches target's complete callee-saved
 * seat assignment with NO /* FAKE *​/ construct of any kind: no `stptr = base;
 * stptr += 0xFC;` chain-extender, no variable reuse, no dead store, no do-while(0) wrap.
 * Every local is once-declared, written where a human would write it, read for its real
 * value.
 *
 * WHAT REPLACED THE CHAIN-EXTENDER. s11 proved candidate.c's split `stptr` assignment is
 * an un-annotated F1 combine-foldable chain-extender (5 refs -> 7 refs, 2439 -> 3414,
 * byte-neutral, floor 15 -> 1). s14 showed it exists only to satisfy ONE of the two
 * priority requirements target's seating imposes - (A) stptr > i - and that (A) has an
 * ordinary-C solution: `i = 0x12;` written as the FIRST statement of the between-loops
 * block closes the dead gap between loop1's last read of i and its redefinition, so
 * flow.c:1685 counts reg_live_length(i) = 99 instead of 97 and i's priority falls
 * 2474 -> 2424, below an un-split stptr's 2439. Positions 2/3/4 give 98/97/96 and all
 * fail (evidence.md E-s14-3).
 *
 * ITS RESIDUAL (3 insns, all in the between-loops block region):
 *   slot 67  OURS li $s4,18                TGT addiu $s1,$s1,0x6C
 *   slot 68  OURS addiu $s1,$s1,0x6C       TGT addiu $s2,$s2,0x6C
 *   slot 69  OURS addiu $s2,$s2,0x6C       TGT addiu $s4,$zero,0x12
 *   slot 71  OURS move  $s3,$s6            TGT addiu $s3,$s7,0x20
 * The first three are one sched1 emission-order shift: flow reads reg_live_length off
 * PRE-sched RTL (source order) while sched1's rank_for_schedule falls back to INSN_LUID
 * (also source order) for these three dependency-free insns, so the statement order that
 * buys i live 99 also forces `i = 0x12` to be emitted first. Y1/Y2/Y3 in
 * tmp/grind/func_80041188/s14/ are three failed attempts to decouple them. Slot 71 is the
 * long-standing requirement-(B) residual, unchanged.
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
    i = 0x12;
    a1 += 0x6C;
    a2 += 0x6C;
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
