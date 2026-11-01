"""wave113 rows (func-second-pass-21): the Inits of commands 0xE / 0xF (records 10 / 11), func_800650EC / func_80065134
(src/main/51268.c:1846-1852 / :1854-1860; prototypes d_8BC50.c:236 / :238; handler-table words :300 / :302 = indexes 30 / 32 =
bases[14] 0x1E / bases[15] 0x20 (d_8BC50.c:203) + phase 0). Research input tmp/codex/func-second-pass-21-research.md (untrusted; every
claim re-derived at HEAD 23f14978d, see notes.txt). The same pair was proposed by func-second-pass-4 (wave95 draft) with the same two
names and FAILed at pre-apply only on the N3 revive cutoff (pending/func-second-pass-4/rows.py:6-7). Words: Init (record set-up:
position copy, phase := 1, counter := 0); Quad(Not)MirroredByViewSetting(OutsideReplay) = the landed partner draws' names
(eff_DrawAndAdvanceQuadMirroredByViewSettingOutsideReplay 51268.c:2017 reads record/counter 10, ...NotMirroredByViewSetting :2029
reads 11), the cross-noun route of eff_InitCosineScaledQuad and the wave95 eff_InitQuadPair* Inits. Verdicts PENDING (the reviewer
fills them). (DATA is empty.)
"""
VERDICT = "PENDING"
VERIFIER_NOTE = "PENDING: naming-reviewer pre-apply not yet run (drafted 2026-10-31, naming-b)"
NOTES = {}
DROPPED = {}

_REV = ("REVIVE (N3): the NO_NAME ledger rows docs/naming/phase3/no_name.tsv:51-52 (rounds w58,w72,w77,w83,w94; commit column "
        "b0572cfb4 = the wave94 landing, 8961e2e4f in HEAD, 2026-10-28 16:04; the row text was last written by ec927446e, "
        "2026-10-28 16:23) are older than commit 67d523b68 (2026-10-31 19:40, an ancestor of HEAD, a descendant of both 8961e2e4f "
        "and ec927446e): it adds docs/naming/runtime/versus-cpu-tatsumi-green-stub-1/ (knockout.md / .tsv, baseline plus two "
        "stubbed-draw runs) and the tools/trace/scenarios/versus-cpu-tatsumi.json change. That is evidence neither the w94 round nor "
        "func-second-pass-4's pre-apply review had, which is the cutoff func-second-pass-4 failed on")

_COMMON = ("dispatch: eff_StartCommand (51268.c:84-115) zeroes g_eff_command_phase[command_word.h] (:89) and calls "
           "g_eff_command_handlers[g_eff_command_handler_bases[cmd] + phase] (:105-109), status_dest = the result; eff_StepCommand "
           "(:117-144) repeats it each step with the stored phase; tables d_8BC50.c:201-204 (bases) and :269- (handlers, index 0 at "
           ":270). Lifecycle by the code: phase 0 = this Init, then the phase-1 draw each step until its counter reaches 0x100")
_ISSUER = ("Command 0xE / 0xF is started only by eff_StartArgSelectedCommandWithGreenLight (51268.c:453-478; its only caller is "
           "fight_RecordReplayEventAndDispatchSoundOrEffect event_kind 15, 17AFC.c:5041-5042, passing its arg0): switch on "
           "command_variant 0 -> status[14] = 0, status_dest = &status[14], command_word 0x21000E (:460-464); 1 -> status[15], "
           "0x21000F (:465-469); git grep 0x21000E / 0x21000F over src include = only those two lines. The variant is chosen by the "
           "caller's argument, not by a free-slot fallback, so no First/Second wording is used")
_NOTE = ("TRACE (Q124, corroborating only; never the source of a word): commit 67d523b68, "
         "docs/naming/runtime/versus-cpu-tatsumi-green-stub-1/knockout.md, stub of the PARTNER DRAW")

