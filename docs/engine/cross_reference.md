# Cross-reference: files, names and pages

A lookup index. It says where code lives, what each name prefix covers, and
which page describes it. Facts about behaviour live on the subsystem pages;
this page only points to them.

## Game translation units

The game TUs are in `src/main/`, in link order. Each file's top comment
gives its `.text` start and its boundary evidence. Sony and SN code is in
`src/main/psxsdk/` ([psyq_usage.md](psyq_usage.md)).

| File | `.text` | Contents (named functions, or the top comment) |
| --- | --- | --- |
| `6CF8.c` | 0x800164F8 | boot and frame loop: `main`, `sys_Init`, `disp_Init`, `func_80016D78`, `sys_Panic`, `eff_Init`, `snd_InitAndLoadCommonVab`, `pcdrv_*`, `rng_*`, `scratchpad_*`, `math_FovToScreenDist`, `math_Distance3D*`; [main_loop.md](main_loop.md), [gpu_pipeline.md](gpu_pipeline.md) |
| `87A0.c` | 0x80017FA0 | 12 functions: `pad_ResetState`, `pad_ResetStateMarkValid`, `pad_Update`, `func_80018094`, `func_80018300` ([replay.md](replay.md)) |
| `9F9C.c` | 0x8001979C | 93 functions, among them `camera_CalcEye`, `math_RotMatrixZYXAngles`, `pad_ClearStateBits`, `fighter_Update` |
| `175A4.c` | 0x80026DA4 | `func_80026DA4` |
| `17AFC.c` | 0x800272FC | 80 functions, the collision code among them (`box_overlap`, `math_LerpAngle`; the scratchpad record at 0x1F8002B8, include/game.h:1180) |
| `24BF0.c`, `24F08.c`, `25788.c` | 0x800343F0.. | `func_800343F0`, `func_800344B4`, `func_80034708`, and four more |
| `25C38.c` | 0x80035438 | 10 functions: `bits_ExtractMask3F83F8`, `bits_DepositMask3F83F8`, `gpu_AddScreenGradient`, `func_80035618` |
| `26730.c`, `267A8.c`, `26940.c`, `cdrom.c` | 0x80035F30.. | the CD module: `cdrom_*`, `snd_SerialMixOn`, `sys_Exec`; [file_io.md](file_io.md) |
| `memcard.c`, `28514.c` | 0x800375EC.. | memory card: `memcard_*`, `func_80037D14`; [file_io.md](file_io.md) |
| `28708.c` | 0x80037F08 | `memcard_Format`, the save flow (`func_800383A4`, `memcard_PollFileCompletion`), the link-cable session layer `comb_*`, `math_Popcount32` |
| `2B344.c` | 0x8003AB44 | 92 functions: `font_*`, `bitstream_ReadBits`, `gpu_SetDrawMoveArray`, `comb_HandshakeStep`, `comb_Connect`, the CD-audio starters `func_8003B9D0` / `func_8003BE10` / `func_8003C040` |
| `309CC.c`, `31548.c`, `31CFC.c`, `31D3C.c` | 0x800401CC.. | 12 + 4 + 1 + 21 functions (`gpu_AddDrawMove`) |
| `32D04.c` | 0x80042504 | rotation-matrix and colour math (`math_RotMatrix*`, `math_RgbToHsv`) and the primitive texture-offset helpers `gpu_OffsetTex*` |
| `35000.c` | 0x80044800 | 44 functions: the `arena_*` allocator and the `ndata_*` NDATA reader / pack cache ([NDATA.md](../formats/NDATA.md)) |
| `368E4.c` | 0x800460E4 | 103 functions: `rcnt_StartCnt1`, `camera_CalcAngles`, `math_Grayscale*`, `gpu_LoadTimClut`, the view set-up `func_80046BF4` ([replay.md](replay.md)), `math_SetLightPlaneProjection` / `math_ApplyLightPlaneProjection` ([gpu_pipeline.md](gpu_pipeline.md)) |
| `3AB48.c` | 0x8004A348 | 72 functions: GTE and math helpers (`gte_*`, `math_*`) and the renderers `func_8004C994` .. `func_80051ED4` |
| `43500.c` | 0x80052D00 | 16 functions: the stage-collision grid walk `func_80052D00` and its tests and accessors (`func_80053614`), then the motion-sequence loader `func_80054604` ([motion.md](motion.md)) |
| `4510C.c` | 0x8005490C | 20 functions: motion-stream playback (`func_8005490C`, `func_80058580`) ([motion.md](motion.md)) |
| `4BC3C.c` | 0x8005B43C | 28 functions: the sound-bank layer `snd_*` / `func_8005C650` / `func_8005C6D0` ([sound.md](sound.md)) |
| `4D0A8.c` | 0x8005C8A8 | 17 functions: UI draw chunks (`func_8005C8A8` .. `func_80060768`) |
| `51268.c` | 0x80060A68 | 99 functions, among them `func_80060CB8` and `func_80060E04` |
| `596CC.c` | 0x80068ECC | 48 functions: the row menu (`func_80068F70`, `func_8006B578`) and `func_8006E49C` ([menus.md](menus.md)) |
| `5ED34.c` | 0x8006E534 | `func_8006E534` .. `func_80073200` (`func_80070188`, [replay.md](replay.md)) |
| `63D2C.c` | 0x8007352C | 5 functions: the sprite walkers `func_8007352C` / `func_80073728` ([gpu_pipeline.md](gpu_pipeline.md)) |
| `64FD8.c` | 0x800747D8 | `func_800747D8` .. `func_800788B0`, the character-select screen among them |

