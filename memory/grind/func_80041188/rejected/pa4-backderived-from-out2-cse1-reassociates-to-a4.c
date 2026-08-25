/* s20 (structural). No wrap on out2 at all: out2 defined from the parameter and
pa4 back-derived from it (`pa4 = (s32 *)(((u8 *)out2) - 0x20);`), to give out2
a byte-free FOURTH reference from a semantically real block-0 statement whose
donor (out2) is the recipient's source rather than its sink.
MEASURED: sandbox 23 at 133 insns; out2 3 refs / live 43 = 697, pa4 6/93=1290 --
i.e. the intended out2 reference does not exist. The ALLOCDBG table and the
score are IDENTICAL to Z2 (`pa4 = a4;` written plainly), proving cse1
reassociated `(plus (plus a4 32) -32)` back to `a4` before flow counted
anything. This extends E-s18-2 (cse1 reassociates plus-derivations inside the
definition's EBB) to the SUBTRACT direction and to a derivation whose result is
consumed everywhere in the function.
WHY DEAD: no block-0 arithmetic derivation of one matrix pointer from the other
can survive cse1, in either direction.
*/
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
    saved = base + 0x94;
    out2 = (s32 *) (((u8 *) a4) + 0x20);
    pa4 = (s32 *) (((u8 *) out2) - 0x20);
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
