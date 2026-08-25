/* s19 REJECTED (sandbox 13, 132 build insns). W6 + `o2t = (u8 *)pa4 + 0x20;
   do { out2 = (s32 *)o2t; } while (0);`. Two facts measured:
   (1) a do-while(0) wrap in BLOCK 0 is byte-neutral here (132 insns) -- unlike the
       in-loop wrap of W8, which costs three nops;
   (2) it lifts NOTHING: out2 stays at 5 refs (live 42). The wrapped insn is a reg-copy
       whose destination out2 outlives the source o2t, so by the s17 make_regs_eqv law
       (cse.c:844-857) out2 is the canonical register, cse1 rewrites o2t's definition to
       set out2 directly and the copy is gone before flow counts it. A block-0 wrap can
       only weight a reference that SURVIVES cse1. */
void func_80041188(s32 a0, u8 *a1, u8 *a2, s32 a3, s32 *a4)
{
    s32 *pa4 = a4;
    s32 i = 1;
    s32 *tbl = D_80094CFC;
    s32 base = D_800A9A10[a0];
    s16 buf[3];
    s32 ents;
    s32 *out2;
    u8 *o2t;
    s32 *out3;
    s32 offset;
    u16 *p;
    s32 stptr2;
    ents = base + 0x94;
    o2t = ((u8 *) pa4) + 0x20;
    do { out2 = (s32 *) o2t; } while (0);
    do {
        offset = (*tbl) * 6;
        p = (u16 *) (offset + (s32) a1);
        buf[0] = p[0];
        buf[1] = -p[1];
        buf[2] = -p[2];
        func_8004A348(buf, pa4);
        tbl++;
        offset = offset + (s32) a2;
        p = (u16 *) offset;
        buf[0] = p[0];
        buf[1] = -p[1];
        buf[2] = -p[2];
        func_8004A348(buf, out2);
        func_800523E0(pa4, out2, a3, ents + i * 0x68 + 0x38);
        *((s16 *) (ents + i * 0x68 + 6)) = 2;
        i++;
    } while (i < 0x12);
    a1 += 0x6C;
    a2 += 0x6C;
    i = 0x12;
    stptr2 = ents + 0x750;
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
