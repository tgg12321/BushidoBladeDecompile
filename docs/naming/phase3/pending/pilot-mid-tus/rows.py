"""wave89 rows (pilot-mid-tus): Q132 mechanical names for 54 of the 61 neutral functions in main/17AFC (the fighter /
fight / object helpers between the bone-hit resolver and the frame-event dispatcher), main/35000 (the frame-animated
node entries, the ndata read wrappers and the slot / selected-entry arena block loaders), main/4D0A8 (the GPU
primitive builders of the result / counter / image screens) and main/5ED34 (the buffered-display slide state and its
entry-quad renderers).
Research input: tmp/codex/pilot-mid-tus-research.md (54 RENAME, 7 NO_NAME over 61 targets; untrusted proposals).
Every row was re-derived from the C body and every caller (src/main/*.c, include/, asm/funcs/*.s, named_syms.txt,
symbol_addrs.txt); names changed against the research say CHANGED FROM RESEARCH in the evidence and in the row note.
Research NO_NAMEs (7; stay neutral, not rows): 8002798C (callers use it both when dropping a queued object and when
setting fighter rendering/state bits, so no single predicate name covers both), 80044F30 80044F80 (fixed-bias
forwarders of ndata_Read(a0 + 0x27 / 0x4D, a1), distinguishable only by the numeric bias), 80071C20 (the property /
result contract of the D_8009BC7C test is not shown), 80072084 800720AC 800720D4 (fixed-field forwarders of the
menu_UpdateChoiceGrid family; a name would only restate the callee).
Research RENAMEs dropped by this draft: none (all 54 kept: 16 as proposed, 38 reworded or re-prefixed, see ADJUSTED).
Pre-apply review (naming-reviewer round 1): 43/54 PASS as proposed; 11 FAIL (80027640 80033898 80045A50 80046020 8005D46C
800600C8 8006EC0C 8006F100 8006F528 80071C4C 80073200) and were replaced by the reviewer-suggested names (NOTES); none dropped.
Prefix evidence (the lessons of pilot-tail-a): object_ is 17AFC.c's thrown-object pool (Obj80106A78) and is not used
for the 35000.c arena-block helpers; those that act on arena ids 0..2 / +3 (the D_800A9A10[3] slot table, func_80045AA4's
a0 < 3 branch calls func_80041430 which rewrites D_800A9A10[a0]) take slot_, the vocabulary pilot-tail-a (wave88)
proposes for the same table (this wave depends on that vocabulary: if wave88 does not land, slot_ must be re-judged).
The D_800A9CF8 entries are transform nodes of the Unk80101DF0Record layout (include/game.h:2037-2051), run from
key-frame data and queued on g_draw_queue, so node_ (node_UpdateWorldMatrix, node_RebaseParents) is used instead of
the research's resource_ for the two that run them; the rebase helper is called from resource_RebaseLoadedBlockSections
and keeps resource_. fight_ / fighter_ follow the TU's own families (fight_GetPositionRecordList,
fight_UpdateBallisticRecords; fighter_ResolveBoneHit, fighter_AccumulateOrClearByBoneGroup); 80033898 loses fight_ because it touches no fight data (it resets the graph, clears a frame counter and sets the main-loop handler index). gpu_ follows the callees
(gpu_AddSpriteCells, gpu_AddScaledSpriteQuads, gpu_AddRotatedSpriteQuads, gpu_InitPrimBufferPair) only where the body builds primitives; the 5ED34 handlers that also write the dispatcher state or the result (8006EC0C, 8006F100, 80071C4C) are unprefixed. menu_ is NOT used in
5ED34.c or 4D0A8.c: the prior manifests (no_name.tsv w56 / w60) record that the draw-only helpers read no pad and that
the outer init / dispatcher spans timed phases as well as choice screens, so those two keep an unprefixed verb name as the
research proposed. The research's data suggestions (g_fight_ballistic_records, g_frame_animated_node_data,
g_gpu_time_row_bar_progress ...) need separate all-access data manifests and are not part of this wave (DATA is empty).
"""
import csv
import glob
import os
import re

VERDICT = "CONFIRM"
VERIFIER_NOTE = ("naming-reviewer pre-apply (round 1): 43/54 PASS as proposed; 11 FAIL (80027640, 80033898, 80045A50, "
                 "80046020, 8005D46C, 800600C8, 8006EC0C, 8006F100, 8006F528, 80071C4C, 80073200) replaced by the "
                 "reviewer-suggested names (NOTES); none dropped.")
NOTES = {
    '0x80027640': 'suggested: naming-reviewer pre-apply round 1 failed fighter_CountRelocateToStartPositionClearVelocitiesStartEventMoveAndDispatchEvent (ToStartPosition false on the D_800A36A4 == 3 path, 17AFC.c:202-215, which blends toward an opponent-derived corner) and suggested fighter_RelocateAndStartEventMove; R6 must judge it fresh at every use.',
    '0x80033898': 'suggested: naming-reviewer pre-apply round 1 failed fight_ResetGraphClearFrameCounterAndSetMainLoopHandler3 (fight_ prefix unexplained: touches no fight data, sole reference is the main-loop handler table D_8008D090, handler 3 is the D_80102778 settings editor) and suggested ResetGraphClearFrameCounterAndSetMainLoopHandler3; R6 must judge it fresh at every use.',
    '0x80045A50': 'suggested: naming-reviewer pre-apply round 1 failed slot_CloseVabAndFreeObjectAndAssetBlocks (callee snd_StopAllVoicesAndCloseSlotVab stops all voices first, 4BC3C.c:101-108) and suggested slot_StopAllVoicesCloseVabAndFreeObjectAndAssetBlocks; R6 must judge it fresh at every use.',
    '0x80046020': 'suggested: naming-reviewer pre-apply round 1 failed resource_FreeSelectedEntryBlockAndCloseVab2And5 (callee snd_StopAllVoicesAndCloseSelectedEntryVabs stops all voices, 4BC3C.c:110-118) and suggested resource_FreeSelectedEntryBlockStopAllVoicesAndCloseVabs; R6 must judge it fresh at every use.',
    '0x8005D46C': "suggested: naming-reviewer pre-apply round 1 failed gpu_DrawFixedCellPairWithSelectedSheets ('CellPair' false: 9 cells over two sheet draws D_8009B340[3] / D_8009B358[6], 4D0A8.c:323-353) and suggested gpu_DrawFixedSpritePairWithSelectedSheets; R6 must judge it fresh at every use.",
    '0x800600C8': "suggested: naming-reviewer pre-apply round 1 failed gpu_DrawDecimalValueWithMagnitudeAlignedLabel ('Label' a guess: a fixed 3-cell sprite whose x shifts with digit count, 4D0A8.c:1410-1452) and suggested gpu_DrawOneOrTwoDigitValueAndDigitCountShiftedSprite; R6 must judge it fresh at every use.",
    '0x8006EC0C': 'suggested: naming-reviewer pre-apply round 1 failed gpu_StepSlideProgressCommitPendingStateAndQueueSound (gpu_ unexplained: no GPU call/prim/OT write; callee snd_, writes dispatcher state D_800A3580/D_800A3578; caller unprefixed) and suggested StepSlideProgressCommitPendingStateAndQueueSound; R6 must judge it fresh at every use.',
    '0x8006F100': "suggested: naming-reviewer pre-apply round 1 failed gpu_DrawRotatedEntriesWithCountdownOrDarkenedSelectionView (handler also writes pending state D_800A3584 = 3, 5ED34.c:419, and queues sounds 7/8; gpu_ claims a primitive builder; 'SelectionView' a reading) and suggested DrawRotatedEntryCountdownAndRequestState3OrDrawDarkenedSelection; R6 must judge it fresh at every use.",
    '0x8006F528': "suggested: naming-reviewer pre-apply round 1 failed gpu_DrawMirroredFrameQuadsWithSlideDrawAreaAndOffset ('Frame' unsupported and ambiguous with per-frame records) and suggested gpu_DrawMirroredQuadsWithSlideDrawAreaAndOffset; R6 must judge it fresh at every use.",
    '0x80071C4C': 'suggested: naming-reviewer pre-apply round 1 failed gpu_DrawRotatedEntriesAdvanceCounterAndStoreSelections (omits the exit writing result D_800A35A0 = 1 or next state D_800A3584 = 5/6 and phase 1, 5ED34.c:1413-1426; gpu_ claims a primitive builder) and suggested DrawRotatedEntriesAndOnFullCounterFinishWithResultOrNextStateAndStoreSelections; R6 must judge it fresh at every use.',
    '0x80073200': 'suggested: naming-reviewer pre-apply round 1 failed gpu_DrawTintedQuadVariantsAndFrameCycledCellLayers (the always-drawn var_1[5] layer is fixed, 5ED34.c:1960; only the optional second layer cycles) and suggested gpu_DrawTintedFlippedQuadsFixedCellLayerAndFrameCycledLayer; R6 must judge it fresh at every use.',
}

