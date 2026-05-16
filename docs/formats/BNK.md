# BNK / VAB — Sound bank format

Files:
* `disc/TIM2D/GAME.BNK`, `GAMECHAN.BNK`, `GAMEDAN.BNK`, `GAMESTO.BNK`,
  `GAMETRA.BNK` — 5 files, each exactly 129,920 bytes.

These are raw **PSX VAB-body files** ("VB" in PsyQ SDK terminology) holding
the SPU-ADPCM sample data for one sound bank. The corresponding **header file**
("VH" / `pBAV`) is not stored alongside the body — it is embedded in
`disc/LOADSE/*.SE` files (see below) and selected at load time based on game
context (regular/chanbara/dan/sto/tra). The five BNKs share the same length
and layout precisely because the matching SE-file headers pre-allocate 5
identical slot layouts and the data is paged across them at runtime.

The combination of a VH and a VB is what PsyQ docs call a "VAB" — when a
single `.VAB` file is on disc, it begins with the `pBAV` magic and embeds
both. BB2 chose to keep them split (VH in `.SE`, VB in `.BNK`).

## VAB-body (VB) layout

The body is a concatenation of up to 254 raw SPU-ADPCM sample bodies with no
in-body headers. To know which bytes belong to which sample, the matching VH
header supplies a 256-entry × 2-byte "VAG size table" where each non-zero
entry `S` means "the next sample is `S * 8` bytes long".

For each sample the SPU-ADPCM format is a stream of 16-byte blocks:

| Offset | Field         |
|--------|--------------|
| 0      | shift / filter byte (high nibble = filter idx 0..4, low nibble = shift 0..12) |
| 1      | flags: bit-0=loop end, bit-1=loop, bit-2=loop start |
| 2..15  | 28 ADPCM nibbles (14 bytes), two 4-bit samples per byte |

A "loop end" flag terminates the sample. Verified in BB2's `GAME.BNK` — the
file is composed of contiguous 16-byte blocks; the first non-zero block at
offset 0x10 contains `32 00 ...` (filter=3, shift=2, no flags) — a standard
"start of sample" header.

## VH (VAB header) layout

See `pBAV` reference summary below; full spec at
[ProblemKaputt PSXSPX][1] and [PsyQ SDK `Filefrmt.pdf`][2]. The header is
present in the `.SE` files (see next section), embedded inside the per-record
sub-TOC.

```
+0x00  char   id[4]            = "pBAV"
+0x04  u32    version          = 6 (BB2) or 7
+0x08  u32    vab_id           usually 0
+0x0C  u32    fsize            VH+VB combined size in bytes
+0x10  u16    reserved         = 0xEEEE
+0x12  u16    nProgs           number of programs (instruments) - 1
+0x14  u16    nTones           total tones across all programs
+0x16  u16    nVags            number of VAG samples - 1
+0x18  u8     mvol             master volume (typically 0x7F)
+0x19  u8     pan              master pan
+0x1A  u8     attr1            user
+0x1B  u8     attr2            user
+0x1C  u32    reserved         = 0xFFFFFFFF
+0x20  u8[0x800]  ProgAttr[128]  16 bytes per program
+0x820 u8[var]    ToneAttr[]     32 bytes per tone (16 per program, max)
+...   u16[256]   vag_size_table  size of each VAG in 8-byte units
+...   raw           VB / SPU-ADPCM payload (only in single-file .VAB)
```

For BB2 the VH and VB are split. The `vag_size_table` in the VH dictates how
to slice the matching BNK body.

## BB2 SE files — embedded VH + auxiliary sound data

Files: `disc/LOADSE/JADGE.SE`, `NINJA_C.SE`, `OTHER_C.SE`, `SYSTEM.SE` (4
files); these share the same internal layout as one record from a
`STAGE\*.BIN`.

