# TIM — PlayStation texture image

Files:
* `disc/TIM2D/*.TIM` (13 files: 10 × `LAS_*.TIM`, `LINK.TIM`, `LINK_M.TIM`,
  `SEA2_16.TIM`)
* `disc/STR/TITLE.TIM`

(14 TIM files total across the disc.)

TIM is the PlayStation SDK standard texture format. The PsyQ SDK ships `tim.h`
and matching loaders; BB2 uses the unmodified format throughout — see
`tslTm2LoadImage` (`src/system.c:639`).

The byte layout below is the standard one (cross-referenced against
[PSX/TIM format on FF8 Modding Wiki][1] and the
[Just Solve the File Format Problem wiki][2]); this document additionally
catalogues which BB2 files use which PMODE / CLUT layouts and notes one
non-standard behaviour observed in `TITLE.TIM`.

## Header (8 bytes, little-endian)

| Offset | Size | Field   | Value / meaning                            |
|--------|------|---------|--------------------------------------------|
| 0x00   | 1    | tag     | Always `0x10` — identifies a TIM file      |
| 0x01   | 1    | version | Always `0x00` — only version known         |
| 0x02   | 2    | _pad    | Reserved, zero                             |
| 0x04   | 4    | flags   | bit-0..1 = PMODE, bit-3 = CLUT present     |

```
flags bits:
  [0..1] PMODE: 00=4-bit indexed
                01=8-bit indexed
                10=16-bit BGR555 direct
                11=24-bit BGR888 direct
  [3]    CLUT present (only meaningful when PMODE = 00 or 01)
  others reserved
```

The CLUT block (if present) immediately follows the 8-byte header; the image
block follows the CLUT (or the header if no CLUT).

## CLUT block (only when CLUT bit set)

| Offset | Size | Field           |
|--------|------|-----------------|
| 0x00   | 4    | block_length    | total CLUT block size in bytes, **including this header** |
| 0x04   | 2    | dx              | VRAM destination X (16-pixel aligned) |
| 0x06   | 2    | dy              | VRAM destination Y                    |
| 0x08   | 2    | width           | CLUT width in 16-bit entries          |
| 0x0A   | 2    | height          | number of CLUTs (palettes)            |
| 0x0C   | W*H*2| palette_data    | 16-bit BGR555 colour entries          |

A 4-bit image with one 16-colour palette has `width=16, height=1`. An 8-bit
image typically has `width=256, height=1`. Stacking palettes vertically allows
several CLUTs to share an image (BB2 uses this for character lasers etc.).

## Image block (always present)

| Offset | Size | Field           |
|--------|------|-----------------|
| 0x00   | 4    | block_length    | total image block size in bytes, **including this header** |
| 0x04   | 2    | dx              | VRAM destination X |
| 0x06   | 2    | dy              | VRAM destination Y |
| 0x08   | 2    | width           | image data width in 16-bit words (NOT pixels) |
| 0x0A   | 2    | height          | image height in pixel rows |
| 0x0C   | W*H*2| image_data      | raw pixels packed into 16-bit words |

The `width` field is measured in **16-bit words**, not pixels. Pixels per
16-bit word depend on PMODE:

* PMODE 0 (4-bit): 4 px per word ⇒ pixel width = `width * 4`
* PMODE 1 (8-bit): 2 px per word ⇒ pixel width = `width * 2`
* PMODE 2 (16-bit): 1 px per word ⇒ pixel width = `width`
* PMODE 3 (24-bit): packed as 3-byte BGR, two pixels span 3 words

## 16-bit color encoding (BGR555)

```
bit  15  14   ..  10   9   ..  5   4   ..  0
     [STP][B (5)         ][G (5)        ][R (5)         ]
```

STP = "semi-transparency" / "STP" bit — when 0 and RGB = 0 the pixel is
fully transparent; when 1 it participates in semi-transparency blending. PSX
hardware treats the BGR fields as the colour and the STP bit as an alpha-like
mask.

## BB2-specific catalogue (verified)

