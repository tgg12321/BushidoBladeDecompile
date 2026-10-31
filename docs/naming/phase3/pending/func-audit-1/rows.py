"""wave105 rows (func-audit-1): function-name correction wave, the func track's counterpart of var-locals-audit-1.
12 already-admitted function names whose name-audit findings (overclaims / wrong) were re-derived, each renamed to
a truer name. Research input: tmp/codex/func-audit-1-research.md (12 RENAME, 0 RESET, 5 KEEP; untrusted proposals);
every home and use re-derived at HEAD d4e1e57b3 (git grep -w over src include asm named_syms.txt symbol_addrs.txt
inline_asm_canonical.txt known_psyq_stdlib.txt docs/naming/function-names.csv). 0 changed, 0 dropped. The 5 KEEP
functions are not rows (reasons in notes.txt). (DATA is empty.)
"""
import glob
import os
import re

VERDICT = "CONFIRM"
VERIFIER_NOTE = "naming-reviewer pre-apply PASS 12/12 (2026-10-31, naming-a); 5 audited names KEEP (notes.txt)"
NOTES = {}

# names changed against the research (research name -> name used); an entry with old == new is a kept row
ADJUSTED = {
    "0x8002798C": ("fighter_TestMoveFrameWindowOrMoveKindWithObjectVariant", "fighter_TestMoveFrameWindowOrMoveKindWithObjectVariant", ""),
    "0x8002C22C": ("fighter_BlendCurrentAndPreviousWeaponPointsToContactPoint", "fighter_BlendCurrentAndPreviousWeaponPointsToContactPoint", ""),
    "0x80047D94": ("wave_SampleRow8BaseCachingIndexAndRemainderParity", "wave_SampleRow8BaseCachingIndexAndRemainderParity", ""),
    "0x80047E5C": ("wave_SampleStepTableBaseAtCachedIndex", "wave_SampleStepTableBaseAtCachedIndex", ""),
    "0x80047ED0": ("wave_RebaseCellBuffer", "wave_RebaseCellBuffer", ""),
    "0x80074488": ("menu_DrawOptionSheetsWithSelectionEffectsAndGreyTiles", "menu_DrawOptionSheetsWithSelectionEffectsAndGreyTiles", ""),
    "0x8005D46C": ("gpu_DrawSheetPairSelectedByDrawMode", "gpu_DrawSheetPairSelectedByDrawMode", ""),
    "0x8005FA98": ("gpu_DrawSelectedStretchedSheetPair", "gpu_DrawSelectedStretchedSheetPair", ""),
    "0x80041BF4": ("gpu_RestoreAndTintBackedUpFixedSlotVramRowsAndRebuildFogLevels", "gpu_RestoreAndTintBackedUpFixedSlotVramRowsAndRebuildFogLevels", ""),
    "0x800224E0": ("motion_SelectVariantNibblePositionMatchingEitherPreferredNibbleInOpponentValue", "motion_SelectVariantNibblePositionMatchingEitherPreferredNibbleInOpponentValue", ""),
    "0x8006BEC4": ("gpu_DrawSheetPairPanelWithOptionalSelection", "gpu_DrawSheetPairPanelWithOptionalSelection", ""),
    "0x8004A4E0": ("gpu_DrawQueuedEntries", "gpu_DrawQueuedEntries", ""),
}
DROPPED = {}

WT = os.environ.get("P3WT", r"C:\Users\Trenton\Desktop\bb2-worktrees\ln-naming-a")

_SRC = {}
for _f in sorted(glob.glob(os.path.join(WT, "src", "main", "*.c"))):
    _SRC[os.path.basename(_f)] = open(_f, encoding="utf-8", errors="replace").read().split("\n")


def B(fn, f):
    """src/main/<file>:<first>-<last> of the C definition of fn (INCLUDE_ASM line when no C body)."""
    lines = _SRC[f]
    for k, l in enumerate(lines):
        if l.startswith("INCLUDE_ASM") and re.search(r"\b%s\b" % fn, l):
            return f"src/main/{f}:{k + 1}"
        if not re.match(r"^[A-Za-z_][\w \*]*\b%s\(" % fn, l) or l.startswith("extern"):
            continue
        j = k
        while j < len(lines) and lines[j] != "}" and not (j == k and l.rstrip().endswith("}")):
            j += 1
        return f"src/main/{f}:{k + 1}-{j + 1}" if j != k else f"src/main/{f}:{k + 1}"
    raise SystemExit(f"no C definition of {fn}")


