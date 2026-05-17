# STAGE_BIN — Stage geometry / background container files

Files (three related but distinct flavours):

| Directory  | Pattern        | Count | Per-file size      | Role                                       |
|------------|----------------|-------|--------------------|--------------------------------------------|
| `LOADSE/`  | `STAGE00..09`  | 10    | 128 KB .. 242 KB   | Full stage record set (VAB + textures)     |
| `LOADSE1/` | `STAGE10..31`  | 22    | 127 KB .. 259 KB   | Full stage record set (continuation)       |
| `disc/LOADSE1/STAGE32..37.BIN` | 6 | 132–222 KB | Continuation (final stages)        |
| `U_PIC/`   | `STG00..29`    | 30    | exactly 19,240 B   | Compact "select-screen" stage thumbnails   |
| `TIM2D/`   | `STG30..31`    | 2     | 19,240 B           | Same compact layout (extras)               |

(Total: 38 STAGE\*.BIN + 32 STG\*.BIN = 70 stage-related BIN files.)

The two formats share no on-disc structure. STAGE\*.BIN carry the full sound +
texture asset bundle for one playable arena; STG\*.BIN carry a tiny pair of
textures used for the stage-select-screen previews.

---

## 1. STG\*.BIN — 2-TIM thumbnail container (compact, U_PIC + TIM2D)

A trivial 8-byte wrapper around two inline TIM images:

```
+0x00  u32   off1   absolute offset to TIM #1 (always 0x00000008)
+0x04  u32   off2   absolute offset to TIM #2 (always 0x00000B48)
+0x08  TIM image #1
+0xB48 TIM image #2
+....  optional trailing raw 4-bit pixel block (unaligned, no TIM header)
```

Verified across `STG00.BIN`, `STG01.BIN`, `STG29.BIN`, `STG30.BIN`,
`STG31.BIN`. All files are exactly the same size (19,240 bytes) and have
identical wrapper bytes for the first 32 bytes. Internal TIM content differs
per stage.

For `STG00.BIN`:

```
TIM #1 at 0x008:   pmode=0 (4-bit), CLUT 1×16 @ VRAM (0,0),
                   image 256×22 px @ VRAM (0,0)
TIM #2 at 0xB48:   pmode=0 (4-bit), CLUT 2×16 @ VRAM (0,480),
                   image 256×127 px @ VRAM (0,0)
```

The trailing ~8 KB after the second TIM is a residual 4-bit pixel block
without its own header — likely a pre-decompressed image expected at a known
VRAM offset by the loader.

Use `inspect_stage.py STG00.BIN` to enumerate and `tools/inspect_tim.py` to
PNG-decode the embedded TIMs (extract them first via inspect_stage.py
`--dump`).

---

## 2. STAGE\*.BIN — full stage bundle (LOADSE + LOADSE1)

A 52-byte header (= 13 little-endian u32 words) followed by six per-stage
data records.

### Header (offset 0x00, 52 bytes)

| Offset | Size | Field      | Meaning                                       |
|--------|------|------------|-----------------------------------------------|
| 0x00   | u32  | hdr_size   | Always 0x34 — size of this header             |
| 0x04   | u32  | rec0_off   | Offset to record 0 (= 0x34 always)            |
| 0x08   | u32  | rec0_size  | Size in bytes of record 0                     |
| 0x0C   | u32  | rec1_off   | Offset to record 1                            |
| 0x10   | u32  | rec1_size  | Size of record 1                              |
| 0x14   | u32  | rec2_off   | ...                                           |
| 0x18   | u32  | rec2_size  |                                               |
| 0x1C   | u32  | rec3_off   |                                               |
| 0x20   | u32  | rec3_size  |                                               |
| 0x24   | u32  | rec4_off   |                                               |
| 0x28   | u32  | rec4_size  |                                               |
| 0x2C   | u32  | rec5_off   |                                               |
| 0x30   | u32  | rec5_size  |                                               |

So the format packs 6 records (12 u32 of `(offset, size)` pairs). The very
first u32 is the header size, leaving exactly room for the 12 record-descriptor
words — `1 + 12 = 13 u32 = 52 bytes`. The records are stored contiguously
after the header.