# names changed against the research (research name -> name used)
ADJUSTED = {
    "0x8002738C": ("fighter_AccumulateHitBoneGroups", "fighter_AccumulateBoneGroupFlagWhenGameModeZero",
                   "'Hit' is the caller's context (fighter_ResolveBoneHit passes the bone), not shown by the body; the body "
                   "ORs one flag per bone group into the byte at D_800A376A + a0 only while the game mode D_800A38DC is 0"),
    "0x80027640": ("fighter_RelocateAndRestartEventMoveWithSound", "fighter_RelocateAndStartEventMove",
                   "'WithSound' overclaims (func_80032854 command 0x30 is a sound-or-effect dispatcher, 17AFC.c:4672) and "
                   "'ToStartPosition' is false on the D_800A36A4 == 3 path (17AFC.c:202-215), which blends toward a corner derived "
                   "from the opponent's world_pos instead of the start-position table; the body also bumps the relocation counter "
                   "unk_34C, clears unk_104 / unk_134 and dispatches the event, which the name leaves to the body; 'Restart' is "
                   "replaced by 'Start' (it starts the move named by the event-move table)"),
    "0x8002C0DC": ("fight_UpdateSpacingFacingAndFrameHitsAndQueueSeparationMoves",
                   "fight_UpdateSpacingTurnFacingWeaponBodyHitsAndQueueMoveEvents",
                   "'FrameHits' is vague (the call is fighter_ProcessWeaponBodyHits(1)) and 'SeparationMoves' is unshown: the "
                   "body writes pending_move_event 4 and 5 on the two fighters, nothing says it is about separation"),
    "0x80032854": ("fight_RecordAndDispatchSoundOrEffectEvent", "fight_RecordReplayEventAndDispatchSoundOrEffect",
                   "the first statement is replay_RecordEvent(arg0..arg3); say which record it makes"),
    "0x80033898": ("fight_ResetGraphAndEnterConfigurationEditor", "ResetGraphClearFrameCounterAndSetMainLoopHandler3",
                   "the body stores D_800A3834 = 3 (the main loop's handler index, D_8008D090[3]); that handler is the "
                   "D_80102778 settings editor, but 'configuration' is a reading of its font_Printf columns, so the name states "
                   "the write, plus the D_800A37B8 (frame counter) clear; no fight_ prefix: the body touches no fight data and "
                   "its sole reference is the main-loop handler table D_8008D090"),
    "0x80033DF4": ("fight_AdvancePresetOpponentSequenceOrRecordTimeAndCompletion",
                   "fight_AdvanceStepTableRowOrInsertBestTimeAndSetFlags",
                   "'PresetOpponent' is unshown (D_8008E778 rows of four bytes feed D_800A384C / D_800A38EC..EE / unk_4[1]); "
                   "'Completion' is unshown (the end branch ORs mask bits into D_80106A50.unk_00 and calls fight_InsertBestTime)"),
    "0x80033FE4": ("fight_SelectPresentationAndPhaseAfterTimeDisplay",
                   "fight_SetSelectorAndMainLoopHandlerFromFlagsAndBestTimeRank",
                   "'AfterTimeDisplay' and 'Presentation' are unshown; the body sets D_800A38A4 (6..9) and the handler index "
                   "0x12 from D_800A36F0 / D_800A3781, else the handler index 0x1A / 8 from D_800A38E9 < 3"),
    "0x80044800": ("resource_UpdateFrameAnimatedNodesAndQueueDraws", "node_AdvanceFrameAnimatedEntriesAndQueueDraws",
                   "the table entries are transform nodes (Unk800A9CF8Entry begins with a node, include/game.h:2037-2051) and "
                   "the callers are three handler stubs in 368E4.c, not resource loaders; node_ is the transform-node vocabulary"),
    "0x80044B30": ("resource_StartIndexedNodeAnimation", "node_StartFrameAnimatedEntryAndInitPairedNode",
                   "same prefix reason as 80044800; the body also initialises the paired Unk800A6690Rec node (unk_10 table)"),
    "0x80044C70": ("resource_RebaseFrameAnimatedNodeData", "resource_RebaseFrameAnimatedEntryBlockAndPointers",
                   "the body is reloc_RebaseBlock(D_800A9CF8.unk0, a0) plus two pointer bumps; 'NodeData' is imprecise"),
    "0x80044ED8": ("ndata_ReadRemappedIndexFromPackOrDisc", "ndata_ReadRemappedIndexPackFirstElseRead",
                   "the fallback is ndata_Read (not shown to be the disc), the first try is ndata_ReadFromPack"),
    "0x80044F50": ("ndata_ReadFromSelectedIndexRange", "ndata_ReadIndexInRangeSelectedByFlag",
                   "a0 is a flag choosing the base (0x83 or 0x10C) added to a1"),
    "0x80045878": ("resource_LoadOrReuseObjectAndAssetBlocks", "slot_OpenObjectBlockFreeSoftbodyAndLoadOrReuseAssetBlock",
                   "slot_ prefix (arena id a0 is the D_800A9A10 slot, func_80045AA4 a0 < 3 -> func_80041430); the body also "
                   "frees the softbody object when the asset block exists (softbody_FreeObject), omitted by the research"),
    "0x80045A28": ("resource_ResizeObjectAssetBlockAndUpdatePeak", "slot_ResizeAssetBlockAndUpdatePeak",
                   "slot_ prefix; the resized block is arena a0 + 3, the asset block, not the object"),
    "0x80045A50": ("resource_CloseObjectSoundAndFreePairedBlocks", "slot_StopAllVoicesCloseVabAndFreeObjectAndAssetBlocks",
                   "slot_ prefix; the callee snd_StopAllVoicesAndCloseSlotVab (4BC3C.c:101-108) stops all voices first "
                   "(func_800858D0(0)) and then SsVabClose(3 * a0 + 1), a VAB close, not 'sound'; both effects are in the name"),
    "0x80045AA4": ("resource_RebaseObjectOrAssetBlockAndSound", "slot_RebaseObjectOrAssetBlockAndFakeOpenVab",
                   "slot_ prefix; the sound step is snd_VabFakeOpen(a1, 3 * arena_id + 1) when bit 1 of unk_00 is set"),
    "0x80046020": ("resource_FreeSelectedEntryBlockAndCloseSounds", "resource_FreeSelectedEntryBlockStopAllVoicesAndCloseVabs",
                   "the callee snd_StopAllVoicesAndCloseSelectedEntryVabs (4BC3C.c:110-118) stops all voices first and then closes "
                   "VAB 2 and VAB 5 (snd_CloseVab1 is the existing single-VAB sibling); the VAB numbers stay out of the name"),
    "0x80046048": ("resource_RebaseSelectedEntryBlockAndSound", "resource_RebaseSelectedEntryBlockAndFakeOpenListedVabs",
                   "the body walks a count-prefixed list at the block and calls snd_VabFakeOpen(a1, id) for each"),
    "0x8005C8A8": ("gpu_DrawChoicePanelWithSelectionHighlight", "gpu_DrawModeSizedPanelWithSelectedCellOpaque",
                   "'Choice' and 'Highlight' are readings; the body draws a panel sized by D_8009B2BC[mode] and keeps the "
                   "cell matching the selection opaque (semi = 0) and the others semi-transparent"),
    "0x8005D814": ("gpu_DrawTimeFieldsAndDecimalCounterPanel", "gpu_DrawTimeFieldsAndDecimalCounter",
                   "'Panel' is more than the body shows; the time fields are minutes / seconds / hundredths from "
                   "fight_SplitFrameCounterIntoMinutesSecondsHundredths (9F9C.c:1460) and the counter is arg1 - 1"),
    "0x8005E098": ("gpu_DrawDecimalCounterPanelWithOptionalOppositeSide", "gpu_DrawTwoDigitCounterWithOptionalSecondSide",
                   "'Panel' and 'Opposite' are readings; arg1 is always drawn at the right (x 0x209) and arg0, when "
                   "non-negative, as a second counter at the left (x 0x50)"),
    "0x8005E51C": ("gpu_DrawDecrementedCounterInSingleSidePanel", "gpu_DrawDecrementedSingleSideCounter",
                   "it is func_8005E098(-1, a0 - 1, a1, a2); 'Panel' dropped as above"),
    "0x8005E54C": ("gpu_DrawPackedComparisonRowsAndTotalsPanel", "gpu_DrawPackedComparisonRowsAndTotals",
                   "'Panel' dropped; per round (D_8009BD24.unk14_10 + 3) the two 2-bit values of arg0 are compared and drawn, "
                   "then the sums"),
    "0x8005F1C8": ("gpu_DrawPairedMarkRowsAndTimePanel", "gpu_DrawPairedMarkRowsAndTimeFields",
                   "'Panel' dropped; arg0 carries unk_2 (seconds) and unk_3 (hundredths) of the split counter (9F9C.c:1608-1612)"),
    "0x8005FBC8": ("gpu_LoadImagePaletteAndResetSplitWipe", "gpu_ReadCdFileUploadTwoRegionsAndResetSplitWipe",
                   "'ImagePalette' reads TIM block offsets (+0x40 / +0x14) the body never names; it makes two LoadImage "
                   "uploads and clears D_800A3278, the counter gpu_DrawImageAndAdvanceSplitWipe advances"),
    "0x800602AC": ("gpu_LoadAndRelocateImagePairWithPalettes", "gpu_ReadCdFileRelocateTwoImagesAndUploadFourRegions",
                   "same TIM-offset reason; the body makes four LoadImage uploads from two relocated pointers"),
    "0x80060544": ("gpu_DrawFixedSpriteCellsAndStretchedQuadWithRedDecoration",
                   "gpu_DrawFixedSpriteCellsStretchedQuadAndRedTintedCells",
                   "'Decoration' is a reading; the last two sprite-cell draws use col 0xFF / 0x10 / 0x10 with has_color = 1"),
    "0x80060758": ("gpu_ResetTimeDisplayBarProgress", "gpu_ResetGrowingBarProgress",
                   "'TimeDisplay' is unshown; D_800A32B4 / D_800A32B6 are the 0..0x1E progress values func_80060768 "
                   "increments and scales into bar widths"),
    "0x80060768": ("gpu_DrawTimeRowsWithGrowingBars", "gpu_DrawIndexedCellsDigitPairsAndGrowingBars",
                   "'Time' is not shown here (the first half is the existing gpu_DrawIndexedCellsAndDecimalDigitPairs call)"),
    "0x8006E534": ("InitChoiceAndTimedDisplayResourcesAndState", "InitDisplayStateLoadPackAndSetEntryFlags",
                   "'Choice' / 'Timed' are readings; the body clears the OT, stores its arguments, reads one of packs "
                   "2..5 by D_800A35BC, relocates it, sets the D_8009BC7C per-entry flags from arg3 and the cursor entries"),
    "0x8006E950": ("gpu_ReadRelocateAndUploadSplitImagePack", "gpu_ReadCdFileRelocateAndUploadImagesAndBottomStrip",
                   "the body makes two LoadImage uploads and calls gpu_LoadVramBottomStrip; 'Split' is unshown"),
    "0x8006EA28": ("resource_RelocateDisplayListsAndLoadSoundPack", "resource_RelocateNineOffsetListsAndLoadVabPack",
                   "'DisplayLists' is unshown; the body relocates nine offset lists and calls snd_LoadVabPack"),
    "0x8006EACC": ("UpdateBufferedDisplayAndDispatchState", "StepSlideBuildFrameRecordAndDispatchStateHandler",
                   "the body picks the draw buffer, steps the slide (func_8006EC0C), builds the frame record and calls "
                   "D_8009BC1C[D_800A3580]; 'Buffered display' alone omits the slide step"),
    "0x8006EC0C": ("gpu_AdvanceSlideAndCommitPendingDisplayState", "StepSlideProgressCommitPendingStateAndQueueSound",
                   "it also queues sound 5 (when the full slide starts closing); no gpu_ prefix: the body has no GPU call, "
                   "primitive or OT write, its sound callee is snd_, it writes the dispatcher state D_800A3580 / D_800A3578, and "
                   "its caller func_8006EACC is unprefixed"),
    "0x8006F100": ("gpu_UpdateRotatedEntryDisplayTransition", "DrawRotatedEntryCountdownAndRequestState3OrDrawDarkenedSelection",
                   "'Transition' is a reading; the body draws rotated quads with per-entry countdown offsets and cues (sounds 7 "
                   "and 8), requests the pending state D_800A3584 = 3 once D_800A355C reaches 0x79 (5ED34.c:419), or, while "
                   "D_800A3550 != 0, darkens the screen and draws the selected entries and the panel layers; no gpu_ prefix "
                   "(the handler also writes dispatcher state, so it is not only a primitive builder); 'SelectionView' was a "
                   "reading and is dropped"),
    "0x8006F528": ("gpu_DrawMirroredFrameBackgroundAndSlideWindow",
                   "gpu_DrawMirroredQuadsWithSlideDrawAreaAndOffset",
                   "'Background' is unshown and 'Frame' is unsupported and ambiguous with the per-frame records; the body draws "
                   "mirrored scaled quads, sets two draw areas, a slide draw offset, a solid tile and two more quad rows"),
    "0x80071C4C": ("gpu_UpdateRotatedEntryFadeAndFinalizeSelections",
                   "DrawRotatedEntriesAndOnFullCounterFinishWithResultOrNextStateAndStoreSelections",
                   "'Fade' is unshown here (D_800A3550 is a counter stepped by 8); when it is full the exit writes the result "
                   "D_800A35A0 = 1 or the next state D_800A3584 = 5 / 6 and phase D_800A3578 = 1 (5ED34.c:1413-1426), and the "
                   "end writes the selections to D_800A3568; no gpu_ prefix (it is not only a primitive builder)"),
    "0x80073200": ("gpu_DrawTintedMirroredQuadsAndAnimatedCellLayers", "gpu_DrawTintedFlippedQuadsFixedCellLayerAndFrameCycledLayer",
                   "the four gpu_AddScaledSpriteQuads calls take variants 0..3 ('Flipped' is the reviewer's reading of that "
                   "variant argument); the always-drawn header 5 layer is fixed (5ED34.c:1960) and only the optional second layer "
                   "(header 10 + unk_08 % 4, when D_800A3580 < 2) cycles"),
    "0x8005D46C": ("gpu_DrawFixedCellPairWithSelectedSheets", "gpu_DrawFixedSpritePairWithSelectedSheets",
                   "'CellPair' is false: the body draws nine cells over two sheet draws (D_8009B340[3] and D_8009B358[6], "
                   "4D0A8.c:323-353), so the pair is of sheet draws, not of cells"),
    "0x800600C8": ("gpu_DrawDecimalValueWithMagnitudeAlignedLabel", "gpu_DrawOneOrTwoDigitValueAndDigitCountShiftedSprite",
                   "'Label' is a guess: the body draws a fixed three-cell sprite (D_8009B758) whose x shifts with the digit "
                   "count (arg0 < 10), then one or two digits (4D0A8.c:1410-1452)"),
}

