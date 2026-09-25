# medium_revisit — ideas examined and dropped (2026-09-25, sweep 3)

| addr | name | idea | why dropped |
|---|---|---|---|
| 0x80077A80 | replay_camera_apply_transform_and_flip | RESET (09-25 MEDIUM: "reach not audited to full depth") | Audited to full depth (work/closure.py, work/path.py): the CD-wait `game_FrameLoop` inside func_8006E950 runs the loading-screen frame func_800174F4 -> PutDrawEnv + DrawOTag [VERIFIED]. A buffer flip IS in reach, so the name is not contradicted as a whole ("transform" absent: no cop2/GTE call in reach, but a 2-D affine texpage renderer is). |
| 0x80016C74 | file_ResetDmaFlag | RESET ("Dma" unsupported) | eff_Init (the only guarded routine) loads a disc file and LoadImage's it to VRAM - DMA work; "Dma" is supported, not contradicted. |
| 0x800274BC | calc_NormVector | RESET (omits the sign flip) | "Normal vector" reading survives: input is a velocity and the output is the unit vector opposing it (collision normal). Emulation (work/normv.py) confirms -4096*v/|v| within 38; RESET not bet-worthy. |
| 0x800274BC | calc_NormVector | RENAME math_NegUnitVector* | Table-sqrt approximation (+-38/4096) of a normalisation -> the owner's open question on approximate normalisation (held gte_NormalizeIR precedent). |
| 0x80078824 | disp_init_video_overlay | RESET | "video overlay" is only false if read as FMV/code overlay; VRAM + overlaid image reading survives. |
| 0x80017748 / 0x800177C8 | math_Distance3D / _16 | UPGRADE as-is | The lead's bar (an approximation must not overclaim) - proposed as RENAME math_ApproxDistance3D_Shr2/_Shr4 instead. |
| 0x80017748 / 0x800177C8 | math_Distance3D / _16 | RENAME math_ApproxDistance3D / math_ApproxDistance3D_16 | "_16" reads as 16-bit; the only difference between the twins is the >>2 / >>4 pre-shift, so both carry an explicit _ShrN. |
| 0x8004881C | gnd_helper | RENAME math_RgbToGray (09-25 miner) | Channel order is caller-dependent (func_80042478 vs func_80041688); replaced by channel-neutral math_Grayscale3. |
| 0x80052A88 | gte_mtx_apply_vec | RENAME gte_SetMatrixRotTransIRLV | Sony *LV routines keep 32-bit precision; this truncates each component to s16. |
| 0x80037540 | mode_helper | RENAME sys_ExecMovovlArgs (09-25 miner) | "Movovl" rests on a data table (g_cd_file_table[156]) - the reason cdrom_LoadCommonBbm is held. Kept as MEDIUM with a neutral sys_ExecArgv6 spelling; still restates a CORROBORATED (not VERIFIED) callee. |
| 0x8003A728 | single_game_HashAndStoreData | RESET | Checksum + store is loosely true; "single_game" is ambiguous (single match vs single console). |
| 0x800F665C | g_game_mirror_mode | RESET (side idea) | Write-only copy of the player-count flag (writers game_SetPlayerCount, game_Init; no reader). "mirror" = a copy, which is literally what it is; not contradicted. |
| 0x8005FBC8 | cpu_load_replay_images_and_init | RESET | "load images and init" true; "cpu"/"replay" unsupported, not contradicted. Kept MEDIUM. |
| 0x8005C8A8 | cpu_render_stage_geometry | RESET | 2-D only, but "stage" may be a stage-select screen and "geometry" of tiles is loose. Kept MEDIUM. |
| 0x800644FC | motutil_GetWalkDir_all | RESET | Rotation matrices can encode directions; "walk" neither supported nor contradicted. Kept MEDIUM. |
| 0x80035430 | mode_handler_14_NoOp | UPGRADE | NoOp proven, table-slot part has no admitted class - policy, not evidence. Kept MEDIUM. |
