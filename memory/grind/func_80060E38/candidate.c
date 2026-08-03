/* func_80060E38 — best form as of grind session 5 (permuter, 2026-08-03).
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
