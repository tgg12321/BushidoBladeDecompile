/* func_8004A4E0 — no pure-C candidate exists (s1 recon finding).
 *
 * canonical() verdict: ASM-WHOLE, citing inline_asm_canonical.txt:199 (owner ruling Q40,
 * 2026-09-29 re-audit, docs/audits/func_8004A4E0-canonical-2026-09-29.md). The audit found a
 * custom callee-saved-register ABI shared across jal with func_8004A940 / func_8004A76C /
 * func_8004A808 ($s4/$s5/$s6/$fp/$s0 handoffs no C form can express without forbidden GPR
 * asm at every call site) plus 2 trapping addi and 4 ctc2 with no C form. See
 * memory/grind/func_8004A4E0/hypotheses.md H1 (KILLED, class) for the full chain.
 *
 * Current src/main/3AB48.c body (unchanged this session):
 *   INCLUDE_ASM("asm/funcs", func_8004A4E0);
 *
 * sandbox --disable all this session: {"score": 163, "no_c_body": true, "build_insns": 0}.
 */
INCLUDE_ASM("asm/funcs", func_8004A4E0);
