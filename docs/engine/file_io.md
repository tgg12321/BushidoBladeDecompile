# File I/O, CD-ROM and memory card

The game reads the disc through its own CD state machine (`src/main/cdrom.c`,
`src/main/26940.c`, `src/main/267A8.c`) on top of PsyQ libcd
(`src/main/psxsdk/libcd/`). The memory card goes through BIOS file calls in
`src/main/memcard.c`. The PCdrv host-file loaders in `src/main/6CF8.c` are a
third, separate path. Disc contents and file formats are in
[docs/formats/](../formats/README.md).

## The CD file table

`g_cd_file_table` (`CdFileEntry[159]`, include/bb2.h:474) gives each disc
file a `CdlLOC` start position and a byte size (include/game.h:2039-2042).
Code names a file by group and index:
`cdrom_GetFileIndex(group, i)` (src/main/cdrom.c:42) returns
`(&g_cdrom_file_group_bases)[group] + i`, a group base from the s16 table `g_cdrom_file_group_bases`
plus an offset. The groups that appear in calls:

- group 1: `motion_LoadCommonData` reads `cdrom_GetFileIndex(1, 1)` into the buffer
  at `g_motion_common_buffer` (src/main/9F9C.c:3229), and the motion data `(1, 0)` /
  `(1, j + 2)` (9F9C.c:3283, :3337).
- group 2: image and data files. Readers include `eff_LoadImagesAndSetup`,
  `gpu_ShowCdFileImageFadeByHalvingColorsAndRestoreDisplayWidth`, and functions in 4BC3C.c, 4D0A8.c, 51268.c, 596CC.c and 5ED34.c.
- group 5: CD-audio tracks, started through `cdrom_StartAudio`. Callers are
  `cdrom_StartAudioEntry` and three functions in src/main/2B344.c:
  `fight_StartDescriptorPlaybackAndAudio` (src/main/2B344.c:478), `fight_InitScrollingSpriteDisplayAndAudio` (:595) and
  `fight_InitImageAndPlaybackPresentation` (:666).
- group 6: `cdrom_GetFileIndex(6, 0)` is the PS-X EXE that `cdrom_LoadExec`
  reads (`STR\MOVOVL.EXE`, [replay.md](replay.md)).
  `sys_ExecMovieOverlay` (src/main/cdrom.c:237) puts pointers to entries
  `(6, file_index_offset)` and `(6, 2)` in the argument block it hands to that
  EXE through `sys_Exec`.

`cdrom_GetFileSize(i)` (src/main/cdrom.c:55) returns entry `i`'s size.

## The CD state block and its two steppers

All game CD work runs through one object, `CdState g_cdrom_state`
(include/game.h:2084-2089; its `rec` member is `Unk80101E60Rec`,
include/game.h:2054-2077). `rec.io_state` is the state. 0 means idle.
States 2..0xD are the data read. States 0x10..0x1E are CD audio.

Requests:

- `cdrom_StartRead(file, dest)` (src/main/cdrom.c:13) returns 0 if a request
  is pending. Otherwise it copies the file's entry into `rec.pair` and stores
  `dest` in `rec.read_dest_addr`. It sets `rec.read_sector_count` to the sector count
  `(size + 0x7FF) >> 11`, enters state 2 and returns 1.
- `cdrom_StartReadAt(file, dest, sector, count)` (src/main/cdrom.c:31) does
  the same, then moves the start position `sector` sectors on
  (`CdPosToInt` / `CdIntToPos`) and reads `count` sectors.
- `cdrom_StartAudio(file, chan)` (src/main/cdrom.c:80) sets up CD audio.
  `rec.audio_end_pos` is the stop position, 0x96 sectors before the file's end.
  - `chan < 0` selects mode 5 (CdlModeDA | CdlModeRept).
  - Otherwise it selects mode 0xC8 (CdlModeSpeed | CdlModeRT | CdlModeSF),
    after `CdControlB(0xD /* CdlSetfilter */, &file)` with
    `g_cdrom_state.file = 1` and `.chan = chan`.
  - It enters state 0x10.
