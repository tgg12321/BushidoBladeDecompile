# func_api vein: rejected / not proposed (2026-09-24)

Scope examined: every census-AUTO function with a VERIFIED callee or a string reference
(`docs/naming/apiscan/evidence.json` + an independent rodata-string rescan of all 326 AUTO .s files,
`strscan.py`), plus every AUTO/INFERRED/SUSPECT `func_*`-glabel function <=120 insns whose calls are ALL
VERIFIED/CORROBORATED (`pure.py`), plus the CD state-machine neighbours of the candidates.

## AUTO functions with VERIFIED callees / strings that were not proposed

| addr | name | insns | reason |
|---|---|---|---|
| 0x800174F4 | func_800174F4 | 136 | ClearOTagR/DrawOTag/PutDrawEnv/SetDefDrawEnv frame-render body, but also calls func_8005D554 (AUTO) and two INFERRED helpers; a name ("draw frame") summarises control flow, not calls |
| 0x8001B294 | func_8001B294 | 75 | verified callees are only math/prim/rand helpers mixed into game logic (and/or unverified callees); any name would need game-semantic inference |
| 0x8001B478 | func_8001B478 | 134 | verified callees are only math/prim/rand helpers mixed into game logic (and/or unverified callees); any name would need game-semantic inference |
| 0x8001C820 | func_8001C820 | 47 | verified callees are only math/prim/rand helpers mixed into game logic (and/or unverified callees); any name would need game-semantic inference |
| 0x8001D790 | func_8001D790 | 93 | memcpy + 5 unverified/INFERRED/SUSPECT callees (game_StageCleanup, gpu_EnableDisplay...) - no restatement possible |
| 0x8001D904 | func_8001D904 | 37 | memcpy + obj_ExecTask/obj_InitTaskCamera (INFERRED) + gpu_EnableDisplay (SUSPECT) - no restatement possible |
| 0x8001D998 | func_8001D998 | 37 | memcpy + AUTO callees (func_8005B8B8/func_8005B98C) + SUSPECT gpu_EnableDisplay - no restatement possible |
| 0x8001DCB0 | func_8001DCB0 | 469 | 469-insn game setup; SetGeomScreen is 1 of ~34 callees |
| 0x8001F2E4 | func_8001F2E4 | 351 | verified callees are only math/prim/rand helpers mixed into game logic (and/or unverified callees); any name would need game-semantic inference |
| 0x800233AC | func_800233AC | 167 | verified callees are only math/prim/rand helpers mixed into game logic (and/or unverified callees); any name would need game-semantic inference |
| 0x800238C4 | func_800238C4 | 219 | verified callees are only math/prim/rand helpers mixed into game logic (and/or unverified callees); any name would need game-semantic inference |
| 0x80027AD8 | func_80027AD8 | 574 | verified callees are only math/prim/rand helpers mixed into game logic (and/or unverified callees); any name would need game-semantic inference |
| 0x8002AB08 | func_8002AB08 | 1112 | verified callees are only math/prim/rand helpers mixed into game logic (and/or unverified callees); any name would need game-semantic inference |
| 0x8002CD58 | func_8002CD58 | 370 | RotMatrixX/RotMatrixY/ratan2 only, but 370 insns of angle/geometry math on game structs; name would describe game computation |
| 0x8002DAD0 | func_8002DAD0 | 212 | RotMatrixX/RotMatrixY/ratan2 only, 212 insns of game geometry; same as 0x8002CD58 |
| 0x8002E838 | func_8002E838 | 123 | RotMatrixX/RotMatrixY/ratan2 only, 123 insns of game geometry; same as 0x8002CD58 |
| 0x8002EBDC | func_8002EBDC | 188 | RotMatrixX/RotMatrixY/ratan2 only, 188 insns of game geometry; same as 0x8002CD58 |
| 0x8002FC80 | func_8002FC80 | 76 | ratan2 only; angle computation on game data - computation naming, not API restatement |
| 0x80031B24 | func_80031B24 | 327 | verified callees are only math/prim/rand helpers mixed into game logic (and/or unverified callees); any name would need game-semantic inference |
| 0x800338CC | func_800338CC | 189 | rand only; game logic |
| 0x800383A4 | func_800383A4 | 173 | close() + 9 memcard helpers incl. INFERRED/AUTO ones; a multi-step memcard dispatcher - name would summarise control flow |
| 0x8003D2C4 | func_8003D2C4 | 12 | one-line LoadImage(&RECT{1008,476,16,36} @D_800A3220, D_80090178) - a name would have to assert what the fixed 16x36 image is (same reason as the README SsVabClose one-liners). named_syms.txt:2329 already flags the old katinuki name MISNAMED |
| 0x8003E164 | func_8003E164 | 50 | MoveImage + DrawSync, but also func_8003E22C (AUTO), get_global/satan_camera_setup_matrix (INFERRED), func_800432A0 (AUTO) |
| 0x8003E6D8 | func_8003E6D8 | 299 | verified callees are only math/prim/rand helpers mixed into game logic (and/or unverified callees); any name would need game-semantic inference |
| 0x80040594 | func_80040594 | 217 | DrawSync + 15 unverified callees |
| 0x80041AC8 | func_80041AC8 | 75 | StoreImage loop over a table D_80094DF0[D_80094E08[idx]] + func_8003E2A0 (AUTO) gating; name would need the table identity |
| 0x80041BF4 | func_80041BF4 | 135 | DrawSync/LoadImage + 4 INFERRED helpers |
| 0x80041EB0 | func_80041EB0 | 136 | verified callees are only math/prim/rand helpers mixed into game logic (and/or unverified callees); any name would need game-semantic inference |
| 0x80044504 | func_80044504 | 83 | verified callees are only math/prim/rand helpers mixed into game logic (and/or unverified callees); any name would need game-semantic inference |
| 0x80044800 | func_80044800 | 204 | verified callees are only math/prim/rand helpers mixed into game logic (and/or unverified callees); any name would need game-semantic inference |
| 0x80044CCC | func_80044CCC | 70 | rsin/rcos/LoadAverage12 only, but builds two game vectors (x, -r*sin, -r*cos) and interpolates; computation naming, not restatement |
| 0x80045B68 | func_80045B68 | 302 | DrawSync + 11 unverified callees |
| 0x80046BF4 | func_80046BF4 | 109 | verified callees are only math/prim/rand helpers mixed into game logic (and/or unverified callees); any name would need game-semantic inference |
| 0x800475A4 | func_800475A4 | 101 | verified callees are only math/prim/rand helpers mixed into game logic (and/or unverified callees); any name would need game-semantic inference |
| 0x800477E8 | func_800477E8 | 170 | GetClut/GetTPage + func_800417D0 (INFERRED) on game texture records |
| 0x800482C8 | func_800482C8 | 69 | LoadImage of a TIM (0x10 magic, CLUT flag 8) + INFERRED palette-convert helpers gnd_helper/efc_palette_convert_rgb555; a "load TIM" name would lean on the unverified CLUT path. Possible future row once those helpers are named |
| 0x80048BA4 | func_80048BA4 | 237 | verified callees are only math/prim/rand helpers mixed into game logic (and/or unverified callees); any name would need game-semantic inference |
| 0x8004A940 | func_8004A940 | 1162 | verified callees are only math/prim/rand helpers mixed into game logic (and/or unverified callees); any name would need game-semantic inference |
| 0x800548DC | func_800548DC | 12 | DrawSync(0) + func_8004659C (INFERRED) + snd_StopSelection (not VERIFIED) |
| 0x80055138 | func_80055138 | 516 | verified callees are only math/prim/rand helpers mixed into game logic (and/or unverified callees); any name would need game-semantic inference |
| 0x80055B60 | func_80055B60 | 1110 | verified callees are only math/prim/rand helpers mixed into game logic (and/or unverified callees); any name would need game-semantic inference |
| 0x800571C0 | func_800571C0 | 287 | verified callees are only math/prim/rand helpers mixed into game logic (and/or unverified callees); any name would need game-semantic inference |
| 0x80057CC8 | func_80057CC8 | 111 | ratan2 x2 only; game angle computation |
| 0x8005B644 | func_8005B644 | 26 | README left-out: SsVabClose one-liner with a hard-coded VAB id; also calls func_800858D0 (owner-held SsUtAllKeyOff PROBABLE). No new evidence |
| 0x8005B6AC | func_8005B6AC | 20 | README left-out (same as 0x8005B644). No new evidence |
| 0x8005B6FC | func_8005B6FC | 12 | README left-out: SsVabClose one-liner with a hard-coded VAB id. No new evidence |
| 0x8005BE84 | func_8005BE84 | 46 | already verified PLAUSIBLE (not applied) in the 2026-09-07 wave as snd_SetReverbType: arg is an index into D_8009AD1C, not a libsnd type. No new evidence |
| 0x8005C2A8 | func_8005C2A8 | 134 | README left-out (multi-step VAB transfer routine). Its string "vab id:%d mistake\n" @0x800158CC is an error message, not an identity statement. No new evidence |
| 0x8005D554 | func_8005D554 | 176 | verified callees are only math/prim/rand helpers mixed into game logic (and/or unverified callees); any name would need game-semantic inference |
| 0x80063084 | func_80063084 | 667 | verified callees are only math/prim/rand helpers mixed into game logic (and/or unverified callees); any name would need game-semantic inference |
| 0x800645B0 | func_800645B0 | 78 | rand x4 only; spawns 15 jittered records into game tables (particle-like) - game semantics |
| 0x80065800 | func_80065800 | 1456 | verified callees are only math/prim/rand helpers mixed into game logic (and/or unverified callees); any name would need game-semantic inference |
| 0x8006A564 | func_8006A564 | 199 | SetTile/SetDrawMode/AddPrim prim builder + func_8006E480 (AUTO) / func_8007352C (INFERRED) |
| 0x8006C21C | func_8006C21C | 622 | prim builder + AUTO/INFERRED callees (same family as 0x8006A564) |
| 0x80070188 | func_80070188 | 698 | prim builder + AUTO/INFERRED callees |
| 0x80070F78 | func_80070F78 | 810 | prim builder + AUTO/INFERRED callees |
| 0x80073200 | func_80073200 | 203 | prim builder + AUTO/INFERRED callees |
| 0x800753D8 | func_800753D8 | 166 | prim builder + AUTO/INFERRED callees |
| 0x80075830 | func_80075830 | 104 | rsin + INFERRED callee |
| 0x800784E4 | func_800784E4 | 30 | ClearOTagR(D_800A374C, 0x1008) + three INFERRED helpers (replay_setup_display, camera_ptr_array_apply_offset, camera_init_matrix_table) |
| 0x8008B488 | func_8008B488 | 387 | owner-held: memory/closer/libsnd-hunt-report.md PROBABLE SpuSetVoiceAttr (near-verbatim, STRUCTURAL policy 2026-08-18) - not wave material |

