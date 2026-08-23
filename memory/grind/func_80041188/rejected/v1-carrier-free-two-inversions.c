/* REJECTED s6 (forensics, 2026-08-23) — the carrier-free world (s5 frontier #3)
 * is STRICTLY HARDER than the pa4-carrier world; axis closed.
 *
 * Form: no pa4 local; the a4 parameter is used at all six sites.
 * ALLOCDBG (tmp/grind/func_80041188/s5/v1/stderr.txt), tail of the order:
 *   ord6 pseudo 75 (a3)    hardreg 22 (s6)  4 refs / 99  pri 808
 *   ord7 pseudo 76 (a4)    hardreg 23 (s7)  7 refs / 190 pri 736
 *   ord8 pseudo 85 (out2)  hardreg 30 (fp)  3 refs / 42  pri 714
 * Target needs out2 -> s6, a4 -> s7, a3 -> fp, i.e. the order out2 > a4 > a3.
 * That is TWO inversions (out2 must pass BOTH, and a4 must additionally pass
 * a3 — a4's 736 is already below a3's 808 here), where the pa4-carrier chassis
 * needs only ONE (out2 above the carrier's 1473.7).  Dropping the carrier
 * therefore adds a constraint instead of removing one. */
void func_80041188(s32 a0, u8 *a1, u8 *a2, s32 a3, s32 *a4)
{
    s32 i = 1;
    s32 *tbl = D_80094CFC;
    s32 base = D_800A9A10[a0];
    s16 buf[3];
    s32 saved;
    s32 *out2;
    s32 stptr;
    s32 offset;
    u16 *p;
    s32 stptr2;
    saved = base + 0x94;
    out2 = (s32 *) (((u8 *) a4) + 0x20);
    stptr = base;
    stptr += 0xFC;
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
    stptr = (s32) (((u8 *) a4) + 0x20);
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
    func_8004A348(buf, (s32 *) stptr);
    func_800523E0(a4, (s32 *) stptr, a3, stptr2 + 0x38);
    *((s16 *) (stptr2 + 6)) = 1;
    stptr2 += 0x68;
    i++;
    if (i < 0x14) {
        goto loop2;
    }
}
