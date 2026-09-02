# Evidence bank — func_80043C7C

## s1 (recon, 2026-09-02)
- canonical verdict: C (46 insns, distance 37 on the inherited body).
- Inherited src body was a register-pin + `__asm__ lw $5,16($29)` cheat form
  (banked at rejected/register-pin-asm-lw-cheat-body.c). Sandbox honest floor
  with it stripped: 37.
- Field-offset map from asm/funcs/func_80043C7C.s: u16 tpage @+0x16, u16 clut
  @+0xE, byte adds at +0xD/+0x15/+0x1D/+0x25 = v0..v3 of PsyQ POLY_FT4
  (0x28 bytes). Param shifts: a1 sra 22 (= (s16)>>6), a2 sra 24 (= (s16)>>8),
  a3 sra 20 (= (s16)>>4), a4 from 0x10($sp). Same shape as the matched
  triangle sibling func_80043BD0 (POLY_FT3), one more vertex.
- Pure-C form (candidate.c: POLY_FT4 typedef + func_80043BD0 body with
  `p->v3 += dv;` added, void return) measured sandbox --disable all = 0
  (46/46 insns) this session. Scheduler-interleaved v-load/store order
  (0x15,0xD,0x25,0x1D) falls out of source order v0,v1,v2,v3 unaided.
- No other src/include reference to func_80043C7C (only the non-build
  include/m2c_context.h prototype).
