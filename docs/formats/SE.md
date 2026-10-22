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
[STAGE_BIN.md](STAGE_BIN.md)). VH field layout:
`docs/formats/BNK.md`.

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

## Naming convention

* `SYSTEM.SE` — system / UI sound effects (menus, options).
* `NINJA_C.SE` — character-specific SFX for the ninja class.
* `OTHER_C.SE` — character-specific SFX for other classes.
* `JADGE.SE` — "Judge" sound effects (likely round-end / referee voice).

## Tools

* `tools/repack_vab.py` splits a pack and rebuilds it (`tools/inspect_bnk.py`'s
  `.SE` mode uses the old reading; see [BNK.md](BNK.md)):

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
