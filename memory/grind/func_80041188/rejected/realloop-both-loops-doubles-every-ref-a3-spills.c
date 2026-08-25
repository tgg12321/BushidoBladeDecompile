/* s19 REJECTED (sandbox 78). BOTH loops converted to real `do {} while` loops with
   index-derived addresses. Every in-loop reference is weighted twice by flow.c:2081,
   so a1/a2 reach 29 refs, pa4 10, and the whole priority table inflates out of
   target's order; a3 (7/100) spills. The note-weighting regime is only usable on ONE
   loop at a time. */
void func_80041188(s32 a0, u8 *a1, u8 *a2, s32 a3, s32 *a4)
{
    s32 *pa4 = a4;
    s32 i = 1;
    s32 *tbl = D_80094CFC;
    s32 base = D_800A9A10[a0];
    s16 buf[3];
    s32 ents;
    s32 *out2;
    s32 *out3;
    s32 ent;
    s32 offset;
    u16 *p;
    s32 stptr2;
    ents = base + 0x94;
    out2 = (s32 *) (((u8 *) pa4) + 0x20);
    do {
        ent = ents + i * 0x68;
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
        func_800523E0(pa4, out2, a3, ent + 0x38);
        *((s16 *) (ent + 6)) = 2;
    } while (i < 0x12);
    a1 += 0x6C;
    a2 += 0x6C;
    i = 0x12;
    out3 = out2;
    do {
    stptr2 = ents + i * 0x68;
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
    i++;
    } while (i < 0x14);
}
