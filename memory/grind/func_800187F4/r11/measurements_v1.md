# func_800187F4 — measurements on the landing chassis (2026-09-28)

fast2 = differing objdump lines on the real per-file recipe (tools/fast2.sh + cmp.py);
sandbox = `sandbox func_800187F4 --disable all --candidate <body>` (engine 21b9bbebd), insns built/644.
Bodies: variants/<tag>.c (templates; `gen.py` expands the @gte_ markers).

| tag | fast2 lines | sandbox | insns |
|---|---|---|---|
| c5 | 0 | 0 | 644 |
| fam_delta_chain_dy0 | 117 | 99 | 644 |
| fam_delta_chain_store | 6 | 6 | 644 |
| fam_delta_deadstore_end | 0 | 0 | 644 |
| fam_delta_selfassign_end | 4 | 4 | 644 |
| fam_idx_chain_loop2 | 110 | 71 | 645 |
| fam_idx_chain_sph | 127 | 103 | 643 |
| fam_idx_dead | 67 | 67 | 644 |
| fam_nbits_chain | 9 | 9 | 644 |
| fam_nbits_dead | 2 | 2 | 644 |
| fam_nforce_chain | 94 | 54 | 645 |
| fam_nforce_dead | 21 | 21 | 644 |
| fam_nforce_selfassign | 21 | 21 | 644 |
| fam_temp_chain_lut1 | 42 | 40 | 644 |
| fam_temp_dead | 42 | 40 | 644 |
| fam_temp_selfassign | 42 | 40 | 644 |
| fam_work_chain | 146 | 123 | 644 |
| fam_work_dead | 26 | 26 | 644 |
| lzc5_2 | 24 | 24 | 644 |
| lzc5_4 | 24 | 24 | 644 |
| lzc5_5 | 0 | 0 | 644 |
| lzc5_7 | 24 | 24 | 644 |
| lzc5_8 | 24 | 24 | 644 |
| r11abl_idx_idx_add | 114 | 70 | 644 |
| r11abl_idx_idx_sph | 67 | 67 | 644 |
| r11abl_idx_idx_sub | 70 | 70 | 644 |
| r11abl_temp_lut1 | 9 | 9 | 644 |
| r11abl_temp_lut2 | 31 | 29 | 644 |
| r11abl_temp_lzc_in | 2 | 2 | 644 |
| r11pv_all | 106 | 102 | 644 |
| r11pv_delta | 4 | 4 | 644 |
| r11pv_f | 0 | 0 | 644 |
| r11pv_idx | 67 | 67 | 644 |
| r11pv_nbits | 2 | 2 | 644 |
| r11pv_nbits2 | 2 | 2 | 644 |
| r11pv_nforce | 21 | 21 | 644 |
| r11pv_temp | 42 | 40 | 644 |
| r11pv_work | 26 | 26 | 644 |
| st_delta_fscope | 4 | 4 | 644 |
| st_delta_inline | 4 | 4 | 644 |
| st_idx_fscope | 67 | 67 | 644 |
| st_idx_while | 67 | 67 | 644 |
| st_nbits2_fscope | 2 | 2 | 644 |
| st_nbits2_onestmt | 2 | 2 | 644 |
| st_nbits_fscope | 2 | 2 | 644 |
| st_nbits_onestmt | 2 | 2 | 644 |
| st_nforce_fscope | 21 | 21 | 644 |
| st_nforce_inline | 64 | 23 | 646 |
| st_temp_fscope | 42 | 40 | 644 |
| st_temp_inline | 42 | 40 | 644 |
| st_work_chainassign | 26 | 26 | 644 |
| st_work_fscope | 26 | 26 | 644 |
| tq_fscope | 2 | 2 | 644 |
| tq_inarm | 2 | 2 | 644 |
| tq_initdecl | 2 | 2 | 644 |
| tq_nocopy | 2 | 2 | 644 |