- `cdrom_StartAudioEntry(n)` (src/main/cdrom.c:110) starts row `n` of
  `g_cdrom_audio_entries`: file `cdrom_GetFileIndex(5, row.audio_file_index_offset)`, channel `row.xa_channel`
  and, when `row.audio_end_offset != -1`, a stop position of the file start plus
  `row.audio_end_offset`.
- `cdrom_Pause` (src/main/cdrom.c:45) clears the ready callback, mutes the CD
  mix, and issues `CdFlush` and `CdControlF(9 /* CdlPause */, 0)`. It then
  sets the stop request `rec.stop_request` and state 0xB.
- `cdrom_RequestStop` (src/main/cdrom.c:143) sets `rec.stop_request = 1`.
  `cdrom_SetAudioRepeat` (:129) sets `rec.audio_repeat`, which sends a finished
  track back to state 0x10.

Steppers:

- `cdrom_Update` (src/main/26940.c:222) handles states below 0x10 and passes
  the rest to `cdrom_UpdateAudio`. The read path:
  1. state 2: `CdControlF(0xE /* CdlSetmode */, {0xA0})` (double speed,
     2340-byte sectors).
  2. state 4: after 3 calls, `CdControl(2 /* CdlSetloc */, &rec.pair.loc)`.
     It loads the destination, sector count and expected position into
     `rec.dest_buffer`, `rec.cd20` and `rec.expected_pos`.
  3. state 5: it installs `cdrom_ReadyCallback` and issues
     `CdControlF(6 /* CdlReadN */)`.
  4. state 6: it waits for `rec.cd20` to reach 0 (done, state 8 -> 0) or go
     negative (error, state 9).

  Each `CdSync` wait gives up after 0x3C calls (state 0xA). The error path
  runs as follows. State 9 goes to 0xA when result byte 0 has bit 0x10 (shell
  open), else to 0xC. State 0xA polls with `CdControlF(1 /* CdlNop */)`.
  State 0xC issues `CdControlF(0x13 /* CdlGetTN */)`. State 0xD retries from
  state 2 after four `VSync(4)`.
- `cdrom_ReadyCallback(intr, result)` (src/main/267A8.c:32) is the
  CdReadyCallback handler during a read. On `intr == 1` (CdlDataReady) it
  reads the 3-word sector header with `CdGetSector(sp, 3)`. If the header's
  position is not `rec.expected_pos`, it sets `rec.cd20 = -2`. Otherwise it
  reads 0x200 words to `rec.dest_buffer`, advances the buffer by 0x800 and
  counts the sector. On any other interrupt it sets `rec.cd20 = -1`. When
  the count reaches 0, or on an error, it removes itself and pauses the drive.
- `cdrom_UpdateAudio` (src/main/26940.c:14) first steps a CD-mix fade.
  `g_cd_atv` moves toward `g_cd_atv_target` over `g_cd_fade_len` calls,
  through `CdMix`. Then it runs the audio states:
  1. 0x10: mutes the mix and issues Setmode `rec.audio_mode`.
  2. 0x11: on sync, Setloc.
  3. 0x12: after 3 calls and a sync, `CdControlF(0x16 /* CdlSeekP */)`.
  4. 0x13: on sync, once the hold flag `rec.audio_hold` is clear, `CdlReadS`
     (0x1B) for XA or `CdlPlay` (3) for CD-DA.
  5. 0x15 / 0x16: playing. It turns the mix on at the second call and stops
     at `rec.audio_end_pos`. For XA, once the position is within 0x96 sectors of
     `rec.audio_end_pos` it loads the remaining sector count into `rec.end_countdown` and
     counts it down by 4 per call; at 0 it mutes the mix and stops (or
     repeats). The volume is not ramped.
  6. 0x1C..0x1E: pause and back to idle.

  0x17..0x1B is its error and retry path.

