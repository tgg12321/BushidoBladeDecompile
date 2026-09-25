# misc vein - candidate ideas examined and dropped (2026-09-25)

| addr | current name | idea | why dropped |
|---|---|---|---|
| 0x800164F8 | breakpoint_trap_loop | UPGRADE (structural restatement: 10000x `break 1`) | Accurate, but no admitted class covers a trap loop (not computation; the `break` code 0x400 is not one of the PCdrv 0x101-0x107 codes, and no public reference was fetched for it). KEEP. The stale MISNAMED alias `syscall_wrapper_break_800164F8` (named_syms.txt:29) should still be retired. |
| 0x800164AC | data_as_code_lb_table | RESET | Current name is true (it is data); RENAME to a jump-table name proposed instead. |
| 0x80017748 / 0x800177C8 | math_Distance3D / _16 | UPGRADE at HIGH | Emulation shows a coarse fixed-point approximation (0 below ~2^9 / ~2^11, <=0.8% error only at large magnitudes) - "true for all inputs" not met. Kept as MEDIUM rows. |
| 0x800272FC | calc_DistanceCategory | RENAME to a neutral classifier name | Pure leaf (|x|>0xA00 -> 0, >=0x600 -> 2, else 1), but "Distance" is only an interpretation, and a coined "classify |x| by 0x600/0xA00" name adds nothing. Same call the 2026-09-24 compute vein made on 0x8006E480. |
| 0x8001F860 | calc_AngleDelta | UPGRADE | Formula accurate (wrap to -0x800..0x7FF) but it reads/writes game-struct fields 0x1CA/0x14C - excluded by the ruling's globals/struct clause. |
| 0x8004A1FC | calc_dirvec_from_angles | UPGRADE | Accurate (rsin/rcos VERIFIED) but operates on a game-struct layout (3 angle records, scale @0x5C, output @0x18). |
| 0x80019488 | pack_status_nibble_triplet | UPGRADE | Nibble packing is accurate but its inputs are fixed game globals D_80106A70..72. |
| 0x80045814, 0x8005507C, 0x8005508C, 0x80054410, 0x80037250, 0x8003A41C, 0x80044498 | get_ptr_D_*, set_D_*, write_zero_to_*, set_flag_*, clear_array_* | UPGRADE | Exact mechanical restatements, but each touches a fixed game global (computation class excludes) and each embeds a D_ address that goes stale when the data is named. |
| 0x80044650 | init_fade_panel_wrapper | RESET | Only calls func_80052C10 = `*(u32*)0x1F800400 = 0` (note: one word PAST the 1 KB scratchpad). A single zero store cannot refute "init" of something - unsupported, not contradicted. |
| 0x80044010 / 0x80044100 | prim_buffer_open_slot / _relocate_slot | RESET ('prim') | Generic relocatable-block slot registry (D_80103608 ptr / D_80103658 count). audio_chain_fade_load relocates slot 6 and then looks up a DIFFERENT table (D_800EED10), so no proof slot contents are non-primitive. Unsupported, not contradicted. |
| 0x8003D774 | set_field0x0 | RESET | Also zeroes the other 7 fields of the 24-byte record, but "sets field 0" is still true. |
| 0x8006E8AC, 0x80077D74, 0x80078634, 0x80077098 | get_global / setup_helper | RESET | Return `D_x + a0*44` (element pointer of a global-rooted array) - loose, not wrong; "helper" makes no claim. |
| 0x8001A484 | DispSleepMenuTexAll | RESET | Callee func_8003D52C is AUTO (formatted print); a debug display of 22 hex triplets is consistent with "Disp...". |
| 0x80047550 | saEft03Start_wrapper | RESET | Callee func_80047A90 is AUTO (a sine-table + sound-global effect routine per 2026-09-24); nothing refutes "effect start". |
| 0x80044FA0 | marionation_GetFrameOffset | RENAME (e.g. an overflow-check name) | The in-binary string supports only the "Marionation ... over flow" error path; "GetFrameOffset" (returns D_800963EE[a0]<<11) is not refuted and a replacement would be a guess. |
| 0x800371AC | set_D_80101E64_if_valid | RESET (callee is RESET) | The name is purely mechanical and true regardless of what its callee is. |
| 0x80052930 / 0x80052754 | robtest_helper / decbs0_helper | RESET | Kengo sole-caller "helper" names are not contradicted by the bodies; proposed as precision RENAMEs instead. |
| 0x8006E440 | copy | RENAME (e.g. reloc_SelfOffsetTable) | Behaviour proven (0 mismatches) but a coined name was judged not worth the false-positive risk; RESET only. |
| 0x80042FA0 | calc_rotated_enemy_pos | RENAME (matrix -> Euler angles) | Would need a round-trip proof (RotMatrix* -> this -> same matrix) and relies on math_RotMatrixXYZ (CORROBORATED); RESET only. |
| 0x8003D478 | memcard_access_section_list | RENAME (a string-draw name) | The glyph routine func_8003D39C is AUTO, so no admitted class; RESET only. |
| 0x8005B58C | title_mv_exec2_wrapper | RENAME (wraps SsUtAllKeyOff) | func_800858D0 is only a PROBABLE SsUtAllKeyOff (near-tier-ruling 2026-09-07, not admitted); RESET only. |
| 0x80041430 | vc_RelocLoadedBlock | UPGRADE | Mechanics accurate, but callee save_vc_ctrl is SUSPECT and "vc" is unsupported. |
| 0x800644FC | motutil_GetWalkDir_all | RESET at HIGH | The name is game-semantic; body = batch RotMatrix per mask bit. Kept MEDIUM. |
| 0x800A38BC (data) | g_rng_state alias | RENAME data to g_rng_state | Left to the verifier; the data row asks only for RESET of the contradicted g_file_heap_base. |
