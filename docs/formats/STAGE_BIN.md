# STAGE_BIN — Stage sound-bank and thumbnail container files

Files (three related but distinct flavours):

| Directory  | Pattern        | Count | Per-file size      | Role                                       |
|------------|----------------|-------|--------------------|--------------------------------------------|
| `LOADSE/`  | `STAGE00..09`  | 10    | 128 KB .. 242 KB   | 6 VAB packs + billboard list               |
| `LOADSE1/` | `STAGE10..31`  | 22    | 127 KB .. 259 KB   | Same (continuation)                        |
| `disc/LOADSE1/STAGE32..37.BIN` | 6 | 132–222 KB | Continuation (final stages)        |
| `U_PIC/`   | `STG00..29`    | 30    | exactly 19,240 B   | Compact "select-screen" stage thumbnails   |
| `TIM2D/`   | `STG30..31`    | 2     | 19,240 B           | Same compact layout (extras)               |

(Total: 38 STAGE\*.BIN + 32 STG\*.BIN = 70 stage-related BIN files.)

The two formats share no on-disc structure. STAGE\*.BIN carry the sound banks and
billboard list for one playable arena; STG\*.BIN carry a tiny pair of
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

## 2. STAGE\*.BIN — per-stage sound banks + billboard list (LOADSE + LOADSE1)

A 52-byte header (13 little-endian u32 words), six VAB packs back to back,
then a billboard list. No geometry or textures: every record is a VAB pack
with exactly the shape of an `.SE` file ([SE.md](SE.md)).

### Loader (`snd_ReloadListedVabsFromFileAndInstallBillboards`, `src/main/4BC3C.c`)

The stage load in `src/main/9F9C.c` calls `snd_ReloadListedVabsFromFileAndInstallBillboards(buf, stage, a, b)`
with `a` and `b` read from `D_8008E5A8` for the two fighter slots. It
reads group-2 file `stage + 9` (`cdrom_GetFileIndex(2, stage + 9)`) into
`buf`, rebases header word 12 and passes it to `eff_SetBillboardList`
(`src/main/51268.c`), then hands record 0, record `a + 1` and record `b + 1`
(only two packs when `a == b`) to `snd_LoadVabPack` for the three VAB ids in
`g_vab_id_list`, compacting the loaded packs to the start of `buf`.

### Header (offset 0x00, 52 bytes)

| Offset | Size     | Field          | Meaning                                       |
|--------|----------|----------------|-----------------------------------------------|
| 0x00   | u32 × 2  | rec0 off, size | record 0 (offset always 0x34)                 |
| 0x08   | u32 × 2  | rec1 off, size | record 1                                      |
| 0x10   | u32 × 2  | rec2 off, size | record 2                                      |
| 0x18   | u32 × 2  | rec3 off, size | record 3                                      |
| 0x20   | u32 × 2  | rec4 off, size | record 4                                      |
| 0x28   | u32 × 2  | rec5 off, size | record 5                                      |
| 0x30   | u32      | billboards     | offset of the billboard list                  |

In all 38 files the records are contiguous from 0x34 and the billboard list
starts where record 5 ends.

For `STAGE00.BIN` (128,988 bytes):

```
record 0: off=0x000034  size=0xE424  VH v6, ps 1 / ts 11 / vs 7
record 1: off=0x00E458  size=0x37A8  VH v6, ps 1 / ts 10 / vs 4
record 2: off=0x011C00  size=0x36F8
record 3: off=0x0152F8  size=0x3AE8
record 4: off=0x018DE0  size=0x34F8
record 5: off=0x01C2D8  size=0x34F8
billboards (word 0x30) = 0x1F7D0  → 12 bytes: the terminator alone
```

### Records

Each record is a VAB pack: 16-byte head (event table, VH and VB offsets, VB
size), event table, VH (`pBAV`, version 6 in all 228 records), 4 zero bytes,
VB, 4 zero bytes. Layout: [SE.md](SE.md). Record 0 is the stage's own bank;
records 1..5 are the per-fighter-slot banks picked by `a` / `b` above.

### Billboard list

12-byte entries (three s32, `Unk800F1198Record`, `include/game.h`).
`eff_SetBillboardList` copies entries into `g_eff_billboards` while word 0
has bit 0 set; the first entry with bit 0 clear ends the list. On disc the
list ends with 12 zero bytes at end of file. 29 stages have no billboards
(the terminator alone); the others hold 2 or 4 entries (24 or 48 bytes)
before it. `eff_DrawBillboardList` (`src/main/51268.c`) decodes an entry as:

