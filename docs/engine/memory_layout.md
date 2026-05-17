# Memory Layout

This document describes how Bushido Blade 2 actually uses each region of
PlayStation 1 memory. Constants here are confirmed against the binary, the
PS-EXE header (`asm/header.s`), `bb2.ld`, and code references in `src/`.

PlayStation 1 has four address spaces; BB2 uses all four. All MIPS addresses
in this doc are KSEG0 virtual addresses (`0x8000_0000`+, cached).

## Main RAM (2 MB) — `0x8000_0000` to `0x801F_FFFF`

```
+----------------------------------------------+ 0x8000_0000  (KSEG0 base)
| BIOS A0/B0/C0 jump tables, kernel state      |
| (~64 KB, BIOS-owned; do NOT clobber)         |
+----------------------------------------------+ 0x8001_0000  <- EXE load address
| .rodata: strings, jump tables, LUTs          |     (PS-EXE header,
|   Build date string "Fri Aug 7 22:26:32      |      .word 0x80010000)
|     1998" at 0x8001_004C                     |
|   Sine table g_sin_table @ 0x8009_73FC       |
|   Sine/cos LUT pairs @ 0x8009_AF94..BF94     |
|   GTE sqrt table @ 0x8009_C7A8               |
+----------------------------------------------+ 0x8001_6CAC  <- splat: .text
| .text: 1,410 functions, ends ~0x8008_D070    |
|   Entry point @ 0x8008_36EC (in ang_hosei.s) |
+----------------------------------------------+ 0x8008_D070  <- g_data_start
| .data: initialized globals                   |
|   g_module_func_tbl @ 0x8008_D090 (mode      |
|     dispatch table; entry per game-mode      |
|     value of D_800A_3834)                    |
|   g_module_type_tbl @ 0x8008_D118            |
|   PsyQ libcd/libapi/libgpu data structs      |
+----------------------------------------------+ 0x8009_4000  <- splat ends
| .bss / engine globals zeroed at boot         |
|   gp window center: g_disp_gp_base 0x800A30CC|
|   GP-relative globals (+/- 32KB from gp):    |
|     g_disp_enable 0x800A3768 (gp + 0x69C)    |
|     g_disp_fade   0x800A36A8 (gp + 0x5DC)    |
|     g_game_timer  0x800A3790 (gp + 0x6C4)    |
|     SpecialCam    0x8008EC34 (gp - 0x4498)   |
|     g_cd_*        0x800A11xx..0x800A14xx     |
|     g_spu_*       0x800A2Cxx..0x800A2Dxx     |
|     g_snd_*       0x800A33xx                 |
|   g_char_data     0x800A6690 (~14 KB region) |
|   g_player_ptrs   0x800A9A10                 |
+----------------------------------------------+
| Heap / scratch areas (not strictly .bss)     |
|   D_800F5370 — scratchpad save buffer        |
|     (0xF8 * 4 = 0x3E0 bytes; see             |
|      scratchpad_Save/Restore in ings.c:941)  |
|   g_file_data_buf  0x800F6740                |
|   g_disp_fb_base   0x800F7438                |
|     two adjacent 0x4090 buffers (draw+disp   |
|     env + OT)                                |
|   videoDec         0x800F0D80 (MDEC dec)     |
|   MotDataBaseAddress 0x80104F38              |
|   g_memcard_data    0x80103600               |
+----------------------------------------------+ 0x801D_8800  <- overlay region
| D_800A3770 = 0x801D8800 (overlay scratch     |
| base, set in sys_GameInit). Used as:         |
|   - MOVOVL.EXE FMV overlay load target       |
|     (file_LoadOverlay; ings.c:370)           |
|   - sound-data scratch during                |
|     file_LoadSoundData (ings.c:388)          |
| D_800A3774 = 0x801EBC00 (second scratch)     |
| D_800A3798 = 0x13400 (size of region)        |
+----------------------------------------------+ 0x801F_F000
| Stack (16 KB), grows down from 0x801F_FFF0   |
|   PS-EXE header: initial $sp/$fp = 0x801F_FFF0|
+----------------------------------------------+ 0x801F_FFFF
```

### Notes on layout

- The PS-EXE header at `asm/header.s` declares:
  - `.text` start = `0x8001_0000`
  - `.text` size  = `0x0009_3800` (604 KB)
  - Initial PC   = `0x8008_36EC`
  - Initial SP   = `0x801F_FFF0`
- The first 64 KB (`0x8000_0000`-`0x8000_FFFF`) is BIOS scratch and the kernel
  A0/B0/C0 jump tables. Code never touches it except via BIOS syscalls.
