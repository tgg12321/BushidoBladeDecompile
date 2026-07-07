# Evidence bank — func_80044F30

- Audit diagnosis (regressions.md): Function-pointer cast cheat: definition declares (s32 a0) with ((void (*)(s32))func_80044E74)(a0+0x27), but callers (text1a_c2.c:14,77,79) declare and call it as func_80044F30(s32, s32) — two explicit args. The cast exists solely to compensate for the missing second parameter declaration, relying on MIPS $a1 register passthrough. Sibling saTan2InfoInit_80044F80 proves the clean form works: void func_80044F30(s32 a0, s32 a1) { func_80044E74(a0+0x27, a1); } produces identical target asm without any cast. Worker must rewrite with two-param signature, drop the cast, and update m2c_context.h:601 accordingly.  (committed code flagged by the re-audit patrol; review and re-do in pure C if confirmed. The byte-correct construct stays on main until a clean replacement lands.)

- [s1] [fable-blitz 2026-07-07] Flagged construct: src/text1a_c.c:1457-1459 -- `void func_80044F30(s32 a0) { ((void (*)(s32))func_80044E74)(a0 + 0x27); }`. The cast exists solely to paper over the missing second parameter; $a1 passes through untouched by ABI accident.

- [s1] [fable-blitz 2026-07-07] Byte-shape proof by sibling: asm/funcs/func_80044F30.s and asm/funcs/saTan2InfoInit_80044F80.s are instruction-for-instruction identical 9-insn bodies (addiu sp,-0x18; sw ra,0x10; jal func_80044E74; delay addiu a0,a0,IMM; lw ra; addiu sp; jr; nop) differing ONLY in the delay-slot immediate (0x27 vs 0x4D). The sibling's committed source is the clean two-param form `void saTan2InfoInit_80044F80(s32 a0, s32 a1) { func_80044E74(a0 + 0x4D, a1); }` (src/text1a_c.c:1467-1469).

- [s1] [fable-blitz 2026-07-07] A second same-file sibling corroborates: func_80044F50 (src/text1a_c.c:1460-1466) forwards a2 as func_80044E74's second arg through plain two-arg calls -- no cast anywhere in the family except the flagged site.

- [s1] [fable-blitz 2026-07-07] Callers already use the two-arg signature: src/text1a_c2.c:14 declares `extern void func_80044F30(s32, s32);` and calls it with two args at text1a_c2.c:77 and 79 -- the current one-param definition is the odd man out (cross-TU declaration mismatch that only links because prototypes are per-TU).

- [s1] [fable-blitz 2026-07-07] include/m2c_context.h:601 carries the stale one-param prototype `void func_80044F30(s32 a0);` -- must be updated to (s32, s32) with the rewrite, exactly as the judge constraint instructs.

- [s1] [fable-blitz 2026-07-07] Callee-side mechanism: `func_80044E74(a0 + 0x27, a1)` with a1 already resident in $a1 emits NO move for the second argument (O32 same-register passthrough; calls.c/expand_call loads only args not already in their target regs) -- which is why the clean form and the cast form produce identical bytes.

- [s1] [fable-blitz 2026-07-07] No do-while/FAKE constructs, no regfix implications; the fix surface is 3 lines (definition signature, call expression, m2c_context.h prototype).
