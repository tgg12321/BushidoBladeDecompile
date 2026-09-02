/* s8 RULING FORM -- NOT a proposal, NOT applied to main.
 *
 * MEASURED 2026-09-02 (session s8, forensics):
 *   sandbox func_80030580 --disable all = 0  (chassis pure-c-floor2-body.c + these
 *   two edits, nothing else; HEAD/INCLUDE_ASM reads 148, chassis alone reads 2).
 *   Instrumented cc1 (BB2_FRAME_DEBUG=1): vars=24 bodydiff=4 sp=0,
 *   ctx=spill_new_p89 | spill_new_p116 | spill_new_p188 | round_frame.
 *   bodydiff=4 is ONLY the two `subu $sp,$sp,N` / `addu $sp,$sp,N` prologue and
 *   epilogue lines -- the 140 body instructions are byte-identical to the target.
 *
 * MECHANISM (dumps in tmp/grind/func_80030580/dumps/):
 *   Each `+ K - K` term, where K is a MEMORY read not already loaded at that point,
 *   forces two lhu insns into RTL.  cse2 proves the sum equals the original value and
 *   the loads die; combine.c distribute_notes then cannot rehome the REG_DEAD notes of
 *   the address/value pseudos and emits bare `(insn (use (reg:SI N)))` orphans
 *   (code6cac_b.combine insns 413/414/416 for regs 89/116/188 -- absent from
 *   code6cac_b.cse2).  Those pseudos have refs but no live range, so global.c gives
 *   them no hard register and reload1.c alter_reg calls assign_stack_local(SImode, 8,
 *   -1) on each -> +8 frame bytes apiece, ZERO instructions.  8 (the base Judge-fold
 *   orphan p110) + 8 + 8 = 24 = the target's frame.
 *
 * WHY IT IS FILED AS A RULING REQUEST AND NOT A CANDIDATE:
 *   `x + K - K` has no observable effect on the function's output.  Cheat-checklist
 *   T1 (semantic purpose) and T2 (human-programmer) both FAIL on this agent's own
 *   reading, and no frozen SOTN family covers "an expression whose value cse proves
 *   redundant".  Per the first-reach rule the correct move is a ruling, not a
 *   submission.  Do NOT apply this file without a ruling.
 */
s32 *func_80030580(s32 *arg0, s32 arg1) {
    u8 *obj;
    u8 *src = (u8 *)arg0;
    s16 *tbl;
    s32 i;

    obj = (u8 *)&D_80106A78;
    for (i = 0; i < 12; i++, obj += 0x64) {
        if (*(s16 *)(obj + 2) == -1 && *(u8 *)(obj + 0xA) == 0xFF) break;
    }
    *(u8 *)(obj + 0xA) = i;
    *(s16 *)(obj + 2) = arg1;
    *(u8 *)(obj + 7) = 0;
    *(u8 *)(obj + 8) = 0;
    *(u8 *)(obj + 4) = 1;
    *(u8 *)(obj + 6) = *(u16 *)(src + 4);
    *(s32 *)(obj + 0x2C) = *(s32 *)(src + 0xF4) + *(u16 *)(src + 4) - *(u16 *)(src + 4);
    *(s32 *)(obj + 0x30) = *(s32 *)(src + 0xF8) - *(s16 *)(src + 0x1A) / 32;
    *(s32 *)(obj + 0x34) = *(s32 *)(src + 0xFC);
    tbl = &D_8008E194 + arg1 * 7;
    *(s32 *)(obj + 0x44) = ((&Judge)[*(u16 *)(src + 0x1CA) & 0xFFF] * tbl[2]) >> 12;
    *(s32 *)(obj + 0x48) = tbl[3];
    *(s32 *)(obj + 0x4C) = ((&Judge)[(*(s16 *)(src + 0x1CA) + 0x400) & 0xFFF] * tbl[2]) >> 12;
    *(s32 *)(obj + 0x2C) += *(s32 *)(obj + 0x44);
    *(s32 *)(obj + 0x30) += *(s32 *)(obj + 0x48);
    *(s32 *)(obj + 0x34) += *(s32 *)(obj + 0x4C);
    *(s32 *)(obj + 0x2C) += *(s32 *)(obj + 0x44) / 2;
    *(s32 *)(obj + 0x30) += *(s32 *)(obj + 0x48) / 2;
    *(s32 *)(obj + 0x34) += *(s32 *)(obj + 0x4C) / 2;
    *(Vec3i *)(obj + 0x38) = *(Vec3i *)(obj + 0x2C);
    *(s16 *)(obj + 0x54) = 0;
    *(u16 *)(obj + 0x56) = *(u16 *)(src + 0x1CA);
    *(s16 *)(obj + 0x58) = 0;
    if (tbl[0] == 1) {
        *(s16 *)(obj + 0x5C) = 0;
        *(u16 *)(obj + 0x5E) = *(u16 *)(tbl + 4);
        *(s16 *)(obj + 0x60) = 0;
    } else if (tbl[0] == 2) {
        *(u16 *)(obj + 0x5C) = *(u16 *)(tbl + 4) + *(u16 *)(src + 4) - *(u16 *)(src + 4);
        *(s16 *)(obj + 0x5E) = 0;
        *(s16 *)(obj + 0x60) = 0;
    } else if (tbl[0] == 3) {
        *(s16 *)(obj + 0x5C) = 0;
        *(u16 *)(obj + 0x5E) = *(u16 *)(tbl + 4);
        *(s16 *)(obj + 0x60) = 0;
    } else {
        *(s16 *)(obj + 0x5C) = 0;
        *(s16 *)(obj + 0x5E) = 0;
        *(s16 *)(obj + 0x60) = 0;
    }
    *(s32 *)(obj + 0x50) = 1;
    *(u8 *)(obj + 5) = 0;
    *(s16 *)obj = 0;
    return (s32 *)obj;
}