```
word 0   bit 0 = present, x = word0 / 2
word 1   type = word1 & 7 (0..3 pick sprite size and frames; 4..7 keep
         the previous entry's), y = word1 / 8
word 2   z
```

The next symbol after `g_eff_billboards` (0x800F1198) is at 0x800F11E0, so
the copy apparently holds at most 5 entries plus the terminator (inferred
from symbol spacing; the disc files use at most 4). `repack_stage.py`
warns when a list holds more than 5.

### Repack

`tools/repack_stage.py` splits a file into one `repack_vab.py` directory per
record plus `billboards.bin`, and rebuilds it with the header recomputed, so
a record may change size (whole 4-byte words; VAG rules in [SE.md](SE.md)):

```
python tools/repack_stage.py unpack disc/LOADSE/STAGE00.BIN out/   # rec0/..rec5/, billboards.bin
python tools/repack_stage.py pack out/ STAGE00.BIN                 # rebuild
python tools/repack_stage.py --roundtrip disc/LOADSE1              # exit 1 unless every STAGE*.BIN is byte-identical
```

All 38 files round-trip byte-identically; both directions check the
billboard list's shape. Size limits: `fight_ResetGraphAndReloadResourcesOnCachedKeyChange` (`src/main/9F9C.c`)
panics when the compacted size `snd_ReloadListedVabsFromFileAndInstallBillboards` returns reaches 0x2519. That
size is the bytes ahead of the VB in the three loaded packs
(`snd_LoadVabPack` returns the VB offset), so it grows with the event table
and VH, not with VAGs. A file that grows also has to fit its disc slot (see
the `disc-build` row in `docs/foundation/backlog.tsv`).

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
python tools/inspect_stage.py disc/TIM2D/SEL.BIN      # outer-TOC summary
python tools/inspect_stage.py disc/TIM2D/SEL.BIN --dump OUT   # segment dump
```

To recover the embedded TIM/VAB resources catalogued above, extract the
segments with `inspect_stage.py --dump`, then run `inspect_tim.py` on the TIM
segments and `inspect_bnk.py` on any VAB (`pBAV`) segment. (The per-file
findings table was produced this way — by dumping segments and validating each
embedded TIM/VAB header individually.)

---

## Inspector tool

`tools/inspect_stage.py` — list / extract records from any of the BIN
container variants.

```
python tools/inspect_stage.py FILE.BIN              auto-detect + summary
python tools/inspect_stage.py FILE.BIN --dump OUT/  extract every record
python tools/inspect_stage.py FILE.BIN --hex N      hex-dump record/segment N
python tools/inspect_stage.py DIR/                  summarise every BIN in dir
```

The tool detects:
* `STG\*.BIN` "2-TIM wrapper" by the header `08 00 00 00 48 0B 00 00`.
* `STAGE\*.BIN` by `0x34` as the first u32 plus a valid 6-pair TOC.
* Generic "5-u32 TOC" BIN (SEL/NAR/MOD/STAFF/D_SEL family) by heuristic.

It does not interpret inner VABs or TIMs — for that, extract records with
`--dump`, split `STAGE*.BIN` packs with `repack_stage.py` / `repack_vab.py`,
and feed any TIMs to `inspect_tim.py`.

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
* `STAGE*.BIN`: `tools/repack_stage.py --roundtrip` over `disc/LOADSE` and
  `disc/LOADSE1` (every record passes `repack_vab.py`'s VH / size-table
  checks; header and billboard-list checks as in §2), read against
  `snd_ReloadListedVabsFromFileAndInstallBillboards`, `eff_SetBillboardList` and `eff_DrawBillboardList`.

## Unverified / TODO

* What `D_8008E5A8` maps a fighter slot to (which of `STAGE*.BIN` records
  1..5 it loads), and what each billboard type draws.
* Semantics of the 9-u32 secondary table in the `wrap5` family — confirmed
  to vary per file (mix of sizes and offsets), needs per-loader trace.
* Schema of the raw VRAM-ready blocks in segments 3 and 4 of `D_SEL.BIN`,
  `STAFF.BIN`, and `SEL/SEL1/SEL2.BIN` (no TIM headers — direct VRAM upload
  via `LoadImage` / `LoadClut`, format implied by destination rect).
* What distinguishes `SEL1.BIN` from `SEL2.BIN` — they share the outer TOC
  and embedded sound bank but their bulk graphics segments (file offsets
  0x49565..0xB50B9) differ in ~5.6% of bytes, suggesting two select-screen
  variants for distinct game modes; runtime trace would confirm.
