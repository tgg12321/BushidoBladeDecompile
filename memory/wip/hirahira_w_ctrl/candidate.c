/* hirahira_w_ctrl - WIP candidate. sandbox --disable all == 23 (HEAD == 27).
 * 132/132 insns and frame 72 == target 0x48 BOTH already correct at HEAD;
 * the gap is register naming plus commutative-add operand order.
 *
 * Two findings folded in:
 *  1. THE REGISTER PIN IS NOT LOAD-BEARING. Replacing
 *     `register s32 *s7_a4 asm("s7") = a4;` with a plain local scores the
 *     SAME as HEAD (27), so this cheat-asm can be retired for free.
 *  2. Reusing `offset` as the pointer carrier for the second address add
 *     (`offset = offset + (s32) a2; p = (u16 *) offset;`) matches target's
 *     `addu s0,s0,s2` destination reuse where we emitted `addu v1,s2,s0`.
 *     27 -> 23.
 *
 * Applying the same reuse to the FIRST add regresses to 30 - only the
 * second one matches. Plain operand-order swaps (`offset + aN`) are inert.
 */
void hirahira_w_ctrl(s32 a0, u8 *a1, u8 *a2, s32 a3, s32 *a4)
{
    s32 *s7_a4 = a4;
    s32 *tbl = D_80094CFC;
    s32 i = 1;
    s32 base = D_800A9A10[a0];
    s16 buf[3];
    s32 saved;
    s32 *out2;
    s32 stptr;
    s32 offset;
    u16 *p;
    saved = base + 0x94;
    out2 = (s32 *) (((u8 *) s7_a4) + 0x20);
    stptr = base + 0xFC;
    loop1:
    offset = (*tbl) * 6;
    p = (u16 *) (a1 + offset);
    buf[0] = p[0];
    buf[1] = -p[1];
    tbl++;
    buf[2] = -p[2];
    func_8004A348(buf, s7_a4);
    offset = offset + (s32) a2;
    p = (u16 *) offset;
    i++;
    buf[0] = p[0];
    buf[1] = -p[1];
    buf[2] = -p[2];
    func_8004A348(buf, out2);
    func_800523E0(s7_a4, out2, a3, stptr + 0x38);
    *((s16 *) (stptr + 6)) = 2;
    stptr += 0x68;
    if (i < 0x12) {
        goto loop1;
    }
    a1 += 0x6C;
    a2 += 0x6C;
    i = 0x12;
    out2 = (s32 *) (((u8 *) s7_a4) + 0x20);
    stptr = saved + 0x750;
    loop2:
    func_80044DE4((s16 *) a1, (s16 *) a2, a3, stptr + 0x4C);
    a1 += 6;
    a2 += 6;
    buf[0] = *((u16 *) a1);
    a1 += 2;
    buf[1] = -(*((u16 *) a1));
    a1 += 2;
    buf[2] = -(*((u16 *) a1));
    a1 += 2;
    func_8004A348(buf, s7_a4);
    buf[0] = *((u16 *) a2);
    a2 += 2;
    buf[1] = -(*((u16 *) a2));
    a2 += 2;
    buf[2] = -(*((u16 *) a2));
    a2 += 2;
    func_8004A348(buf, out2);
    func_800523E0(s7_a4, out2, a3, stptr + 0x38);
    *((s16 *) (stptr + 6)) = 1;
    stptr += 0x68;
    i++;
    if (i < 0x14) {
        goto loop2;
    }
}
