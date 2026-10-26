"""wave90 rows (pilot-tail-b): Q132 mechanical names for 43 of the 44 neutral functions in the tail TUs main/43500 (the
collision segment wrappers and per-cell testers, the normal-colour stream, the motion-playback wrappers), main/25C38 (the
row-menu phase handlers), main/64FD8 (two menu helpers), main/28708 (the memory-card step helpers, the shared-buffer
getter and the point-pair probe), main/4510C (the playback frame wrapper and the CPU helpers), main/31548 (the slot-object
pose / patch / rebase helpers), main/87A0 (the saved button-mapping helpers), main/25788 (the working / saved record
transfers), main/63D2C (two sheet renderers), main/24BF0 (the pending-config apply pair), main/24F08 (the config editor) and
main/175A4 (the paired-fighter update).
Research input: tmp/codex/pilot-tail-b-research.md (43 RENAME, 1 NO_NAME over 44 targets; untrusted proposals). All 44
addresses were still neutral (tier AUTO) in docs/naming/function-names.csv at HEAD cfc74aba4, so none was skipped as
already named.
Every row was re-derived from the C body (or the asm for the INCLUDE_ASM body) and every caller (src/main/*.c, include/,
asm/funcs/*.s, asm/data, named_syms.txt, symbol_addrs.txt); names changed against the research say CHANGED FROM RESEARCH
in the evidence and in the row note (33 of the 43 differ; 10 are kept as researched).
Research NO_NAME (1; stays neutral, not a row): 800545F4. It is not a function: named_syms.txt:1120-1123 and 31548.c:261-278
show it is two lui/ori instruction words (with D_800545F8 / D_800545FC / D_80054600) read as data by func_80041398, which
copies them with new immediates over the eight code sites of D_80015820; nothing calls it and nothing executes it in place.
The old no_name.tsv reason (w60: 'C treats the symbol as a word ... function identity is not established') still holds
and no function verb would be true; the problem is data classification (a later data wave), not an unknown identity.
Research RENAMEs dropped by this draft: none (all 43 kept; 10 as researched, 33 reworded, see ADJUSTED).
Old NO_NAME reasons of the 43 revived addresses (w60 / w64 / w73 / w78; each answered in its row's 'Prior manifests'):
the Q132 rule applies (name what the body does, worded to tell it from its siblings); the mechanics each old reason found
missing are now explicit in the name or named by callees wave84-wave88 landed (slot_ / node_ / resource_ / gpu_ names).
Prefix evidence: collision_ is 43500.c's own family (collision_WalkSegmentGrid, collision_TestSegment,
collision_TestSegmentWithWork, collision_GetHit, collision_SetGrid) and its callees; gte_ follows the body (the whole
function is cop2 asm: gte_MulMatrix0ClearTrans, nct, swc2 of the colour FIFO; same prefix as the TU's gte_SumSquares3);
motion_ is motion_StartPlayback / motion_UpdatePlayback / motion_FreePlaybackBlock and the callees of both wrappers;
menu_ is the callee vocabulary of 25C38.c's handlers (menu_SetEnabledRows, menu_UpdateRowMenuStoreResult,
menu_InitSelWithStoredBits, menu_ResetFlowBackgroundAndConfirmCursor, InitRowMenuBlackBgResetFlow) and of the
SELWORK / menu_RunSelPhases renderers; cdrom_ is the callee family of 800355E8 (cdrom_StartAudioEntry,
cdrom_SetAudioRepeat); memcard_ is the callee family of the three card step helpers (memcard_RequestLoad,
memcard_RequestSave, memcard_RequestOverwrite, memcard_RequestFormat, memcard_Update); fighter_ is the family of
4510C.c's CPU helpers (fighter_StepCpuPadScript, fighter_BuildCpuPad, fighter_StartCpuPadScript) and of wave84's
fighter_TestDownwardProbeFromCollisionPosHitsUpwardFacingSurface, the precedent for a probe taking a fighter record;
fight_ is the caller family of 80026DA4 (fight_UpdateFighterInteractionsAndPointCaches) and its fighter callees; slot_
and node_ are wave88's vocabulary of the table D_800A9A10[3] (first argument = slot index) and the transform nodes it holds;
pad_ is 87A0.c's own (pad_ResetState, pad_ClearStateBits) and the consumer of the three bytes is the pad transform in 9F9C.c;
gpu_ follows the primitive construction callees (gpu_AddSpriteCells, gpu_SetPolyF4RgbFromBlockFlag, gpu_GetSheetTPage);
node_ for 8005508C follows wave86's node_InitRootNodePairWithNegatedRotationMatrices, which initialises D_80101DF0.
Names with no prefix (Apply..., Copy..., Get..., Draw..., Patch...) are used where the code gives no single subsystem
(the record transfers touch the saved and the working record and a pending config; the getters return one global); the
working / saved / pending vocabulary is stated in _WORK below.
Unprefixed vs prefixed was decided per body; the research's data suggestions (g_working_selection_options,
g_pending_fight_config, g_cpu_profile_rules ...) need separate all-access data manifests and are not part of this wave
(DATA is empty).
"""
import csv
import glob
import os
import re

VERDICT = "CONFIRM"
VERIFIER_NOTE = ("naming-reviewer pre-apply (round 1): 40/43 PASS as proposed; 3 FAIL (80035618, 80040D48, 80041398) "
                 "replaced by the reviewer-suggested names (NOTES); none dropped.")
NOTES = {
    "0x80035618": ("suggested: naming-reviewer pre-apply round 1 failed menu_InitSubflowForRow ('ForRow' false at 25C38.c:205, "
                   "where the switched value is the mode cell D_800A38DC that func_800343F0 copies from D_80102778.unk_D; it "
                   "equals the row only through the identity table on the 25C38.c:270 path; case 6 is the comb_Connect link "
                   "mode) and suggested menu_InitSubflowForMode; R6 must judge it fresh at every use."),
    "0x80040D48": ("suggested: naming-reviewer pre-apply round 1 failed slot_ApplyPoseUpdateMatricesQueueNodesSoftbodyAndLightTarget "
                   "('ApplyPose' false at 3 of 4 callers: 9F9C.c:4982, 28708.c:1408 and fighter_Update pass a NULL pose in mode 1, "
                   "where the per-node matrices come from func_80041188 called just before and the body only sets the root "
                   "transform) and suggested slot_SetRootTransformUpdateMatricesQueueNodesSoftbodyAndLightTarget; R6 must judge "
                   "it fresh at every use."),
    "0x80041398": ("suggested: naming-reviewer pre-apply round 1 failed PatchImmediateColorLoadsAtRegisteredCodeSites ('Registered' "
                   "unsupported: the eight sites are the const ROM table D_80015820 of code labels, d_5BF0.c:137-140, with no "
                   "registration step) and suggested PatchColorImmediatesAtListedCodeSites; R6 must judge it fresh at every use."),
}

