/* func_8004A4E0 — no pure-C candidate exists (s1 recon finding).
 *
 * canonical() verdict: ASM-WHOLE, citing inline_asm_canonical.txt:199 (owner ruling Q40,
 * 2026-09-29 re-audit, docs/audits/func_8004A4E0-canonical-2026-09-29.md). The audit found a
 * custom callee-saved-register ABI shared across jal with func_8004A940 / func_8004A76C /
 * func_8004A808 ($s4/$s5/$s6/$fp/$s0 handoffs no C form can express without forbidden GPR
 * asm at every call site) plus 2 trapping addi and 4 ctc2 with no C form. See
 * memory/grind/func_8004A4E0/hypotheses.md H1 (KILLED, class) for the full chain.
 *
 * MIGRATION NOTE (representation banner; s18b, measured at HEAD 9d185896b): func_8004A4E0 is
 * committed on main as INCLUDE_ASM (whole-body, canonical grant inline_asm_canonical.txt:199);
 * this file carries NO C candidate - it is a comments-only record. Re-check the
 * representation in src/main/3AB48.c before trusting any state claim here.
 *
 * s18b (synthesis): the 2 trapping addi have a compiler predicate - GCC 2.7.2 addsi3_internal
 * emits only addu (tools/gcc-2.7.2/config/mips/mips.md:381); compiled C 0/1299 carry a trapping
 * addi (tmp/grind/func_8004A4E0/s18b/addi_census.txt). scan_hand_coded has no such signal.
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
 * s20 (forensics): unbounded jtbl index is reachable via GNU computed goto (no bound check;
 * tmp/grind/func_8004A4E0/s20/probe.s E_cgoto); every switch keeps it (expr.c:9963). Signal (a)
 * downgraded to "not a switch"; s8 cross-function targets + s18b addi stand.
 *
 * s19 (solver): classify has no residual - sandbox object lacks the symbol (no C body); same as s6.
 *
 * sandbox --disable all (s1..s19): {"score": 163, "no_c_body": true, "build_insns": 0}.
 */
INCLUDE_ASM("asm/funcs", func_8004A4E0);

/* s9 (forensics): helper func_8004A940 saves/restores $ra via scratchpad 4($s1);
 * compiled C 0/1164 do that (mips.c:4632 save_restore_insns bases only off $sp).
 * Callee-saved live-in at entry: 0/1164 C, but all 3 helpers A76C/A808/A940.
 * tmp/grind/func_8004A4E0/s9/rascan.txt. INCLUDE_ASM stays. */

/* s10 (forensics): scan_hand_coded rates func_8004A4E0 itself LOW 0/8 (helpers STRONG via S7);
 * jtbl targets outside the referencing function: compiled C 0/58, asm 1/8 (only this function).
 * tmp/grind/func_8004A4E0/s10/jtblscan.txt. INCLUDE_ASM stays. */

/* s11 (rederive): sibling ledgers checked (math_RotMatrix*, puts, InitGeom, patchgte,
 * chclrpad, func_800790A4) — all closed/COMPLETED, 0 grind sessions, and share only
 * global DATA references (g_draw_queue, D_80103608) with this function, not code
 * blocks; no transplantable form exists. decomp.me corpus mining (tools/decomp_me_scrape.py)
 * is infeasible in this sandbox: curl_cffi is not installed in .venv and no corpus is
 * cached under tmp/decomp_me_corpus/ (tmp/grind/func_8004A4E0/s11/decompme_probe.txt).
 * Even if reachable, decomp.me scratches are single-function compiled-C targets under
 * the same GCC per-function model the s8/s10 class kill already covers, so they cannot
 * contain this function's cross-function-register-sharing jtbl dispatch shape.
 * Floor re-confirmed 163, no_c_body, INCLUDE_ASM stays (tmp/grind/func_8004A4E0/s11/sandbox.txt). */

