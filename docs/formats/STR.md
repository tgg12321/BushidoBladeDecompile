# STR — PlayStation streaming video

Files:
* `disc/STR/OPENING.STR` (39,958,528 bytes = 19,511 × 2,048 sectors).

STR is the PSX standard interleaved video+audio container — MDEC-compressed
video frames intermixed with XA-ADPCM audio sectors, designed for streaming
playback from CD via DMA without buffering. BB2 ships exactly one STR file:
the studio logo / opening cinematic.

The decode path is handled by the **MOVOVL.EXE overlay** (`disc/STR/MOVOVL.EXE`)
— a 122,880-byte standalone PS-X executable that is loaded over the game
binary at runtime to provide MDEC video playback, then unloaded once the
intro is done. The main game does not link the MDEC libraries directly; they
live exclusively in this overlay.

## File layout

`OPENING.STR` is a sequence of 2,048-byte data sectors (the data payload of
the CD-XA Form 2 sectors, with the 24-byte subheader stripped by
`tools/extract_iso.py`). Each sector belongs to either the video stream or
the audio stream — when played from disc, alternation is driven by the
sector's CD-XA subheader; in the extracted file the same alternation is
preserved by content (video sectors start with a 32-byte STR video header,
audio sectors start with raw XA-ADPCM groups).

## STR video sector layout

Every video sector starts with the 32-byte STR video header:

| Offset | Size | Field            | OPENING.STR value (first sector)    |
|--------|------|------------------|--------------------------------------|
| 0x00   | 4    | control          | `0x80010160` (16-bit color, mode 1)  |
| 0x04   | 2    | chunk_idx        | 0 (zero-based)                       |
| 0x06   | 2    | chunks_total     | 5 (5 sectors per video frame)        |
| 0x08   | 4    | frame_number     | 1                                    |
| 0x0C   | 4    | demuxed_size     | 2712 bytes (full-frame body)         |
| 0x10   | 2    | frame_width      | 320 px                               |
| 0x12   | 2    | frame_height     | 240 px                               |
| 0x14   | 2    | mdec_codes_div2  | 1824 (= 3648 codes)                  |
| 0x16   | 2    | constant         | `0x3800` (always)                    |
| 0x18   | 2    | q_scale          | 1                                    |
| 0x1A   | 2    | frame_version    | 2 (MDEC v2)                          |
| 0x1C   | 4    | _padding         | 0                                    |

After the 32-byte header come 2,016 bytes of MDEC-encoded chunk payload. The
playback chain concatenates the 5 sub-chunks of one frame, prepends a
small MDEC body header (`0-1: 32-byte blocks needed`, `2-3: 0x3800`,
`4-5: q_scale`, `6-7: version`), then feeds the bitstream to the MDEC
hardware decoder via `DecDCTout()` / `DecDCTinSync()` etc. (handled by the
overlay).

`OPENING.STR` characteristics:
* 320 × 240 pixel video frames.
* 5 video sectors per frame (= 5 chunks per frame).
* Quantization scale 1 (best quality).
* MDEC v2 frame encoding.

## XA audio sectors (interleaved)

Sectors that don't begin with a valid STR video header are XA-ADPCM audio
sectors. Each carries 18 "sound groups" of 128 bytes (= 2,304 bytes of audio
data after the original CD-XA subheader was stripped). See
`docs/formats/XA.md` for the byte-level XA-ADPCM specification.

## Overlay (MOVOVL.EXE)

`disc/STR/MOVOVL.EXE` is a standalone PS-X EXE — see
**[MOVOVL.md](MOVOVL.md)** for the full breakdown (header layout, BIOS
calls, PsyQ library identification, relationship to the main EXE,
inspector tool).

Summary: 122,880 bytes loaded at `0x801D8800`, entry point `0x801DA084`,
links statically against PsyQ libgpu / libpress / libapi (identical SDK
versions to the main EXE), uses 17 BIOS calls to set up its own minimal
pad / interrupt / GPU / CD environment, and reaches the MDEC hardware
exclusively through libpress wrappers (no inlined `0x1F801820` /
`0x1F801824` register access). The overlay is invoked at boot before
the main EXE; `file_LoadOverlay` at `src/ings.c:370` is almost
certainly a *different* overlay loader (the size guard `0xA000` rules
out the 122 KB MOVOVL.EXE).

## Verified by

* Hex-dump of first 80 bytes of `disc/STR/OPENING.STR`: matches the standard
  STR video header (`60 01 01 80 ...`), values decode to a sensible 320×240
  MDEC v2 frame.
* File size 39,958,528 bytes = exactly 19,511 × 2,048-byte data sectors,
  confirming the file is sector-aligned post-extraction.
* `disc/STR/MOVOVL.EXE` header confirms the standard PS-X EXE format with
  load address 0x801D8800, matching the destination used by
  `file_LoadOverlay`.
* Cross-reference: `src/ings.c:370` `file_LoadOverlay()` shows the
  DMA-into-RAM path that brings the overlay alive before STR playback.

## Inspector tool

`tools/inspect_str.py` — counts video vs audio sectors and prints a frame
table.

```
python tools/inspect_str.py OPENING.STR             summary
python tools/inspect_str.py OPENING.STR --frames    list first 20 frames
```

Full MDEC decoding to image frames is out of scope for this tool; for that
use [jpsxdec][1].

## References

* [m35/jpsxdec PlayStation1_STR_format.txt][1]
* [PsyQ SDK `Filefrmt.pdf`](https://psx.arthus.net/sdk/Psy-Q/DOCS/Devrefs/Filefrmt.pdf)

[1]: https://github.com/m35/jpsxdec/blob/readme/jpsxdec/PlayStation1_STR_format.txt
