/* func_80041188 -- s22 REJECTED (form A1). pa4-free chassis (s21 Q1) with the
 * block-0 do-while(0) FAKE wrap on out2's definition REPLACED by an F1-style
 * split-init chain sourced from the parameter:
 *     out2 = (s32 *) a4;
 *     out2 = (s32 *) (((u8 *) out2) + 0x20);
 *
 * MEASURED: sandbox --disable all = 10 at 132 of 132 build insns (so the chain
 * really is byte-free) but out2 stays at 3 refs / live 42 = 714 and is seated
 * $fp; a3 4/99 = 808 takes $s6 and a4 7/190 = 736 takes $s7. Only a3 and out2
 * are out of target order.
 *
 * WHY IT IS DEAD (mechanism read out of the dumps, s22/A1/red.i.{cse,flow}):
 * cse1 rewrites insn 37's source from (plus reg85 32) to (plus reg76 32) --
 * reg 76 is the parameter a4 and OUTLIVES out2, so by make_regs_eqv
 * (cse.c:844-857) a4 is the canonical register of the equivalence class. That
 * leaves insn 34 (`out2 = a4`) dead, and it is absent from red.i.flow: flow.c
 * deletes it INSIDE life analysis, so its registers are never counted
 * (E-s21-5). The chain therefore yields +0 references, not +1.
 *
 * GENERAL LAW (s22 E-s22-2): an F1 split-init chain-extender delivers its +1
 * flow-counted reference IF AND ONLY IF the recipient outlives the donor.
 * `stptr = base; stptr += 0xFC;` works because `base` dies at that point;
 * NOTHING sourced from `a4`/`pa4` can ever work for out2, on any chassis,
 * because the matrix pointer outlives out2 by construction. Do not re-spell
 * this idea -- the closure is derived, not measured-by-cases.
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
    out2 = (s32 *) a4;
    out2 = (s32 *) (((u8 *) out2) + 0x20);
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