DROPPED = {}

# row notes: each adjustment against the research, with its reason (kept rows keep the default note)
for _a, (_old, _new, _why) in ADJUSTED.items():
    if _old != _new and _a not in NOTES:
        NOTES[_a] = f"adjusted from research {_old}: {_why}. Pending naming-reviewer pre-apply (round 1)."

WT = os.environ.get("P3WT", r"C:\Users\Trenton\Desktop\bb2-worktrees\ln-naming-b")

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
    raise SystemExit(f"no C definition of {fn}")


def T(addr):
    """Reach clause from coverage.tsv (verified at build time); no role rests on it."""
    h = _COV[addr.lower()]
    return (f"TRACE: not used; coverage.tsv lists the function in {len(h)} of {len(_CAPS)} registered captures "
            f"(docs/naming/runtime/matrix/coverage.tsv), and no part of the name rests on a runtime observation (Q132 mechanical name).")


def ch(addr):
    """CHANGED FROM RESEARCH clause for rows whose name differs from the research."""
    if addr not in ADJUSTED:
        return ""
    old, new, why = ADJUSTED[addr]
    if old == new:
        return ""
    return f"CHANGED FROM RESEARCH: {old}; {why}. "


def R(addr, new, fn, f, body, callers, why, counter, prior):
    ev = (f"{ch(addr)}BODY: {B(fn, f)}: {body}. CALLERS: {callers}. WHY: {why}. COUNTER-EVIDENCE: {counter}. "
          f"Prior manifests: no_name.tsv ({prior}). {T(addr)}")
    return (addr, new, "RENAME", ev)