Waits:

- `cdrom_WaitIdle` (src/main/cdrom.c:60) loops until state 0. Each pass calls
  `comb_ExchangeEmptyPad`, `cdrom_Update`, `snd_FlushSoundQueue` (while
  `g_snd_initialized` is set), `gpu_DrawDisplayModeOnLocalOtAndSync`, and `VSync(2)`. The loop sits
  between `comb_BeginWait` and `comb_EndWait`.
- `cdrom_WaitAudioStarted` (src/main/cdrom.c:150) loops until state 0x16.
- `cdrom_PauseAndWait` (:154) pauses a busy drive, then waits.

Set-up: `cdrom_Init` (src/main/267A8.c:16) runs `CdInit`, `CdSetDebug(0)`, a
muted mix and state 0. `sys_Init` calls it (src/main/6CF8.c:183).

## Synchronous reads: `cdrom_ReadWait` and `cdrom_LoadExec`

`cdrom_ReadWait(nbytes, buf, mode)` (src/main/cdrom.c:165) is the one caller
of libcd `CdRead`. It reads `ceil(nbytes / 0x800)` sectors and spins on
`CdReadSync(1, 0)` with `VSync(0)`. `cdrom_LoadExec` (src/main/cdrom.c:185)
uses it to load the overlay EXE for `sys_Exec` (src/main/cdrom.c:213). Both
are described in [replay.md](replay.md).

## Callers that load data

- `eff_LoadImagesAndSetup(dest, prim_buf)` (51268.c) picks a group-2 file by
  `g_working_record.fight_mode` (0x3C, 0x2F, 0x30, 0x31 or 0). It reads the file to
  `dest` and uploads two VRAM rectangles from it: (0x380, 0, 0x80, 0x1DC),
  then (0x380, 0x1DC, 0x70, 0x24) from `dest + 0x1DC00`. It then splits
  `prim_buf` into two 0x4650-byte `POLY_FT4` buffers
  (`g_eff_first_quad_buffer` / `g_eff_second_quad_buffer`) and returns the end of the second.
- `eff_Init` (src/main/6CF8.c:248) calls
  `eff_LoadImagesAndSetup(0x801D8800, 0x8010E800)` once (flag `g_eff_initialized`, cleared by
  `eff_ClearInitFlag`). It prints the base 0x8010E800 and `eff_LoadImagesAndSetup`'s
  return (the end of the second buffer, 0x801174A0) as `size`. The
  `sys_Panic` check (`0xA000 < size`) is a signed compare against a KSEG0
  address, so it never fires.
- `snd_InitAndLoadCommonVab` (src/main/6CF8.c:265) runs `snd_Init` and
  `snd_LoadCommonVab(0x801D8800)`, and calls `sys_Panic` when the bank's
  main-RAM part (sound table + VabHdr, everything before the body) is 0xD01
  bytes or more. It copies that part to 0x8010DB00, opens it with
  `snd_VabFakeOpen(0xFFF35300, 0)` and sets `g_snd_initialized`. See
  [sound.md](sound.md).
- `gpu_ShowCdFileImageFadeByHalvingColorsAndRestoreDisplayWidth` (src/main/6CF8.c:188) reads group-2 file 0x61 to 0x80118800
  (`main`, src/main/6CF8.c:440) and shows it as a 15-bit image. From frame
  0x79 of 0x96 it halves each RGB channel of the image per frame.

`sys_Panic` (src/main/6CF8.c:239) prints `"OVER FLOW\n"` and then calls
`func_800164F8` forever. That function runs `pollhost()` (`break 1024`)
10000 times (src/main/6CF8.c:39).

## libcd (Sony)

The modules are verbatim PsyQ 4.0 LIBCD; each file's top comment states its
span:

