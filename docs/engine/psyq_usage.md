# PsyQ Library Usage

BB2 is built against the **PsyQ SDK 3.5** (DTL-S3000), Sony's official PS1
development kit. This was confirmed by CVS `$Id:` tags left in the binary:

- `$Id: sys.c,v 1.129 1996/12/25 03:36:20 noda Exp $` (libgpu) — at
  `0x80015E28`
- `$Id: bios.c,v 1.86 1997/03/28 07:42:42 makoto Exp $` (libcd) — at
  `0x80016274`
- `$Id: intr.c,v 1.76 1997/02/12 12:45:05 makoto Exp $` (libapi) — at
  `0x80016328`

The PsyQ libraries (libgpu, libcd, libapi, libspu, libgte, libpress) are
distributed in pre-compiled form; BB2 links them all into the same EXE.
This document describes how BB2 uses each library — useful for modders who
want to know which calls are PsyQ stock vs BB2-custom.

The MOVOVL.EXE overlay additionally links `libmdec` for FMV playback.

## libgpu — `sys.c v1.129`

Sony's GPU library. BB2 wraps everything via the dispatch table at
`g_gpu_dev_table` (see [gpu_pipeline.md](gpu_pipeline.md)). The wrappers
in `gpu.c` and `display.c` use it via indirect calls.

Identifiable libgpu functions (these are PsyQ originals, not BB2-custom):

| BB2 wrapper / direct | PsyQ original | Notes |
| --- | --- | --- |
| `gpu_SetMode` | `GsInitGraph` / `SetGraphReverse` | Debug string `"SetGraphReverse(%d)...\n"` at 0x80015E90 |
| `gpu_SetDispMask` | `SetDispMask` | Debug string `"SetDispMask(%d)...\n"` at 0x80015F04 |
| `gpu_DrawSync` | `DrawSync` | Debug string `"DrawSync(%d)...\n"` |
| `gpu_DrawOTag` | `DrawOTag` | Debug string `"DrawOTag(%08x)...\n"` |
| `func_8007B9B0` | `PutDrawEnv` | Debug `"PutDrawEnv(%08x)...\n"` |
| `func_8007BC08` | `PutDispEnv` | Debug `"PutDispEnv(%08x)...\n"` |
| `gpu_LoadImage` | `LoadImage` | Image upload to VRAM |
| `gpu_StoreImage` | `StoreImage` | Image readback from VRAM |
| `gpu_ClearOTag` | `ClearOTag` | OT init (forward chain) |
| `func_8007B844` | `ClearOTagR` | OT init (reverse chain) |
| `gpu_CalcTPage`, `gpu_CalcClut` | `GetTPage`, `GetClut` | Texture page / CLUT id calculation |
| `gpu_InitDrawEnv`, `gpu_InitDispEnv` | `SetDefDrawEnv`, `SetDefDispEnv` | Initialize env structs |

BB2 also uses several `Vu0SetLightColMatrix_*` named asm functions — these
are NOT VU0 (PS2) ops; they're libgpu `SetLightMatrix` / `SetColorMatrix`
variants that the Kengo name table mislabelled.

## libcd — `bios.c v1.86`

PsyQ's CD-ROM library. BB2 wraps via `cdrom_*` (in `system.c`) — see
[file_io.md](file_io.md). All CD I/O ultimately goes through these.

Identifiable libcd functions:

