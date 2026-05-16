# File I/O and CD-ROM

BB2 reads files from the disc using PsyQ libcd, a thin wrapper over the
PS1 BIOS CD-ROM filesystem. The BIOS provides POSIX-like `open / read /
close / seek` on ISO9660 names; BB2 wraps these in higher-level helpers.

Almost all file I/O code is in:
- `ings.c` (`file_*`, ~0x80016ABC..0x80017FA0) — high-level loaders
- `system.c` (`cdrom_*`, `0x8008008C..0x800807A8`) — libcd shim and CD
  command dispatch
- `ings2.c` (`bios_*`, `~0x80082xxx`) — BIOS syscall trampolines

## Disc layout

The disc is BIN/CUE Mode 2/2352. Extracted via `tools/extract_iso.py` to
the `disc/` directory:

```
disc/
  SLUS_006.63          ; main EXE, 606 KB
  LOADSE/              ; stage data + sound effect banks
    STAGE00.BIN, STAGE01.BIN, ... STAGE37.BIN
    *.SE                ; SPU sample banks (24 voices each)
  LOADSE1/             ; alternate sound banks
  MOTION/              ; character animation bundles
    Y123.BBM, N123.BBM, K123.BBM, T123.BBM, S123.BBM (5 chars)
    WIN.DAT             ; win-pose animation table
  NDATA/
    NDATA.DAT (~55 MB)  ; large packed archive
    NDATA.INF           ; index into NDATA.DAT
  TIM2D/               ; 2D textures, UI/menu data, sound banks
    *.TIM, *.BNK
  U_PIC/               ; stage backgrounds
    STG00.BIN ... STG29.BIN
  XA_0/, XA_1/         ; XA-ADPCM streaming audio (music, voice, endings)
  STR/                 ; FMV opening, title, movie overlay
    OPENING.STR, TITLE.TIM, MOVOVL.EXE
```

## High-level file API — `file_*` in `ings.c`

| Function | Purpose |
| --- | --- |
| `file_LoadAll(name, dest)` | Open, full-read, close. Returns size or -1/-2 on error. Internally uses `bios_FileRead` in 0x4000-byte chunks. |
| `file_LoadSectors(name, dest, sector, count)` | Open, seek to `sector*0x800`, read `count` sectors of 0x800 bytes, close. |
| `file_LoadOverlay()` | Load `MOVOVL.EXE` overlay into 0x801D8800 (calls `func_80060CB8`). Panics if larger than 0xA000. |
| `file_LoadSoundData()` | Load SE bank to 0x801D8800 scratch, copy to 0x8010DB00 in main RAM, then DMA-upload to SPU via `saFidLoad`. |
| `file_GetFlag0/1/2()` | Test `g_file_flags` bits — disc-config related |
| `file_ResetDmaFlag` | Clear `g_file_dma_flag` to 0 |

Globals:
- `g_file_flags` (`0x80106A73`) — disc-config flags
- `g_file_disc_type` (`0x80106A54`) — disc type byte
- `g_file_disc_size` (`0x80106A50`) — disc data size
- `g_file_vram_timer` (`0x800A3710`) — VRAM transfer cooldown
- `g_file_dma_flag` (`0x800A3716`) — 1 if a DMA is in flight
- `g_file_heap_base` (`0x800A38BC`) — heap pointer (also RNG state — see `rng_*`)
- `g_file_data_buf` (`0x800F6740`) — generic file-data buffer

The high-level `file_*` API hides the "open + read + close" trio: BB2's
file load is always full-file or sector-range — there's no streaming
random-access for game data (XA streaming is separate; see [sound.md](sound.md)).

## CD-ROM operation — `cdrom_*` in `system.c`

Lower-level CD-ROM control:

| Function | Purpose |
| --- | --- |
| `cdrom_CheckReady(a0)` | Probe drive: a0=2 → shutdown; otherwise init + optionally config SPU |
| `cdrom_GetMode` | Read `g_cd_mode` |
| `cdrom_GetReadyFlag` | Read `g_cd_ready_flag` |
| `cdrom_SetCallbackA/B(fn)` | Install async I/O callbacks. A is the per-sector "data ready" callback; B is the per-stream callback used by XA. |
| `cdrom_SetDebugLevel(n)` | Verbose debug output level |
| `cdrom_GetCmdName(id)` / `cdrom_GetResultName(id)` | Translate command/result IDs to strings (for debug) |
| `cdrom_FramesToBcd(frames, out)` | Convert linear frame count to MSF/BCD `(mm:ss:ff)` for CD seek commands |
| `cdrom_BcdToFrames(bcd)` | Inverse |
| `cdrom_SendCmd` / `cdrom_DmaToRam` / `cdrom_DmaChain` | Thin wrappers over PsyQ libcd |

Globals:
- `g_cd_index_reg/param_fifo/req_reg/irq_reg/spu_voice` — pointers to the
  CD-ROM hardware registers (mapped at `0x1F801800..0x1F801803`)
