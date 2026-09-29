# inferred vein - ideas examined and dropped (naming sweep 2026-09-29)

Method: `work/index.py` (call graph from raw asm words), `work/triage.py` (per-target callee tiers, reach
subsystems, cop2/hardware use, stores/loads), `work/newtrust.py` (targets adjacent to every 09-25/25b applied
row), `work/verbs.py` (verb-shape checks), `work/strs.py` (lui/addiu string refs), `work/refscan.py` (raw-EXE
data reference scan). Outputs `work/triage.tsv`, `work/newtrust.txt`.

| addr | name | idea | why dropped |
|---|---|---|---|
| 0x80046F24 / 0x8004700C | camera_InitMatrix / camera_Transform | RENAME to a planar-projection / shadow name | No admitted class: "shadow" is game meaning; a computation name is excluded (reads the light record, writes a global D_800EEDB0 that is not pair-private under the ruling: its inputs 0x800F62F8.. are also read by func_800470B0). RESET only. |
| 0x80062020 | se_helper | RENAME to a particle/billboard-table name | Its meaning comes from the consumer's GTE/GPU calls, not from calls it makes itself (it makes none); no admitted class. RESET only. |
| 0x80060758 | text_render_cursor_reset | RESET ("text" contradicted: the counters feed a TILE, not glyphs) | The TILE is a 2-px underline at x = 0x5B + 26*sel under a func_8006D808 sprite row: a menu cursor. "text" is not concretely false. KEEP. |
| 0x800450BC / 0x80045188 / 0x80045194 / 0x80020CDC | seq_* | RESET ("seq" contradicted by the archive-override mechanism) | New finding: seq_Start arms an override that serves NDATA ids 0x1E..0x24 from a preloaded archive (func_800450F4). "seq(uence)" is unsupported, not contradicted, and whether the archive holds SEQ data was not checked on the disc image. KEEP. |
| 0x80049E1C | snd_helper | RESET | Now SUPPORTED: its tables are read only by efc_rob_type_dispatch, which gates slot-6 VAB free (SsVabClose [VERIFIED]) / VAB load. KEEP. |
| 0x80032C50 | cpu_check_special_move_input | RESET at HIGH | void event/effect dispatcher with no input read, but "input" may mean the kind argument. Filed MEDIUM. |
| 0x80053754 / 0x80053E9C | move_to_target_step | RESET ("move" absent: a segment-vs-cell-plane test) | Both are per-cell callbacks handed to the grid walker func_80052D00 by init_move_to_target - a step of a walk toward a target. Not contradicted. KEEP. |
| 0x80022580 | mario_test_initialize_match | RESET (initialises one player record, not a match) | Loose, not false. KEEP. |
| 0x80047210 / 0x800472B0 / 0x800472C0 / 0x80047570 / 0x80047384 | camera_InitBoneData / GetBoneData / InitRotation / InitBone2 / CalcAngles | RESET with the camera_InitMatrix pair | Different data: g_cam_bone_data (matrix copy with row 1 halved), generic node init, angles of *D_800A3708. None is shown to be non-camera; the 25b draw-list lead (g_cam_bone_data2 appended to D_80102C00) needs its consumer traced. KEEP. |
| 0x80077D10 | camera_ptr_array_apply_offset | RESET ("camera" on a 2-D screen file) | The relocated tables belong to CD file 2:0x32 (LoadImage'd by func_8006E950), but the tables themselves were not traced to non-camera use. KEEP. |
| 0x800464C4 | md_game_FlushDeferred | RESET | One-shot slot-7 registration gated on and clearing g_stage_variant: a deferred step, "Flush" loose. KEEP. |
| 0x80042478 | disp_set_fade_color | UPGRADE (all callees now trusted: math_Grayscale3, gpu_SetDrawEnvBg, SetFarColor) | "fade_color" interprets the GTE far colour; the gray branch is gated by INFERRED file_GetFlag0. No restatement name. KEEP. |
| 0x80077A28 | replay_camera_init_framebuffer | RESET | gpu_SetDrawEnvBg(1,0,0,0) = clear-to-black enable; "init framebuffer" consistent. KEEP. |
| 0x80065264 | motion_ex_init_state_c | RESET (writes the id-0x10 slot D_800F0BC8, not id 0xC) | "_c" may be a sequence letter (a/b/c), not a hex id; init_state_b does write id 0xB but that is not proof of the scheme. KEEP. |
| 0x8002D320 / 8002DE20 / 8002F2D0 / 8002FF20 / 800300B4 / 80031890 | gte_mvmva (six) | re-propose the 09-25 MEDIUM RESETs | No new evidence (8002DE20's C body shows the same inline macros; 80031890's rng_Next upgrade is irrelevant). |
| 0x800414FC | save_vc_ctrl (SUSPECT) | new RESET row | Census action is already RESET; the apply is excluded by the owner-delegated action-overrides.csv row (build input). Nothing to add. |
| 0x8002A458 | calc_loc_mat_fw_helper | in-binary-string rename from "ILLEGAL GUN MOTION : %d\n" | The string names one error path, not the function. |
| 0x80017848 | graph_add_edge | UPGRADE (math_Distance3D now CORROBORATED) | No admitted class for a graph-structure name. |
| 0x800417D0 | efc_effect_dispatch_child_recursive | RENAME to a recursive matrix-composition name | Callees now CORROBORATED, but the body walks a game node tree (game struct layout) - not computation-restatement. |
