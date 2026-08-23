/* s7 (forensics, 2026-08-23): the ALL-TARGET-SEATS chassis. sandbox 4, 132/132.
 * = pa4-read MID re-init + the out2 staging store before loop2's func_800523E0
 *   (rejected/s4-form-minus-banned-wrap-floor9.c) + the tbl sym-K split-init
 *   ref lift (rejected/tbl-symK-splitinit-refs6-steals-s4.c) + `i = 1` moved out
 *   of the declaration to the LAST preamble statement.
 *
 * Every callee-saved register disposition equals target for the first time in
 * the ledger: 73 a1->s1, 74 a2->s2, 87 stptr->s3, 90 stptr2->s0, 78 i->s4,
 * 79 tbl->s5, 86 out2->s6, 77 pa4->s7, 75 a3->fp, 85 saved spilled.
 * (i 8 refs/live 96 = 2500.0 and tbl 6/48 = 2500.0 tie exactly; the tie breaks
 * our way on allocno number 78 < 79.)
 *
 * REJECTED: the residual is 4 insns of PREAMBLE EMISSION ORDER, and it is
 * anti-correlated with the seats. Target emits
 *     sw s4,48(sp) / addiu s4,zero,1 / sw s5,52(sp) / lui s5,HI / addiu s5,s5,LO
 * i.e. i's def BEFORE tbl's symbol def; this form emits the tbl pair first.
 * Measured law (15 variants this session): whichever of the two is defined
 * first in the C gets live+1 and the other live-1, and sched1 emits them in
 * RTL/def order (equal INSN_PRIORITY, tie broken by LUID). i-first =>
 * (i 97 = 2474, tbl 47 = 2553) => tbl steals s4 (sandbox 12); tbl-first =>
 * (96, 48) => target seats but the emission pair is swapped (sandbox 4).
 * Closing this needs i at >= 9 flow refs (pri 2783 > 2553) in the i-FIRST
 * order, byte-free -- and every preamble use of i is constant-propagated by
 * cse1 (i is the literal 1 there), while split-init ref lifts work only on
 * symbol-constant pseudos (s5 law). */
void func_80041188(s32 a0, u8 *a1, u8 *a2, s32 a3, s32 *a4)
{
    s32 *pa4 = a4;
    s32 i;
    s32 *tbl = (s32 *) (((u8 *) D_80094CFC) - 0x10);
    s32 base = D_800A9A10[a0];
    s16 buf[3];
    s32 saved;
    s32 *out2;
    s32 stptr;
    s32 offset;
    u16 *p;
    s32 stptr2;
    saved = base + 0x94;
    out2 = (s32 *) (((u8 *) pa4) + 0x20);
    stptr = base;
    tbl = (s32 *) (((u8 *) tbl) + 0x10);
    i = 1;
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
    i = 0x12;
    stptr2 = saved + 0x750;
    stptr = (s32) (((u8 *) pa4) + 0x20);
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
    func_8004A348(buf, (s32 *) stptr);
    out2 = (s32 *) stptr;
    func_800523E0(pa4, out2, a3, stptr2 + 0x38);
    *((s16 *) (stptr2 + 6)) = 1;
    stptr2 += 0x68;
    i++;
    if (i < 0x14) {
        goto loop2;
    }
}
