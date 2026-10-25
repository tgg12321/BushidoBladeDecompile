"""wave80 rows (pilot-menus-knockout): Q132 role names from the menu-tour knockout capture
(docs/naming/runtime/menu-tour-knockout-1/) and the per-phase hit maps, over 9 pilot functions in
596CC.c / 64FD8.c whose runtime coverage is confined to the option-mode screens. The mechanics rest on
the code; the role (Option Menu / Memory Card / Key Configuration / option mode) rests on the hit map +
knockout only (Q132) because every static caller is itself inside the same screen. Research input:
tmp/codex/pilot-menus-knockout-research.md (not retained); every claim re-derived here from code and captures.
0x8006C1FC stays NO_NAME (DROPPED).
"""

VERDICT = "CONFIRM"
VERIFIER_NOTE = ("naming-reviewer pre-apply PASS 9/9 (round 1); two evidence texts corrected per the reviewer (key-config coverage captures; RunFlow case-2 entry paths)")

NOTES = {}

TRACE_USED = {
    '0x8006B120': True,
    '0x8006B578': True,
    '0x8006B898': True,
    '0x8006B92C': True,
    '0x8006BB68': True,
    '0x8006C168': True,
    '0x8006C21C': True,
    '0x8006CFBC': True,
    '0x80077B30': True,
}

ADJUSTED = {
    '0x8006B120': 'menu_DrawToggleChoices -> menu_DrawOptionMenuChoices: role Option Menu added (hit map + knockout, Q132). "Toggle" dropped: the body draws the whole six-row screen (title sheet, six row labels, three two-state pairs, closing tiles) and only three of the six rows are toggles; "Choices" kept.',
    '0x8006B578': 'menu_HandleToggleChoices -> menu_HandleOptionMenuChoices: role Option Menu added; "Toggle" dropped: only cursor rows 0-2 flip a bit, rows 3-5 return the Memory Card / Key Configuration / leave codes.',
    '0x8006B898': 'menu_UpdateToggleChoices -> menu_UpdateOptionMenuChoices: role Option Menu added (hit map + sole caller; the first knockout run, menu-tour-knockout-1, was not_alive and is inconclusive; the retry menu-tour-knockout-2 is ok and corroborates); "Toggle" dropped as for its Draw/Handle.',
    '0x8006B92C': 'menu_HandleReturningChoices -> menu_HandleMemCardChoices: role Memory Card added (hit map + knockout of its sibling draw). "Returning" dropped: only one of its three rows (code 1) leaves; the other two return the 2 and 3 codes. MemCard follows the memcard_ subsystem spelling of existing names (memcard_RequestLoad etc.) and no second "Menu" sits next to menu_; research proposed ...MemCardMenuChoices, shortened.',
    '0x8006BB68': 'menu_DrawReturningChoices -> menu_DrawMemCardChoices: role Memory Card added; "Returning" dropped as for its Handle; shortened from the research name menu_DrawMemCardMenuChoices.',
    '0x8006C168': 'menu_UpdateReturningChoices -> menu_UpdateMemCardChoices: role Memory Card added; "Returning" dropped; shortened from the research name menu_UpdateMemCardMenuChoices.',
    '0x8006C21C': 'menu_DrawRowBitSheetsTilesAndGradientQuads -> menu_DrawKeyConfigRowBitSheetsTilesAndGradientQuads: role Key Configuration added; the wave79 mechanical "RowBitSheetsTilesAndGradientQuads" kept whole (research dropped RowBit here but kept RowBits in the sibling; both keep it now). No button / binding meaning claimed.',
    '0x8006CFBC': 'menu_DrawRowBitsGetEmptyRowSides -> menu_DrawKeyConfigRowBitsGetEmptyRowSides: role Key Configuration added; the wave78 mechanical part and its reviewer-suggested wording kept whole.',
    '0x80077B30': 'menu_RunFlowStateWithRowBitEdit -> menu_RunOptionModeFlowStateWithRowBitEdit: role option mode added; "FlowStateWithRowBitEdit" kept whole (the state 4/5 copy of unk1D <-> unk17 around menu_UpdateRowBitMenu is still the row-bit edit); research proposed menu_RunOptionModeFlow, which drops the state-machine and edit verbs the body shows.',
}

