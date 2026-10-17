# Memory layout

How the game uses PS1 memory, from the EXE header (`asm/header.s`), the linker
script (`bb2.ld`), the entry point and the code in `src/`. Main-RAM addresses
are KSEG0 (`0x80000000`+). The scratchpad (`0x1F800000`) and hardware
registers (`0x1F801000`+) are listed in their own sections.

## The EXE image

`asm/header.s` gives:

- `.text` start 0x80010000 and size 0x93800, so the image covers
  0x80010000..0x800A37FF;
- initial PC 0x800836EC (`__SN_ENTRY_POINT`);
- initial `$sp` / `$fp` base 0x801FFFF0.

The header's `$gp` and `.bss` fields are 0. `bb2.ld` links one output section
`.main` at 0x80010000, in this order:

| Range | Content | Linker symbols |
| --- | --- | --- |
| 0x80010000..0x800164F8 | `.rodata` of every TU | `main_RODATA_START` / `_END` (bb2.ld:89) |
| 0x800164F8..0x8008D120 | `.text`. The last 0xB0 bytes (0x8008D070..) are game data emitted from `src/main/d_7D870.c`: `g_data_start`, the 34-entry handler table `D_8008D090` that `main` calls through `D_800A3834` (src/main/6CF8.c:468), and the start of `g_sqrt_table_u8` | `main_TEXT_START` / `_END` (bb2.ld:91, :317) |
| 0x8008D120..0x800A3800 | `.data`, with each -G8 TU's `.sdata` / `.sbss` in link order | `main_DATA_START` / `_END` (bb2.ld:319, :594) |