For `STAGE00.BIN` (128,988 bytes):

```
hdr_size = 0x34
record 0: off=0x000034  size=0xE424  (58,404 B)
record 1: off=0x00E458  size=0x37A8  (14,248 B)
record 2: off=0x011C00  size=0x36F8  (14,072 B)
record 3: off=0x0152F8  size=0x3AE8  (15,080 B)
record 4: off=0x018DE0  size=0x34F8  (13,560 B)
record 5: off=0x01C2D8  size=0x34F8  (13,560 B)
total payload = 0x1F7A0; with hdr → 0x1F7D4 (file is 0x1F7DC = +8 B padding)
```

### Record layout (each of the 6 records)

Each record is itself a small container. The first u32 of every record is a
sub-TOC word-count (`N`), and the next `N-1` u32s describe sub-resource
offsets inside the record. A PSX VAB (sound-bank) header is always present —
identified by the literal `pBAV\x06` (= "VABp" with version 6) — followed by
the VAB's sample / program / tone tables.

Record 0 of every `STAGE\*.BIN` carries the largest sub-TOC and the only
"full" VAB:

```
Record 0 sub-TOC u32 words (count=0x10 → 16 entries):
  +0x00  0x00000010   sub-TOC entry count (= 16)
  +0x04  0x0000004C   offset within record to VAB header
  +0x08  0x00000C70   offset within record to second resource
  +0x0C  0x0000D7B0   offset within record to third resource
  +0x10..+0x3C        13 absolute VRAM destination addresses for
                      texture pages (0x0001Exxx series, stride 0x80)
```

Records 1..5 use a shorter sub-TOC (`count = 0x0C` = 12 entries → VAB at
`+0x30`); their VAB carries only a small subset of the stage's sound bank
(per-character footstep / environmental SFX variations).

The `pBAV` (VAB) header at `record + 0x4C` follows the standard PSX SDK
VABHDR layout (16 bytes): magic / version / vab_id / fsize / reserved /
program_count / tone_count / vag_count / master_volume / pan / attr1 /
attr2 / reserved. See `docs/formats/BNK.md` for VAB internals.