DROPPED = {
    '0x8006C1FC': 'stays func_8006C1FC (NO_NAME): a pure forwarder, 596CC.c:1687-1689 `return menu_UpdateReturningChoices(a0, a1);`, no work of its own. A role from the hit map (Memory Card, same phases as its callee: coverage.tsv:934) would only restate menu_UpdateMemCardChoices, and its knockout (7150 / 7500: 121285 / 121303 px, knockout.tsv) is identical to its callee\'s and is the callee\'s output. Same drop reason as the five forwarders wave79 dropped.',
}

# --- TRACE clause pieces -------------------------------------------------------------------------
_KO = ("docs/naming/runtime/menu-tour-knockout-1/ (scenario menu-tour-stub, pcsx-redux, stub live from phase t_option, "
       "frames 6580 option_menu, 6700 key_config, 6880 option_menu_2, 7150 memory_card, 7500 option_toggles; "
       "knockout.manifest.json SHA1 13824aa0c5b4d7129a7cddf7557a780d183d76f1; baseline/capture.manifest.json SHA1 "
       "300fd8c5cd131ce7c110447893c615f32c92f8f4; knockout.tsv status %s")
_MT = ("docs/naming/runtime/menu-tour-hitmap-1/capture.jsonl (SHA1 b328b6f997bb4e9ae40a2f79a297ca0a2e15ff8b, "
       "hitmap.tsv; hit events carry the phase) phases %s")
_MC = ("docs/naming/runtime/memory-card-hitmap-1/capture.jsonl (SHA1 2f3bbbd7fb1d3079c0bd858146dbf2909f5bbaf7) phases %s")
_COV = ("docs/naming/runtime/matrix/coverage.tsv:%d: hit only in memory-card-hitmap-1, menu-tour-hitmap-1 and their "
        "lib- twins lib-memory-card-save-load-1 / lib-menu-tour-1 (%s); armed and never hit in the other %d of 38 captures "
        "(story, training, versus, group-battle) and in the menu-tour phases not listed")
_COVK = ("docs/naming/runtime/matrix/coverage.tsv:%d: hit only in menu-tour-hitmap-1 and its lib- twin lib-menu-tour-1 "
         "(%s); armed and never hit in the other %d of 38 captures (memory-card, story, training, versus, group-battle) "
         "and in the menu-tour phases not listed")
_OPT_MT = "t_option, option_menu, t_key, t_key_exit, option_menu_2, t_card, t_card_exit, option_toggles, t_top"
_OPT_MC = "t_option, option_menu, t_card, t_card_exit, option_menu_2"
_CARD_MT = "t_card, memory_card, t_card_exit"
_CARD_MC = "t_card, memory_card, load_empty, save, load, t_card_exit"
_KEY_MT = "t_key, key_config, t_key_exit"

