# Sound / SPU System

BB2 uses a multi-layered sound architecture: PsyQ libspu for raw SPU
access, a custom voice-allocator (`coli_HitPauseKatana` family), a
channel-oriented dispatcher (`snd_*`), and a high-level "sequence"
scheduler (`seq_*` family in `text1a_c.c`).

The actual sound output is done by the PS1 SPU (Sound Processing Unit)
with 24 voices, fed from 512KB of SPU RAM. BGM is streamed via XA-ADPCM
from the disc. Sound effects come from `.SE` files in `disc/LOADSE/`,
loaded into SPU RAM at boot.

## Channel model

BB2 partitions the 24 SPU voices into named channels. The channel
constants are in `include/bb2_const.h`:

```c
#define SND_CHANNEL_BGM  8      /* Background music channel */
#define SND_CHANNEL_SE   9      /* Sound effects channel */
#define SND_CHANNEL_UI   0xA    /* UI/selection sounds channel */
```

The number isn't a single SPU voice — it's a **channel category index**
that the channel allocator (`func_8004574C`, `func_800455AC`) maps to a
range of actual SPU voices. Each channel has its own playback state,
volume, and queue management.

The per-channel state is at `g_snd_ch_data` (`0x800EF848`) — a per-channel
array of state blocks. The "currently active count" is tracked at
`g_snd_ch_count` (`0x80107808`). Per-channel status at `g_snd_ch_status`
(`0x80102A68`). Per-channel SPU memory addresses at `g_snd_ch_addr`
(`0x80107810`).

## High-level API — `snd_*`

The named `snd_*` functions (libsnd wrappers; `src/main/3AB48.c` unless noted):

| Function | What the body does |
| --- | --- |
| `snd_Init` | Clears the 16 VAB slots (`g_vab_rec_ptr`, `g_vab_vb_sbaddr`), `SsInit`, reverb off, `SsSetTickMode(1)`, sets the 24-voice volume table `D_800EFB78` to 0x7F, `SsStart` |
| `snd_Quit` | Reverb off, `SsEnd`, `SsQuit`, clears the 16 VAB slots |
| `snd_InitAndLoadCommonVab` (`6CF8.c`) | `snd_Init`; `snd_LoadCommonVab(0x801D8800)` (panics above 0xD00 bytes); copies it to 0x8010DB00 and rebases slot 0 there with `snd_VabFakeOpen` |
| `snd_LoadCommonVab(dst)` | Reads disc file `cdrom_GetFileIndex(2, 1)` into `dst` (waiting with `cdrom_WaitIdle`) then hands it to `func_8005C2A8` |
| `snd_VabOpen(pack, id)` | `SsVabOpenHeadSticky` + `SsVabTransBody` for VAB slot `id` |
| `snd_VabFakeOpen(delta, id)` | Rebases slot `id`'s record by `delta`, then `SsVabFakeHead` / `SsVabFakeBody` |
| `snd_VabFakeOpen8And4`, `snd_VabFakeOpen9` | `snd_VabFakeOpen` on slots 8 and 4, or 9 |
| `snd_MoveVabBody(buf, id, dst, src)` | Moves slot `id`'s VAB body in SPU RAM from `src` to `dst` through `buf`, then re-fakes its head |
| `snd_CloseVab1`, `snd_CloseListedVabs` | `SsVabClose` on slot 1, or on the three slots in `g_vab_id_list`, clearing their slot records |
| `snd_SerialMixOn` (`267A8.c`) | `SsSetSerialAttr` (CD input mix on) and `SsSetSerialVol` 0x7F / 0x7F |

## BGM streaming — XA-ADPCM

BGM in BB2 is **streamed from disc XA sectors**, NOT loaded as raw SPU RAM
samples. The XA files are in `disc/XA_0/` and `disc/XA_1/`. The PS1
hardware can decode CD-XA ADPCM directly into the SPU's CD audio input,
so the CPU only needs to start/stop the stream.

`marionation_camera_Init_80036064` (now `cdrom_ReadyCallback`, `code6cac_b4_post.c`) is the XA-stream
controller — despite its name (Kengo naming collision), it manages the
CD callback-B mechanism that polls the CD for XA-stream progress and
calls back at frame boundaries.

XA loading sequence:
1. `cdrom_SetCallbackB(callback)` (`system.c:116`) installs the
   per-sector callback
2. `func_80080390(9, 0)` (now `CdControlF`; called from `code6cac_b4_post.c` / `code6cac_b5.c`) sets
   the SPU SE channel into XA-receive mode