| BB2 wrapper | PsyQ original | Notes |
| --- | --- | --- |
| `cdrom_GetMode`, `cdrom_SetCallbackA/B` | `CdGetSysMode`, `CdReadyCallback` | Async callback installation |
| `cdrom_CheckReady` | `CdInit` | Drive initialization. Debug `"CdInit: Init failed\n"` at 0x8001605C |
| `cdrom_SendCmd` | `CdControl` | Issue a CD command |
| `cdrom_DmaToRam`, `cdrom_DmaChain` | `CdRead`, `CdRead2` | DMA-based reads |
| `cdrom_GetCmdName`, `cdrom_GetResultName` | (internal libcd debug helpers) | Used for verbose logging |
| `cdrom_FramesToBcd`, `cdrom_BcdToFrames` | `CdIntToPos`, `CdPosToInt` | MSF/sector conversion |
| `bios_FileRead`, `bios_FileReadRaw` | `read()` BIOS syscall | Posix-like file I/O |
| `tslTm2LoadImage` | (PsyQ wrapper, name from Kengo) | High-level CD command dispatcher |

CD streaming (XA-ADPCM) uses libcd's `CdReadyCallback` (the "B" callback
slot) to fire per-sector, plus the SPU's CD-input voice to render the
audio. See [sound.md](sound.md).

## libapi — `intr.c v1.76`

PsyQ's interrupt and event API. Used for IRQs, timers, pad input, and
critical sections.

Identifiable libapi functions:

| BB2 wrapper | PsyQ original | Notes |
| --- | --- | --- |
| `irq_DisableInterrupts` | `EnterCriticalSection` (BIOS A(2)) | Disables hardware IRQs |
| `EnterCriticalSection`, `ExitCriticalSection` | (BIOS-level) | Save/restore IRQ state |
| `irq_AcknowledgeVblank` | `EnterCriticalSection` + ack | VBlank ack |
| `irq_SetAlarm`, `irq_EnableInterrupts` | `SetAlarm`, `EnableEvent` | Event system |
| `func_80078C9C` (`InitPAD`) | `InitPAD` | Initialize pad input buffers |
| `func_80078D38` (`StartPAD`) | `StartPAD` | Start pad input polling |
| `func_80078A58` | `VSyncCallback` | Install VBlank callback |
| `func_80078BA8` | `EnterCriticalSection` variant | Used for SPU I/O |
| `sys_VSync` | `VSync` | Vblank wait. Debug `"VSync: timeout\n"` at 0x80016318 |

Timer / IRQ globals:
- `g_sys_irq_vtable` (`0x800A2600`) — interrupt vector table
- `g_sys_vblank_count` (`0x800A157A`) — vblank counter (libapi)
- `g_sys_vsync_mode` (`0x800A14CC`) — VSync wait mode

## libspu — Sound Processing Unit

