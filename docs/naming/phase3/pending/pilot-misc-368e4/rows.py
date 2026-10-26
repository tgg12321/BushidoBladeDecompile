"""wave86 rows (pilot-misc-368e4): Q132 mechanical names for 27 of the 46 neutral functions of main/368E4 (the
loaded-resource block family, the node-asset block and retained draw-node functions, the playback-block loader, the
entry-request / pair-selection family, two wave samplers, a TIM group loader variant and three small accessors).
Research input: tmp/codex/pilot-misc-368e4-research.md (29 RENAME, 17 NO_NAME over 46 targets; untrusted
proposals). Every row was re-derived from the C body in src/main/368E4.c and every caller (src/main/9F9C.c,
2B344.c, 43500.c, 35000.c, the hook table src/main/d_80DF8.c, asm/funcs/*.s); names changed against the research say
CHANGED FROM RESEARCH in the evidence.
Research NO_NAMEs (17; stay neutral, not rows): 80046780 80046934 80046954 800469A0 80046A80 80046DE4 80046E8C
8004746C 8004748C 800474AC 800474D0 800474F0 80047738 8004775C 8004777C 800477BC 800477CC (pure forwarders with an
instruction-identical twin, selector-only differences, empty hooks, a constant return, a getter whose global has no
role, a mask setter whose whole value is not explained).
Research RENAMEs dropped by this draft (2 of 29; 27 remain, 16 of them renamed or reworded against the research, see ADJUSTED):
  8004695C (resource_SetPeakAndBlockEndWithEmptyMoveCallback): it works on arena block 9, which no function in
    src/ identifies (the only other users are two neutral NO_NAME forwarders); a resource_/arena_ prefix would claim
    a subsystem the code does not show, and a name that omits the block would read as generic. Stays neutral.
  80046B44 (resource_InitReadArenaTransformsLightingAndDrawState): an aggregate of nine unrelated startup calls
    (ndata, arena, rotation-matrix dispatch, reloc counts, root nodes, light defaults, fog draw moves, two more
    neutral callees) and six bare global resets; no single prefix is explained and the name would omit func_800404D8
    / func_8003F7F4 and the D_800F665x stores. Stays neutral.
Prefix evidence: resource_ is the TU's (resource_GetLoadedSelector / resource_HasRetainedSubblocks already read the
state of arena block 7, and the entry-request functions feed func_80045B68's block-6 load through the same
selection state); node_ is the TU's (node_InitIdentity, node_GetSlotMatrixPointerArray, node_OrientAndQueueViewOffsetNode);
wave_ / gpu_ / motion_ / snd_ are the callee subsystems of those bodies (wave_Update, gpu_LoadTimGroup,
motion_FreePlaybackBlock, snd_LoadVabPack). The research's data suggestions (D_80099478 ... D_800EF980 ...) need
separate all-access data manifests and are not part of this wave (DATA is empty).
"""
import csv
import glob
import os
import re

VERDICT = "CONFIRM"
VERIFIER_NOTE = "naming-reviewer pre-apply (round 1): 24/27 PASS as proposed; 3 FAIL (800464C4, 8004659C, 800493E4) replaced by the reviewer-suggested names (NOTES)."
NOTES = {
    "0x800464C4": "suggested: naming-reviewer pre-apply round 1 failed resource_ApplyRetainedSectionsAndResizeBlock (for selector 0x22 the applied sections are the header sections the loader already installed, 368E4.c:103/167/171-173; the D_8009947A clear was left out) and suggested resource_ApplyHeaderSectionsResizeBlockAndClearRetainedSubblocks; R6 must judge it fresh.",
    "0x8004659C": "suggested: naming-reviewer pre-apply round 1 failed resource_SelectRetainedSubblockOrApplyRetainedSections (the a0 < 0 branch is 800464C4, which applies header sections) and suggested resource_SelectRetainedSubblockOrApplyHeaderSections; R6 must judge it fresh.",
    "0x800493E4": "suggested: naming-reviewer pre-apply round 1 failed resource_RequestEntryWithLinkedEntriesAndCheckCategory (the first call after ResetEntrySelectionState writes the category, 368E4.c:1748-1753; Check omits that) and suggested resource_RequestEntryWithLinkedEntriesAndLatchCategory; R6 must judge it fresh.",
}

