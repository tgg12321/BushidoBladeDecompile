"""wave95 rows (func-second-pass-4): Q132 mechanical names for 4 of the 20 neutral functions revived in the
func-second-pass-4 batch (NO_NAME ledger re-scan for evidence newer than each row's commit): the four
eff_Init* partners 0x8006505C / 0x800650A4 / 0x800650EC / 0x80065134 (main/51268) of the wave94
eff_DrawAndAdvanceQuad* drawers, the failed-check stub 0x80052C10 (main/3AB48, INCLUDE_ASM) and its
primitive-handler-table forwarder 0x80044650 (main/32D04).
DROPPED at pre-apply (naming-reviewer FAIL, N3 revive cutoff): 0x800650EC eff_InitQuadMirroredByViewSettingOutsideReplay and
0x80065134 eff_InitQuadNotMirroredByViewSetting; their NO_NAME ledger commit is wave94, the commit that named their partner
drawers, so no evidence is newer. Bring back only with evidence newer than wave94 or an owner ruling.
Research input: tmp/codex/func-second-pass-4-research.md (6 RENAME, 14 NO_NAME over 20 targets; untrusted proposals).
Every row was re-derived from the C bodies / asm, eff_StepCommand's dispatch, the handler tables, every C and asm
caller of 80052C10 and the packet dispatch loops that index the primitive-handler tables. No name changed.
Research NO_NAME (14; each stays neutral, not a row):
 80040D48 slot-object pose / draw-queue / softbody / light-target aggregate; no covering verb (g_node_pose_rotation_indices names only a table).
 800430E4 light-setup store + Y-halved view matrix + unrestored scratch word + table choice + unnamed callee func_8004DDB4.
 8004D634 instruction-identical to func_80050334 (packed-vertex handler twin); the table name cannot split them.
 8004D838 instruction-identical to func_80050538 (packed-vertex handler twin); the table name cannot split them.
 80050334 instruction-identical to func_8004D634.
 80050538 instruction-identical to func_8004D838.
 800474AC gpu_InitVramScroll(0, 0); preset-0 twin of func_80047738, presets differ only numerically.
 80047738 gpu_InitVramScroll(1, 0); preset-1 twin of func_800474AC.
 800474D0 gpu_QueueVramScroll(0); identical to func_8004775C.
 8004775C gpu_QueueVramScroll(0); identical to func_800474D0.
 800477BC empty selector hook, identical to three siblings (Q132 empty-hook exception).
 800477C4 empty selector hook, identical to three siblings.
 800477CC empty selector hook, identical to three siblings.
 800477D4 empty selector hook, identical to three siblings.
(DATA is empty.)
"""
import csv
import glob
import os
import re

VERDICT = "CONFIRM"
VERIFIER_NOTE = "naming-reviewer pre-apply PASS 4/6 (2026-10-28, naming-a); 0x800650EC / 0x80065134 FAIL N3 and dropped"
NOTES = {}

# names changed against the research (research name -> name used); an entry with old == new is a kept row
ADJUSTED = {
    "0x8006505C": ("eff_InitQuadPairWithFirstMirroredByViewSettingOutsideReplay", "eff_InitQuadPairWithFirstMirroredByViewSettingOutsideReplay", ""),
    "0x800650A4": ("eff_InitQuadPairNotMirroredByViewSetting", "eff_InitQuadPairNotMirroredByViewSetting", ""),
    "0x80052C10": ("sys_StoreZeroPastScratchpadOnFailedCheck", "sys_StoreZeroPastScratchpadOnFailedCheck", ""),
    "0x80044650": ("gpu_FailCheckOnUnhandledPrimitiveType", "gpu_FailCheckOnUnhandledPrimitiveType", ""),
}
DROPPED = {
    "0x800650EC": "pre-apply FAIL (N3 revive cutoff): its ledger commit is wave94 itself, where the partner drawer names landed; no newer evidence",
    "0x80065134": "pre-apply FAIL (N3 revive cutoff): its ledger commit is wave94 itself, where the partner drawer names landed; no newer evidence",
}