- `_gp` is `0x800A30CC` (see `splat.yaml:20`). Every global within ±32 KB of
  this address can be loaded with a single GP-relative instruction (`lw $X,
  -1234($gp)`). This is why so many globals cluster in `0x80082xxx` to
  `0x800B2xxx`.
- The 0x801D8800-0x801F_F000 region is shared between (a) the MOVOVL.EXE FMV
  overlay code/data and (b) transient sound-data loads during `game_Init`.
  These do not coexist — the overlay is loaded for opening/ending movies, the
  sound load happens at title/character-select boot.

## Scratchpad SRAM (1 KB) — `0x1F80_0000` to `0x1F80_03FF`

PS1 has a 1 KB on-die SRAM mapped at `0x1F80_0000`. Access is single-cycle
(zero wait states), but it lives outside KSEG0/KSEG1 — you reach it via
KUSEG/KSEG1 physical addresses. BB2 uses it as a per-frame **GTE scratch +
collision detection workspace**.

Observed usage (from `code6cac.c`, `code6cac_b.c`):

| Offset | Use |
| --- | --- |
| `0x00..0x1F` | Vertex-pair workspace for collision proximity tests. `code6cac_b.c:953-967` stores delta vectors and bone positions for "is point A within radius of point B" checks. |
| `0x24..0x2F` | `marionation_camera_Exec` distance scratch. `code6cac.c:264-272` stores `(dx, dy, dz)` = mat-pos minus camera target, then `0x317` writes them back rescaled. Inputs to a `mtc2/swc2` GTE square-root call. |
| `0x4C..0x5F` | Source vectors for the proximity loop (`code6cac_b.c:935`). |
| `0xBC..` | Output collision-result array base (`code6cac.c:346`, `out asm("s2") = (s32 *)0x1F8000BC`). |
| `0x2B8..` | `code6cac_b.c:921` — `s32 *t0 = (s32 *)0x1F8002B8;` — uncertain, possibly per-fighter result struct. |
| `0x360..0x37F` | Final result registers for that block: dst position, sum-of-displacements, accumulator. |

The full 1 KB is saved/restored once per major mode transition by
`scratchpad_Save` / `scratchpad_Restore` (`ings.c:941, 949`), copying `0xF8`
words (0x3E0 = 992 bytes; the top 8 words are skipped) to `g_scratchpad_save` (D_800F5370) in main
RAM. The two functions do nothing else; the only call sites use them as
"checkpoint" markers around mode transitions.

The GTE `mtc2/swc2` instructions encoded as `.word 0x488CF000 / 0xE99F0000`
(see e.g. `code6cac.c:291-297`) use the scratchpad as the swc2 destination —
this is the standard PSX idiom for `Sqrt(distance)` GTE results, because the
scratchpad is the only RAM the GTE can write to without a CPU memory cycle.

## Video RAM (1 MB) — managed via libgpu

VRAM is a 1024x512 16bpp framebuffer space accessed only by GPU and DMA
commands; it does NOT appear in the CPU address space. BB2 uses the standard
PS1 double-buffered layout:

```
   0,0  +------------+------------+ 1023,0
        | Frame  A   | Frame  B   |
        | 640x240    | 640x240    |  (32bpp scanline, treated as
        | drawn for  | displayed  |   2x16bpp; gpu_InitDrawEnv mode
        | next vsync | this vsync |   bit a4 < 0x121 sets it
        |            |            |   to "WIDE" - see gpu.c:320)
   0,240+------------+------------+
        | Frame  B   | Frame  A   |
        | drawn next | displayed  |
        |            |            |
   0,479+------------+------------+
   0,480| Texture region (CLUTs, TIMs, motion sprites)
1023,511+-------------------------+
```

BB2 calls `gpu_InitDrawEnv` four times (`ings.c:286-288`) — twice for the two
draw buffers, then twice for the two display buffers:

- `(x=0,    y=0,   w=0x280=640, h=0xF0=240)`  — drawenv A
- `(x=0,    y=0xF0=240, w=640, h=240)`        — drawenv B
- `(x=0,    y=0xF0, w=640, h=240)`            — dispenv (paired with drawenv A)
- `(x=0,    y=0,   w=640, h=240)`             — dispenv (paired with drawenv B)

Per-buffer overhead is `0x4090` bytes (`g_disp_fb_base`-relative). This holds
the libgpu DRAWENV (0x5C bytes) + DISPENV (0x14 bytes) + the OT (Ordering
Table) chain + a sprite primitive pool. The frame-flip pointer
`D_800A36AC & 1` toggles which slot the renderer fills (`ings.c:605`,
`code6cac_b2.c` similar).

