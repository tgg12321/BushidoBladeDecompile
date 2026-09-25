# motion_ai vein — candidate ideas examined and dropped

| addr | idea | why dropped |
|---|---|---|
| 0x800372C0 | RENAME `cdrom_PauseWaitIdle` | Second callee game_FrameLoop is INFERRED (its body is a wait-until-cdrom_IsIdle loop that also pumps func_80036940 / func_8003AA48 / func_800174F4); api-restatement needs VERIFIED/CORROBORATED callees. Filed as RESET only. |
| 0x80016A8C | RENAME to an image-splash restatement (e.g. `gpu_ShowCdImageFadeOut`) | Body mixes VERIFIED LIBGPU calls with the INFERRED game_FrameLoop wait and a pixel-halving fade; a restatement name would carry interpretation ("splash"). Filed as RESET only. |
| 0x8001DA2C | RENAME `snd_*` teardown restatement | obj_InitChars calls AUTO func_800858D0 / func_80086130; can't restate the whole body over VERIFIED calls. RESET only. |
| 0x8003AB44 / 0x8003ACB8 | RENAME `comb_Handshake` / `comb_Connect` | Body mixes CORROBORATED comb_* with AUTO/INFERRED helpers and the handshake protocol states are interpretation. RESET only. |
| 0x800355E8 | RENAME `cdrom_PlayAudioTrack1WithMix` | func_80037110 is AUTO and the D_8008F13C table entry meaning is not established. RESET only. |
| 0x80045A50 | RENAME `snd_*`/table-release restatement | func_800453E0 / func_800456F0 are INFERRED/AUTO table ops on an unnamed table. RESET only. |
| 0x80030A2C cpu_set_move_command_and_dir | RESET | Body = func_80030580 slot + vec3 copy + rng_Next random velocity/spin; reads like a debris/particle spawn but "set ... dir" is not concretely contradicted (it does set a direction/velocity). KEEP. |
| 0x8003339C cpu_check_same_dir_timer | RESET | Compares list entries with base+0x40 ("dir") — "check same dir" fits; only "timer" unsupported. KEEP. |
| 0x80016E60 cpu_exec_match_round_stage_select | RESET | Name was derived from main's old refuted name, but the body is a menu loop and "stage select" may be exactly what the menu is. Unsupported, not contradicted. KEEP. |
| 0x8005FBC8 cpu_load_replay_images_and_init | RESET HIGH | "replay" came from the refuted replay_camera_Init, but the body (CD file -> 2x LoadImage) does not itself contradict "load images". Filed MEDIUM. |
| 0x8006E49C camera_init_matrix_table | RESET | Writes 20 base+constant pointers (buffer carve) — "matrix" unsupported but the pointers could address matrix storage. KEEP. |
| 0x8006E950 replay_setup_display | RESET | CD file load + 3 LoadImage uploads to VRAM; "setup display" is loose, not contradicted. KEEP. |
| 0x80077A80 replay_camera_apply_transform_and_flip | RESET HIGH | No transform/flip in reach at the depth audited (800770B8 -> 8006E950/8006E49C/80076FF8), but reach not audited exhaustively. Filed MEDIUM. |
| 0x8005C8A8 cpu_render_stage_geometry | RESET HIGH | 2-D HUD/menu primitive builder called with a menu cursor index, but "stage" could mean a stage-select screen. Filed MEDIUM. |
| 0x80077AE0 / 0x80077B00 replay_camera_helper | RESET HIGH | Wrappers of display-env setup (func_8006E10C / func_8006E2A8), only caller is the SIO connect routine; the vacuous *_helper prefix is the only claim. Filed MEDIUM. |
| 0x800194C0 / 0x800194F4 replay_camera_helper | RESET | Nibble split / variable reset used by the SIO connect routine; no concrete camera claim beyond the prefix and no VERIFIED contradiction. KEEP. |
| 0x80077940 replay_camera_decode_player_bits | RENAME computation-restatement | Writes a global (D_800A35E8), so not pure computation under the 2026-09-24 ruling. KEEP. |
| 0x8003F388 / 0x8003F3D4 | RENAME `grid_SetFlag4` / `grid_SetFlag8` | Read/write the game global grid D_800A8FB0 — not computation-restatement; current name for 0x8003F388 is mechanically right anyway. KEEP. |
| 0x80041584 player_find_empty_slot | UPGRADE | Behaviour exactly matches, but "player" rests on the data name g_player_ptrs (not an admitted class) and the function touches a game global. KEEP. |
| 0x800200DC cpu_helper | RENAME computation-restatement | 182-insn arg-only geometry with SquareRoot0; formula not derived/emulated this pass. Follow-up candidate for a compute vein. KEEP. |
| 0x8001F888 cpu_helper | RENAME `math_Distance2D...` | Reads four unnamed game globals (already dropped 2026-09-24 rejected/func_api.md). KEEP. |
| 0x8003032C cpu_get_dist | RENAME to a velocity-transform restatement | Writes game-struct fields (a0+0x44..0x4C) — not pure computation. RESET only. |

## Out-of-vein leads noticed (not in targets.csv; for the owning vein)
- `disp_SetFramebufferMode` (0x80016768, INFERRED, src/ings.c:188): writes bytes 0x18..0x1B of both g_gpu_db DRAWENVs = LIBGPU DRAWENV `isbg, r0, g0, b0` — it sets background-clear enable + clear colour, not a framebuffer mode. Contradiction candidate.
- `copy` (0x8006E440, INFERRED): adds the base address to every word of a -1-terminated offset table in place (pointer relocation), no copy. Contradiction candidate.
- `game_FrameLoop` (0x80036F40, INFERRED unattributed): loops until cdrom_IsIdle() != 0 pumping the CD state machine — a CD-idle wait, not a frame loop.
- `obj_InitChars` / `obj_Reset` / `obj_InitTask` / `obj_InitPair` (INFERRED unattributed): all libsnd teardown (see the 0x8001DA2C row).
- `game_SndInit` spelling of 0x80047530 in src/sound.c (LINK-MAP DESYNC) — no sound call in reach.
