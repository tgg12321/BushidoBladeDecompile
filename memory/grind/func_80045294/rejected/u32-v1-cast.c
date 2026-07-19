/* s12 KILLED (neutral): declare v1 as u32 with (u32)a0 cast on shift operand.
 * Result: score=2 NEUTRAL. The declared C type of v1 has no visible effect on
 * cc1 codegen because the ashift RTL node is signedness-agnostic for a constant
 * shift; expand_shift produces (ashift (reg:SI a0) (const_int 4)) regardless.
 * The subsequent address arith `(u8 *)&D_800EED14 + v1` still routes through
 * an addu insn identically. Complements s5's a0*16 / (u32)a0*16u / (u8*)0+a0
 * neutral kills by extending the width/signedness axis to the DEST-type
 * declaration position.
 */
void func_80045294(s32 a0, s32 a1) {
    s32 sum = 0;
    u32 v1 = (u32)a0 << 4;
    s32 s4 = *(s32 *)((u8 *)&D_800EED14 + v1);
    s32 i = a0;
    s32 count = D_800A33AC;
    s32 s5 = s4 + a1;
    /* ... rest identical to candidate.c ... */
}
