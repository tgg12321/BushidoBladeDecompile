"""wave118 rows (func-second-pass-26): func_8001EEB4, func_8004746C, func_8004748C, func_800474F0, func_80047510.
Research input tmp/codex/func-second-pass-26-research.md (untrusted); every code claim re-derived at HEAD 005c275e9.

Re-derived code facts:
- func_8001EEB4 (src/main/9F9C.c:2358-2381): the gated common-script start plus the unconditional tail below. Only use:
  `.word func_8001EEB4` at src/main/d_7D870.c:46 = slot 0x10 of g_main_loop_handlers (fight_Init is slot 0 at :30). No jal.
  Handler 0x10 is written only at src/main/28708.c:1606 (replay_PlayFrame default case, not both fighters is_defeated).
  g_fight_followup_fighter_index: -1 at 9F9C.c:1270 and :2052; set to the other fighter at :5754-5756 when one is_defeated.
  Slot 0x11 (d_7D870.c:47) = fight_UpdateInterpolatedFighterViewAndSelectNextPhaseByModeOnTimeoutOrPadPress (9F9C.c:2383).
- func_8004746C / func_8004748C: 368E4.c:679 / :681, one-call wrappers of node_InitFrameEntriesFromWorldTranslations /
  node_AdvanceFrameAnimatedEntriesAndQueueDraws. Only use: row 4 (d_80DF8.c:879; table starts :875 = row 0) of g_resource_selector_callbacks.
- func_800474F0 / func_80047510: 368E4.c:687 / :689, the same two wrappers. Only use: row 7 (d_80DF8.c:882).
- Dispatch: .init by resource_InvokeLoadedSelectorInitCallback (2B344.c:2293-2297; called 368E4.c:70, :182); .unk4 by
  gpu_CollectGridDrawablesInvokeLoadedSelectorCallbackAndDrawBillboards (2B344.c:2082-2084).
- Callee keys on the selector: 32D04.c:1182-1186 case 7 -> frame_entry_count 0x11, case 4 -> 2; 35000.c:99-101 (!= 4), :122-129 (case 4 yaws).
"""
import glob
import os
import re

VERDICT = "CONFIRM"
VERIFIER_NOTE = "naming-reviewer pre-apply PASS 5/5 (fresh default-FAIL reviewer, lane naming-a 2026-11-01)"
NOTES = {}
DROPPED = {}

WT = os.environ.get("P3WT", r"C:\Users\Trenton\Desktop\bb2-worktrees\ln-naming-a")

_SRC = {}
for _f in sorted(glob.glob(os.path.join(WT, "src", "main", "*.c"))):
    _SRC[os.path.basename(_f)] = open(_f, encoding="utf-8", errors="replace").read().split("\n")


def B(fn, f):
    """src/main/<file>:<first>-<last> of the C definition of fn."""
    lines = _SRC[f]
    for k, l in enumerate(lines):
        if not re.match(r"^[A-Za-z_][\w \*]*\b%s\(" % fn, l) or l.startswith("extern"):
            continue
        j = k
        while j < len(lines) and lines[j] != "}" and not (j == k and l.rstrip().endswith("}")):
            j += 1
        return f"src/main/{f}:{k + 1}-{j + 1}" if j != k else f"src/main/{f}:{k + 1}"
    raise SystemExit(f"no C definition of {fn}")


_WRAP_USES = (
    "CALLERS: the {slot} slot of g_resource_selector_callbacks row {row} (src/main/d_80DF8.c:{line}; decl d_80DF8.c:{decl}); no jal, no other "
    "table entry (git grep over src, include, asm, symbol_addrs.txt, named_syms.txt, docs/engine: definition, decl, the one row, glabel, and the "
    "named_syms.txt RESET note only). {disp}"
)