# names changed against the research (research name -> name used)
ADJUSTED = {
    "0x800460E4": ("resource_LoadAndInstallSelectedBlock", "resource_LoadOrReuseSelectedBlock",
                   "the early return when the selector is already resident (D_80099478 == arg0, default selectors) reuses the block and "
                   "installs nothing, so 'LoadAndInstall' is false for that path"),
    "0x800464C4": ("resource_ApplyRetainedSectionsAndTrimBlock", "resource_ApplyHeaderSectionsResizeBlockAndClearRetainedSubblocks",
                   "arena_Resize to (s1 - s0) is shown; 'Trim' (a shrink) is not shown by the code"),
    "0x8004659C": ("resource_SelectRetainedSubblockOrApplyAndTrimSections", "resource_SelectRetainedSubblockOrApplyHeaderSections",
                   "follows the renamed 800464C4 (resize, not trim)"),
    "0x8004668C": ("resource_FreeLoadedBlockAndClearSelector", "resource_FreeLoadedBlockAndClearSelectorAndRetainedSubblocks",
                   "the body also clears D_8009947A (the flag resource_HasRetainedSubblocks reads)"),
    "0x800467B8": ("resource_LoadOrReuseGrayscaleSelectedNodeAsset", "resource_LoadOrReuseGrayscaleAdjustedNodeAsset",
                   "'Selected' is not shown here (arg0 is a table index that gpu_RefreshGrayscaleMode() shifts by 0x1B); the word the code shows is the grayscale adjustment"),
    "0x800468B0": ("resource_ResizeNodeAssetBlockAndResetPeak", "resource_ResizeNodeAssetBlockAndUpdatePeak",
                   "arena_UpdatePeak(0) (35000.c:334) takes the arena free pointer and raises the stored peak to it; it never lowers or resets it"),
    "0x800468DC": ("node_RebaseRetainedDrawBufferAndAssetRelocations", "resource_RebaseNodeAssetBlock",
                   "the callback is registered for arena block 8 (the block resource_FreeNodeAssetBlock frees), so it is the block's move callback "
                   "like resource_RebaseLoadedBlockSections; the retained buffer and the reloc table are both parts of that block"),
    "0x80046AA0": ("resource_ReleaseSlotsAndBlocksAndInvalidateEntryCache", "resource_FreeSlotsAndBlocksAndInvalidateEntryCache",
                   "the slot callee is softbody_FreeObjectById + func_80045A50 + clearing D_800A9A10[slot]: a free, not a release"),
    "0x80046EDC": ("resource_LoadSelectedBlockLightingAndFogImageLevels", "resource_LoadOrReuseSelectedBlockSetLightingAndBuildFogImageLevels",
                   "the first callee is the load-or-reuse function; the second writes the light setup (31D3C.c:426) and the third builds fog image levels"),
    "0x80048AD0": ("node_LoadSlotMappedAssetAndInitRetainedDrawNodes", "node_LoadSlotMappedAssetAndInitRetainedDrawNodes",
                   "kept as researched"),
    "0x80049584": ("resource_RankRequestedEntriesAndReloadChangedSelection", "resource_RankRequestedEntriesAndReloadOnChange",
                   "the reload fires on a membership-sign change OR a pair-index change; 'ChangedSelection' covers only the second"),
    "0x800493E4": ("resource_RequestEntryAndLinkedEntriesWithCategoryCheck", "resource_RequestEntryWithLinkedEntriesAndLatchCategory",
                   "the check also initializes the category (D_800A33EC == -1 branch); the verb 'Check' follows the rest, wording only"),
    "0x800494D4": ("resource_SetPairSelectionWithUpperBoundChecks", "resource_SetPairSelectionSlotWithBoundChecks",
                   "idx >= 2 is a bound on the slot index and val < 16 / val < 8 a bound on the value; 'UpperBound' would hide that the first is on the slot"),
    "0x80049C24": ("snd_CopySelectedVabPacksAndLoadRepackedBanks", "snd_CopySelectedPackSectionsAndLoadVabPacks",
                   "'Repacked banks' is not shown; what the code shows is mem_MoveBlockRoundedUpToWords of the chosen pack section followed by snd_LoadVabPack of the moved copy"),
    "0x80049E4C": ("node_InitViewAndComposeRootsWithNegatedRotation", "node_InitRootNodePairWithNegatedRotationMatrices",
                   "both records get parent_node = 0 (roots) and node_SetMatrixFromNegatedRot; 'View' / 'Compose' are claims about the consumers that the initializer itself does not show"),
    "0x80047D94": ("wave_SampleLastRowWithRemainderParityAndCachePosition", "wave_SampleLastRowCachingIndexAndRemainderParity",
                   "wording only: the two cached globals are an index and the remainder's low bit"),
    "0x80047E5C": ("wave_SampleLastRowStepsAtCachedPosition", "wave_SampleLastRowStepsAtCachedIndex",
                   "wording only: the cached value is the index D_800A33D4 (plus the parity weight)"),
    "0x80046E44": ("gpu_ClearRaisedOtCutoffRequest", "gpu_ClearRaisedOtCutoffRequest", "kept as researched"),
}

DROPPED = {
    "0x8004695C": "arena block 9 is unidentified anywhere in src/; no prefix is explained and a generic name would mislead",
    "0x80046B44": "aggregate of unrelated startup calls and resets; prefix unexplained, a name would omit calls",
}

