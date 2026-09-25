# mode_ui vein - candidate ideas examined and dropped (2026-09-25)

| addr | name | idea | why dropped |
|---|---|---|---|
| 0x8003AAB0 | game_exec_display_dma_loop | RESET ("dma" contradicted) | Body has no DMA call (VSync, Reset/GetRCnt(RCntCNT1), comb exchange), but VSync's library internals in reach do touch DMA. Not a clean contradiction, so KEEP. |
| 0x8003AA78 | game_exec_display_sync | RENAME to an api restatement | VSync(2) + comb exchange + VSync(2); a name would have to summarise the link-cable handshake (D_800A3870 state). |
| 0x80036F40 | game_FrameLoop | RENAME cdrom_WaitIdle | Still MEDIUM (as on 2026-09-24): the loop also renders (func_800174F4) and runs the link exchange, so "FrameLoop" is not wrong. |
| 0x80036EA8 | mode_helper | RENAME cdrom_GetEntryIndex | Still MEDIUM (2026-09-24): D_8008F12C as a "base index into g_cd_file_table" is one inference step. |
| 0x80042E90 | game_helper | RENAME math_InitRotFnTable | Slot 0 of the table is func_8004A348 (INFERRED gte_gpf), so "rotation function table" is not proven for every slot. |
| 0x80049F4C | game_helper | RENAME gte_SetColorAndBackColor | Also copies D_800153F0 into D_800F62E0 and calls func_8004A09C (AUTO); no clean restatement. |
| 0x800477BC/C4/CC/D4 | game_Stub1..4 | UPGRADE (empty bodies) | "Stub" is proven, but the "game_" prefix and the numbering are not. |
| 0x80046DE4 | game_GetDummyFlag | UPGRADE / RENAME math_ReturnZero | Returns 0. The name is consistent; a rename adds nothing. |
| 0x8003C958 | mode_handler_24_DispatchToMode25 | UPGRADE | The asm proves "sets D_800A3834 = 0x19", but the name leaves out the display init and global clears. The table-slot word has no admitted class. |
| 0x8003F1D4 | game_GetCharData | RESET | Returns &0x800A6690. func_8003E6D8 also walks that region as 0x68-byte grid records, but "char" (character) data is not disproven. |
| 0x80046EDC | game_StageCleanup | RESET | Calls func_800460E4/func_800421C8 (StageLight tables)/func_8003E0E0. Its callers (func_8001D790, game_StageSetup) do not show a per-frame pattern. |
| 0x8003C560 | mode_handler_21_FrameTimerSfx | RESET "Sfx" | The sound claim rests on 0x8005C650 (a request-queue writer) and its consumer 0x8005C6D0 (snd_FlushKeyOnTable, held PLAUSIBLE). This is unproven, not contradicted. |
| 0x8003CCCC / 0x8003CD10 | mode_handler_32_RebootDispatch / 33_RebootBegin | RESET "Reboot" | These are not a system reset: the real Exec path is handler 15 -> 0x80037540 -> sys_Exec. But "reboot" of the game flow is too vague to contradict. |
| 0x8003A728 | single_game_HashAndStoreData | RESET | The xor checksum is hash-like and the function stores the merged data into a0, so the name is loosely true. Filed as a MEDIUM RENAME instead. |
| 0x8001FAE4 | single_game_FindStatusUpData | RESET | A list search; "StatusUp" is unsupported but not contradicted. StatusUpBuf is itself an unverified data name. |
| 0x80060768 / 0x80073060 / 0x8006D808 / 0x80072F30 / 0x80072FCC | hud_* | RESET / RENAME | GPU-primitive emitters with VERIFIED libgpu calls. "hud" is inference, not contradicted, and matches the 2026-09-24 func_api rejects. |
| 0x80068ECC | mode_helper | computation-restatement | Its bit remap writes a game global (D_8009BC04), so it is not pure. |
| 0x8003984C | mode_helper | computation-restatement | Averages two 3-vectors from a caller struct, then calls AUTO func_80053584/func_80054434. Not a leaf. |

## Follow-ups outside this vein
- `pad_analog_volume_calc` (0x80037AA4) = 15 - (sum of g_memcard_file_list[i].+0x18 >> 13), i.e. a memory-card free-block count. `pad_analog_button_test` (0x80037B00) = a filename compare over g_memcard_file_list. `pad_controller_init` (0x80038170) loops over the D_80106A50 bitmask and the D_8008F204 table, then calls strcpy. It sits only in the memory-card tick closure (called from func_800383A4). The first two are INFERRED "pad_" names on memory-card code, so their pad claim is contradicted and they are RESET candidates for whichever vein owns `pad_`. The third needs its own check.
- `g_game_p2_ctrl_plus_2` (0x800F665A) inherits the false g_game_p2_ctrl base name.
- `g_char_data_base_idx` (0x800A3368) sits next to the angle word 0x800A336C (old g_game_mode). The data vein should re-check it.
- The D_800F1B18 records (stride 0x570, used by 0x8001979C/0x800198D0) start 4 bytes after the held `_svm_orev1` (0x800F1B14). This bears on that held Sony-static naming.
- `gpu_InitDisplay`, `disp_SetFramebufferMode`, `seq_ConditionalReset`, `player_Destroy`, `obj_Init*` are INFERRED callees used throughout the handlers. None of them were relied on here.