# names changed against the research (research name -> name used); an entry with old == new is a kept row
ADJUSTED = {
    "0x80053304": ("collision_WalkSegmentExtendingShortStart", "collision_TestSegmentExtendingShortStart",
                   "Walk is the callee collision_WalkSegmentGrid's own verb; the TU's wrappers are TestSegment / "
                   "TestSegmentWithWork (same body shape, result read by collision_GetHit), and this one differs by the start "
                   "extension; the discarded return is not named (research claimed it by 'Walk')"),
    "0x8005344C": ("collision_TestSegmentExtendingShortStartWithWork", "collision_TestSegmentExtendingShortStartWithWork", ""),
    "0x80053754": ("collision_TestGridCellUsingRoundedScalarProjection", "collision_TestCellKeepingNearestHitScalarProjection",
                   "both testers walk a cell's face records and keep the nearest hit (the same tail); what differs is the "
                   "projection into the face plane (plain integer products here, GTE transform in 80053E9C); the rounding is "
                   "a detail, the kept-nearest-hit effect was missing"),
    "0x80053E9C": ("collision_TestGridCellUsingGteProjection", "collision_TestCellKeepingNearestHitGteProjection",
                   "same reasoning as 80053754 (the nearest-hit write is the shared effect, GTE vs scalar projection the "
                   "difference); both testers also call gte_SumSquares3, so 'Gte' names the projection only"),
    "0x80054440": ("gte_ColorGroupedNormalStreamWithSelectedMatrices", "gte_ColorGroupedNormalStreamWithSelectedMatrices", ""),
    "0x80054884": ("motion_StartPlaybackFromBaseTableAndRelativeEntry", "motion_StartPlaybackAtIndexedBasePlusOffset",
                   "the body is motion_StartPlayback(D_80099D54[a0] + a1 - const, ...): a base looked up by index plus an "
                   "offset; 'BaseTable' read as the table itself, and the constant is a literal"),
    "0x800548DC": ("motion_SyncRestoreResourceSectionsAndFreePlayback",
                   "motion_DrawSyncApplyHeaderSectionsAndFreePlaybackBlock",
                   "'Restore' is not shown: the callee (resource_SelectRetainedSubblockOrApplyHeaderSections(-1)) applies the "
                   "header sections via resource_ApplyHeaderSectionsResizeBlockAndClearRetainedSubblocks; DrawSync(0) is "
                   "named by its callee and motion_FreePlaybackBlock by its own name"),
    "0x80035438": ("menu_ResetDispatchCopySavedSettingsAndSetEnabledRows",
                   "menu_ResetSubflowCopySavedToWorkingAndSetEnabledRows",
                   "'Dispatch' / 'Settings' are not the code's words: D_800A3740 is the subflow state of 80035828's switch, "
                   "func_80035280 copies the saved record into the working record; vocabulary as the sibling names"),
    "0x80035480": ("menu_ResetResourcesConditionallyCloseLinkAndVabsAndInitRowFlow",
                   "menu_ReleaseResourcesConditionallyCloseLinkAndVabsAndInitRowMenu",
                   "slot_ReleaseObjectAndResources / motion_Invalidate... are releases, not resets; the callee "
                   "InitRowMenuBlackBgResetFlow names the row menu init"),
    "0x800355E8": ("cdrom_EnableSerialMixAndStartRepeatingAudio", "cdrom_EnableSerialMixAndStartRepeatingAudio", ""),
    "0x80035618": ("menu_EnterRowSelectedSubflow", "menu_InitSubflowForMode",
                   "the body switches on the mode value (D_800A38DC at 25C38.c:205) and initialises that mode's subflow (calls its init, sets D_800A3740); "
                   "'Entering' is the caller's act and 'Selected' is not shown for the D_800A38DC call"),
    "0x80035828": ("menu_UpdateRowSelectionCardAndTimedSubflows", "menu_UpdateRowMenuAndSubflows",
                   "the body also runs the link-connect, option, memcard and sprite-sequence subflows; the shorter name "
                   "covers all of them and still tells it from its sibling update 80035E38"),
    "0x80035DC8": ("menu_ResetResourcesCloseVabsAndInitRowFlow", "menu_ReleaseResourcesCloseVabsAndInitRowMenu",
                   "releases not resets; wording as 80035480, which it differs from by the unconditional VAB close and the "
                   "absent link close"),
    "0x80035E38": ("DrawGradientAndStepCardSaveFlow", "DrawGradientAndStepMemcardSavePromptsThenSelectRowMenuInit",
                   "the stepped callee is the memcard prompt machine (memcard_ family), and the body ends by selecting "
                   "handler 8 (D_800A3834 = 8), which is func_80035480 in d_7D870.c's table"),
    "0x80077860": ("DrawWhiteTileAndSheetsAndResetFlowOnPadMatch", "DrawWhiteTileAndSheetsAndResetFlowOnPadMatch", ""),
    "0x80077D00": ("GetWorkingSelectionOptionsAndTimeRecord", "GetWorkingRecord",
                   "the record is only shown as the working copy of the saved record (80035280 / 80034F88 transfer it), edited "
                   "by the menus and read by gpu_ / snd_ code for single bits; the three nouns of the research are each only "
                   "true for some fields"),
    "0x8003880C": ("memcard_StepInitialLoadAndDrawStatus", "memcard_StartLoadOnceStepAndDrawSheetPair",
                   "'Initial' and 'Status' are not shown; the body arms one load request with a latch, maps memcard_Update's "
                   "result to 0 / 1 / -1 and calls gpu_DrawSheetPairCenteredTileWithOptionalSelection"),
    "0x800388A8": ("menu_DrawAndHandleRearmedBinaryPrompt", "menu_StepTwoChoicePromptDrawSheetsAndRearmAfterResult",
                   "'Binary' read as a number; the body is a two-choice prompt (pad moves the choice, confirm returns 1 or -1 "
                   "by choice) whose latch clears the choice once a result was returned"),
    "0x80038988": ("memcard_RetryLoadDrawStatusAndWaitForDismissal",
                   "memcard_RetryLoadDrawStatusSheetsAndWaitForTimeoutOrPress",
                   "the drawn items are the status-selected sheet pairs and the dismissal is a countdown or a confirm press"),
    "0x80038C70": ("memcard_StepSaveOverwriteAndFormatPrompts", "memcard_StepSaveOverwriteAndFormatPrompts", ""),
    "0x800392B8": ("GetSharedSaveReplayDisplayAndMovieBuffer", "GetSharedBuffer",
                   "the body returns D_800F33D8; the buffer is shared (save block 28708.c:368-388, replay frames 28708.c:1104, "
                   "6CF8.c:539, the movie exec stack at +0x7FC in sys_ExecMovieOverlay), and the single caller needs no more"),
    "0x8003984C": ("collision_ProbeFighterPointPairMidpointTagAndClassify",
                   "fighter_ProbeVerticalCollisionAtPointPairMidpointForTagAndClass",
                   "prefix as wave84's fighter_TestDownwardProbeFromCollisionPosHitsUpwardFacingSurface (a probe built from a "
                   "fighter record); the segment is vertical (+-0x190 around the midpoint), which the research name omitted"),
    "0x80054F68": ("motion_ResetDrawQueueAdvanceCounterUpdatePlaybackAndFlush",
                   "motion_ResetDrawQueueAdvanceCounterSetOtUpdatePlaybackAndFlush",
                   "the body also stores the two OT pointers (g_gpu_ot_ptr, g_gpu_ot_ptr + 4), as 368E4.c:449-453 does"),
    "0x8005508C": ("node_GetViewMatrixTranslation", "node_GetRootNodeMatrixTranslation",
                   "the returned object is D_80101DF0, the first node node_InitRootNodePairWithNegatedRotationMatrices "
                   "(368E4.c:2190) initialises; 'View' is a role, 'root node' is wave86's own word"),
    "0x80056FE8": ("fighter_CalcOpponentMetadataAndTableDistanceThreshold", "fighter_CalcOpponentDistanceThresholdFromTables",
                   "shorter wording of the same shown mechanics (table terms plus the opponent's scale_distance_offset and a "
                   "constant, compared against the distance D_800A387C at 4510C.c:768-778)"),
    "0x80057094": ("fighter_BuildCpuRoutePadWithSurfaceProbe", "fighter_BuildCpuRoutePadWithSurfaceProbe", ""),
    "0x80058580": ("fighter_UpdateCpuNavigationChooseScriptsAndHeldInput",
                   "fighter_UpdateCpuNavigationChooseScriptsAndHeldInput", ""),
    "0x80040D48": ("slot_ApplyPoseUpdateSoftbodyQueueNodesAndSetLightTarget",
                   "slot_SetRootTransformUpdateMatricesQueueNodesSoftbodyAndLightTarget",
                   "the research omitted the node-matrix update loop; wording only otherwise"),
    "0x80041188": ("slot_BlendPoseRotationsAndTrailingTransforms", "slot_BlendTwoPosesIntoNodeMatrices",
                   "the two input buffers are the two poses; the output is each node's matrix (rotation lerp through the GTE "
                   "callee, and the translation of the two trailing nodes), so 'TrailingTransforms' is only a part"),
    "0x80041398": ("PatchImmediateColorLoadsAtRegisteredCodeSites", "PatchColorImmediatesAtListedCodeSites", "suggested by naming-reviewer pre-apply round 1: the sites are the const table D_80015820 (d_5BF0.c:137), nothing registers them"),
    "0x80041430": ("slot_RebaseObjectNodeLinksAndRefreshMatrixPointers",
                   "slot_RebaseObjectPointerNodeLinksAndRefreshMatrixPointers",
                   "the body first moves the slot's own pointer D_800A9A10[a0] by a1; the other effects as researched"),
    "0x8001945C": ("pad_InitSavedButtonMappings", "pad_SetDefaultSavedButtonMappings",
                   "wording: three constant bytes are stored in the saved record, which the pad transform treats as masks"),
    "0x80019488": ("pad_PackSavedLocalButtonMappings", "pad_PackSavedButtonMappingLowNibbles",
                   "'Local' is a link-mode role; the body packs the low nibble of each of the three saved bytes into one value"),
    "0x800194C0": ("pad_UnpackRemoteButtonMappings", "pad_UnpackButtonMappingNibblesToCells",
                   "'Remote' rests on D_800A36C6 whose writer is not in C; the body shows the three nibbles of the argument "
                   "stored in D_800A3912-14, the cells the pad transform reads when D_800A38DC == 6"),
    "0x80034F88": ("CopyWorkingOptionsAndButtonMappingsToSavedRecord",
                   "CopyWorkingFlagsAndButtonMappingsToSavedRecord",
                   "the stored bits are record flag bits, 'Options' is a role; wording"),
    "0x8003504C": ("ApplyWorkingSelectionsAndInitializeFightConfig",
                   "ApplyWorkingRecordToPendingConfigAndInitializeMode",
                   "the writer target is D_80102778 (the config 80034708 edits and 800343F0 copies to live cells) and the "
                   "tail call func_800344B4 initialises per mode (switch on D_800A38DC)"),
    "0x80035280": ("CopySavedOptionsButtonMappingsAndTimesToWorkingRecord",
                   "CopySavedFlagsButtonMappingsAndTimesToWorkingRecord", "record flag bits, as 80034F88"),
    "0x80074220": ("gpu_DrawSheetBackdropWithOptionalTileAndOpaqueQuads",
                   "gpu_DrawThreeSheetsWithOptionalTileAndOpaqueQuads",
                   "'Backdrop' is a role the code does not show; the body adds the cells of three sheets, an optional "
                   "semi-transparent tile, a draw-mode primitive and three flat quads"),
    "0x80074488": ("menu_DrawSelectedOptionSheetsWithPulseAndOffset",
                   "menu_DrawSheetsWithSelectedPulseOffsetDimmedOthersAndGreyTiles",
                   "'Option' is a role; the body also dims the unselected sheets and adds the grey tiles"),
    "0x800343F0": ("ApplyPendingConfigAndResetTextureRequestsAndFightState",
                   "ApplyPendingConfigResetCellsAndClearSlotTimGroupRequests",
                   "'FightState' is not shown for the cleared cells; the requests are slot_RequestTimGroupsAndMarkReloadOnChange "
                   "(slot, 0) for slots 0 and 1"),
    "0x800344B4": ("ApplyPendingConfigAndInitializeMode", "ApplyPendingConfigAndInitializeMode", ""),
    "0x80034708": ("menu_PrintEditAndApplyConfigWithIndependentPadCursors", "menu_PrintEditAndApplyConfigWithPerPadCursors",
                   "wording only (shorter)"),
    "0x80026DA4": ("fight_UpdatePairedInputTotalsEventsAndMidpointEffects",
                   "fight_UpdatePairedInputTotalsEventsAndMidpointEffects", ""),
}

