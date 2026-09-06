/* MIGRATION BANNER (asm-until-matched, owner ruling 2026-08-19): func_800480C0 is
 * committed on main as INCLUDE_ASM("asm/funcs", func_800480C0) in src/text1b.c. This
 * file is a CANDIDATE, not the state of HEAD; every 'measured on main' statement in the
 * headers below means 'measured with this body installed over that INCLUDE_ASM line'.
 * Install with tmp/grind/func_800480C0/s3/install.py. */
/* s21 (rederive, 2026-09-05, chassis HEAD 16834fcf) - BODY UNCHANGED; floor re-measured 20
 * with this body installed over the INCLUDE_ASM line (74 target insns, 74 build insns,
 * rules_dropped 0, raw cc1 .frame $sp,56 # vars= 0, regs= 8/0, args= 24). The dispatch
 * brief's "measurement unavailable" was a driver-side gap, not a broken body.
 * s21 executed the rederive modality on all three legs and closed s20's frontier item 1.
 * (1) A FRESH m2c DECOMPILE, run with no ledger input, recovers THIS body's shape line for
 * line and declares no aggregate of any kind (tmp/grind/func_800480C0/s21/m2c_fresh.txt) -
 * there is no structurally different C shape for these bytes.
 * (2) DONATION CENSUS, CLOSED NEGATIVE: src/text1b.c defines ZERO static and ZERO inline
 * functions across 583 function definitions, and holds no 25..32-byte local aggregate shared
 * with this function, so an inline-callee donation carrier here would have to be a helper
 * invented for this one function with no second caller to make its 32-byte local genuine.
 * (3) BUT s20's KILL OF THE HELPER AXIS WAS TOO WIDE, AND THE CORRECTION IS THE SESSION'S
 * REAL FINDING. s20 measured only helpers ADDED to the body. A helper carrying computation
 * this body ALREADY performs is BYTE-NEUTRAL: `static __inline__ s32 sxadd(s16 v, s32 b)
 * { return (s32)v + b; }` substituted at all four sign-extend-and-add sites scores 20 at 74
 * insns with a .frame line identical to this one, and so does `rec_base` at both of its
 * sites. Giving either helper an unreferenced `u32 t[8]` then makes raw cc1 print the
 * TARGET'S EXACT `.frame $sp,88,$31 # vars= 32, regs= 8/0, args= 24, extra= 0` with the
 * 74-instruction stream untouched - the integrate.c:2085-2092 donation fires perfectly
 * through an honest, value-flowing, four-times-per-iteration call site.
 * (4) AND THAT IS WHAT CLOSES THE FAMILY. The unreferenced carrier is the frame pad at one
 * remove and the sandbox strips it (score stays 20). Making it referenced costs exactly one
 * instruction no fold can remove - `u32 t[8]; t[0] = (u32)(s32)v; return (s32)t[0] + b;`
 * builds 75 insns and scores 1, the whole delta being a single `sw $2,24($sp)` into the
 * target's untouched window, because all four inline sites write the same frame address and
 * the LAST write has nothing to kill it. THAT SCORE OF 1 IS A DETECTOR GAP, NOT A FLOOR, AND
 * IS DELIBERATELY NOT PROPOSED: an eight-word array holding one scalar fails cheat-checklist
 * T1/T2/T5 (dead-vars-local-array, whose carve-out requires the dead stores to appear in the
 * TARGET bytes) and materialises a byte the target does not contain. It is banked as
 * rejected/s21-referenced-helper-array-leaves-one-surviving-store.c so no later session
 * mistakes it for progress; the stripper keys on the array name being unreferenced and one
 * live element defeats it.
 * NET: the donation family is now closed on a MECHANISM rather than a census - a donated
 * block buys the 32 bytes byte-neutrally only if it carries zero traffic, and a frame object
 * carries zero traffic only if it is never written, which is the unwritten pad this function
 * is barred from until an owner row exists. Floor stays 20. */
/* s20 (rederive, 2026-09-05, chassis HEAD a4c735da) - BODY UNCHANGED, floor re-measured
 * 20 with this body installed over the INCLUDE_ASM line (74 target insns, 74 build insns,
 * rules_dropped 0; raw cc1 reads .frame $sp,56 - vars= 0, regs= 8/0, args= 24, 72 insns,
 * unalloc= 0). Mandated FAKE re-audit re-run on this chassis
 * (tmp/grind/func_800480C0/s20/fake_ablate.txt): one FAKE unit (arg0 = 0;), keep-all 20 /
 * drop-1 32 - load-bearing, masking no lever, identical on seven chassis now.
 * THE SESSION'S RESULT, and it is the largest single step this function has taken:
 * (1) A BYTE-EXACT BUILD EXISTS AND THE RESIDUAL IS PROVEN TO BE EXACTLY A 32-BYTE
 * get_frame_size() CHARGE AND NOTHING ELSE. s19's frontier item 1 (inline-callee frame
 * donation, integrate.c:2085-2092) was executed to a measurement instead of an argument.
 * Body tmp/grind/func_800480C0/s20/bodies/h1_stmt_tail.c - THIS candidate's body verbatim,
 * plus a `static __inline__ s32 pack_off(s32 v, s32 n)` carrying `u32 t[8]` written only
 * under `if (n != 0)`, invoked as the discarded statement `pack_off(arg1, 0);` after the
 * loop - prints `"score": 0`, target_insns 74, build_insns 74, rules_dropped 0 from
 * `sandbox func_800480C0 --disable all`, and its raw cc1 listing is LINE-FOR-LINE IDENTICAL
 * to this candidate's except `.frame $sp,88 # vars= 32` in place of `.frame $sp,56 # vars= 0`
 * (tmp/grind/func_800480C0/s20/last_h1_stmt_tail.s). Fifteen sessions of frame-decomposition
 * inference are now a measured fact: nothing but the 32 vars bytes stands between this C and
 * the shipped function.
 * (2) BUT EVERY BYTE-NEUTRAL PLACEMENT OF THAT DONATION IS A DEAD STATEMENT, i.e. the pad
 * relocated into a helper, and h1 is therefore banked in rejected/ and NOT proposed. Ten
 * placements were measured (tmp/grind/func_800480C0/s20/bodies/): every site where the
 * helper's value FLOWS into the program perturbs the stream - entry shift d3 and stack-param
 * sx_arg4 g8 score 6 (the two incoming-parameter loads hoist above the register saves and a
 * load-delay nop appears, build_insns 75); loop new_var e6 and d2 score 16 (the srl/sll/addu
 * for new_var hoists to the loop head); statement-call-at-TOP e1 clusters all eight saves;
 * register-param placements f3/g1/g6/g7 cost one insn (73). Only the two value-free sites -
 * the discarded call at the end of the function (h1) and at the end of the if-block (h2) -
 * leave the stream bit-identical. A discarded call to a side-effect-free helper fails
 * cheat-checklist T1 and T2 outright, so the donation family closes the same way the pad
 * family did: it reaches the bytes only by carrying an object nothing on this path needs.
 * (3) DETECTOR GAP, RECORDED FOR THE PIPELINE (not acted on - engine/ is denylisted): the
 * sandbox's find_unused_local_arrays only strips arrays never referenced in their own
 * function body, so `u32 t[8]` referenced inside the helper's dead `if (n != 0)` arm
 * survives stripping and the honest-floor scorer printed 0 for a relocated pad. Compare the
 * same helper with an UNREFERENCED `u32 t[8]` (d1): stripped, score 20. The honest floor for
 * this function is therefore still 20, not 0, and this session does not claim otherwise.
 * (4) ORDINARY-C RESULT WORTH KEEPING: m2c's own rederive shape - `s32 arg4, s32 arg5` with
 * `(s16)` casts at the sign-extend sites (s10) - is BYTE-NEUTRAL on this body (k1, listing
 * line-for-line identical to this candidate) and is REQUIRED under any donation: with s16
 * parameters the donated frame makes cc1 emit `lhu $3,104($sp)` / `lhu $4,108($sp)` where the
 * target emits `lw`, costing two further diffs. Kept out of this candidate only because the
 * body is verdict-keyed and unchanged bodies inherit their prior review state.
 * Artifacts: tmp/grind/func_800480C0/s20/{probe.sh,runall.sh,fake_ablate.txt,bodies/,last_*.s},
 * rejected/s20-*.c. */