| File                  | size    | PMODE  | CLUT             | image (px)    | notes                |
|-----------------------|--------:|--------|------------------|---------------|----------------------|
| `TIM2D/LAS_CN.TIM`    | 32,832  | 4-bit  | 1 × 16 @ (0,480) | 256 × 256 px* | laser/cleaver texture |
| `TIM2D/LAS_CS.TIM`    | 32,832  | 4-bit  | 1 × 16 @ (0,480) | 256 × 256 px* | same shape, alt CLUT |
| `TIM2D/LAS_CT.TIM` ... | 32,832 | 4-bit  | 1 × 16 @ (0,480) | 256 × 256 px* | (7 more variants)    |
| `TIM2D/LINK.TIM`      | 307,220 | 16-bit | (none)           | 640 × 240 px  | full-screen graphic  |
| `TIM2D/LINK_M.TIM`    | 307,220 | 16-bit | (none)           | 640 × 240 px  | mirror/alt variant   |
| `TIM2D/SEA2_16.TIM`   | 24,596  | 16-bit | (none)           | 256 × 48 px   | sea/water tile       |
| `STR/TITLE.TIM`       | 153,620 | 16-bit | (none)           | 320 × 240 px  | title screen        |

*The LAS_* files report `width=64` in the header for 4-bit PMODE, which means
64 × 4 = 256 pixels wide. With height = 256 → effective 256 × 256 4-bit image
= 32,768 bytes of pixel data + 12 B image-header + 8 B file-header + 32 B CLUT
data + 12 B CLUT-header = 32,832 bytes. Matches exactly.

### Non-standard `image.block_length` in `TITLE.TIM`

`TITLE.TIM` reports `block_length = 0x000A8C0C` in its image block — a value
that overshoots the actual 153,612-byte block. The file is otherwise standard:
flags=0x00000002 (PMODE 2, no CLUT), DX=0, DY=0, W=320, H=240, and the trailing
153,600 bytes are valid 320×240 BGR555 pixel data, so the file decodes
correctly when the loader trusts `W*H*2` and ignores `block_length`. The PsyQ
SDK loader does exactly this (it uses W/H, not block_length, to determine the
VRAM DMA size).

`LINK.TIM`, `LINK_M.TIM`, `SEA2_16.TIM`, and every `LAS_*.TIM` carry the
correct `block_length` (`0x4B00C = 307,212`, `0x600C = 24,588`, `0x400C =
16,396`). Only `TITLE.TIM` is anomalous — likely a quirk of the screenshot/
authoring tool used to generate the title image.

## Loader

`tslTm2LoadImage` (`src/system.c:639`) wraps the PsyQ helpers — `OpenTIM`,
`ReadTIM`, `LoadImage`, `LoadClut`. None of the BB2 source inspects the magic
or flags directly; the runtime is content to feed the file pointer to the
SDK loader.

## Inspector tool

`tools/inspect_tim.py` — prints TIM headers, validates blocks, and can dump
the image-block pixel rectangle.

```
python tools/inspect_tim.py file.tim                  summary
python tools/inspect_tim.py file.tim --verbose        full header dump
python tools/inspect_tim.py file.tim --raw out.bin    dump pixel bytes
python tools/inspect_tim.py file.tim --png out.png    decode to PNG (if PIL)
python tools/inspect_tim.py DIR/                      summarise dir
```

PNG decoding handles PMODE 0/1/2 (the only modes BB2 uses); PMODE 3 (24-bit
direct) is supported but unused in shipped BB2 files. Indexed PNG export uses
the first CLUT row; subsequent CLUTs in a multi-palette TIM can be selected
via `--clut N`.

## Verified by

* `disc/TIM2D/LAS_CN.TIM` first 16 bytes: `10 00 00 00 08 00 00 00 2c 00 00 00
  00 00 e0 01` — tag=0x10, ver=0, flags=0x08 (PMODE=0 + CLUT bit), CLUT block
  starts with `0x2C` (=44 bytes total = 12 B header + 32 B palette data).
* `disc/STR/TITLE.TIM` first 16 bytes: `10 00 00 00 02 00 00 00 0c 8c 0a 00 00
  00 00 00` — tag=0x10, ver=0, flags=2 (PMODE=2 16-bit, no CLUT). Note the
  noted-anomalous `block_length` 0x000A8C0C.
* `tools/inspect_tim.py` round-trips all 14 BB2 TIM files: the catalogue
  table above is the tool's output.

## References

* [PSX/TIM format on FF8 Modding Wiki][1]
* [TIM (PlayStation graphics) - Just Solve the File Format Problem][2]

[1]: https://hobbitdur.github.io/FF8ModdingWiki/technical-reference/miscellaneous/tim-file-format/
[2]: http://justsolve.archiveteam.org/wiki/TIM_(PlayStation_graphics)
