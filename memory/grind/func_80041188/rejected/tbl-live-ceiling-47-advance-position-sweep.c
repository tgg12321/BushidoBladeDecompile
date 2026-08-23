/* s9: KILLED - tbl's reg_live_length has a hard CEILING of 47 in the i-first
 * order and the sym-K advance's position cannot raise it.  Sweep of the advance
 * insn `tbl = (s32 *)((u8 *)tbl + 0x10);` across every slot of block 0 (this file
 * = slot 0; also measured slots 1, 2, 3 and last): live is 47 for slots 0-3 and
 * 45 for the last slot, refs 6 throughout.  Together with the s8 tbl-assignment
 * position sweep, the s6 16-position `tbl++` sweep and the s9 block-0 split-init
 * probes (P5A/P5B/P5C, W2, W3, X1), NINE distinct block-0 mutations leave tbl at
 * 45 or 47 and none reaches 48.  47 = loop1's 41 insns + a fixed 6-insn block-0
 * tail.  Only moving `i = 1` past tbl's definition has ever produced 48.
 */
void func_80041188(s32 a0, u8 *a1, u8 *a2, s32 a3, s32 *a4)
{
    s32 *pa4 = a4;
    s32 i = 1;
    s32 *tbl = (s32 *) (((u8 *) D_80094CFC) - 0x10);
    s32 base = D_800A9A10[a0];
    s16 buf[3];
    s32 saved;
    s32 *out2;
    s32 *out3;
    s32 stptr;
    s32 offset;
    u16 *p;
    s32 stptr2;
    tbl = (s32 *) (((u8 *) tbl) + 0x10);
    saved = base + 0x94;
    out2 = (s32 *) (((u8 *) pa4) + 0x20);
    stptr = base;
    stptr += 0xFC;
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
    out2 = (s32 *) (((u8 *) pa4) + 0x20);
    stptr2 = saved + 0x750;
    out3 = out2;
    i = 0x12;
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
