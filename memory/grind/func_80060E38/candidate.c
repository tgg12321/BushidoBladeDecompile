/* func_80060E38 — best form as of grind session 3 (structural, 2026-08-03).
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