## Other AUTO functions examined and dropped

| addr | reason |
|---|---|
| 0x800335D8 (func_800335D8) | string rescan false positive: D_8008EBF4 bytes "|}}~|}" are table data, not text |
| 0x8003FA24 (func_8003FA24) | loads "Multipul Model" @0x80010D8C (src/config.c:9) but only passes it to func_80052C10 on an error path; the string names an error, not this function |
| 0x80052C10 (func_80052C10) | error sink: ignores its (string) argument and stores 0 to 0x1F800400 (one past the 1 KB scratchpad); 16 callers, all on error/overflow paths. Not an API restatement (no library call); a "trap/fault" name is hardware inference - flag for a separate reviewed vein |
| 0x8003A3F0 (func_8003A3F0) | func_8003A39C (comb_ResetClose candidate) + D_800A3928 = 1; the flag meaning is unknown so the wrapper cannot be named honestly beyond its callee |
| 0x8005B98C (func_8005B98C) | snd_VabFakeOpen(a0, 8); snd_VabFakeOpen(a0, 4) - hard-coded VAB ids (README SsVabClose one-liner precedent) |

## INFERRED-alias functions examined whose alias was left alone

| addr | notes |
|---|---|
| 0x80016888 | gpu_InitDisplay: SetDispMask(0); ResetGraph(1); ClearImage(&{0,0,640,480}, 0,0,0); DrawSync(0). Alias is consistent with the body; no rename proposed (could be CORROBORATED as-is) |
| 0x80017748 | math_Distance3D: Square12 of ((a-b)>>2) then SquareRoot12(sum)<<2 = |a-b|. Alias consistent; corroboration-only |
| 0x800177C8 | math_Distance3D_16: same with >>4/<<4. Alias consistent; corroboration-only |
| 0x8001A538 | md_game_helper: identity, RotMatrixX/Y/Z(-angles) then pos - m.col2*dist>>12 (eye-point-behind-target math); name would need camera semantics |
| 0x8001F888 | cpu_helper: SquareRoot0 2-D distance between two pairs of unnamed globals (D_80102408/10 vs D_80101FBC/C4) with overflow halving; name needs the globals identity |
| 0x80044DE4 | hirahira_flap_wing_calc: LoadAverage12 of two SVECTORs with Y/Z negated; the alias is game-semantic but a restating name ("LerpNegYZ") is not useful - leave for a math vein |
| 0x800485EC | gpu_SetTexAttr: parses a TIM header, GetClut/GetTPage into a texture-attribute struct; alias consistent |
| 0x80069898 | gpu_draw_layered_box: 3x SetTile/SetSemiTrans/AddPrim; alias consistent |
| 0x8006F038 | gpu_draw_fullscreen_overlay: TILE 640x240 grey D_800A3550, semi-trans, + DR_MODE tpage 0x40 (abr 2 subtractive); alias consistent |
| 0x80072BC4 | gpu_build_quad: SetPolyG4/SetSemiTrans/AddPrim with fixed colours; alias consistent |
| 0x80072CD4 | gpu_build_gradient_quad: same family; alias consistent |
| 0x80072F30 | hud_helper: SetTile/SetSemiTrans/AddPrim with fixed colours; "hud" is game inference but not contradicted; no restating improvement worth a row |
| 0x80072FCC | hud_helper: same as 0x80072F30 |
| 0x8003A41C | set_flag_D_800A3730: sets D_800A3730 = 1; no library call - out of vein |
| 0x8003A6FC | single_game_helper: 32-bit popcount loop; no library call - out of vein (a math_Popcount32 row would be sound but is not api-restatement) |