for _a, (_old, _new, _why) in ADJUSTED.items():
    if _old != _new:
        NOTES[_a] = f"adjusted from research {_old}: {_why}. naming-reviewer pre-apply PASS."

WT = os.environ.get("P3WT", r"C:\Users\Trenton\Desktop\bb2-worktrees\ln-naming-a")

# ---------------------------------------------------------------------------------------------------------
_SRC = {}
for _f in sorted(glob.glob(os.path.join(WT, "src", "main", "*.c"))):
    _SRC[os.path.basename(_f)] = open(_f, encoding="utf-8", errors="replace").read().split("\n")

_COV = {}
_covp = os.path.join(WT, "docs", "naming", "runtime", "matrix", "coverage.tsv")
_rd = list(csv.reader(open(_covp, encoding="utf-8"), delimiter="\t"))
_CAPS = _rd[0][5:]
for _r in _rd[1:]:
    _COV[_r[0].lower()] = [_CAPS[i] for i in range(len(_CAPS)) if _r[5 + i] not in (".", "")]


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
    for k, l in enumerate(lines):
        if l.startswith("INCLUDE_ASM") and fn in l:
            return f"src/main/{f}:{k + 1} (INCLUDE_ASM, no C body)"
    raise SystemExit(f"no C definition of {fn}")


def T(addr):
    """Reach clause from coverage.tsv (verified at build time); no role rests on it."""
    h = _COV[addr.lower()]
    return (f"TRACE: not used; coverage.tsv lists the function in {len(h)} of {len(_CAPS)} registered captures "
            f"(docs/naming/runtime/matrix/coverage.tsv), and no part of the name rests on a runtime observation (Q132 mechanical name).")


def ch(addr):
    if addr not in ADJUSTED:
        return ""
    old, new, why = ADJUSTED[addr]
    if old == new:
        return ""
    return f"CHANGED FROM RESEARCH: {old}; {why}. "


def R(addr, new, fn, f, body, callers, why, counter, prior):
    assert ADJUSTED[addr][1] == new, (addr, new)
    ev = (f"{ch(addr)}BODY: {B(fn, f)}: {body}. CALLERS: {callers}. WHY: {why}. COUNTER-EVIDENCE: {counter}. "
          f"Prior manifests: {prior}. {T(addr)}")
    return (addr, new, "RENAME", ev)


