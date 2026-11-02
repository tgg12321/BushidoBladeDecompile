"""wave121 rows (func-second-pass-30): func_80048744 (src/main/368E4.c:1453-1459) and func_80069E18 (src/main/596CC.c:623-670).
Research input tmp/codex/func-second-pass-30-research.md (untrusted; every claim re-derived at HEAD f613d4089, see notes.txt).
Verdicts PENDING (the reviewer fills them). The six NO_NAME rows of the research file are not rows here (notes.txt). (DATA is empty.)
"""
VERDICT = "CONFIRM"
VERIFIER_NOTE = "naming-reviewer pre-apply PASS (round 1, 2/2, 2026-11-01, naming-b)"
NOTES = {}
DROPPED = {}

FUNCS = [
    ("0x80048744", "gpu_SetColorMode", "RENAME",
     "BODY: src/main/368E4.c:1453-1459: void func_80048744(s32 grayscale_flag): g_color_mode = grayscale_flag ? 1 : 0 (two sh to "
     "D_800F6652, asm/funcs/func_80048744.s:4-5, :9-10); no call, no return value, no other write. "
     "CALLERS: none. Raw scan of disc/SLUS_006.63 and disc/STR/MOVOVL.EXE for the jal word, j word, 32-bit address word and "
     "lui/addiu|ori pair = 0 hits; git grep -w func_80048744 over src include asm = the C definition and the asm glabel only; no "
     "table word. Never hit in 135 captures (docs/naming/runtime/matrix/facts.tsv:826, per the research file). "
     "g_color_mode = landed global (docs/naming/phase3/wave17/data_keep.csv:17; extern s16 368E4.c:315). Its only writers: "
     "func_80046B44 (368E4.c:439, = 0 at init), gpu_RefreshGrayscaleMode (368E4.c:1444-1451: sets 1 when gpu_GetSavedGrayscaleFlag() "
     "(6CF8.c:114, g_saved_record.flags & 1) else 0, and returns it) and this body (same 1/0 store from its argument). Its readers "
     "are only that return value: every use of g_color_mode as a value goes through gpu_RefreshGrayscaleMode()'s result "
     "(31D3C.c:115/269/468/508, 32D04.c:18, 368E4.c:336/1283/1350), each choosing a grayscale path "
     "(math_Grayscale3 / math_GrayscaleRgb555 / s = 0 in the HSV conversion / node asset id + 0x1B). "
     "WHY: Set = the whole effect (one global stored, 1 or 0 from the argument). The noun is the global's landed name, which this "
     "body writes and nothing else; gpu_ is the prefix of the sibling writer gpu_RefreshGrayscaleMode, which writes the same "
     "global the same way from the saved flag. "
     "COUNTER-EVIDENCE: (1) gpu_SetGrayscaleMode is the alternative: the argument is named grayscale_flag and every consumer of the "
     "global's value picks a grayscale path, so the CODE supports 'grayscale' as the meaning of 1 and the sibling spells it that way; "
     "'ColorMode' is the global's existing name (g_color_mode, wave17 KEEP) and claims no more than that, so the setter reads as "
     "'set g_color_mode' and the two stay greppable together; the reviewer should prefer gpu_SetGrayscaleMode if it reads the "
     "global's name as not covering the value's meaning; (2) never called and never hit, so no runtime role is claimed; the name "
     "rests on the body; (3) the twin func_8004283C is NO_NAME only because its global D_800F6650 has no role, which does not apply "
     "here (research file); (4) not a Q132 pure forwarder: it stores, it does not call. "
     "Alias: symbol_addrs.txt:1231 set_color_mode_80048744 (comment '2-line setter for g_color_mode'); no src/include/asm identifier "
     "uses it. Census docs/naming/function-names.csv:623."),
    ("0x80069E18", "gpu_DrawWhiteTileAndSheets", "RENAME",
     "BODY: src/main/596CC.c:623-670: void func_80069E18(s32 draw_context_addr): takes the TILE cursor ctx[+0x18], SetTile, "
     "r0/g0/b0 = 0xFF, x0 = y0 = 0, w = 0x280, h = 0xF0, SetSemiTrans(.., 0), AddPrim(g_gpu_ot_ptr + 0x14), cursor += 1 TILE; "
     "then sheets ptr = ((Unk8006919CRec *)ctx[+4])->unk_14; SetDrawMode(DR_MODE ctx[+0x1C], 1, 0, "
     "gpu_GetSheetTPage(ptr[0], 0), 0), AddPrim(g_gpu_ot_ptr + 0x11), DR_MODE cursor += 0xC; then three gpu_AddSpriteCells "
     "calls (ot_idx 0x10, semi 0, x = y = 0, no colour) for ptr[0], ptr[1], ptr[2] in turn, each result stored back as the SPRT "
     "cursor ctx[+0x14] (596CC.c:659-670); void. "
     "CALLERS: exactly one, gpu_DrawWhiteTileAndSheetsTestPadMask (596CC.c:219-230, the call at :224, jal 0x80069278 in "
     "asm/funcs/gpu_DrawWhiteTileAndSheetsTestPadMask.s:12); raw scan of disc/SLUS_006.63 finds one jal word (file offset 0x59A78) "
     "and no j, address word or lui pair; MOVOVL.EXE 0 hits. That caller is reached from DrawWhiteTileAndSheetsAndResetFlowOnPadMatch "
     "(64FD8.c:1384-1385). The prototype is unprototyped at 596CC.c:40 and the comment at :35-37 names it. "
     "WHY: Draw = it builds and queues primitives into the ordering table (a white TILE at OT +0x14, a DR_MODE at +0x11, sprite "
     "cells at +0x10); WhiteTile = the opaque 0xFF/0xFF/0xFF 640x240 TILE; AndSheets = the sprite-sheet cell batches from the "
     "descriptor's sheet table; the words are the landed caller's own (gpu_DrawWhiteTileAndSheetsTestPadMask), whose only job "
     "besides the pad test is to call this body; gpu_ matches the caller and the callees gpu_GetSheetTPage / gpu_AddSpriteCells. "
     "COUNTER-EVIDENCE: (1) the alias draw_bg_and_sprites_80069E18 says 'bg', which the code does not show (a white TILE, not a "
     "background asset); (2) 'FullScreen' is code-true (640x240 is the draw env, 596CC.c:2625-2628) but separates no sibling; "
     "(3) 'Three' is exact (ptr[0..2]) but the landed caller's term is 'Sheets' and the count separates no sibling, so it is left "
     "out; (4) not a pure forwarder: it calls and writes; (5) never hit (facts.tsv:875 per the research file), so no runtime role is "
     "claimed. Alias: symbol_addrs.txt:1362 draw_bg_and_sprites_80069E18; no src/include/asm identifier uses it. Census "
     "docs/naming/function-names.csv:911."),
]

DATA = []
