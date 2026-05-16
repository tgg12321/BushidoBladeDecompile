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

## High-level API — `snd_*` in `sound.c`

The main user-facing sound API:

| Function | Purpose |
| --- | --- |
| `snd_LoadBgm(id)` | Load a BGM track from sound bank (allocates channel 8) |
| `snd_PlayBgm(id)` | Start playback on the loaded BGM (`saSeMain_80045510`) |
| `snd_StopBgm` | Halt BGM playback |
| `snd_AllocSe` | Reserve channel 9 for sound effects |
| `snd_LoadSe(id)` | Load an SE sample into channel 9 |
| `snd_PlaySe(id)` | Play loaded SE |
| `snd_LoadSelection(id)` | Load a UI/selection sound into channel 0xA |
| `snd_StopSelection` | Halt UI sound |
| `snd_PlaySystemSe` | Boot/init sound (`func_80078A68(0xF2000001, ...)`) |
| `snd_StopAll` | Stop everything |
| `snd_SetVolume(v)` | Set master `g_snd_volume` |

The BGM callbacks (`snd_BgmCallback`, `snd_SeNullCallback`,
`snd_SelectionCallback`) are invoked by the channel manager when the
underlying sample finishes — letting the engine queue the next entry in
a sequence.

## BGM streaming — XA-ADPCM

BGM in BB2 is **streamed from disc XA sectors**, NOT loaded as raw SPU RAM
samples. The XA files are in `disc/XA_0/` and `disc/XA_1/`. The PS1
hardware can decode CD-XA ADPCM directly into the SPU's CD audio input,
so the CPU only needs to start/stop the stream.

`marionation_camera_Init_80036064` (`code6cac_b2.c:380`) is the XA-stream
controller — despite its name (Kengo naming collision), it manages the
CD callback-B mechanism that polls the CD for XA-stream progress and
calls back at frame boundaries.

XA loading sequence:
1. `cdrom_SetCallbackB(callback)` (`system.c:129`) installs the
   per-sector callback
2. `func_80080390(9, 0)` (called frequently in `code6cac_b2.c:408`) sets
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
`file_LoadSoundData` (`ings.c:388`) loads them at boot:

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

`coli_HitPauseKatana` (`main.c:1693`), `coli_HitPauseKatana_2`
(`main.c:1983`), and `exec_game` (`main.c:1798`) are the misnamed **SPU
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

## Sequence scheduler — `seq_*` (`text1a_c.c`)

The `seq_*` family (`seq_Start`, `seq_Reset`, `seq_GetState` in
`text1a_c.c:1480-1521`) is a small high-level sound sequencer. It plays
**a sequence of cues** (boot music → mode-select music → in-game music)
with timing controlled by frame counters.

State:
- `D_800A3244` (`seq_GetState`) — 0=idle, 1=playing
- `D_800A3398` — current sequence data pointer
- `D_800963EE` — table of sequence base offsets

`seq_Start(seq_id, data_ptr)` starts a sequence at the data pointer,
playing the cue id `seq_id + 0x25` (offset 0x25 is the sequencer's
"start of sequence cues" marker).

`func_800450F4(cue_id, callback)` advances the sequence to a specific cue,
calling the data-pointer's offset table to find the cue's content and
invoking `func_800520B8(cue_data, callback, length)` to play it.

This is the layer above `snd_LoadBgm`/`snd_PlayBgm` — it handles the
sequencing of multiple BGM segments (e.g., "play title theme → fade to
character-select music"). Used during scene transitions where the same
"theme" has multiple parts.

## Per-frame voice control

`single_game_VoiceContorol()` is called every frame from the main loop
(`ings.c:613`). It's the per-frame heartbeat for the sound system:
- Advance the BGM sequencer state
- Tick the SE channels
- Apply fade effects (`g_snd_fade_pos`, `g_snd_fade_amt`)
- Update master volume from `g_snd_volume`

The function itself is still asm-only.

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
- `single_game_VoiceContorol` — per-frame voice ticker
- `func_8005B43C`, `func_8005B7C4`, `func_8005C614`, `func_8005C650` — the
  SE bank loaders
- Most of the `text1a_c*.c` 0x42504-0x460E4 range (effect-channel mgmt)
- All `saTan*` functions except a few small wrappers