WT = os.environ.get("P3WT", r"C:\Users\Trenton\Desktop\bb2-worktrees\ln-naming-c")

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


def B(fn, f="368E4.c"):
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


_BLK7 = ("Arena block 7 is the block this function family keeps: resource_GetLoadedSelector (D_80099478) and "
         "resource_HasRetainedSubblocks (D_8009947A) are the state of the block opened by func_800460E4 (arena_OpenBlock(7)) and "
         "freed by func_8004668C (arena_Free(7)). ")

FUNCS = [
    ("0x800460E4", "resource_LoadOrReuseSelectedBlock", "RENAME",
     f"{ch('0x800460E4')}BODY: {B('func_800460E4')}: arena_GetBlockAddr(7); when D_80099478 == arg0 the default selectors set s7 = 1 and, after func_8003F168, "
     f"return without reading anything (selectors 4/7/18 first install a section with func_8003EDC0, selector 34 sets s7 = 0 and falls through to a reload); "
     f"otherwise D_80099478 = arg0, arena_OpenBlock(7), func_80044F30 reads the selected block (or arena_Copy copies arg1), the header words are turned into section pointers, "
     f"reloc_RelocateBlock / func_800481E8 (TIM group) / func_8003EDC0 / collision_SetGrid install them, D_800A33B0 / D_800A33B4 take two section pointers, "
     f"arena_SetBlockEnd(7) and arena_SetMoveCallback(7, func_800466C0), and func_8004659C(-1) runs when D_800A38DC is set. "
     f"CALLERS: the only caller is func_80046EDC ({B('func_80046EDC')}), itself called from src/main/43500.c and 9F9C.c. {_BLK7}"
     f"WHY: Load = the opened-and-read path, Reuse = the resident-selector early return, Selected = arg0 is stored as the loaded selector (resource_GetLoadedSelector reads it back). "
     f"COUNTER-EVIDENCE: no stage, menu or asset identity is claimed (arg0 is only a selector); the section installs are part of the load path and not named. "
     f"Prior manifests: no_name.tsv w49/w61 (stage/menu identity, unresolved callees); Q132 lifts the identity requirement. {T('0x800460E4')}"),

    ("0x800464C4", "resource_ApplyHeaderSectionsResizeBlockAndClearRetainedSubblocks", "RENAME",
     f"{ch('0x800464C4')}BODY: {B('func_800464C4')}: returns when D_8009947A == 0 (resource_HasRetainedSubblocks); otherwise takes the block of arena 7, picks header section s1 by D_80099478 (0xD -> s0[6], 3 / 0x22 -> s0[5]), "
     f"reloc_RelocateBlock(s0[1] section, 7), func_8003EDC0(s1, 7), arena_Resize(7, s1 - s0) and clears D_8009947A. "
     f"CALLERS: {B('func_8004659C')} (negative argument; the only caller). WHY: ApplyHeaderSections = relocate and install the header-word sections s0[1] / s0[5] / s0[6] (not the retained subblocks, which 8004659C applies for a0 >= 0), "
     f"ResizeBlock = arena_Resize(7, ...), ClearRetainedSubblocks = the guard on and store of D_8009947A (resource_HasRetainedSubblocks). COUNTER-EVIDENCE: no claim about what the sections contain or that the resize shrinks; no deferred-work reading. "
     f"Prior manifests: no_name.tsv w49/w61 (resource noun unresolved; the sections are mechanical). {T('0x800464C4')}"),

    ("0x8004659C", "resource_SelectRetainedSubblockOrApplyHeaderSections", "RENAME",
     f"{ch('0x8004659C')}BODY: {B('func_8004659C')}: a0 < 0 calls func_800464C4 and returns; else returns when D_8009947A == 0, otherwise indexes five section offsets at 20 bytes per subblock a0 from the arena-7 block, "
     f"reloc_UnrelocateBlock(7), reloc_RelocateBlock(s2, 7), collision_SetGrid(s3), D_800A33B0 = s4, D_800A33B4 = s1, func_8003EDC0(s0, 7). "
     f"CALLERS: src/main/368E4.c ({L('368E4.c', r'func_8004659C\(-1\)')}), 2B344.c:529 and 43500.c:634 / 648 (argument -1), 9F9C.c:1333 / 1337 / 1341 (arguments 1, 2, 3); "
     f"the -1 callers reach func_800464C4 and the others select subblock a0. WHY: the name states both branches. COUNTER-EVIDENCE: no select-only reading (a0 < 0 applies); "
     f"no mode teardown is claimed. Prior manifests: no_name.tsv w49/w61 (select-only omitted the negative branch; this name includes it). {T('0x8004659C')}"),

    ("0x8004668C", "resource_FreeLoadedBlockAndClearSelectorAndRetainedSubblocks", "RENAME",
     f"{ch('0x8004668C')}BODY: {B('func_8004668C')}: arena_Free(7); D_80099478 = -1 (the selector resource_GetLoadedSelector returns); D_8009947A = 0 (the flag resource_HasRetainedSubblocks returns). "
     f"CALLERS: {L('368E4.c', r'^\s+func_8004668C\(\)')} (func_80046AA0) and the asm of that function; no other caller. WHY: the three effects are the whole body, and both cleared words are the named state of the block func_800460E4 opens. "
     f"COUNTER-EVIDENCE: not collision-only (the block holds every installed section); no asset identity. Prior manifests: no_name.tsv w49/w61 (whole-family noun; the opener and the two getters supply 'loaded block'). {T('0x8004668C')}"),

    ("0x800466C0", "resource_RebaseLoadedBlockSections", "RENAME",
     f"{ch('0x800466C0')}BODY: {B('func_800466C0')}: reloc_RebaseBlock(7, a1), collision_RebaseGrid(a1), D_800A33B0 += (a1 / 4) * 4, D_800A33B4 += (a1 / 4) * 4, then per selector func_80044C70(a1) (4 / 7 / 18), reloc_RebaseBlock(8) (13) "
     f"or wave_MoveCellBuffer(a1) (11). CALLERS: registered as the move callback of arena block 7 at {L('368E4.c', r'arena_SetMoveCallback\(s7, \(s32\)func_800466C0')}; never called directly. "
     f"WHY: a1 is the movement delta applied to every pointer the loader stored (the reloc table, the collision grid, the two section pointers, the selector's extra storage). "
     f"COUNTER-EVIDENCE: the one-argument reloc_RebaseBlock(8) call (selector 13) is an arity anomaly the name neither explains nor depends on; no stage identity. "
     f"Prior manifests: no_name.tsv w49/w61 (unresolved callees; the rebase is mechanical). {T('0x800466C0')}"),

    ("0x800467B8", "resource_LoadOrReuseGrayscaleAdjustedNodeAsset", "RENAME",
     f"{ch('0x800467B8')}BODY: {B('func_800467B8')}: arg = a0, plus 0x1B when gpu_RefreshGrayscaleMode(); arena_FindBlock(8) with D_800A3248 == arg returns the cached block's word 1; otherwise arena_OpenBlock(8), func_80044F80(arg, block) reads it, "
     f"reloc_RelocateBlock(section, 9), arena_UpdatePeak, arena_SetBlockEnd(8), D_800A3248 = arg; arena_SetMoveCallback(8, func_800468DC) always. "
     f"CALLERS: {L('368E4.c', r'base = \(u8 \*\)func_800467B8')} (func_80048AD0) only: it indexes the returned block as the table of the 17 draw-node records it initializes. "
     f"WHY: Load/Reuse = the cached-block branch vs the read branch, Grayscale = the +0x1B adjustment on gpu_RefreshGrayscaleMode(), NodeAsset = the sole caller's use of the block (func_80048AD0 stores its node records in it). "
     f"COUNTER-EVIDENCE: the cached return (descriptor word 1) and the loaded return (arena_OpenBlock result) are different expressions, so no 'pack base' claim; no preview / equipment identity. "
     f"Prior manifests: no_name.tsv w49/w61 (uniform pack-header base and audio identity were refuted; neither is claimed). {T('0x800467B8')}"),

    ("0x800468B0", "resource_ResizeNodeAssetBlockAndUpdatePeak", "RENAME",
     f"{ch('0x800468B0')}BODY: {B('func_800468B0')}: arena_Resize(8, a0); arena_UpdatePeak(0). CALLERS: {L('368E4.c', r'func_800468B0\(delta')} (func_80048AD0, after it stored the node records) only. "
     f"WHY: block 8 is the node asset func_800467B8 opens; arena_UpdatePeak(0) (35000.c:334) records the arena high-water mark from the free pointer. "
     f"COUNTER-EVIDENCE: no sound or numeric block identity; the peak is raised, never reset. Prior manifests: no_name.tsv w49/w61 (resource noun unresolved; node asset follows from the opener and the sole caller). {T('0x800468B0')}"),

    ("0x800468DC", "resource_RebaseNodeAssetBlock", "RENAME",
     f"{ch('0x800468DC')}BODY: {B('func_800468DC')}: func_80048B8C(a1) (D_800A33E4 += a1, the retained node buffer pointer), reloc_RebaseBlock(9, a1). "
     f"CALLERS: the move callback of arena block 8, {L('368E4.c', r'arena_SetMoveCallback\(var_1, \(s32\)func_800468DC')}; never called directly. "
     f"WHY: block 8 is the node asset block (func_800467B8 opens it, func_80046914 frees it); the callback rebases the one pointer func_80048AD0 retains into it and the relocation table func_800467B8 installed (reloc_RelocateBlock(..., 9)). "
     f"COUNTER-EVIDENCE: no claim about all node storage. Prior manifests: no_name.tsv w49/w61 (node-storage relationship accepted). {T('0x800468DC')}"),

    ("0x80046914", "resource_FreeNodeAssetBlock", "RENAME",
     f"BODY: {B('func_80046914')}: arena_Free(8). CALLERS: func_80046AA0 ({L('368E4.c', r'^\s+func_80046914\(\)')}) and src/main/43500.c ({L('43500.c', r'func_80046914\(\)')}, motion_StartPlayback frees it when ndata_GetArenaRoomAfter < 0, to make room). "
     f"WHY: frees exactly the block func_800467B8 opens and func_80048AD0 fills with node records. COUNTER-EVIDENCE: the playback caller uses it for room, which does not make the block playback data. "
     f"Prior manifests: no_name.tsv w49/w61 (insufficient identity; ownership is shown by the opener and the filler). {T('0x80046914')}"),

    ("0x800469C4", "motion_LoadPlaybackBlock", "RENAME",
     f"BODY: {B('func_800469C4')}: arena_OpenBlock(0xA), ndata_ReadChecked(a0, block), end = block + (block[block[0] + 1] & ~3), arena_UpdatePeak, arena_SetBlockEnd(0xA), D_800A324A = a0, arena_SetMoveCallback(0xA, func_80046A80), returns the block. "
     f"CALLERS: motion_StartPlayback ({L('43500.c', r'func_800469C4\(id\)')}) only, which stores the result as s->unk2C and reads the header words at +4 / +8 (Unk800469C4Hdr); func_80046A80 (the callback) forwards to motion_RebasePlayback; "
     f"the existing motion_FreePlaybackBlock ({B('motion_FreePlaybackBlock')}) is arena_Free(0xA), the matching free. WHY: Load = open and read, PlaybackBlock = arena block 0xA, owned by the motion playback start/free pair. "
     f"COUNTER-EVIDENCE: motion_StartPlayback can instead read into caller storage (a6 != 0); the name claims the allocated path only. Prior manifests: no_name.tsv w49/w61 (resource class unproved; now shown by the caller, the callback and motion_FreePlaybackBlock). {T('0x800469C4')}"),

    ("0x80046AA0", "resource_FreeSlotsAndBlocksAndInvalidateEntryCache", "RENAME",
     f"{ch('0x80046AA0')}BODY: {B('func_80046AA0')}: func_800415C4(0), func_800415C4(1) (31D3C.c:32: softbody_FreeObjectById(a0), func_80045A50(a0), D_800A9A10[a0] = 0), func_8004668C (frees block 7), func_80046020 (35000.c:754: arena_Free(6) and func_8005B6AC), "
     f"func_80049E1C (invalidates the entry selection cache), func_80046914 (frees block 8). CALLERS: src/main/596CC.c, 5ED34.c, 64FD8.c, asm of func_8006E534 / menu_InitSelScreen / menu_LoadResourcesInitStateSelectFirstEnabledRow. "
     f"WHY: the verb list is the body; 'Slots' are the two D_800A9A10 object slots, 'Blocks' the arena blocks 7, 6 and 8. COUNTER-EVIDENCE: no claim of a complete fight, session or menu teardown. "
     f"Prior manifests: no_name.tsv w49/w61 (cross-family aggregate; Q132 permits naming the demonstrated effects). {T('0x80046AA0')}"),

    ("0x80046E18", "node_GetSlotTransformNodes", "RENAME",
     f"BODY: {B('func_80046E18')}: v0 = func_8004153C(a0) (D_800A9A10[a0], the object slot); returns v0->transform_nodes or NULL. The sibling node_GetSlotMatrixPointerArray ({B('node_GetSlotMatrixPointerArray')}) is the same shape returning node_matrix_ptrs; "
     f"the members differ, so the names differ by the member. CALLERS: none (no jal in asm/, no reference in src/ beyond the definition). WHY: typed member getter. "
     f"COUNTER-EVIDENCE: no caller, so no higher-level role is claimed; transform_nodes is used as node storage by the object code in 368E4.c (obj->node, matrix operations further down). Prior manifests: no_name.tsv w49/w61 (no intended abstraction; a typed getter needs none). {T('0x80046E18')}"),

    ("0x80046E44", "gpu_ClearRaisedOtCutoffRequest", "RENAME",
     f"BODY: {B('func_80046E44')}: D_800F6654 = 0. The same word is set by gpu_SetRaisedOtCutoffRequested (1 for nonzero, 0 for zero) and read by gpu_GetRaisedOtCutoffRequested, both named in this TU; "
     f"src/main/32D04.c selects the raised OT cutoff from it. CALLERS: none. WHY: clears the request the two named neighbours set and read. COUNTER-EVIDENCE: not pause state; the setter with 0 has the same effect but a different body, so this is not an instruction-identical twin. "
     f"Prior manifests: no_name.tsv w49/w61 (rendering meaning unresolved; the named setter/getter now establish 'raised OT cutoff request'). {T('0x80046E44')}"),

    ("0x80046EDC", "resource_LoadOrReuseSelectedBlockSetLightingAndBuildFogImageLevels", "RENAME",
     f"{ch('0x80046EDC')}BODY: {B('func_80046EDC')}: func_800460E4(a0, a1); func_800421C8(a0) (31D3C.c:426-443: reads the light setup D_80094E48[a0], writes the colours with func_800422BC and the yaw / pitch of lights 0, 1 and 4); "
     f"func_8003E0E0 (2B344.c:1635: gpu_BuildFogImageLevels). CALLERS: src/main/43500.c (motion_StartPlayback, twice), src/main/9F9C.c, asm of func_8001D790. WHY: three calls, three effects. "
     f"COUNTER-EVIDENCE: no stage-only role (the motion caller passes a loaded block's selector); a cleanup reading is contradicted by the loader call. Prior manifests: no_name.tsv w49/w61 (load behaviour and subsystem unresolved). {T('0x80046EDC')}"),

    ("0x80047570", "node_InitViewOffsetNodeWithYXZRotation", "RENAME",
     f"BODY: {B('func_80047570')}: node_InitIdentity(&D_800EEDF0); D_800EEDF0.rotation_order = 4. D_800F66A0[4] = math_RotMatrixYXZ (src/main/32D04.c:402). CALLERS: the hook table src/main/d_80DF8.c ({L('d_80DF8.c', r'func_80047570, node_OrientAndQueueView')}) beside node_OrientAndQueueViewOffsetNode, "
     f"which orients this exact node (D_800EEDF0), places it at the view translation plus 0x6590 and queues it. WHY: Init = node_InitIdentity, ViewOffsetNode = the node the paired function places relative to the view, YXZRotation = rotation_order 4. "
     f"COUNTER-EVIDENCE: the order store does not recompute a matrix; no sky or background identity. Prior manifests: no_name.tsv w49/w61 (particular drawable role unresolved; the name states the placement mechanics). {T('0x80047570')}"),

    ("0x80047D94", "wave_SampleLastRowCachingIndexAndRemainderParity", "RENAME",
     f"{ch('0x80047D94')}BODY: {B('func_80047D94')}: a1 = (a0 + 0x7D00) / 3200 and the remainder a0 - (a0 / 3200) * 3200 give D_800A33D4 = a1 and D_800A33D8 = remainder & 1; a1 >= 18 returns 0xFFFE7960; otherwise "
     f"((row8[a1] * parity + row8[a1 + 1] * (0x1000 - parity)) >> 12) - 0x3F48 with row8 = D_800EF59C[8], the last row wave_Update ({B('wave_Update')}) fills. CALLERS: none. "
     f"WHY: 'LastRow' = D_800EF59C[8], 'CachingIndexAndRemainderParity' = the two stores. COUNTER-EVIDENCE: the weight is the remainder's low bit, not a normalized fraction, so no interpolation or height claim; "
     f"a1 == 17 / 18 reads into the following D_800EF800 and the name makes no bounds claim. Prior manifests: no_name.tsv w49/w61 (height interpolation hid the AND-one weight; this name states it). {T('0x80047D94')}"),

    ("0x80047E5C", "wave_SampleLastRowStepsAtCachedIndex", "RENAME",
     f"{ch('0x80047E5C')}BODY: {B('func_80047E5C')}: v1 = D_800A33D4 (the index func_80047D94 stores); >= 18 returns 0; else (D_800EF800[v1] * D_800A33D8 + D_800EF800[v1 + 1] * (0x1000 - D_800A33D8)) >> 12. "
     f"wave_Update writes D_800EF800[j] = v1 (the per-call increment added to row 8) at {L('368E4.c', r'D_800EF800\[j\] = v1')}. CALLERS: none. "
     f"WHY: the same weighted pair of neighbours as func_80047D94, taken from the array of last-row increments instead of the row, at the cached index and parity. "
     f"COUNTER-EVIDENCE: 'Steps' means the per-update increments the code stores, no velocity or derivative claim; the upper-boundary read past the 18-word array stays documented in the extern. "
     f"Prior manifests: no_name.tsv w49/w61 (derivative meaning unproved; not claimed). {T('0x80047E5C')}"),

    ("0x800481E8", "gpu_LoadTimGroupWithConditionalClutRowAdvance", "RENAME",
     f"BODY: {B('func_800481E8')}: the same offset-selected descriptor walk as gpu_LoadTimGroup ({B('gpu_LoadTimGroup')}); per record it reads four s16 (x, y, clut x, clut y) and, when clut x < 0x280, increments clut y before gpu_LoadTim. "
     f"gpu_LoadTimGroup passes all four through unchanged, which is the difference. CALLERS: func_800460E4 ({L('368E4.c', r'func_800481E8\(\(u32 \*\)var_7')}) only, asm jal in func_800460E4. "
     f"WHY: the name states the one conditional step (clut y advanced when clut x is below a bound). COUNTER-EVIDENCE: no threshold value, asset or UI identity is claimed; whether the advance always fires is the condition. "
     f"Prior manifests: none for this address in no_name.tsv. {T('0x800481E8')}"),

    ("0x80048AD0", "node_LoadSlotMappedAssetAndInitRetainedDrawNodes", "RENAME",
     f"BODY: {B('func_80048AD0')}: slot = func_8004153C(arg0) (0 returns 0); D_800A33E0 = arg0; var_1 = D_80099BCC[slot->unk_08] (0xFF returns 0); base = func_800467B8(var_1); D_800A33E4 = base + (base[2] aligned); "
     f"17 records of 0x68 bytes get fields 0 / 1 / 2 / 4 / 0xA set; func_800468B0(delta + 0x6E8) resizes the block; returns 1. CALLERS: fight_Init ({L('9F9C.c', r'func_80048AD0\(i\)')}) only. "
     f"WHY: Slot-mapped = the slot's field mapped through D_80099BCC, Load = func_800467B8, InitRetainedDrawNodes = the 17 node records whose pointer D_800A33E4 is retained and later queued by func_80048BA4 (g_draw_queue_cursor stores) and rebased by func_80048B8C. "
     f"COUNTER-EVIDENCE: the registry's sound / voice comments are contradicted by the node stores; no equipment or preview identity. Prior manifests: no_name.tsv w49/w61 (node-init-only omitted loading and resizing; this name has both). {T('0x80048AD0')}"),

    ("0x80048B8C", "node_RebaseRetainedDrawNodeBuffer", "RENAME",
     f"BODY: {B('func_80048B8C')}: D_800A33E4 += a0. D_800A33E4 is assigned in func_80048AD0 ({L('368E4.c', r'D_800A33E4 = \(s32\)p;')}) and read as the node array in func_80048BA4 ({L('368E4.c', r'node = \(Unk80045878Node \*\)D_800A33E4')}); "
     f"no other access in src/. CALLERS: func_800468DC (node asset block move callback) only. WHY: moves the one retained pointer by the movement delta. COUNTER-EVIDENCE: not all node storage and not the object pool. "
     f"Prior manifests: no_name.tsv w49/w61 (distinguishing noun; the pointer's whole life is the retained node buffer). {T('0x80048B8C')}"),

    ("0x80048BA4", "node_SetSlotRelativeMatrixQueueRetainedNodesAndLinkOrderingTable", "RENAME",
     f"BODY: {B('func_80048BA4')}: mat_tbl = node_GetSlotMatrixPointerArray(D_800A33E0) (NULL returns); builds D_800FF558 (translation = ApplyMatrix(mat_tbl[0], offset by arg0) + the slot's translations; rotation = mat_tbl[0] times a Y rotation of 0xC00 - arg0, transposed); "
     f"copies mat_tbl[index] for each index of D_80099C14 into the retained nodes (D_800A33E4) and pushes each on g_draw_queue_cursor, plus optional entries 18 (arg1) and 19 (arg2); "
     f"ClearOTagR on g_gpu_ot256_db[D_800A36AC & 1] and splices it into D_800A378C[0]. CALLERS: {L('9F9C.c', r'func_80048BA4\(')} (asm jal in func_8001E800) only. "
     f"WHY: three effects: slot-relative matrix, queued retained nodes, linked ordering table. COUNTER-EVIDENCE: no portrait, equipment or preview identity; the transposed products are not simplified into a camera operation. "
     f"Prior manifests: no_name.tsv w49/w61 (queue-only underdescribed the matrix and OT effects). {T('0x80048BA4')}"),

    ("0x800493E4", "resource_RequestEntryWithLinkedEntriesAndLatchCategory", "RENAME",
     f"{ch('0x800493E4')}BODY: {B('func_800493E4')}: D_800EF980[arg0] = 1; D_800A33EC (category) set to arg0 >= 0x33 on first use, else func_80052C10() on a category mismatch; D_80099CC8[arg0] (two bytes, 0xFF = none) marks two more entries. "
     f"CALLERS: func_8003AFFC / func_8003B10C / fight_Init (src/main/9F9C.c:1820-1835, 2B344.c:184-205), always followed by func_80049584, which ranks the marked entries. WHY: Request = the mark, LinkedEntries = D_80099CC8, LatchCategory = the D_800A33EC logic. "
     f"COUNTER-EVIDENCE: callers pass fighter fields and mapped indices; no fighter-only or sound-only claim. Prior manifests: no_name.tsv w49/w61 (identity unknown; mechanics named). {T('0x800493E4')}"),

    ("0x800494D4", "resource_SetPairSelectionSlotWithBoundChecks", "RENAME",
     f"{ch('0x800494D4')}BODY: {B('func_800494D4')}: cond = val < 16 when D_800A33EC == 0 else val < 8, func_80052C10() on failure; idx >= 2 (unsigned) calls func_80052C10(); D_800A33E8[idx] = (s16)val. "
     f"CALLERS: the same three setup functions as func_800493E4; func_80049584 and func_80049C24 read D_800A33E8[0..1] as the two selections (math_UnorderedPairIndex of the pair; the pack choice). "
     f"WHY: stores slot idx of the two-entry selection after bounding both. COUNTER-EVIDENCE: no lower-bound check on val, so no validation claim; no fade meaning. Prior manifests: no_name.tsv w49/w61 (semantics unresolved; the pair role is shown by its readers). {T('0x800494D4')}"),

    ("0x80049584", "resource_RankRequestedEntriesAndReloadOnChange", "RENAME",
     f"{ch('0x80049584')}BODY: {B('func_80049584')}: snapshots D_800EF980 into D_80099C50 noting any change of sign (>= 0); assigns dense ranks 0, 1, ... to the entries >= 0; computes the pair index from D_800A33E8 (0x24 / 0x88 constants when both are -1, else math_UnorderedPairIndex); "
     f"when the index differs from D_800A324C or any sign changed: func_80046020 (frees block 6), func_80045B68(D_800A33EC, i, D_800EF980, arg0) (loads it), func_8003E120. "
     f"CALLERS: func_8003AFFC, func_8003B10C, fight_Init, last in each setup. WHY: Rank = the dense numbering, Requested = entries >= 0 (marked by func_800493E4), ReloadOnChange = the free-and-load guarded by the cache comparison. "
     f"COUNTER-EVIDENCE: membership sign is compared, not full entry equality; no asset identity. Prior manifests: no_name.tsv w49/w61 (cache / reload clear, identity unresolved). {T('0x80049584')}"),

    ("0x80049C24", "snd_CopySelectedPackSectionsAndLoadVabPacks", "RENAME",
     f"{ch('0x80049C24')}BODY: {B('func_80049C24')}: reads the pack table arg0 (count, offsets); maps D_800A33E8[0..1] to source sections (0 / 1, or skip when -1); writes the count then, per selected section, a bank id (2, then 5), "
     f"mem_MoveBlockRoundedUpToWords(section, dest, size), arena_UpdatePeak(dest end), snd_LoadVabPack(dest, id, shared tail); returns the destination end. "
     f"CALLERS: func_80045B68 ({L('35000.c', r'func_80049C24\(prev')}) only. WHY: Copy + Selected = the moves keyed by the D_800A33E8 pair (set by func_800494D4), LoadVabPacks = snd_LoadVabPack per selected section. "
     f"COUNTER-EVIDENCE: no weapon or character identity for the selections. Prior manifests: wave70 rejected for lack of selection provenance, now provided by func_800494D4 and its callers. {T('0x80049C24')}"),

    ("0x80049E1C", "resource_InvalidateEntrySelectionCache", "RENAME",
     f"BODY: {B('func_80049E1C')}: D_80099C50[0..0x39] = -1; D_800A324C = -1. Their only comparison and reload use is func_80049584 (snapshot sign compare, pair index compare). CALLERS: func_80046AA0, src/main/35000.c ({L('35000.c', r'func_80049E1C\(\)')}, arena_Reset). "
     f"WHY: invalidates the saved snapshot and last pair index; distinct from the existing ResetEntrySelectionState ({B('ResetEntrySelectionState')}), which clears the live D_800EF980 requests and D_800A33E8 / D_800A33EC. "
     f"COUNTER-EVIDENCE: does not clear live requests or pair choices. Prior manifests: no_name.tsv w49/w61 (cache scope known, asset family not claimed). {T('0x80049E1C')}"),

    ("0x80049E4C", "node_InitRootNodePairWithNegatedRotationMatrices", "RENAME",
     f"{ch('0x80049E4C')}BODY: {B('func_80049E4C')}: two Unk80101DF0Record nodes, D_80101DF0 (unk0 0x64, rotation_order 5) and D_800FF638 (unk0 0x65, rotation_order 2): rot and translation zeroed, parent_node = 0, node_SetMatrixFromNegatedRot on each, "
     f"then D_800A3708 = &D_80101DF0, D_800A370C = &D_800FF638. CALLERS: func_80046B44 ({L('368E4.c', r'^\s+func_80049E4C\(\)')}) only. The two pointers are used by src/main/32D04.c:1128-1131 (MulMatrix chain) and camera_CalcAngles. "
     f"WHY: Root = parent_node 0, NodePair = the two records, NegatedRotationMatrices = node_SetMatrixFromNegatedRot on both. COUNTER-EVIDENCE: no claim that either is a camera target or what each composes; the unk0 tags are not used. "
     f"Prior manifests: no_name.tsv w49/w61 (second record's renderer role vs camera identity; this name claims neither). {T('0x80049E4C')}"),
]

DATA = []
