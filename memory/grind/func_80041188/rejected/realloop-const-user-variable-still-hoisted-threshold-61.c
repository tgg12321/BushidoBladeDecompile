/* func_80041188 -- s22 REJECTED (form W4b). s19's real-loop chassis (W4) with
 * the invariant store's value made a USER VARIABLE inside the loop body:
 *     { s16 two = 2; *((s16 *)(ents + i * 0x68 + 6)) = two; }
 *
 * MEASURED: sandbox --disable all = 3 at 132/132 -- byte-identical to W4. The
 * `li 2` is still hoisted into the preheader, still spilled, still
 * rematerialised by reload into $t0 where target has $v0.
 *
 * WHY (exact arithmetic, s22 E-s22-1, from tmp/grind/func_80041188/s22/W4/
 * red.i.loop:109-111): loop.c's three movable-eligibility tests
 * (loop.c:686-700) are an OR -- making the destination a user variable only
 * falsifies test (2); test (1) still passes (maybe_never == 0, the reg is not
 * used before its set) and test (3) still passes (the whole loop body is one
 * basic block, so reg_in_basic_block_p is true). The profitability test at
 * loop.c:1631 then reads `threshold * savings * lifetime >= insn_count` with
 * threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs) = 1 * (1 + 60)
 * = 61 (MIPS FIXED_REGISTERS has 8 ones out of FIRST_PSEUDO_REGISTER = 68),
 * savings = 1, lifetime = 1 and insn_count = 48 -- the dump prints exactly
 * "Loop from 42 to 172: 48 real insns." / "Insn 155: regno 122 (life 1),
 * move-insn savings 1  moved to 315". 61 >= 48 by a margin of 13, and
 * insn_count is the ONLY C-side dial, so blocking the hoist would need 13 more
 * real insns inside loop1. The real-loop chassis therefore carries a hard +2.
 */
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
    s32 offset;
    u16 *p;
    s32 stptr2;
    ents = base + 0x94;
    out2 = (s32 *) (((u8 *) pa4) + 0x20);
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
        { s16 two = 2; *((s16 *) (ents + i * 0x68 + 6)) = two; }
        i++;
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
