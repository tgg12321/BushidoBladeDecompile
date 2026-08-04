/* func_80060E38 — best form as of grind session 8 (rederive, 2026-08-03).
 *
 * SESSION 8 UPDATE — body unchanged (the rederive instruments produced nothing worth
 * compiling into the tree), floor re-measured at 18 (sandbox --disable all: score 18,
 * target_insns 139 == build_insns 139, 18 rules dropped). Session 8 ran the three
 * rederive instruments and named the root cause:
 *   (1) FRESH m2c RE-DERIVATION (tools/m2c, --target mipsel-gcc-c) reconstructs the
 *       accepted body verbatim plus nine dead s32 locals named for the TARGET's own
 *       slot offsets (sp0, sp8, …, sp40) and one statement reorder. Both parts are
 *       already measured dead — declared locals take the align==0 path and give
 *       stride 4 (s2), reordering was killed by 11 variants (s2) and ~159.5k permuter
 *       mutations (s4+s5) — and the dead-locals part is the forbidden frame-coercion
 *       family besides. Banked as
 *       rejected/m2c-rederive-yields-dead-spill-locals-already-killed.c.
 *   (2) decomp.me CORPUS (3,754 cached scratches on the three BB2-class GCC 2.7.2 PS1
 *       compilers). No matching scratch has a genuine 0-mod-8 word reload spill: the 6
 *       apparent hits are blocks of stride-8 DECLARED STRUCT LOCALS (frame address-taken,
 *       same offsets also read with lhu), not spills. No existence proof, no transplant.
 *   (3) CROSS-PROJECT MEASUREMENT — the decisive one. All 1,751 MATCHING, non-override
 *       scratches were recompiled through OUR cc1 (the s7 instrumented build) from their
 *       own context+source at their own flags. 1,745 compiled; our cc1 allocated 300
 *       four-byte reload spill slots and 300/300 landed at 4 mod 8. Of the 30 comparable
 *       against the community-MATCHED target's own spill-shaped offsets, 29 sit exactly
 *       4 bytes ABOVE the reference compiler's slot and 1 is ambiguous; ZERO unambiguous
 *       agreements, across 7 scratches, 3 compiler packages and several flag families.
 *       This also closes the transplant leg: 1,745 foreign C bodies, no escape.
 *   (4) ROOT CAUSE, MEASURED: tools/gcc-2.7.2/Makefile:168 says `target=mips-mips-gnu`.
 *       Our cc1 is configured BIG-ENDIAN for a little-endian game, which is what arms
 *       BYTES_BIG_ENDIAN and the function.c:702-703 correction — and it is the same
 *       one-bit cause behind the documented [[bitfield-direction-divergence]]. This is a
 *       diagnosis, NOT a lever: rebuilding cc1 for mipsel is off the grind edit surface,
 *       forbidden by [[no-compiler-divergence]], and not free even for the operator,
 *       since flipping the bit also flips bitfield direction tree-wide and would break
 *       the ~1,400 functions already matched against current behaviour. Owner's call.
 *
 * (session-7 header follows)
 * func_80060E38 — best form as of grind session 7 (forensics, 2026-08-03).
 *
 * SESSION 7 UPDATE — body unchanged (forensics modality made no src/ edits), floor
 * re-measured at 18 (sandbox --disable all: score 18, target_insns 139 == build_insns 139,
 * 18 rules dropped). Session 6 instrumented alter_reg (the CALLER); session 7 instrumented
 * assign_stack_local itself (the CALLEE, function.c:665-728) in a private GCC copy at
 * tmp/grind/func_80060E38/s7/gcc, again validated byte-identical to stock build/cc1 on the
 * s1 probe with the trace both off and on. That closes the two inputs of session 2's closed
 * form that every prior session ASSUMED rather than measured:
 *   (A) frame_offset on entry. Declared locals advance the cursor at their natural size
 *       (stride 4 for s32, 1 for char), so locals totalling an odd multiple of 4 leave the
 *       cursor at 4 mod 8 before reload allocates the spills — which, if it survived, would
 *       put the nine spills at 0 mod 8 (target's congruence) under plain C control. It does
 *       not survive: function.c:692-698 does `frame_offset = CEIL_ROUND (frame_offset,
 *       alignment)` FIRST, and the spill path's alignment is BIGGEST_ALIGNMENT/8 == 8.
 *       Measured on 7 leaf probes (1/2/3/4/5/12/20 bytes of address-taken locals): traces
 *       show fo_in=1 -> fo_rounded=8 and fo_in=3 -> fo_rounded=8, and every spill in every
 *       probe still lands at 4 mod 8. Tree-wide, only 2 of 317 align==-1 allocations even
 *       entered non-8-aligned, and both were rounded away.
 *   (B) STARTING_FRAME_OFFSET. mips.h:1651-1653 uses the RAW
 *       current_function_outgoing_args_size (the MIPS_STACK_ALIGN padding is separate, at
 *       mips.c:4466), which is session 1's frontier item F1 sub-question (a) — never
 *       measured until now. It cannot be 4 mod 8: all three assignment sites (calls.c:1241,
 *       2388, 2738) round args_size.constant up to STACK_BYTES = STACK_BOUNDARY/8 = 8, then
 *       MAX it with REG_PARM_STACK_SPACE (16); OUTGOING_REG_PARM_STACK_SPACE is defined so
 *       no subtraction happens, STACK_POINTER_OFFSET is 0, and -mno-abicalls zeroes the
 *       ABICALLS term. Measured across all 31 TUs: 1694 assign_stack_local calls, sfo mod 8
 *       == 0 in 1694/1694, distinct sfo values {0,16,24,32,40}.
 * Consequence: `offset === -GET_MODE_SIZE(mode) (mod 8)` is now an UNCONDITIONAL theorem for
 * this fork over all C inputs, not a per-path result — both free variables are pinned. The
 * tree-wide split confirms it: 131/131 word slots on the spill path at 4 mod 8, while the 43
 * word slots that DO sit at 0 mod 8 are all align==0 declared locals at stride 4, the shape
 * s2's v_locals already killed (target needs stride 8 AND congruence 0, mutually exclusive
 * for a 4-byte value here).
 *
 * SESSION 6 UPDATE — body unchanged (forensics modality made no src/ edits), floor
 * re-measured at 18 (sandbox --disable all: score 18, target_insns 139 == build_insns 139,
 * 18 rules dropped). An instrumented cc1 (private copy of the GCC source at
 * tmp/grind/func_80060E38/s6/gcc, validated byte-identical to stock build/cc1 on the s1
 * probe) traced every reload spill-slot allocation across ALL 31 src/*.c translation units:
 * 131 allocations in 69 functions, 100% on alter_reg's from_reg == -1 path (reload1.c:2352),
 * ZERO on the REUSE path (2363) and ZERO on the "bigger slot" path (2369), every one SImode
 * with adjust == 0 and an offset === 4 (mod 8). The REUSE path is unreachable by construction
 * here: spill_hard_reg makes 98 from_reg != -1 calls tree-wide, but 97 have reg_renumber >= 0
 * and one has reg_equiv_memory_loc, so none reaches the allocation block that would populate
 * spill_stack_slot[]. The same sweep confirmed this function's nine slots inside its REAL
 * TU (src/text1b.c): pseudos 75-83, path=1, offsets 4,12,...,68 — identical to the isolated
 * probe, so the five-session harness is faithful. The 18-instruction gap is the reload pass's
 * big-endian slot correction, with no C-level lever.
 *
 * SESSION 5 UPDATE — body unchanged, floor re-measured at 18 (sandbox --disable all:
 * score 18, target_insns 139 == build_insns 139, 18 rules dropped; no src/ edits).
 * Session 5's permuter mandate was spent on the ONE neighbourhood sessions 4's two
 * chassis could not reach, plus a reading of the permuter's own mutation set:
 *
 *  (1) NEW MECHANISM FACT (source): function.c:879 allocates an assign_stack_temp slot as
 *      assign_stack_local(mode, size, mode == BLKmode ? -1 : 0), and function.c:702 computes
 *      bigend_correction ONLY when `mode != BLKmode`. So a BLKmode (aggregate) slot is the
 *      one route in function.c that gets the align == -1 treatment (8-byte alignment,
 *      CEIL_ROUND(size,8)) WITHOUT the +4. Measured: 8 aggregate variants (s5/gen5.py) —
 *      the aggregate slot really does start at 0 mod 8, confirming the mechanism.
 *  (2) It still cannot produce target's shape. An aggregate of 4-byte members is accessed
 *      at stride 4 inside that slot, not target's stride 8; reaching stride 8 needs explicit
 *      padding members (v_structpad: word slots 0,8,16,… — the forbidden dead-vars /
 *      frame-coercion family, and 184 insns vs target's 139). And in the mixed shapes
 *      (v_structfield / v_structfield_at) the aggregate sits at 0 mod 8 while the nine
 *      SImode reload spills alongside it are STILL at 44,52,…  ≡ 4 (mod 8). The slot-
 *      creation taxonomy in function.c is now closed: spills (align -1) = stride 8 / +4;
 *      locals (align 0) = stride 4 / +0; aggregates (BLKmode) = 0 mod 8 base / stride 4.
 *      No route gives a 4-byte value stride 8 AND congruence 0.
 *  (3) The permuter owns exactly ONE frame-layout operator, perm_pad_var_decl
 *      ("Inserts an unused variable to adjust stack offsets", randomizer.py:2247) — i.e. a
 *      generator of the forbidden dead-vars family, so its output could never be accepted;
 *      and it is inert anyway, because assign_stack_local CEIL_ROUNDs frame_offset to 8
 *      before every spill slot, erasing any padding-induced shift. Every other operator is
 *      expression/statement-level and cannot touch a congruence fixed by GET_MODE_SIZE.
 *  (4) Two more campaigns, both stopped in-session: "aggregate-chassis" (s5/ws3, base_score
 *      5133, 2,029 iters — finds only in the 4851-5133 range, none remotely near 72) and
 *      "decl-type-weighted" (s5/ws4, the near-floor base_score-72 chassis with
 *      perm_pad_var_decl zeroed and the decl/type/temp operators boosted, 62,904 iters,
 *      ZERO finds). Running total across s4+s5: ~159,500 mutations, no find below 72.
 * The permuter axis is closed by construction now, not only by sample size.
 *
 * (session-4 header follows)
 * func_80060E38 — best form as of grind session 4 (permuter, 2026-08-03).
 *
 * SESSION 4 UPDATE — body unchanged, floor re-measured at 18 (sandbox --disable all:
 * score 18, target_insns 139 == build_insns 139, 18 rules dropped; no src/ edits).
 * Session 4 put sessions 2-3's closed-form proof to a MACHINE search. A validated fast
 * permuter chassis was built (tmp/grind/func_80060E38/s4/mkws.sh -> s4/ws): it runs the
 * real build pipeline and its base.o differs from target.o in EXACTLY the 18 spill-offset
 * instructions (139 vs 139), i.e. it reproduces the engine's honest floor bit for bit.
 * Two campaigns through tools/permuter_campaign.py:
 *   chassis 1 "probe-chassis"      54,551 iterations / ~20 min / 0 finds
 *   chassis 2 "base-arith-chassis" 40,003 iterations / ~19 min / 0 finds
 * ~94,554 randomized C mutations produced ZERO outputs below the 72 baseline (= 18
 * differing instructions). Both harvested with --stop; neither outlives the session.
 *
 * TOOLING WARNING carried forward: the permuter's DEFAULT scorer normalizes sp-relative
 * offsets away, so a raw `permuter.py` run on this function reports score 0 — a FALSE
 * MATCH — because the whole remaining gap IS an sp-offset shift. Always go through
 * tools/permuter_campaign.py, which passes --stack-diffs (both campaigns correctly
 * launched at base_score 72). Do not run another campaign on this function.
 *
 * (session-3 header follows)
 * func_80060E38 — best form as of grind session 3 (structural, 2026-08-03).
 *
 * SESSION 3 UPDATE — body unchanged, floor re-measured at 18 (sandbox --disable all:
 * score 18, target_insns 139 == build_insns 139, 18 rules dropped; no src/ edits).
 * Session 3 attacked the ONE route session 2's closed-form proof left open — the
 * slot-REUSE branch of alter_reg (reload1.c:2363-2367), where an SImode pseudo
 * inherits a wider spill slot's base with no big-endian correction and would land at
 * 0 mod 8. Two findings, both measured:
 *   (1) 23 new compiled C variants (8 mode/pressure shapes + 15 leaf shapes designed
 *       to co-allocate a DImode and an SImode pseudo in one hard register) produced
 *       ZERO single-word spill slots at 0 mod 8. Every SImode spill was 4 mod 8, in
 *       every shape, including ones where reload demonstrably DID commandeer the hard
 *       register pair holding a long long ("Spilling reg 8/9", "24/25", "16/17").
 *   (2) Even a firing reuse branch is arithmetically insufficient: it yields at most
 *       ONE 0-mod-8 slot per commandeered hard register, and each requires a PRIOR
 *       8-byte-mode spill from that same register. Target needs NINE such slots, i.e.
 *       nine commandeered registers each with its own DImode/DFmode pseudo — while
 *       target's 139-instruction stream contains no 8-byte-mode instruction at all.
 * The reuse route is therefore dead as a closing lever, not merely unobserved.
 * Details in evidence.md (session 3 block) and hypotheses.md.
 *
 * (session-2 header follows)
 * func_80060E38 — best form as of grind session 2 (structural, 2026-08-03).
 *
 * SESSION 2 UPDATE — this body is unchanged and remains the best form; the floor is
 * still 18 (re-measured: sandbox --disable all score 18, 139 == 139). Session 2 proved
 * from the compiler source (reload1.c:2337-2410 + function.c:666-727) that a spilled
 * pseudo of mode M lands at offset === -GET_MODE_SIZE(M) (mod 8), so every 4-byte
 * reload spill in this fork is at 4 mod 8 and only DImode/DFmode reaches 0 mod 8.
 * Ten structural variants (order reversal, block scoping, statement re-association,
 * spill-count changes, long long / double mixing) all measured 4 mod 8; declared
 * locals measured stride 4, not target's stride 8. The original PsyQ cc1psx, fed the
 * byte-identical preprocessed input, emits the target's 0,8,...,64 with the same
 * 0x70 frame — the divergence is our fork's BYTES_BIG_ENDIAN. Details in evidence.md.
 *
 * (original session-1 header follows)
 * func_80060E38 — best form as of grind session 1 (recon, 2026-08-03).
 *
 * Honest pure-C floor: sandbox --disable all = 18 (unchanged this session; recon
 * modality made no source edits, so this is verbatim src/text1b.c:13562-13597).
 *
 * This body is already instruction-for-instruction identical to
 * asm/funcs/func_80060E38.s: 139 insns vs 139, same opcodes, same registers, same
 * order, same delay slots, same 0x70 frame, same s0-s7/fp save block at 0x48-0x68.
 * The entire remaining distance of 18 is the base of the 9 reload spill slots:
 *   target  sp+0x00 0x08 0x10 0x18 0x20 0x28 0x30 0x38 0x40
 *   ours    sp+0x04 0x0C 0x14 0x1C 0x24 0x2C 0x34 0x3C 0x44
 * i.e. a uniform +4, emitted by cc1's assign_stack_local() big-endian correction on
 * 8-rounded reload spill slots (function.c:681-703 + reload1.c:2352). See
 * memory/grind/func_80060E38/evidence.md for the full trace and hypotheses.md for
 * the live frontier.
 *
 * Do NOT try to "fix" the +4 with a padding local, dead array, or register pin —
 * that is the forbidden frame-coercion family and the cheat-invisible sandbox
 * strips it before scoring anyway.
 */