## Evidence notes for the verifier

- `libcomb_psyq46.h` / `libcd_psyq.h` / `libgpu_psyq.h` in this directory were fetched 2026-09-24 from
  `raw.githubusercontent.com/krystalgamer/memories-decomp/master/src/psyq/` (header banner "$PSLibId: Run-time
  Library Release 4.6$"). BB2 links PsyQ 3.5 libraries, so every comb_ row ALSO cites the in-binary
  `_comb_control` switch (jump table words read from the EXE) which agrees with the 4.6 macro map for every
  sub-command used: (0,0) status, (1,1) set control, (2,0) reset, (3,0) set RTS, (3,1) CTS, (4,0) wait callback.
- The CD rows share one chain: SpecialCam (0x8008EC34) is an 8-byte {CdlLOC, byte length} table (apiscan
  cdrom_LoadExec row), D_80101E62 is the state variable of the func_80036940 state machine. If the verifier
  rejects that chain, cdrom_StartRead / cdrom_StartReadAt / cdrom_IsIdle / snd_LoadCommonVab fall together;
  cdrom_FlushInit, cdrom_ReadWait, cdrom_ReadyCallback, cdrom_Pause and cdrom_StartAudio stand on their own
  bodies (cdrom_StartAudio uses one hop for the Setmode byte).
- Owner-held items NOT used as evidence: func_800858D0 (probable SsUtAllKeyOff), func_8008B488 (probable
  SpuSetVoiceAttr) - memory/closer/libsnd-hunt-report.md.
