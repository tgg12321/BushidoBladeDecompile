"""wave125 rows (func-second-pass-33): func_80044F30 (src/main/35000.c:238), func_80044F80 (35000.c:248) and func_80046A80 (src/main/368E4.c:413-415).
Research input tmp/codex/func-second-pass-33-research.md (untrusted; every claim re-derived at HEAD 95f22b253, see notes.txt).
Verdicts CONFIRM (naming-reviewer pre-apply PASS 3/3). The three NO_NAME rows of the research file (0x80046934, 0x8004695C, 0x800469A0) are not rows here (notes.txt). (DATA is empty.)
"""
VERDICT = "CONFIRM"
VERIFIER_NOTE = "naming-reviewer pre-apply PASS (round 1, 3/3, 2026-11-06, naming-a)"
NOTES = {}
DROPPED = {}

FUNCS = [
    ("0x80044F30", "ndata_ReadStageFile", "RENAME",
     "BODY: src/main/35000.c:238: void func_80044F30(s32 a0, s32 dest_addr) { ndata_Read(a0 + 0x27, dest_addr); } (asm/funcs/func_80044F30.s:4-5: "
     "jal ndata_Read with addiu a0, a0, 0x27 in the delay slot); nothing else, no return value. "
     "CALLERS: exactly two, both in resource_LoadOrReuseSelectedBlock (368E4.c:41): :87 into staging_addr (then arena_Copy into the block) and :89 straight into "
     "the block s0 = arena_OpenBlock(7) (:84); asm/funcs/resource_LoadOrReuseSelectedBlock.s:70 is the one jal. No other jal, j, address word or table use in src "
     "include asm (git grep -w func_80044F30: the C definition, the two calls, include/bb2.h:918 extern, the glabel). "
     "The selector is a stage index at every origin: resource_LoadOrReuseSelectedBlockSetLightingAndBuildFogImageLevels(g_fight_stage_index, ..) (9F9C.c:1752-1753) or n = (control_word & 0x3F) - 1 "
     "(43500.c:834, passed at :837/:841), which hands the selector to resource_LoadOrReuseSelectedBlock (368E4.c:560); g_fight_stage_index is the landed stage global (include/bb2.h:327). The file read into block 7 is then parsed as a "
     "stage: collision grid (368E4.c:180 collision_SetGrid), fight start-position records (:182 g_fight_position_records), TIM group and grid stream "
     "(:178-179), with per-selector header cases for selectors 3/4/7/11/13/18/34 (the switch at :140-175). "
     "NDATA: ids 0x27..0x4C are DATA2 S00..S37 (docs/formats/NDATA.md:145, :183; base 0x27 lands on S00 by the pool-to-INF rule :130-133; pool strings "
     "src/main/d_15EC.c:519-537 spell DATA2 S37 down to S01, S00 follows). "
     "WHY: ndata_ prefix = the only callee is ndata_Read, as for the landed sibling ndata_ReadIndexInRangeSelectedByFlag (35000.c:240-246, the WEP reader); "
     "Read = the whole effect (one ndata_Read of the file); StageFile = the file it reads is DATA2 S<selector> for every caller (stage selector in, file parsed as a "
     "stage by the one caller). "
     "COUNTER-EVIDENCE: (1) no range check: the selector range 0..37 is a data fact, not a code bound (NDATA.md:187 and following); no caller passes 38 or more; "
     "(2) the caller's landed name resource_LoadOrReuseSelectedBlock avoids the word stage (it predates this evidence; its parse is stage-shaped throughout, so "
     "no conflict); (3) the S<n> filename agrees with, but is not independent of, the caller parse (NDATA.md is a foundation note; re-derived here against the "
     "d_15EC.c pool and the callers); (4) forwarder class (naming-final-classes: one call of a named callee, arguments passed through unchanged): not met, a0 is "
     "biased by 0x27, and that bias is what separates it from its siblings (0x4D, 0x83, 0x10C). "
     "Fresh: ndata_ReadStageFile, ndata_ReadStage, ndata_ReadStageData occur nowhere in src, include, symbol_addrs.txt, named_syms.txt, function-names.csv. "
     "Aliases: none (census aliases column empty). Prior ledger: docs/naming/phase3/no_name.tsv:18 (blocker was a bias-forwarder sibling, now separated by what "
     "each bias selects). Runtime: fight captures only per the research file (facts.tsv:347/:1149, not re-derived); no runtime role claimed beyond the body. "
     "Census docs/naming/function-names.csv:511."),
    ("0x80044F80", "ndata_ReadNodeAssetFile", "RENAME",
     "BODY: src/main/35000.c:248: void func_80044F80(s32 a0, s32 dest_addr) { ndata_Read(a0 + 0x4D, dest_addr); } (asm/funcs/func_80044F80.s:4: jal ndata_Read); "
     "nothing else, no return value. "
     "CALLERS: exactly one, resource_LoadOrReuseGrayscaleAdjustedNodeAsset (368E4.c:335), the call at :351 (asm/funcs/resource_LoadOrReuseGrayscaleAdjustedNodeAsset.s:33 "
     "jal), reading into block_base = arena_OpenBlock(8) (:350). No other jal, j, address word or table use (git grep -w func_80044F80: definition, the call, "
     "include/bb2.h:919 extern, the glabel, and the symbol_addrs.txt:986 MISNAMED note). "
     "The id is a node-asset id: node_asset_id comes from g_node_retained_draw_asset_ids (368E4.c:1591-1594; table d_89C78.c:201, values 0x00..0x1A and 0xFF = none), "
     "plus 0x1B when gpu_RefreshGrayscaleMode() (:340-342). Block 8 is the landed node-asset block: resource_ResizeNodeAssetBlockAndUpdatePeak (:368, "
     "arena_Resize(8)), resource_RebaseNodeAssetBlock (:373, registered as block 8's move callback at :364), resource_FreeNodeAssetBlock (:378, arena_Free(8)); "
     "the caller then reads the file's header words 1 and 2 as section offsets, relocates, and caches the id in g_resource_cached_node_asset_id. "
     "NDATA: ids 0x4D..0x67 are DATA5 T00..T26 and 0x68..0x82 DATA6 TINYM00..TINYM26 (NDATA.md:146-147, :184; base 0x4D lands on T00, 0x4D + 0x1B on TINYM00, :131-132); "
     "each pair is the colour vs grayscale version of one asset. "
     "WHY: ndata_ prefix and Read = as func_80044F30 (one ndata_Read of the biased id); NodeAsset = the landed block-8 vocabulary of its only caller "
     "(resource_*NodeAsset*) and of the retained-draw tables the id comes from; File = it reads the file only (the caller alone relocates, sets the end, caches "
     "the id and applies the grayscale +0x1B), which tells it from the caller. "
     "COUNTER-EVIDENCE: (1) the grayscale adjustment is the caller's, so the name does not claim it; NodeAsset covers both the T and TINYM groups; (2) a tiny-model "
     "reading exists (the string 'Destruction tiny model.' at 43500.c:774 and the TINYM filenames), so ndata_ReadTinyModelFile is the alternative; left out "
     "because the colour T group is not named tiny and the caller's own landed word is node asset; (3) forwarder class not met: the 0x4D bias is not an unchanged "
     "pass-through (same reasoning as func_80044F30). "
     "Fresh: ndata_ReadNodeAssetFile, ndata_ReadNodeAsset, ndata_ReadTinyModelFile occur nowhere in src, include, symbol_addrs.txt, named_syms.txt, function-names.csv. "
     "Aliases: none in the census; symbol_addrs.txt:986 is a MISNAMED note line under the neutral name (func_80044F80 = 0x80044F80), not an alias. "
     "Prior ledger: docs/naming/phase3/no_name.tsv:19. Census docs/naming/function-names.csv:513."),
    ("0x80046A80", "motion_RebasePlaybackBlock", "RENAME",
     "BODY: src/main/368E4.c:413-415: void func_80046A80(s32 a0, s32 byte_offset) { motion_RebasePlayback(byte_offset); } (asm/funcs/func_80046A80.s:4-5: jal "
     "motion_RebasePlayback with a1 moved to a0 in the delay slot); a0 (the block id) is dropped. "
     "CALLERS: no call; one address use, arena_SetMoveCallback(0xA, (s32)func_80046A80) in motion_LoadPlaybackBlock (368E4.c:394-409, the store at :407; "
     "asm/funcs/motion_LoadPlaybackBlock.s:28-29 lui/addiu %hi/%lo). Invoked by arena_ShiftBlocks as g_arena_blocks[i].fn(id, byte_offset) (35000.c:384-385) when "
     "the blocks after a resized one move. Prototype at 368E4.c:392. git grep -w func_80046A80: definition, prototype, that store, the glabel; no other word. "
     "Block 0xA holds the DATA7 MARcc_mm motion playback file (NDATA.md:185, :207): opened by motion_LoadPlaybackBlock (arena_OpenBlock(0xA), ndata_ReadChecked, "
     "arena_SetBlockEnd(0xA)), freed by motion_FreePlaybackBlock (368E4.c:411, arena_Free(0xA)). The callee motion_RebasePlayback (4510C.c:217) adds the delta to "
     "g_motion_playback_record's block_base, camera_stream and the non-zero motion_streams (the pointers into that file; game.h:726-754). "
     "WHY: Rebase = it relocates the playback pointers by the byte offset (the callee's effect, its only effect); PlaybackBlock = it is the move callback "
     "of arena block 0xA, the block the landed motion_LoadPlaybackBlock / motion_FreePlaybackBlock open and free. It completes that Load/Free/Rebase trio and "
     "follows the landed Rebase<Block> move-callback pattern: resource_RebaseNodeAssetBlock (368E4.c:373, block 8), resource_RebaseLoadedBlockSections (:267, "
     "block 7). "
     "COUNTER-EVIDENCE: (1) the effect is wholly the callee's (4510C.c:217), so a strict reader may say the name restates motion_RebasePlayback; it states the "
     "callback role and block (a0 dropped, offset moved to a0 = the arena's fn(id, offset) signature adaptation), which the callee's name does not; the prior "
     "no_name.tsv:26 objection was exactly that restatement; (2) forwarder class (naming-final-classes: one call of a named callee, arguments passed through "
     "unchanged): arguably met in part (one call, offset passed through) but the block id is dropped and the argument position changes, so the reviewer rules; "
     "if it rules forwarder the row is dropped, not patched; (3) the sibling callbacks are not pure (resource_RebaseNodeAssetBlock makes two calls); "
     "(4) never hit at runtime (facts.tsv:818 per the research file, not re-derived), so no runtime role is claimed. "
     "Fresh: motion_RebasePlaybackBlock occurs nowhere in src, include, symbol_addrs.txt, named_syms.txt, function-names.csv. Aliases: none (named_syms.txt:268 is "
     "a RESET note for the retired snd_SelectionCallback_80046A80, history only). Census docs/naming/function-names.csv:565."),
]

DATA = []