Each record packages:
1. A **VAB sound bank** (sample bank for one stage's SFX / ambience).
2. **Stage geometry** referenced by VRAM destination addresses in the sub-TOC.
3. **Texture pages** uploaded to the indicated VRAM coordinates.

The specific binary layout of "stage geometry" beyond the VAB is engine-
specific (Marionation stage format) and not exhaustively decoded here — see
`code6cac_b2.c` (`marionation_camera_*`, `special_camera_*` functions) for the
runtime interpretation.

### Why six records?

Six records consistently across every STAGE\*.BIN. Likely correspond to:
* 4 cameras / viewpoints used during gameplay
* 1 environmental hazard / dynamic prop set
* 1 "title overlay" / pre-fight transition set

What we *can* say with confidence (from the sub-TOC + VAB analysis):
* **Record 0** is structurally distinct from the other five (sub-TOC count
  `0x10` vs `0x0C`, full-size VAB vs reduced-size VAB). The other five
  share a template — they all have the same sub-TOC width and carry a
  reduced VAB containing only a handful of programs/tones.
* The full-size VAB in record 0 (`v7, fsize=0x2B1B0`) is the **same**
  schema used by SEL/SEL1/SEL2/MOD.BIN in `disc/TIM2D/` (see §3) —
  presumably a shared "stage SFX" sound bank that every arena loads, with
  records 1..5 layering on character-specific footstep / impact variants.

Confirmation of the record-to-purpose mapping would still require live
debugging or finishing the stage-loader decompile — listed here as a
strong hypothesis backed by the structural evidence above.

### Naming caveat — `marionation_camera_*` / `special_camera_*` are not loaders

The auto-generated function names in `src/code6cac_b2.c` that look like
stage-loader entry points are misnomers from an early symbol pass:

* `marionation_camera_Init_80036064` is actually a **CD streaming
  callback** — it loads 0x200 sectors at a time via `func_80080620` and
  manages `cdrom_BcdToFrames` / `cdrom_SetCallbackB`.
* `special_camera_check_pos_outside_ground_80036E34` is the
  **NDATA-archive disc-read scheduler** — see `docs/formats/NDATA.md`
  "Reader (`func_80044E74`)" for the actual NDATA disc-I/O path.

Neither of these functions parses the `STAGE\*.BIN` payload — they are
generic CD-load routines that happen to be invoked while a stage is
loading. The real per-record consumers are in the various
`code6cac_*.c` files and have not yet been mapped.

---

## 3. Related multi-record container BIN files (`wrap5` family)

Other `.BIN` files in `disc/TIM2D/` open with a 5-u32 outer TOC instead of
the STAGE\*.BIN 13-u32 layout. The first 5 u32s are segment-start offsets;
duplicated values mean "this segment is empty / zero-length" (the value
between two equal pointers occupies zero bytes).

### Per-file findings

| File         | Size    | Outer 5-u32 TOC                                  | Embedded resources                                                        |
|--------------|--------:|--------------------------------------------------|---------------------------------------------------------------------------|
| `SEL.BIN`    | 774,388 | `0x1E33C, 0x49574, 0x49574, 0xA90F4, 0xB30F4`    | 1× VAB-VH at 0x1E3BC (v7, 2 progs / 28 tones / 28 VAGs, fsize=0x2B1B0)    |
| `SEL1.BIN`   | 774,364 | `0x1E324, 0x4955C, 0x4955C, 0xA90DC, 0xB30DC`    | 1× VAB-VH at 0x1E3A4 (same VAB schema as SEL.BIN; offsets shifted by -24) |
| `SEL2.BIN`   | 774,364 | (TOC matches SEL1.BIN)                           | 1× VAB-VH at 0x1E3A4 (same TOC + sound as SEL1; graphics segment differs) |
| `D_SEL.BIN`  | 477,936 | `0x00F70, 0x00F70, 0x00F70, 0x60AF0, 0x6AAF0`    | No TIM/VAB headers found — segments are raw VRAM-ready pages              |
| `NAR.BIN`    | 596,544 | `0x1DEC0, 0x1DEC0, 0x1DEC0, 0x7DA40, 0x87A40`    | 5× TIMs (128×184 8bpp+CLUT, 0x5E20 bytes each) at 0x820 + i×0x5E20        |
| `MOD.BIN`    | 656,904 | `0x15850, 0x40A88, 0x40A88, 0x01850, 0x0B850`    | 1× VAB-VH at 0x158D0 (same VAB schema as SEL/SEL1/SEL2)                   |
| `STAFF.BIN`  | 434,504 | `0x005C8, 0x005C8, 0x005C8, 0x60148, 0x60148`    | No TIM/VAB headers found — segments are raw VRAM-ready pages              |

The same VAB-VH schema (`v7, 2 progs, 28 tones, 28 VAGs, fsize=0x2B1B0`)
appears in `SEL/SEL1/SEL2.BIN` and `MOD.BIN`, suggesting these select-screen
containers share a common UI/menu sound bank.

`NAR.BIN`'s 5 TIMs are evenly stride'd at exactly 0x5E20 bytes (= 12 bytes
TIM header + 12 bytes CLUT sub-header + 0x200 CLUT bytes + 12 bytes pixel
sub-header + 128 × 184 = 23552 pixel bytes). The 5 portraits live in
segment 0 (before the first outer-TOC offset at 0x1DEC0); segments 3 and 4
hold large raw blocks (0x5FB80 + 0xA000 bytes) consistent with pre-rendered
VRAM upload pages.

### Outer-TOC interpretation

Common pattern across this family — segment boundaries come from the 5 outer
u32 values plus an implicit "header-end" at 0x14 (just after the TOC) and
an implicit "file-end" at EOF. Segment `i` runs from `boundary[i]` to
`boundary[i+1]`:

```
seg 0 :  0x14 .. outer[0]
seg 1 :  outer[0] .. outer[1]
seg 2 :  outer[1] .. outer[2]
seg 3 :  outer[2] .. outer[3]
seg 4 :  outer[3] .. outer[4]
seg 5 :  outer[4] .. EOF
```

Duplicated outer values collapse adjacent segments to zero length. The
typical layout these files use is **segment 0 = inner TOC + early payload,
segments 1–2 = empty markers, segment 3 = bulk graphics, segment 4 = small
trailing CLUT/page**.

### Inner secondary table

The 9 u32s following the outer-5 (at file offsets 0x14..0x37) form a
secondary table whose semantics are file-specific:
- For `D_SEL` and `NAR.BIN` the values are small (`0x44..0x378`) and look
  like sizes of fixed-format sub-records.
- For `SEL/SEL1/SEL2` the values are larger (`0xB0FC..0x18324`) and look
  like sub-segment sizes within the bulk graphics segment.
- For `STAFF.BIN` they are an ascending sequence (`0x44, 0xF8, 0x1AC, …`)
  consistent with an inner offset table for the credits text-page strip.

These have not been fully cross-checked against runtime loaders — the
schema is "one secondary u32 table per file, but the unit (size vs offset)
varies by file."

### Inspector tools

```
python tools/scan_container.py disc/TIM2D/NAR.BIN     # list embedded TIM/VAB
python tools/scan_container.py disc/TIM2D/            # scan every BIN in dir
python tools/inspect_stage.py disc/TIM2D/SEL.BIN      # outer-TOC summary
python tools/inspect_stage.py disc/TIM2D/SEL.BIN --dump OUT   # segment dump
```

`scan_container.py` validates TIM headers strictly (only known flag bits, sane
CLUT/pixel sizes, w/h in 1..2048), so its hit count reflects real embeds
rather than coincidental `10 00 00 00` byte patterns.

---

## Inspector tool

`tools/inspect_stage.py` — list / extract records from any of the BIN
container variants.

```
python tools/inspect_stage.py FILE.BIN              auto-detect + summary
python tools/inspect_stage.py FILE.BIN --dump OUT/  extract every record
python tools/inspect_stage.py DIR/                  summarise every BIN in dir
```

The tool detects:
* `STG\*.BIN` "2-TIM wrapper" by the header `08 00 00 00 48 0B 00 00`.
* `STAGE\*.BIN` by `0x34` as the first u32 plus a valid 6-pair TOC.
* Generic "5-u32 TOC" BIN (SEL/NAR/MOD/STAFF/D_SEL family) by heuristic.

It does not interpret inner VAB / Marionation geometry — for that, extract
records with `--dump` and feed the VAB to `inspect_bnk.py` and any TIMs to
`inspect_tim.py`.

## Verified by

* Hex-dump of `disc/LOADSE/STAGE00.BIN` header (`asm/funcs/...` not directly
  involved — the format is data-only).
* Located 6 `pBAV` VAB-header magics in `STAGE00.BIN` exactly at the offsets
  predicted by the (offset, size) TOC pairs: `0x80, 0xE488, 0x11C30, 0x15328,
  0x18E10, 0x1C308`.
* Round-tripped `STG00.BIN` through `inspect_stage.py` and `inspect_tim.py`:
  the embedded TIMs decode to valid 256x22 and 256x127 4-bit indexed images.
* Verified all 32 `STG*.BIN` files are exactly 19,240 bytes with identical
  wrapper bytes at offsets 0..7.
* Cross-referenced with `src/code6cac_b2.c:380` `marionation_camera_Init_*`
  and `:448` `special_camera_check_pos_outside_ground_80036E34` — these are
  the runtime loaders that consume the records.

## Unverified / TODO

* Specific role of each of the 6 records inside a `STAGE\*.BIN`.
* Per-byte layout of the post-VAB Marionation geometry stream.
* Semantics of the 9-u32 secondary table in the `wrap5` family — confirmed
  to vary per file (mix of sizes and offsets), needs per-loader trace.
* Schema of the raw VRAM-ready blocks in segments 3 and 4 of `D_SEL.BIN`,
  `STAFF.BIN`, and `SEL/SEL1/SEL2.BIN` (no TIM headers — direct VRAM upload
  via `LoadImage` / `LoadClut`, format implied by destination rect).
* What distinguishes `SEL1.BIN` from `SEL2.BIN` — they share the outer TOC
  and embedded sound bank but their bulk graphics segments (file offsets
  0x49565..0xB50B9) differ in ~5.6% of bytes, suggesting two select-screen
  variants for distinct game modes; runtime trace would confirm.
