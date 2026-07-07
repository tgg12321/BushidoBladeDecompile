# Evidence bank — func_800343F0

- Audit diagnosis (regressions.md): Cheat: `*(s8 *)&D_80102787` where D_80102787 is `extern u8`. This pointer-type-cast is used to force a `lb` (signed byte load) instruction instead of `lbu + sll 24 + sra 24`, compensating for an incorrect `u8` type declaration in include/code6cac.h. Observable behavior is byte-identical to `(s8)D_80102787`; the construct serves no semantic purpose beyond GCC instruction-selection coercion. Analogous to the forbidden `*(volatile T *)&G` family (same structural intent: take address of global, cast to different pointer type, coerce codegen). Clean fix: change `extern u8 D_80102787;` to `extern s8 D_80102787;` in include/code6cac.h (target asm `lb` proves original was signed), then write `s32 val_87 = D_80102787;` without any cast.  (committed code flagged by the re-audit patrol; review and re-do in pure C if confirmed. The byte-correct construct stays on main until a clean replacement lands.)

- [s1] [fable-blitz 2026-07-07] Flagged construct located: src/code6cac_b.c:3969 `s32 val_87 = *(s8 *)&D_80102787;` against `extern u8 D_80102787;` (include/code6cac.h:409).

- [s1] [fable-blitz 2026-07-07] Target asm/funcs/func_800343F0.s:11 loads D_80102787 with `lb` and stores $a3 to D_800A3140 (line 26) with NO sll/sra pair -- while the u8-declared neighbors D_80102784/85/86 load lbu + explicit sll 24/sra 24 (lines 5-9, 15-20, matching the (s8) VALUE casts at :3966-3968). The lb requires a signed-typed access: the pun is LOAD-BEARING, and the within-function contrast (3 lbu+extend vs 1 native lb) is strong evidence the ORIGINAL declared 87 signed while 84-86 were unsigned.

- [s1] [fable-blitz 2026-07-07] Blast-radius census (grep D_80102787): committed C uses are only :3969 (this site) and src/code6cac.c:1349 `D_80102787 = 0;` -- a byte STORE (sb $zero, asm/funcs/func_8001C444.s:30), width-neutral under retype. No other src reference -> retyping the header to s8 is safe for all committed code.

- [s1] [fable-blitz 2026-07-07] Future caveat: undecompiled func_80034708 reads D_80102787 with `lbu` (asm/funcs/func_80034708.s:533) and takes its address (lines 344, 435). Under the s8 decl that function will need a (u8) VALUE cast (combine.c narrows zero_extend-of-load to lbu) when it is eventually matched -- note for its future ledger, not a blocker.

- [s1] [fable-blitz 2026-07-07] Judge's clean fix (retype include/code6cac.h:409 u8 -> s8, then `s32 val_87 = D_80102787;` with no cast) is mechanism-sound: type-level signed byte access emits lb natively; no coercion construct remains.