/* s18 (forensics, 2026-09-05, chassis HEAD e25a492f) - BODY UNCHANGED, floor re-measured
 * 20 with this body installed over the INCLUDE_ASM line (74 target insns, 74 build insns).
 * Mandated FAKE re-audit re-run on this chassis: one FAKE unit (arg0 = 0;), keep-all 20 /
 * drop-1 32 - load-bearing, masking no lever, identical on five chassis now.
 * THE SESSION'S FINDING: the axis s17 reopened is closed, and the residual is back to a
 * single sentence. (1) THE TARGET FORMS NO $sp-RELATIVE ADDRESS AT ALL - all 25 mentions of
 * $sp in asm/funcs/func_800480C0.s are the two frame adjustments, the 8 saves at 0x38..0x54,
 * the 8 restores, the two INCOMING-parameter loads at 0x68/0x6C and the single
 * sw $v0,0x10($sp). s1 had only grepped sw/lw, so the base-register blind spot
 * ([[base-register-store-invisible-to-symbol-grep]]) was still open; it is now shut. Every
 * frame producer whose object is REFERENCED must emit an sp-relative memory reference or an
 * sp-relative address computation, and the target has neither in 0x18..0x37, so the object
 * charged for those 32 bytes is unreferenced in the emitted RTL.
 * (2) A LIVE CALL CANNOT SIZE THE OUTGOING-ARGS BLOCK PAST 24 WITHOUT WRITING INTO THE
 * WINDOW. The body's own in-loop call widened to 6/7/8/10 live arguments reads
 * args= 24/32/32/40 with outgoing-arg stores at {16,20} / {16,20,24} / {16,20,24,28} /
 * {16,20,24,28,32,36} - one store per word past the fourth, contiguous, emitted by
 * store_one_arg's emit_push_insn (calls.c:3135). The SIXTH word is free of the window (store
 * at 0x14) but raises args_size by nothing; the SEVENTH is the first that raises args_size
 * and it stores at 0x18. So args_size == 24 and var_size == 32 for every reachable-call
 * body, restoring s16's decomposition and bounding s17's reopening to dead-code spellings.
 * (3) THE ALIGNMENT-HOLE LOOPHOLE, WHICH NO SESSION HAD CONSIDERED, IS REAL BUT TOO SMALL: a
 * 6th long long argument leaves a genuine unstored HOLE at offset 20 (args= 32, stores at
 * 16,24,28), but the hole is at 0x14, below the window, and the long long's own words land
 * inside it; a second long long adds no hole (already aligned). Eight words of args therefore
 * carry at least four stores in 0x18..0x37 under any type mixture.
 * (4) extra_size is not a third term: mips.c:4462 makes it TARGET_ABICALLS-only and the
 * build is -mno-abicalls, so every probe reads extra= 0 and the frame identity is
 * var_size + args_size + gp_reg_rounded.
 * Instruments banked: s18/{probe.sh,bodies/,last_*.s,nrefs_census.txt,label_census.txt,
 * loop_movables.txt}. nrefs_census shows EVERY allocno seated (no hardreg=-1) - there is no
 * allocation residual, which is why s17's solver verdict was PRE-RA; loop_movables shows an
 * EMPTY movable table (loop insns 78..162, insn_count=34, loop_has_call=True, threshold=61)
 * so loop.c is not a lever; label_census confirms 2 labels and eight referenced callee-saved
 * registers, already reproduced by this body. */
/* s17 (solver, 2026-09-05, chassis HEAD eab57aaf) - BODY UNCHANGED, floor re-measured
 * 20 with this body installed over the INCLUDE_ASM line (74 target insns, 74 build
 * insns, rules_dropped 0; probe reads .frame sp,56 - vars= 0, regs= 8/0, args= 24,
 * 72 cc1 insns, unalloc= 0). Mandated FAKE re-audit re-run on this chassis: one FAKE
 * unit (arg0 = 0;), keep-all 20 / drop-1 32 - load-bearing, masking no lever.
 * SOLVER VERDICT (the modality's deliverable): inverse_compose.py classify REFUSES this
 * function (zero-rule guard - it would print a fictitious PRE-RA verdict) and names
 * goal_from_tgt.py classify as the object-level route. That returns FIRST DIVERGENCE:
 * PRE-RA, 'next tool: none - the residual is upstream of every model'. The only shapes
 * present in one stream are the frame constant and the sp displacements derived from it.
 * The RA and scheduler layers are foreclosed as the site of this residual; the solver
 * suite should not be re-run on this function.
 * THE SESSION'S FINDING, which reopens an axis sixteen sessions had closed by inference:
 * THE RESIDUAL IS NOT NECESSARILY var_size == 32. current_function_outgoing_args_size is
 * set by expand_call at RTL-generation time and never revised, so when jump.c later
 * deletes a call as unreachable the outgoing-args block it sized SURVIVES into
 * compute_frame_size (mips.c:4467) while its argument stores vanish with the block that
 * held them. An outgoing-args block can be allocated with no stores inside it. Measured:
 * a 14-argument call placed after this function's return - or jumped over by a forward
 * goto, or after a for (;;) {} - yields .frame sp,88 - vars= 0, regs= 8/0, args= 56 at
 * 72 cc1 insns, the candidate's own count, and sandbox --disable all prints score 0
 * (74/74, rules_dropped 0). The same call left REACHABLE costs 97 insns, which is the
 * effect s16 correctly predicted for a live call and wrongly generalised to the block.
 * s16's 'args_size is pinned to 24 by the single-store evidence' is therefore SUPERSEDED:
 * the single store pins live argument traffic, not the block's size, and args=24/var=32
 * and args=56/var=0 are byte-indistinguishable (mips.c:4547 places the saved-register
 * block at a sum, and the shipped body's one non-save sp reference, sw v0,0x10(sp), lies
 * in the first 24 bytes under either reading).
 * THOSE FORMS ARE NOT PROPOSED AND ARE NOT THIS CANDIDATE. A call to a 14-parameter extern
 * that does not exist in the game, sitting in code that can never execute, fails
 * cheat-checklist T1, T2 and T5 (first reach of a dead-code-to-inflate-outgoing-args
 * family). Both spellings are banked in rejected/s17-BYTES-0-BUT-CHEAT-*.c. What they buy
 * the next session is a price signal: every var_size=32 producer costs instructions unless
 * it is the banned pad, whereas the args_size=56 route costs ZERO. The open question is
 * narrow - is there an ordinary-C construct that raises current_function_outgoing_args_size
 * to 56 without leaving a live call?
 * ALSO CLOSED THIS SESSION: frontier item 2 (orphan multiplicity) - copy chains of depth
 * 1..4 all give exactly one orphan, a second folded guard on the loop-exit test adds none,
 * and two/three/four NESTED folded guards give vars= 0, destroying the single orphan
 * rather than doubling it. Frontier item 1 (alter_reg alignment) - reload1.c:2404 passes
 * align == -1, function.c:687 rounds to BIGGEST_ALIGNMENT, mips.h:1082 sets that to 64,
 * so DImode buys the same 8 bytes as SImode; func_80042874's six phantoms in vars= 48
 * confirm 8 bytes each. Frontier item 3 (a post-cc1 stage writing 0x58) - all three
 * prologue_fix config files are empty and every rewriter is gated on the function being
 * named in one of them.
 */
