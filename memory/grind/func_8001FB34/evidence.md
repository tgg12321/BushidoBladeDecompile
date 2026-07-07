# Evidence bank — func_8001FB34

- Audit diagnosis (regressions.md): One construct fails tests 2+3: `v0 = (u32)0 < (u32)v0;` mirrors the target instruction `sltu $v0, $zero, $v0` by reversed-operand unsigned comparison rather than natural `v0 != 0`; a worker should verify whether `v0 = (v0 != 0);` produces the same sltu in GCC 2.7.2 (false flag if yes; genuine cheat if slt is emitted instead), then rework the final line to the cleaner form or confirm the match.  (committed code flagged by the re-audit patrol; review and re-do in pure C if confirmed. The byte-correct construct stays on main until a clean replacement lands.)

- [s1] [fable-blitz 2026-07-07] Flagged construct located: `v0 = (u32)0 < (u32)v0;` at src/code6cac.c:2133, mirroring `sltu $v0, $zero, $v0` at asm/funcs/func_8001FB34.s:47

- [s1] [fable-blitz 2026-07-07] Compiler-source proof the natural spelling emits the same byte: tools/gcc-2.7.2/config/mips/mips.md:4978-4986 `sne_si_zero` matches (set d (ne:SI s (const_int 0))) and emits "sltu\t%0,%.,%1" - i.e. `x != 0` as a VALUE is store-flag-expanded to exactly sltu dest,$zero,src. The judge's 'verify whether v0 != 0 produces the same sltu' question is answered YES from the .md pattern; no slt path exists for the NE-zero shape

- [s1] [fable-blitz 2026-07-07] `(u32)0 < (u32)v0` and `v0 != 0` are the same predicate on the same operand; both canonicalize to the ne-zero store-flag - the construct is spelling-only, semantically true, and inert-likely

- [s1] [fable-blitz 2026-07-07] Surrounding shape is clean and should be preserved: v0 = 1 seeds the beqz $a1 delay slot (asm/funcs/func_8001FB34.s:43-44), lh 0x26C($a0) as a separate insn (line 45), then the store-flag - the two-statement source form at src/code6cac.c:2130-2133 (load into v0, then flag v0) maps 1:1; keep the load as its own statement in the respell

- [s1] [fable-blitz 2026-07-07] Rest of the function (if-chain with shared return-0 tail at .L8001FBC4, asm lines 39-41) is unflagged and matches the goto-spelled C at src/code6cac.c:2109-2135
