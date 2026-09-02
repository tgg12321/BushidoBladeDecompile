# Evidence bank — func_80043D34

## s1 (2026-09-02, recon) — matched: sandbox --disable all = 0 (43/43)
- HEAD chassis at dispatch: src/text1a_c.c carried a register-pin + `__asm__` body (pre-migration form, banked in retired-chassis-2026-08/body.c); pinned honest floor 35/43. Canonical gate this session: verdict C, distance 0.
- OBJECT IDENTITY: field offsets (u16 clut +0xE, u16 tpage +0x1A, u8 v0/v1/v2 at +0xD/+0x19/+0x25) match PsyQ LIBGPU.H `POLY_GT3` (0x28 bytes). The caller func_80043454 advances `a0` by 0x28 between calls (asm/funcs/func_80043454.s:182,198). Semantics identical to func_80043BD0: tpage.x (4 bits) += (s16)du >> 6; tpage.y (1 bit) += (s16)dv >> 8; v0/v1/v2 += dv; clut.x (6 bits) += (s16)dcx >> 4; clut.y (9 bits) += dcy (5th arg, `lw 0x10($sp)`, added uncast).
- FORM: verbatim transfer of the func_80043BD0 s1 score-0 spelling (memory/grind/func_80043BD0/evidence.md: u16 local copy per packed word, distinct once-written locals tx/ty/cx/cy, `x | ((w & Mclear) | (y << s))` pack, field-first adds on tpage / delta-first adds on clut, byte adds in v0,v1,v2 source order). First measurement on this chassis scored 0; no other spelling was needed or tried.
- The byte-add sb order in the target (0xD, 0x25, 0x19) is the scheduler's rearrangement of source order v0(+0xD), v1(+0x19), v2(+0x25) — same phenomenon as BD0's 0xD, 0x1D, 0x15.
- TU-local typedef POLY_GT3 sits beside POLY_FT3 in src/text1a_c.c (precedent: POLY_G4 in src/code6cac_b2_pre.c:158). No libgpu prim types exist in include/.
- Artifacts: tmp/grind/func_80043D34/s1/{apply.py,final.c,text1a_c.disable-all.o}.
- SIBLING LEAD (remaining): func_80043C7C (tpage +0x16, four v bytes at +0xD/+0x15/+0x1D/+0x25 per its pin body => POLY_FT4, 0x28 bytes) and func_80043DE0 (tpage +0x1A ... POLY_GT4, 0x34 stride per func_80043454.s:213). Same transfer should close both.
