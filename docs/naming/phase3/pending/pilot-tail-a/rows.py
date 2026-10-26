"""wave88 rows (pilot-tail-a): Q132 mechanical names for 46 of the 54 neutral functions in the tail TUs main/31D3C
(the three-slot object table accessors, the texture-group / VRAM-row helpers and the light / background colour
setters), main/309CC (the slot-object build pipeline and its node-list helpers), main/32D04 (render-state helpers,
the draw-queue flush and the frame-entry setup), main/6CF8 (the saved-record flag getters and the boot helpers) and
main/4BC3C (the VAB close / load helpers).
Research input: tmp/codex/pilot-tail-a-research.md (46 RENAME, 8 NO_NAME over 54 targets; untrusted proposals).
Every row was re-derived from the C body and every caller (src/main/*.c, include/, asm/funcs/*.s, named_syms.txt,
symbol_addrs.txt); names changed against the research say CHANGED FROM RESEARCH in the evidence and in the row note.
Research NO_NAMEs (8; stay neutral, not rows): 8004283C 80042864 (the setter / getter of D_800F6650, whose only reader is
the uncalled getter, so the global has no role to name), 800444BC 800444E0 (instruction-identical forwarders of
func_80044504(D_800A378C)), 80044650 (forwarder of func_80052C10), 800168F0 80017F90 80017F98 (empty hooks).
Research RENAMEs dropped by this draft: none (all 46 kept, 42 of them renamed or reworded, see ADJUSTED).
Pre-apply review dropped 800430E4 and 80016D78 (no suggested name; they stay neutral) and replaced 6 names (NOTES): 44 rows.
Prefix evidence: slot_ is the vocabulary of the table D_800A9A10[3] itself (func_800404D8 clears its three entries,
func_80041584 searches them, 368E4.c's locals slot_obj / slot_index, node_GetSlotMatrixPointerArray); the research's
object_ is NOT used: object_ is the 17AFC.c prefix of the thrown-object pool (object_ResetPool, object_SpawnFromFighter,
object_UpdatePool ... over Obj80106A78), a different structure, and model_ was rejected as a guess (no_name.tsv: 'model_
requires whole-family terminology review'). node_ is the transform-node vocabulary of the same code
(node_UpdateWorldMatrix, node_RebaseParents, node_GetSlotMatrixPointerArray and the members node_by_id /
node_matrix_ptrs). gpu_ follows the callees (gpu_LoadTimGroup, gpu_AddDrawMove, gpu_SetDrawEnvBg ...) and the draw
environment / OT work of the bodies; snd_ follows SsVabClose / snd_LoadVabPack / func_800858D0 (the body of the existing
snd_StopAllVoices); fighter_ is the family of 4510C.c's CPU-control readers (fighter_InitCpuControl, fighter_BuildCpuPad), the only readers of func_800167BC; light_ is the TU's own
light_SetPointLight family over D_800F62E0; math_ is for the pure comparison function.
The research's data suggestions (g_object_slots, g_light_and_background_far_color_presets ...) need separate all-access
data manifests and are not part of this wave (DATA is empty).
"""
import csv
import glob
import os
import re

VERDICT = "CONFIRM"
VERIFIER_NOTE = "naming-reviewer pre-apply (round 1): 38/46 PASS as proposed; 6 FAIL (80041554, 80041650, 80041BF4, 80040510, 800167BC, 80016A8C) replaced by the reviewer-suggested names (NOTES); 2 FAIL with no suggestion dropped (800430E4: the name omits the depth-shift swap and leaves func_8004DDB4 unnamed; 80016D78: does not separate it from sys_Init)."
NOTES = {'0x80041554': 'suggested: naming-reviewer pre-apply round 1 failed slot_GetNdataIndex (unk_08 is the load id the slot was built from, not the ndata index: func_80044ED8 (35000.c:212-214) remaps ids >= 0x1F by -0x1B before the ndata read, and the same field indexes the 64-entry tables D_80094D40 / D_80094E08 / D_80094B48) and suggested slot_GetAssetIndex; R6 must judge it fresh at every use.',
    '0x80041650': "suggested: naming-reviewer pre-apply round 1 failed slot_GetLoadedTimGroups ('Loaded' unshown: 309CC.c:263-264 stores the unfiltered request D_80094B88 & 0x1F, while func_80041988 loads nothing for slot >= 2 and drops groups missing from D_80094D40[unk_08]; func_80041604 compares requests against it) and suggested slot_GetLastLoadTimGroupRequest; R6 must judge it fresh at every use.", '0x80041BF4': 'suggested: naming-reviewer pre-apply round 1 failed gpu_RestoreAndTintBackedUpFixedSlotVramRows (an effect missing: 31D3C.c:282-283 calls func_8003E120 (gpu_BuildFogImageLevels over 0x140,0x1E8, 2B344.c:1644-1651) when the fog-CLUT object is slot 1) and suggested gpu_RestoreTintBackedUpFixedSlotVramRowsAndFogLevels; R6 must judge it fresh at every use.',
    '0x80040510': 'suggested: naming-reviewer pre-apply round 1 failed slot_LoadObjectAndBuildNodesAndSoftbody (an effect missing: 309CC.c:150 calls func_8003E120 (gpu_BuildFogImageLevels) unconditionally) and suggested slot_LoadObjectBuildNodesSoftbodyAndFogLevels; R6 must judge it fresh at every use.',
    '0x800167BC': 'suggested: naming-reviewer pre-apply round 1 failed cpu_GetSavedBehaviorAdjustFlag (N4 prefix: every reader is in the fighter_ family (fighter_InitCpuControl, fighter_BuildCpuPad, 4510C.c); no cpu_ function family exists) and suggested fighter_GetSavedCpuBehaviorAdjustFlag; R6 must judge it fresh at every use.',
    '0x80016A8C': "suggested: naming-reviewer pre-apply round 1 failed gpu_ShowDiscImageFadeByHalvingColorsAndRestoreDisplayWidth (N4 wording: 'DiscImage' reads as the ISO/BIN; the body reads CD file (2,0x61) and shows the image in it (6CF8.c:196-200)) and suggested gpu_ShowCdFileImageFadeByHalvingColorsAndRestoreDisplayWidth; R6 must judge it fresh at every use."}

