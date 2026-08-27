/* KILLED s28 (2026-08-27). V15a chassis + an EXPRESSION-ROOTED block-0 split of
 * out2's definition: `out2 = pa4 + 0x28; out2 = out2 - 8;` (F1; F2 = +0x30/-0x10,
 * F3 = -0x10/+0x30 measured identically). The idea was that s22's make_regs_eqv
 * iff-law ("the chain delivers its reference iff the RECIPIENT outlives the DONOR")
 * only governs COPY-rooted chains, because that law works through cse.c's
 * canon_reg / qty_first_reg machinery, which is only reached when the first insn's
 * source is a bare REG. With an expression source `(plus 77 40)` out2 gets its OWN
 * quantity (insert_regs finds no REG in the class -> make_new_qty), so canon_reg
 * cannot substitute pa4 for it.
 *
 * MEASURED: all three spellings are exactly V15a. ALLOCDBG out2 = 3 refs / live 42
 * / pri 714 -> $fp, every other allocno unchanged, 142 model insns, sandbox 15.
 *
 * MECHANISM (dumps tmp/grind/func_80041188/s28/F1/): the fold is done by cse.c's
 * fold_rtx REASSOCIATION, not by canon_reg. red.i.rtl insn 40 = (set 86 (plus 77 40)),
 * insn 43 = (set 86 (plus 86 -8)); in red.i.cse insn 43 has become
 * (set 86 (plus 77 32)) -- cse substituted reg 86's KNOWN VALUE (plus 77 40) into the
 * second insn and simplified. Insn 40 is thereby dead and is absent from red.i.flow
 * (flow.c deletes it; per E-s21-5 flow-deleted insns are never counted). out2 stays
 * at 3 references.
 *
 * CONSEQUENCE: the block-0 reference-lift class is now closed on BOTH of its horns --
 * copy-rooted chains by the make_regs_eqv law (E-s22-2) and expression-rooted chains
 * by this reassociation law. Do not re-propose any two-insn block-0 derivation of
 * out2 from the matrix pointer in any spelling.
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
    out2 = (s32 *) (((u8 *) pa4) - 0x10);
    out2 = (s32 *) (((u8 *) out2) + 0x30);
    stptr = base;
    stptr += 0xFC;
        loop1:
    offset = (*tbl) * 6;
    p = (u16 *) (offset + (s32) a1);
    buf[0] = p[0];
    buf[1] = -p[1];
    buf[2] = -p[2];
    func_8004A348(buf, pa4);
    tbl += 2;
    tbl -= 1;
    offset = offset + (s32) a2;
    p = (u16 *) offset;
    i += 2;
    i -= 1;
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