3. The CD spins up at sector address `D_80101E6C` (BCD form), reading
   each sector's XA-ADPCM payload directly into SPU CD-input
4. Per-sector callback advances the read pointer and decrements a
   sector-count

The "BGM track" identifier in `snd_LoadBgm` is therefore an XA sector
address (or a small index into a table of XA sector addresses) — not
SPU RAM data.

## SE bank — `.SE` files

`.SE` files in `disc/LOADSE/` and `disc/LOADSE1/` are raw SPU sample banks.
`file_LoadSoundData` (`ings.c:364`) loads them at boot:

1. `func_8005B43C()` — claim sound-data load
2. `func_8005B7C4(0x801D8800)` — read SE bank into the overlay scratch
   region at 0x801D8800; returns size
3. `bb2_memcpy(0x8010DB00, 0x801D8800, size)` — copy decoded data to
   `0x8010DB00` (the SE bank's permanent home in main RAM)
4. `saFidLoad(0xFFF35300, 0)` — kick the SPU DMA upload to SPU RAM
5. `func_8005C614()` — final mark

`g_snd_se_bank` (`0x80099C34`) is the SE bank parameter table — maps SE id
to sample address/length within the loaded bank.

## SPU initialization

`spu_Init` (called from `motion_Open`-driven CTORs) does:
1. `EnterCriticalSection`
2. Configure DMA channel 4 for SPU
3. Set SPU mode/volume registers via `spu_WriteReg`
4. Wait for SPU ready (polled on `g_spu_init_flag`)
5. `ExitCriticalSection`
6. Set `g_spu_init_flag = 1`

The boot code in `main.c:122-188` (`saTan5TakeAnim2_2`) is one of the SPU
initialization paths — it's named misleadingly because the Kengo name
table assigned it `saTan5TakeAnim2_2` (which is normally an animation
helper).

After init, `spu_TransferData` and `spu_TransferDirect` are used to upload
sample banks to SPU RAM via channel 4 DMA.

## Voice-key allocator — `coli_HitPauseKatana`

`coli_HitPauseKatana` (`main.c:2597`), `coli_HitPauseKatana_2`
(`main.c:2787`), and `exec_game` (`main.c:2693`) are the misnamed **SPU
voice allocator**. Despite the `coli_` prefix (Kengo collision), they
manage the assignment of SPU voices to sounds.

State:
- `g_spu_voice_key_a` (`0x800A2D38`) = number of allocated voices (count)
- `g_spu_voice_key_b` (`0x800A2D3C`) = "fragmenting" index
- `g_spu_voice_key_c` (`0x800A2D40`) = pointer to voice-key array

Each "voice key" is a 2-word entry:
- Word 0: status bits + length
  - `0x80000000` = allocated/playing
  - `0x40000000` = sentinel (end-of-list)
  - `0x2FFFFFFF` = freed-but-not-collected
  - low 28 bits = start offset within SPU RAM
- Word 1: length in samples

`coli_HitPauseKatana(req_len)` allocates a `req_len`-sample contiguous
region from the SPU RAM free-list. It scans the voice-key array, finds
the first slot that's free (`!(field0 & 0x80000000)`) and big enough,
splits it if necessary (inserting a new sentinel slot at offset+req_len),
and returns the offset.

`exec_game()` is the "compactor" — it walks the voice-key array forward and
backward, merging adjacent freed entries, removing sentinels (`0x2FFFFFFF`
markers), and shifting later entries down to fill gaps. It runs in 5 phases:
1. Forward merge: combine adjacent flagged-allocated entries
2. (handled in coli_HitPause's deallocate)
3. (handled in coli_HitPause's deallocate)
4. Compact: replace first sentinel with last entry, shrink count
5. Backward finalize: re-flag entries with the right header bits

The "fragmenting" index `g_spu_voice_key_b` tracks where the most recent
split happened, so subsequent allocations preferentially fragment further
into a known empty region rather than scan from the start.

This is a hand-tuned SPU voice manager — PsyQ's standard libspu allocator
is too simple for BB2's voice budget. The "katana" naming is presumably
because the original implementation was for the "hit pause" effect where
the engine slows the SPU voice rate during a blade strike (giving the
recognizable BB2 "swoosh" on hits).

## NDATA pack cache (not sound code)

`ndata_LoadPack` / `ndata_ReadFromPack` / `ndata_InvalidatePack` /
`ndata_IsPackLoaded` (`src/main/35000.c`) cache one NDATA entry and serve
reads from it; none calls libsnd. See
[NDATA.md](../formats/NDATA.md#pack-cache).

## Per-frame call once named `single_game_VoiceContorol`

The main-loop call at 0x80019568 is `pad_Update` (`src/main/87A0.c`): it reads both
pads and updates `g_pad_state`. It is not sound code.

## Fade curve

`g_snd_fade_curve` (`0x800EF800`) is a tabular fade curve — when the
engine wants to fade music in/out it indexes this table by current
`g_snd_fade_pos` and applies the result as a volume multiplier. The
curve is logarithmic (perceptual loudness ramp).

## Notes about the "saTan" prefix

Many sound functions have `saTan` in their name (e.g., `saSeMain_80045510`,
`saTan2InfoInit_80044F80`). Per project audit notes, "saTan" likely
stands for "samurai tanren" ("samurai training") and refers to the
practice/tutorial mode's sound layer. The same functions are reused for
non-training modes — only the naming convention is sound-engine specific.

## Where unmatched sound asm still hides

- `spu_Init`, `spu_TransferData`, `spu_TransferDirect` — PsyQ libspu wrappers
- `func_8005B43C`, `func_8005B7C4`, `func_8005C614`, `func_8005C650` — the
  SE bank loaders
- Most of the `text1a_c*.c` 0x42504-0x460E4 range (effect-channel mgmt)
- All `saTan*` functions except a few small wrappers

## SPU init/config state (2026-05-17)

The SPU initialization writes 4 constants used during channel setup,
located adjacent to `g_spu_base_addr`:

| Symbol | Address | Value | Role |
|--------|---------|-------|------|
| `dma_dpcr` | `0x800A2CEC` | ptr | Pointer to DPCR (0x1F8010F0), the DMA control register, not an SPU register; `_spu_init`'s `*p \|= 0xB0000` sets DMA channel 4's enable/priority (libspu spu.c:39) |
| `g_spu_init_const_a` | `0x800A2D00` | `2` | Used during channel setup |
| `g_spu_addr_shift` | `0x800A2D04` | `3` | Existing name; shift amount for channel addressing |
| `g_spu_init_const_c` | `0x800A2D08` | `8` | |
| `g_spu_init_mask` | `0x800A2D0C` | `7` | 3-bit mask; used as `& ~D_800A2D0C` to round-down channel index |
| `g_spu_init_reg_D1_val` | `0x800A2D44` | reg val | Default value written to SPU register 0xD1 via `spu_WriteReg` |

`g_spu_init_reg_D1_val` is cached in `D_800A2884` after writing to SPU reg 0xD1
on every call to `spu_InitEx`.

### Voice volume table

`g_spu_voice_vol_table` (`0x800A28A4`) is a 24-entry u16 default-volume table.
When `spu_InitEx(0)` runs, it fills the table by decrementing from
`D_800A28D2` down to `D_800A28A4` with `0xC000` (near-max volume).

Located between `g_spu_voice_mask` (`0x800A28A0`) and
`g_satan0_gauge_data_block` (`0x800A28D4`).

## Wave synthesis tables (2026-05-17)

The engine generates audio waveforms by indexing into the global `Judge[]`
sin/cos table with per-channel phase counters.

| Symbol | Address | Layout | Role |
|--------|---------|--------|------|
| `g_snd_voice_init_block` | `0x800EF070` | struct | SPU voice descriptor passed to `func_800417D0`; first byte = 0xE (cmd) |
| `g_snd_wave_phase_table` | `0x800EF558` | 17 × s32 | Phase indices, stepped by 0x12 per call to `func_80047A90` |
| `g_snd_wave_output_table` | `0x800EF59C` | 9 × 17 s32 | Audio wave-synthesis output: `(Judge[phase_idx] * 0x271) >> 10` |

The output is 9 audio channels × 17 samples = 153 entries (stride 0x44).
Each channel runs its own phase counter through `Judge[]`. See `func_80047A90`
(`text1b.c`).

## Sound-channel texture metadata (2026-05-17)

`AddTbpOfst` (`main.c:1843`) looks up texture metadata for an animated sprite
associated with each sound channel. There are 3 parallel per-channel tables
indexed by `g_snd_ch_status[idx]`:

| Symbol | Address | Per-entry purpose |
|--------|---------|-------------------|
| `g_snd_ch_status` | `0x80102A68` | u8 × 16, channel state (must == 1 for valid) |
| `g_snd_ch_texture_base_ptr` | `0x800F6660` | Base ptr to texture data |
| `g_snd_ch_texture_tpage` | `0x800F66B8` | TPage / u32 value |
| `g_snd_ch_texture_meta_ptr` | `0x800F6700` | Secondary ptr |
| `g_snd_ch_texture_max_frame` | `0x800FF634` | Max frame offset (compared against arg) |

## Named function helpers (2026-05-17)

| Function | Address | Purpose |
|----------|---------|---------|
| `spu_FlushVoiceChannels` | `0x80088740` | Clears SPU control regs, polls voice-status (mask 0x7FF) with timeout. Debug: `"SPU:T/O [%s]"`. |
| `snd_ChannelDmaFlush` | `0x80087F64` | Per-channel DMA cleanup + decrement count. |
| `snd_ClearAllSeBuffer` | `0x8002906C` | Walks `snd_GetSeId()` list, zeroes +2 of each 0x10-byte entry. |
| `spu_TransferDirect_capped` | `0x8008AD64` | `spu_TransferDirect(a0, min(a1, 0x7EFF0))` + clear busy if !init. |
| `spu_TransferData_capped` | `0x8008ADC4` | Same with `spu_TransferData`. |

## MIDI-style sequence event dispatcher — `saTan0Main`

`saTan0Main(s16 char_id, s16 channel)` (`main.c:334-454`) is BB2's
**per-character sequence-stream interpreter**.  It reads a stream of
command bytes from a per-character × per-channel state struct
(`((void**)&g_per_char_state_F28[char_id]) + channel*0xB0`, where
`g_per_char_state_F28` = `D_80106F28`) and dispatches each event to a
slot in the 5-entry function-pointer table at `0x800F3340..0x800F3350`.

The encoding follows MIDI conventions:

| Status | Slot | Handler |
|---|---|---|
| 0x90 (Note On) | `g_seq_event_handler_90_NoteOn` | Reads note+velocity; also triggers `spu_ReadMotionFrame()` |
| 0xB0 (Control Change) | `g_seq_event_handler_B0_CtrlChange` | Reads controller byte |
| 0xC0 (Program Change) | `g_seq_event_handler_C0_PgmChange` | Reads program byte |
| 0xE0 (Pitch Bend) | `g_seq_event_handler_E0_PitchBend` | Reads pitch byte |
| 0xFF (Meta) | `g_seq_event_handler_FF_Meta` | Reads meta type; sub-type 0x2F (end-of-track) goes to `func_80084A7C` instead |

The function implements MIDI-style "running status" — if a byte's high
bit is clear, it re-uses the last status byte stored at `state[0x16]`.

This is the LOW-LEVEL event dispatcher.

**Mystery: the 5 dispatch slots are NEVER WRITTEN by BB2 code.**  A
byte-level scan of both main and overlay binaries found zero stores to
these addresses (no sw with matching offset, no literal value, no
indirect construction).  Yet saTan0Main calls them unconditionally in
their case-branch.  The most plausible interpretation is that the
`if (b & 0x80)` + switch arms that reach the dispatch slots are
*effectively dead code* in shipped BB2 — the runtime data stream
never has a byte with the high bit set, so the call sites are never
reached.  This suggests the dispatch table was implemented for a
debug/future build but never populated in the shipped ROM.  See
`pre-slim-2026-10-01:docs/engine/recent_naming_findings.md` §11 for the binary-scan
evidence.

## Cross-references (naming pass 2026-05-17; full traces at `pre-slim-2026-10-01:docs/engine/recent_naming_findings.md`)

Three clusters from the placeholder-refinement pass document the
SPU/voice update path more directly:

- §11 Sequence-event handler table (MIDI-style dispatch)
  — `g_seq_event_handler_{90_NoteOn, B0_CtrlChange, C0_PgmChange,
  E0_PitchBend, FF_Meta}` at `0x800F3340..0x800F3350` (the dispatch
  table for the `saTan0Main` MIDI-style interpreter above).
- §12 Sound data buffer pointer cluster
  — `g_snd_data_buf_base` + 5 subblock pointers at `0x800EFB14`,
  set by `func_80054604` (snd_LoadSelection path) and `func_8005490C`
  (header-offset unpack), relocated together by `func_80054FDC(delta)`
  when the buffer moves.  Connects the sound-loader to the saTan family.
- §13/14 SPU voice0E setup cluster
  — Full SPU voice0E definition: `g_snd_voice_init_block` struct at
  0x800EF070 with vol/pitch baselines at +0x4C/+0x54; double-buffered
  envelope blocks `g_snd_voice_envelope_block_a/b` for scratchpad-DMA;
  `g_snd_wave_phase_table` + `g_snd_wave_output_table` for the synth
  generator that ALSO computes `g_snd_fade_curve` as its 8th channel.