- `g_cd_status_a/b/c` — register shadows updated by IRQ
- `g_cd_mode` — current command mode
- `g_cd_callback_a/b` — installed async callbacks
- `g_cd_init_flag` — set once `cdrom_Initialize` succeeds
- `g_cd_dma_madr/bcr/dest/size/ctrl` — register shadows for DMA
- `g_cd_cmd_table` — command dispatch table (28 entries)
- `g_cd_result_table` — result handler table (7 entries)
- `g_cd_sector_buf` — sector buffer

The CD timing is critical for XA streaming — the BGM has to land in the
SPU's CD-audio input every 1/75 second (one CD sector at 2x speed = 12.6 ms
per sector). The per-sector callback (`g_cd_callback_b`) fires every time
a sector lands, and is what `marionation_camera_Init_80036064` uses to
advance the XA pointer (see [sound.md](sound.md)).

### The `tslTm2LoadImage` helper

`tslTm2LoadImage(cmd, a1, a2, a3)` is a wrapper around the CD command
dispatch. The first arg is a command code:
- 1 = "stop / abort"
- 2 = "issue read at sector a1 of length a2"
- Other indices map to specific CD command IDs

Functions like `func_80080258`, `func_80080390`, `tslPolyF4Init` are all
"submit the command, retry up to 3 times if it fails" wrappers (the inner
do-while loops at `system.c:155-181, 247-281`). The retry pattern is the
PsyQ standard "transient CD error" recovery.

`func_80080390(9, 0)` (often seen) means "stop the SE channel's CD-audio
playback" — i.e., halt XA stream into channel 9.

### Retry pattern (`pad_ClearAppliBuffer` cluster)

A common BB2 idiom is "issue command, retry on transient error, eventually
panic". `pad_ClearAppliBuffer` and several other functions named after the
SDK's pad-input helper actually do CD-retry work — yet another Kengo
naming collision. The actual function bodies are 3-retry loops over
`tslTm2LoadImage(2, ...)`.

## Memory card I/O

The memory card is accessed via libapi (cards are pad-port devices). BB2
wraps it in `LWCard_*` helpers (Kengo name suggests "LightWeight Card"):

- `LWCard_SetAccessData` (`code6cac.c:1329`) — set up an access struct
  (4 fields, mostly zero/-1)
- `_McAccessSection` (asm-only at 0x8003D39C) — perform a memory card
  access section
- `_CardCheckPulled` / `_CardCheckPulled2` (asm-only at 0x80027438,
  0x80037A20) — check whether the card was removed mid-operation

Memory card slots/data:
- `g_memcard_busy` (`0x800FF578`) — set while a memcard op is in progress
- `g_memcard_slot` (`0x80101BCC`) — which slot is active
- `g_memcard_data` (`0x80103600`) — the loaded data block

The save/load is async — `g_memcard_busy` is the polled flag.

## BIOS file I/O — `bios_*` in `ings2.c`

The BIOS file I/O syscalls (`bios_FileRead`, `bios_FileReadRaw`) are
trampolines into PS1 BIOS calls A(34h) / A(35h):

- `bios_FileReadRaw(fd, dest, len, ?)` — single-call raw read; hand-coded
  asm in `ings2.c:601`. The opcode `.word 0x0000414D` is a `syscall`
  variant for the A0 jump table at offset 0x34 / 0x4D / etc.
- `bios_FileRead(fd, dest, len)` — chunked-read wrapper (`ings2.c:575`).
  Calls `bios_FileReadRaw` in pieces of up to 0x8000 bytes. Used by
  `file_LoadAll` and `file_LoadSectors`.

Other BIOS-call trampolines:
- `func_80083698` — file-open BIOS call (returns fd or -1)
- `ang_hosei` (`asm/funcs/ang_hosei.s` — note this is the ROTATION helper,
  not the file helper of the same address range) — wraps BIOS file lseek
- `func_80078A18` — `BIOS_close` for file descriptors
- `func_8008386C` — `BIOS B(39h) InitHeap` (called from boot)

These BIOS calls are kept in hand-coded asm because the trampoline form is
specific to PsyQ's syscall encoding (`.word 0x0000414D` = `syscall <imm>`
with the BIOS table index in the upper bits).

## Error / panic

When file I/O catastrophically fails:

1. `sys_Panic` (`ings.c:360`) — print "OVER FLOW" via debug_printf
2. Infinite loop on `func_800164F8()` — `break` instruction to halt CPU

`file_LoadSoundData` calls `sys_Panic` if the loaded SE bank exceeds
0xD01 bytes. `file_LoadOverlay` calls it if MOVOVL.EXE exceeds 0xA000
bytes. The main-loop OT overflow check also calls it (see [main_loop.md](main_loop.md)).

These are "should never happen" asserts — the build system precalculates
sizes and the disc layout is fixed.

## Where unmatched file/CD asm still hides

- The full `cpu_side_move_dir_4` (which is actually a CD command dispatcher
  — Kengo name collision) at `system.c:386`
- `tslTm2LoadImage` core implementation
- `cdrom_Initialize`, `cdrom_Shutdown`, `cdrom_ConfigSPU`
- `cdrom_SendCmd`, `cdrom_DmaToRam`, `cdrom_DmaChain` cores
- Several `func_8008XXXX` helpers that look like CD-IRQ handlers