PsyQ's SPU library. BB2 uses it via `spu_*` (in `main.c`) and the channel
allocator in `sound.c`. The CVS Id tag is not visible in the bin (libspu's
source isn't tagged), but the linked functions are unambiguous:

| BB2 wrapper / direct | PsyQ original | Notes |
| --- | --- | --- |
| `spu_Init` (asm-only) | `SpuInit` | Initialize SPU subsystem |
| `spu_TransferData`, `spu_TransferDirect` | `SpuWrite0`, `SpuWriteRaw` | Sample upload |
| `spu_SetVolume` | `SpuSetCommonMasterVolume` | Master volume |
| `spu_WriteReg`, `spu_WriteReg16` | (low-level register access) | Direct SPU register writes |
| `spu_NotifyChannel` (`main.c:1148`) | (BB2-custom) | Per-channel notify (not PsyQ) |
| `func_80078A68` | `SpuMalloc` / channel-alloc | SPU RAM management |

Globals:
- `g_spu_base_addr` (`0x800A2CDC`) — base of SPU register shadow
- `g_spu_dma_ctrl` (`0x800A2CF0`), `g_spu_xfer_addr` (`0x800A2CF4`),
  `g_spu_reverb_mode` (`0x800A2CF8`), etc.

The `coli_HitPauseKatana` voice-allocator (despite the name) is BB2-custom
on top of libspu — see [sound.md](sound.md).

## libgte — Geometry Transformation Engine

PsyQ's GTE wrappers. BB2 doesn't use the inline-asm macros from
`include/inline_n.h` directly — instead, it uses raw `.word` encodings via
the project's own `include/gte.h` macros (`gte_rtps()`, `gte_rtpt()`, etc.).
This matches PsyQ's `LIBGTE.LIB` output bytes but lets the project build
without PsyQ-specific headers.

PsyQ entry points BB2 calls directly:
- `func_8007E094` = `InitGeom` — initialize GTE registers at boot
- `gte_SetScreenOffset(x, y)` = sets screen center for projection
- `gte_SetRotMatrix(mat)`, `gte_SetTransVector(vec)` = matrix/vec load

The `g_gte_sqrt_table` (`0x8009C7A8`) is the PsyQ-standard 64-entry square
root lookup that the GTE square-root op (`SQRT`) uses for the integer
remainder Newton-Raphson step.

The `g_gte_saved_ra` (`0x8009C798`) is a per-thread save area that PsyQ's
`InitGeom` populates with the return-address-to-restore — used to bail
out of GTE callbacks safely.

## libpress / libcomb / libpad

Not linked into BB2's main EXE (no symbol evidence). Pad input goes
through libapi's `InitPAD` / `StartPAD`.

## MDEC overlay — MOVOVL.EXE

The FMV overlay (loaded at runtime to `0x801D8800`) handles `.STR`
playback. It links libmdec — the PS1 Motion Decoder driver:

- `MDEC_in` / `MDEC_out` register access (at `0x1F801820..0x1F801824`)
- Macroblock decode (Y, Cb, Cr planes)
- YUV-to-RGB conversion (handled in the GPU's "macroblock receive" mode)

The overlay loads OPENING.STR (intro FMV), TITLE.TIM (title screen — not
FMV, just a TIM texture), and possibly per-ending movies. The overlay is
loaded only when FMV playback is needed, then thrown away (its memory
region is reused for sound-data scratch otherwise).

## Other PsyQ patterns BB2 uses

### Debug printf

`debug_printf` is BB2's name for PsyQ's `printf` (which goes to SIO if
enabled, or is no-op'd in release builds). The release build has
`debug_printf` enabled but with no SIO destination — the strings are still
present and would be visible in a debugger.

The `g_str_*` strings at `0x80015F00..0x80016400` are mostly debug
format strings PsyQ's libgpu/libcd debug-callback would emit. BB2 ships
with the debug strings still in the binary (helpful for decomp).

### BIOS syscalls

PsyQ provides POSIX-like wrappers (`open`, `read`, `close`, `lseek`) on
top of the BIOS A0/B0/C0 jump tables. BB2 calls these as:

- `func_80083698` = `open` (BIOS A(36h) or wrapper)
- `bios_FileRead` = `read` (chunked over BIOS A(34h))
- `func_80078A18` = `close` (BIOS A(33h))
- `ang_hosei` (the OTHER one at `asm/funcs/ang_hosei.s:1-11`, NOT the
  rotation helper) = `lseek` (BIOS A(35h))

The naming collisions ("ang_hosei" matching both a rotation helper and a
file-seek wrapper) are Kengo-name-table artifacts; BB2 originally had
distinct PsyQ-style names like `FileSeek`.

### Critical sections

`EnterCriticalSection` / `ExitCriticalSection` are the standard PsyQ
disable/enable IRQ pair. Used wherever multi-step register state needs
to be atomic — e.g., updating SPU voice keys, swapping DMA channels,
flipping framebuffers.

## What's NOT a PsyQ library

Most of BB2 is NOT PsyQ. The Marionation engine is entirely custom:
combat, motion playback, character data, replay/cam, fighting AI, menu
state machines, character select. The PsyQ usage is limited to:

- GPU low-level (libgpu)
- CD I/O (libcd)
- IRQ + timer + pad input (libapi)
- SPU low-level (libspu)
- GTE math (libgte)
- FMV playback (libmdec, only in overlay)

The high-level game logic — the parts that make BB2 distinctively "Bushido
Blade" — are all original code by Lightweight.
