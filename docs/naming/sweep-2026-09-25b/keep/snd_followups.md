# snd_followups — examined, no change (KEEP)

- 0x80046AA0 snd_StopAll — player_Destroy x2, heap frees of slots 7/6/8, func_80049E1C. Its reach includes SsVabClose [VERIFIED] 3x (func_80045A50 -> func_8005B644; func_80046020 -> func_8005B6AC) and func_800858D0: sound teardown is really there, so "snd_…All" is not contradicted (same call as the first sweep).
- 0x80049E1C snd_helper — fills 58 halfwords ending at D_80099CC2 and D_800A324C with -1; callers = heap init func_800451D0 and snd_StopAll. The data roles are unknown; not contradicted.
- 0x80044010 prim_buffer_open_slot / 0x80044100 prim_buffer_relocate_slot — register/relocate an offset table in D_80103608[slot]. Slot 9's table (set by func_800467B8 from the slot-8 MAR file) is read by the GTE command renderers, so "prim" is plausible. Unsupported, not contradicted (misc vein agreed).
- 0x80045808 const_return_0x45000 / 0x80045814 get_ptr_D_800A9D10 — literal names, true (heap size / arena base).
- 0x800A3368 g_char_data_base_idx — mode_ui lead. Refs: written in func_8003EDC0.s:73, read in func_8003E6D8.s:164 and func_8003EB84.s:85; src/code6cac_c2.c:1977-2091 uses it as `(v1 - D_800A3368) * 0x68` into D_800A6690. "base index" is mechanically right; "char" rests on the unproven game_GetCharData, but nothing contradicts it.
- 0x800A38BC g_file_heap_base / g_rng_state — the CONFIRMed alias RESET stays held (data_wave cannot retire one alias and keep the held g_rng_state). Its two derived byte names are the separate HIGH rows 0x800A38C0/C1.
- 0x800EFB0C g_snd_data_header_FB0C (= D_800EFAE8.unk24, returned by address from 0x8005507C) — role not traced; unlike unk2C..unk40 it is not relocated by the slot-10 callback. Unsupported, not contradicted.
- 0x800EF848 g_snd_ch_data / 0x80099C34 g_snd_se_bank — func_80048F58 (text1b.c:748-765) zeroes a word in a 308-byte record of D_800EF848 and copies 7 halfwords from D_80099C34[a0*7] to record+0x124. The consumer (func_80048FFC, 232 insns) was not traced. Unsupported, not contradicted.
- 0x800A307C g_snd_callback — `AddDrv(&g_snd_callback)` (src/main.c:3838). A device-driver/library object, out of this vein. Not examined in depth.
- 0x800F1B18 records vs held `_svm_orev1` (0x800F1B14) — mode_ui lead. Nothing to add beyond the held Sony-static note (keep/followups.md).
- 0x80016888 gpu_InitDisplay, 0x80020CDC seq_ConditionalReset, 0x800415C4 player_Destroy — mode_ui "callees not relied on" note. Already KEEP in the first sweep; no new evidence.
- 0x80036F40 game_FrameLoop — motion_ai lead (CD-idle wait). mode_ui already dropped a RENAME twice as MEDIUM (the loop also renders, so "FrameLoop" is not contradicted). No new evidence.
- Already applied, so no row: pad_analog_volume_calc / pad_analog_button_test / pad_controller_init, g_game_p2_ctrl_plus_2, disp_SetFramebufferMode, copy 0x8006E440, obj_* libsnd teardown, 0x80047530 (game_SndInit), 0x8001DA2C.