- `event.c`: `CdInit`.
- `sys.c`: the command layer.
  - `CdReset`, `CdFlush`, `CdSetDebug`, `CdComstr` / `CdIntstr` (debug
    names from the string tables `CD_comstr` / `CD_intstr`), `CdSync`,
    `CdReady`.
  - `CdSyncCallback`, `CdReadyCallback`, `CdControl` / `CdControlF` /
    `CdControlB`, `CdMix`.
  - `CdGetSector` / `CdGetSector2`, `CdDataCallback` (DMA channel 3),
    `CdIntToPos`, `CdPosToInt`.
- `bios.c`: the controller driver.
  - `CD_cw` (src/main/psxsdk/libcd/bios.c:323) writes a command and its
    parameters to the CD registers. With `async == 0` it waits for the
    acknowledge.
  - `CD_sync` / `CD_ready` wait on the interrupt status `Intr`.
  - `getintr` reads and acknowledges an interrupt. `cdrom_IrqHandler`
    (:584) dispatches `CD_cbready` / `CD_cbsync` from IRQ 2.
  - `CD_init`, `CD_initvol`, `CD_flush`, `CD_getsector*`.
  - The four register pointers `libcd_CDRegister0..3` (0x800A147C..88)
    point at 0x1F801800..3. `g_com_delay_reg` points at 0x1F801020 and is
    set to 0x1325 by `CD_flush` (:409). `D_800A1490` is the SPU register
    base `CD_initvol` writes the CD and main volumes through (:414-425).
- `cdread.c`: `CdRead`, `CdReadSync`, `CdReadBreak`, `CdReadCallback`,
  `CdReadMode` and the callbacks `cb_read` / `cb_data` / `cd_read_retry`.

### `CdRead` state

`CdRead` (src/main/psxsdk/libcd/cdread.c:166) keeps its state in one Sony
struct, `CdlREAD D_800A14D0` (include/psxsdk/libcd.h:41-55):

| Member | Address | Role |
|--------|---------|------|
| `sectors` | `0x800A14D0` | sectors requested (cdread.c:181) |
| `buf` | `0x800A14D4` | the buffer argument (cdread.c:180) |
| `p` | `0x800A14D8` | write pointer: `buf` at each (re)start (:146), advanced per sector (:57, :89) |
| `mode` | `0x800A14DC` | CdlSetmode byte; `mode & 0x30` picks `size`, 0x20 is ORed in (:167-179) |
| `size` | `0x800A14E0` | sector size in words: 0x200 / 0x249 / 0x246 (:168-177) |
| `cnt` | `0x800A14E4` | sectors left; -1 on error |
| `t2` | `0x800A14E8` | `VSync(-1)` at each sector or retry (:65, :149); `CdReadSync` retries 60 vsyncs after it (:200) |
| `t1` | `0x800A14EC` | `VSync(-1)` at the start (:187); 1200-vsync timeout (:69, :199) |
| `pos` | `0x800A14F0` | `CdPosToInt(CdLastPos())` (:141), +1 per sector |
| `cbsync` / `cbready` / `cbdata` | `0x800A14F4` / `F8` / `FC` | callbacks saved at the start (:182-186), restored at the end (:75-79) |
| `tslmode` | `0x800A1500` | `CdReadMode`'s flag: bit 0 reads with `CdGetSector2` and advances in `cb_data` |

## Interrupt hooks (libetc)

`startIntrVSync` (src/main/psxsdk/libetc/intr_vb.c:15) does four things:

- writes 0x107 to root counter 1's mode register through `D_800A2638`;
- clears `Vcount`;
- clears the eight slots `D_800A2614[8]`;
- hooks IRQ 0 to `trapIntrVSync`.

`trapIntrVSync` (:23) increments `Vcount` and calls each non-null slot.
`setIntrVSync(slot, fn)` (:40) fills a slot.