The right half of VRAM (and the lower half below the buffers) holds packed
textures, palettes (CLUTs), and motion-sprite (zanzou) tile pages. BB2 loads
them via `gpu_LoadTexture`, `gpu_LoadClut256`, `gpu_LoadClut16` (`gpu.c:267,
287, 296`).

## SPU RAM (512 KB) — managed via libspu

SPU RAM is reached only via DMA. BB2's layout:

- Voice 0..23 used as standard sample voices.
- Sound effect bank loaded by `snd_LoadSe` (`sound.c:187`) — uses channel
  index 9 (`SND_CHANNEL_SE`) — sample data DMA'd from the .SE files in
  `disc/LOADSE/`.
- BGM channel uses channel 8 (`SND_CHANNEL_BGM`); BGM is streamed from XA
  sectors in `disc/XA_0/`, `disc/XA_1/` via CD-XA hardware — does NOT
  consume SPU RAM beyond the small XA reception buffers.
- UI/selection sounds on channel `0xA` (`SND_CHANNEL_UI`) — see
  `snd_LoadSelection`, `sound.c:199`.

Globals tracking SPU state:

- `g_spu_base_addr` (`0x800A2CDC`) — base of the libspu register shadow
- `g_spu_init_flag` (`0x800A2D14`) — set once `spu_Init` succeeds
- `g_spu_voice_key_a/b/c` (`0x800A2D38/3C/40`) — voice-key bitmaps used by
  `coli_HitPauseKatana`/`exec_game` for the SPU voice-allocator
- `g_spu_xfer_addr`, `g_spu_addr_shift` — DMA destination tracking

The voice-key allocator in `exec_game` / `coli_HitPauseKatana` /
`coli_HitPauseKatana_2` (`main.c:1798, 1693, 1983`) is interesting: it
implements a fragmenting free-list with marker bits (`0x80000000` = allocated,
`0x40000000` = sentinel, `0x2FFFFFFF` = freed-but-not-collected). This is
PsyQ's standard SPU voice management adapted for the larger BB2 voice budget.

## GP register window (±32 KB around `0x800A_30CC`)

The PsyQ ABI puts almost-all global state within ±32 KB of `$gp` so that a
single instruction `lw $reg, off($gp)` suffices. Splat configures
`gp_value: 0x800A30CC` (see `splat.yaml:20`). Everything in
`0x80082xxx..0x800B2xxx` is GP-rel, which is most of the engine's runtime
state.

Examples (from `named_syms.txt`):

| Address | Symbol | $gp offset |
| --- | --- | --- |
| 0x800A3308 | (BSS clear start, see ang_hosei.s) | +0x023C |
| 0x800A336C | `g_game_mode` | +0x02A0 |
| 0x800A3768 | `g_disp_enable` | +0x069C |
| 0x800A3790 | `g_game_timer` | +0x06C4 |
| 0x800A3834 | game-mode dispatch register | +0x0768 |
| 0x800A2CDC | `g_spu_base_addr` | -0x03F0 |
| 0x8008EC34 | `SpecialCam` | -0x4498 |

If you find a `lw $v0, NNNN(gp)` in disassembly, the global it loads is at
`0x800A30CC + sign_extend(NNNN)`.

## Globals organization (cross-reference by region)

This is a coarse who-owns-what map within main RAM .bss. Detail in
`symbol_addrs.txt`.

| Region | Owner |
| --- | --- |
| `0x8008_DCxx..F2xx` | Kengo-derived static tables: menu data, training data, character setup |
| `0x800A_11xx..14xx` | CD-ROM register shadows (`g_cd_*`) |
| `0x800A_15xx..16xx` | System / IRQ (`g_sys_*`), libapi state |
| `0x800A_2Cxx..2Dxx` | SPU register shadows (`g_spu_*`) |
| `0x800A_30xx..33xx` | Misc engine state (gp anchor at `0x800A30CC`) |
| `0x800A_33xx..38xx` | Display, fade, character, game-mode state |
| `0x800A_6690..` | `g_char_data` — per-character runtime data (~14 KB) |
| `0x800A_8FB0..` | `g_stage_collision` — 32x32 stage collision grid |
| `0x800A_9A10..` | `g_player_ptrs` — player object pointers |
| `0x800E_EDB0..` | `g_cam_matrix` and camera state |
| `0x800E_F7BC..` | `g_snd_config_tbl` and sound playback state |
| `0x800F_5370..` | scratchpad save area |
| `0x800F_6740..` | `g_file_data_buf` — file-I/O buffer |
| `0x800F_7438..` | `g_disp_fb_base` — double drawenv+OT |
| `0x80101_xxx..0x80107_xxx` | Late-allocated tables, motion data, character-anim runtime |
