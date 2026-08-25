/* func_80041188 / hirahira_w_ctrl -- s21 ALTERNATE CHASSIS (form Q1). NOT the
 * candidate: sandbox --disable all == 7 at 132 of 132 insns, where
 * memory/grind/func_80041188/candidate.c scores 1.
 *
 * WHY IT IS BANKED. This is the first form that holds ALL-TARGET callee-saved
 * seats together with target's block-2 `addiu $s3,$s7,0x20` at 132 insns AND
 * without materialising the parameter copy that cost s20's Y2b/Y4 an insn. Its
 * whole residual is sched1 EMISSION ORDER inside block 0: target emits
 * `sw $s5,0x34($sp)` / `lui $s5` / `addiu $s5,$s5` as block 0's insns 2-4, this
 * form emits that trio ~7 slots later (and `sw $t0,0x18($sp)` two slots early),
 * because tbl's definition has to be the LAST statement of block 0 to keep its
 * live length at 41. See evidence.md E-s21-1 / E-s21-3 / E-s21-4.
 *
 * ALLOCDBG (this body): stptr 7/42 = 3333 -> $s3, stptr2 6/48 = 2500 -> $s0,
 * i 8/97 = 2474 -> $s4, tbl 4/41 = 1951 -> $s5, out2 4/43 = 1860 -> $s6,
 * a4 8/192 = 1250 -> $s7, a3 4/99 = 808 -> $fp, out3 3/47 -> $s3, saved spilled.
 *
 * CONSTRUCTS -- both are FAKE-annotated and NEITHER has been cleared by a Judge
 * for this function. Do not ship this body as-is.
 *   1. the block-0 `do { out2 = ...; } while (0);` wrap (loop-note reference
 *      weighting, flow.c:2081);
 *   2. candidate.c's `stptr = base; stptr += 0xFC;` F1 chain-extender.
 */
void func_80041188(s32 a0, u8 *a1, u8 *a2, s32 a3, s32 *a4)
{
    s32 i = 1;
    s32 *tbl;
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
    do { out2 = (s32 *) (((u8 *) a4) + 0x20); } while (0); /* FAKE: loop-note ref weighting, +1 flow reference on out2 */
    stptr = base;
    stptr += 0xFC; /* FAKE: F1 chain-extender */
    tbl = D_80094CFC;
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