void func_80060E38(s32 arg0, s32 arg1) {
    D_800A3468 = 0x1F800000;
    D_800A346C = 0x1F800018;
    D_800A3470 = 0x1F800020;
    D_800A3474 = 0x1F800030;
    D_800A3488 = 0x1F800050;
    D_800A3490 = 0x1F800058;
    D_800A3494 = 0x1F80005C;
    D_800A3498 = 0x1F800060;
    D_800A349C = 0x1F800062;
    D_800A34A0 = 0x1F800064;
    D_800A34A4 = 0x1F800066;
    D_800A34A8 = 0x1F800068;
    D_800A34AC = 0x1F80006A;
    D_800A34B0 = 0x1F80006C;
    D_800A34B4 = 0x1F800070;
    D_800A34B8 = 0x1F800074;
    D_800A34BC = 0x1F800080;
    D_800A34C0 = 0x1F800082;
    D_800A34C4 = 0x1F800084;
    D_800A34C8 = 0x1F800088;
    D_800A34CC = 0x1F80008C;
    D_800A34D0 = 0x1F800090;
    D_800A34D4 = 0x1F800098;
    D_800A34D8 = 0x1F80009A;
    D_800A34DC = 0x1F80009C;
    D_800A34E0 = 0x1F80009E;
    D_800A34E4 = 0x1F8000A0;
    D_800A34E8 = 0x1F8000A4;
    D_800A3480 = 0x1F8000A8;
    D_800A3484 = 0x1F8000AC;
    D_800A348C = 0x1F8000B0;
    D_800A34EC = 0x1F8000B8;
    *(s32 *)0x1F800004 = arg0;
    *(s32 *)0x1F800008 = arg1;
}