Data-only files: `d_1068.c`, `d_15EC.c` (the asset path table, see
NDATA.md), `d_5BF0.c` and `d_7D870.c` (`g_data_start`, the mode table
`D_8008D090`; [memory_layout.md](memory_layout.md)).

## Name prefixes in use

Every prefix listed is used by current definitions in `src/main/`.
Unnamed functions keep `func_<addr>`.

| Prefix | Files | Page |
| --- | --- | --- |
| `cdrom_` | cdrom.c, 26730.c, 267A8.c, 26940.c | [file_io.md](file_io.md) |
| `memcard_` | memcard.c, 28708.c | [file_io.md](file_io.md) |
| `comb_` | 28708.c, 2B344.c | the libcomb link-cable session; [psyq_usage.md](psyq_usage.md) |
| `snd_` | 4BC3C.c, 6CF8.c, 267A8.c | [sound.md](sound.md) |
| `gpu_` | 32D04.c, 25C38.c, 2B344.c, 309CC.c, 368E4.c, 6CF8.c | [gpu_pipeline.md](gpu_pipeline.md) |
| `math_` | 32D04.c, 3AB48.c, 6CF8.c, 9F9C.c, 17AFC.c, 368E4.c, 2B344.c, 28708.c | |
| `gte_` | 3AB48.c | [gpu_pipeline.md](gpu_pipeline.md) |
| `arena_`, `ndata_` | 35000.c | [NDATA.md](../formats/NDATA.md) |
| `font_` | 2B344.c | |
| `pad_` | 87A0.c, 9F9C.c | |
| `sys_`, `disp_`, `eff_`, `rng_`, `scratchpad_`, `pcdrv_`, `main` | 6CF8.c, cdrom.c | [main_loop.md](main_loop.md), [memory_layout.md](memory_layout.md) |
| `camera_` | 9F9C.c, 368E4.c | |
| `bits_`, `bitstream_`, `box_`, `rcnt_` | 25C38.c, 2B344.c, 17AFC.c, 368E4.c | |

## Objects that several pages use

| Object | Type / declaration | Page |
| --- | --- | --- |
| `D_8008D090` | the 34 mode handlers `main` calls through `D_800A3834` | [main_loop.md](main_loop.md) |
| `g_gpu_db[2]` | `GpuDb`, include/game.h:54-63 | [gpu_pipeline.md](gpu_pipeline.md) |
| `D_800A3770[2]` | the two 0x13400-byte primitive buffers | [memory_layout.md](memory_layout.md) |
| `g_cd_file_table` | `CdFileEntry[159]`, include/bb2.h:474 | [file_io.md](file_io.md) |
| `D_80101E58` | `CdState`, include/game.h:2084 | [file_io.md](file_io.md), [replay.md](replay.md) |
| `D_800F33D8` / `D_800F34D8` | memory-card image / `Unk800F34D8Save` | [file_io.md](file_io.md) |
| `g_vab_rec_ptr[16]`, `g_vab_vb_sbaddr[16]` | VAB slots, `Unk8005C2A8Pack` | [sound.md](sound.md) |
| `D_800EFB78[24]` | `Unk800EFB78Entry` sound request pool | [sound.md](sound.md) |
| `D_80106A50` | `FileRecord`; `flags` bits read by `func_800167AC` / `func_800167BC` / `func_800167D4` (src/main/6CF8.c:115-119) | |
| `g_rng_state` | `rng_Next`'s state (src/main/6CF8.c:415) | |

## Finding a name

- Current names and addresses: `named_syms.txt` (hand-named),
  `undefined_syms_auto.txt` and `symbol_addrs.txt`. A `/* ... was X */`
  comment there records a retired data name.
- Retired function names: `git log -S<name>`, and
  `docs/naming/MISNOMERS.md` for names that described the wrong code.
- Old file names (`ings.c`, `text1b.c`, `code6cac*.c`, `system.c`, ...):
  `tools/tu_renames.tsv` at tag `pre-restructure-2026-10-03`.