FUNCS = [
    ("0x800650EC", "eff_InitQuadMirroredByViewSettingOutsideReplay", "RENAME",
     "BODY: src/main/51268.c:1846-1852: g_eff_animated_quad_positions[10].world_x/y/z = g_eff_command_position[0..2]; "
     "g_eff_command_phase[14] = 1; g_eff_animated_quad_counters[10] = 0; void, no calls, no read of view_setting; the asm "
     "(asm/funcs/func_800650EC.s:10-12) leaves v0 = 1 from the phase store, so status[14] = 1 selects the draw. "
     "CALLERS: none by name; the one use is the address-taken table word g_eff_command_handlers[30] "
     "(d_8BC50.c:300; prototype :236) = g_eff_command_handler_bases[14] (0x1E, d_8BC50.c:203) + phase 0; no jal and no other "
     "C/asm reference (git grep -w func_800650EC: the C definition, the prototype, the table word and the asm glabel/endlabel "
     ":1, :20 only). Partner (phase 1): "
     "eff_DrawAndAdvanceQuadMirroredByViewSettingOutsideReplay (0x800655E0, 51268.c:2017-2027, table word :301) calls "
     "eff_DrawAnimatedQuads(0xA) and steps counters[10] by 0x32 until >= 0x100. " + _ISSUER + ". " + _COMMON + ". "
     "WHY: Init = the record set-up (as the landed eff_InitCosineScaledQuad and eff_InitQuadPair* Inits). The qualifier is the "
     "partner draw's, true of the record this Init fills: eff_DrawAnimatedQuads(0xA) reads positions[10] and counters[10] "
     "(:2139-2144) and case 10 negates *p_w only when g_working_record.view_setting && g_main_loop_handler_index != 5 "
     "(:2451-2455), then falls through into the shared case 7/11 body. Mode 10 is not in the `mode_index < 9` re-run "
     "(:2501-2507), so it draws ONE quad (the Quad of the name, against the QuadPair Inits of modes 6/7). Index 5 is the replay "
     "mode (the draw's own OutsideReplay). "
     "COUNTER-EVIDENCE: (1) the Init draws nothing and never reads view_setting; its name is its partner draw's (cross-noun route "
     "of eff_InitCosineScaledQuad and wave95's eff_InitQuadPair*); (2) the twin 0xF differs only in record, phase and the draw's "
     "missing negation, so the qualifier is the only separator and is required; (3) 'Mirrored' is the conditional width "
     "negation, not an established texture identity or an observed configuration change (the capture did not exercise that "
     "branch); (4) no fighter-index, P1/P2, colour, Burst, Streak or Glow word: the issuer's argument is not tied to a fighter in "
     "the code and the Green word belongs to the issuer's point-light write; (5) not a Q132 pure forwarder: five stores, no calls. "
     "Prior manifests: docs/naming/phase3/no_name.tsv:51 rounds w58/w72/w77/w83/w94 (stores alone and the new global names do not "
     "separate the twin Inits); pending/func-second-pass-4 proposed this exact name and FAILed it at pre-apply only on N3 ("
     + _REV + "). The answer to the old objection is the consumer mechanics above (single record 10, one-quad draw, the draw's "
     "case-10 negation), not the global names alone. "
     + _NOTE + " eff_DrawAndAdvanceQuadMirroredByViewSettingOutsideReplay (0x800655E0, returns 0 from frame 8350): "
     "local changed boxes only at frames 9889-9900 (215/319/422/189 px) and 12314-12322 (288/421/598/653 px), every other sampled "
     "frame 0 px; both runs reached end_frame 13000. This corroborates the draw's output and reach; the Init was not stubbed and "
     "the knockout gives no word. coverage.tsv:858 (reach in 5 captures: lib-story-final-fight-retail-1, story-final-fight-stages-1, "
     "story-gengoro-stage-0e-1, versus-cpu-clear-retail-1, versus-cpu-tatsumi-1)."),
    ("0x80065134", "eff_InitQuadNotMirroredByViewSetting", "RENAME",
     "BODY: src/main/51268.c:1854-1860: g_eff_animated_quad_positions[11].world_x/y/z = g_eff_command_position[0..2]; "
     "g_eff_command_phase[15] = 1; g_eff_animated_quad_counters[11] = 0; void, no calls, no read of view_setting; the asm "
     "(asm/funcs/func_80065134.s) leaves v0 = 1 from the phase store, so status[15] = 1 selects the draw. "
     "CALLERS: none by name; the one use is the address-taken table word g_eff_command_handlers[32] "
     "(d_8BC50.c:302; prototype :238) = g_eff_command_handler_bases[15] (0x20, d_8BC50.c:203) + phase 0; no jal and no other "
     "C/asm reference (git grep -w func_80065134: the C definition, the prototype, the table word and the asm glabel/endlabel "
     ":1, :20 only; docs/naming/data_evidence/D_800F0BC4.md:14 is stale pre-restructure history). Partner (phase 1): "
     "eff_DrawAndAdvanceQuadNotMirroredByViewSetting (0x80065630, 51268.c:2029-2039, table word :303) calls "
     "eff_DrawAnimatedQuads(0xB) and steps counters[11] by 0x32 until >= 0x100. " + _ISSUER + ". " + _COMMON + ". "
     "WHY: Init = the record set-up. The qualifier is the partner draw's: eff_DrawAnimatedQuads(0xB) reads positions[11] and "
     "counters[11]; case 11 enters the shared body below case 10's width negation (:2456-), so the negation is skipped; one "
     "quad (mode 11 is not in the `mode_index < 9` re-run). "
     "COUNTER-EVIDENCE: as for 0xE: (1) the Init draws nothing and never reads view_setting; (2) the qualifier separates it from "
     "the twin and from the QuadPair Inits (modes 6/7, two quads by re-run); (3) NotMirroredByViewSetting means only the absence "
     "of case 10's conditional negation: mode 11 still changes its texture record, size and shape with view_setting "
     "(:2202-2219, :2456-2500), so the name must not be read as configuration-independent; (4) no fighter, colour or Burst word; "
     "(5) not a pure forwarder. Prior manifests: docs/naming/phase3/no_name.tsv:52 rounds w58/w72/w77/w83/w94; "
     "pending/func-second-pass-4 proposed this exact name, FAIL only on N3 (" + _REV + "). "
     + _NOTE + " eff_DrawAndAdvanceQuadNotMirroredByViewSetting (0x80065630, returns 0 from frame 8350): local "
     "changed boxes only at frames 9983-9992 (272/406/527/638 px), every other sampled frame 0 px; both runs reached end_frame "
     "13000. Corroboration of the draw's output only; the Init was not stubbed. coverage.tsv:859 (reach in 5 captures: "
     "lib-story-final-fight-retail-1, story-final-fight-stages-1, story-gengoro-stage-0e-1, versus-cpu-clear-retail-1, "
     "versus-cpu-tatsumi-1)."),
]

DATA = []