`startIntrDMA` (src/main/psxsdk/libetc/intr_dma.c:22) clears the eight
callback slots `D_800A2640` (seven are used) and DICR (`*D_800A263C`, 0x1F8010F4), then hooks
IRQ 3 to `trapIntrDMA`. `trapIntrDMA` (:29) loops while DICR bits 24..30 are
set: it acknowledges each bit and calls its slot. It prints a bus-error
report when DICR shows one. `setIntrDMA(ch, fn)` (:52) fills a slot and sets
or clears the channel's enable bit (16 + ch).

## Memory card

`src/main/memcard.c` wraps the BIOS card calls:

- `memcard_Init` (:28) runs `InitCARD(1)`, `StartCARD()` and `_bu_init()`.
  It then opens and enables eight events: four software-card events (class
  0xF4000001) and four hardware-card events (class 0xF0000011). Each set
  covers spec 4 (I/O end), 0x8000 (error), 0x100 (timeout) and 0x2000 (new
  card). `memcard_Quit` (:53) closes them and runs `StopCARD`.
- `memcard_PollSwEvents` (:91) returns 1..4 for whichever software event has
  fired, else 0. `memcard_PollSwEventsTimeout` (:67) tests all four, letting the
  later events win, and returns 2 once `g_memcard_poll_count` passes 0x78. `memcard_WaitHwEvent` (:111)
  spins on the hardware events. `memcard_AckSwEvents` (:104) and
  `memcard_AckHwEvents` (:128) clear them.
- `memcard_CountFiles(port, card)` (:135) lists `"bu%1d%1d:*"` into
  `g_memcard_file_list` with `firstfile` / `nextfile`.
  `memcard_CountFreeBlocks` (:153) returns 15 minus the 8 KB blocks in use.
  `memcard_HasFile(name)` (:189) searches the list.
- `memcard_ReadFile` (:244) / `memcard_WriteFile` (:262) open
  `"bu%1d%1d:%s"` and keep the descriptor in `g_memcard_fd`. They issue
  `read` / `write` and return 0 when `memcard_WaitHwEvent` reports I/O end.
  `memcard_WriteFile` first creates the file (`open(name, (blocks << 16) |
  0x200)`) when its last argument is set.

The save flow is `memcard_StepRequest` (28708.c:277), stepped on `g_memcard_request_state`:

- **Save (state 3):** it checks for the save file `D_80010AAC`
  (`"BASLUS-00663BUSHIDO2"`, through `g_memcard_save_file_name`) and for free blocks. It
  clears the 0x200-byte buffer `g_shared_buffer` (`memcard_ClearBuffer`) and writes the
  card header into it (`memcard_BuildFileHeader`: icon CLUT and image). It fills the
  save block at `+0x100` (`memcard_BuildSaveBlock`, an `Unk800F34D8Save`,
  include/game.h:40-52), then writes 0x200 bytes.
- **Load (state 5):** it reads the file into the same buffer.
- **Completion:** `memcard_PollFileCompletion` (28708.c:413) reaps the
  result through `memcard_PollSwEvents` and closes the file. After a load it
  restores the save block with `memcard_RestoreSaveBlock(&D_800F34D8)`. It posts a
  result code to `g_memcard_request_status`.

## PCdrv host files

`pcdrv_LoadFile(name, dest)` and `pcdrv_LoadSectors(name, dest, sector,
count)` (src/main/6CF8.c:47, :78) read files from a development PC through
the SN Systems PC file server. They use `PCopen`, `PClseek` and `PCclose`
(src/main/psxsdk/libsn/73E98.c:12-14), and `PCread`
(src/main/psxsdk/libsn/7407C.c:14), which calls `_SN_read` in pieces of at
most 0x8000 bytes. `pcdrv_LoadFile` reads in 0x4000-byte pieces and returns
the size. `pcdrv_LoadSectors` reads `count` 0x800-byte sectors from
`sector * 0x800`. Both return -2 when the open fails and -1 on a short read.
Nothing in `src/` or `asm/` references either function. They are separate
from the memory-card path, though their short-read error path closes with
BIOS `close`.