/* s16 (synthesis, 2026-09-05, chassis HEAD a5ebaa6f) - BODY UNCHANGED. Floor
 * re-measured 20 with this body installed over the INCLUDE_ASM line (74 target
 * insns, 74 build insns, rules_dropped 0). Mandated kill re-audit re-run on the
 * current chassis: candidate keep-all 20 / drop-1 32, s14 i9 20 / 32, s14 i8
 * 13 / 26 - the single FAKE unit is load-bearing in all three and masks no lever
 * (tmp/grind/func_800480C0/s16/fake_ablate_reaudit.txt).
 * THE SESSION'S STRUCTURAL RESULT: the residual is NOT get_frame_size equal to
 * 32. Fifteen sessions read the shipped frame as vars 32 plus args 24, but
 * mips.c:4557 places the callee-saved block at args_size + extra_size +
 * var_size + gp_reg_size - 4 and mips.c:4475 totals the frame as var_size +
 * args_size + gp_reg_rounded, so the shipped listing constrains only the SUM
 * var_size + args_size == 56. Measured end to end: body e4, whose single call
 * carries 14 arguments, reads .frame sp,88 - vars= 0, regs= 8/0, args= 56 -
 * the target's exact frame total, exact register count and exact save offsets
 * 0x38..0x54, with ZERO frame vars.
 * THE DECOMPOSITION IS NEVERTHELESS RESOLVED, BY MEASUREMENT RATHER THAN BY
 * ASSUMPTION: every argument word past the fourth is stored, contiguously, into
 * offsets 16..args_size-1 (e2 args 32 stores at 16,20,24; e3 args 40 stores at
 * 16..36; e4 args 56 stores at 16..52). An args_size of 32/40/48/56 therefore
 * puts 2/4/6/8 stores inside the target's untouched 0x18-0x37 window, and the
 * target has exactly one sp-relative store outside its register saves. So the
 * target really is args 24 plus vars 32 - now a measured fact. e4 scores 22
 * against the candidate's 20 despite a byte-exact frame, and a fabricated
 * argument list on a five-parameter callee fails cheat-checklist T1 and T2
 * anyway, so the outgoing-args axis is dead twice over.
 * TWO CENSUS ENTRIES CLOSED BY MEASUREMENT, LEAVING NONE UNMEASURED:
 *   caller-save.c:315 (s15 frontier 3) - five pressure levels f7..f11 (7 to 11
 *     extra call-crossing values) give vars 40/48/56/64/72 tracking unalloc
 *     5/6/7/8/9 one for one, every slot 8-byte aligned and touched twice. No
 *     4-byte caller-save slot appears at any level: global alloc spills instead
 *     of caller-saving once the callee-saved file is exhausted.
 *   function.c:3605 and 3888 assign_parms - g1..g4 (an unused 32-byte struct
 *     parameter in seventh position, an unused s32 seventh parameter, the same
 *     struct in FIRST position, and the struct read once) all read vars= 0.
 *     Parameter homes live in the CALLER's frame under REG_PARM_STACK_SPACE,
 *     so assign_parms cannot charge this frame at all.
 * s15 FRONTIER 2 CLOSED WITH A CITE: mips.h:1651 defines STARTING_FRAME_OFFSET
 * as current_function_outgoing_args_size, so frame objects are allocated ABOVE
 * the outgoing-args block and no frame object can ever be placed below it. The
 * i8 body's surviving store at offset 24 could not have been made to coincide
 * with the target's store at offset 16.
 * WHERE THIS LEAVES THE ATTACK: var_size + args_size must be 56; args_size is
 * pinned to 24 by the single-store evidence; therefore var_size is 32, and
 * alter_reg slots are 8-byte aligned (reload1.c:2382-2385), so an allocation
 * route needs FOUR zero-traffic slots. This body caps at one across roughly 85
 * measured spellings, the mult-free tree maximum is three (s8 census over 1096
 * functions), and four is attested nowhere without a mult in the stream. The
 * only zero-traffic producer that reaches 32 in one step remains a declared
 * unreferenced 32-byte local - the pad, banned here and granted to three
 * siblings.
 */