FUNCS = [
    # ------------------------------------------------------------------------------------------ 17AFC.c
    R("0x8002738C", "fighter_AccumulateBoneGroupFlagWhenGameModeZero", "func_8002738C", "17AFC.c",
      "returns at once when D_800A38DC != 0; else switches on a1 (bones 0-5, 6-9, 10-13, 14-17, 18-21) and ORs 0x10 / 0x01 / "
      "0x02 / 0x04 / 0x08 into the byte at D_800A376A + a0; any other a1 does nothing",
      "17AFC.c:471 (fighter_ResolveBoneHit, `D_800A38DC == 0 && var_4 == 0` then func_8002738C(0, arg2)) and the asm of "
      "fighter_ResolveBoneHit; the bone grouping 0-5 / 6-9 / 10-13 / 14-17 / 18-21 is the one the sibling "
      "fighter_AccumulateOrClearByBoneGroup (17AFC.c) switches on",
      "the verb is the body: one flag bit per bone group, accumulated by OR into a byte; D_800A38DC is the game mode "
      "(symbol_addrs.txt: copied from D_80102778.unk_D, only compared)",
      "the only caller passes 0, D_800A376A is also read by 2B344.c:242-260 / 423 (passed on to slot_RequestTimGroupsAndMarkReloadOnChange), so no "
      "injury-mask or texture meaning is claimed",
      "w52,w63"),

    R("0x80027640", "fighter_RelocateAndStartEventMove", "func_80027640",
      "17AFC.c",
      "unk_34C counted to 0x40 (the blend weight 0x3C - 4 * (n - 1), min 10); target x / z from the start-position pair table "
      "(fight_GetStartPositionPairTable) or, when D_800A36A4 == 3, a blend toward a corner chosen from the opponent's world_pos (17AFC.c:202-215); adds the delta to world_pos, "
      "base_pos and collision_pos, zeroes unk_104 and unk_134, starts the move script found through "
      "cur_move->event_move_table_id and r1[0x1D] (motion_ResolveMoveData, fighter_StartMoveScript) and calls "
      "func_80032854(index, 0x30, &world_pos.x, 0)",
      "17AFC.c:400 (fighter_ResolveBoneHit: the opponent record when `unk_0C == 0xD && is_defeated == 0 && move_kind != 0x2E`) "
      "and the asm of fighter_ResolveBoneHit",
      "Relocate and Start are statements of the body; 'ToStartPosition' is left out because the D_800A36A4 == 3 path "
      "targets an opponent-derived corner, and the counter bump, velocity clears and event dispatch are left to the body",
      "the research's 'WithSound' (command 0x30 may also start an effect) and 'Restart' are not used",
      "w52")
    ,

    R("0x8002906C", "fight_ClearPositionRecordUsage", "func_8002906C", "17AFC.c",
      "walks the PosRec list from fight_GetPositionRecordList() until `type == 0` and stores used = 0 in each entry",
      "9F9C.c:1639 (fight_ResetGraphAndReloadResourcesOnCachedKeyChange) and its asm",
      "the list is the one fight_ProcessPositionRecordsInQuad (17AFC.c:1001-1041) tests and marks `used = 1` in; this "
      "is its reset",
      "no sound / geometry role of the records is claimed",
      "w52,w63,w71"),

    R("0x8002BC68", "fighter_SetBasePositionSeparationCorrectionsAndGetDistance", "func_8002BC68", "17AFC.c",
      "x / z distance between D_80101EC8[0] and [1] base_pos through the sqrt LUT; with arg0 the target spacing it stores "
      "the opposite unk_134 vx / vz offsets (+/- (dx, dz) * var_a0 / (arg0 - 100)) in both records and returns the distance",
      "17AFC.c:2087 (func_8002C0DC, `D_800A371C` as arg0) and the asm of func_8002C0DC",
      "the sibling fighter_CalcSeparationErrorAndOpposedOffsets works on the stored positions (named_syms.txt:1710); "
      "this one on base_pos and a caller-supplied spacing, and also returns the distance",
      "a distance-only name would omit the two unk_134 writes; no repel / attack meaning is claimed",
      "w52,w63,w71"),

    R("0x8002C0DC", "fight_UpdateSpacingTurnFacingWeaponBodyHitsAndQueueMoveEvents", "func_8002C0DC", "17AFC.c",
      "calls func_8002BC68(D_800A371C); for both fighters fighter_SetTurnDelta(e, ratan2(dx, dz)) toward the other base_pos; "
      "when D_800A376E == 0, D_800A3758 == 0xFF and unk_AA == move_frame calls fighter_ProcessWeaponBodyHits(1); then, "
      "while move_frame is in [D_800A38E8, unk_AA) and the distance exceeds D_800A371C + 0xC8, stores pending_move_event "
      "4 and 5 in the two fighters",
      "17AFC.c:2232 (fight_UpdateFighterInteractionsAndPointCaches) and its asm",
      "each clause is a statement of the body",
      "no gameplay identity (clash, grapple) is claimed",
      "w52,w63"),

    R("0x8002FC80", "math_TriangleNormalYawWithPositiveYHalfTurn", "func_8002FC80", "17AFC.c",
      "(a1 - a0) x (a2 - a0) through GTE op0 into SPAD->unk2B8.rec.unkC8, returns ratan2(x, z) plus 0x800 when y > 0",
      "17AFC.c:1044 (fight_ProcessPositionRecordsInQuad: the result is func_80044B30's second argument) and its asm",
      "a pure function of three vectors; 0x800 is a half turn of the 4096-unit angle (ratan2)",
      "the rotating-entry use by the caller is not part of the name",
      "w52,w63,w71"),

    R("0x8003032C", "math_ReflectAndDampRecordVector", "func_8003032C", "17AFC.c",
      "angle = ratan2(a1[0], a1[2]); rotates the (a0[0x11], a0[0x13]) pair into that frame, negates the perpendicular "
      "component, rotates back with >> 15 (an 1/8 scale of the rotation) and stores a0[0x12] = a0[0x12] / 4",
      "none (no C call, no jal, no address word)",
      "the arithmetic is a reflection about the a1 direction of the x / z words 0x11 and 0x13 with both damped, "
      "the y word 0x12 divided by 4",
      "the function is uncalled so no use disagrees; the record and the vector's role (velocity) are not claimed",
      "w52,w63,w71"),

    R("0x80030D04", "object_ClearFixedKindRange", "func_80030D04", "17AFC.c",
      "for the 12 entries of g_object_pool sets kind = -1 where 0x12 <= kind < 0x1E",
      "9F9C.c:1200 and the asm of func_8001C51C",
      "g_object_pool is the thrown-object pool (object_ResetPool, object_UpdatePool); -1 is the empty kind "
      "(17AFC.c:3706, 3802, 3964)",
      "no kind meaning is claimed; the bounds are not in the name",
      "w52,w63,w71"),

    R("0x80032064", "fight_CreateBallisticRecordAndDispatchEvent", "func_80032064", "17AFC.c",
      "finds the first free of the four D_80104E88 records (unk_00 == 0), stores type, owner index, a position and a "
      "velocity (Judge * 0x50 >> 12 in x / z, -0xC8 in y), then func_80032854(unk_B2, type == 1 ? 0xC : 0xD, ...) and "
      "returns the record, or 0 when all four are in use",
      "9F9C.c:5218 and the asm of fighter_Update",
      "fight_ClearBallisticRecords, fight_UpdateBallisticRecords and "
      "fight_TriggerOpposingFighterEventFromNearBallisticRecords (17AFC.c) are the rest of this pool; this is its creator",
      "no projectile species is claimed",
      "w52,w63"),

    R("0x80032854", "fight_RecordReplayEventAndDispatchSoundOrEffect", "func_80032854", "17AFC.c",
      "replay_RecordEvent(arg0..arg3); then switches on arg1: eff_Start* calls only (1 / 2 / 3 / 4 / 9 / 10 / 15), "
      "snd_QueuePositionalSound only (17 / 33..50), or both (5..8, 11..14, 18), with D_800A3910 set from the 5..8 effect "
      "returns",
      "about 100 call sites: 17AFC.c fighter / object code (arg0 is a fighter index or `kind ^ D_800A36F2[0]`), "
      "175A4.c, 28708.c, 9F9C.c, d_7D920.c data (D_8008EB54 row arg1) and the asm of replay_PlayFrame and the fighter_* / "
      "object_* resolvers listed under HOMES",
      "the verb pair is the body: the first statement records the event, the switch starts a sound, an effect or both",
      "arg0 is sometimes a fighter index and sometimes a kind comparison, so no fighter_ / snd_ / eff_ prefix",
      "w63"),

    R("0x80033898", "ResetGraphClearFrameCounterAndSetMainLoopHandler3", "func_80033898", "17AFC.c",
      "gpu_ResetGraphMode1(); D_800A37B8 = 0; D_800A3834 = 3",
      "d_7D870.c:32 (the .word of the D_8008D090 handler table, its only reference)",
      "D_8008D090 is the handler table `main` calls through D_800A3834 (symbol_addrs.txt:804, 1934); handler 3 is the "
      "D_80102778 settings editor (not named); D_800A37B8 is a frame counter (symbol_addrs.txt:715)",
      "no fight_ prefix: the body touches no fight data, and the table neighbours fight_Init / fight_Update do not make it one; "
      "the destination handler's role is not claimed",
      "w52,w63,w71")
    ,

    R("0x80033DF4", "fight_AdvanceStepTableRowOrInsertBestTimeAndSetFlags", "func_80033DF4", "17AFC.c",
      "step D_800A38E2: at 100 it sets D_800A36F0 / D_800A3781 and ORs a mask into D_80106A50.unk_00 under two conditions, "
      "calls fight_InsertBestTime, sets D_800A3834 = 4 and returns 0; else stores step + 1 and copies row [step] of "
      "D_8008E778 (and the D_8008EC24 / D_8008E908 lookups) into D_800A384C, D_800A38DE, D_800A38EC..EE and "
      "D_80102778.unk_4[1], returns 1",
      "24BF0.c:72, 9F9C.c:1327 (`if (func_80033DF4() == 0)`), fight_UpdateDefeatDelayAndModeProgress asm, func_800344B4 asm",
      "all statements of the body; symbol_addrs.txt:806, 821-822, 1936-1941 describe the same globals",
      "the research's 'PresetOpponent' and 'Completion' are readings and are not used",
      "w52,w63")
    ,

    R("0x80033FE4", "fight_SetSelectorAndMainLoopHandlerFromFlagsAndBestTimeRank", "func_80033FE4", "17AFC.c",
      "if D_800A36F0: D_800A38A4 = 6 (7 when D_8008D9EC[D_80101EC8[0].unk_0A]), D_800A3834 = 0x12; else if D_800A3781: "
      "D_800A38A4 = 8 / 9, D_800A3834 = 0x12; else D_800A3834 = 0x1A when D_800A38E9 < 3, else 8",
      "2B344.c:874 (func_8003C8B4) and the asm of func_8003C8B4",
      "D_800A36F0 / D_800A3781 are the flags func_80033DF4 sets; D_800A38E9 is the best-time rank "
      "fight_InsertBestTime stored (symbol_addrs.txt:1937)",
      "the destination screens are not claimed",
      "w52,w63,w71"),

    # ------------------------------------------------------------------------------------------ 35000.c
    R("0x80044800", "node_AdvanceFrameAnimatedEntriesAndQueueDraws", "func_80044800", "35000.c",
      "for each of the D_800A9CF8.unk6 entries with unk58 >= 0: while the frame unk58 < unk2, reads six s16 from the "
      "key-frame array at unk8 + frame * 12 (position and rotation), updates the node world matrix "
      "(node_UpdateWorldMatrix), rotates the offset by the entry's Y angle, advances the frame (also the paired "
      "unk10 node when unk4 == 0x12); after the last frame lowers unk60 by 0x40 and ends at -2; every entry is "
      "pushed on g_draw_queue",
      "368E4.c:660 / 668 / 733 (three handler stubs func_8004748C / func_80047510 / func_8004779C) and their asm",
      "include/game.h:2037-2051 documents Unk800A9CF8Entry as a transform node run from key frames and queued on "
      "g_draw_queue",
      "no animated artwork is claimed",
      "w64")
    ,

    R("0x80044B30", "node_StartFrameAnimatedEntryAndInitPairedNode", "func_80044B30", "35000.c",
      "returns unless a0 < D_800A9CF8.unk6 and the entry's unk58 == -1; picks the Y angle (forced for unk4 == 4 / 0x12), "
      "stores it in unk5C, sets unk58 = 0, then initialises the paired unk10 node and rotation matrix "
      "(and work.t when unk4 == 0x12)",
      "17AFC.c:1043 (fight_ProcessPositionRecordsInQuad, with func_8002FC80's yaw) and the asm of "
      "fight_ProcessPositionRecordsInQuad",
      "same table as func_80044800: the frame counter -1 idle -> 0 started (include/game.h:2045-2046)",
      "no artwork is claimed",
      "w64")
    ,

    R("0x80044C70", "resource_RebaseFrameAnimatedEntryBlockAndPointers", "func_80044C70", "35000.c",
      "reloc_RebaseBlock(D_800A9CF8.unk0, a0); D_800A9CF8.unkC += a0; D_800A9CF8.unk8 += a0",
      "368E4.c:272 (resource_RebaseLoadedBlockSections, selectors 4 / 7 / 18) and its asm",
      "the caller is the resource block rebase and the three words are the block, entry table and key-frame array of "
      "the D_800A9CF8 header",
      "no resource identity is claimed",
      "w64")
    ,

    R("0x80044CCC", "math_BlendCylindricalPositionsWithNegatedYZ", "func_80044CCC", "35000.c",
      "turns two (height, angle, radius) s16 triples into x = sin * r >> 12, y = -height, z = -(cos * r >> 12) and "
      "LoadAverage12-blends them with weights 0x1000 - a2 and a2 into a3",
      "none (no C call, no jal, no address word)",
      "pure arithmetic; the function is uncalled, so no use disagrees",
      "the coordinate convention is stated only as the negations of y and z",
      "w64"),

    R("0x80044ED8", "ndata_ReadRemappedIndexPackFirstElseRead", "func_80044ED8", "35000.c",
      "a0 >= 0x1F: a0 -= 0x1B; if !ndata_ReadFromPack(a0, a1) then ndata_Read(a0, a1)",
      "35000.c:543 / 545 (func_80045878) and its asm",
      "both callees are the ndata_ readers; the remap and the fallback are statements of the body",
      "the bank meaning of the index is not claimed",
      "w64"),

    R("0x80044F50", "ndata_ReadIndexInRangeSelectedByFlag", "func_80044F50", "35000.c",
      "ndata_Read(a1 + 0x83, a2) when a0 == 0, else ndata_Read(a1 + 0x10C, a2)",
      "35000.c:635 (func_80045B68) and its asm",
      "the flag a0 picks one of two ndata index bases; the callee is the ndata_ reader",
      "the bases are not in the name; the selector's meaning is not claimed",
      "w64"),

    R("0x80045878", "slot_OpenObjectBlockFreeSoftbodyAndLoadOrReuseAssetBlock", "func_80045878", "35000.c",
      "arena block a0 found or opened (end set, peak updated, move callback func_80045AA4, unk_08 = -1, status 0); "
      "when block a0 + 3 exists softbody_FreeObject(object); reuses the asset block (status 0) when unk_08 == a1 and "
      "status != -2, else opens a0 + 3, reads asset a1 through func_80044ED8 into it (or into the supplied a2), sets "
      "status 1; stores the ids and returns the object",
      "309CC.c:142 (the D_800A9A10 slot loader) and the asm of slot_LoadObjectBuildNodesSoftbodyAndFogLevels",
      "the arena id is the D_800A9A10 slot index: func_80045AA4 sends a0 < 3 to func_80041430, which rewrites "
      "D_800A9A10[a0]; a0 + 3 is the asset block (35000.c:585-600)",
      "the slot table's vocabulary comes from pilot-tail-a (wave88)",
      "w64")
    ,

    R("0x80045A28", "slot_ResizeAssetBlockAndUpdatePeak", "func_80045A28", "35000.c",
      "arena_Resize(a0 + 3, a1); arena_UpdatePeak(0)",
      "309CC.c:270 and 2B344.c:2460 (both pass the object's arena_id)",
      "arena a0 + 3 is the asset block of slot a0 (func_80045878)",
      "the object record itself is not resized",
      "w64"),

    R("0x80045A50", "slot_StopAllVoicesCloseVabAndFreeObjectAndAssetBlocks", "func_80045A50", "35000.c",
      "snd_StopAllVoicesAndCloseSlotVab(a0) (4BC3C.c:101-108: func_800858D0(0); SsVabClose(3 * a0 + 1); clears g_vab_rec_ptr / g_vab_vb_sbaddr); "
      "arena_ClearMoveCallback and arena_Free for blocks a0 + 3 and a0",
      "31D3C.c:34 (slot_ReleaseObjectAndResources: after softbody_FreeObjectById, before D_800A9A10[a0] = 0) and its asm",
      "the callee stops all voices and closes the slot's VAB; both blocks are freed",
      "the slot pointer is cleared by the caller, not here",
      "w64"),

    R("0x80045AA4", "slot_RebaseObjectOrAssetBlockAndFakeOpenVab", "func_80045AA4", "35000.c",
      "a0 < 3: func_80041430(a0, a1); else reloc_RebaseBlock for blocks a0 - 3 and a0, unk_1C += a1, "
      "softbody_RebaseObject, and when bit 1 of unk_00 is set snd_VabFakeOpen(a1, 3 * arena_id + 1)",
      "35000.c:527, 550 (arena_SetMoveCallback in func_80045878); never called directly",
      "an arena move callback for the paired blocks; the two paths are the two statements groups",
      "the callback id is either block of the pair",
      "w64"),

    R("0x80045B68", "resource_LoadSelectedEntriesTexturesAndSound", "func_80045B68", "35000.c",
      "opens arena block 6, reads a pack through func_80044F50, collects the texture groups named by the entry list "
      "arg2 (D_800993FC), uploads them with gpu_LoadTimGroupSplitOffset, copies or keeps the selected entries "
      "(reloc_CopySelectedEntries / reloc_KeepSelectedEntries), relocates, offsets the texture primitives and loads the "
      "VAB sections with snd_CopySelectedPackSectionsAndLoadVabPacks; sets the end and move callback func_80046048",
      "368E4.c:1884 (resource_... call with D_800A33EC) and its asm",
      "each step is a statement of the body",
      "no weapon / stage identity of the loaded entries is claimed",
      "w64")
    ,

    R("0x80046020", "resource_FreeSelectedEntryBlockStopAllVoicesAndCloseVabs", "func_80046020", "35000.c",
      "arena_Free(6); snd_StopAllVoicesAndCloseSelectedEntryVabs() (4BC3C.c:110-118: func_800858D0(0); SsVabClose(2) and SsVabClose(5) with the g_vab tables cleared)",
      "368E4.c:411 and 1883 (the two callers around func_80045B68) and the asm of both",
      "block 6 is func_80045B68's block; this is its release, and the callee stops all voices before closing the VABs",
      "VAB numbers 2 and 5 are named only as the callee's arguments",
      "w64"),

    R("0x80046048", "resource_RebaseSelectedEntryBlockAndFakeOpenListedVabs", "func_80046048", "35000.c",
      "reloc_RebaseBlock(6, a1); reads the count-prefixed id list at the block's first offset and calls "
      "snd_VabFakeOpen(a1, id) for each",
      "35000.c:751 (arena_SetMoveCallback(6, func_80046048) in func_80045B68); never called directly",
      "the move callback of block 6; the list is the one func_80045B68 wrote with the VAB pack loader",
      "the ids' meaning is not claimed",
      "w64")
    ,

    # ------------------------------------------------------------------------------------------ 4D0A8.c
    R("0x8005C8A8", "gpu_DrawModeSizedPanelWithSelectedCellOpaque", "func_8005C8A8", "4D0A8.c",
      "for mode 0 / 1 / 2 builds sprite cells (gpu_AddSpriteCells) and border TILEs sized by D_8009B2BC[mode].w / .h, "
      "keeps the cell matching the selection (arg1) opaque and the others semi-transparent, adds the DR_MODE and "
      "returns the size",
      "2B344.c:902 (func_8003C9A4), 6CF8.c:340 / 343 (menu_RunPauseMenu), 9F9C.c:2141 (the three-item fighter menu) and their asm",
      "the body is the panel builder; the selection argument is the one the callers keep at D_800A3817 / var_2",
      "'Choice' and 'Highlight' are not claimed; no pad is read here",
      "w60")
    ,

    R("0x8005D46C", "gpu_DrawFixedSpritePairWithSelectedSheets", "func_8005D46C", "4D0A8.c",
      "two gpu_AddScaledSpriteQuads calls over the fixed tables D_8009B340[3] / D_8009B358[6] (nine cells in all, 4D0A8.c:323-353) with the headers "
      "D_8009B2C8[idx][0] / [1], idx = arg1 - 1 when arg1 > 0",
      "6CF8.c:546 (gpu_DrawDisplayModeOnLocalOtAndSync) and its asm",
      "the body draws two fixed sheet draws whose sheets are picked by the index",
      "no artwork identity is claimed; 'CellPair' is not used (nine cells, two draws)",
      "w60,w74,w76"),

    R("0x8005D554", "gpu_DrawJitteredQuadPairsAndAdvanceRepeatCycle", "func_8005D554", "4D0A8.c",
      "D_800A326C %= 4; one random base point; (D_800A326C + 1) * 2 repeats of two gpu_AddScaledSpriteQuads draws "
      "at base +/- a random jitter; D_800A326C += 1; returns the cursor",
      "6CF8.c:551 and 554 (gpu_DrawDisplayModeOnLocalOtAndSync) and its asm",
      "the repeat count is the cycle counter the function itself advances; one base point per call",
      "no effect species (spark, smoke) is claimed",
      "w60"),

    R("0x8005D814", "gpu_DrawTimeFieldsAndDecimalCounter", "func_8005D814", "4D0A8.c",
      "draws three two-digit fields from arg0->unk_0 / unk_2 / unk_3, a one-to-three-digit counter from arg1 - 1 "
      "with leading zeros suppressed, two red rules and sprite-cell pairs",
      "9F9C.c:1461 and 2137 (fight_UpdateModeDisplaysAndMatchProgress, the three-item fighter menu) and their asm",
      "arg0 is the buffer filled by fight_SplitFrameCounterIntoMinutesSecondsHundredths (9F9C.c:1460)",
      "no clock or rank panel identity is claimed",
      "w60,w74,w76"),

    R("0x8005E098", "gpu_DrawTwoDigitCounterWithOptionalSecondSide", "func_8005E098", "4D0A8.c",
      "arg1 is drawn at the right as two digits (x 0x209) and, when arg0 >= 0, arg0 at the left (x 0x50); arg0 < 0 "
      "draws one side and one rule only",
      "4D0A8.c:735 (func_8005E51C) and 9F9C.c:1464 (D_800A37D2, D_800A37D3)",
      "the loops break after one pass when arg0 < 0",
      "the counters' meanings are not claimed",
      "w60,w74,w76"),

    R("0x8005E51C", "gpu_DrawDecrementedSingleSideCounter", "func_8005E51C", "4D0A8.c",
      "return func_8005E098(-1, a0 - 1, a1, a2)",
      "9F9C.c:1450 (D_800A3783)",
      "a forwarder with the sentinel -1 and the value minus one",
      "the displayed quantity is not claimed",
      "w60,w74,w76"),

    R("0x8005E54C", "gpu_DrawPackedComparisonRowsAndTotals", "func_8005E54C", "4D0A8.c",
      "per round (D_8009BD24.unk14_10 + 3) the two 2-bit values of arg0 are compared and drawn with mark cells, then "
      "the sums, red rules and the DR_MODE",
      "2B344.c:804 (fight_UpdateMatchTallyPresentation) and 6CF8.c:558 (gpu_DrawDisplayModeOnLocalOtAndSync)",
      "each clause is a statement of the body",
      "no team / win-score meaning of the values is claimed",
      "w60,w74,w76"),

    R("0x8005F1C8", "gpu_DrawPairedMarkRowsAndTimeFields", "func_8005F1C8", "4D0A8.c",
      "two mark rows per side counted from the nibbles of arg1, then time digits from arg0->unk_2 / unk_3 with red rules",
      "9F9C.c:1614 (arg0 is the split counter with unk_2 = seconds, unk_3 = hundredths, 9F9C.c:1608-1612)",
      "the marks are cell choices by k >= nibble; the digits are the buffer's fields",
      "the marks' scoring meaning is not claimed",
      "w60,w74,w76"),

    R("0x8005FA98", "gpu_DrawSelectedStretchedSpritePair", "func_8005FA98", "4D0A8.c",
      "two gpu_AddScaledSpriteQuads draws with scale_x 0x200 (headers D_8009B63C[arg0], D_8009B610[arg0]); returns 0x190",
      "2B344.c:928, 9F9C.c:1491, 1517, 2173",
      "arg0 selects the sheets and the table slot (0 / 2 / 3)",
      "no message artwork is claimed",
      "w60,w74,w76"),

    R("0x8005FBC8", "gpu_ReadCdFileUploadTwoRegionsAndResetSplitWipe", "func_8005FBC8", "4D0A8.c",
      "cdrom_GetFileIndex(2, arg0 + 0x33), cdrom_StartRead into arg1, cdrom_WaitIdle, two LoadImage uploads "
      "(arg1 + 0x40 and + 0x14) and D_800A3278 = 0",
      "2B344.c:700 (func_8003C040) and its asm",
      "D_800A3278 is the counter gpu_DrawImageAndAdvanceSplitWipe (4D0A8.c:1293) reads and advances",
      "the file's identity is not claimed",
      "w60,w74"),

    R("0x800600C8", "gpu_DrawOneOrTwoDigitValueAndDigitCountShiftedSprite", "func_800600C8", "4D0A8.c",
      "a fixed three-cell sprite (D_8009B758) at x 0x93 (arg0 < 10) or 0xA3, then one or two digits of arg0 at x positions that depend on "
      "arg0 < 10, and the DR_MODE",
      "2B344.c:983 (func_8003CD10) and its asm",
      "the x offsets of the sprite and the digits switch at 10 (the digit count)",
      "the quantity and any label role of the sprite are not claimed",
      "w60"),

    R("0x800602AC", "gpu_ReadCdFileRelocateTwoImagesAndUploadFourRegions", "func_800602AC", "4D0A8.c",
      "reads CD file (2, arg0 + 0x3D) into arg1, relocates its first two words, and makes four LoadImage uploads "
      "from the two images",
      "2B344.c:159 (func_8003AE5C) and its asm",
      "each step is a statement of the body",
      "the file's identity is not claimed",
      "w60,w74,w76"),

    R("0x80060414", "gpu_DrawSpriteAtPackedTablePosition", "func_80060414", "4D0A8.c",
      "x = (D_8009B850[arg0 & 0x7FFF] >> 7) + 0x37, y = (& 0x7F) + 0x2A; header D_8009B7AC[0] when arg0 has bit 0x8000, "
      "else [1] or [2] by D_8009BD24.unk00[0][0].bd0 < 0xC; one gpu_AddSpriteCells and the DR_MODE",
      "6CF8.c:584 (gpu_DrawDisplayModeOnLocalOtAndSync) and its asm",
      "the position is unpacked from the table word and the header is selected by the flag",
      "no map / route meaning is claimed",
      "w60,w74,w76"),

    R("0x80060544", "gpu_DrawFixedSpriteCellsStretchedQuadAndRedTintedCells", "func_80060544", "4D0A8.c",
      "four fixed headers D_8009B770[i] (three as sprite cells, one as a stretched quad), D_8009B7A0, then two cells "
      "of D_8009B398[2..3] with col 0xFF / 0x10 / 0x10, and the DR_MODE",
      "2B344.c:436 (func_8003B8E4) and its asm",
      "each draw is a statement of the body",
      "no screen label is claimed",
      "w60,w74,w76"),

    R("0x80060758", "gpu_ResetGrowingBarProgress", "func_80060758", "4D0A8.c",
      "D_800A32B6 = 0; D_800A32B4 = 0",
      "2B344.c:834 (func_8003C714)",
      "func_80060768 (4D0A8.c:1613) increments both up to 0x1E and scales them into bar widths; nothing else reads them",
      "the rows the bars belong to are not claimed",
      "none"),

    R("0x80060768", "gpu_DrawIndexedCellsDigitPairsAndGrowingBars", "func_80060768", "4D0A8.c",
      "calls gpu_DrawIndexedCellsAndDecimalDigitPairs(..., arg1, arg2), then (arg2 < 3 for the first) up to two bar TILEs "
      "whose width grows with D_800A32B4 / D_800A32B6 (clamped to 0x1E) and three fixed semi-transparent TILEs",
      "2B344.c:872 (func_8003C8B4) and its asm",
      "the body is the callee call plus the bar tiles",
      "no recorded-time meaning is claimed",
      "w60,w74,w76"),

    # ------------------------------------------------------------------------------------------ 5ED34.c
    R("0x8006E534", "InitDisplayStateLoadPackAndSetEntryFlags", "func_8006E534", "5ED34.c",
      "ClearOTagR; stores its arguments and the mode D_800A35BC; calls resource_FreeSlotsAndBlocksAndInvalidateEntryCache (5ED34.c:61-145); reads one of packs 5 / 3 / 4 by the mode "
      "(func_8006E950), func_8006EA28, gpu_InitPrimBufferPair; zeroes the slide / state words; sets the D_8009BC7C "
      "per-entry bits 1 and 4 from arg3 and the cursor entries; MoveImage; returns 1",
      "64FD8.c:1303 (InitWithCachedValueAndStoredBitsBlackBg) and its asm",
      "each step is a statement of the body",
      "no prefix: the subsystem spans choice and timed phases (no_name.tsv w56); no replay_ / menu_ claim",
      "w56")
    ,

    R("0x8006E8AC", "gpu_GetIndexedPrimBufferRecord", "func_8006E8AC", "5ED34.c",
      "return &D_800A35AC[a0]",
      "5ED34.c:228 (func_8006EACC, a0 = (unk_0C + 1) & 1)",
      "D_800A35AC is the buffer pair gpu_InitPrimBufferPair fills (5ED34.c:100); every returned field feeds the frame record",
      "no screen claim",
      "none"),

    R("0x8006E950", "gpu_ReadCdFileRelocateAndUploadImagesAndBottomStrip", "func_8006E950", "5ED34.c",
      "cdrom read of file (2, a0) into a1, reloc_RelocateSelfOffsets, two LoadImage uploads from unk_08, then "
      "gpu_LoadVramBottomStrip(a1)",
      "596CC.c:80, 5ED34.c:80-95 (func_8006E534), 64FD8.c:1089 / 1604 / 1705",
      "each step is a statement of the body",
      "the pack identity (a0 = 2..6, 0x32, 0x5F) is not claimed",
      "none"),

    R("0x8006EA28", "resource_RelocateNineOffsetListsAndLoadVabPack", "func_8006EA28", "5ED34.c",
      "nine reloc_RelocateOffsetList calls over the words unk_54..unk_74, then snd_LoadVabPack(unk_00.unk_00, 1, "
      "unk_00.unk_04) and returns unk_00.unk_04",
      "5ED34.c:99 (func_8006E534) and its asm",
      "each statement is named by its callee",
      "the lists' roles are not claimed",
      "w56"),

    R("0x8006EACC", "StepSlideBuildFrameRecordAndDispatchStateHandler", "func_8006EACC", "5ED34.c",
      "sets D_800A35C0 = &g_gpu_db[D_800A36AC & 1] and D_800A3548 / 354C, calls func_8006EC0C, advances the D_800A35C4 "
      "counters, builds the Unk8006EACCRec from func_8006E8AC, calls func_8006F528 unless D_800A3580 - 2 < 2, "
      "calls D_8009BC1C[D_800A3580] and returns D_800A35A0",
      "64FD8.c:1309 (StepThenCloseVab1OnNonzero) and its asm",
      "each step is a statement of the body",
      "no prefix: the dispatcher covers interactive and timed handlers (no_name.tsv w56)",
      "w56"),

    R("0x8006EC0C", "StepSlideProgressCommitPendingStateAndQueueSound", "func_8006EC0C", "5ED34.c",
      "phases 1 / 3 add 0x20 to D_800A3570 up to 0x1E8, then D_800A3580 = D_800A3584 (and the phase advances when its "
      "high byte is 0); phases 2 / 4 queue sound 5 at 0x1E8, subtract 0x20 and clear the phase at 0",
      "5ED34.c:224 (func_8006EACC) and its asm",
      "D_800A3570 is consumed as an X displacement and clip width by func_8006F528 / func_8006ECF4; the dispatcher state D_800A3580 / D_800A3578 is written here",
      "no gpu_ prefix: no GPU call, primitive or OT write; the sound callee is snd_ and the caller func_8006EACC is unprefixed; the slide's visual identity is not claimed",
      "none"),

    R("0x8006ECF4", "gpu_UploadAndDrawSelectedEntryQuads", "func_8006ECF4", "5ED34.c",
      "for each active cursor entry draws a scaled quad set with a flag-dependent tint or semi-transparency, "
      "LoadImage-uploading its image first when needed, then the remaining layers",
      "5ED34.c:415 (func_8006F100), 779 (menu_UpdateEntryGrid), 1061 (menu_UpdateRow) and their asm",
      "each step is a statement of the body",
      "no portrait / name artwork is claimed",
      "w56"),

    R("0x8006F100", "DrawRotatedEntryCountdownAndRequestState3OrDrawDarkenedSelection", "func_8006F100", "5ED34.c",
      "while D_800A3550 != 0 steps it by 0x10, gpu_AddScreenDarken, func_8006ECF4, gpu_AddGradientPanel, func_80073200 "
      "and returns; else per entry draws gpu_AddRotatedSpriteQuads at a position offset by D_800A35C8[i] * 800 / 20 "
      "(664 in y), counts it down with sound cues 7 and 8, bumps D_800A355C and stores the pending state D_800A3584 = 3 once it is >= 0x79 (5ED34.c:419)",
      "no direct call; slot 2 of the D_8009BC1C per-mode update handler table (d_8C41C.c:33-36), called by func_8006EACC as D_8009BC1C[D_800A3580]",
      "each branch is a statement of the body",
      "no pad is read; no gpu_ prefix (the handler also writes dispatcher state and queues sounds); no screen subject is claimed",
      "w56"),

    R("0x8006F528", "gpu_DrawMirroredQuadsWithSlideDrawAreaAndOffset", "func_8006F528", "5ED34.c",
      "scaled quads from the D_800A35A8->unk_5C headers at x = slide or 0x200 - slide (mirrored for the second "
      "set), SetDrawArea with the slide width, SetDrawOffset by the phase, a solid TILE, two more quad rows",
      "5ED34.c:240 (func_8006EACC) and its asm",
      "each draw is a statement of the body",
      "no background or per-frame-record identity is claimed",
      "w56"),

    R("0x80071C4C", "DrawRotatedEntriesAndOnFullCounterFinishWithResultOrNextStateAndStoreSelections", "func_80071C4C", "5ED34.c",
      "per entry (unk0 not 5 / 16) draws gpu_AddRotatedSpriteQuads; D_800A3550 += 8; at 0xFF sets phase 1, "
      "queues sound 6, then either stores func_80071C20() and the result D_800A35A0 = 1 or sets the pending state D_800A3584 = 5 / 6 by mode (5ED34.c:1413-1426), and copies the selections "
      "D_800A3560.rec[i] to D_800A3568->unk00[i]",
      "no direct call; slot 3 of the D_8009BC1C per-mode update handler table (d_8C41C.c:33-36), called by func_8006EACC",
      "each branch is a statement of the body",
      "no pad is read; no gpu_ prefix (the exit writes result and state); the result/property of func_80071C20 is not claimed",
      "w56"),

    R("0x80072FCC", "gpu_AddBrownOrBlackPanelGridTile", "func_80072FCC", "5ED34.c",
      "SetTile; colour (0x46, 0x24, 0x0A) semi-transparent when D_800A3580 < 4, else black opaque; AddPrim at OT 0x17; "
      "returns the next TILE",
      "5ED34.c:1889 / 1896 (gpu_AddPanelGrid) and its asm",
      "arg0 is ignored; the colours are the literals",
      "the sibling gpu_AddPanelGridTile's colours differ",
      "none"),

    R("0x80073200", "gpu_DrawTintedFlippedQuadsFixedCellLayerAndFrameCycledLayer", "func_80073200", "5ED34.c",
      "a DR_MODE, four tinted gpu_AddScaledSpriteQuads variants 0..3 when D_800A3580 < 4, a fixed cell header 5 sprite layer "
      "(5ED34.c:1960) and, when D_800A3580 < 2, a second semi-transparent layer from header 10 + unk_08 % 4 that cycles",
      "5ED34.c:417 (func_8006F100), 783, 1063, 1690 (menu_UpdateChoiceGrid) and their asm",
      "each draw is a statement of the body",
      "no subject (logo, header) is claimed",
      "w56"),
]

DATA = []
