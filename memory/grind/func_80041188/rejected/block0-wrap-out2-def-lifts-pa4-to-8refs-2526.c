/* s20 (structural). Goto chassis, Z0 block-2 spelling (`out3 = pa4 + 0x20`),
tbl-load + stptr-store wraps kept from s18 V7, and the s18 V7 out2-CALL wrap
replaced by a do-while(0) wrap around out2's block-0 DEFINITION.
MEASURED: sandbox 27 at 132 build insns. ALLOCDBG: stptr 6/41=2926 $s3,
pa4 8/95=2526 $s4, stptr2 6/48=2500 $s0, i 8/97=2474 $s5, tbl 5/47=2127 $s6,
out2 4/42=1904 $s7, a3 4/99=808 $fp.
WHY DEAD: the wrap gives out2 exactly the priority the seat order needs
(1904 sits inside the required (pa4, tbl) window), but flow.c:2081 weights
EVERY reference in the enclosed insn, and out2's definition necessarily
references pa4 -- so pa4 goes 7 -> 8 refs (1473 -> 2526) and jumps above i,
tbl and out2 at once. A block-0 wrap can never lift out2 alone, because
out2's only block-0 insn is its definition from pa4.
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
    do { out2 = (s32 *) (((u8 *) pa4) + 0x20); } while (0); /* FAKE: loop-note ref weighting, +1 flow reference on out2 */
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
    func_8004A348(buf, out2);
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
