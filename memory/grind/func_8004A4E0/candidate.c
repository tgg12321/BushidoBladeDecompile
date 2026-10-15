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
 * s8 (rederive): m2c refuses this body outright (jump table not inline-providable).
 * Decisive NEW finding: jtbl_8001541C (func_8004A4E0's own case-dispatch table,
 * src/main/d_5BF0.c) has 8/16 entries that are jlabels living only inside
 * func_8004A940's body (asm/funcs/func_8004A940.s) — a different function's
 * own prologue-protected code. func_8004A4E0's switch jumps directly into the
 * middle of func_8004A940, bypassing its prologue. No switch/goto/fnptr
 * lowering under GCC 2.7.2's per-function compilation model (toplev.c:2647,
 * stmt.c:4704) can put a jump-table entry inside a DIFFERENT function's body.
 * Class kill — see hypotheses.md [s8], tmp/grind/func_8004A4E0/s8/.
 *
 * sandbox --disable all (s1..s7): {"score": 163, "no_c_body": true, "build_insns": 0}.
 */
INCLUDE_ASM("asm/funcs", func_8004A4E0);

/* s9 (forensics): helper func_8004A940 saves/restores $ra via scratchpad 4($s1);
 * compiled C 0/1164 do that (mips.c:4632 save_restore_insns bases only off $sp).
 * Callee-saved live-in at entry: 0/1164 C, but all 3 helpers A76C/A808/A940.
 * tmp/grind/func_8004A4E0/s9/rascan.txt. INCLUDE_ASM stays. */

/* s10 (forensics): scan_hand_coded rates func_8004A4E0 itself LOW 0/8 (helpers STRONG via S7);
 * jtbl targets outside the referencing function: compiled C 0/58, asm 1/8 (only this function).
 * tmp/grind/func_8004A4E0/s10/jtblscan.txt. INCLUDE_ASM stays. */
