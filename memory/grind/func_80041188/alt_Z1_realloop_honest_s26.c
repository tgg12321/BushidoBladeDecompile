/* alt_Z1_realloop_honest_s26.c -- s26 best HONEST chassis.
 *
 * sandbox func_80041188 --disable all = score 13 at 132 build / 132 target insns.
 * (Not a floor improvement: candidate.c still holds the floor at 1. This form is
 *  banked because it is the first chassis that obtains the loop-depth reference
 *  weighting HONESTLY -- loop1 is a real `do { } while (i < 0x12);`, not a
 *  do-while(0) wrap, and it carries NO increment splits and NO stptr chain
 *  extender, i.e. no FAKE construct of any kind.)
 *
 * Mechanism: GCC 2.7.2 flow.c increments REG_N_REFS by `loop_depth`, and
 * loop_depth is driven by NOTE_INSN_LOOP_BEG/END, which the front end emits only
 * for real loop CONSTRUCTS. A goto-loop therefore counts every in-loop reference
 * once; a real loop counts it twice. That is precisely the dial the banned
 * do-while(0) wrap was faking. On this chassis out2 rises 3 refs/42 = 714 to
 * 5 refs/41 = 2439 with no instruction cost.
 *
 * Residual (exactly two things, see evidence.md E-s26-4):
 *   (1) out2/pa4 seat swap: out2 2439 vs pa4 2872 -- out2 must outrank pa4.
 *   (2) loop.c strength-reduces the stptr biv into the stptr+0x38 giv
 *       (addu $19,$2,308 / addu $7,$19,$zero / sh $8,-50($19) instead of
 *        addu $19,$2,252 / addu $7,$19,56 / sh $2,6($19)).
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
    do {
    offset = (*tbl) * 6;
    p = (u16 *) (offset + (s32) a1);
    buf[0] = p[0];
    buf[1] = -p[1];
    buf[2] = -p[2];
    func_8004A348(buf, pa4);
    tbl += 1;
    offset = offset + (s32) a2;
    p = (u16 *) offset;
    i += 1;
    buf[0] = p[0];
    buf[1] = -p[1];
    buf[2] = -p[2];
    func_8004A348(buf, out2);
    func_800523E0(pa4, out2, a3, stptr + 0x38);
    *((s16 *) (stptr + 6)) = 2;
    stptr += 0x68;
    } while (i < 0x12);
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
