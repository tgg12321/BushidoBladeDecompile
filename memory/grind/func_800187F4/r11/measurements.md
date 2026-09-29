# func_800187F4 — measurements on the v2 landing chassis c6 (2026-09-28)

lines = differing objdump lines on the real per-file recipe (r11/tools/fast3.sh + cmp.py); sandbox =
`sandbox func_800187F4 --disable all --candidate` (engine 21b9bbebd), insns built/644. Bodies:
variants/<tag>.c (templates; `gen.py` expands the @gte_ markers). The v1 rows (c5a chassis, work
and delta reused) are in measurements_v1.md.

| tag | lines | sandbox | insns |
|---|---|---|---|
| c6 | 0 | 0 | 644 |
| c6_no_depth_ds | 4 | 4 | 644 |
| c6_no_ds | 30 | 30 | 644 |
| c6_no_sq1_ds | 26 | 26 | 644 |
| r11abl_idx_idx_add | 114 | 70 | 644 |
| r11abl_idx_idx_sph | 67 | 67 | 644 |
| r11abl_idx_idx_sub | 70 | 70 | 644 |
| r11abl_temp_lut1 | 33 | 33 | 644 |
| r11abl_temp_lut2 | 31 | 29 | 644 |
| r11abl_temp_lzc_in | 27 | 27 | 644 |
| r11pv_all | 98 | 98 | 644 |
| r11pv_idx | 67 | 67 | 644 |
| r11pv_nbits | 2 | 2 | 644 |
| r11pv_nbits2 | 2 | 2 | 644 |
| r11pv_nforce | 21 | 21 | 644 |
| r11pv_temp | 42 | 40 | 644 |
| st_idx_fscope | 67 | 67 | 644 |
| st_idx_while | 67 | 67 | 644 |
| st_nbits2_fscope | 2 | 2 | 644 |
| st_nbits2_onestmt | 2 | 2 | 644 |
| st_nbits_fscope | 2 | 2 | 644 |
| st_nbits_onestmt | 2 | 2 | 644 |
| st_nforce_fscope | 21 | 21 | 644 |
| st_nforce_inline | 64 | 23 | 646 |
| st_temp_fscope | 42 | 40 | 644 |
| st_temp_inline | 61 | 59 | 644 |