/* s15 (synthesis, 2026-09-05, chassis HEAD e18d7715) - BODY UNCHANGED, floor
 * re-measured 20 with this body installed over the INCLUDE_ASM line (74/74,
 * rules_dropped 0; probe reads .frame sp,56 - vars= 0, regs= 8/0, args= 24,
 * 72 insns, unalloc=0). src/text1b.c restored from HEAD after every install.
 * MANDATED KILL RE-AUDIT run on the candidate AND on the two closest-to-target
 * instance kills (s14 i8 and i9): one FAKE unit each, load-bearing in each,
 * keep-all/drop-1 = 20/32, 20/32 and 13/26 respectively
 * (tmp/grind/func_800480C0/s15/fake_ablate_i8_i9.txt). NOTE THE 13: the s14 i8
 * donation helper whose 32-byte local is actually WRITTEN is not stripped by
 * the sandbox and scores 13 at 75 build insns - the lowest number ever measured
 * on this function. It is NOT a floor and NOT proposed: an eight-word array
 * holding one scalar intermediate fails cheat-checklist T1 and T2 and is the
 * dead-vars-local-array family. It is the tightest measurement of the price of
 * the residual - a correct frame is worth 20 points, one surviving store inside
 * the untouched window costs 13.
 * THE SESSION'S STRUCTURAL RESULT: a COMPLETE source-level census of every
 * producer of frame vars in this compiler (grep of assign_stack_local and
 * assign_stack_temp over tools/gcc-2.7.2). Eight sites exist, no ninth:
 * expand_decl of a local aggregate (stmt.c:3412), put_var_into_stack
 * (function.c:1347), assign_parms homes (function.c:3605 and 3888), expand-time
 * BLKmode temps (calls.c and expr.c), inline-callee donation (integrate.c:2092),
 * alter_reg (reload1.c:2404), the caller-save area (caller-save.c:315), and a
 * group unreachable in ordinary C (trampolines, setjmp, inline-asm operands,
 * builtin_apply). EXACTLY ONE of them is zero-traffic at arbitrary size, and it
 * is the frame-pad family. Full partition in hypotheses.md s15.
 * FIVE HYPOTHESES CLOSED, ALL MEASURED (bodies in
 * tmp/grind/func_800480C0/s15/bodies, bodies2, bodies3; banked as rejected/s15-):
 *   a3 address-taken parameter in the inline helper - donates vars= 16 but adds
 *      exactly one insn, sw 4,24(sp), a store INTO the target's untouched
 *      window. a4 eight address-taken scalars - reaches vars= 32 at 80 insns.
 *      s14 frontier item 2 closed negative.
 *   a1 static WITHOUT the inline keyword - not auto-inlined at -O2, a real call
 *      is emitted (75 insns, regs= 10). a2 (28-byte local) reads vars= 32, so
 *      the donation is ROUNDED UP to 8 bytes, not exact. a6 (16-byte local)
 *      reads vars= 16 at 72 insns - proportional and byte-neutral at any size,
 *      but always with a dead object. s14 frontier item 3 closed.
 *   c1-c4 the BLKmode conditional (cond ? structA : structB) - the one front-end
 *      site that would charge the frame at PARSE time for an expression that is
 *      never evaluated - is DEAD CODE: c-typeck.c:3513 wraps it in if-zero. All
 *      four bodies read vars= 0. CLASS kill with a cite.
 *   d1 twelve values live across the in-loop call - vars= 32 with FOUR
 *      alter_reg slots, correcting the ledger's phantom-ceiling-of-one claim:
 *      the ceiling of one bounds the ZERO-TRAFFIC regime only. All four slots
 *      (offsets 24, 32, 40, 48) carry a store and a load, at 108 insns.
 *   a5 an inline helper declared but never called is completely inert, which
 *      with integrate.c:2092 charging every expansion retires s14 frontier
 *      item 1 (the shared helper live at another call site would move the
 *      COMPLETED-C siblings' frames too).
 * WHERE THIS LEAVES THE ATTACK: the residual is one quantity, get_frame_size
 * equal to 32, and the only zero-traffic producer with unbounded multiplicity
 * is a declared unreferenced aggregate - the pad, banned here and granted to
 * three siblings. The only open zero-traffic alternative is the combine-orphan
 * phantom, capped at one on this body and at three tree-wide. Frontier for s16
 * is written out in hypotheses.md.
 */
/* s14 (synthesis, 2026-09-05, chassis HEAD 298b7f40) - BODY UNCHANGED, floor
 * re-measured 20 with this body installed over the INCLUDE_ASM line (72 insns,
 * .frame $sp,56 - vars= 0, regs= 8/0, args= 24, unalloc=0); src/text1b.c
 * restored from HEAD afterwards. Mandated FAKE re-audit re-run on this chassis
 * (tmp/grind/func_800480C0/s14/fake_ablate.txt): one FAKE unit (arg0 = 0;),
 * keep-all 20 / drop-1 32 - load-bearing, masking no lever.
 * THE SESSION'S FINDING, which reframes the whole residual: a THIRD producer of
 * frame vars exists besides the two phantom classes, and it hits the target
 * frame exactly. GCC 2.7.2 integrate.c:2085-2092 allocates
 * assign_stack_temp (BLKmode, DECL_FRAME_SIZE (inlinee), 1) in the CALLER for
 * every inline expansion, and DECL_FRAME_SIZE is snapshotted pre-optimisation
 * at integrate.c:345, so a static __inline__ helper carrying a 32-byte local
 * donates 32 bytes of vars here whether or not anything survives to touch them.
 * Measured: .frame $sp,88 - vars= 32, regs= 8/0, args= 24, i.e. the target
 * decomposition, at the candidate's own 72 insns, and the full scorer prints
 * score 0 for the i11 spelling. THAT SPELLING IS NOT PROPOSED: its array is
 * written inside a never-taken if, which is the banned dead-conditional-store
 * family and a relocated frame pad. All four donation spellings are banked in
 * rejected/s14-inline-*. The honest sub-question is now narrow and is written
 * up as frontier item 1 in hypotheses.md.
 * ALSO KILLED THIS SESSION: split shifts written across separate STATEMENTS
 * (four forms, all codegen-transparent at 72 insns - s6 had only killed the
 * one-expression spelling) and cross-block named step constants (six extra
 * insns, regs=10, vars=0, because combine has no LOG_LINK across basic blocks).
 * INSTRUMENT CORRECTION: the probe's unalloc column counts hardreg=-1 pseudos,
 * which INCLUDES reg_equiv_constant pseudos that pay zero frame bytes
 * (reload1.c:2382-2385). Only the vars term of the .frame comment is ground
 * truth. LEDGER RULE, still in force: never write a comment terminator inside
 * candidate.c prose.
 */
