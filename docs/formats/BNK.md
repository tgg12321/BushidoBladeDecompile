# BNK / VAB — Sound bank format

Files:
* `disc/TIM2D/GAME.BNK`, `GAMECHAN.BNK`, `GAMEDAN.BNK`, `GAMESTO.BNK`,
  `GAMETRA.BNK` — 5 files, each exactly 129,920 bytes.

Their content format is **not established**. Earlier notes called them raw
VAB bodies ("VB": SPU-ADPCM samples, no header), but the bytes fit that badly:
in an SPU-ADPCM block byte 1 holds flags 0..7, yet only 5,179 of GAME.BNK's
8,120 16-byte blocks (4,451..5,298 across the five files) have byte 1 <= 7, and
the files sit in `TIM2D/` with the textures. Each starts
`00 x15, 10, 32 00 ...`. No VH ("VAB header", `pBAV`) for them is known
either; it is not one of the `disc/LOADSE/*.SE` files: each of those is a self-contained VAB pack carrying its own VB
(`vb_size` 0x5E30 / 0xDF20 / 0x26F00 / 0x28050, none of them 129,920 =
0x1FB80; see [SE.md](SE.md)).

The combination of a VH and a VB is what PsyQ docs call a "VAB" — when a
single `.VAB` file is on disc, it begins with the `pBAV` magic and embeds
both.

## VAB-body (VB) layout

(The VB of BB2's VAB packs, [SE.md](SE.md).) The body is a concatenation of
up to 255 (vs is a u8) raw SPU-ADPCM sample bodies with no
in-body headers. To know which bytes belong to which sample, the matching VH
header supplies a 256-entry × 2-byte "VAG size table" where each non-zero
entry `S` means "the next sample is `S * 8` bytes long".

For each sample the SPU-ADPCM format is a stream of 16-byte blocks:

| Offset | Field         |
|--------|--------------|
| 0      | shift / filter byte (high nibble = filter idx 0..4, low nibble = shift 0..12) |
| 1      | flags: bit-0=loop end, bit-1=loop, bit-2=loop start |
| 2..15  | 28 ADPCM nibbles (14 bytes), two 4-bit samples per byte |

A "loop end" flag terminates the sample. The VAG size table's entry 0 is 0
in every `.SE` VH, so a VB starts with VAG 1.

## VH (VAB header) layout

See `pBAV` reference summary below; full spec at
[ProblemKaputt PSXSPX][1] and [PsyQ SDK `Filefrmt.pdf`][2]. In BB2 it sits
inside the VAB packs (`.SE` files, `STAGE*.BIN` records; [SE.md](SE.md)).

```
+0x00  char   id[4]            = "pBAV"
+0x04  u32    version          = 6 (the .SE files) or 7 (e.g. MOD.BIN, SEL*.BIN)
+0x08  u32    vab_id           usually 0
+0x0C  u32    fsize            VH+VB combined size in bytes
+0x10  u16    reserved         = 0xEEEE
+0x12  u16    ps               number of programs (instruments)
+0x14  u16    ts               total tones across all programs
+0x16  u8     vs               number of VAG samples (size-table entries 1..vs)
+0x17  u8     pad
+0x18  u8     mvol             master volume (typically 0x7F)
+0x19  u8     pan              master pan
+0x1A  u8     attr1            user
+0x1B  u8     attr2            user
+0x1C  u32    reserved         = 0xFFFFFFFF
+0x20  u8[0x800]  ProgAttr[128]  16 bytes per program
+0x820 u8[ps*0x200] ToneAttr[]   16 tones x 32 bytes per program
+...   u16[256]   vag_size_table  size of each VAG in 8-byte units (entry 0 unused)
```

The VH's size is 0x20 + 0x800 + ps * 0x200 + 0x200; the VB follows it (in a
`.VAB`) or sits at the pack's VB offset (in a BB2 VAB pack), and the
`vag_size_table` slices it.

## BB2 SE files — VAB packs

`disc/LOADSE/*.SE` are VAB packs: a 16-byte head (event-table, VH and VB
offsets, VB size), an event table, the VH and the VB. Layout, per-file
values and the repack tool: [SE.md](SE.md).

## Inspector tool

`tools/inspect_bnk.py` summarises a BNK body (block count, first ADPCM
headers). Its `.SE` mode predates [SE.md](SE.md): it reports a "sub_TOC
count", prints ps + 1 and vs + 1 as the program and VAG counts, and `--vh`
writes `fsize` bytes from the VH offset (in a pack: VH, gap, most of the VB). Use `tools/repack_vab.py unpack` for `.SE`
files.

```
python tools/inspect_bnk.py file.bnk           summary (block count, first headers)
python tools/inspect_bnk.py DIR/               list every BNK (and .SE, old reading) in DIR
```

## Verified by

* All 5 BNK files are 129,920 bytes; the byte-1 count above is over every
  16-byte block of each file.
* Search for `pBAV` magic in all 4 `*.SE` files yields exactly one hit per
  file, at the VH offset in the pack head (word 1); the bytes that follow
  match the documented VH layout.
* Layout cross-referenced against [ProblemKaputt PSXSPX VAB docs][1] and
  [PsyQ SDK `Filefrmt.pdf`][2] (the official Sony reference).

## Unverified / TODO

* What the BNK files hold (sample data or something else), and which code
  loads them.
* Whether the five same-size BNKs are variants of one thing.

## References

[1]: https://problemkaputt.de/psxspx-cdrom-file-audio-sample-sets-vab-and-vh-vb-sony.htm
[2]: https://psx.arthus.net/sdk/Psy-Q/DOCS/Devrefs/Filefrmt.pdf
