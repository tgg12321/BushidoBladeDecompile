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
 * s5 (synthesis): new corpus signal — 69 insns of case bodies sit AFTER the single
 * `jr $ra`; 0/1225 pure-C functions do that (tmp/grind/func_8004A4E0/s5/midret.txt).
 *
 * s6 (solver): no solver residual (INCLUDE_ASM guard); cc1 sketches confirm the epilogue is
 * always emitted last (final.c:1066) � tmp/grind/func_8004A4E0/s6/sketch.s.
 *
 * s7 (forensics): $s6 written-never-read callee-saved reg - compiled C 0/506 (flow.c:1758,
 * only global register vars survive); $s3 saved-unreferenced is NOT a signal (5/506 C do it).
 * tmp/grind/func_8004A4E0/s7/savescan4.txt.
 *
 * sandbox --disable all (s1..s7): {"score": 163, "no_c_body": true, "build_insns": 0}.
 */
INCLUDE_ASM("asm/funcs", func_8004A4E0);