/* s12 (rederive): unblocked s11's decomp.me probe — `pip install curl_cffi` into .venv
 * succeeded, downloaded 316 gcc2.7.2-psx scratches to tmp/decomp_me_corpus/, ran the
 * similarity search against this function's asm. Best match similarity 0.057, against an
 * UNRELATED function (func_80044240), not a func_8004A4E0 candidate at all — null result,
 * confirming s11's prediction that no ordinary compiled-C corpus entry can resemble a body
 * whose jtbl dispatches into a different function's interior (s8 class kill). Independently
 * re-derived the jtbl_8001541C cross-function-target list from src/main/d_5BF0.c (matches
 * s8 exactly) and re-ran m2c (same missing-jump-table refusal as s8). canonical() still
 * ASM-WHOLE, sandbox still {"score": 163, "no_c_body": true}.
 * tmp/grind/func_8004A4E0/s12/decomp_me_search.txt. No live rederive-axis frontier remains;
 * every item on the s1-s11 ladder (recon/structural/permuter/enumerate/synthesis/solver/
 * forensics x3/rederive x2) has independently landed on the same ASM-WHOLE class kill.
 *
 * s13 (structural, mandated kill re-audit): fake_ablate.py on this exact candidate.c reports
 * zero FAKE constructs to ablate (file is comments-only over the unmodified INCLUDE_ASM stub),
 * confirming no prior kill was measured against a FAKE-propped chassis. sandbox --disable all
 * --diff re-confirms {"score": 163, "no_c_body": true} with "no diff: still whole-body
 * INCLUDE_ASM". New class kill specific to this session's structural modality: every lever in
 * docs/MATCHING.md's structural family (var splits, decl order, type narrowing, statement
 * re-association) transforms an EXISTING single-function C body (toplev.c:2647 compiles one
 * function at a time; stmt.c:4704 lowers one function's switch into that function's own jump
 * table) -- none can be the mechanism that relocates a case arm into a DIFFERENT function's
 * body, which is what s8/s12's jtbl_8001541C finding requires. Structural modality has no
 * applicable surface here until the s8 class kill is overturned. tmp/grind/func_8004A4E0/s13/.
 *
 * s14 (structural, mandated kill re-audit repeat): identical re-measurement at HEAD
 * 8a35d2704 -- same {"score": 163, "no_c_body": true} and same "no diff" message. Confirms
 * s13 char-for-char two sessions later.
 *
 * s15 (enumerate): the systematic spelling sweep (mark a differing region ENUM-BEGIN/END,
 * tools/spelling_enum.py + tools/sweep_variants.py) presupposes an existing candidate C file
 * with a block of named sub-expressions to carve a region out of. This file has zero C
 * statements -- it is comments-only over the unmodified whole-body INCLUDE_ASM stub. Fresh
 * sandbox --diff and canonical re-measured at HEAD b9601d742: identical {"score": 163,
 * "no_c_body": true}, identical ASM-WHOLE reason string. No region exists to enumerate
 * spellings of; the sweep's precondition is unmet for the same structural reason s13/s14
 * found (no existing C statements/declarations), now confirmed for the enumerate axis too.
 *
 * s16 (enumerate, 2nd re-run): sandbox --diff re-measured again at HEAD e0b805366, byte-for-
 * byte identical fields to s15 ({"score":163,"no_c_body":true,"build_insns":0}). No frontier
 * change. See hypotheses.md [s16] for the full record.
 *
 * s17 (enumerate, 3rd re-run): canonical + sandbox re-measured at HEAD 0dbd72780 — identical
 * ASM-WHOLE verdict and {"score":163,"no_c_body":true,"build_insns":0}. Sweep precondition
 * (an existing C region to carve) still unmet; zero C statements in src/main/3AB48.c. Checked
 * docs/grind/decisions.md: no existing entry for this function. See hypotheses.md [s17].
 *
 * s21 (forensics): hazard-free explicit nop at 0x8004A718 (addiu $s4 -> nop -> jal); GCC/maspsx
 * emit nop only in delay slots/load/HI-LO hazards (mips.c:3618, mips.c:5200); compiled C 0/1297
 * (tmp/grind/func_8004A4E0/s21/nop_census.txt, probe.objdump.txt).
 */