```
+0x00  u32    sub_toc_count  number of u32 entries in the sub-TOC
              (almost always 0x10 = 16 entries → sub-TOC is 64 bytes)
+0x04  u32    vab_hdr_off    offset within file of the embedded VH
+0x08  u32    aux_off1       offset to second resource (per-stage SFX VAG?)
+0x0C  u32    aux_off2       offset to third resource
+0x10..+0x3C  13 absolute VRAM destination addresses for texture/sample pages
```

For `JADGE.SE`:

```
sub_toc_count = 0x10
vab_hdr_off   = 0x5C  -> 'pBAV' magic appears at byte 0x5C
aux_off1      = 0xE80
aux_off2      = 0x26F00
```

Verified: every `*.SE` file in `disc/LOADSE/` matches this layout, with the
`pBAV` magic at the offset given by sub_TOC word[1]:

| File              | size   | vab_hdr_off | second offset | third offset |
|-------------------|--------|-------------|---------------|--------------|
| `SYSTEM.SE`       | 27,260 | 0x24        | 0xC48         | 0x5E30       |
| `NINJA_C.SE`      | 60,312 | 0x50        | 0xC74         | 0xDF20       |
| `JADGE.SE`        | 163,204| 0x5C        | 0xE80         | 0x26F00      |
| `OTHER_C.SE`      | 167,672| 0x80        | 0xEA4         | 0x28050      |

(SYSTEM.SE has its VAB at offset 0x24, which means its sub-TOC stops after
just 9 entries instead of the usual 16 — this is allowed by the format since
sub_toc_count is read explicitly.)

So `.SE` is just the BB2 file extension for "one STAGE-record extracted to a
top-level file" — the format is identical to any record found inside
`STAGE\*.BIN` (see `docs/formats/STAGE_BIN.md`).

## Why 5 identically-sized BNK files?

Each BNK is the VB body for one weapon-class sound-bank variation
("chambara" sword chop sounds, "dan" blunt impacts, "sto" stomping,
"tra" traditional combat, plus the base GAME bank). Because the matching
VH (in JADGE.SE / OTHER_C.SE / NINJA_C.SE / SYSTEM.SE) defines a fixed VAG
table, the body always allocates the same 129,920 bytes regardless of which
variation is active — variations differ only in sample content, not size.

## Inspector tool

`tools/inspect_bnk.py` — auto-detects whether the file is a VAB-body (BNK)
or a SE container (with embedded VH) and prints structure / first-block
ADPCM headers.

```
python tools/inspect_bnk.py file.bnk           summary (block count, first headers)
python tools/inspect_bnk.py file.se            detect VH, print program count
python tools/inspect_bnk.py file.se --vh OUT   extract the VH portion to file
python tools/inspect_bnk.py DIR/               list every BNK/SE in DIR
```

## Verified by

* Hex-dump of all 5 BNK files: 129,920 bytes each, first 16 bytes are zero
  (the silent "VAG #0" reserved by the VAB spec), next 16-byte ADPCM blocks
  follow the standard format with valid shift/filter bytes.
* Search for `pBAV` magic in all 4 `*.SE` files yields exactly one hit per
  file at the offset given by the file's sub_TOC word[1]; the bytes that
  follow match the documented VH layout.
* Layout cross-referenced against [ProblemKaputt PSXSPX VAB docs][1] and
  [PsyQ SDK `Filefrmt.pdf`][2] (the official Sony reference).

## Unverified / TODO

* Which BNK is loaded with which SE under what game state.
* Whether the BNK files share VAG samples or all five are completely distinct
  (the file sizes match, but sample-bank diffing would clarify).
* Layout of the auxiliary resources at sub_TOC `aux_off1` / `aux_off2` —
  appears to be VAG samples but the count and per-sample size mapping is
  unverified.

## References

[1]: https://problemkaputt.de/psxspx-cdrom-file-audio-sample-sets-vab-and-vh-vb-sony.htm
[2]: https://psx.arthus.net/sdk/Psy-Q/DOCS/Devrefs/Filefrmt.pdf
