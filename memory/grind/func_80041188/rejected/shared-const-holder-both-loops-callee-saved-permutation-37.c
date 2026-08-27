/* s36 KILL. Attempt to break loop.c's movable gate (loop.c:695-701) by making the constant
 * holder a USER variable whose set and uses span TWO basic blocks -- one `s16 two;` written
 * 2 in loop1 and 1 in loop2 -- so that both `! REG_USERVAR_P` and `reg_in_basic_block_p`
 * fail. It does defeat those two gates, but gate (1) (`! maybe_never &&
 * ! loop_reg_used_before_p`) still holds for a do-while body, and the shared holder becomes
 * a long-lived pseudo that competes for a callee-saved seat: sandbox 37 at 132 insns, i.e.
 * the whole allocation permutes. Strictly worse than doing nothing. */
void func_80041188(s32 a0, u8 *a1, u8 *a2, s32 a3, s32 *a4)
{
    s32 i = 1;
    s32 *tbl = D_80094CFC;
    s32 base = D_800A9A10[a0];
    s16 buf[3];
    s32 ents;
    s32 *out2;
    s32 *out3;
    s32 offset;
    u16 *p;
    s32 stptr2;
    s16 two;
    ents = base + 0x94;
    out2 = (s32 *) (((u8 *) a4) + 0x20);
    do {
        offset = (*tbl) * 6;
        p = (u16 *) (offset + (s32) a1);
        buf[0] = p[0];
        buf[1] = -p[1];
        buf[2] = -p[2];
        func_8004A348(buf, a4);
        tbl++;
        offset = offset + (s32) a2;
        p = (u16 *) offset;
        buf[0] = p[0];
        buf[1] = -p[1];
        buf[2] = -p[2];
        func_8004A348(buf, out2);
        func_800523E0(a4, out2, a3, ents + i * 0x68 + 0x38);
        two = 2;
        *((s16 *) (ents + i * 0x68 + 6)) = two;
        i++;
    } while (i < 0x12);
    a1 += 0x6C;
    a2 += 0x6C;
    i = 0x12;
    stptr2 = ents + 0x750;
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
    two = 1;
    *((s16 *) (stptr2 + 6)) = two;
    stptr2 += 0x68;
    i++;
    if (i < 0x14) {
        goto loop2;
    }
}