FUNCS = [
    ('0x8006B120', 'menu_DrawOptionMenuChoices', 'RENAME',
     "BODY: src/main/596CC.c:1182-1328 draws, through gpu_AddSpriteCells, a title/backdrop sheet tbl[0] (:1196-1206) then a DR_MODE; "
     "six row sheets tbl[1..6] (loop :1211-1228), the row whose index equals the cursor field (D_800A34F8 >> 10) & 7 drawn "
     "solid with an rsin pulse colour (D_800A3514) at y offset D_800A34FC->unk_0C[1], the others semi-transparent; then three "
     "two-sheet pairs tbl[7..12] (loops :1230-1313) where the sheet whose index equals the stored flag D_800A3524->unk20_0 / unk20_1 / "
     "unk20_2 is solid and pulses only when the cursor field is 0 / 1 / 2; then a DR_MODE and gpu_AddThreeGreyTilesStackedUp "
     "(:1327). It draws every row of one screen: six rows, three of them two-state flags. "
     "CALLER / RETURN USE: sole caller menu_UpdateToggleChoices (596CC.c:1438; asm/funcs/menu_UpdateToggleChoices.s:29 jal); void; no address-taken site "
     "(grep src/, include/, asm/funcs, symbol_addrs.txt, named_syms.txt). The caller chain stays in one screen: menu_UpdateToggleChoices <- "
     "menu_RunFlowStateWithRowBitEdit state 0 only (64FD8.c:1359, asm/funcs/menu_RunFlowStateWithRowBitEdit.s:22 jal) <- func_80035828 case 2 only "
     "(25C38.c:279-281). "
     "WHY: Draw = the sheet and tile output; OptionMenu = role: with it stubbed the screen titled Option Menu loses its title, the six "
     "labels and the Color/B&W, Hard/Easy, On/Off choices (knockout below) and hits are confined to the option screens (Q132: role from hit map + "
     "knockout, mechanics from the code); Choices = the labelled rows and flag choices it draws (existing mechanical part). The flags it reads are the ones "
     "menu_HandleToggleChoices flips (:1368 / :1379 / :1390). "
     "COUNTER-EVIDENCE: 'Toggle' dropped (ADJUSTED): the body also draws three non-toggle rows. No claim about what Monitor / Difficulty / Music change in "
     "the game (those are screenshot text; the code only shows unk20_0/1/2). PRIOR: retired alias named_syms.txt:1724 'draw_anim_obj_8006B120' (animation guess, RESET); this name claims no animation. "
     "TRACE (corroborates; role per Q132): knockout " + _KO % "ok" + "): frames changed 6580 9519 px, 6880 9519, 7500 9519 (bbox 107,22-489,212: "
     "the Option Menu title, the six row labels and the Color/B&W, Hard/Easy, On/Off texts vanish, backdrop and clouds stay); unchanged 6700 (Key "
     "Configuration) and 7150 (Memory Card), 0 px; the game kept running (script end_frame 9000). Hit maps: " + _MT % _OPT_MT + "; " + _MC % _OPT_MC +
     " (none in key_config, memory_card, load/save, top_menu, boot_intro). " + _COV % (926, 'phases as above', 34) + "."),
    ('0x8006B578', 'menu_HandleOptionMenuChoices', 'RENAME',
     "BODY: src/main/596CC.c:1332-1428 steps a six-position cursor in D_800A34F8 bits 10-12 with pad_StepOffsetsGetDirectionEvents (:1340) "
     "and snd_QueueSound (wrap rule :1344-1362); cursor 0 / 1 / 2: a left-right event ((ret & 0xFF) != 0) flips D_800A3524->unk20_0 / unk20_1 / unk20_2 ^= 1 "
     "(:1368, :1379, :1390) and a confirm (*arg1 & 0x400040) moves the cursor to the next row; cursor 3 + confirm sets the next screen's cursor field "
     "(D_800A34F8 & 0xFFFF1FFF | 0x4000) and returns 2 (:1399-1403); cursor 4 returns 3 (:1406-1409); cursor 5 returns 1 (:1412-1415); the cancel "
     "bits 0x100010 return 1 (:1421-1423); otherwise 0. "
     "CALLER / RETURN USE: sole caller menu_UpdateToggleChoices (596CC.c:1439, asm menu_UpdateToggleChoices.s:32 jal), whose result menu_RunFlowStateWithRowBitEdit "
     "switches on at 64FD8.c:1360-1372: 1 -> return -1 (leave), 2 -> state 1 (the screen menu_HandleReturningChoices handles), 3 -> copy unk1D to unk17 and state 4 "
     "(menu_UpdateRowBitMenu). Same single-screen caller chain as menu_DrawToggleChoices; no address-taken site. "
     "WHY: Handle = it consumes pad events and returns the action; OptionMenu = role from the hit map plus the knockout of its sibling draw (the same six rows: row 3 -> 2 "
     "= Memory Card state, row 4 -> 3 = Key Configuration state, row 5 / cancel -> 1 = leave, matching the screenshot's row order Monitor, Difficulty, Music, "
     "Memory Card, Key Configuration, Return to Top Menu); Choices = the rows it dispatches. "
     "COUNTER-EVIDENCE: 'Toggle' dropped (ADJUSTED), cursor rows 3-5 are not toggles. The 6700 / 7150 pixel changes are downstream of the lost return code, not the "
     "function's own output; only 7500 shows its own writes lost. The trace does not say what the codes mean. "
     "TRACE (corroborates; role per Q132): knockout " + _KO % "ok" + "): frames changed 6700 72029 px (Key Configuration never entered: the Option Menu shows "
     "instead; downstream), 7150 11317 (Memory Card never entered: Option Menu shows; downstream), 7500 2350 px (bbox 107,119-478,180: Music stays On and the cursor stays on Key "
     "Configuration, i.e. the cursor and flag writes are lost: its own output); unchanged 6580 and 6880 (0 px, no input in those frames). Hit maps: "
     + _MT % _OPT_MT + "; " + _MC % _OPT_MC + ". " + _COV % (927, 'phases as above', 34) + "."),
    ('0x8006B898', 'menu_UpdateOptionMenuChoices', 'RENAME',
     "BODY: src/main/596CC.c:1431-1440 D_800A3514 += 1 (the pulse counter menu_DrawToggleChoices reads), picks g_gpu_db[D_800A36AC & 1], "
     "gpu_AdvanceBufferCounterAndBuildDrawContext(sp10, &D_800A3518), gpu_DrawModeTilesThreeSheetsAndThreeFlatQuads(sp10, 1, t), menu_DrawToggleChoices(sp10), and returns "
     "menu_HandleToggleChoices(&arg0, &arg1). "
     "CALLER / RETURN USE: sole caller menu_RunFlowStateWithRowBitEdit state 0 (64FD8.c:1359; asm/funcs/menu_RunFlowStateWithRowBitEdit.s:22 jal; prototype include/bb2.h:992); "
     "the result is switched on at 64FD8.c:1360-1372. No other caller or address-taken site (grep src/, include/, asm/funcs, registries; symbol_addrs.txt:1398 only "
     "mentions it in a comment). "
     "WHY: Update = one frame step (advance counter, build context, draw, handle), the shape of menu_UpdateRowBitMenu (:2161); "
     "OptionMenu = role from the hit map plus the sole caller's state 0, and from the Draw and Handle it calls, whose knockout and hit maps are Option Menu only; "
     "Choices = those two callees' noun. "
     "COUNTER-EVIDENCE: its knockout run in menu-tour-knockout-1 was not_alive (the emulator dropped the gdb link after the first stubbed call; knockout.tsv rows menu_UpdateToggleChoices, "
     "status not_alive, every frame 'NOT COMPARABLE'): that run is INCONCLUSIVE and is not cited for any pixel claim. The retry menu-tour-knockout-2 landed with status ok (below); the role "
     "still rests on the hit map and the sole caller (Q132), the retry only corroborates it. Its black frames at 6700 / 7150 are downstream (the state machine never leaves state 0 once the "
     "update is a no-op), so only the option frames count. "
     "TRACE (corroborates; role per Q132): retry docs/naming/runtime/menu-tour-knockout-2/ (same scenario menu-tour-stub, stub from phase t_option returning 0; knockout.manifest.json SHA1 "
     "9195c6e9d8caec24d4c0ff358f3348935ee65e97; baseline/capture.manifest.json SHA1 b88382c00747a69871c8ac53060f2667be9a8a89; knockout.tsv status ok, script end_frame 9000): frames changed "
     "6580 121002 px, 6880 120960, 7500 121303 (the whole Option Menu frame, backdrop included, is replaced by a black frame, SHA1 5a01b4b272fb595479a807216bb09f56d0768a96, bbox "
     "0,0-639,239) and, downstream, 6700 116972 and 7150 121285; unchanged: none of the five. First run " + _KO % "not_alive" + ") INCONCLUSIVE. Hit maps: "
     + _MT % _OPT_MT + "; " + _MC % _OPT_MC + " (none in key_config or memory_card). " + _COV % (928, 'phases as above', 34) + "."),
    ('0x8006B92C', 'menu_HandleMemCardChoices', 'RENAME',
     "BODY: src/main/596CC.c:1442-1503 steps a three-position cursor in D_800A34F8 bits 13-15 with pad_StepOffsetsGetDirectionEvents (:1451, wrap rule :1455-1473) and "
     "snd_QueueSound; cursor 0 + confirm (*arg1 & 0x400040) returns 2 (:1479), cursor 1 returns 3 (:1485), cursor 2 returns 1 and advances the option-screen cursor "
     "field (bits 10-12) by one (:1491-1494); the cancel bits 0x100010 return 1 (:1500). "
     "CALLER / RETURN USE: sole caller menu_UpdateReturningChoices (596CC.c:1684; asm menu_UpdateReturningChoices.s:32 jal). That update is reached only through the forwarder "
     "func_8006C1FC (596CC.c:1688; asm func_8006C1FC.s:4 jal), called at 64FD8.c:1377 (state 1: result 1 -> state 0, 2 -> state 2, 3 -> state 3, :1378-1383) and at 25C38.c:307 / :315 "
     "(top-flow cases 13 / 14, run each frame with arguments 0, 0 beside func_80038988 / func_80038C70). Cases 13 / 14 are entered only by D_800A3740 = 0xD / 0xE (25C38.c:298 / :302) from "
     "the RunFlow return codes 2 / 3 (25C38.c:296-302), and func_80038988 / func_80038C70 are memory-card request flows (28708.c:609 and :778 call memcard_RequestLoad / "
     "memcard_RequestSave / memcard_RequestOverwrite / memcard_RequestFormat; symbol_addrs.txt:746). Every caller is therefore inside the Memory Card screen and its Load / Save sub-flows; "
     "no address-taken site. "
     "WHY: Handle = pad events to a returned action; MemCard = role: hit map confined to the card phases (t_card, memory_card, load_empty, save, load, t_card_exit) and the "
     "knockout of its sibling draw (title 'Memory Card' with three rows); Choices = the three rows it moves between (existing noun). "
     "COUNTER-EVIDENCE: 'Returning' dropped (ADJUSTED). Returns 2 / 3 route to the two card flows by position only; the code does not say which row is which, so the name claims neither "
     "Load nor Save. PRIOR: retired alias named_syms.txt:1709 'draw_anim_sprites_seq_8006BB68' belongs to its draw (animation guess, RESET). "
     "TRACE (corroborates; role per Q132): knockout " + _KO % "ok" + "): frames changed 7500 11319 px (bbox 107,22-489,212: with its return codes lost the Memory Card screen is never "
     "left; downstream, not its own output); unchanged 6580, 6700, 6880 and 7150 (0 px, no input at the card frame). Hit maps: " + _MT % _CARD_MT + "; " + _MC % _CARD_MC +
     " (none in option_menu, key_config, top_menu). " + _COV % (929, 'phases as above', 34) + "."),
    ('0x8006BB68', 'menu_DrawMemCardChoices', 'RENAME',
     "BODY: src/main/596CC.c:1505-1558 draws a backdrop sheet from list unk_2C (s.ot_idx 0xA, :1518-1527) then a DR_MODE; three row sheets (loop :1531-1546, q advanced each pass) drawn "
     "solid at the cursor's y offset when (D_800A34F8 >> 13) & 7 equals the row index and semi-transparent otherwise; a DR_MODE from list unk_28 and "
     "gpu_AddThreeGreyTilesStackedUp at (0xE8, 0x25) (:1557). "
     "CALLER / RETURN USE: sole caller menu_UpdateReturningChoices (596CC.c:1683; asm menu_UpdateReturningChoices.s:29 jal); void; no address-taken site. The cursor field it reads is the "
     "one menu_HandleReturningChoices steps. Caller chain as for menu_HandleReturningChoices: all inside the Memory Card screen and its sub-flows. "
     "WHY: Draw = sheets and tiles; MemCard = role from hit map plus knockout (title and rows of the screen headed Memory Card vanish); Choices = the three rows with a cursor. "
     "COUNTER-EVIDENCE: 'Returning' dropped (ADJUSTED). Load Data / Save Data / Return are screenshot text; the code shows three row sheets. PRIOR: retired alias named_syms.txt:1709 "
     "'draw_anim_sprites_seq_8006BB68' (animation guess, RESET); no animation claimed. "
     "TRACE (corroborates; role per Q132): knockout " + _KO % "ok" + "): frames changed 7150 4341 px (bbox 232,22-406,169: the Memory Card title and the three row labels vanish, "
     "backdrop stays); unchanged 6580, 6700, 6880 and 7500 (0 px). Hit maps: " + _MT % _CARD_MT + "; " + _MC % _CARD_MC + " (none in option_menu, key_config). "
     + _COV % (930, 'phases as above', 34) + "."),
    ('0x8006C168', 'menu_UpdateMemCardChoices', 'RENAME',
     "BODY: src/main/596CC.c:1676-1685 D_800A3514 += 1, g_gpu_db[D_800A36AC & 1], gpu_AdvanceBufferCounterAndBuildDrawContext, "
     "gpu_DrawModeTilesThreeSheetsAndThreeFlatQuads(sp10, 1, t), menu_DrawReturningChoices(sp10), returns menu_HandleReturningChoices(&arg0, &arg1). "
     "CALLER / RETURN USE: sole caller func_8006C1FC (596CC.c:1688; asm func_8006C1FC.s:4 jal), whose callers are 64FD8.c:1377 (state 1) and 25C38.c:307 / :315 (cases 13 / 14); "
     "the result is used at 64FD8.c:1378-1383 and discarded at 25C38.c:307 / :315. All inside the Memory Card screen and its card flows (chain under menu_HandleReturningChoices); no address-taken site. "
     "WHY: Update = one frame step, the same shape as menu_UpdateRowBitMenu; MemCard = role: hit map confined to the card phases and the knockout shows no frame at all is composed "
     "without it on the card screen (callees' output: backdrop via gpu_DrawModeTilesThreeSheetsAndThreeFlatQuads plus the Draw's rows); Choices = its Draw / Handle nouns. "
     "COUNTER-EVIDENCE: 'Returning' dropped (ADJUSTED). The black frame at 7500 is not its own output (the card screen is never left once the update is stubbed); only 7150 counts. "
     "PRIOR: retired alias named_syms.txt:1438 'satan_helper_8006C168' (first-pass subsystem tag, RESET); not claimed. "
     "TRACE (corroborates; role per Q132): knockout " + _KO % "ok" + "): frames changed 7150 121285 px (whole frame black, bbox 0,0-639,239: backdrop, title and rows all gone), "
     "7500 121303 px (black; downstream, the card screen is stuck); unchanged 6580, 6700, 6880 (0 px). Hit maps: " + _MT % _CARD_MT + "; " + _MC % _CARD_MC +
     " (none in option_menu, key_config). " + _COV % (933, 'phases as above', 34) + "."),
    ('0x8006C21C', 'menu_DrawKeyConfigRowBitSheetsTilesAndGradientQuads', 'RENAME',
     "BODY: src/main/596CC.c:1696-1947 draws sprite sheets (gpu_AddSpriteCells; loops over 2 x 4 option bits and 6 x 2 row sheets, :1771-1822), 11 x 2 thin TILEs "
     "(SetTile, :1827-1840), and Gouraud POLY_G4 quads (SetPolyG4, setXYWH, :1852-1940) from the Rec_8006C21C rows MOD.BIN's unk_44 holds (include/game.h:271-276, :2319), "
     "indexed by each side's cursor unk_28.half[j] and the per-side bit bytes D_800A3524->unk17, with a DR_MODE per sheet group. "
     "CALLER / RETURN USE: sole caller menu_UpdateRowBitMenu (596CC.c:2169; asm menu_UpdateRowBitMenu.s:29 jal), whose sole caller is menu_RunFlowStateWithRowBitEdit "
     "states 4 / 5 (64FD8.c:1398), state 4 entered only from the option screen's return code 3 (64FD8.c:1366-1371; menu_HandleToggleChoices :1406-1409). No address-taken site. "
     "WHY: Draw = sheets, tiles and gradient quads as before (the wave79 mechanical RowBitSheetsTilesAndGradientQuads kept); KeyConfig = role from hit map and knockout only "
     "(Q132): hits only in t_key, key_config, t_key_exit and the knockout removes the screen headed Key Configuration (title, 1P / 2P headers, pad pictures, action labels, "
     "Default / Custom, panel cells); every static caller is the one row-bit menu state. "
     "COUNTER-EVIDENCE: no narrower meaning claimed: the screenshot text (Frontal Attack, Reverse Attack, Change Stance, Default, Custom) is not evidence for what the bits do, and the "
     "name says nothing of buttons or bindings. PRIOR: no retired alias for this address in named_syms.txt. "
     "TRACE (corroborates; role per Q132): knockout " + _KO % "ok" + "): frames changed 6700 48674 px (bbox 88,22-551,218; the pad pictures, headers, action labels, Default / Custom "
     "and panels vanish; the action icons and EXIT bars stay), unchanged 6580, 6880, 7150, 7500 (0 px). Hit maps: " + _MT % _KEY_MT + "; " + _MC % "(none: no key_config in that scenario)" +
     ". " + _COVK % (935, 't_key, key_config, t_key_exit', 36) + "."),
    ('0x8006CFBC', 'menu_DrawKeyConfigRowBitsGetEmptyRowSides', 'RENAME',
     "BODY: src/main/596CC.c:2062-2151 loops three rows (var_3 0..2) over the per-row byte D_800A3524->unk17[var_3] (low nibble = side 0, high nibble = side 1): for each of 4 columns "
     "and 2 sides it draws sheet table[column + 8] when the bit is set (x advanced 23 per drawn sheet via counts.count[side]); when a side drew nothing on that row it ORs 1 << side "
     "into result and draws the empty-row sheet table[12]; then per side a status sheet (table[17] if D_800A34FC->unk_28.word == 0x50005, else table[19] if that side's bit is "
     "set in result, else table[18]); then a DR_MODE for table[14] at OT 8 and gpu_AddThreeGreyTilesStackedUp(arg0, rect, 0x11); returns result (bit i = side i has an empty row). "
     "CALLER / RETURN USE: sole caller menu_UpdateRowBitMenu (596CC.c:2170, asm menu_UpdateRowBitMenu.s:31 jal), which passes (r << 16) >> 16 to menu_HandleRowBitInput (:2171), whose "
     "side-limit test caps the cursor when a side has an empty row; no address-taken site. Caller chain as for menu_DrawRowBitSheetsTilesAndGradientQuads. "
     "WHY: wave78 landed menu_DrawRowBitsGetEmptyRowSides (reviewer-suggested) and its mechanical wording is kept whole; KeyConfig = role from hit map + knockout only (Q132): "
     "hits only in t_key, key_config, t_key_exit and the knockout removes exactly the per-action icons and the two EXIT bars of the screen headed Key Configuration, which are the "
     "set-bit sheets and status sheets the code draws. "
     "COUNTER-EVIDENCE: no narrower meaning (buttons, actions, bindings) is claimed; which action a row is comes from screenshot text only. PRIOR: retired alias named_syms.txt:1722 "
     "'draw_anim_obj_8006CFBC' (animation guess, RESET); none claimed. "
     "TRACE (corroborates; role per Q132): knockout " + _KO % "ok" + "): frames changed 6700 6063 px (bbox 90,35-549,217: the circle / cross / square icons and the EXIT bars vanish, the "
     "rest of the screen stays), unchanged 6580, 6880, 7150, 7500 (0 px). Hit maps: " + _MT % _KEY_MT + "; " + _MC % "(none: no key_config in that scenario)" + ". "
     + _COVK % (938, 't_key, key_config, t_key_exit', 36) + "."),
    ('0x80077B30', 'menu_RunOptionModeFlowStateWithRowBitEdit', 'RENAME',
     "BODY: src/main/64FD8.c:1352-1409 switches on D_800A35E4: state 0 runs menu_UpdateToggleChoices (:1359) and maps its result 1 -> return -1, 2 -> state 1, 3 -> copy "
     "D_8009BD24.unk1D[0..2] to unk17[0..2] and state 4 (:1360-1372); state 1 runs func_8006C1FC (:1377): 1 -> state 0, 2 / 3 -> state 2 / 3; states 2 / 3 return 2 / 3; "
     "state 4 runs menu_ResetRowBitCursors then falls into state 5, which runs menu_UpdateRowBitMenu (:1398): 1 -> state 0 and copy unk17 back to unk1D, -1 -> state 0 (:1399-1405). "
     "CALLER / RETURN USE: sole caller func_80035828 case 2 (25C38.c:279-281; asm func_80035828.s:193 jal), which on -1 / 1 leaves to the top-menu state (D_800A3740 = 1, 25C38.c:282-292) "
     "and on 2 / 3 enters cases 13 / 14 (25C38.c:296-302). func_80035828 is the top-menu handler (word table d_7D870.c:39); case 2 (D_800A3740 = 2) is entered from func_80035618 case 7 (25C38.c:181-183), reached "
     "through func_80035618((s8)D_80102778.unk_D) in case 11 (25C38.c:270) and func_80035618(D_800A38DC) on the D_800A31DA post-game return (25C38.c:204-206, flag set at 2B344.c:946), "
     "and set directly by the case 13 / 14 completions (25C38.c:311, :318); the D_800A31D9 one-shot (25C38.c:200-203) sets D_800A3740 = 7, which runs func_8003880C and ends in func_80035438 "
     "(D_800A3740 = 1, 25C38.c:20-30), never case 2. Every use is the one flow that runs the three "
     "screens below it; no address-taken site. "
     "WHY: Run = it drives the state machine each frame; OptionMode = role from hit map + knockout (Q132): hits in every option phase, key_config, memory_card and the load/save phases, "
     "none in any other mode or the top menu, and stubbing it blanks the frame at all five option frames; the top menu's Option Menu entry (menu-tour-hitmap-1 phases.tsv top_menu, "
     "entry7 shot) is what precedes t_option; FlowStateWithRowBitEdit = existing mechanical part kept (D_800A35E4 state and the unk1D <-> unk17 copies around menu_UpdateRowBitMenu). "
     "COUNTER-EVIDENCE: the post-game return and case 13 / 14 entries are not isolated by a capture; each enters the same case 2, so it runs the same flow. 'Mode' is the role word for the "
     "D_800A3740 state it serves, not a claim about the top menu's other 'Mode' rows. Research's menu_RunOptionModeFlow dropped the state / edit verbs (ADJUSTED). "
     "PRIOR: retired alias named_syms.txt:1460 'mode_helper_80077B30' (alias subject unsupported, RESET); not claimed. "
     "TRACE (corroborates; role per Q132): knockout " + _KO % "ok" + "): frames changed 6580 121002 px, 6700 116972, 6880 120960, 7150 121285, 7500 121303 (whole frame black, bbox "
     "0,0-639,239, stub screenshot SHA1 5a01b4b272fb595479a807216bb09f56d0768a96; the callees compose nothing at any option screen); unchanged: none of the five. Hit maps: "
     + _MT % "t_option, option_menu, t_key, key_config, t_key_exit, option_menu_2, t_card, memory_card, t_card_exit, option_toggles, t_top" + "; "
     + _MC % "t_option, option_menu, t_card, memory_card, load_empty, save, load, t_card_exit, option_menu_2" +
     " (none in top_menu, top_menu_2, boot_intro, t_menu). " + _COV % (1021, 'option, key_config, memory_card, load/save phases', 34) + "."),
]

DATA = []