_INIT4 = (
    f"BODY: {B('func_8004746C', '368E4.c')}: `node_InitFrameEntriesFromWorldTranslations();` and nothing else. "
    + _WRAP_USES.format(slot=".init", row=4, line=879, decl=26,
                        disp="Called only by resource_InvokeLoadedSelectorInitCallback (src/main/2B344.c:2293-2297) on the reuse (368E4.c:70) and fresh-load (368E4.c:182) paths. ")
    + "WHY: the callee subsystem keys on selector 4 itself: node_BindFrameDataAndReserveEntryBuffer case 4 -> frame_entry_count 2 (32D04.c:1185-1186); "
    "node_StartFrameAnimatedEntryAndInitPairedNode case 4 fixed yaws 0x800 / 0 (35000.c:122-129); node_AdvanceFrameAnimatedEntriesAndQueueDraws keeps "
    "faded selector-4 entries (35000.c:99-101). Selector4 = decimal table row, the wave101 / wave106 scheme (node_AdvanceSelector18FrameAnimatedEntriesAndQueueDraws, "
    "gpu_InitSelector14VramScroll, gpu_InitSelector5VramScroll). The body is instruction-identical to its row-7 and row-18 twins, so the row is the only code distinction. "
    "COUNTER-EVIDENCE: no stage word (the Vs. CPU opp3 stage that loads selector 4 is unnamed, 068c9bc78); trace is reach only "
    "(docs/naming/runtime/selector-hooks-versus-cpu-tatsumi-1), no role taken. Retired named_syms.txt:165 note 'game_EffInit_8004746C' is not evidence."
)

_ADV4 = (
    f"BODY: {B('func_8004748C', '368E4.c')}: `node_AdvanceFrameAnimatedEntriesAndQueueDraws();` and nothing else. "
    + _WRAP_USES.format(slot=".unk4", row=4, line=879, decl=27,
                        disp="Called only by gpu_CollectGridDrawablesInvokeLoadedSelectorCallbackAndDrawBillboards (src/main/2B344.c:2082-2084). ")
    + "WHY: instruction-identical to node_AdvanceSelector18FrameAnimatedEntriesAndQueueDraws (368E4.c:756) and func_80047510; the row is the code "
    "distinction, and the callee itself branches on selector 4 (35000.c:99-101 `resource_selector != 4`). Same Selector<row> scheme as its init twin. "
    "COUNTER-EVIDENCE: reach only in the trace (selector-hooks-versus-cpu-tatsumi-1); no stage word. Retired named_syms.txt:166 note is not evidence."
)

_INIT7 = (
    f"BODY: {B('func_800474F0', '368E4.c')}: `node_InitFrameEntriesFromWorldTranslations();` and nothing else. "
    + _WRAP_USES.format(slot=".init", row=7, line=882, decl=30,
                        disp="Called only by resource_InvokeLoadedSelectorInitCallback (2B344.c:2293-2297). ")
    + "WHY: the callee has a selector-7 case (32D04.c:1182-1184, frame_entry_count 0x11). Stage word by Q132, the evidence shape that landed "
    "node_InitLumberyardFrameEntriesFromWorldTranslations (wave102): docs/naming/runtime/evidence/only_x.tsv:2572-2605 has 34 pair rows for this address "
    "(tag stage:bamboo_thicket), 17 conclusive, with a conclusive zero for each of the 17 other stages (recounted at HEAD: 17 distinct other_tag values); "
    "docs/naming/runtime/matrix/facts.tsv:2063 reach.hit = 8 captures, all tagged stage:bamboo_thicket in matrix/scenarios.tsv, "
    "all in t_load; selector-hooks-versus-2p-bamboo-1 loads selector 7 once in Bamboo Thicket with this function the same frame. "
    "COUNTER-EVIDENCE: only_x.tsv:283 inconclusive (story-mode loads unchecked), so the stage word holds for the Select Battle Stage / Vs. paths shown. "
    "ALTERNATIVE (code-only, no stage claim): node_InitSelector7FrameEntriesFromWorldTranslations. Retired named_syms.txt:278 note is not evidence."
)

_ADV7 = (
    f"BODY: {B('func_80047510', '368E4.c')}: `node_AdvanceFrameAnimatedEntriesAndQueueDraws();` and nothing else. "
    + _WRAP_USES.format(slot=".unk4", row=7, line=882, decl=31,
                        disp="Called only by gpu_CollectGridDrawablesInvokeLoadedSelectorCallbackAndDrawBillboards (2B344.c:2082-2084). ")
    + "WHY: instruction-identical to func_8004748C and node_AdvanceSelector18FrameAnimatedEntriesAndQueueDraws; the row is the code distinction. "
    "No stage word: only_x.tsv:2606-2639 for this address, 0 of 34 pair rows conclusive (recounted), the same case that kept the selector-18 advance hook at Selector18 "
    "while its init took Lumberyard (fsp12). Trace: every second frame in Bamboo, selector 7, reach only. Retired named_syms.txt:279 note is not evidence."
)