/* s13 (structural, 2026-09-05, chassis HEAD 3c8d48d6) - BODY UNCHANGED. ONE
 * REAL DEFECT FIXED IN THIS HEADER: the s12 line describing the split-cursor
 * form wrote the two pointer types as u32-star-slash-u16-star, which embeds a
 * C comment terminator and closed this comment block early. install.py splices
 * this file VERBATIM over the INCLUDE_ASM line, so the candidate has been
 * UNCOMPILABLE since the s12 ledger commit: cc1 reported
 * `src/text1b.c:145: parse error before '='` and emitted no func_800480C0, and
 * the sandbox answered `scorable: false - func_800480C0 not found in
 * text1b.o`. That is why the s13 dispatch CHASSIS CHECK read 'measurement
 * unavailable'. With the text fixed the floor re-measures 20 (74/74,
 * rules_dropped 0) and the probe reads .frame $sp,56 - vars= 0, regs= 8/0,
 * args= 24, 72 insns, unalloc=0. LEDGER RULE: never write a comment
 * terminator inside candidate.c prose.
 * MANDATED FAKE RE-AUDIT on this chassis (tmp/grind/func_800480C0/s13/
 * fake_ablate.txt): one FAKE unit (arg0 = 0;), keep-all 20 / drop-1 32 -
 * load-bearing, masking no lever.
 * SEVEN NEW FORMS (tmp/grind/func_800480C0/s13/{runall.txt,rung.txt,runr.txt},
 * banked in rejected/ as s13-*):
 *   i1/i2/i3 static __inline__ helper chassis (4 / 2 / 6 integrate.c
 *     expansions of sx_sum and scaled) - ALL codegen-transparent: vars=0,
 *     regs=8, 72 insns, unalloc=0. A producer class never previously tried on
 *     this body, and it is dead.
 *   i5 inline pre_dec() guard expanded at BOTH branch sites - vars=8, regs=8,
 *     73 insns, unalloc=1. Slightly better than s5's two-branch guard, which
 *     needed a NINTH callee-saved register for the same 8 bytes, but still one
 *     phantom of the four the target needs.
 *   g2 distinct constants on the two branches - vars=0, regs=9, unalloc=0.
 *   g3 shifted entry guard (count-2 != -2) - vars=0, regs=8, 71 insns.
 *   r1 redundant equal-valued second entry guard - cse2/jump deletes compare
 *     AND constant together: vars=8, unalloc=1.
 *   r2 redundant shifted second entry guard - three extra emitted insns (76)
 *     for the same single phantom.
 * THE CLASS-B PHANTOM IS NOW NAMED AT RTL LEVEL (s13/dumps/i5.flow insns
 * 64-65, i5.lreg): it is a compare CONSTANT pseudo,
 * (set (reg:SI 93) (const_int -1)) feeding a branch_equality, printed
 * 'used 2 times across 2 insns in block 0; dies in 0 places; ST_REGS or none'.
 * A second one needs another entry-block compare against a non-zero constant
 * with a degenerate live range; on this body g2 measures that the backedge's
 * constant is loop-invariant and gets a hard register instead.
 * Instruments: tmp/grind/func_800480C0/s13/{probe.sh,dump.sh,runall.sh,
 * rung.sh,runr.sh,bodies/,dumps/}.
 */
/* s12 (structural, 2026-09-05, chassis HEAD 2dba1d48) - BODY UNCHANGED, floor
 * re-measured 20 (74/74, rules_dropped 0) with this body installed over the
 * INCLUDE_ASM line; src/text1b.c restored from HEAD afterwards. Mandated FAKE
 * re-audit re-run on this chassis (tools/fake_ablate.py,
 * tmp/grind/func_800480C0/s12/fake_ablate.txt): one FAKE unit (`arg0 = 0;`),
 * keep-all 20 / drop-1 32 - still load-bearing, still masking no lever.
 *  FOURTEEN NEW FORMS, ALL vars=0 / unalloc=0 (tmp/grind/func_800480C0/s12/
 *  {runall.txt,rundiag.txt}; bodies banked in rejected/ as s12-*):
 *    b1 new_var declared in the loop block      b2 all locals at function scope
 *    b3 for(;;) + inline break (67-72 insns)    b4 pre-test `while ((count--)!=0)`
 *       (67 insns - deletes the guard the target ships)
 *    b5 split u32-ptr / u16-ptr cursors (regs=9, frame 64)
 *    b6 integer offset cursor instead of a pointer
 *    b7 new_var computed first in the body (statement re-association)
 *    b8 unsigned loop counter (74 insns but regs=9, frame 64)
 *    b9 sign-extends taken inside the loop      b10 half hoisted / half in-loop
 *    b11 extra nested block around the body     b12 swapped sum operand order
 *    d1/d2 HImode accumulate (see below).
 *  THE DECISIVE NEW RESULT - PRODUCER 2's STATED PRECONDITION IS SATISFIED AND
 *  STILL DOES NOT ORPHAN HERE. .claude/rules/phantom-slot-frame-lever.md:47-56
 *  says the combine orphan-USE fires when the `reg:HI` being widened has a
 *  SECOND use AS AN HIMODE VALUE (func_8007CE0C r8, tslLineG5Init pseudo 92);
 *  an s32-only consumer "takes the widened value and the site folds clean".
 *  d1 gives all four narrow fields exactly that second HImode use
 *  (`a1v = (s16)(a1v + arg2);` then `(s32)a1v`), d2 gives it to one field.
 *  Both measure unalloc=0: d1 66 insns, d2 71 insns - combine re-forms the
 *  loads as `lh` and folds the whole chain instead of stranding a note. So on
 *  THIS body the class-A recipe fails even when its precondition is met, not
 *  merely because s7/s9 found the precondition absent.
 *  INSTRUMENT CONTROL (tmp/grind/func_800480C0/s12/runctrl.txt): re-running
 *  rejected/phantom-guard-vars8-ceiling.c on this chassis still reads
 *  vars=8 / unalloc=1, so the fourteen zeros are real negatives and not a dead
 *  probe. Frame algebra re-confirmed: target 88 = args 24 + vars 32 + regs 32;
 *  only nine callee-saved registers exist, so no regs= value can absorb the 32.
 *  Instruments: tmp/grind/func_800480C0/s12/{gen.py,probe.sh,runall.sh,
 *  rundiag.sh,runctrl.sh,bodies/,diag/,ctrl/}. */
/* s11 (escalation, 2026-09-02, chassis HEAD 37f9ecdb) - BODY UNCHANGED. Floor
 * re-measured 20 (74/74, rules_dropped 0) with this body installed over the
 * INCLUDE_ASM line; src/text1b.c restored to HEAD afterwards. Mandated FAKE
 * re-audit re-run on this chassis (tools/fake_ablate.py): one FAKE unit
 * (`arg0 = 0;`), keep-all 20 / drop-1 32 - load-bearing, masking no lever.
 * DISPOSITION FILED: docs/grind/decisions.md:20353 - RESOLVED BY STANDING
 * RULING (2026-07-27): FORECLOSED. Gate (a) canonical-asm FAILS
 * (scan_hand_coded tier=LOW 1/8, only S4 fires, no STRONG signal). Gate (b)
 * SOTN precedent for the volatile-leading-pad CONSTRUCT is in hand
 * (docs/reference/sotn-construct-index.md:103 -> src/st/sel/stream.c:80
 * `volatile u32 pad[4]; // FAKE`; :84 -> e_background_bushes_trees.h:160
 * `volatile char pad[8]; //! FAKE`) but does NOT open the family: BB2's
 * carve-out is a CLOSED per-function enumeration, owner-only to extend
 * (.claude/rules/no-new-park-categories.md:340-342,
 * engine/volatile_cheats.py:744). This body is therefore the final banked form
 * and is UNBLOCKED the moment an owner ruling adds the
 * `("pre_pad", 8)` row - the bytes are already proven (SHA1 match,
 * tmp/grind/func_800480C0/s1/build.log). Do NOT re-file this as an INTEGRATION
 * HANDOFF (Judge FAIL, decisions.md:20349). */
