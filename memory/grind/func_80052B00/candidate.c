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