DROPPED = {}

# row notes: each adjustment against the research, with its reason (kept rows keep the default note)
for _a, (_old, _new, _why) in ADJUSTED.items():
    if _old != _new:
        NOTES[_a] = f"adjusted from research {_old}: {_why}. Pending naming-reviewer pre-apply (round 1)."

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
    """src/main/<file>:<first>-<last> of the C definition of fn (asm/funcs/<fn>.s for an INCLUDE_ASM body)."""
    if f == "asm":
        n = len(open(os.path.join(WT, "asm", "funcs", fn + ".s"), encoding="utf-8", errors="replace").read().split("\n"))
        return f"asm/funcs/{fn}.s:1-{n - 1} (INCLUDE_ASM at src/main/43500.c)"
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
    assert ADJUSTED[addr][1] == new, (addr, new)
    ev = (f"{ch(addr)}BODY: {B(fn, f)}: {body}. CALLERS: {callers}. WHY: {why}. COUNTER-EVIDENCE: {counter}. "
          f"Prior manifests: {prior}. {T(addr)}")
    return (addr, new, "RENAME", ev)


_WORK = ("Working record = D_8009BD24 (func_80077D00 returns it): 25788.c copies the saved record D_80106A50 into it "
         "(func_80035280) and back (func_80034F88), the menus edit it; pending config = D_80102778: func_8003504C fills it from "
         "the working record, 24F08.c edits it, func_800343F0 copies its bytes into the live cells. ")
_CARD = ("memcard_ is the callee family (memcard_RequestLoad / RequestSave / RequestOverwrite / RequestFormat / Update). ")
_SLOT = ("Slot = an index of D_800A9A10[3] (wave88: slot_GetObject, slot_FindEmpty, slot_ReleaseObjectAndResources); node_ = the "
         "transform nodes of the object it holds (node_UpdateWorldMatrix, node_RebaseParents). ")
_ROWMENU = ("Row menu = the handler pair 80035480 (init) / 80035828 (update) in d_7D870.c's table; its callees are "
            "menu_SetEnabledRows, menu_UpdateRowMenuStoreResult, InitRowMenuBlackBgResetFlow; D_800A3740 is the subflow state "
            "the update switch dispatches on. ")