/* s10 (rederive, 2026-09-02, chassis HEAD a0198d09) - BODY UNCHANGED, floor
 * re-measured 20 (74/74, `sandbox func_800480C0 --disable all`). The mandated
 * rederive axes were all three run to the end and they CONVERGE on this body:
 *  1. FRESH m2c. `tools/m2c/m2c.py --target mipsel-gcc-c --valid-syntax` on
 *     asm/funcs/func_800480C0.s produces the same control flow this body has
 *     (the only deltas are m2c's s32-typed arg4/arg5 and a post- rather than
 *     pre-decrement loop test). Spelled out as a chassis it emits 71 insns with
 *     vars=0: dropping the second base carrier costs the `move $18,$16` the
 *     target ships, so the m2c shape is one insn SHORT, not a new lever.
 *  2. SIBLING TRANSPLANT - THE DECISIVE RESULT. func_80047FBC (src/text1b.c:82)
 *     is this same routine with 4 parameters instead of 6, and it is COMPLETED-C
 *     on main. Its body is line-for-line the body below (same `base_addr`/`p`
 *     init chain, same annotated `arg0 = 0;` dead param store, same in-loop
 *     cursor arithmetic, same `while ((count--) != 0)` tail). The ONLY
 *     structural difference between the accepted sibling and this candidate is
 *     its leading `volatile u32 pre_pad[8];`, granted by the 2026-08-20 owner
 *     ruling at engine/volatile_cheats.py:757-758. func_80047EE8 (src/text1b.c:35)
 *     and func_800481E8 carry the identical grant. So the rederive axis does not
 *     have a different shape to find: the shape is already on main three times.
 *  3. STRUCTURAL RESPELLINGS - 10 new forms measured (55 total on this body),
 *     every one vars=0 / unalloc=0:
 *       b1 fresh-m2c chassis (71 insns)      b2 u8* byte-cursor chassis (72)
 *       b3 `long long` loop counter (86, regs=10 - a DImode pseudo that IS
 *          allocated pays REGISTERS, not vars)
 *       b4 `unsigned long long` scale intermediate with a provably dead high
 *          word (76 - cc1 lowers it to SImode and orphans nothing)
 *       b5 12-byte struct-record chassis (63) b6 for-index chassis (62)
 *       b7 sub-word read of the two STACK-passed s16 params (68)
 *       b8 sub-word read of all four s16 params (68)
 *       b9 s16 round-trip temporaries, stream preserved (72)
 *  TWO LIVE FRONTIER ITEMS CLOSED. (a) s9 frontier 1 (DImode route to four
 *  phantoms) is dead: b3/b4 are the two ways a long long can enter this body and
 *  neither leaves an unallocated DImode pseudo. (b) s9 frontier 3 (the two
 *  stack-passed s16 args as a class-A site) is dead in the useful direction: the
 *  substitution DOES fire there - b7 drops 72 -> 68 insns because both
 *  `lw + sll + sra` triples fold into `lh` - but it produces unalloc=0 and it
 *  DELETES four instructions the target ships. Class A remains a byte-cost, not
 *  a phantom source, on this body.
 *  MANDATED FAKE RE-AUDIT (rejected/s10-candidate-minus-fake-reseats-registers.c):
 *  with the annotated `arg0 = 0;` physically removed the body still emits 72
 *  insns with vars=0/unalloc=0, but the register seating changes (base carrier
 *  $22 instead of $18/$s2, second base use binds $4) and the prologue reorders.
 *  The FAKE is load-bearing for the stream and masks no phantom lever - s8's
 *  ablation verdict re-confirmed on the current chassis.
 *  Instruments: tmp/grind/func_800480C0/s10/{probe.sh,runall.sh,runall2.sh,
 *  gen_bodies.py,gen_bodies2.py,bank.py,bodies/,bodies2/,bodies3/}. */
/* s9 (forensics, 2026-09-02, chassis HEAD 0c7f30e4) - BODY UNCHANGED, floor
 * re-measured 20 (74/74). Both s8 frontier items closed NEGATIVE, and the
 * class-A producer is now fully named:
 *  1. THE FAMILY HAS EXACTLY FOUR MEMBERS. grep for callers of func_800482C8
 *     over asm/funcs/*.s + src/*.c returns only the four known siblings; there
 *     is no fifth batch loader anywhere in the binary, so no live sibling can
 *     name the 32-byte object.
 *  2. func_80041AC8 IS NOT A THIRD PRODUCER CLASS. Its three orphan pseudos
 *     (115/105/85, .cse insns 171/106/26) and SetDrawEnv's three (140/137/128,
 *     .cse insns 217/209/166) are defined by the IDENTICAL RTL pattern
 *     `(set (reg:SI P) (ashift:SI (subreg:SI (reg:HI Q) 0) (const_int 16)))`
 *     followed by an `ashiftrt ... 16` carrying REG_EQUAL (sign_extend (reg:HI Q)).
 *     One class-A producer in this tree, not two.
 *  3. ITS PRECONDITION IS BYTE-VISIBLE AND ABSENT HERE. combine deletes that
 *     ashift only by substituting a MEMORY equivalent for the HImode pseudo and
 *     re-forming the extension as a signed narrow load - func_80041AC8 pays for
 *     each orphan with an emitted `lh $2,0($16)`, SetDrawEnv with `lh $5,22($sp)` /
 *     `lh $2,D_8009BE78`. The substitution is gated by can_combine_p's
 *     use_crosses_set_p (tools/gcc-2.7.2/combine.c:917). func_800480C0's shipped
 *     stream has ZERO `lh`, and each of its four `lhu $aN,0x0($s0)` loads carries
 *     an `addiu $s0,$s0,0x2` on the base register between the load and its
 *     sll/sra pair - which is precisely why those pairs survive into the target
 *     bytes. Class A is closed here on a cited predicate, not by analogy.
 *  Four new spellings measured (45 total on this body), all vars=0/unalloc=0/
 *  orphanUSE=0: immediate s32 sign-extends of the u16 loads, and three
 *  combine-foldable chain extensions of the arg1 scale (chain length is not the
 *  lever - shift merges rewrite i3 in place and orphan nothing).
 *  Instruments: tmp/grind/func_800480C0/s9/{dump.sh,dump2.sh,run.sh,bodies/,dumps/}.
 *  Full record: evidence.md / hypotheses.md s9. */
