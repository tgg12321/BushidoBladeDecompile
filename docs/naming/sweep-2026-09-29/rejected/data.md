# data vein (sweep 2026-09-29): ideas I examined and dropped

Tools I used (all under tmp/naming_sweep3/data/):
- inventory.py: builds inventory.csv (every registry data name, src/inc/asm ref counts, manifest backing).
- vscan.py: raw-EXE reference scan, copied from snd_followups/verify.
- look.py, refs2.py, basescan.py, inside.py.
- reach.py: jal closure plus census tier.
- fptab.py: resolves function-pointer tables.
- tabscan.py: tests every initialized data object against closed-form formulas.
- scan.py/report.py: the 09-24 API-argument scan, re-run against today's census.

## API-argument rescan (route b)
- Re-running the 09-24 scan gives 1665 VERIFIED-call sites (was 1650). Almost all of the difference comes from renamed callees. The only new data argument is D_800900EC as a0 of eff_Init. eff_Init is not a Sony API, so that gives no role. No new api-restatement rows beyond the two buffer members in candidates.csv.
- D_80101E70 / D_80101E90 (CD streaming words near g_cd_loc): these were MEDIUM on 09-24. Nothing new, and I could not find a second site. Dropped.
- D_800A36AC (g_frame_parity): some readers still don't mask the value with &1, so the 09-24 MEDIUM reason still holds. Not re-proposed.
- 0x800A3220 LoadImage RECT: I proposed the RESET only. A positive name such as `g_gpu_load_rect_16x36` just restates one literal at a single site, so I left it unnamed, as 09-24 did.

## Tables (route d / computation-restatement)
- tabscan.py tested 1276 data objects against sin/cos/sqrt/atan/recip/square/pow2 families × floor/round/trunc. It found only 0x800973FC (g_sin_table, already applied) and 0x80015620 (row). 0x8008D118 and 0x800154A0 were split by dlabels, so I found them by hand. All three are rows in candidates.csv. No other exact-formula table exists.
- g_sin_lut_q3 / g_cos_lut_q2 / g_cos_lut_q4 (0x8009AF94 / 0x8009B794 / 0x8009A794; src/display.c:1173-1203): not tables. They are rsin_tbl minus 0x1000 / 0x800 / 0x1800, because GCC folds `rsin_tbl[a-0x800]`-style indexing in Sony's sin_1/rcos. That is a lib-vein / C-spelling matter; I passed it to miner-lib and did not propose it.

## g_* names audited, no concrete contradiction (kept)
- g_cpu_dist_table_12x4_plus_12 (0x8009B3B0): record 2 of D_8009B398[4] (include/game.h). Its base alias was removed by the manual lane (8b60169b0), and the note says "retire with func_8005E54C". The manual lane owns it, and "cpu_dist" is unsupported rather than proven false.
- g_stage_light_pos/_dir (+_1/_2) (0x800A93B0..C4): written as zeros or by a dead setter, and read only to feed sys_StubEmpty3. The case is circular, not contradictory. 25 held.csv already put this to the owner as a policy question.
- g_cam_interp (0x800EEDF8): the only store is the constant 4 (func_80047570), at node+8 of the 0x800EEDF0 transform node, which 25b calls a rotation slot. The renderer's use of node+8 is not traced, so "timer" is not yet provably false. Good lead.
- g_hira_packet_cursor (0x800A3820 co-alias): unsupported, not proven false (see the 0x800A3820 row note).
- g_str_sleep_menu_text (0x800100A4 = "%04x %04x %04x\n", passed to the printf-style func_8003D52C): "sleep menu" is unsupported, not contradicted.
- g_snd_stream_* / g_snd_stream_slot_* (0x800A33A0-AC, 0x800A9CFC-0x800A9D10, 0x800EED10-1C): the 25b snd_followups ruling stands. The arena holds VAB files too, so "sound" is not contradicted. Nothing new.
- g_cd_spu_voice (0x800A1490 = 0x1F801C00, used by the libcd CD-volume setup), g_gte_saved_ra (InitGeom), g_gpu_* / g_cd_* / g_sys_* / g_spu_* register-pointer names: Sony statics, so they belong to the lib vein. The names are roughly right, not contradicted.
- g_module_func_tbl (0x8008D090, main.c inline asm, 34 code pointers indexed by the mode): consistent with its name.
- g_data_start (0x8008D070): equals SNMAIN's __data word (25b reloc readback). Correct.
- g_anim_func_table (0x800F66A0): a table of rotation-matrix builder pointers (25b). "anim" is unsupported, not contradicted.
- g_pad_rec_*, g_voice_*, g_se_slot_*, g_seq_*, g_memcard_op_*, g_file_* game names, and the remaining ~270 `_plus_N` registry aliases: unsupported only. The "base accessed at scalar width, so +N is outside it" test is a heuristic, not proof: GCC -G0 splits struct members into separate splat D_ symbols. I only proposed a RESET where the base object's size or identity is itself proven (Sony symbol, CONFIRMed API object, matched-C object model, string, emulated state).

## Strings
- All but one of the ~100 string D_ symbols used in src already have a registry g_str_* alias; C just still spells them D_. Making those aliases canonical is a bulk mechanical edit, not a naming claim, so I left it to the owner. The exception is D_80010478 "ILLEGAL GUN MOTION : %d\n", which has no alias. Not proposed (low value).
- D_8008EB38 "34567833" and D_8008EBF4 "|}}~|}" only look like ASCII. They are byte tables, so no string name.

## Pair-private state (route e)
- Of the CORROBORATED computation-restatement functions, only rng (done in 25b), scratchpad_Save/Restore (row) and math_SquareRoot0 (its table, row) touch data. The rest reference no data (reach.py over vscan).
