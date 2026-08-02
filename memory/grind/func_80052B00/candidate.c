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
 *
 * SESSION 7 (forensics, 2026-08-01) — form UNCHANGED; the register axis now has
 * a SECOND independent impossibility proof, and the s6 frontier's last open
 * corner is measured shut.
 *
 *  - H16, THE STRONGEST RESULT ON THIS FUNCTION SO FAR: the target is not a
 *    fixed point of this compiler configuration. `reload1.c:3606
 *    order_regs_for_reload()` admits a hard register to `bad_spill_regs` by
 *    exactly two routes — `fixed_regs[i]`, or `regs_explicitly_used[i]`, which
 *    `reload1.c:486` copies wholesale from `regs_ever_live[]` — plus uses
 *    contributed by pseudos already assigned to hard registers (of which there
 *    are none here, since combine folds all eight loads into the asm). On MIPS
 *    with these flags $2,$3,$5,$6,$7 are not fixed. And the target's 17
 *    instructions mention only $4, $8..$15 and $31 — $2,$3,$5,$6,$7 appear
 *    NOWHERE in them. So a C source emitting exactly the target's instruction
 *    stream would leave all five spillable and reload would hand the eight
 *    values $2,$3,$5,$6,$7,$8,$9,$10 — precisely the honest output we measure —
 *    never $t0..$t7. The shipped register allocation is unreachable from the
 *    shipped instruction stream. Verbatim source:
 *    s7/reload1_order_regs_for_reload.txt.
 *
 *  - This is ORTHOGONAL to H1. H1 says the delay-slot `ctc2` can never come from
 *    C (reorg.c:730-735 stop_search_p halts at any asm insn). H16 says the
 *    register NAMES can never come from C either. Two independent proofs, and
 *    together they say these bytes are not GCC 2.7.2 output from any C source in
 *    this configuration — affirmative provenance evidence for hand-written asm
 *    rather than one more null result, and exactly why the HEAD body needs eight
 *    `register asm("$N")` pins PLUS regfix.txt:3411 to reproduce them.
 *
 *  - H17 KILLED the four constructs the s6 frontier named as the last untested
 *    corner. Measured with cc1 -da (s7/ctl_occupancy.sh, dumps in s7/dumps/):
 *    an s32 return value is the ONE zero-cost occupancy construct — its
 *    `move $2,$0` is absorbed by the empty `jr $ra` delay slot — and it buys
 *    exactly one of the five registers needed (score unchanged at 17). A DImode
 *    return buys $2+$3 for +2 insns. Varargs buys NOTHING (register set
 *    unchanged despite the emitted `sw $5/$6/$7` homing) and costs +4, killing
 *    [[fake-varargs-explicit-homing]] here. setjmp costs +21, alloca +28.
 *
 *  - H18 CORRECTS six sessions of wording, honestly: the register residual was
 *    never "unreachable", it is PRICED, and the price is score-positive.
 *    `sandbox --disable all` over the full curve (s7/sweep_price_results.txt):
 *    base 17 / ret-s32 17 / ret-dimode 19 / occ3-param-store 20 /
 *    occ5-param-store-dimode **7** / occ5-locals-ctlE (the s6 shape, scored for
 *    the first time) **11**. The two sub-17 forms reproduce `ctc2 t0..t7`
 *    exactly — and both are cheats: they add parameters no caller passes, write
 *    memory past the eight words the function reads, and change the return type,
 *    all solely to make five hard registers appear in the RTL. That is a
 *    register pin expressed through the function signature. Both are banked
 *    under rejected/ and neither can reach 0 regardless (the extra instructions
 *    have no home in the target's 17, and the delay slot is still `nop`). The
 *    honest floor stays 17. Full data: evidence.md §Session 7, hypotheses.md
 *    H16-H18, tmp/grind/func_80052B00/s7/.
 *
 * SESSION 8 (rederive, 2026-08-01) - form UNCHANGED, and for the first time the
 * argument for it rests on AFFIRMATIVE evidence rather than on an accumulation
 * of null results. The rederive brief asks for a structurally different shape;
 * what this session found was a different QUESTION. Sessions 1-7 all asked "can
 * the C make cc1 fill the delay slot?" and none asked which pipeline stage
 * actually emits the nop, or whether a LATER stage could have filled it.
 *
 *  - F1: cc1 does NOT emit the delay-slot nop. Its tail is a bare `j $31` with
 *    no delay-slot instruction and no `.set noreorder` wrapper - it hands the
 *    slot to the assembler. maspsx appends `nop  # DEBUG: branch/jump`
 *    (tools/maspsx/maspsx/__init__.py:1192-1195) and forces `.set noreorder`
 *    after every `.ent` (:945-948), which also denies GNU as its own
 *    reorder-mode swap. Six sessions had attributed the nop solely to reorg.c.
 *
 *  - F2, THE LOAD-BEARING RESULT: that opened a real alternative - if the
 *    ORIGINAL assembler filled `j $31` slots in reorder mode (which is exactly
 *    what the `fill_delay` regfix action emulates, and four of the tree's six
 *    fill_delay rules are "previous insn into the jr slot"), then the target's
 *    delay-slot ctc2 would need no C explanation at all and this whole
 *    disposition would collapse into a maspsx fidelity gap. It is FALSE, by
 *    whole-binary census: of 1,369 functions ending in `jr $ra`, 1,104 leave
 *    the slot `nop` with a benign trivially-swappable predecessor - 881 of them
 *    a plain `addiu $sp,$sp,N` stack restore. No reorder-mode assembler leaves
 *    1,104 stack restores outside the slot. ASPSX 2.34 did not fill delay slots.
 *    So the origin space for `ctc2 $t7,$7` in that slot is now exhaustively
 *    enumerated: cc1's reorg (H1 - halts at any asm insn, 32/32 spellings plus
 *    ~164k permuter iterations), the assembler (measured never to fill), or a
 *    human. Only the third survives, and sessions 1-7 had merely ASSUMED the
 *    second away.
 *
 *  - F3: the cop2-tail population partitions by ORIGIN and the split is
 *    contiguous in address space. 17 such functions leave the slot nop - 16 of
 *    them inside the PsyQ libgte block 0x8007E1AC-0x8007F1A8, plus
 *    tslDmaDrawListDelAll (closed in pure C) - and our toolchain reproduces that
 *    shape exactly. FIVE hold the cop2 op IN the slot, and all five occupy one
 *    contiguous run of BB2's own code, 0x80052A80-0x80052BDC:
 *    game_2d_CheckLifeGaugeNoDisp, func_80052A88, func_80052B00, func_80052B44,
 *    func_80052B7C. func_80052B44 sits in the middle of that run and is already
 *    Judge-authorized canonical-asm; the other four each carry exactly one
 *    regfix rule, a `fill_delay` pulling the immediately preceding instruction -
 *    four of only six such rules tree-wide. A single authoring unit of
 *    hand-written GTE asm, with this function inside it.
 *
 *  - F4: m2c, run fresh, returns no C body at all - eight
 *    M2C_ERROR("unknown instruction: ctc2 ...") lines and all eight lw
 *    dropped as dead. F5: the asm-BOUNDARY axis (loads moved INSIDE the asm -
 *    the real PsyQ libgte macro shape, and the one thing no session 1-7 form
 *    ever varied) is KILLED: the honest sandbox strips such a block as cheat-asm
 *    (`lw` is its first template instruction), and standalone it emits
 *    lw $9,0($4) ... lw $2,28($4) - neither $t0..$t7 nor ascending, further from
 *    the target than this form's own predecessor. Banked at
 *    rejected/asm-side-loads-psyq-macro-shape-stripped-as-cheat.c. The
 *    decomp.me corpus lane was run: top similarity 0.048, pure noise.
 *
 * Floor re-verified independently with a fresh harness: 17 (build_insns 18,
 * target 17), flat for the fifth consecutive session. Full data: evidence.md
 * Session 8, hypotheses.md H19-H22, tmp/grind/func_80052B00/s8/.
 *
 * SESSION 9 (rederive, 2026-08-01) - form UNCHANGED, and the single largest
 * unexamined assumption underneath eight sessions of reasoning is now closed by
 * measurement. Sessions 1-8 built two impossibility proofs (H1 from
 * reorg.c:730-735 stop_search_p; H16 from reload1.c:3606 order_regs_for_reload)
 * and every line of source they read, and every one of the 32 hand spellings and
 * ~164k permuter iterations that corroborated them, came from
 * `tools/gcc-2.7.2` - the decompals/mips-gcc-2.7.2 open-source port, a
 * KMC-tailored fork. SLUS-00663 was not built with that compiler. It was built
 * with SN Systems cc1psx, GCC 2.7.2.SN.1. If the shipped compiler's reload
 * ordered hard registers differently, or its reorg admitted asm insns as
 * delay-slot candidates, then the target's bytes would be ordinary compiler
 * output from ordinary C and eight sessions would have been reasoning about the
 * wrong compiler. No session 1-8 ran cc1psx on this function; session 8 closed
 * the ASSEMBLER as a non-human origin (H20) but left the COMPILER identity
 * untouched.
 *
 *  - H23 KILLED. tools/cc1psx_wrapper.sh (the original PsyQ cc1psx.exe via
 *    dosemu2, available as a self-disproof tool per the 2026-05-29 user
 *    directive) was run against our fork under the identical canonical flag set
 *    on two structurally distinct chassis: the banked floor-17 fused-8 body and
 *    the session-1/2 eight-separate-__asm__ body. Both compilers emit
 *    INSTRUCTION-IDENTICAL streams; the only diffs are the banner, `.file
 *    "T.C"`, cc1psx's extra `__gnu_compiled_c:` label, and missing
 *    `.version`/`.type`/`.size`/`.ident`. cc1psx's own output is
 *    `lw $2,0($4) ... lw $10,28($4)` / `ctc2 $2,$0 ... ctc2 $10,$7` / bare
 *    `j $31` - the exact {$2,$3,$5,$6,$7,$8,$9,$10} allocation H16 predicts,
 *    and no delay-slot fill. On the eight-separate chassis the agreement extends
 *    even to the idiosyncratic deferred self-clobbering `lw $4,0($4)` scheduled
 *    last. The shipped compiler cannot produce the shipped function's registers
 *    from this C any more than our reference port can.
 *
 *  - H24 KILLED, with a differential rather than a null. Control ctlB is the
 *    fused-8 body plus one trailing plain C store (`matrix[0] = t7;`), so the
 *    last insn needing a delay slot is not an asm. BOTH compilers then emit
 *    `.set noreorder / .set nomacro / j $31 / sw $2,0($4) / .set macro /
 *    .set reorder` - the store IS pulled into the slot, with the identical
 *    protective wrapper. Same body, same flags, one non-asm trailing insn is the
 *    entire difference. reorg in the SHIPPED compiler is equally willing and
 *    equally blocked. H1 is a property of GCC 2.7.2 as a family, verified
 *    against the actual build compiler, not an artifact of the decompals port.
 *
 * NET EFFECT ON THIS DISPOSITION. The origin space for `ctc2 $t7,$7` in the
 * `jr $ra` delay slot is now exhaustively enumerated AND exhaustively empty of
 * non-human authors: (a) the reference compiler's reorg - halts at any asm insn
 * (H1); (b) the SHIPPED compiler's reorg - measured identical this session, and
 * measured to fill the same slot when the candidate is not an asm (H24); (c) the
 * assembler - ASPSX 2.34 measured never to fill delay slots at all, by
 * whole-binary census of 1,369 `jr $ra` tails (s8 H20); (d) a human. Only (d)
 * survives. The same enumeration now covers the register axis: H16 from
 * reload1.c source, confirmed identical under cc1psx. The argument for this form
 * no longer rests anywhere on "our toolchain is a faithful stand-in" - that
 * premise has been measured on this exact function.
 *
 * Floor re-verified with the banked pin-free fused-8 body spliced into
 * src/text1b.c: sandbox --disable all = 17 (target 17, build 18, rules_dropped
 * 1), then src restored; flat for the SIXTH consecutive session. Full data:
 * evidence.md Session 9, hypotheses.md H23/H24, tmp/grind/func_80052B00/s9/.
 *
 * SESSION 10 (synthesis, 2026-08-02) - form UNCHANGED, floor UNCHANGED at 17
 * for the seventh consecutive session, but this session BOTH strengthened the
 * argument for this disposition and CORRECTED a claim the ledger had been
 * leaning on since session 6. Read both halves; the correction matters.
 *
 *  - H25, the merged result the synthesis pass was for: the register residual
 *    and the delay-slot residual are NOT two independent defects. Both are
 *    consequences of OCCUPANCY - which call-used hard registers are unavailable
 *    when the allocator runs. Measured (s10/couple.sh, instrumented cc1 -da,
 *    reload's `;; Spilling reg` lists): every register occupied slides the
 *    eight-register allocation window up by exactly one; the target needs all
 *    five of $2,$3,$5,$6,$7 occupied to reach $t0..$t7; and the ONLY
 *    zero-instruction occupancy channel this compiler has is an insn absorbed by
 *    the otherwise-empty `jr $ra` delay slot (retB: `s32` + `return 0;` slides
 *    the window one for +0 insns, because its `move $2,$0` takes the slot). That
 *    channel has capacity ONE, five units are needed, and spending it forfeits
 *    the very slot the target needs for `ctc2 $t7,$7`. The 17-point residual is
 *    one problem whose two halves are mutually exclusive at zero cost.
 *
 *  - H26 CORRECTS s6/H15, which sessions 7-9 quoted as settled. The allocation
 *    pool is NOT the fixed set {2,3,5,6,7,8,9,10}; it is (call-used GPRs) MINUS
 *    (occupied), and s6's control simply occupied nothing. With $5,$6,$7
 *    occupied the pool tops out at $13; with $2,$3 occupied too it reaches $15.
 *    s10/pool.sh's `poolK` emits `ctc2 $8,$0 / ctc2 $9,$1 / ... / ctc2 $15,$7` -
 *    the target's register mapping EXACTLY, the first time in ten sessions - and
 *    its eight `lw` carry the target's (register, offset) pairs. So "the
 *    register set is unreachable" is retired from the ledger's vocabulary: it is
 *    reachable, and the open question is price and legality.
 *
 *  - H27: the price, as measured, is a cheat AND is score-negative. poolK's
 *    occupancy comes from empty-template `__asm__ ("" :: "r"(x))` blocks (the
 *    register-occupancy / scheduling-barrier family forbidden by
 *    .claude/rules/inline-asm-policy.md) plus three spare parameters and a
 *    `long long` return on a void function (the ABI break s7 already rejected).
 *    Honest sandbox, spliced into src/text1b.c and restored (s10/score.py):
 *    HEAD 18 / banked pin-free fused-8 17 / poolK 19. The sandbox strips the
 *    empty asms, the occupancy evaporates, and only the four `acc` instructions
 *    remain. Banked at
 *    rejected/empty-asm-occupancy-reaches-t0t7-honest-19.c.
 *
 * WHAT THIS MEANS FOR THIS FORM, stated honestly. The delay-slot half of the
 * residual remains closed outright - H1 (reorg.c:730-735 stop_search_p), H20
 * (ASPSX 2.34 never filled a delay slot, whole-binary census) and H24 (the
 * SHIPPED cc1psx behaves identically, and fills the same slot when the candidate
 * is not an asm) leave a human as the only possible author of `ctc2 $t7,$7` in
 * that slot, so no C source can drive the floor to 0. But the register half is
 * now an OPEN, precisely-stated question rather than a proof: is there a LEGAL C
 * construct that makes a value live ACROSS the fused asm without emitting an
 * instruction? Measured negatives: unused parameters do not occupy (s6
 * ctlF/ctlG); an `s32` return occupies $2 only and pays the delay slot; a
 * `long long` return costs +1..+4; volatile pointers and empty-template asms are
 * cheats. Until that question is enumerated, this form's supporting argument
 * should be stated as "the delay slot is impossible and the registers are
 * priced", not as "both are impossible". Full data: evidence.md Session 10,
 * hypotheses.md H25/H26/H27, tmp/grind/func_80052B00/s10/.
 *
 * SESSION 11 (escalation / DISPOSITION, 2026-08-02) — form UNCHANGED, honest
 * floor UNCHANGED at 17 for the eighth consecutive session. This session
 * answered s10's one open frontier question and then filed the disposition.
 *
 *  - H28/H29, THE ANSWER TO "IS THERE A ZERO-COST OCCUPANCY CONSTRUCT?": yes,
 *    and it is a cheat. Adding a clobber list `: "$2","$3","$5","$6","$7"` to
 *    the REAL fused ctc2 asm makes cc1 emit `lw $8,0($4) ... lw $15,28($4)` /
 *    `ctc2 $8,$0 ... ctc2 $15,$7` / `j $31` — the target's registers, order and
 *    offsets EXACTLY, in the SAME 17 instructions, at zero cost (s11/occ.sh).
 *    It is illegal because the template writes cop2 control registers and no
 *    GPR, so the clobber list is a false statement about the asm whose only
 *    effect is to steer reload1.c:3606 onto the shipped bytes' registers — the
 *    register-pin family (.claude/rules/inline-asm-policy.md), identical in
 *    intent to the HEAD body's eight `register asm("$N")` pins. Banked at
 *    rejected/false-clobber-list-occupancy-cheat-honest-sandbox-scores-2.c.
 *    ENGINE DETECTOR GAP, reported to the owner and NOT fixed here (engine/ is
 *    off-surface): the sandbox strips empty-template occupancy asms but not a
 *    false clobber list on a real template, so this cheat scored 2 against the
 *    banked floor form's 17 (s11/score.py). A floor drop of that shape is a
 *    cheat signature, not progress.
 *
 *  - H30, WHY THIS STRENGTHENS THIS FORM: with the target's own register
 *    allocation handed to the compiler for free, the ENTIRE residual is the
 *    delay slot — target `ctc2 $t6,$6 / jr $ra / ctc2 $t7,$7` vs the form's
 *    `ctc2 $14,$6 / ctc2 $15,$7 / j $31 / nop`, every other byte agreeing.
 *    Sessions 1-10 could only argue the delay-slot impossibility with the
 *    register residual muddying the measurement; s11 measured it in isolation.
 *
 *  - DISPOSITION FILED. Gate 1 (canonical-asm STRONG hand-coded signals) FAILS:
 *    `scan_hand_coded --single func_80052B00` = tier=LOW score=0/8. Gate 2 (an
 *    in-hand SOTN-master precedent) FAILS: only the in-PROJECT sibling
 *    precedent func_80052B44 (inline_asm_canonical.txt:340) exists, which the
 *    standing ruling classes as "same family". Both gates failing is the case
 *    the owner's 2026-07-27 standing ruling pre-decides, so the entry filed at
 *    docs/grind/decisions.md (2026-08-02, func_80052B00) is
 *    REFUSED / OWNER-ACCEPTED INCOMPLETE and the function is parked terminally
 *    — nothing pending on the owner. This file stays banked and ready to apply
 *    if that ruling is ever revisited; the entry carries the operator sequence
 *    and asks that any revisit cover the whole 0x80052A80-0x80052BDC block.
 *    Full data: evidence.md Session 11, hypotheses.md H28/H29/H30,
 *    tmp/grind/func_80052B00/s11/.
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