/* s8 (forensics, 2026-09-02, chassis HEAD 28583e8e) - BODY UNCHANGED, floor
 * re-measured 20 (74/74). Mandated FAKE re-audit: tools/fake_ablate.py finds one
 * FAKE unit (the annotated `arg0 = 0;`) and scores keep-all 20 / drop-1 32 - the
 * store is load-bearing and masks no lever. Three forensic results: (1) every
 * caller of the four text1b siblings passes scalars and func_800482C8 owns its
 * own locals, so the 32 bytes are not a caller-built record; (2) all four
 * siblings' shipped frames reserve the IDENTICAL untouched window 0x18-0x37 with
 * args=0x18 and callee-saved base 0x38 despite 2/4/6/2 parameters - the signature
 * of one shared source declaration, not of shape-dependent allocation residue;
 * (3) a filter-free census of all 1096 functions this tree compiles finds the
 * mult-free phantom ceiling is THREE (SetDrawEnv, SetDrawEnv2, func_80041AC8),
 * with only the two mult/div bodies at 6 - four phantoms on a mult-free body is
 * attested nowhere. Two further spellings measured inert (unsigned /4*4 for the
 * power-of-two scales is codegen-identical: GCC folds it at expand). Full record:
 * evidence.md / hypotheses.md s8. */
/* s7 (solver, 2026-09-02, chassis HEAD 7e18adc2) - BODY UNCHANGED, floor re-measured
 * 20 (74/74). goal_from_tgt.py classify returns PRE-RA, 'next tool: none - the residual
 * is upstream of every model': registers and instruction order already match exactly,
 * so neither ra_solver nor sched_solver has any purchase here. Three further class-A
 * spellings measured negative (s16* direct loads, four u16* copy temps, five copy
 * temps). The target's own narrow loads are lhu+sll+sra, NOT lh, so the HImode
 * sign-extend fold that makes class-A orphans elsewhere in this tree cannot fire on
 * this body without changing bytes. Full record: hypotheses.md / evidence.md s7. */
/* func_800480C0 - s3 (structural, 2026-09-02) BEST POLICY-LEGAL FORM.
 * sandbox --disable all = 20 (74/74 insns); .frame sp,56 (vars=0, regs=8,
 * args=24) vs target .frame sp,88 (vars=32, regs=8, args=24).
 *
 * TWO s2 LEDGER CORRECTIONS, both measured this session:
 *
 * 1. THE HONEST FLOOR IS 20, NOT 32. engine/volatile_cheats.py's
 *    find_dead_param_assigns strips an UN-ANNOTATED `arg0 = 0;` out of the
 *    scored TU (engine/test_engine.py:1094-1116 pins exactly this: an
 *    annotated dead param assign is bypassed, an un-annotated one is
 *    flagged/stripped). Every s1/s2 body carried the store WITHOUT a
 *    FAKE annotation comment, so the sandbox scored a body with the
 *    store deleted. Adding the annotation - which the dead-store family
 *    REQUIRES anyway (.claude/rules/dead-store-fake-exception.md) - moves the
 *    score 32 -> 20 with no other change (measured s3).
 *
 * 2. THERE IS NO CALLEE-SAVED SEAT ROTATION. s2 recorded base_addr in s6
 *    against the target's s2 and built a whole frontier (ra_solver /
 *    sched_solver) on it. That rotation only exists in the STRIPPED sandbox
 *    build, i.e. it is the downstream effect of correction 1. With the store
 *    honoured, cc1 emits `s2 = s0` (s2 = s0) and seats
 *    sx_arg2..5 in s6/s5/s4/s3 - identical to the target
 *    (tmp/grind/func_800480C0/s3/probe.s, BB2_ALLOC_DEBUG order in
 *    tmp/grind/func_800480C0/s3/alloc.err: pseudo 83 -> hardreg 18).
 *
 * RESIDUAL: purely the frame size. The instruction STREAM is byte-identical
 * to the split target listing for func_800480C0; the 20 differing insns are the 20 sp-relative
 * operands (1 addiu sp, 8 sw, 2 incoming stack-arg lw, 8 lw, 1 addiu sp).
 * The target reserves 32 bytes of `vars` at sp+0x18..0x37 that no instruction
 * touches. args+vars = 0x38 is all the binary fixes, so (args=24, vars=32) and
 * (args=56, vars=0) are indistinguishable layouts - but args=56 needs a
 * 14-word call, i.e. the fabricated-dead-call-site family REFUSED by owner
 * ruling 2026-08-17 (.claude/rules/no-new-park-categories.md:357-366).
 *
 * The pad remains banned here (Judge 2026-09-02 04:28,
 * docs/grind/decisions.md:20349); the bytes-proven pad body is preserved at
 * rejected/pad-judge-banned-2026-09-02.c. */
/* s5 (synthesis, 2026-09-02) - BODY UNCHANGED, floor re-measured 20 (74/74).
 * Two ledger corrections this session, both tree-wide measurements:
 *  1. ONE UNALLOCATED PSEUDO == 8 BYTES OF `vars`, NOT 4. s3 modelled the
 *     ST_REGS compare residue as a 4-byte alter_reg slot rounded up to 8.
 *     Correlating cc1's `# vars=` with the instrumented cc1's BB2_ALLOC_DEBUG
 *     hardreg=-1 count across all 32 TUs gives `vars = 8 * phantoms` exactly
 *     (func_80042874 6->48, func_80041E10 3->24, get_cs 2->16, ~30 at 1->8).
 *     So the target's 32 untouched bytes are FOUR phantoms, and every spelling
 *     in the 30-form ceiling supplies ONE of them.
 *  2. THE PAD IS NOT THE ONLY PRODUCER OF UNTOUCHED `vars`. 54 ordinary-C
 *     COMPLETED-C bodies on main reserve untouched frame bytes with no pad
 *     aggregate and no spill, and eight reach 16 bytes (two phantoms) -
 *     get_cs/get_ce (src/display.c:556), func_8003FECC, func_80038170,
 *     func_80040594, SsSeqCalledTbyT, _SsSeqPlay, SpuSetCommonAttr. Same
 *     ST_REGS residue class (display.lreg: Registers 85 and 99, blocks 1 and 6).
 *     What is missing here is multiplicity, not mechanism.
 *  Bound: among the 45 census candidates with no mult/div, untouched `vars`
 *  takes only {8, 16, 32} and every 32 is a sanctioned-pad function whose
 *  phantom count is ZERO (the 32 bytes are the declared array). Four
 *  folded-compare residues on a mult-free body has no precedent in this tree.
 *  Instruments: tmp/grind/func_800480C0/s5/{census.sh,census2.py,census3.py,
 *  alloc_tu.sh,dump2.sh}. */