The linked objects' `.bss` is empty (`main_BSS_START == main_BSS_END ==
0x800A3800`). Zero-initialised globals above the image are absolute symbols
in `undefined_syms_auto.txt` / `named_syms.txt`, or C tentative definitions.

## Start-up (`__SN_ENTRY_POINT`)

`__SN_ENTRY_POINT` (`asm/funcs/__SN_ENTRY_POINT.s`, LIBSN SNMAIN, kept as
canonical asm in `src/main/psxsdk/libsn/73E98.c`) runs these steps:

1. Zeroes 0x800A3308..0x801078E0 word by word. This covers the image's tail
   from 0x800A3308 and all RAM up to `D_801078E0`.
2. Sets `$sp = (_ramsize - 8) | 0x80000000`. `_ramsize` is 0x800000 in this
   build (the word at 0x800A2690).
3. Computes the heap: `__heapbase = 0x801078E0 + 4` and
   `__heapsize = _ramsize - 8 - _stacksize - 0x001078E0`, with `_stacksize`
   0x8000.
4. Loads `$gp = _gp = 0x800A30CC` (bb2.ld:19), calls `InitHeap`, then calls
   `main`.

`main` (src/main/6CF8.c:423) calls `__main`, then `SetSp(0x801FFF00)` and
`SetMem(2)` (2 MB) before anything else.

## Fixed buffers in main RAM

These addresses are set in code:

| Address | Size | Use | Set / used at |
| --- | --- | --- | --- |
| 0x801D8800 | 0x13400 | primitive buffer 0 (`D_800A3770[0]`) | `func_80016D78` (src/main/6CF8.c:289) |
| 0x801EBC00 | 0x13400 | primitive buffer 1 (`D_800A3770[1]`); ends at 0x801FF000 | src/main/6CF8.c:290 |
| 0x801D8800 | | also load scratch before the frame loop: `eff_Init`'s `func_80060CB8(0x801D8800, ...)` and `snd_LoadCommonVab(0x801D8800)` | src/main/6CF8.c:254, :269 |
| 0x8010DB00 | <= 0xD00 | the common VAB, copied from 0x801D8800 | src/main/6CF8.c:273 |
| 0x8010E800 | 2 x 0x4650 | `POLY_FT4` buffers `D_800A3420` / `D_800A3424` set by `func_80060CB8` | 51268.c, `func_80060CB8` |
| 0x80118800 | | `func_80016A8C`'s image buffer at boot | src/main/6CF8.c:440 |
| 0x801FFF00 | | stack top after `SetSp` | src/main/6CF8.c:435 |

Each frame, `main` checks the cursor against the buffer's end, `tbl[idx] +
0x13400` (src/main/6CF8.c:489). `func_80016D78` also stores 0x13400 in
`D_800A3798` (src/main/6CF8.c:291). See [gpu_pipeline.md](gpu_pipeline.md).

Large objects named elsewhere in these pages:

- `GpuDb g_gpu_db[2]` at 0x800F7438: two 0x4090-byte records holding
  DRAWENV, DISPENV and OT (include/game.h:54-63).
- `CdState D_80101E58`: the CD state block (include/game.h:2084).
  `g_cd_file_table` (0x8008EC34, include/bb2.h:475) holds the disc file
  records. See [file_io.md](file_io.md).
- `D_800F33D8`: the 0x200-byte memory-card image. Its second half is
  `Unk800F34D8Save D_800F34D8` (include/game.h:40-52).
  `func_800174F4` also builds its overlay primitives there
  (src/main/6CF8.c:537).
- `g_scratchpad_save` (0x800F5370): `scratchpad_Save` /
  `scratchpad_Restore` copy 0xF8 words (992 bytes) between it and
  0x1F800000 (src/main/6CF8.c:782, :791). The one caller pair is in
  `func_80023F08` (src/main/9F9C.c:4900, :4903), around `func_80040D48`.
- `CdlREAD D_800A14D0`: libcd `CdRead` state (include/psxsdk/libcd.h:41-57).

## The `$gp` window

With `_gp = 0x800A30CC`, a signed 16-bit offset reaches 0x8009B0CC through
0x800AB0CB. cc1 runs at `-G8` only for the TUs in `GP_FILES`
(Makefile:129). maspsx runs `-G8` for every file except Sony library code
(`PSYQ_LIBRARY_FILES`, Makefile:137). A file uses `$gp` only for a small
symbol it defines (Makefile:132). So an address inside the window does not
prove that code reaches it through `$gp`. Check the listing.

## Scratchpad (`0x1F800000`..`0x1F8003FF`)

The 1 KB scratchpad is shared work space. Different code puts different
records there at different times. `include/game.h` holds the per-file views:

- `Unk1F800000Rec`, the 51268.c view from 0x1F800000. `func_80060E38` caches
  pointers into it in `D_800A3468..`.
- the 17AFC.c collision record at 0x1F8002B8..0x1F8003FF.

The comment above the 17AFC.c record (include/game.h:1180-1196) lists the
other users of those bytes. GTE helpers also stage values there, for example
`gte_ldlvl((s32 *)0x1F800000)` in `func_80018300` (src/main/87A0.c:291).

## VRAM

The two display buffers are 640 x 240 at (0, 0) and (0, 240) (`disp_Init`,
src/main/6CF8.c:164-167). `func_80060CB8` uploads its image to the column
x = 0x380..0x3FF, as rectangles (0x380, 0, 0x80, 0x1DC) and
(0x380, 0x1DC, 0x70, 0x24) (51268.c).

## SPU RAM

libsnd's `_SsVmInit` calls `SpuInitMalloc(0x20, D_800F19D0)`
(src/main/psxsdk/libsnd/vm_init.c:16). `SsVabOpenHeadWithMode` allocates
each VAB's sample area with `SpuMalloc` (src/main/psxsdk/libsnd/vs_vh.c:115). `SpuMalloc` and
`_spu_gcSPU` (src/main/psxsdk/libspu/s_m_m.c, s_m_int.c) manage those
blocks. They allocate SPU RAM, not voices. See [sound.md](sound.md).

## Hardware register pointers

Sony library code reaches the hardware through pointer variables:

- libcd: `libcd_CDRegister0..3` -> 0x1F801800..3, `g_com_delay_reg` ->
  0x1F801020, `D_800A1490` -> 0x1F801C00 (SPU).
- libgpu: `GPU_DATA` / `GPU_STATUS`, `DMA2_*`, `DMA6_*`, `DPCR`.
- libetc: `D_800A2638` -> root counter 1 mode, and `D_800A263C` -> DICR
  0x1F8010F4.

[file_io.md](file_io.md) and [gpu_pipeline.md](gpu_pipeline.md) describe the
code that uses each pointer.