def R(addr, new, fn, cfile, audit, body, why, counter):
    assert ADJUSTED[addr][1] == new, (addr, new)
    ev = (f"AUDIT: {audit}. BODY: {B(fn, cfile)}: {body}. WHY RENAME: {why}. COUNTER-EVIDENCE: {counter}. "
          f"Research input tmp/codex/func-audit-1-research.md, every use re-derived at HEAD (no change against the research).")
    return (addr, new, "RENAME", ev)


FUNCS = [
    R("0x8002798C", "fighter_TestMoveFrameWindowOrMoveKindWithObjectVariant",
      "fighter_TestMoveFrameWindowOrObjectVariantKind", "17AFC.c",
      "audit-43 (overclaims; suggested fighter_TestMoveFrameWindowOrVariantMoveKind), audit-201 (overclaims; suggested fighter_TestMoveFrameWindowOrSelectedMoveKindsWithObjectVariant)",
      "returns 1 when move_kind is none of 6 / 0x25 / 0x33 / 4 / 0x14 and move_frame lies in [frame_window_start, frame_window_end]; "
      "otherwise returns 1 only when object_variant_index != -1 and move_kind is one of 0xF / 0x1C..0x21 / 0x2B; else 0. Callers "
      "(17AFC.c fighter_TryDropQueuedHeadAtBone, 9F9C.c fighter_Update) run it only when variant_index == object_variant_index and "
      "object_variant_enabled and use the result as a yes/no gate",
      "the second test reads the fighter's move_kind, not any kind of the object variant (game.h object_variant_index is an s16 index; "
      "no variant record is loaded); the variant only gates it. 'ObjectVariantKind' reads as a kind belonging to the variant, which "
      "nothing in the body reads",
      "the name does not list which move kinds; claims only the two tests and the variant gate"),
    R("0x8002C22C", "fighter_BlendCurrentAndPreviousWeaponPointsToContactPoint",
      "fighter_BlendCurrentAndPreviousContactPoints", "17AFC.c",
      "audit-201 (overclaims; suggested fighter_BlendWeaponEndpointsToContactPoint)",
      "per fighter k, bit k of g_fight_weapon_contact_result picks SPAD second_weapon_points[k][0..1] (else weapon_points[k][0..1]) summed "
      "into scr->unkA8 and the matching rec->prev_second_weapon_points (else prev_weapon_points) summed into scr->unkB8; then "
      "contact_point = (3 * unkA8 + unkB8) >> 4, a 3:1 current-to-previous blend of the mean of four points. One caller: "
      "fight_UpdateFighterInteractionsAndPointCaches (17AFC.c / .s jal), result passed to fighter_ResolveWeaponContact",
      "the inputs are the current and previous weapon points (ScrPad.weapon_points / second_weapon_points and the prev_* copies); none is "
      "a contact point. The only contact point is the single output scr->contact_point (game.h). 'Points' not 'Endpoints': weapon_points "
      "holds three per fighter and nothing establishes [0] / [1] as ends",
      "the name does not state the 3:1 weighting or the bit-k selection; docs/engine/combat.md:28 cites the old name (doc, not rewritten by the wave)"),
    R("0x80047D94", "wave_SampleRow8BaseCachingIndexAndRemainderParity",
      "wave_SampleLastRowCachingIndexAndRemainderParity", "368E4.c",
      "audit-32 (wrong; suggested wave_SampleWordsFromLastRowBaseCachingIndexAndRemainderParity), audit-102 (wrong, weak lead), audit-210 (wrong; suggested wave_SampleRow8BaseCachingIndexAndRemainderParity)",
      "column_index = (a0 + 0x7D00) / 3200 rejected only when (u32)column_index >= 18 (so 0..17 accepted); stores g_wave_cached_column_index and "
      "g_wave_cached_remainder_parity = (a0 mod 3200) & 1; returns ((grid[8][ci] * parity + grid[8][ci+1] * (0x1000 - parity)) >> 12) - 0x3F48, "
      "else 0xFFFE7960. No call site (no jal, no address word). Layout (asm/data/94000.bss.s): g_wave_vertex_grid 0x800EF59C rows 0-7, "
      "D_800EF7BC + D_800EF7C0 = row 8 (17 words to 0x800EF7FF), g_wave_last_row_steps 0x800EF800, g_gpu_vram_scrolls 0x800EF848; "
      "the asm indexes from D_800EF7BC",
      "two of the eighteen accepted indices sample step storage, not the last row (ci 16: second read is word 17 = g_wave_last_row_steps[0]; "
      "ci 17: both reads past row 8), so 'SampleLastRow' is contradicted by the body itself; a neutral reset would drop what is true (base, cached "
      "index, parity weight)",
      "named_syms.txt:16 (RETIRED note) and :270 (RESET note) name the function in comments only; no caller to confirm the intent"),
    R("0x80047E5C", "wave_SampleStepTableBaseAtCachedIndex",
      "wave_SampleLastRowStepsAtCachedIndex", "368E4.c",
      "audit-32 (wrong; suggested wave_SampleWordsFromStepBufferBaseAtCachedIndex), audit-102 (wrong, weak lead), audit-210 (wrong; suggested wave_SampleStepTableBaseAtCachedIndex)",
      "ci = g_wave_cached_column_index (written only by 0x80047D94), rejected when >= 18; returns (g_wave_last_row_steps[ci] * parity + "
      "g_wave_last_row_steps[ci+1] * (0x1000 - parity)) >> 12 with parity = g_wave_cached_remainder_parity, else 0. No call site. The only writer "
      "of the step table is wave_Update, g_wave_last_row_steps[j] for j 0..16; word 17 is never written and ci + 1 = 18 reads g_gpu_vram_scrolls[0]",
      "at ci 16 the second read is the never-written word 17 and at ci 17 the reads are word 17 and the first word of another table, so "
      "'SampleLastRowSteps' (row 8's steps) is contradicted at accepted indices; StepTableBase = indexed from the base of g_wave_last_row_steps "
      "(true at every accepted index); AtCachedIndex kept (no argument read). Pair with 0x80047D94",
      "comment cite at 368E4.c:306 names this function (comment only, retargeted by the apply)"),
    R("0x80047ED0", "wave_RebaseCellBuffer",
      "wave_MoveCellBuffer", "368E4.c",
      "audit-210 (overclaims; suggested wave_RebaseCellBuffer)",
      "g_wave_cell_buffer = (s16 *)((u8 *)g_wave_cell_buffer + byte_offset); no copy, no other write. One caller: resource_RebaseLoadedBlockSections "
      "case 11 (368E4.c:280, local extern 368E4.c:261, asm jal in resource_RebaseLoadedBlockSections.s)",
      "'Move' says the buffer is moved, which the body never does; it follows a buffer the arena already moved, which is what every sibling in the same "
      "callback is named for (reloc_RebaseBlock, collision_RebaseGrid, softbody_RebaseObject, motion_RebasePlayback, node_RebaseRetainedDrawNodeBuffer)",
      "none beyond the single caller; the name claims only the pointer fix-up"),
    R("0x80074488", "menu_DrawOptionSheetsWithSelectionEffectsAndGreyTiles",
      "menu_DrawSheetsWithSelectedPulseOffsetDimmedOthersAndGreyTiles", "63D2C.c",
      "audit-29 (wrong; suggested menu_DrawSheetsWithConditionalSelectionPulseOffsetsDimmingAndGreyTiles), audit-108 (wrong; suggested menu_DrawOptionSheetsWithSelectionEffectsAndGreyTiles)",
      "for sheet_index 0..14 of sel_option_sheets picks a style from the selection mask and draws with gpu_AddSpriteCells, then a DR_MODE and "
      "gpu_AddThreeGreyTilesStackedUp. Selected value sheets 5-13 pulse and offset only when their row is the cursor row; unselected sheets 0-4 are "
      "untinted and semi-transparent, 5-13 colour 0x40, sheet 14 untinted and opaque. One caller: menu_RunSelPhases (64FD8.c:1296)",
      "'SelectedPulseOffset' fails for selected value sheets off the cursor row (no pulse, no offset); 'DimmedOthers' fails for unselected sheet 14 "
      "(full brightness, opaque) and for unselected rows 0-4 (untinted, semi-transparent rather than colour-dimmed); one contradicting use fails the name. "
      "OptionSheets = the table walked is sel_option_sheets",
      "the name does not say which effect each sheet gets; comment cite include/bb2.h:61"),
    R("0x8005D46C", "gpu_DrawSheetPairSelectedByDrawMode",
      "gpu_DrawFixedSpritePairWithSelectedSheets", "4D0A8.c",
      "audit-30 (overclaims; suggested gpu_DrawTwoFixedSpriteGroupsWithSelectedSheets)",
      "sheet_row_index = draw_mode - 1, then two gpu_AddScaledSpriteQuads draws at x = y = 0, scale 0x100: header D_8009B2C8[row][0] with cells "
      "D_8009B340 (3 cells) and header D_8009B2C8[row][1] with D_8009B358 (6 cells); returns the quad cursor. One caller: gpu_DrawDisplayModeOnLocalOtAndSync "
      "(6CF8.c:554, temp2 = the local-OT draw mode)",
      "what comes in a pair is the two sprite-sheet headers (drawn as nine sprite quads), so 'SpritePair' miscounts the sprites; 'WithSelectedSheets' is the "
      "same two sheets said twice; the row is chosen from the draw_mode argument",
      "comment cites at 4D0A8.c:325 and :1979 name the function (comments only)"),
    R("0x8005FA98", "gpu_DrawSelectedStretchedSheetPair",
      "gpu_DrawSelectedStretchedSpritePair", "4D0A8.c",
      "audit-30 (overclaims; suggested gpu_DrawTwoSelectedStretchedSpriteGroups)",
      "scale_x 0x200, scale_y 0x100; draws header D_8009B63C[sheet_index] with cells from D_8009B660 (counts 2 / 1 / 4) and then header D_8009B610[sheet_index] "
      "with the one cell D_8009B634; returns 0x190. Callers 2B344.c:993 (0), 9F9C.c:1578 (2), :1605 (1), :2325 (0) and asm jals in fight_UpdateRotatingViewThreeOptionChoice, "
      "fight_UpdateFighterFocusViewAndThreeItemMenuWithConfirmDelayedPhaseChange, fight_UpdateModeDisplaysAndMatchProgress (x2); all pass 0..2",
      "same reading as the sibling 0x8005D46C: the pair is two sprite-sheet headers drawn as up to five quads, so 'SpritePair' miscounts; Selected and Stretched hold",
      "comment cites at 4D0A8.c:3 and :1252 name the function (comments only)"),
    R("0x80041BF4", "gpu_RestoreAndTintBackedUpFixedSlotVramRowsAndRebuildFogLevels",
      "gpu_RestoreTintBackedUpFixedSlotVramRowsAndFogLevels", "31D3C.c",
      "audit-30 (overclaims; suggested gpu_RestoreAndTintBackedUpFixedSlotVramRowsAndRebuildFogLevels)",
      "returns unless slot 1's object exists with the backup asset index and a row list; converts the tint to 4.12 multipliers; for both VRAM copies LoadImages "
      "every 16x1 row from g_gpu_store_buf and tints it in place with gpu_TintVramRowInPlace; then, only when gpu_GetFogClutObject() is slot 1, calls "
      "gpu_BuildFogBlockLevels. Callers 2B344.c:561, 9F9C.c:1432 / :1968 / :1972 / :1979, asm jals in fight_Init, fight_StartDescriptorPlaybackAndAudio, "
      "fight_UpdateDefeatDelayAndModeProgress; all pass a red / green / blue tint",
      "'RestoreTint ... AndFogLevels' makes the fog levels a second object of restore and tint; nothing restores or tints them, they are built again from the "
      "just-tinted CLUT block",
      "the rebuild is conditional on the fog CLUT object being slot 1; the name states the action, not the condition. Comment cites include/bb2.h:302 and "
      "symbol_addrs.txt:760 / :1939-1941 name the function (comments only)"),
    R("0x800224E0", "motion_SelectVariantNibblePositionMatchingEitherPreferredNibbleInOpponentValue",
      "motion_SelectVariantNibblePositionMatchingPreferredPairInOpponentValue", "9F9C.c",
      "audit-33 (overclaims; suggested motion_SelectVariantNibblePositionMatchingEitherPreferredNibbleInOpponentValue)",
      "p = g_motion_preferred_variant_nibble_pairs[g_fight_step_row_column]; val = g_fighter_motion_data_ids[other->character_data_index][other->weapon_id]; "
      "for j = 0, 1 and i = 0..2 returns the first i whose nibble (val >> 4i) & 0xF equals p[j]; else 0. One caller: fighter_Init (9F9C.c:3948)",
      "the table row holds two preferred nibbles; the body never requires both, it returns the position of the first nibble matching p[0], else the first matching p[1]; "
      "'MatchingPreferredPair' reads as matching the pair",
      "the name does not encode the p[0]-first priority or the default 0; 78 characters but all parts are the admitted ones. Comment cite src/main/d_7D920.c:634 names the function"),
    R("0x8006BEC4", "gpu_DrawSheetPairPanelWithOptionalSelection",
      "gpu_DrawSheetPairCenteredTileWithOptionalSelection", "596CC.c",
      "audit-36 and audit-107 (wrong; suggested gpu_DrawSheetPairPanelWithOptionalSelection)",
      "draws sheet pair sheet_pair_index (gpu_DrawIndexedSheetPair); with a selection (index != -1) also draws sheet pair 0x12 at a y offset and three one-pixel red "
      "tiles under the selected cell and sets h = 16; then one semi-transparent black TILE at x = 0x140 - w/2, y = 0x78 - box_h/2, size w by box_h + h. Callers "
      "28708.c:557 (-1), :606 (cursor 0/1), :638 (-1), :696, :808 (-1), :921 and asm jals in memcard_RetryLoadDrawStatusSheetsAndWaitForTimeoutOrPress (x2), "
      "memcard_StartLoadOnceStepAndDrawSheetPair, memcard_StepSaveOverwriteAndFormatPrompts (x2), menu_StepTwoChoicePromptDrawSheetsAndRearmAfterResult",
      "at 28708.c:606 and :921 a selection is passed, the tile is box_h + 16 tall but its top is still placed for box_h, so it is centred on x only; 'CenteredTile' "
      "fails at those calls. Panel is the noun the TU's own globals carry (g_sheet_pair_panel_tile_cursor / _buffers)",
      "comment cites at include/game.h:2537, 28708.c:854 and symbol_addrs.txt:1440 name the function (comments only)"),
    R("0x8004A4E0", "gpu_DrawQueuedEntries",
      "gpu_DrawQueuedNodes", "3AB48.c",
      "audit-40 (overclaims; suggested gpu_DrawQueuedEntries)",
      "whole-body canonical asm (INCLUDE_ASM; inline_asm_canonical.txt:200, known_psyq_stdlib.txt:28): walks from g_draw_queue to g_draw_queue_cursor, loads each entry "
      "pointer, reads its byte 0 and jumps through jtbl_8001541C. One caller: gpu_PrepareViewAndFlushDrawQueue (32D04.c:1159; local extern 32D04.c:1131; asm jal). "
      "Queue writers: 2B344.c:2053-2055 queues &g_gpu_grid_cell_records[record_id], :2075-2079 the g_gpu_grid_node_records entries, 368E4.c queues &g_wave_draw_node, "
      "more at 31548.c, 35000.c",
      "'Nodes' claims every queued entry is a node; the grid cell records are not, by the repo's own cell-record vs node-record split; the body draws every entry of the "
      "draw queue by its kind byte. LEAD (not in this batch): wave100's gpu_ProjectQueuedNodeRunVerticesAndDispatchPackets (0x8004C404, the kind-12 handler) took "
      "'QueuedNode' from this name",
      "APPLIER WATCH: the name also occurs in inline_asm_canonical.txt:200 and known_psyq_stdlib.txt:28 and a comment at 32D04.c:1226; naming_wave.py lists both .txt files "
      "in its rule-file set (lines 80, 97), confirm the dry run's file edits show them"),
]

DATA = []