/* s6 (synthesis, 2026-09-02, chassis HEAD ba593529) - BODY UNCHANGED, floor
 * re-measured 20 (74/74). Two ledger corrections, both dump-verified:
 *  1. THE PHANTOM PRODUCER HAS A NAME. s3/s5 called it "an ST_REGS-classed
 *     compare residue". It is an orphan `(insn (use (reg P)))` planted by
 *     combine.c's distribute_notes (tools/gcc-2.7.2/combine.c:10832-10841):
 *     combine rewrites/deletes the insn that DEFINED intermediate pseudo P,
 *     P's REG_DEAD note finds no home, the backward scan (combine.c:10757-10762)
 *     stops at the block's leading jump/label, and the USE is planted there.
 *     P then has no set and no constraint-bearing reference, so regclass leaves
 *     its printed class at the default `ST_REGS or none`, find_reg cannot seat
 *     it, and alter_reg pays 8 bytes of vars for zero emitted instructions.
 *     New instrument tmp/grind/func_800480C0/s6/count_uses.py counts these per
 *     function in a .combine dump; over six TUs it predicts the BB2_ALLOC_DEBUG
 *     phantom count exactly (SetDrawEnv 3/3, get_cs 2/2, ... ), the only
 *     disagreements being the mult/div DImode-HILO producer.
 *  2. s5's TREE-WIDE BOUND OF TWO IS REFUTED. SetDrawEnv (src/display.c:360)
 *     and SetDrawEnv2 (src/display.c:436) are mult-free ordinary C, COMPLETED-C
 *     on main, and each carry THREE orphan USEs / THREE phantoms:
 *     tmp/grind/func_800480C0/s5/asm/display.s:1243 reads
 *     `.frame $sp,64 # vars= 32, regs= 3/0, args= 16` (24 phantom + 8 for the
 *     live u16 buf[4]). s5's bound was an artifact of its census filter, which
 *     drops any function that also owns live stack traffic.
 *  AND THE REFRAME THAT MATTERS: this body's own phantom is the OTHER class.
 *  rejected/phantom-guard-vars8-ceiling.c and
 *  rejected/two-branch-guards-still-one-phantom.c both re-measure unalloc=1 with
 *  orphanUSE=0 on the current chassis, so the 30-form ceiling of s2-s5 bounds
 *  class B (non-combine) only; the combine-orphan producer that reaches 3 in
 *  this tree has never fired on func_800480C0 at all.
 *  Four new spellings measured class-A-negative (vars=0/unalloc=0/orphanUSE=0),
 *  banked in rejected/: s32-typed params with explicit (s16) casts, block-scope
 *  s16 intermediates, function-scope s16 intermediates, and a split shift
 *  ((word>>1)>>1)<<2 at the loop head.
 *  Instruments: tmp/grind/func_800480C0/s6/{probe.sh,runall.sh,dumpall.sh,
 *  count_uses.py}. probe.sh restores src/text1b.c from HEAD around every
 *  measurement, which the s3 probe did not. */
/* s19 (forensics, 2026-09-05, chassis HEAD e0174a57) - BODY UNCHANGED, floor re-measured
 * 20 with this body installed over the INCLUDE_ASM line (74 target insns, 74 build insns,
 * rules_dropped 0). Mandated FAKE re-audit re-run: one FAKE unit (arg0 = 0;), keep-all 20 /
 * drop-1 32 - load-bearing, masking no lever, identical on six chassis now.
 * THE SESSION'S TWO FINDINGS, both frontier items executed to a measurement.
 * (1) FRAME CHARGES ARE ADDITIVE, AND 8 + 24 REACHES THE TARGET FRAME. The s17 m1 folded
 * entry guard (one combine orphan, vars= 8) combined with a static __inline__ helper carrying
 * a 24-byte local (integrate.c:2085-2092 donates DECL_FRAME_SIZE verbatim) prints
 * .frame $sp,88,$31 # vars= 32, regs= 8/0, args= 24, extra= 0 - the target's exact
 * decomposition and its exact 0x58 - at 73 cc1 insns. The 16-byte donation gives vars= 24 and
 * the 32-byte donation vars= 40, so get_frame_size() is the plain sum of the alter_reg slot
 * (reload1.c:2382-2385, 8 bytes via function.c:687) and the donated block. No earlier session
 * had reached the target frame from two producers.
 * (2) AND IT STILL LOSES, ON THE STREAM RATHER THAN THE FRAME, WHICH RETIRES THE THREE-WAY
 * SPLIT. The phantom half is not free here: every folded-guard spelling puts the guard value
 * in a second pseudo and has to move it back at the loop head (addu $4,$17,-1 ... addu $17,$4)
 * where the target spends one addiu $s1,$s1,-0x1, so the combined body emits 73 instructions
 * against the target's 72 non-nop instructions and scores 25 - worse than this candidate's 20.
 * This candidate's stream already matches the target's length exactly; the residual is the 20
 * sp-relative operands and nothing else, so any producer costing an instruction is out.
 * (3) THE REGISTER-MASK ROUTE IS CLASS-DEAD. gp_reg_size == 4 * popcount(mask) by construction
 * (mips.c:4479-4486) and save_restore_insns emits one memory reference per bit (mips.c:4680);
 * the prologue is RTL (mips.md:6029) but the EPILOGUE IS PRINTED TEXT (mips.md:6056 expander
 * commented out, mips.h:2036 -> function_epilogue, mips.c:5174 save_restore_insns(FALSE,...,
 * file)), so no pass can delete a restore. The target prints eight restores, therefore
 * gp_reg_size = 32 and var_size + args_size = 56 with no register-mask freedom at all.
 * Bodies tmp/grind/func_800480C0/s19/bodies/p{0..4}.c, listings s19/last_p*.s,
 * ablation s19/fake_ablate.txt. */
void func_800480C0(s32 arg0, s32 arg1, s16 arg2, s16 arg3, s16 arg4, s16 arg5)
{
    u32 *p;
    s32 base_addr;
    s32 count;
    s32 new_var;
    base_addr = arg0;
    p = (u32 *)arg0;
    arg0 = 0; /* FAKE: breaks the $a0 == base_addr value association so cse2 cannot canonicalise the two base copies, mechanism: cse.c canonical-register substitution, lever-exhaustion: hypotheses.md s1-s3 */
    p = (u32 *)((s32)p + (((s32)(arg1 << 16)) >> 14));
    p = (u32 *)(base_addr + (((*p) >> 2) << 2));
    count = *(p++);
    if (count != 0) {
        s32 sx_arg2;
        s32 sx_arg3;
        s32 sx_arg4;
        s32 sx_arg5;
        count--;
        sx_arg2 = arg2;
        sx_arg3 = arg3;
        sx_arg4 = arg4;
        sx_arg5 = arg5;
        do {
            u32 word;
            s16 a1v;
            s16 a2v;
            s16 a3v;
            s16 v0v;
            word = *p;
            p = (u32 *)(((s32)p) + 4);
            a1v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a2v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            new_var = base_addr + (((u32)word >> 2) << 2);
            a3v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            v0v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            func_800482C8(new_var,
                          (s32)a1v + sx_arg2,
                          (s32)a2v + sx_arg3,
                          (s32)a3v + sx_arg4,
                          (s32)v0v + sx_arg5);
        } while ((count--) != 0);
    }
}
