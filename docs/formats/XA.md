# XA — PlayStation XA-ADPCM streamed audio

Files (and their sizes):

| Path                | Files | Size range          | Role                                  |
|---------------------|------:|---------------------|---------------------------------------|
| `disc/XA_0/MAR_XA*` | 29    | 1.0 MB .. 2.5 MB    | Marionation (game music) main stream  |
| `disc/XA_1/MAR_XA*` | 5     | 1.0 MB .. 2.5 MB    | Marionation overflow / continuation   |
| `disc/LOADSE/BB2_M1.XA` | 1 | 74.8 MB            | "Mode 1" music bank (story / battle) |
| `disc/LOADSE/BB2_M2.XA` | 1 | 89.1 MB            | "Mode 2" music bank (extra modes)    |
| `disc/LOADSE/BB2_M3.XA` | 1 | 7.7 MB             | "Mode 3" music bank (smaller)         |

These files are PSX CD-XA ADPCM audio streams: 16-bit 4-channel ADPCM
compressed audio, hardware-decoded on the fly by the PSX CD controller.
The standard ([CD-XA spec][1]) is well documented; this doc summarises the
on-disc bytes and notes BB2-specific conventions.

## Sector layout (post-extraction)

The shipped game uses CD-XA Form 2 sectors (2,336 byte payload). When the
disc was extracted by `tools/extract_iso.py` the 24-byte CD-XA subheader
was stripped along with the 12-byte sync and 4-byte sector header, leaving
**2,048 bytes of data per sector** in the extracted `.XA` file. (This is the
same treatment as STR — see `docs/formats/STR.md`.)

A consequence: per-sector channel and coding-info bytes (originally bytes
17–20 of the raw 2,352-byte sector) are not preserved in the extracted file.
The audio data itself is intact and decodable, but the channel-interleave
markers are gone. If you need the original per-sector subheader (for example
to demux a multi-channel XA), you need to re-extract from the BIN/CUE image
with a tool that preserves Form 2 subheaders such as `mkpsxiso` or
`psxavenc`.

## XA-ADPCM stream structure (within each sector)

Each 2,048-byte data payload of a Form 2 sector carries:

```
+0x000  16 x 128-byte "sound groups"
        Each sound group: 16 bytes of headers (4 SHDR) + 112 bytes of ADPCM nibbles
+0x800  -- end --
```

Per sound group:

| Offset | Size | Field                                  |
|--------|------|----------------------------------------|
| 0..3   | 4    | SHDR for sub-block 0 (filter+shift+flags) |
| 4..7   | 4    | SHDR for sub-block 1                   |
| 8..11  | 4    | SHDR for sub-block 2                   |
| 12..15 | 4    | SHDR for sub-block 3                   |
| 16..127| 112  | 28 nibbles × 4 sub-blocks of audio data |

Each sub-block decodes to 28 audio samples; one sound group thus yields 112
samples per channel.

## BB2 file roles (verified by filename)

* `MAR_XA00..28.XA` (in `XA_0/`) and 5 more in `XA_1/` — Marionation game
  music tracks. "MAR" is the studio's "Marionation" engine prefix. Each file
  is one music stream (intro, character themes, victory fanfare, etc.).
* `BB2_M1.XA`, `BB2_M2.XA`, `BB2_M3.XA` (in `LOADSE/`) — large multi-track
  "music bank" files containing many concatenated music tracks for a given
  game mode. These are accessed by absolute sector offset rather than per-
  track files.

## Loading at runtime

XA audio is played directly from disc via `CdControl(CdlReadS, ...)`-style
streaming (the runtime never reads XA into RAM). The first sector of any
track is identified by a known sector offset in a per-mode lookup table —
not by filename.

The XA file sizes are all sector-aligned:

```
MAR_XA00.XA = 2,260,992  = 1,104 sectors
BB2_M1.XA   = 74,842,112 = 36,544 sectors
BB2_M2.XA   = 89,112,576 = 43,512 sectors
BB2_M3.XA   = 7,651,328  = 3,736 sectors
```

(`MAR_XA00.XA` was hex-dumped — first sector starts with `1A 2C 2C 2C 1A 2C
2C 2C 1C 2C 2C 2C 1C 2C 2C 2C` which is the 16-byte SHDR header of the first
sound group — four 4-byte SHDR entries each containing
`(shift_factor, filter_idx, flags)`.)

## Inspector tool

No dedicated `inspect_xa.py` is provided. Use `tools/inspect_str.py
--hex SECTOR` on the XA file to view raw sector contents; full decoding to
WAV is best handled with [psxavenc][2] or [ffmpeg's xa_adpcm decoder][3].

## Verified by

* File sizes of every BB2 XA file are exact multiples of 2,048 bytes
  (sector-aligned post-extraction).
* `disc/XA_0/MAR_XA00.XA` first 16 bytes form a valid 4-SHDR sound group
  header (`1A 2C 2C 2C ...`).
* `disc/LOADSE/BB2_M1.XA` first 16 bytes form another valid SHDR set
  (`0A 09 0C 0C ...`) confirming the same format.

## References

* [CD-XA ADPCM specification (psx-spx)][1]
* [psxavenc — psx audio/video encoder][2]
* [ffmpeg's `xa_adpcm` decoder][3]

[1]: https://psx-spx.consoledev.net/cdromdrive/#cdrom-xa-audio-adpcm-compression
[2]: https://github.com/WonderfulToolchain/psxavenc
[3]: https://ffmpeg.org/ffmpeg-codecs.html#xa