_KO = (
    f"BODY: {B('func_8001EEB4', '9F9C.c')}: idx = g_fight_followup_fighter_index; entry = &D_80101EC8[idx]; when entry->move_kind is not 0xA / 0x17 / 0x18, "
    "airborne_until_landing == 0 and is_defeated == 0: motion_ResetVariantIndex(idx); script = motion_GetCommonScriptAtFighterBufferIndexedOffset(idx, "
    "entry->character_data_index); entry->move_is_common = 1; fighter_StartMoveScript(idx, script, 1); entry->hit_record_group_6_9_intact = 1 (:2375). "
    "Always: eff_StopCommands(); g_handler_frame_counter = 0; g_main_loop_handler_index = 0x11. "
    "USES: one, slot 0x10 of g_main_loop_handlers (src/main/d_7D870.c:46); docs/engine/main_loop.md:109 row 16 describes it (comment pass: update the name there). "
    "ENTRY: handler 0x10 is written only at 28708.c:1606 (replay_PlayFrame default case after the replay ends, not both fighters is_defeated; both -> 0xC); the "
    "follow-up index is the other fighter once one becomes is_defeated (9F9C.c:5754-5756), -1 at fight start (9F9C.c:1270, :2052). "
    "WHY: Followup fighter = the global's own noun; common script start + intact-flag write answer the w84 R6 N1 FAIL (the +0x26C write, member hit_record_group_6_9_intact); "
    "EnterInterpolatedViewPhase = handler 0x11 (fight_UpdateInterpolatedFighterView..., 9F9C.c:2383); eff_StopCommands left out as in sibling fight_InitFighterFocusViewAndThreeItemMenu. "
    "PostKo = Q132 role from runtime: c0f63fc4c guard-only stub-and-observe (docs/naming/runtime/versus-2p-bamboo-ko-start-poke-1: patched block skipped, survivor stays "
    "arms-raised at t_ko.10350, 48.48% differ) and ko-followup-versus-2p-bamboo-1 (one entry at 10150 t_ko, move_is_common 0 -> 1); hit map matrix/facts.tsv:1571 "
    "(32 hit captures, every hit in a round-ending kill phase). "
    "COUNTER-EVIDENCE (weak row): (1) no only-X event:ko row (only_x.tsv:26, :951-968 inconclusive), two hit captures lack the event:ko tag; (2) many KO captures never hit it "
    "(story-*, training-practice-ko-1, versus-2p-pair-*); (3) the screenshot change cannot be split between the script start and the intact flag; (4) which move the "
    "script is stays unknown; (5) eff_StopCommands not in the name; (6) 99 characters, longest landed fight_ name is 80. "
    "FALLBACK (code-only, no PostKo): fight_StartFollowupFighterCommonScriptMarkHitRecordGroup6To9IntactAndEnterInterpolatedViewPhase. If the reviewer rules the role word a "
    "respelling as in w97: NO_NAME (no_name.tsv:7) until an event:ko only-X matrix row exists."
)

FUNCS = [
    ("0x8001EEB4", "fight_StartFollowupFighterPostKoCommonScriptMarkHitRecordGroup6To9IntactAndEnterInterpolatedViewPhase", "RENAME", _KO),
    ("0x8004746C", "node_InitSelector4FrameEntriesFromWorldTranslations", "RENAME", _INIT4),
    ("0x8004748C", "node_AdvanceSelector4FrameAnimatedEntriesAndQueueDraws", "RENAME", _ADV4),
    ("0x800474F0", "node_InitBambooThicketFrameEntriesFromWorldTranslations", "RENAME", _INIT7),
    ("0x80047510", "node_AdvanceSelector7FrameAnimatedEntriesAndQueueDraws", "RENAME", _ADV7),
]

DATA = []