FUNCS = [
    R("0x8006505C", "eff_InitQuadPairWithFirstMirroredByViewSettingOutsideReplay", "func_8006505C", "51268.c",
      "copies g_eff_command_position[0..2] into g_eff_animated_quad_positions[6].world_x/y/z, sets "
      "g_eff_command_phase[12] = 1, clears g_eff_animated_quad_counters[6]; void, no calls, "
      "no read of view_setting",
      "only the handler-table word (g_eff_command_handlers[26], d_8BC50.c); eff_StepCommand calls "
      "g_eff_command_handlers[g_eff_command_handler_bases[cmd] + g_eff_command_phase[cmd]] (51268.c:138-141) and "
      "g_eff_command_handler_bases[12] = 0x1A (d_8BC50.c:202), so this is the phase-0 handler of command 12; "
      "phase 12 = 1 hands it to slot 27, eff_DrawAndAdvanceQuadPairWithFirstMirroredByViewSettingOutsideReplay (d_8BC50.c:296), which calls eff_DrawAnimatedQuads(6) "
      "and steps g_eff_animated_quad_counters[6] by 0x32 until 0x100",
      "eff_DrawAnimatedQuads reads g_eff_animated_quad_positions[mode_index] (51268.c:2139-2144) and counters[mode_index], "
      "so the record and counter this body fills are exactly those its paired drawer draws and advances; "
      "same body shape and Init + drawer-noun pattern as the accepted eff_InitCosineScaledQuad / "
      "eff_DrawAndAdvanceCosineScaledQuad and eff_InitRateSelectedCosineScaledQuad / "
      "eff_DrawAndAdvanceRateSelectedCosineScaledQuad. Mode 6 re-runs the body as mode 8 (mode_index += 2, goto again, :2500-2507) from the same projected position, so the record starts a pair of quads (the Pair is the drawer's). The twin (func_800650A4) differs from this one by the drawer's code, "
      "not by index alone: eff_DrawAnimatedQuads negates the width for modes 6 and 10 only when "
      "g_working_record.view_setting && g_main_loop_handler_index != 5 (51268.c:2450-2453) and never for 7 and 11",
      "the init itself never reads view_setting; the name borrows the drawer's noun through the handler pairing "
      "and the record index, as eff_InitCosineScaledQuad borrows CosineScaled; with the view setting off or in "
      "replay mode 6 draws like 7 and 10 like 11 (51268.c:2450-2475), which is why the condition is spelled in the name",
      "no_name.tsv w58/w72/w77/w83 record-index twin objection; the drawer names it rested on landed in wave94 (8961e2e4f)"),
    R("0x800650A4", "eff_InitQuadPairNotMirroredByViewSetting", "func_800650A4", "51268.c",
      "copies g_eff_command_position[0..2] into g_eff_animated_quad_positions[7].world_x/y/z, sets "
      "g_eff_command_phase[13] = 1, clears g_eff_animated_quad_counters[7]; void, no calls, "
      "no read of view_setting",
      "only the handler-table word (g_eff_command_handlers[28], d_8BC50.c); eff_StepCommand calls "
      "g_eff_command_handlers[g_eff_command_handler_bases[cmd] + g_eff_command_phase[cmd]] (51268.c:138-141) and "
      "g_eff_command_handler_bases[13] = 0x1C (d_8BC50.c:202), so this is the phase-0 handler of command 13; "
      "phase 13 = 1 hands it to slot 29, eff_DrawAndAdvanceQuadPairNotMirroredByViewSetting (d_8BC50.c:298), which calls eff_DrawAnimatedQuads(7) "
      "and steps g_eff_animated_quad_counters[7] by 0x32 until 0x100",
      "eff_DrawAnimatedQuads reads g_eff_animated_quad_positions[mode_index] (51268.c:2139-2144) and counters[mode_index], "
      "so the record and counter this body fills are exactly those its paired drawer draws and advances; "
      "same body shape and Init + drawer-noun pattern as the accepted eff_InitCosineScaledQuad / "
      "eff_DrawAndAdvanceCosineScaledQuad and eff_InitRateSelectedCosineScaledQuad / "
      "eff_DrawAndAdvanceRateSelectedCosineScaledQuad. Mode 7 re-runs as mode 9 from the same projected position, so this record also starts a pair. The twin (func_8006505C) differs from this one by the drawer's code, "
      "not by index alone: eff_DrawAnimatedQuads negates the width for modes 6 and 10 only when "
      "g_working_record.view_setting && g_main_loop_handler_index != 5 (51268.c:2450-2453) and never for 7 and 11",
      "the init itself never reads view_setting; the name borrows the drawer's noun through the handler pairing "
      "and the record index, as eff_InitCosineScaledQuad borrows CosineScaled; with the view setting off or in "
      "replay mode 6 draws like 7 and 10 like 11 (51268.c:2450-2475), which is why the condition is spelled in the name",
      "no_name.tsv w58/w72/w77/w83 record-index twin objection; the drawer names it rested on landed in wave94 (8961e2e4f)"),
    R("0x80052C10", "sys_StoreZeroPastScratchpadOnFailedCheck", "func_80052C10", "3AB48.c",
      "asm/funcs/func_80052C10.s:2-6: lui/ori $t9 = 0x1F800400; sw $zero, 0($t9); jr $ra. It ignores its arguments and "
      "returns nothing; 0x1F800400 is the first word after the 1 KB scratchpad (0x1F800000-0x1F8003FF)",
      "every call is reached only when a check has failed. C: 2B344.c:2256 (g_gpu_grid_cell_run_entry_peak >= 1000, the count is "
      "passed in $a0 and ignored), 2B344.c:2520 (count >= 5), 2B344.c:2703 (*src != 0 after the arena peak update), "
      "309CC.c:201 (null pointer), 32D04.c:694 and :736 (switch default), 32D04.c:985 (entry >= old_first), 35000.c:356 (arena "
      "size > 0x44FFF), 35000.c:461 (allocation did not fit), 368E4.c:1675 (scroll_index > 0), :1789/:1792 (entry "
      "category vs 0x33 split), :1811/:1814 (value / index bound), :1932 and :2056 (g_resource_entry_request_state < 0), :2172 "
      "(final else of a comparison chain). asm: arena_SetBlockEnd.s:36 (fall-through after the fit case), arena_UpdatePeak.s:23 "
      "(slt 0x44FFF), gpu_InitVramScroll.s:9 (blez skips it, so index > 0), gpu_OffsetTexPrimStream.s:332/:414 (after the "
      "handled-case j), gpu_UnpackGridRecordsNodesAndCellRuns.s:237 (peak >= 1000), node_QueueEntryNodesFromTransformOrUpdatedSlotNode.s:22 "
      "and node_QueueTableEntryNodePairUnderSlotNode.s:25 (bgez skips it, so a halfword < 0), reloc_KeepListedEntries.s:39 (slt bound), "
      "resource_RequestEntryWithLinkedEntriesAndLatchCategory.s:29/:39, resource_SetPairSelectionSlotWithBoundChecks.s:17/:23, "
      "slot_FinalizeObjectLoadSoundTexturesAndArena.s:66 (bnez skips it), snd_CopySelectedPackSectionsAndLoadVabPacks.s:68, "
      "softbody_CreateForNode.s:242 (beqz skips it), softbody_SetupObject.s:45 (slti 5 bnez skips it), func_8004C404.s:338 "
      "(tail j when the stream word is nonzero); table slots: D_80015470[2] and [3] (d_5BF0.c:73-74) and, through "
      "func_80044650, the default slots of five primitive-handler tables; no caller reads a result",
      "StoreZeroPastScratchpad states the whole body; OnFailedCheck is the call-graph role (naming-bar N3 class 3): every "
      "caller sits on the failed branch of a bound, null, state or table-slot check. sys_ because the callers span arena, "
      "resource, gpu, softbody, reloc and sound with no single subsystem (sys_Panic, 6CF8.c:236, is the sibling pattern). "
      "The guard globals are named (g_gpu_grid_cell_run_entry_peak, g_resource_entry_request_state, "
      "g_resource_entry_category, g_gpu_vram_scrolls)",
      "no Panic/Abort/Trap wording: it returns and the store's hardware effect (bus error or not) is not established; "
      "2B344.c:2251-2255 calls it a halt stub, which is a comment, not evidence; the fallback is sys_StoreZeroPastScratchpadEnd",
      "no_name.tsv w59/w91: Panic/Abort/Trap rejected because it returns, specific bounds-error names rejected because of the "
      "table entries; neither is claimed here"),
    R("0x80044650", "gpu_FailCheckOnUnhandledPrimitiveType", "func_80044650", "32D04.c",
      "func_80052C10(); return (asm/funcs/func_80044650.s:4 jal, no other work, no result)",
      "no direct call; it fills every slot with no emitter in the five primitive-handler tables D_800951D8 (32D04.c:1260-1302), "
      "D_80095280 (:1304-1346), D_80095328 (:1348-1436), g_gpu_packed_vertex_textured_primitive_handlers (:1438-1470) and "
      "g_gpu_embedded_lit_packet_handlers (:1472-1504); the tables are installed at scratchpad word 0x1F80001C "
      "(func_800430E4 32D04.c:495-510, :1147) and indexed by the halfword packet type in the stream loops "
      "(func_8004A940.s:7-13 and func_8004DDB4.s:440-446: lhu type, sll 2, add table, lw, jalr)",
      "slots with a real emitter hold gpu_Emit*, every other type value holds this, whose only effect is the failed-check "
      "store of its callee; gpu_ comes from the tables and the dispatch loops (all neighbours are gpu_Emit*)",
      "the exception for pure forwarders allows neutral, it does not require it; the callee is also called for bounds and null "
      "failures, so the forwarder's own content is the unhandled-type role every use bears out. Depends on the "
      "0x80052C10 row: if that row fails, this row stays NO_NAME",
      "no_name.tsv w88: pure func_80052C10 forwarder, table placement adds no body effect; the tables are named now"),
]

DATA = []
