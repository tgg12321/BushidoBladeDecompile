/* func_80052B00 — grind candidate, session 1 (recon, 2026-08-01).
 *
 * STATUS: NOT APPLIED to src/text1b.c. This form is a whole-body canonical-asm
 * (COMPLETED-INLINE-ASM-CANONICAL) packaging and is only legal once a matching
 * entry exists in inline_asm_canonical.txt — a file this grind session is not
 * permitted to touch. It is banked here so the session (or operator) that owns
 * that surface can apply it directly.
 *
 * WHY this form rather than pure C. func_80052B00 is LIBGTE SetRotMatrix +
 * SetTransMatrix fused: 8 word loads from *a0 feeding cop2 control registers
 * CR0-CR7 (packed 3x3 rotation matrix RT11RT12/RT13RT21/RT22RT23/RT31RT32/RT33
 * in CR0-CR4, translation vector TRX/TRY/TRZ in CR5-CR7). There is zero
 * general-purpose computation; the loads are mechanical I/O packaging that
 * exists solely to feed the hardcoded cop2 register encodings.
 *
 * The target's `jr $ra` delay slot holds `ctc2 $t7, $7`. `ctc2` has no C
 * analog, so it can only come from an __asm__ block — and
 * tools/gcc-2.7.2/reorg.c:730-735 (stop_search_p) halts fill_simple_delay_slots
 * unconditionally at any ASM_INPUT / asm_noperands insn. GCC 2.7.2 therefore
 * can never place this instruction in the delay slot from ANY C source. That is
 * an impossibility proof, not a plateau: the honest sandbox floor of 18 is not
 * a number that further pure-C search can move to 0.
 *
 * Precedent: func_80052B44 (src/text1b.c:10995, the very next function in this
 * file) is the same construct with a 5-word matrix and was Judge-authorized
 * canonical-asm on 2026-07-27 citing the same reorg.c mechanism
 * (inline_asm_canonical.txt:340). Same family, already authorized:
 * gte_SetRotMatrix (:326), gte_SetColorMatrix (:325), gte_SetTransVector (:324),
 * func_8007ED6C (:308). Category: the [[gte-wrapper-misroute-park]] GTE-leaf
 * carve-out (no-C-form by construction), not the scan_hand_coded S1-S8 route.
 *
 * PACKAGING NOTES (mirrored verbatim from the authorized func_80052B44 block):
 *  - .set directives are duplicated in TAB form and SPACE form; maspsx's
 *    noreorder stripping requires both ([[maspsx-noreorder-stripping]]).
 *  - Memory offsets are DECIMAL — maspsx parses `0x0($a0)` as base-10 and
 *    chokes; hex is only safe for plain immediates.
 *  - Applying this replaces the whole `void func_80052B00(s32 *matrix) {...}`
 *    body at src/text1b.c:10969-10994 (which carries 8 forbidden
 *    `register asm("$N")` pins) and lets `retire func_80052B00` drop the tree's
 *    single rule, regfix.txt:3411 `func_80052B00: fill_delay @ 16 <- 15`.
 *
 * SESSION 2 (structural, 2026-08-01) — form UNCHANGED, now measured rather
 * than argued. 14 distinct pure-C spellings were compiled and scored with
 * `sandbox --disable all`: 8 tie the floor at exactly 18, 5 are strictly worse,
 * none improve it, and ALL 14 emit `jr $ra; nop` — the eighth `ctc2` is always
 * hoisted above the jump, including under a `do { } while (0)` probe
 * (NOTE_INSN_LOOP_BEG, the one documented reorg.c-perturbing lever). H1 is
 * therefore empirical, not just a reading of reorg.c. A complete census of the
 * tree's cop2-control-loader leaves (6 bodies) shows func_80052B00 is the last
 * unauthorized member, and that the family partitions on exactly the H1
 * criterion. Full data: memory/grind/func_80052B00/evidence.md §Session 2 and
 * memory/grind/func_80052B00/rejected/structural-sweep-14-forms-floor-18.c
 * (which also records the honest pure-C, pin-free floor-18 body).
 *
 * SESSION 3 (structural, 2026-08-01) — THE HONEST FLOOR MOVED 18 -> 17, and the
 * movement STRENGTHENS this disposition rather than undermining it. Sessions
 * 1-2 always emitted eight separate one-operand `__asm__ ("ctc2 %0,$N")`
 * statements; session 3 fused all eight cop2 writes into ONE `__asm__` with
 * eight "r" inputs, which forces all eight loaded values to be simultaneously
 * live and thereby removes the deferred `matrix[0]` load AND the extra
 * load-delay nop. The resulting honest body is instruction-for-instruction
 * ISOMORPHIC to the target — same opcodes, same order, same offsets — and the
 * whole 17-point residual is now exactly (a) 16 instructions naming the wrong
 * GPRs ({v0,v1,a1,a2,a3,t0,t1,t2} vs the target's {t0..t7}) and (b) the one
 * unfilled `jr $ra` delay slot.
 *
 * Both are mechanically closed. (a): `tools/gcc-2.7.2/config/mips/mips.h`
 * defines no `REG_ALLOC_ORDER`, so local-alloc walks hard registers in ascending
 * number order and takes the first non-conflicting one; only a call conflict or
 * a copy suggestion to a named hard register can move that, and a call-free
 * leaf has neither — measured invariant across 18 fused-form spellings
 * (parameter types, declaration order, "d" vs "r" constraints, a ninth base
 * operand, a copy chain, 1/2/4/7/8-write granularities). (b): H1, now
 * corroborated by 32/32 measured spellings across three sessions.
 *
 * The pin-free floor-17 body is banked at
 * memory/grind/func_80052B00/best_pure_c_fused8_floor17.c. If the canonical-asm
 * disposition below is ever REFUSED, that body — not the pinned HEAD body — is
 * what an operator should ship: it strictly reduces the cheat surface (eight
 * register pins disappear; only the delay-slot paperwork and register fixes
 * remain). Full data: evidence.md §Session 3, hypotheses.md H5/H6/H7,
 * tmp/grind/func_80052B00/s3/.
 *
 * SESSION 4 (permuter, 2026-08-01) - form UNCHANGED; the last remaining
 * automated search modality is now measured dead. Two telemetry-tracked
 * decomp-permuter campaigns (tools/permuter_campaign.py, labels
 * `fused8-r-constraints` and `struct-fields-direct-rvalue-fused8`) ran 117,314
 * iterations across two structurally distinct chassis against a real
 * cc1 -> prologue_fix -> maspsx -> multu_pad workspace. Campaign 1 (the s3
 * fused body) produced ZERO outputs - not even a score tie. Campaign 2 (struct
 * field references fed directly as the fused asm's eight operands, no named
 * temporaries) produced exactly one output, a TIE at the base score of 140,
 * consisting of a dead `if (new_var)` on an uninitialized local - noise, and a
 * dead-local cheat shape besides (banked at
 * rejected/permuter-tie-dead-if-noise.c). Nothing below the honest floor of 17.
 * Two incidental confirmations: the no-temporaries chassis emits the same
 * {v0,v1,a1,a2,a3,t0,t1,t2} register set (H7 now holds across 19 spellings),
 * and a minimal standalone TU reproduces this function's in-tree codegen
 * exactly (H9), so TU context is not a variable here. Full data: evidence.md
 * Session 4, hypotheses.md H8/H9, tmp/grind/func_80052B00/s4/.
 *
 * SESSION 5 (permuter — DIRECTED, 2026-08-01) — form UNCHANGED; the last
 * uncovered corner of the automated search space is now measured empty.
 * Session 4 killed the UNDIRECTED permuter axis; session 5 killed the DIRECTED
 * one, which is a different search: PERM_* macros make decomp-permuter
 * ENUMERATE a cross-product rather than randomly mutate one base. The chassis
 * (tmp/grind/func_80052B00/s5/ws3/base.c, hand-authored per
 * .claude/rules/permuter-directives.md because permuter_annotate.py's four
 * hints all need a construct this body lacks) crossed PERM_LINESWAP over the
 * eight load statements — all 8! = 40,320 orderings, where the s1-s3 hand
 * sweeps had covered exactly TWO — with PERM_GENERAL over four asm-operand
 * shapes, a 161,280-point space. 46,653 iterations / 1,869 s / 8 workers
 * produced ZERO outputs, and the score distribution is strictly bimodal: 140
 * (= base = the honest floor of 17) and 180 (= base + 40 = eight more
 * register-name mismatches), nothing in between and nothing below. Load order
 * is the one lever in that space that moves the score at all and it moves it
 * only UPWARD, permuting which pseudo lands on which ascending hard register
 * without ever moving the set off {v0,v1,a1,a2,a3,t0,t1,t2} — exactly what
 * H7's no-REG_ALLOC_ORDER mechanism predicts. Running totals across sessions
 * 1-5: 32 hand-measured spellings plus 163,967 permuter iterations, zero
 * delay-slot fills and nothing below the honest floor. Full data: evidence.md
 * §Session 5, hypotheses.md H10, tmp/grind/func_80052B00/s5/.
 *
 * SESSION 6 (forensics, 2026-08-01) — form UNCHANGED; the two residual
 * components now have named passes, named source lines, and a compiler-internal
 * differential, and one long-standing ledger claim is CORRECTED.
 *
 * Vehicle: the honest floor-17 fused body compiled in a minimal standalone TU
 * (faithful per H9) with the instrumented cc1 (tools/gcc-2.7.2/cc1, NOT
 * build/cc1) and -da; full pass dump set plus four differential controls in
 * tmp/grind/func_80052B00/s6/.
 *
 *  - CORRECTION. Sessions 3-5 attributed the register naming to local-alloc's
 *    find_free_reg. It is not: `combine` folds all eight (mem) loads directly
 *    into the fused asm's operands (9 (set (reg ...)) insns survive .flow, ZERO
 *    survive .combine), so the .lreg dump holds a single insn and local-alloc
 *    has nothing to allocate. The eight `lw` are RELOAD-generated, and the
 *    register choice is reload1.c:3606 order_regs_for_reload(), #else branch
 *    (no REG_ALLOC_ORDER in mips.h): zero-use call-used hard regs in ascending
 *    regno, with $4 skipped because reload1.c:486/:3651 mark explicitly-used
 *    regs as bad spill regs. The .greg dump prints it verbatim: ";; Need 8 regs
 *    of class GR_REGS" / "Spilling reg 2. 3. 5. 6. 7. 8. 9. 10."
 *
 *  - THE REGISTER SET IS REACHABLE, AND PRICED. ctlE (the fused body plus five
 *    extra locals occupying $2,$3,$5,$6,$7 across the asm) emits
 *    `lw $8,0($4) … lw $15,28($4)` / `ctc2 $8,$0 … ctc2 $15,$7` — the target's
 *    registers, order and offsets EXACTLY. It costs 10 extra instructions (27 vs
 *    the target's 17), and no zero-cost occupancy exists: extra unused register
 *    parameters do not set regs_ever_live (ctlF/ctlG, spill list unchanged). So
 *    the register residual is now closed by arithmetic, not by absence of
 *    evidence. Banked: rejected/regocc5-reproduces-t0t7-costs-10-extra-insns.c.
 *
 *  - H1 GOT A DIFFERENTIAL. The honest body's .dbr header reads "3 insns needing
 *    delay slots / 3 got 0 delays" and the return stays a bare
 *    (jump_insn … (parallel[(return)(use (reg:SI 31 ra))]) {return_internal}),
 *    never a (sequence); cc1 emits `j $31` with no .set noreorder block. Control
 *    ctlA — the identical body with one plain C store after the asm — flips to
 *    "2 got 0 delays, 1 got 1 delays" and emits `.set noreorder / j $31 / sw`.
 *    reorg is willing and able to fill this jr; the ASM insn is the only thing
 *    stopping it, exactly as reorg.c:730-735 stop_search_p specifies.
 *
 * Net for this disposition: GCC in this toolchain can produce the target's
 * register names OR its 17-instruction length, never both, and can never produce
 * the delay-slot `ctc2` from C at all. The target does all three at once. Full
 * data: evidence.md §s6, hypotheses.md H11-H15, tmp/grind/func_80052B00/s6/.
 */
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80052B00\n"
    "    lw     $t0, 0($a0)\n"
    "    lw     $t1, 4($a0)\n"
    "    lw     $t2, 8($a0)\n"
    "    lw     $t3, 12($a0)\n"
    "    lw     $t4, 16($a0)\n"
    "    lw     $t5, 20($a0)\n"
    "    lw     $t6, 24($a0)\n"
    "    lw     $t7, 28($a0)\n"
    "    ctc2   $t0, $0\n"
    "    ctc2   $t1, $1\n"
    "    ctc2   $t2, $2\n"
    "    ctc2   $t3, $3\n"
    "    ctc2   $t4, $4\n"
    "    ctc2   $t5, $5\n"
    "    ctc2   $t6, $6\n"
    "    jr     $ra\n"
    "    ctc2   $t7, $7\n"
    "endlabel func_80052B00\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
