/* s19 REJECTED (sandbox 80, 132 build insns). loop1 as a real `do {} while (i < 0x12)`
   with the entry address held in a USER variable (`ent = ents + i * 0x68;`).
   loop.c strength-reduces the multiply into a giv pseudo but, because the destination
   is a user variable (REG_USERVAR_P), it does NOT rename the variable to the giv --
   it emits `ent = <giv>` at the top of every iteration (insn 339 in
   tmp/grind/func_80041188/s19/W1/red.i.combine). That copy makes `ent` a SECOND,
   short-lived quantity which local_alloc seats in $s1 (it is live across the calls),
   so the 9 callee-saved seats are consumed before `a3` is reached and a3 spills.
   FIX: never name the induction address; write the address expression inline at its
   two use sites so the giv's destination is a compiler pseudo (see W4). */
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
    stptr2 = ents + 0x750;
    out3 = out2;
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
