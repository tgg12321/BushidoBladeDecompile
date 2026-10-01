# M3 / M4 merge evidence (tree ab8b147ca, 2026-10-01)

## 1. Link order

- M3 .rodata: text1a_c2 text1a_b text1a_b_pre_rodata text1b text1a_b_pre_rodata_b text1b_tu1c text1b_tu1d text1b_tu1e text1b_tu2 text1a_b_mid_rodata text1b_b text1b_b_tu2 text1b_b_tu3 text1a_b_post_rodata system text1a_b_tail_rodata main sound -> NOT contiguous
- M3 .text: text1a_c2 text1a_b sound text1b -> contiguous
- M3 .data: text1a_c2 text1a_b sound text1b -> contiguous
- M3 .bss: text1a_c2 text1a_b sound text1b -> contiguous
- M4 .rodata: text1b_tu2 text1a_b_mid_rodata text1b_b -> NOT contiguous
- M4 .text: text1b_tu2 text1b_b -> contiguous
- M4 .data: text1b_tu2 text1b_b -> contiguous
- M4 .bss: text1b_tu2 text1b_b -> contiguous

## 2. Small-data objects reached gp, per member

### M3

- text1a_c2: D_800A33B0 (static), D_800A33B4 (static)
- text1a_b: D_800A33B0 (static), D_800A33B4 (static)
- text1a_b_pre_rodata: none
- sound: D_800A3248 (sdata), D_800A324A (sdata), D_800A33B0 (static), D_800A33B4 (static), D_800A33BC (static), D_800A33C0 (static), D_800A33C8 (static), D_800A33CA (static), D_800A33D0 (static), D_800A33D4 (static), D_800A33D8 (static)
- text1b: D_800A324C (sdata), D_800A3250 (sdata), D_800A326C (sdata), D_800A3278 (sdata), D_800A32B4 (sdata), D_800A32B6 (sdata), D_800A33D0 (static), D_800A33E0 (static), D_800A33E4 (static), D_800A33E8 (static), D_800A33EA (static), D_800A33EC (static), D_800A33F0 (static), D_800A33F4 (static), D_800A33F8 (static), D_800A3400 (static), g_vab_sticky_sbaddr (static), D_800A3408 (static), D_800A340C (static), D_800A3418 (static), g_gpu_ot256_ptr (common)
- reached gp from two or more members: D_800A33B0 0x800A33B0 (static) by sound+text1a_b+text1a_c2, D_800A33B4 0x800A33B4 (static) by sound+text1a_b+text1a_c2, D_800A33D0 0x800A33D0 (static) by sound+text1b

### M4

- text1b_tu2: D_800A35D8 (static), D_800A35DC (static), D_800A35E0 (static), D_800A35E4 (static), D_800A35E8 (static), D_800A36A0 (common)
- text1b_b: D_800A3304 (sdata), D_800A35E4 (static), D_800A35F0 (static), D_800A35F4 (static), D_800A35F8 (static), D_800A35FC (static), D_800A3600 (static), D_800A3608 (static), D_800A360C (static), D_800A3610 (static), D_800A3614 (static)
- reached gp from two or more members: D_800A35E4 0x800A35E4 (static) by text1b_b+text1b_tu2

### text1a_c tail

- text1a_c_tu2: D_800A3240 (sdata), D_800A3244 (sdata), D_800A3398 (static), D_800A33A0 (static), D_800A33A4 (static), D_800A33A8 (static), D_800A33AC (static)
- reached gp from two or more members: none

text1a_c tail and M3 share 0 non-COMMON gp object(s): none

## 3. mergecheck2 on the merged groups

```
M3: text1a_c2 + text1a_b + text1a_b_pre_rodata + sound + text1b: contradictions none; COMMON-offset signature none
M4: text1b_tu2 + text1b_b: contradictions none; COMMON-offset signature none
```

## 4. Jump-table phases (rodata-object-alignment condition 1)

```
text1a_c2: rodata 0x800152b4..0x800153b4; tables 0x800152b4 (phase 4)
text1a_b: rodata 0x800153b4..0x800153f0; tables 0x800153b4 (phase 4)
text1a_b_pre_rodata: rodata 0x800153f0..0x8001585c; tables 0x8001541c (phase 4), 0x8001545c (phase 4)
sound: no .rodata
text1b: rodata 0x8001585c..0x800158b4; tables 0x8001585c (phase 4), 0x80015884 (phase 4), 0x8001589c (phase 4)
text1b_tu2: rodata 0x80015a0c..0x80015a3c; tables 0x80015a0c (phase 4), 0x80015a24 (phase 4)
text1b_b: rodata 0x80015a3c..0x80015a68; tables 0x80015a3c (phase 4)
```

## 5. PSYLINK probe (Sony PSYLINK 2.37, per-file .lcomm blocks in link order, then .comm)

```
Linking completed.
0 error(s) in 0.6 seconds

    Start     Stop   Length      Obj Group            Section name
 80010000 8001004F 00000050 80010000 TEXT             .TEXT
 80010050 80010063 00000014 80010050 SBSS             .SBSS
 80010064 8001007F 0000001C 80010064 BSS              .BSS


  Address  Names alphabetically
 80010060  A_COM
 80010000  FA
 8001002C  FB

  Address  Names in address order
 80010000  FA
 8001002C  FB
 80010060  A_COM

  a_s1   at 80010050
  a_big  at 80010064
  a_s2   at 80010054
  a_com  at 80010060
  b_s1   at 80010058
  b_big  at 80010070
  b_s2   at 8001005c
```