# names changed against the research (research name -> name used)
ADJUSTED = {
    "0x8004153C": ("object_GetSlot", "slot_GetObject",
                   "object_ is the 17AFC.c thrown-object pool prefix (another structure); the table's own vocabulary is slot_ "
                   "(D_800A9A10[3]); the function returns the object stored in a slot"),
    "0x80041554": ("object_GetSlotAssetSelector", "slot_GetAssetIndex",
                   "unk_08 is func_80045878's a1, which func_80044ED8 passes to ndata_ReadFromPack / ndata_Read as the pack index "
                   "('Selector' is not shown; the ndata subsystem is); prefix as 8004153C"),
    "0x80041584": ("object_FindEmptySlot", "slot_FindEmpty", "prefix as 8004153C"),
    "0x800415C4": ("object_ReleaseSlotAndResources", "slot_ReleaseObjectAndResources",
                   "prefix as 8004153C; it releases the object stored in the slot (softbody, VAB, both arena blocks) and the pointer"),
    "0x80041604": ("object_RequestTextureGroupsReload", "slot_RequestTimGroupsAndMarkReloadOnChange",
                   "the reload mark is conditional (only when the stored group bits differ); the groups are the TIM groups that "
                   "gpu_LoadTimGroup loads (i + 1 for bit i); prefix as 8004153C"),
    "0x80041650": ("object_GetLoadedTextureGroups", "slot_GetLastLoadTimGroupRequest",
                   "same TIM-group wording as 80041604; prefix as 8004153C"),
    "0x80041688": ("node_SelectObjectRenderHandlersAndPatchColor", "node_SetPrimitiveTableSelectOnObjectNodesAndPatchColor",
                   "'RenderHandlers' is not the table's shown role; what the asm shows is a node flag byte bit that picks one of two "
                   "primitive tables (func_8004A4E0 and func_800430E4 add 0xB0 to / swap the table pointer on it); the node set is the "
                   "transform nodes with a valid id plus the duplicate nodes, not 'the object' as a whole"),
    "0x80041988": ("gpu_LoadObjectRequestedTimGroups", "gpu_LoadRequestedAvailableTimGroupsForSlot",
                   "the loop loads only the groups present in BOTH the per-asset mask D_80094D40[a1] and the requested bits a2; the "
                   "slot (a0 < 2) selects the VRAM offsets"),
    "0x80041AC8": ("gpu_BackUpObjectVramRows", "gpu_BackUpFixedSlotVramRowsAndRecordAssetIndex",
                   "it returns unless arena_id == 1 (one fixed slot) and it records the asset index in D_800A9A20, which "
                   "func_80041BF4 compares before restoring"),
    "0x80041BF4": ("gpu_RestoreAndTintObjectVramRowCopies", "gpu_RestoreTintBackedUpFixedSlotVramRowsAndFogLevels",
                   "the object is always func_8004153C(1) (fixed slot) and only if its asset index equals the one recorded by the "
                   "backup; 'Copies' is the g_gpu_store_buf content"),
    "0x800421C8": ("ApplyLightAndBackgroundClearAndFarColorPreset", "ApplyLightPresetRowBackgroundClearAndFarColor",
                   "wording only: the argument selects a row of D_80094E48 (colour words, angle word, background word)"),
    "0x800422BC": ("light_SetSelectedSetupLightOrBackColor", "light_SetLightOrBackColorOfSelectedSetups",
                   "a2 selects a set of light setups (two together, or one), not one setup"),
    "0x80042478": ("gpu_SetBackgroundAndFarColorAndEnableClear", "gpu_SetBackgroundClearColorAndFarColor",
                   "wording: gpu_SetDrawEnvBg(1, r, g, b) sets isbg and the clear colour in both draw environments"),
    "0x80040304": ("gpu_QueueObjectSelectedDrawMoves", "gpu_QueueSlotDrawMovesForSelectedSet",
                   "a1 picks one of seven move sets (a mask of five, or two direct entries), the slot's per-asset mask D_80094B48 "
                   "filters them; prefix wording as 8004153C"),
    "0x80040400": ("node_AppendProjectedChild", "node_AppendProjectedNodeToList",
                   "'Child' hides that the new node is a member of the terminated list a1 (its fields match func_80040CB8's "
                   "projected_nodes entries); its parent a0[6] is not named"),
    "0x8004046C": ("node_GetObjectNodeById", "node_GetSlotNodeById",
                   "slot wording as node_GetSlotMatrixPointerArray (368E4.c:504); the lookup is node_by_id[a1] of the slot's object"),
    "0x800404A0": ("node_SetListProjectionData", "node_SetWordOnAllListedNodes",
                   "'ProjectionData' is a role of the word read through unk58 by the renderer, shown for this one caller's list but "
                   "not for the function (unk58 is polymorphic across node kinds); the body stores one word in every listed node"),
    "0x800404D8": ("object_ClearSlotsAndTextureRequests", "slot_ClearObjectPointersAndTextureRequests",
                   "prefix as 8004153C; it clears the object pointers D_800A9A10[] and the request words D_80094B88[]"),
    "0x80040510": ("object_LoadAssetsAndInitNodesAndSoftbody", "slot_LoadObjectBuildNodesSoftbodyAndFogLevels",
                   "prefix as 8004153C; it builds (allocates, stores in the slot, finalises) the object, and then its node lists, "
                   "softbody and the fog block"),
    "0x80040594": ("object_FinalizeAssetsSoundTexturesAndArena", "slot_FinalizeObjectLoadSoundTexturesAndArena",
                   "prefix as 8004153C; it finalises the pending load of the object"),
    "0x800408F8": ("node_InitObjectTransformHierarchyAndMatrixPointers", "node_InitObjectHierarchyFromTemplates",
                   "the nodes are built from the template records D_80094B94 (parent index, scaled position); the matrix-pointer "
                   "refresh is a nested call, and has its own name (80040A78)"),
    "0x80040A78": ("node_RefreshObjectMatrixPointerTables", "node_RefreshObjectMatrixPointerTables", ""),
    "0x80040B44": ("node_BindObjectGeometryIdsWithDuplicates", "node_BuildIdToNodeMapWithDuplicates",
                   "'Geometry' ids are only (id, node index) pairs of the asset here; the output is node_by_id (the member's name)"),
    "0x80040CB8": ("node_InitObjectProjectedChildren", "node_BuildProjectedNodeList",
                   "the output is the terminated list projected_nodes (the member's name), built for the template records whose id "
                   "is not -1; 'Children' is a role"),
    "0x800430E4": ("gpu_SelectLightSetupAndDispatchWithHalvedViewRow",
                   "gpu_CallWithHalvedViewYRowLightSetupAndPrimitiveTable",
                   "'Dispatch' is func_8004DDB4's role; what this body shows is the scratch matrix with row 1 and its translation "
                   "halved, D_800A3828 set to the arg2 light setup, and a primitive table chosen by the node flag bit before the call"),
    "0x80043244": ("math_ClassifySignedThresholdRange", "math_ClassifyThresholdBand",
                   "the three constants split the value into four bands (the comparisons are signed); no memory is touched"),
    "0x80043278": ("gpu_ComputeOtIndexWithScratchDepthShift", "gpu_RemapDepthToOtIndex",
                   "the emitter gpu_EmitOtCutoffRawTexturedTriangles computes the same remap from avsz3 and uses the result to index "
                   "the OT at 0x1F80000C; 'Scratch' is a place, not a role"),
    "0x80044504": ("gpu_PrepareViewAndFlushDrawQueue", "gpu_PrepareViewAndFlushDrawQueue", ""),
    "0x80044670": ("node_BindResourceMotionDataAndReserveEntries", "node_BindFrameDataAndReserveEntryBuffer",
                   "the data is a frame count plus 12-byte frames indexed by the entry progress (func_80044800); 'Motion' is the "
                   "separate fighter-motion subsystem (motion_StartPlayback); 'Resource' is only the selector call; nothing is "
                   "initialised, only the buffer end returned"),
    "0x8004473C": ("node_InitResourceMotionEntriesFromWorldTranslations", "node_InitFrameEntriesFromWorldTranslations",
                   "same 'Motion' / 'Resource' reasoning as 80044670"),
    "0x800167AC": ("gpu_GetSavedGrayscaleOption", "gpu_GetSavedGrayscaleFlag",
                   "'Option' is a role the code does not show for every bit (it is a flag byte bit of the saved record, toggled by "
                   "24F08.c and copied by 25788.c); 'Saved' is the record's save-block copy"),
    "0x800167BC": ("cpu_GetBehaviorAdjustmentOption", "fighter_GetSavedCpuBehaviorAdjustFlag", "same 'Option' / 'Saved' reasoning as 800167AC"),
    "0x800167D4": ("snd_GetCdAudioSuppressionOption", "snd_GetSavedCdAudioSuppressFlag",
                   "same 'Option' / 'Saved' reasoning as 800167AC; this bit is only loaded from the save block (25788.c), no toggle "
                   "was found"),
    "0x800167EC": ("InitSavedRecordAndResetCameraDistance", "InitSavedRecordAndResetCameraDistanceOffset",
                   "the cleared variable is g_camera_distance_offset (9F9C.c:680-695 clamps and adds to it), and the record init "
                   "includes func_8001945C's frec20 bytes"),
    "0x80016A8C": ("gpu_ShowTimedDiscImageAndHalvePixels", "gpu_ShowCdFileImageFadeByHalvingColorsAndRestoreDisplayWidth",
                   "the halving of the three 5-bit colour fields is repeated every frame from a threshold on, a fade; the body also "
                   "restores the display width at its end"),
    "0x80016D78": ("InitRuntimeResourcesAndState", "InitRuntimeResourcesAndState", ""),
    "0x800174F4": ("gpu_DrawAuxiliaryDisplayAndAdvanceScript", "gpu_DrawDisplayModeOnLocalOtAndSync",
                   "'Auxiliary' / 'Script' are roles the body does not show; it switches on the display mode D_800A3768, draws into a "
                   "local two-entry OT in its own draw environment and syncs"),
    "0x8005B644": ("snd_CloseObjectVab", "snd_StopAllVoicesAndCloseSlotVab",
                   "it first stops all voices (func_800858D0(0), the body of snd_StopAllVoices); the VAB id arg * 3 + 1 is the one "
                   "func_80040594 loads for the slot; slot wording as 8004153C"),
    "0x8005B6AC": ("snd_CloseSelectedEmbeddedVabs", "snd_StopAllVoicesAndCloseSelectedEntryVabs",
                   "voices are stopped first; the two VABs are the ones func_80049C24 loads into arena block 6 from the selected "
                   "entries (D_800A33E8), freed together by func_80046020"),
    "0x8005B72C": ("snd_CloseNonCommonVabsAndResetVoicesQueueAndReverb", "snd_CloseNonCommonVabsAndResetVoicesQueueAndReverb", ""),
    "0x8005B868": ("snd_CloseSequentialFileVabPair", "snd_StopAllVoicesAndClosePairedFileVabs",
                   "voices are stopped first; the two VABs are the ones func_8005B8B8 loads from two CD files"),
    "0x8005B8B8": ("snd_LoadConsecutiveFileVabPair", "snd_ClosePairedFileVabsThenLoadFromConsecutiveFiles",
                   "it first calls the closer 8005B868; the file indices are consecutive (cdrom_GetFileIndex(2, n), (2, n + 1))"),
    "0x8005B9C4": ("snd_ResetVoicesAndCloseFixedVab", "snd_StopAllVoicesAndCloseSingleFileVab",
                   "'Reset' wording is snd_ResetVoicesAndQueue's (it also clears the queue); this one only stops voices, and the VAB "
                   "is the one func_8005B9FC loads from a single CD file"),
    "0x8005B9FC": ("snd_ReadFileAndReplaceVab", "snd_CloseThenReadSingleFileAndLoadVab",
                   "the first act is the closer func_8005B9C4 (which also stops the voices); the body then reads one CD file and "
                   "loads the VAB from it"),
    "0x8005BA8C": ("snd_LoadAndCompactSelectedVabsAndInstallBillboards", "snd_ReloadListedVabsFromFileAndInstallBillboards",
                   "the loaded ids are g_vab_id_list (the TU's snd_CloseListedVabs / snd_RebaseListedVabs vocabulary); compaction is "
                   "part of the reload"),
    "0x8005C614": ("snd_ResetVoicesAndSetMaxMasterVolumeStereoAndClearAutoKeyOffMode",
                   "snd_StopVoicesSetMaxMasterVolumeStereoAndClearAutoKeyOffMode",
                   "'Reset' wording as 8005B9C4: func_800858D0(0) is the stop-all-voices call"),
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


def L(f, pat, nth=0):
    """src/main/<f>:<line> of the nth line matching regex pat."""
    hits = [k + 1 for k, l in enumerate(_SRC[f]) if re.search(pat, l)]
    if len(hits) <= nth:
        raise SystemExit(f"{f}: no line {nth} matching {pat}")
    return f"src/main/{f}:{hits[nth]}"


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


_SLOT = ("Slot = an index of D_800A9A10[3] (func_800404D8 clears its three entries, func_80041584 scans i < 3, "
         "func_80045878 / func_80040510 store the object); the object is the Unk80045878Obj the table holds. ")

FUNCS = [
    # ------------------------------------------------------------------------------------------ 31D3C.c
    R("0x8004153C", "slot_GetObject", "func_8004153C", "31D3C.c",
      "returns D_800A9A10[a0]",
      "17 C call sites in 2B344.c, 309CC.c, 31D3C.c, 368E4.c, 43500.c, 4510C.c and the asm of the callers listed under HOMES; "
      "every argument is a slot index (a literal 0 / 1, a loop index, g_gpu_fog_clut_object, flags >> 1 at 368E4.c) and the result "
      "is always used as an Unk80045878Obj",
      _SLOT + "Get = the plain load; nothing is checked or created",
      "no player, fighter or model claim; the 17AFC.c object_ family (Obj80106A78) is a different structure",
      "w64 deferred 'model/object vocabulary'; w5 player_ and w10 get_global stay rejected"),

    R("0x80041554", "slot_GetAssetIndex", "func_80041554", "31D3C.c",
      "returns D_800A9A10[a0]->unk_08, or -1 when the slot is empty",
      "none (no C call, no jal, no address word)",
      _SLOT + "unk_08 is func_80045878's a1 (35000.c:556, stored with the slot's arena id), the value it passes to func_80044ED8 "
      "(35000.c:543 / 545), which reads pack entry a0 (a0 >= 0x1F: minus 0x1B) via ndata_ReadFromPack / ndata_Read (35000.c:212-219), "
      "and the index of D_80094D40 / D_80094E08 / D_80094B48 / D_80094C68 in 31D3C.c and 309CC.c",
      "the entry's content (character, model) is not named; the function is uncalled, so no caller disagrees",
      "w64 'exact asset/character/model identity requires cross-family proof'; the index role is shown by func_80044ED8"),

    R("0x80041584", "slot_FindEmpty", "func_80041584", "31D3C.c",
      "returns the first i < 3 with D_800A9A10[i] == 0, else -1",
      "none (no C call, no jal, no address word)",
      _SLOT + "Find the first empty entry, -1 when none",
      "no player identity (w5 player_find_empty_slot reset); the count 3 stays out of the name",
      "w64 'no admitted exact subsystem noun for these three slots'; slot_ is the table's own word"),

    R("0x800415C4", "slot_ReleaseObjectAndResources", "func_800415C4", "31D3C.c",
      "softbody_FreeObjectById(a0); func_80045A50(a0); D_800A9A10[a0] = 0",
      "25C38.c:41-42 / 401-402, 2B344.c:594-595 / 650-651, 368E4.c:407-408 (always slot 0 then 1) and the asm of func_80035480, "
      "func_80035DC8, func_8003BE10, func_8003BFC4, func_80046AA0",
      _SLOT + "func_80045A50 (35000.c:571-577) closes the slot's VAB (func_8005B644), clears the move callbacks of both arena blocks "
      "a0 and a0 + 3 and frees both; the softbody free comes first; the pointer is cleared last",
      "the effect is not only 'clear slot'; no player / fighter claim (w5 player_Destroy stays rejected)",
      "w64 'graph and sound/asset side effects make a simple slot-clear name incomplete'"),

    R("0x80041604", "slot_RequestTimGroupsAndMarkReloadOnChange", "func_80041604", "31D3C.c",
      "if the slot's object exists and (unk_00.word >> 16) & 0x1F != a1 sets load_status = -2; always D_80094B88[a0] = a1",
      "24BF0.c:24-25, 2B344.c:244 / 257 / 354 / 419-420, 9F9C.c:1797 and the asm of fight_Init, fight_RunCommandStream and "
      "func_800343F0 (a value of 0 or a stored word; the value is only a bit set)",
      _SLOT + "D_80094B88[slot] is the third argument of func_80041988 (309CC.c:259-260), where bit (0x10 >> i) selects TIM group "
      "i + 1 (gpu_LoadTimGroup, 31D3C.c:166-177); func_80040594 stores the set into bits 16..20 of unk_00 and clears the request "
      "(309CC.c:263-265); func_80045878 reloads when load_status == -2 (35000.c:535-536)",
      "no character id (w5 player_SetCharId stays rejected) and no immediate visibility change; group contents are not named",
      "w64 'precise groups remain opaque and request also changes load state'; both effects are now named"),

    R("0x80041650", "slot_GetLastLoadTimGroupRequest", "func_80041650", "31D3C.c",
      "returns (D_800A9A10[a0]->unk_00.word >> 16) & 0x1F, or -1 without an object",
      "none (no C call, no jal, no address word)",
      _SLOT + "bits 16..20 of unk_00 are written only by func_80040594 (309CC.c:263-264) with the request set that "
      "func_80041604 stored, i.e. the set that was loaded; func_80041604 compares against the same field",
      "no group identity; uncalled",
      "w64 'no current caller and group identities remain unresolved'"),

    R("0x80041688", "node_SetPrimitiveTableSelectOnObjectNodesAndPatchColor", "func_80041688", "31D3C.c",
      "return if the slot is empty; set (arg1) or clear bit 0 of the flag byte unk1 of transform_nodes[1], of transform_nodes[2..17] "
      "whose unk2 >= 0, and of every duplicate node (the +0x10D5 walk ends at the node whose unk58 is 0, duplicate_nodes at "
      "+0x10D4); then the colour bytes of unk_18 (math_Grayscale3 when gpu_RefreshGrayscaleMode()) go through func_80041398",
      "28708.c:1295-1296, 2B344.c:526-527 / 706-707 / 1026-1027, 9F9C.c:865-866 / 1840-1841 / 1974-1978 / 2093-2094 and the asm of "
      "camera_ApplyFightView, fight_Init, func_8001B6F4, func_8001EA04, func_8003B9D0, func_8003C040, func_8003CE18, replay_Start "
      "(always (slot, 0 or 1))",
      "the flag byte unk1 bit 0 is read by func_8004A4E0 (asm lines 106-116: adds 0xB0 to the table pointer D_80095328 for the node) "
      "and by func_800430E4 (32D04.c:502-506: D_80095280 vs D_800951D8); the tables are the primitive handler tables installed at "
      "0x1F80001C; func_80041398 (31548.c:266-289) rewrites the colour immediates of the emitter code at 0x800545F4 ff. through "
      "D_80015820",
      "no hide / show or ground claim; the table's content is not named",
      "w64 'hide/show is unproved without flag consumer verification and would omit colour patching'; the consumers are now shown"),

    R("0x80041988", "gpu_LoadRequestedAvailableTimGroupsForSlot", "func_80041988", "31D3C.c",
      "returns unless a0 < 2; mask = D_80094D40[a1]; for bit 0x10 down to 1 (i = 0..4) with the bit in both mask and a2: slot 0 "
      "calls gpu_LoadTimGroupSplitOffset when gpu_GetFogClutObject() == 0 else gpu_LoadTimGroup, slot 1 the Offset / SplitOffset "
      "variants (group i + 1 of the data a3)",
      "309CC.c:259 (func_80040594) and the asm of func_80040594: (arena_id, unk_08, D_80094B88[arena_id], the TIM data)",
      "gpu_ follows the callees gpu_LoadTimGroup / gpu_LoadTimGroupOffset / gpu_LoadTimGroupSplitOffset; the request bits are "
      "func_80041604's",
      "no allocation (w5 AllocRobRmd_helper stays rejected) and no character or group-content identity",
      "w64 'mask/group identities and exact object family require fuller asset review'; the bit semantics are shown by the callees"),

    R("0x80041AC8", "gpu_BackUpFixedSlotVramRowsAndRecordAssetIndex", "func_80041AC8", "31D3C.c",
      "return unless arg0->arena_id == 1 and D_80094E08[arg0->unk_08] != 0xFF; D_800A9A20 = unk_08; walk the list "
      "D_80094DF0[D_80094E08[unk_08]] ({x, y} pairs ending in a negative x), StoreImage of each 16 x 1 row (offset by the "
      "fog-clut object) into g_gpu_store_buf[i]; DrawSync",
      "309CC.c:254 (func_80040594, case 1 of its arena-id switch) and the asm of func_80040594",
      "the backup is read back by func_80041BF4 (LoadImage of g_gpu_store_buf[idx++] over the same list, after checking "
      "obj->unk_08 == D_800A9A20); 'rows' are the 16 x 1 rectangles; the fixed slot is the arena_id == 1 test",
      "no palette / CLUT claim (the rectangle width does not prove it)",
      "w64 'CLUT interpretation plausible from row width but not enough independently verified table data'"),

    R("0x80041BF4", "gpu_RestoreTintBackedUpFixedSlotVramRowsAndFogLevels", "func_80041BF4", "31D3C.c",
      "obj = func_8004153C(1); return if empty, if unk_08 != D_800A9A20, or if D_80094E08[unk_08] == 0xFF; r / g / b = (a << 12) / "
      "255 (grayscale-converted when gpu_RefreshGrayscaleMode()); for both VRAM layouts LoadImage each row of the list from "
      "g_gpu_store_buf and gpu_TintVramRowInPlace it with (r, g, b); when the fog-clut object is slot 1 calls func_8003E120",
      "2B344.c:531, 9F9C.c:1352 / 1843 / 1845 / 1849 and the asm of fight_Init, fight_UpdateDefeatDelayAndModeProgress, "
      "func_8003B9D0 (colour arguments 0..255)",
      "restores what func_80041AC8 stored (same list, same buffer, same record check) and tints it; gpu_ follows the callees "
      "(LoadImage, gpu_TintVramRowInPlace)",
      "not an arbitrary VRAM tint (it needs the backup) and no fighter-palette claim",
      "w64 'not arbitrary VRAM tint and not proven exclusively fighter palette data'"),

    R("0x800421C8", "ApplyLightPresetRowBackgroundClearAndFarColor", "func_800421C8", "31D3C.c",
      "p = D_80094E48[a0]; func_800422BC(a0, p[0..3], ...) four times; yaw / pitch (low 12 bits of p[4] and of p[4] >> 16) stored in "
      "light[0] of D_800F62E0 setups 4, 1 and 0; the fifth word gives yaw / pitch and func_80042478(the sixth word) sets the background clear and far colour",
      "368E4.c:546 and the asm of func_80046EDC",
      "the argument indexes the 38 rows of D_80094E48 (31D3C.c:533 ff.); the last call enables screen clearing (isbg = 1), the "
      "reason wave64's R6 rejected light_ApplyPreset; the name states both effects and has no light_ prefix because the body "
      "also sets the draw environment and far colour",
      "no stage or time-of-day claim",
      "w64 R6 rejected light_ApplyPreset (omitted background clear); not reused"),

    R("0x800422BC", "light_SetLightOrBackColorOfSelectedSetups", "func_800422BC", "31D3C.c",
      "unpacks r / g / b from arg1 (grayscale-converted when gpu_RefreshGrayscaleMode()); a3 == 0: scales to Q12 into cmat.m[.][0] "
      "and calc_LightMatrix of D_800F62E0[0] and [1] (a2 == 0) or of [4] (a2 != 0); a3 != 0: stores the byte colour into back[] of "
      "[0] and [1] (a2 == 0) or [4]",
      "31D3C.c:430-433 (func_800421C8, once per (a2, a3) pair) and the asm of func_800421C8",
      "light_ follows calc_LightMatrix, cmat and the TU's light_ functions over D_800F62E0; the selected setups are the two groups "
      "above",
      "no directional / primary-light claim and no setup number in the name",
      "w64 'precise group identity and directional-light role are not sufficiently named'"),

    R("0x80042478", "gpu_SetBackgroundClearColorAndFarColor", "func_80042478", "31D3C.c",
      "unpacks r / g / b from a0 (grayscale-converted when gpu_RefreshGrayscaleMode()); gpu_SetDrawEnvBg(1, r, g, b); SetFarColor(r, g, b)",
      "31D3C.c:442 (func_800421C8) and the asm of func_800421C8",
      "gpu_SetDrawEnvBg (6CF8.c:105-112) sets isbg and the RGB of both draw environments; SetFarColor is Sony's",
      "no fade or progression claim (w5 disp_set_fade_color stays rejected)",
      "no_name.tsv: free"),

    # ------------------------------------------------------------------------------------------ 309CC.c
    R("0x80040304", "gpu_QueueSlotDrawMovesForSelectedSet", "func_80040304", "309CC.c",
      "returns without an object; a1 0..5 picks a five-bit mask, a1 == 6 queues two entries directly; mask &= D_80094B48[unk_08]; "
      "for each remaining bit gpu_AddDrawMove(a0, 4 - i)",
      "28708.c:1454, 4510C.c:166, 9F9C.c:5221 and the asm of fighter_Update, motion_UpdatePlayback, replay_PlayFrame",
      "gpu_AddDrawMove (309CC.c:20-48) adds a DR_MOVE from g_gpu_draw_move_table; the per-asset mask is the TU's data D_80094B48",
      "no damage / weapon / face region claim",
      "w64 'selector and region meanings not established'"),

    R("0x80040400", "node_AppendProjectedNodeToList", "func_80040400", "309CC.c",
      "advances a1 to the entry with unk2 == -1 and fills it (unk2 = 2, unk0 = 3, unk1 = 0, parent a0[6].node, matrix_state 1, "
      "rotation_order 0, light_setup_index 0, unk4 = a2, unk58 = 0), then ends the list with a1[1].unk2 = -1",
      "none (no C call, no jal, no address word)",
      "the same field writes as func_80040CB8's projected_nodes entries (309CC.c:432-440) and the same terminator; "
      "node_ follows the transform-node vocabulary",
      "no bone / weapon identity; uncalled, so no caller disagrees",
      "w64 'no caller and opaque render-kind/index semantics'"),

    R("0x8004046C", "node_GetSlotNodeById", "func_8004046C", "309CC.c",
      "returns func_8004153C(a0)->node_by_id[a1]",
      "none (no C call, no jal, no address word)",
      "node_by_id is filled by func_80040B44 (309CC.c:396-409) with the original or the duplicate node of each id; the slot "
      "wording is node_GetSlotMatrixPointerArray's",
      "no bone claim; no NULL check is promised",
      "w64 'no caller'"),

    R("0x800404A0", "node_SetWordOnAllListedNodes", "func_800404A0", "309CC.c",
      "returns if the first entry has unk2 == -1; else stores a1 into unk58 of every entry up to the unk2 == -1 terminator",
      "31548.c:183 (func_80040D48), passing s4->projected_nodes and its arg5, which is also stored in unk_1A84 and passed to "
      "softbody_SetObjectGround; and the asm of func_80040D48",
      "the word is written in every listed node; what the renderer does with it is not named (unk58 is polymorphic: the "
      "duplicate nodes hold a node address, the projected nodes a word)",
      "no ground-height or projection claim; the word's role is left out on purpose",
      "w64 'unk58 is polymorphic across node kinds'"),

    R("0x800404D8", "slot_ClearObjectPointersAndTextureRequests", "func_800404D8", "309CC.c",
      "for i < 3: D_800A9A10[i] = 0 and D_80094B88[i] = 0",
      "368E4.c:430 and the asm of func_80046B44",
      _SLOT + "D_80094B88[] is the request set of func_80041604; the pointers are released elsewhere (func_800415C4), this clears only",
      "does not release resources",
      "w64 'object ownership and mask semantics need broader model-family review'"),

    R("0x80040510", "slot_LoadObjectBuildNodesSoftbodyAndFogLevels", "func_80040510", "309CC.c",
      "func_80045878(a0, a1, a2) allocates or reuses the object; D_800A9A10[a0] = it; func_80040594 (finalise the load), "
      "func_800408F8 (nodes from templates), func_80040B44 (id map), softbody_SetupObject, softbody_RequestRebuild, func_80040CB8 "
      "(projected list), func_8003E120 (fog block); returns the object",
      "2B344.c:171 (func_8003AF40), 9F9C.c:1328 / 1813 / 1816 / 1818 and the asm of fight_Init, "
      "fight_UpdateDefeatDelayAndModeProgress, func_8003AF40",
      _SLOT + "the pipeline is generic: arguments are the slot, the pack index and an optional destination",
      "not fighter-only (the name has no fighter_), not the retired player_rob_Init",
      "w64 'covers more than fighter records, so fighter_ is unsafe; model_ requires whole-family review'"),

    R("0x80040594", "slot_FinalizeObjectLoadSoundTexturesAndArena", "func_80040594", "309CC.c",
      "returns unless load_status != 0, then clears it; closes the slot's VAB (func_8005B644) unless the pack is loaded and slot 1; "
      "for packs with at least six entries loads the VAB pack (snd_LoadVabPack, id arena_id * 3 + 1) and may move the block; "
      "relocates two blocks (reloc_RelocateBlock), sets softbody_params; loads / offsets TIM groups per slot, backs up VRAM rows "
      "(func_80041AC8), calls func_80041988, stores the loaded group bits, DrawSync and resizes the arena block (func_80045A28)",
      "309CC.c:144 (func_80040510) and the asm of func_80040510",
      "load_status is set by func_80045878 and cleared here; the callees are the sound, relocation, texture and arena functions "
      "named in the body",
      "not texture-only or relocation-only",
      "w64 'texture-only or relocation-only name would omit sound registration and allocation effects'"),

    R("0x800408F8", "node_InitObjectHierarchyFromTemplates", "func_800408F8", "309CC.c",
      "position_scale = D_80094C68[unk_08] (0x1000 outside its list); for each of 21 template records D_80094B94.recs[i] "
      "initialises transform_nodes[i] (parent = nodes[recs[i].unk_00] or none, translation = recs[i].unk_02 * scale >> 12, zero "
      "rotation, matrix_state 0, light_setup_index = unk_10, unk4 = unk_14); then func_80040A78",
      "309CC.c:145 (func_80040510) and the asm of func_80040510",
      "node_ follows the transform nodes (Unk80045878Node.node is an Unk80101DF0Record whose matrix node_UpdateWorldMatrix "
      "derives from parent_node); the matrix-pointer refresh is the nested func_80040A78",
      "no skeleton / bone identity, though the parent links form a tree",
      "w64 'skeleton/bone identity plausible but not independently established'"),

    R("0x80040A78", "node_RefreshObjectMatrixPointerTables", "func_80040A78", "309CC.c",
      "unk_18F4[0..19] = &transform_nodes[k].node.xf.mat for a fixed permutation k; node_matrix_ptrs[0..19] = &transform_nodes[1 + i]"
      ".node.xf.mat",
      "309CC.c:336 (func_800408F8), 31548.c:322 (func_80041430, after relocation) and the asm of func_800408F8, func_80041430",
      "the tables are the object's matrix-pointer arrays (node_GetSlotMatrixPointerArray returns node_matrix_ptrs); the function "
      "rewrites them after the nodes are built or moved",
      "no mapping-content identity (what the permutation means is not named)",
      "w64 'object-specific mapping identities remain opaque'"),

    R("0x80040B44", "node_BuildIdToNodeMapWithDuplicates", "func_80040B44", "309CC.c",
      "clears node_by_id[0..19]; reads the (id, node index) pairs of the asset block (word 2) up to 0xFFFF: the first use of a node "
      "stores the id in the original node's unk2 and the original in node_by_id[id]; a repeated node is copied into "
      "duplicate_nodes with unk2 = id and unk58 = the original; ends the duplicate list with unk58 = 0",
      "309CC.c:146 (func_80040510) and the asm of func_80040510",
      "node_by_id is the member's name; func_8004046C reads it; func_80041430 rebases the duplicate unk58 values",
      "no bone or geometry identity",
      "w64 'exact asset mapping terminology not established'"),

    R("0x80040CB8", "node_BuildProjectedNodeList", "func_80040CB8", "309CC.c",
      "for the first 18 template records with id (unk_08) != -1 appends a node to projected_nodes (unk2 = id, unk0 = 3, unk1 = 0, "
      "parent = transform_nodes[1 + i], matrix_state 1, light_setup_index 0, unk4 = unk_16, unk58 = 0) and ends the list with "
      "unk2 = -1",
      "309CC.c:149 (func_80040510) and the asm of func_80040510",
      "projected_nodes is the member's name; 31548.c:170-180 queues exactly this list on g_draw_queue_cursor until unk2 == -1",
      "no shadow / weapon identity",
      "w64 'render-kind and table identity still opaque'"),

    # ------------------------------------------------------------------------------------------ 32D04.c
    R("0x80043244", "math_ClassifyThresholdBand", "func_80043244", "32D04.c",
      "returns 3 if a0 > 0x16A09, 2 if a0 > 0xB500, 1 if a0 >= 0x5A01, else 0",
      "none (no C call, no jal, no address word)",
      "pure comparisons of the signed argument against three ascending constants; no memory, no calls; math_ is the pure-function "
      "prefix (math_ClassifyAbsMagnitude is the unsigned-magnitude sibling at 17AFC.c:37)",
      "no magnitude or depth identity; the thresholds stay out of the name",
      "no_name.tsv: free"),

    R("0x80043278", "gpu_RemapDepthToOtIndex", "func_80043278", "32D04.c",
      "v0 = a0 >> scratch word 0x1F800008; returns ((v0 & 0x7FF | -0x1000) >> (v0 >> 11)) & 0xFFF",
      "none (no C call, no jal, no address word)",
      "gpu_EmitOtCutoffRawTexturedTriangles (asm lines 73-90) shifts the avsz3 average depth by the same scratch word, applies the "
      "same exponent / mantissa remap, compares it against 0x1F800018 and uses it to index the OT at 0x1F80000C; this body is the "
      "remap alone (arithmetic shifts where the emitter's are logical)",
      "reads renderer state, so not math_; uncalled",
      "no_name.tsv: free"),

    R("0x80044504", "gpu_PrepareViewAndFlushDrawQueue", "func_80044504", "32D04.c",
      "builds D_80101BD0 (math_RotMatrixZXY) and the view matrices (MulMatrix, MulMatrix2, MulMatrix0 into D_800FF610); sets the "
      "scratch words (0x1F800014, 0x1F80001C, 0x1F80000C = a0, 0x1F800010, 0x1F800018); func_8004A4E0 (walks g_draw_queue); "
      "gpu_SetRaisedOtCutoffRequested(1); g_draw_queue_cursor = g_draw_queue",
      "32D04.c:1119 (func_800444BC), 1121 (func_800444E0) and the asm of both",
      "func_8004A4E0 walks the queue (retired note 'walks g_draw_queue'); the cursor reset empties it; the argument is the OT "
      "D_800A378C both wrappers pass",
      "not pure matrix math",
      "no_name.tsv: free"),

    R("0x80044670", "node_BindFrameDataAndReserveEntryBuffer", "func_80044670", "32D04.c",
      "D_800A9CF8.unk0 = a1, unk2 = *a0++ (frame count), unk8 = a0 (frames), unkC = a2 (entry buffer), unk4 = "
      "resource_GetLoadedSelector(); unk6 = entry count for selectors 7 / 4 / 0x12; returns a2 + unk6 * 0x68",
      "368E4.c:151 (inside func_800460E4)",
      "func_80044800 (35000.c:11-100) reads D_800A9CF8.unk8 + progress * 12 as six s16 (position, rotation) per frame, unk2 as the "
      "frame count, unk6 as the entry count and unkC as the entry array; entries are 0x68-byte nodes",
      "no stage identity; not the fighter motion_ subsystem; nothing is initialised here",
      "no_name.tsv: free"),

    R("0x8004473C", "node_InitFrameEntriesFromWorldTranslations", "func_8004473C", "32D04.c",
      "src = func_8003F1D4() (D_800A6690, the grid node records) stored in D_800A9CF8.unk10; for i < unk6 initialises the "
      "0x68-byte entries at unkC (zero flags, unk4 = D_800A9CF8.unk0, no parent, light_setup_index 4, zero rotation, translation = "
      "src[i].node.xf.mat.t, matrix_state 0, unk58 = -1)",
      "368E4.c:654 / 662 / 727 (func_8004746C, func_800474F0, func_8004777C) and the asm of those three",
      "the entries are the ones func_80044670 reserved; func_80044800 advances them from unk58 = 0 (func_80044B30 starts one when "
      "unk58 == -1) and writes the world matrices from the frame data",
      "distinct from the reservation and from the per-frame playback; no effect or stage identity",
      "no_name.tsv: free"),

    # ------------------------------------------------------------------------------------------ 6CF8.c
    R("0x800167AC", "gpu_GetSavedGrayscaleFlag", "func_800167AC", "6CF8.c",
      "returns D_80106A50.flags & 1",
      "368E4.c:1397 (gpu_RefreshGrayscaleMode) and the asm of gpu_RefreshGrayscaleMode",
      "gpu_RefreshGrayscaleMode copies it into g_color_mode, whose consumers gray colours (the gpu_RefreshGrayscaleMode() tests at 31D3C.c:110 / "
      "260 / 453 / 493); 24F08.c:62 / 113 / 153 shows and toggles the bit; 25788.c:18 / 119 serialises it; so 'Saved' (the record "
      "is in the save block) and 'Grayscale' are both read off the uses",
      "no file-I/O claim (w3 file_GetFlag0 stays rejected); no bit number in the name",
      "no_name.tsv: free"),

    R("0x800167BC", "fighter_GetSavedCpuBehaviorAdjustFlag", "func_800167BC", "6CF8.c",
      "returns (D_80106A50.flags >> 1) & 1",
      "4510C.c:324 / 711 / 1682 / 1836 and the asm of fighter_BuildCpuPad, fighter_InitCpuControl, func_80058580",
      "all four readers are CPU-control code: with the flag (and D_800A38DC != 3) unk_438 is scaled by 11 / 16 (4510C.c:324-325), "
      "a history block is skipped (:711), the action choice is inverted (:1682) and a branch is gated (:1836); 24F08.c:64 / 116 "
      "toggles it, 25788.c:19 / 120 serialises it",
      "no easy / difficulty identity; the mode conditions stay with the callers",
      "w64 'bit extraction alone does not establish the option's meaning'; the CPU-only readers are now shown"),

    R("0x800167D4", "snd_GetSavedCdAudioSuppressFlag", "func_800167D4", "6CF8.c",
      "returns (D_80106A50.flags >> 2) & 1",
      "9F9C.c:1700 (snd_StartFightCdAudio), 1737 (snd_ShouldUpdateStageSoundSlots) and the asm of both",
      "snd_StartFightCdAudio returns before every cdrom_StartAudioEntry when it is set; snd_ShouldUpdateStageSoundSlots returns it "
      "for modes 2..4 (and 1 otherwise); 25788.c:20 / 121 serialises the bit; no toggle of it exists in 24F08.c",
      "does not assert a universal stage-sound policy; 'Suppress' is the CD-audio start it prevents",
      "w64 'it suppresses fight CD audio but also supplies a mode-dependent predicate'; both users are sound paths"),

    R("0x800167EC", "InitSavedRecordAndResetCameraDistanceOffset", "func_800167EC", "6CF8.c",
      "g_camera_distance_offset = 0; D_80106A50.flags = 0, unk_00 = 0x7007, unk_04 = 0, times[0..2] = {0, 0, 0x1A5E0}, then "
      "times[0].unk_4 = 0x6978; func_8001945C() writes the three frec20 bytes",
      "6CF8.c:277 (func_80016D78) and the asm of func_80016D78",
      "D_80106A50 is the FileRecord that 25788.c serialises into and from the save block; g_camera_distance_offset is the "
      "variable 9F9C.c:680-695 adjusts and clamps",
      "no invented file-I/O subsystem",
      "w64 'exact scope of FileRecord and D_800A3710 remains insufficiently explained'"),

    R("0x80016A8C", "gpu_ShowCdFileImageFadeByHalvingColorsAndRestoreDisplayWidth", "func_80016A8C", "6CF8.c",
      "SetDispMask(0); SetDefDispEnv(.., 0x140 x 0xF0); reads CD file (2, 0x61) into arg0 (cdrom_StartRead); PutDispEnv; LoadImage of "
      "the rect D_800A30D4 from arg0 + 0x14; SetDispMask(1); 0x96 frames of DrawSync / VSync, from frame 0x79 on halving each 5-bit "
      "colour field of the w x h pixels and reloading; then SetDefDispEnv(.., 0x280 x 0xF0) and SetDispMask(1)",
      "6CF8.c:436 (main, once, before the main loop) and the asm of main",
      "the reloaded halved pixels darken each frame, so the image fades; the final SetDefDispEnv restores the display width",
      "no menu / input / image identity",
      "w64 'image identity is not established by code; no input handler, so menu_ is inadmissible'"),

    R("0x800174F4", "gpu_DrawDisplayModeOnLocalOtAndSync", "func_800174F4", "6CF8.c",
      "returns when D_800A3768 == DISP_DISABLED; SetDefDrawEnv / PutDrawEnv (isbg = 0) with the buffer half of D_800A36AC; local "
      "two-entry OT cleared by ClearOTagR; switch on D_800A3768: cases 1 / 2 call func_8005D46C and func_8005D554 (random "
      "repeats), case 10 func_8005E54C, case 20 steps the list D_800A37A8 and calls func_80060414; DrawOTag; DrawSync",
      "2B344.c:489, 9F9C.c:1757 / 1764 / 1788, cdrom.c:68 and the asm of cdrom_WaitIdle, fight_Init, "
      "fight_WaitForCounterThenClearAndEnableDisplay, func_8003B9D0",
      "DISP_DISABLED / DISP_LOADING / DISP_ACTIVE (bb2_const.h:6-8) are the values of D_800A3768 the switch tests; every path draws "
      "into the local OT and syncs; the callers are wait loops",
      "no single screen, choice handler or fade-only claim",
      "w64 'not one proven screen identity, not merely a fade, and no choice handler'"),

    # ------------------------------------------------------------------------------------------ 4BC3C.c
    R("0x8005B644", "snd_StopAllVoicesAndCloseSlotVab", "func_8005B644", "4BC3C.c",
      "func_800858D0(0); v = a0 * 3 + 1; SsVabClose(v); g_vab_rec_ptr[v] = 0; g_vab_vb_sbaddr[v] = 0",
      "309CC.c:182 (func_80040594), 35000.c:573 (func_80045A50) and the asm of func_80040594, func_80045A50",
      "func_80040594 loads the slot's VAB with id arena_id * 3 + 1 (309CC.c:191-194, 214) and func_80045A50 closes it with the slot's "
      "release; func_800858D0(0) is the body of snd_StopAllVoices (4BC3C.c:89)",
      "no fighter identity and no arithmetic in the name",
      "w60 'the role distinguishing this indexed VAB group remains unproved'; the slot ownership is shown by load and release"),

    R("0x8005B6AC", "snd_StopAllVoicesAndCloseSelectedEntryVabs", "func_8005B6AC", "4BC3C.c",
      "func_800858D0(0); SsVabClose and clear g_vab_rec_ptr / g_vab_vb_sbaddr for the two ids that func_80049C24 loads",
      "35000.c:756 (func_80046020) and the asm of func_80046020",
      "func_80049C24 (368E4.c:2064-2166) loads those two VABs (snd_LoadVabPack ids written as the entry header) from the entries "
      "selected by D_800A33E8 into arena block 6; func_80046020 frees block 6 and calls this",
      "no sound-content or slot-role claim",
      "w60 'slot roles are not established sufficiently for a non-numeric distinguishing name'; the producer and consumer are shown"),

    R("0x8005B72C", "snd_CloseNonCommonVabsAndResetVoicesQueueAndReverb", "func_8005B72C", "4BC3C.c",
      "func_800858D0(0); SsUtReverbOff; SsUtSetReverbType(0); SsUtSetReverbDepth(0, 0); SsVabClose and clear the registrations of "
      "ids 1..15; D_800A3408 = 0; snd_ResetVoicesAndQueue()",
      "25C38.c:45 / 404, 2B344.c:597 and the asm of func_80035480, func_80035DC8, func_8003BE10",
      "snd_LoadCommonVab loads the common bank as id 0 (4BC3C.c:153-167) and this loop starts at id 1, so it stays open",
      "neither CloseAllVabs (id 0 stays) nor Quit (the driver stays started)",
      "w60 'CloseAllVabs is false because slot zero remains; Quit is false because the sound driver remains started'"),

    R("0x8005B868", "snd_StopAllVoicesAndClosePairedFileVabs", "func_8005B868", "4BC3C.c",
      "func_800858D0(0); SsVabClose and clear the registrations of the two ids that func_8005B8B8 loads",
      "4BC3C.c:187 (func_8005B8B8), 9F9C.c:1677 / 1694 and the asm of func_8001D998, func_8001DA2C, func_8005B8B8",
      "func_8005B8B8 (4BC3C.c:181-202) loads exactly those two VABs, one from each of two CD files; func_8001DA2C closes them for "
      "D_800A38DC == 3",
      "no sound-content claim",
      "w60 'no proven role for this pair beyond fixed slot identity'; the producer is shown"),

    R("0x8005B8B8", "snd_ClosePairedFileVabsThenLoadFromConsecutiveFiles", "func_8005B8B8", "4BC3C.c",
      "func_8005B868(); func_800858D0(0); reads CD file (2, 0x5D) into arg0 and snd_LoadVabPack of it, then file (2, 0x5E) after the "
      "first result and loads its VAB; returns the combined length",
      "9F9C.c:1678 (func_8001D998) and the asm of func_8001D998",
      "the two reads are cdrom_GetFileIndex(2, n) and (2, n + 1) into a growing destination; func_8001D998 copies the result and "
      "calls snd_VabFakeOpen8And4 (a name that already exists for the pair)",
      "no bank-content claim",
      "w60 'numeric file/slot names are specifically disallowed'; consecutive is the shown relation, not the numbers"),

    R("0x8005B9C4", "snd_StopAllVoicesAndCloseSingleFileVab", "func_8005B9C4", "4BC3C.c",
      "func_800858D0(0); SsVabClose and clear the registrations of the one id that func_8005B9FC loads",
      "4BC3C.c:221 (func_8005B9FC), 9F9C.c:1661 / 1691 and the asm of func_8001D904, func_8001DA2C, func_8005B9FC",
      "func_8005B9FC loads exactly that VAB from a single CD file; snd_VabFakeOpen9 (4BC3C.c) is the existing name for its "
      "fake-open",
      "no bank-content claim; differs from snd_CloseVab1 (which does not stop the voices)",
      "w60 'CloseVab9 would encode a literal selector rather than meaning'"),

    R("0x8005B9FC", "snd_CloseThenReadSingleFileAndLoadVab", "func_8005B9FC", "4BC3C.c",
      "func_8005B9C4(); reads CD file (2, 8) into a0 (cdrom_StartRead); returns snd_LoadVabPack(a0, id 9, a0 + size)",
      "9F9C.c:1662 (func_8001D904) and the asm of func_8001D904",
      "differs from snd_LoadCommonVab (SPU-address initialisation, other id) and func_8005B8B8 (two files)",
      "no roster or content identity",
      "w60 'bank's semantic identity unproved'"),

    R("0x8005BA8C", "snd_ReloadListedVabsFromFileAndInstallBillboards", "func_8005BA8C", "4BC3C.c",
      "func_800858D0(0); closes the three ids of g_vab_id_list; reads CD file (2, arg1 + 9) into arg0; relocates word 12 and calls "
      "eff_SetBillboardList(it); selects entries 0, arg2 + 1 and arg3 + 1 (two when arg2 == arg3), loads each with "
      "snd_LoadVabPack, packs them contiguously (snd_VabFakeOpen), aliases g_vab_rec_ptr[6] = [3] when two; returns the packed length",
      "9F9C.c:1638 (func_8001D790) and the asm of func_8001D790",
      "g_vab_id_list is what snd_CloseListedVabs and snd_RebaseListedVabs (4BC3C.c) walk; eff_SetBillboardList "
      "(51268.c:702-713) copies the list that the renderer reads",
      "includes a graphics-side effect; no character or stage bank identity",
      "w60 'LoadListedVabs omits the auxiliary initialization'; the billboard installation is now named"),

    R("0x8005C614", "snd_StopVoicesSetMaxMasterVolumeStereoAndClearAutoKeyOffMode", "func_8005C614", "4BC3C.c",
      "SsSetMVol(0x7F, 0x7F); func_800858D0(0); SsSetStereo(); SsSetAutoKeyOffMode(0)",
      "6CF8.c:265 (snd_InitAndLoadCommonVab) and the asm of snd_InitAndLoadCommonVab",
      "each statement is named by its callee; 0x7F is libsnd's maximum volume; the mode is cleared, its meaning is not claimed",
      "neither 'Defaults' (no external policy) nor SetStereo alone",
      "w60 'combined output policy needs an agreed noun'"),
]

DATA = []