FUNCS = [
    # ------------------------------------------------------------------------------------------ 43500.c
    R("0x80053304", "collision_TestSegmentExtendingShortStart", "func_80053304", "43500.c",
      "D_800A33F4 = &D_800EF9F8 (the static work area); copies endpoints arg0 / arg1 into +0x8 / +0x18; when "
      "gte_SumSquares3(end - start) <= 0x9C3F it sets the cell tester at +0x5C to func_80053754 and moves the start to "
      "start - 2 * (end - start), otherwise sets func_80053E9C; then collision_WalkSegmentGrid(arg2, arg3), result discarded "
      "(void)",
      "none (no C call, no jal, no address word; only the comments at include/game.h:1152 and named_syms.txt:582 mention it)",
      "same shape as collision_TestSegment (43500.c:216: the static work area, the GTE tester, the walk) with the short-segment "
      "start extension added; the walk stores its hit in the work area, which collision_GetHit reads back; the wrapper pair "
      "TestSegment / TestSegmentWithWork is the TU's own vocabulary",
      "the void return is not claimed; no movement-to-target claim (the retired name init_move_to_target stays rejected); the "
      "function is uncalled, so no caller disagrees",
      "no_name.tsv (w60: 'no executable caller found and intended contract of short-segment extension versus void return remains "
      "unclear'; answered: the extension is the name's distinguishing mechanic, the return is simply not named)"),

    R("0x8005344C", "collision_TestSegmentExtendingShortStartWithWork", "func_8005344C", "43500.c",
      "D_800A33F4 = arg4 (caller-supplied work area); the same endpoint copy, short-segment test (<= 0x9C3F), tester choice and "
      "start extension as func_80053304; returns collision_WalkSegmentGrid(arg2, arg3)",
      "9F9C.c:3504 / 3523 (work 0x1F8002B8), 9F9C.c:3924 / 3938, 9F9C.c:4311 / 4321 (end point 0x1F8001E0), 17AFC.c:4110 / 4459 "
      "and the asm of fight_SearchRadialFlatSurfacePointOrUseStartPositionPair, fight_UpdateBallisticRecords, "
      "fighter_CacheDownwardProbeHit, fighter_ResolveMovement, fighter_TestDownwardProbeFromCollisionPosHitsUpwardFacingSurface, "
      "object_UpdatePool; every caller passes a work address as the fifth argument and tests the result as a hit flag",
      "differs from collision_TestSegmentWithWork (43500.c:224) only by the short-segment start extension, and from "
      "func_80053304 by the caller's work area and the returned walk result; the same wrapper-pair naming as the TU",
      "no hit-semantics claim beyond 'Test' (the result is the walk's, as in collision_TestSegmentWithWork)",
      "no_name.tsv (w60: 'precision/extension variant needs a clear non-speculative distinguishing name'; answered: the "
      "extension is named, QuerySegmentWithWork's collision with func_80053614 is avoided by keeping the TU's TestSegment verb)"),

    R("0x80053754", "collision_TestCellKeepingNearestHitScalarProjection", "func_80053754", "43500.c",
      "(cell x, cell z): returns 0 outside the 32x32 grid or for an empty cell (0xFFFF); otherwise for each face record of the cell "
      "computes the plane side of both segment ends (products >> 10 with sign handling), and for a crossing finds the "
      "intersection with integer division and rounding, projects it into the face plane with two integer dot products >> 14, "
      "tests the point against the face's 2-D polygon and, when inside and nearer than +0x0, stores cell, point, plane and "
      "distance (+0x48 .. +0x40, +0x28 .. +0x34, +0x0, +0x4); returns whether any hit exists (+0x0 != 0x7FFFFFFF)",
      "only stored as the per-cell tester pointer at +0x5C by func_80053304 / func_8005344C (43500.c:179 / 204), which "
      "collision_WalkSegmentGrid (43500.c:40) calls per crossed cell",
      "the tail (nearest-hit update, return) is instruction-for-instruction the tail of func_80053E9C; the two bodies differ in "
      "the projection arithmetic (here plain integer products and rounding, there the GTE transform helpers); 'Cell' = the "
      "cell arguments indexing the grid word table D_800A33F0",
      "'Scalar' means the projection only (both bodies call gte_SumSquares3 for the distance); no gameplay surface type "
      "claimed (face records stay 'records')",
      "no no_name.tsv row"),

    R("0x80053E9C", "collision_TestCellKeepingNearestHitGteProjection", "func_80053E9C", "43500.c",
      "same cell lookup, plane sides (products >> 14) and nearest-hit update as func_80053754, but the intersection point is "
      "transformed with gte_TransformPointDifferenceWithInheritedLastRow and read back with gte_ReadIR1IR2Sra2 before the "
      "polygon test; returns whether any hit exists",
      "the default tester: stored at +0x5C by collision_TestSegment (43500.c:220), collision_TestSegmentWithWork (43500.c:229), "
      "func_80053304 / func_8005344C (43500.c:186 / 211)",
      "see func_80053754: same effects, GTE projection instead of integer products; its two named GTE callees are the evidence "
      "of the projection",
      "no hit-type claim; 'Gte' does not mean the whole body is cop2",
      "no no_name.tsv row"),

    R("0x80054440", "gte_ColorGroupedNormalStreamWithSelectedMatrices", "func_80054440", "asm",
      "a0 = stream, a1 = matrix pointer array: per group reads a selector (-1 ends the stream), multiplies the matrix at "
      "D_800A3828 + 0x18 by a1[selector] with gte_MulMatrix0ClearTrans into scratchpad 0x1F800020 and loads it into the GTE "
      "light-matrix registers (ctc2 c2_8..12), then reads a count and runs nct over the group's normals three at a time (0x12 bytes per step), "
      "storing the three colour FIFO words (swc2 20-22) per triple into D_800F2B70; returns the advanced input pointer",
      "asm/funcs/func_8004DDB4.s:23 (the single jal; INCLUDE_ASM in 3AB48.c:35, called from 32D04.c:508)",
      "every step is a cop2 or GTE-wrapper call: normal colour triples (nct) lit through a light matrix composed with a "
      "selected matrix; the group / selector / sentinel layout is read straight from the loads; the gte_ prefix is the whole "
      "body's, as the TU's gte_SumSquares3 and the callee gte_MulMatrix0ClearTrans",
      "no claim which lights, which matrices or what consumes D_800F2B70; the data suggestion g_normal_stream_colors needs its "
      "own all-access review",
      "no_name.tsv (w60: 'input group contract, preinstalled GTE state and exact output consumption need fuller verification'; "
      "answered: the contract is the stream layout read from the asm; the preinstalled state is only D_800A3828 and the "
      "scratchpad product, named as such; the consumer is not named)"),

    R("0x80054884", "motion_StartPlaybackAtIndexedBasePlusOffset", "func_80054884", "43500.c",
      "motion_StartPlayback(D_80099D54[a0] + a1 - 0x131, a2 .. a7)",
      "2B344.c:529 (D_800A376C, D_800A3878[0]), 2B344.c:709 (0x16, D_8009016C[D_800A38A4]), 2B344.c:1029 (0x16, s0) and the asm of "
      "func_8003B9D0, func_8003C040, func_8003CE18",
      "the wrapper only forms the first argument of motion_StartPlayback from an s16 table entry selected by the first argument "
      "plus the second, and forwards the rest (all three callers pass a table index and an offset into that entry's range)",
      "the table's meaning (CD file bases) comes only from a registry comment and is not used; the literal subtracted is not "
      "named",
      "no no_name.tsv row"),

    R("0x800548DC", "motion_DrawSyncApplyHeaderSectionsAndFreePlaybackBlock", "func_800548DC", "43500.c",
      "DrawSync(0); resource_SelectRetainedSubblockOrApplyHeaderSections(-1); motion_FreePlaybackBlock()",
      "2B344.c:557 / 750 / 1098 and the asm of func_8003BCB4, func_8003C2C0, func_8003CF84 (each after a playback ended or was "
      "skipped)",
      "with a negative argument the resource callee (368E4.c:217-221) is resource_ApplyHeaderSectionsResizeBlockAndClearRetained"
      "Subblocks; motion_FreePlaybackBlock is arena_Free(0xA) (368E4.c:403); 'motion_' is the family of the freed block's "
      "allocator (motion_LoadPlaybackBlock)",
      "no 'stop playback' claim (the body changes no playback state directly)",
      "no_name.tsv (w60: 'complete resource teardown/restoration contract of both callees needs verification'; answered: wave86 "
      "named the resource callee, and the name states the three calls instead of a contract)"),

    # ------------------------------------------------------------------------------------------ 25C38.c
    R("0x80035438", "menu_ResetSubflowCopySavedToWorkingAndSetEnabledRows", "func_80035438", "25C38.c",
      "D_800A3740 = 1; func_80035280() (copy the saved record into the working record); menu_SetEnabledRows(0xFF when "
      "D_80106A50.unk_04 == 0x3F, else 0xF7)",
      "25C38.c:49 (func_80035480) and 25C38.c:234 (func_80035828) and the asm of both",
      _ROWMENU + _WORK + "all three effects are named by the body: the subflow state reset, the transfer, the enabled-row mask",
      "which rows the two masks enable is not claimed",
      "no_name.tsv (w64: 'selection mask and callee meaning not sufficiently established'; answered: the callee names are now "
      "wave90's own 80035280 transfer and menu_SetEnabledRows, and the mask is not interpreted)"),

    R("0x80035480", "menu_ReleaseResourcesConditionallyCloseLinkAndVabsAndInitRowMenu", "func_80035480", "25C38.c",
      "gpu_ResetGraphMode1; gpu_HideResetAndClear; comb_RequestClose when D_800A31DA == 0; motion_InvalidateMarkedNdataPack..., "
      "slot_ReleaseObjectAndResources(0 and 1), eff_ClearInitFlag; when D_800A31D8 != 0 snd_CloseNonCommonVabsAndResetVoicesQueueAnd"
      "Reverb and D_800A390E = -1; D_800A31D8 = 1; func_80035438(); InitRowMenuBlackBgResetFlow(0x80118800); D_800A37B8 = 0; "
      "D_800A3834 = 9; gpu_SetDispMaskOn",
      "d_7D870.c:38 (handler table entry 8, address-taken; D_800A3834 = 8 selects it from func_80035E38 and 25C38.c:388)",
      _ROWMENU + "the two conditions (link flag, VAB flag) are explicit; D_800A3834 = 9 selects the update handler 80035828",
      "no screen identity (title / mode select) is claimed; the unconditional release calls are the ones named",
      "no_name.tsv (w64: 'screen identity and conditional resource reset remain opaque; shared plumbing alone does not admit "
      "menu_'; answered: menu_ now rests on the callees listed above, the conditions are in the name)"),

    R("0x800355E8", "cdrom_EnableSerialMixAndStartRepeatingAudio", "func_800355E8", "25C38.c",
      "snd_SerialMixOn(); cdrom_StartAudioEntry(1); cdrom_SetAudioRepeat(1)",
      "25C38.c:114-183 and 362 (every subflow init of func_80035618 and func_80035828), asm of func_80035618",
      "each statement is named by its callee; the entry is a literal and is not named, the repeat is on",
      "no track identity; func_80035828 starts entry 7 inline (25C38.c:238-241), so this is not 'the menu music'",
      "no_name.tsv (w64: 'entry identity not established ... naming by literal entry would violate scope rule'; answered: the "
      "name does not name the entry)"),

    R("0x80035618", "menu_InitSubflowForMode", "func_80035618", "25C38.c",
      "(mode / row value, 0..7 else return) switch: per case calls the subflow's init (InitTimedSpriteSequence, "
      "InitWithCachedValueAndStoredBitsBlackBg, menu_ResetFlowBackgroundAndConfirmCursor, comb_Connect polling, menu_InitSelWithStoredBits, "
      "StoreCompactedBits), sets the subflow state D_800A3740, and starts repeating audio (func_800355E8) in most cases",
      "25C38.c:205 (func_80035828 with D_800A38DC when the link flag is set), 25C38.c:270 (with D_8009BD58[row][0] from "
      "ResetFlowAndFetchRowTablePair, the identity table d_8C4E4.c:72, stored into D_80102778.unk_D) and the asm of func_80035828",
      _ROWMENU + "the argument is the mode value: D_800A38DC (the mode cell func_800343F0 copies from D_80102778.unk_D) at 25C38.c:205, "
      "the identity-table value at 25C38.c:270; case 6 is the comb_Connect link mode (D_800A38DC == 6 at 9F9C.c:1065); the "
      "body initialises that mode's subflow (CHANGED: suggested by pre-apply review, see note)",
      "not every row is a player choice (timed sprite sequence, link connect, option menu); no row meaning claimed",
      "no_name.tsv (w64: 'multiple opaque constructors; not uniformly a player-choice screen'; answered: the name says 'subflow' "
      "and names no screen)"),

    R("0x80035828", "menu_UpdateRowMenuAndSubflows", "func_80035828", "25C38.c",
      "per frame: runs menu_UpdateRowMenuStoreResult in subflow 1 and, per D_800A3740, the memcard load / prompt / retry / save "
      "steps (func_8003880C, func_800388A8, func_80038988, func_80038C70), the timed sprite sequence, the option-mode flow, the "
      "link connect, the vab-closing steps; draws the screen gradient (gpu_AddScreenGradient) in the card states; transitions "
      "set D_800A3740 / D_800A3834",
      "d_7D870.c:39 (handler table entry 9; selected by func_80035480)",
      _ROWMENU + "its callees cover the row menu update and each subflow; the sibling update 80035E38 steps only the memcard "
      "prompt flow",
      "no title / mode-select claim; 'Subflows' covers the card, timed, option and link states without naming them",
      "no_name.tsv (w64: 'includes timed and card states as well as selection handlers; no single specific menu identity "
      "established'; answered: the name names the row menu update and 'subflows')"),

    R("0x80035DC8", "menu_ReleaseResourcesCloseVabsAndInitRowMenu", "func_80035DC8", "25C38.c",
      "gpu_ResetGraphMode1; gpu_HideResetAndClear; motion_InvalidateMarkedNdataPack...; slot_ReleaseObjectAndResources(0 and 1); "
      "eff_ClearInitFlag; snd_CloseNonCommonVabsAndResetVoicesQueueAndReverb (unconditional); InitRowMenuBlackBgResetFlow(0x80118800); "
      "D_800A3834 = 0x1B; gpu_SetDispMaskOn",
      "d_7D870.c:56 (handler table entry 26, address-taken); never called in C",
      _ROWMENU + "differs from func_80035480 by the unconditional VAB close, no link close, no D_800A31D8 / func_80035438 and the "
      "next handler 0x1B (func_80035E38)",
      "no screen identity; never-hit in the captures, which only means the static mechanics carry the name",
      "no_name.tsv (w64: 'resource sequence cannot establish exact screen identity'; answered: the name states the release / "
      "close sequence and the menu init callee, not the screen)"),

    R("0x80035E38", "DrawGradientAndStepMemcardSavePromptsThenSelectRowMenuInit", "func_80035E38", "25C38.c",
      "D_800A36F1 = 1; gpu_AddScreenGradient(); when func_80038C70() (the memcard save / overwrite / format prompt machine) "
      "returns nonzero: D_800A3834 = 8, D_800A36F1 = 2",
      "d_7D870.c:57 (handler table entry 27, address-taken; selected by func_80035DC8)",
      _CARD + "func_80038C70 requests save / overwrite / format, which is the 'memcard save prompts' of the name; handler 8 is "
      "func_80035480 (d_7D870.c:38), the row menu init",
      "no menu_ prefix: the body has no choice handler of its own (the old w64 objection stays honoured)",
      "no_name.tsv (w64: 'polling screen wrapper; no own choice handler; menu_ unsupported by body'; answered: unprefixed, the "
      "stepped machine is named by what it requests)"),

    # ------------------------------------------------------------------------------------------ 64FD8.c
    R("0x80077860", "DrawWhiteTileAndSheetsAndResetFlowOnPadMatch", "func_80077860", "64FD8.c",
      "calls gpu_DrawWhiteTileAndSheetsTestPadMask (cast to an argument-less call) and when it returns 1 sets D_800A35E4 = 0 "
      "and returns 1, else returns 0",
      "none (no C call, no jal, no address word)",
      "the callee draws the white tile and sheets and returns 1 when the pressed mask hits 0x400040 (596CC.c:214-225); the "
      "flow-state vocabulary is the TU's own (SetFlowStateOne, ResetFlowAndFetchRowTablePair, InitRowMenuBlackBgResetFlow all "
      "write D_800A35E4 the same way)",
      "the arguments are not forwarded (the C form calls it without any), so what the pad test sees is not claimed; the function "
      "is uncalled",
      "no_name.tsv (w73: 'no external caller found and callee screen identity unresolved'; answered: the name keeps the callee's "
      "own words and claims no screen)"),

    R("0x80077D00", "GetWorkingRecord", "func_80077D00", "64FD8.c",
      "returns &D_8009BD24",
      "25788.c:16 / 38 / 117, 2B344.c:832, 4BC3C.c:446 (snd_LoadVabPack), 596CC.c:450 / 467 (gpu_SetTileRgbFromBlockFlag and "
      "gpu_SetPolyF4RgbFromBlockFlag), 5ED34.c:153 and the asm of gpu_LoadVramBottomStrip, func_8003C714",
      _WORK + "every caller uses the result as the Unk8009BD24Block, whether to copy it to or from the saved record or to read a "
      "single bit; the getter says only that it is the working copy",
      "the record's fields are not summarised (options, mappings, times, flags are all in it); no gauge identity (the old "
      "symbol_addrs comment is MISNAMED)",
      "no_name.tsv (w78: 'main__64FD8.tsv:36 hit_all gives no role'; answered: the name rests on the transfers, not on a hit map)"),

    # ------------------------------------------------------------------------------------------ 28708.c
    R("0x8003880C", "memcard_StartLoadOnceStepAndDrawSheetPair", "func_8003880C", "28708.c",
      "when the latch D_800A3203 is set: clear it, D_800A31FC = 1, memcard_RequestLoad(); v = memcard_Update(); result 0 for "
      "status 4, 1 for status 5, -1 for the others; gpu_DrawSheetPairCenteredTileWithOptionalSelection(0, -1); on a nonzero "
      "result re-arms the latch and clears D_800A31FC; returns the result",
      "25C38.c:211 (func_80035828 subflow 7) and the asm of func_80035828",
      _CARD + "no retry and no dismissal wait (the other two step helpers, 80038988 and 80038C70, have them), so the "
      "start-once / step / draw shape tells it apart",
      "no card-status meaning claimed (status 4 / 5 are not named); 'initial' (research) is not shown",
      "no_name.tsv: none for this address"),

    R("0x800388A8", "menu_StepTwoChoicePromptDrawSheetsAndRearmAfterResult", "func_800388A8", "28708.c",
      "a latch D_800A3204 clears the choice D_800A3318; on pressed bits 0x400040 queues sound 1 and returns 1 when the choice is 0 "
      "else -1; on 0x80008000 / 0x20002000 queues sound 0 and sets the choice 0 / 1; draws the sheet pair "
      "(gpu_DrawSheetPairCenteredTileWithOptionalSelection(0x13, choice)); after a result re-arms the latch",
      "25C38.c:222 (func_80035828 subflow 8) and the asm of func_80035828",
      "a prompt with exactly two choices, the pad moves the choice, confirm returns a result by choice, the latch clears the "
      "choice before the next use; menu_ is the caller family",
      "what the two choices mean and what the sheets show are not claimed",
      "no no_name.tsv row"),

    R("0x80038988", "memcard_RetryLoadDrawStatusSheetsAndWaitForTimeoutOrPress", "func_80038988", "28708.c",
      "re-arming latch D_800A3205 resets the retry counters and issues memcard_RequestLoad(); each frame memcard_Update() status "
      "selects the sheet pair to draw (including -1 for none) and bumps a per-status retry counter (< 5, then re-request or "
      "give up, or flag the failure); once ended counts D_800A3330 down and returns 1 when it reaches 0 or on the confirm bits "
      "0x100010 (sound 2)",
      "25C38.c:233 (func_80035828 subflow 9) and 25C38.c:308 (subflow 13) and the asm of func_80035828",
      _CARD + "versus 8003880C (no retries) and 80038C70 (save / overwrite / format prompts): bounded per-status retries, "
      "status-selected sheets, a timed or pressed dismissal",
      "no meaning for the individual status codes; the numbers are not in the name",
      "no no_name.tsv row"),

    R("0x80038C70", "memcard_StepSaveOverwriteAndFormatPrompts", "func_80038C70", "28708.c",
      "per frame: memcard_Update() drives a state machine that retries load (memcard_RequestLoad), requests save "
      "(memcard_RequestSave), overwrite (memcard_RequestOverwrite) or format (memcard_RequestFormat) after the pad's yes / no "
      "prompts, draws the status sheets with the selection, queues sounds and returns a done flag",
      "25C38.c:316 (func_80035828 subflow 14), 25C38.c:413 (func_80035E38) and the asm of both",
      _CARD + "the four requests are named by callees; the prompts are the sheet draws that take a selection",
      "the preparatory load retries are part of it but not named; no screen identity",
      "no no_name.tsv row"),

    R("0x800392B8", "GetSharedBuffer", "func_800392B8", "28708.c",
      "returns D_800F33D8 (u8[])",
      "cdrom.c:247 (the third argument of sys_Exec(6, ..., buf + 0x7FC) in the movie launcher) and the asm of sys_ExecMovieOverlay",
      "the same address is the save block buffer (28708.c:368-370 / 388, + 0x100 at 41 / 369), the replay frame array "
      "(28708.c:1104, g_replay_fighter_frames) and 6CF8.c:539's workspace, so it is a buffer shared by several users; the "
      "single caller uses it as the exec stack",
      "its extent and every user's role are not claimed; D_800F33D8 itself needs its own all-access data review",
      "no no_name.tsv row"),

    R("0x8003984C", "fighter_ProbeVerticalCollisionAtPointPairMidpointForTagAndClass", "func_8003984C", "28708.c",
      "(fighter record, out1, out2): midpoint of the record's two world points (unk_198[0], unk_198[1]); builds a vertical "
      "segment +-0x190 around it; collision_TestSegment; on a hit out1 = collision_GetHitTag() and out2 = -1 for tags 7 / 0 "
      "else ((0x2A >> tag) ^ 1) & 1; on a miss out1 = out2 = -1",
      "28708.c:1427 (replay_PlayFrame; asm/funcs/replay_PlayFrame.s:225, stores the outputs into the record's unk_B1 / unk_B2)",
      "the callees are collision_TestSegment and collision_GetHitTag; the first argument is the fighter record (D_80101EC8 "
      "type) as in wave84's fighter_TestDownwardProbeFromCollisionPosHitsUpwardFacingSurface; the class is the masked bit",
      "no bone, material or surface-type claim; the two outputs are named by their derivation only",
      "no no_name.tsv row"),

    # ------------------------------------------------------------------------------------------ 4510C.c
    R("0x80054F68", "motion_ResetDrawQueueAdvanceCounterSetOtUpdatePlaybackAndFlush", "func_80054F68", "4510C.c",
      "g_draw_queue_cursor = g_draw_queue; D_800A38D6++; D_800A3808 = g_gpu_ot_ptr; D_800A378C = g_gpu_ot_ptr + 4; "
      "s = motion_UpdatePlayback(); func_800444E0() (a forwarder of gpu_PrepareViewAndFlushDrawQueue(D_800A378C), 32D04.c:1121); "
      "returns s",
      "2B344.c:553 / 729 / 1094 and the asm of func_8003BCB4, func_8003C2C0, func_8003CF84 (the playback frame callers test the "
      "result)",
      "the first four stores are the per-frame draw-queue / OT setup 368E4.c:449-453 does; the playback update and the flush are "
      "named by callees; 'motion_' is the playback update's family",
      "D_800A38D6's role is not claimed (an incremented counter); the flush is a neutral forwarder, so it is described as flush",
      "no_name.tsv (w60: 'retain neutral name until the complete draw-side effect and counter lifetime are established'; "
      "answered: wave88 named gpu_PrepareViewAndFlushDrawQueue, and the counter is only 'advanced')"),

    R("0x8005508C", "node_GetRootNodeMatrixTranslation", "func_8005508C", "4510C.c",
      "returns D_80101DF0.xf.mat.t",
      "2B344.c:1092 (b, passed with motion_GetPlaybackViewAngles() to func_80061064) and the asm of func_8003CF84",
      "D_80101DF0 is the first node initialised by node_InitRootNodePairWithNegatedRotationMatrices (368E4.c:2190), written by "
      "368E4.c:457-479 and read as a node (xf.mat, xf.rot, work.t) in 2B344.c; the getter returns the matrix's translation "
      "array",
      "no camera / view claim; the second root node (D_800FF638) is not touched",
      "no no_name.tsv row"),

    R("0x80056FE8", "fighter_CalcOpponentDistanceThresholdFromTables", "func_80056FE8", "4510C.c",
      "a = arg0->other; base = a->motion_metadata[3] * 40, plus D_8009A830[a->unk_0E] * 2 or D_8009A838[..] * 8 when "
      "weapon_hit_end_frames[0] != 0xFF (by move_is_common) else D_8009A840[a->unk_14] * 2; returns base + "
      "a->scale_distance_offset + 0x12C",
      "4510C.c:763 (fighter_BuildCpuPad: temp2 / temp3 compared against the distance D_800A387C, 768-778) and the asm of "
      "fighter_BuildCpuPad",
      "the value is a sum of table terms of the opponent record plus its scale_distance_offset field and a constant, and every "
      "use compares it with D_800A387C (also used as a radius with the sin table at 4510C.c:1188)",
      "no attack-range / threat-range claim; the tables' semantics are not named",
      "no_name.tsv (w60: 'likely a reach-related threshold but the contributing fields do not yet prove a precise noun'; "
      "answered: 'distance threshold from tables' claims only the compared quantity)"),

    R("0x80057094", "fighter_BuildCpuRoutePadWithSurfaceProbe", "func_80057094", "4510C.c",
      "(record, x, z, flag): angle between the record-to-D_800F6608 and record-to-(x, z) bearings, minus 0x100, quantised to eight "
      "sectors (with a +-1 sector nudge from the build counter when flag == 0 and the sector is even); returns D_8009AA50[sector] "
      "| 4 when flag == 1, | 8 when fighter_ProbeInputDirectedSurfaceClearanceAndSurfaceBelow reports nonzero",
      "4510C.c:2086 (inside func_80058580: p->unk_3CC = func_80057094(p, wx, wz, work3) with the route's last node "
      "coordinates) and the asm of func_80058580 (its only caller)",
      "the arguments are the route's waypoint and a near / far flag; the result is pad bits (a table of direction bits plus two "
      "extra bits); the surface probe is a named callee; fighter_ is the CPU pad family",
      "what D_800F6608 is and what bits 4 / 8 mean are not claimed",
      "no no_name.tsv row"),

    R("0x80058580", "fighter_UpdateCpuNavigationChooseScriptsAndHeldInput", "func_80058580", "4510C.c",
      "per call: profile-driven choice of a pad script (fighter_StartCpuPadScript) or random hold, maintains the route "
      "(fighter_FindNearestNavCrossing, fighter_AppendNavDetour, cpu_route nodes, nearest_nav_crossing_distance), builds the held "
      "input (func_80057094) and returns it or the retry sentinel -1",
      "4510C.c:886 (fighter_BuildCpuPad, in the loop with fighter_StepCpuPadScript; temp == -1 retries up to four times) and the "
      "asm of fighter_BuildCpuPad",
      "its callees are the CPU navigation, route and script starters; the alternative caller branch (script cursor set) is "
      "fighter_StepCpuPadScript; D_80099D88 is the CPU profile table it reads",
      "no policy names (attack / defend / approach); the retry sentinel is not interpreted",
      "no_name.tsv (w60: 'CPU family is proven but finer policy names would require decoding state and profile semantics'; "
      "answered: the name states the three effect families, not the policies)"),

    # ------------------------------------------------------------------------------------------ 31548.c
    R("0x80040D48", "slot_SetRootTransformUpdateMatricesQueueNodesSoftbodyAndLightTarget", "func_80040D48", "31548.c",
      "(slot, mode, translation, root rotation, pose rotations, word): returns for an empty slot; stores the root rotation and "
      "translation, in mode 0 the per-node rotations through D_80094CFC and the two trailing nodes, in mode 1 (NULL pose at 3 of 4 callers; the per-node matrices come from func_80041188 called just before) only zeroes node 1; "
      "scales the y translation, node_UpdateWorldMatrix over 18 nodes; queues the nodes, the duplicate nodes (copying the linked "
      "matrix) and the projected nodes into the draw queue; node_SetWordOnAllListedNodes(projected_nodes, word); "
      "softbody_SetObjectGround, softbody_UpdateObject, light_SetPointLightTarget",
      "28708.c:1408 (replay), 4510C.c:154 (motion playback), 9F9C.c:4982, and the asm of fighter_Update",
      _SLOT + "each part is a named callee or a plain store: the root transform, the matrix updates, the queueing, the softbody and "
      "the light target",
      "no fighter claim (the same code runs for the playback object); the pose format is the rotation triples only",
      "no_name.tsv (w64: 'broader than pose setter or draw queue helper; fighter_ does not cover all object-oriented uses'; "
      "answered: slot_ is wave88's vocabulary and the name lists the effects)"),

    R("0x80041188", "slot_BlendTwoPosesIntoNodeMatrices", "func_80041188", "31548.c",
      "(slot, pose a, pose b, factor, scratch matrices): for the 17 nodes of D_80094CFC builds a rotation matrix from each pose's "
      "triple (math_RotMatrixZYX) and writes the blend (gte_LerpMatrixRowsAndRebuildOrthonormalBasis) into the node's matrix with "
      "matrix_state 2; for the two trailing nodes also blends the translation (math_LerpShortVectorsNegateYZ) and the "
      "rotation, matrix_state 1",
      "28708.c:1400 (replay_PlayFrame), 9F9C.c:4979, and the asm of fighter_Update, replay_PlayFrame",
      _SLOT + "the interpolation is stated by the GTE and math callees; the two inputs are the two poses",
      "no Euler or quaternion claim; the pose buffer format beyond rotation triples and two trailing records is not named",
      "no_name.tsv (w64: 'full semantics of the interpolation callee and pose format should be checked'; answered: the callee "
      "names state the lerp, and the name claims only a blend)"),

    R("0x80041398", "PatchColorImmediatesAtListedCodeSites", "func_80041398", "31548.c",
      "builds four words from the instruction templates at func_800545F4 / D_800545F8 / D_800545FC / D_80054600 with the colour's "
      "bits 16-23 and 0-15 in their immediates and stores them over the eight code addresses of D_80015820 (two per site, four "
      "sites)",
      "31D3C.c:114 (grey colour) / 119 (rgb colour) and the asm of node_SetPrimitiveTableSelectOnObjectNodesAndPatchColor",
      "named_syms.txt:1120-1123 states the template words are two lui / ori pairs; the body mutates immediates of instructions, "
      "and each caller passes a packed 24-bit colour; D_80015820 holds code addresses (31548.c:261-278)",
      "which renderer the sites belong to is not claimed (a knockout would be needed); the code-patching is explicit instead of "
      "'SetColor'",
      "no_name.tsv (w64: 'generic SetColor would hide instruction mutation; target routines need separate verification'; "
      "answered: the name states the mutation and names no renderer)"),

    R("0x80041430", "slot_RebaseObjectPointerNodeLinksAndRefreshMatrixPointers", "func_80041430", "31548.c",
      "(slot, delta): D_800A9A10[slot] += delta; node_RebaseParents(delta, transform_nodes, 0x15), (projected_nodes, 0x14), "
      "(duplicate_nodes, 0x14); adds delta to the nonzero unk58 links of the duplicate nodes and to the node_by_id pointers; "
      "node_RefreshObjectMatrixPointerTables(object)",
      "35000.c:589 and the asm of func_80045AA4",
      _SLOT + "the effect list is the body; no asset block is touched (the caller handles those separately)",
      "no arena or relocation-callback claim (the research reason only gave the context)",
      "no_name.tsv (w64: 'does not relocate asset block; object/asset distinction should be made consistently'; answered: slot_ / "
      "Object per wave88, nothing about asset blocks)"),

    # ------------------------------------------------------------------------------------------ 87A0.c
    R("0x8001945C", "pad_SetDefaultSavedButtonMappings", "func_8001945C", "87A0.c",
      "D_80106A50.frec20[0..2] = 0x11, 0x44, 0x88",
      "6CF8.c:141 (InitSavedRecordAndResetCameraDistanceOffset) and its asm",
      "frec20's three bytes are masks of source pad buttons per output button: 9F9C.c:1074-1115 (fighter_BuildCpuOrMappedPadState) "
      "tests each bit of each byte against a pad bit (1 -> 0x20, 2 -> 0x10, 4 -> 0x40, 8 -> 0x80) and ORs 0x20 / 0x40 / 0x80 "
      "into the output; one nibble per local player; the constants are the initial masks",
      "which button each constant maps is not claimed; the 'colour' reading of r / g / b locals is rejected by the code",
      "no_name.tsv (w64: 'nibble values alone do not establish what the three settings mean'; answered: the consumer shows the "
      "three bytes are input-button mappings)"),

    R("0x80019488", "pad_PackSavedButtonMappingLowNibbles", "func_80019488", "87A0.c",
      "returns (frec20[0] & 0xF) | ((frec20[1] & 0xF) << 4) | ((frec20[2] & 0xF) << 8)",
      "2B344.c:93 (comb_Connect; the value is stored in D_800A37C4) and the asm of comb_Connect",
      "the mapping bytes of the saved record are one nibble per player (see func_8001945C); this packs the low nibble of each "
      "of the three; its counterpart unpacks three nibbles into D_800A3912-14",
      "the link exchange role (which player, sent or received) is not claimed",
      "no_name.tsv (w64: 'packing is clear but settings identity remains opaque; no literal-width name proposed'; answered: "
      "the name has no widths and no identity, only the pack)"),

    R("0x800194C0", "pad_UnpackButtonMappingNibblesToCells", "func_800194C0", "87A0.c",
      "D_800A3912 = a & 0xF; D_800A3913 = (a >> 4) & 0xF; D_800A3914 = (a >> 8) & 0xF",
      "2B344.c:100 (comb_Connect, with D_800A36C6) and the asm of comb_Connect",
      "D_800A3912-14 are read by 9F9C.c:1074-1091 as the same mask nibbles the saved bytes hold, when D_800A38DC == 6 and the "
      "player is not D_800A38A0; the body is the exact inverse of func_80019488's packing",
      "'remote' is not claimed (D_800A36C6's writers are not in C); the three cells are not renamed here",
      "no_name.tsv (w64: 'output settings roles not independently established; comb_ would describe only current caller'; "
      "answered: pad_ rests on the consumer; the name says nothing about the link)"),

    # ------------------------------------------------------------------------------------------ 25788.c
    R("0x80034F88", "CopyWorkingFlagsAndButtonMappingsToSavedRecord", "func_80034F88", "25788.c",
      "p = func_80077D00(); D_80106A50.flags &= ~7, then |= 1 / 2 / 4 from p->unk20_0..2; D_80106A50.frec20[0..2] = p->unk17[0..2]",
      "25C38.c:286 / 297 / 301 (after the option-mode flow) and the asm of func_80035828",
      _WORK + "the direction is working to saved (the sibling 80035280 is the reverse); the mappings are the frec20 bytes "
      "shown in func_8001945C's row",
      "no physical card write (the old w64 'memcard_' objection holds); the three flag bits' meanings are not claimed",
      "no_name.tsv (w64: 'exact record/settings roles remain incomplete; memcard_ would claim physical I/O absent from body'; "
      "answered: unprefixed, direction and fields named)"),

    R("0x8003504C", "ApplyWorkingRecordToPendingConfigAndInitializeMode", "func_8003504C", "25788.c",
      "p = func_80077D00(); for two players maps the record's per-player byte through D_8008D55C into D_80102778.unk_4[i] (with a "
      "-3 adjustment for some values when unk_D == 0) and copies the second byte; copies p->unk14_4 / unk20_3 to unk_C / unk_E; "
      "for unk_D == 2 or 5 sets more cells (g_fight_match_count, g_fight_win_target, D_800A389A ...) from the record; "
      "func_800344B4()",
      "25C38.c:382 (the reset_mode path of func_80035828, after D_800A3834 = 0) and the asm of func_80035828",
      _WORK + "the tail call is ApplyPendingConfigAndInitializeMode's address (func_800344B4), the per-mode fields are "
      "visible in the two branches",
      "no fighter / mode identities are claimed; the table D_8008D55C is not named",
      "no_name.tsv (w64: 'mode-specific fields and mapping semantics not established enough for precise noun'; answered: the "
      "name says record, pending config and mode init only)"),

    R("0x80035280", "CopySavedFlagsButtonMappingsAndTimesToWorkingRecord", "func_80035280", "25788.c",
      "p = func_80077D00(); p->unk20_0..2 = D_80106A50.flags bits 0-2; p->unk17[i] and p->unk1D[i] = frec20[i]; for three "
      "clock records p->unk21[i] = minutes (/1800), seconds ((/30) % 60), hundredths ((% 30) * 100 / 30) and "
      "the saved times[i].unk_0 byte",
      "25C38.c:23 (func_80035438), 25C38.c:310 and the asm of func_80035438, func_80035828",
      _WORK + "the direction is saved to working; the time conversion is shown by the divisions",
      "no replay-data or card claim; what the three records are (best times) is not claimed",
      "no_name.tsv (w64: 'neither physical card operation nor replay data; full working-record identity remains uncertain'; "
      "answered: the name states the transfer and the three field groups)"),

    # ------------------------------------------------------------------------------------------ 63D2C.c
    R("0x80074220", "gpu_DrawThreeSheetsWithOptionalTileAndOpaqueQuads", "func_80074220", "63D2C.c",
      "(context, flag): when flag == 0 adds one semi-transparent TILE (colour via gpu_SetTileRgbFromBlockFlag) at OT 0x1E; "
      "adds the sprite cells of three sheets (gpu_AddSpriteCells) at OT 0x1F, a draw-mode primitive for the first sheet's tpage, "
      "and three flat POLY_F4 quads (gpu_SetPolyF4RgbFromBlockFlag) at OT 0x20",
      "64FD8.c:1181 (the SELWORK state's call, flag = SELWORK->f14.half[0]) and the asm of menu_RunSelPhases",
      "the effects are the primitives the body builds; gpu_ follows the primitive construction callees; its sibling 80074488 "
      "draws the cursor-dependent sheets",
      "no backdrop, frame or screen identity is claimed",
      "no_name.tsv (w64: 'screen identity not established from generic plumbing or geometry; menu_ would require tracing owning "
      "handler choices'; answered: gpu_, and no screen)"),

    R("0x80074488", "menu_DrawSheetsWithSelectedPulseOffsetDimmedOthersAndGreyTiles", "func_80074488", "63D2C.c",
      "mask from SELWORK->f3C[0], f65, f67, f66; for 15 sheets: selected ones get a colour pulsed by rsin(SELWORK->f34) and (for "
      "some index ranges, when the cursor row matches) the offset SELWORK->f40[0], the others the dim grey 0x40 (semi or "
      "coloured by index range); adds sheet cells (gpu_AddSpriteCells), a draw mode primitive and "
      "gpu_AddThreeGreyTilesStackedUp",
      "64FD8.c:1191 (SELWORK state) and the asm of menu_RunSelPhases",
      "it reads only SELWORK fields (the menu work), and its sibling renderer 80074220 draws the fixed sheets; menu_ is the "
      "family of SELWORK's cursor handlers (menu_HandleSelPhase0CursorInput at include/bb2.h:60)",
      "which screen or option group is drawn is not claimed",
      "no_name.tsv (w64: 'exact option groups not established; SELWORK name and nearby input handler alone are insufficient'; "
      "answered: the name states the pulse / offset / dim / grey tile effects and no group)"),

    # ------------------------------------------------------------------------------------------ 24BF0.c
    R("0x800343F0", "ApplyPendingConfigResetCellsAndClearSlotTimGroupRequests", "func_800343F0", "24BF0.c",
      "copies D_80102778.unk_D / unk_E / unk_C / unk_F into D_800A38DC / D_800A38BA / D_800A36A4 / D_800A3140; zeroes D_800A36F6, "
      "D_800A376A, D_800A376B, D_800A380C, D_800A37D3, D_800A37D2, sets D_800A38D4 = 2; "
      "slot_RequestTimGroupsAndMarkReloadOnChange(0, 0) and (1, 0)",
      "24BF0.c:42 (func_800344B4) and the asm of func_800344B4",
      _WORK + "the copies are the 'apply', the zeroed cells and the two slot requests (group 0 = no groups) the rest",
      "no fight-state claim for the cleared cells; the cells' roles are not named",
      "no_name.tsv (w64: 'fields and reset cells are incompletely explained; not solely a menu initializer'; answered: the name "
      "has no menu claim and lists the effect families)"),

    R("0x800344B4", "ApplyPendingConfigAndInitializeMode", "func_800344B4", "24BF0.c",
      "func_800343F0(); switch D_800A38DC: per case initialises the command stream (fight_InitCommandStream, "
      "fight_RunCommandStream), the opponent order (fight_BuildOpponentOrder, fight_AdvanceOpponentOrder) or the match "
      "(fight_AdvanceMatch, eff_Init) and sets handler / cells; clears the CPU tally (fighter_ClearCpuTally(1)) when "
      "unk_4[5] != 0",
      "24F08.c:178 (the config editor's confirm), 25788.c:107 (func_8003504C) and the asm of func_80034708, func_8003504C",
      _WORK + "both callers (the interactive editor and the working-record application) pass through it, so no menu owns it; "
      "the cases are initialisations by D_800A38DC",
      "no mode names (the case values stay unnamed)",
      "no_name.tsv (w64: 'calls from both interactive configuration and saved-record loading; menu_ would not cover every use'; "
      "answered: unprefixed)"),

    # ------------------------------------------------------------------------------------------ 24F08.c
    R("0x80034708", "menu_PrintEditAndApplyConfigWithPerPadCursors", "func_80034708", "24F08.c",
      "D_800A37B8++; prints 16 numeric fields (font_Printf with highlight D_800A3178 / D_800A3180 by the cursor D_800A3174[pad]) "
      "from D_80102778, D_80106A50.flags and four cells; per pad (two) moves the cursor with pad bits 0x1000 / 0x4000, edits the "
      "selected field with 0x8000 / 0x2000 (sounds 0 / 4) and wraps / masks each value; on 0x08000800 queues sound 1 and "
      "calls func_800344B4()",
      "d_7D870.c:33 (handler table entry 3, address-taken)",
      "the printing, cursor movement, editing, normalisation and the final apply call are the body; the two cursors are "
      "independent per pad; menu_ follows the cursor / selection handling of menu handlers and the font printing",
      "no screen identity (debug, practice, config) is claimed; field meanings stay unnamed",
      "no_name.tsv (w64: 'menu_ is admitted by actual cursor and selection handling; precise screen identity and several settings "
      "remain unproved'; answered: the name keeps menu_ and claims no screen or setting)"),

    # ------------------------------------------------------------------------------------------ 175A4.c
    R("0x80026DA4", "fight_UpdatePairedInputTotalsEventsAndMidpointEffects", "func_80026DA4", "175A4.c",
      "d = fighter_CalcSeparationErrorAndOpposedOffsets(); for the fixed pair of D_80101EC8: per move_kind adds unk_20 to the "
      "total unk_28C on pressed 0x20 / 0x40 and applies a held-direction drift to unk_134; every 0x2B frames compares the totals "
      "and sets pending_move_event 3 / 4 (and fighter_TryDropQueuedHeadAtBone, func_80032854), sets event 2 / 5 by distance, "
      "height and held bits; at the tail, unless D_800A3910 counts down, emits a midpoint effect (func_80032854) offset by the "
      "bearing",
      "17AFC.c:2230 and the asm of fight_UpdateFighterInteractionsAndPointCaches (its only caller)",
      "the callees are fighter_ and effect functions on the fixed pair D_80101EC8[0..1]; the caller is a fight_ function; "
      "'Events' are the pending_move_event writes",
      "no move / state identity (practice, contest, struggle) is claimed; D_800A3876 is only a selected index",
      "no_name.tsv (w64: 'exact shared action across state IDs and outcome codes remains opaque; practice/contest/struggle labels "
      "would be guesses'; answered: the name lists totals, events and midpoint effects and no identity)"),
]

DATA = []
