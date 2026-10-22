# SE — VAB pack (sound bank with its event table)

Files: `disc/LOADSE/JADGE.SE`, `NINJA_C.SE`, `OTHER_C.SE`, `SYSTEM.SE` (4
files).

An `.SE` file is one **VAB pack** followed by 4 zero bytes. The pack is the
unit `snd_LoadVabPack`
(`src/main/4BC3C.c`) loads: a 16-byte head (`Unk8005C2A8Pack`,
`include/game.h`), an event table, a VAB header (VH, `pBAV`) and the VAB body
(VB). Every `STAGE\*.BIN` record has exactly the shape of an `.SE` file (record
0 of `STAGE00.BIN` starts at 0x34), and `MOD.BIN`'s head pointer leads to one
(`STAGE*.BIN` container and repack tool:
[STAGE_BIN.md](STAGE_BIN.md)). VH and VB layout: [below](#vh-and-vb).

## Layout

```
+0x00  u32  events_off   file-relative offset of the event table (0x10 in every .SE)
+0x04  u32  vh_off       file-relative offset of the VH (pBAV)
+0x08  u32  vb_off       file-relative offset of the VB
+0x0C  u32  vb_size      VB size in bytes (the bank's size in SPU memory)
+0x10  u32[]             event table, up to vh_off (snd_FlushSoundQueue reads it)
+vh_off                  VH: 0x20 + 0x800 + ps * 0x200 + 0x200 bytes
                         (header, 128 program slots, 16 tones per program,
                         256-entry u16 VAG size table)
                         4 zero bytes
+vb_off                  VB: VAG 1..vs back to back, VAG n = table[n] * 8 bytes
                         4 zero bytes (end of file)
```

`snd_LoadVabPack` adds the pack's address to the first three words in place,
then `snd_VabOpen` passes the VH to `SsVabOpenHeadSticky` and the VB to
`SsVabTransBody`; `unk_0C` (vb_size) advances the sticky SPU address for the
next bank. In all four files the VAG size table's entry 0 and every entry
past `vs` are 0, the entries sum to `vb_size`, and the VH's `fsize` equals
VH size + `vb_size`.

| File        | size    | vh_off | vb_off | vb_size | ps / ts / vs |
|-------------|--------:|-------:|-------:|--------:|--------------|
| `SYSTEM.SE` |  27,260 | 0x24   | 0xC48  | 0x5E30  | 1 / 5 / 5    |
| `NINJA_C.SE`|  60,312 | 0x50   | 0xC74  | 0xDF20  | 1 / 13 / 7   |
| `JADGE.SE`  | 163,204 | 0x5C   | 0xE80  | 0x26F00 | 2 / 19 / 19  |
| `OTHER_C.SE`| 167,672 | 0x80   | 0xEA4  | 0x28050 | 2 / 32 / 17  |

(`ps` = programs, `ts` = tones, `vs` = VAGs; `SsVabOpenHeadWithMode`,
`src/main/psxsdk/libsnd/vs_vh.c`, reads `ps` and `vs`, and `ts` equals the sum
of the programs' tone counts.)

## VH and VB

The combination of a VH and a VB is what PsyQ docs call a "VAB" — when a
single `.VAB` file is on disc, it begins with the `pBAV` magic and embeds
both.

### VH (VAB header)

Full spec at [ProblemKaputt PSXSPX][1] and [PsyQ SDK `Filefrmt.pdf`][2].

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

### VB (VAB body)

The body is a concatenation of
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

## Naming convention

* `SYSTEM.SE` — system / UI sound effects (menus, options).
* `NINJA_C.SE` — character-specific SFX for the ninja class.
* `OTHER_C.SE` — character-specific SFX for other classes.
* `JADGE.SE` — "Judge" sound effects (likely round-end / referee voice).

## Tools

* `tools/repack_vab.py` splits a pack and rebuilds it (`tools/inspect_bnk.py`'s
  `.SE` mode predates this reading: it reports a "sub_TOC count", prints
  ps + 1 and vs + 1 as the program and VAG counts, and `--vh` writes
  `fsize` bytes from the VH offset, i.e. VH, gap and most of the VB):

```
python tools/repack_vab.py unpack disc/LOADSE/JADGE.SE out/   # events.bin, vh.bin, vag_NNN.adpcm, pack.json
python tools/repack_vab.py pack out/ JADGE.SE                 # rebuild
python tools/repack_vab.py --roundtrip disc/LOADSE            # exit 1 unless every .SE is byte-identical
```

VAGs are raw SPU-ADPCM (no `VAGp` header). `pack` recomputes the head
offsets, the VAG size table and `fsize`, so a VAG may change size (whole
16-byte blocks); the VAG count must stay `vs`, and `events.bin` and the gap
must keep the VH and VB 4-byte aligned. Growing a bank can overflow SPU RAM
(`SsVabOpenHeadWithMode` refuses one whose SPU address plus size passes
0x80000). All four `.SE` files round-trip byte-identically.

## Verified by

* The head words of all four files and the checks above (script over
  `disc/LOADSE/*.SE`; the same checks run in `repack_vab.py unpack`).
* `snd_LoadVabPack` / `snd_VabOpen` (`src/main/4BC3C.c`) and the
  `Unk8005C2A8Pack` comment in `include/game.h`.
* One `pBAV` magic per `.SE` file, at the VH offset in the pack head; VH
  layout cross-referenced against [ProblemKaputt PSXSPX VAB docs][1] and
  [PsyQ SDK `Filefrmt.pdf`][2].

## References

[1]: https://problemkaputt.de/psxspx-cdrom-file-audio-sample-sets-vab-and-vh-vb-sony.htm
[2]: https://psx.arthus.net/sdk/Psy-Q/DOCS/Devrefs/Filefrmt.pdf
