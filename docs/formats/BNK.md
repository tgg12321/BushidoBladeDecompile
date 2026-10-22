# BNK — VRAM block (effect textures and more)

Files: `disc/TIM2D/GAME.BNK`, `GAMECHAN.BNK`, `GAMEDAN.BNK`, `GAMESTO.BNK`,
`GAMETRA.BNK` — 5 files, each exactly 129,920 bytes.

Despite the name, a BNK is not a sound bank: it is raw 16-bit VRAM data with
no header, uploaded as two rectangles in the VRAM column x = 896..1023.

## Loader (`eff_LoadImagesAndSetup`, `src/main/51268.c`)

It reads one BNK into a buffer, picked by `D_8009BD24.unk14_0` (the row
`func_80077894` stores from the main row menu), and uploads it with
`LoadImage`:

| `unk14_0` | CD table entry | File           |
|-----------|----------------|----------------|
| 0         | group 2 + 0x3C | `GAMESTO.BNK`  |
| 3         | group 2 + 0x2F | `GAMECHAN.BNK` |
| 2         | group 2 + 0x30 | `GAMETRA.BNK`  |
| 5         | group 2 + 0x31 | `GAMEDAN.BNK`  |
| other     | group 2 + 0    | `GAME.BNK`     |

## Layout

```
+0x00000  0x1DC00 bytes  VRAM rect (896, 0)   128 x 476 halfwords
+0x1DC00  0x01F80 bytes  VRAM rect (896, 476) 112 x 36 halfwords
                          (end of file = 129,920 bytes)
```

The five files are identical for their first 0x10000 bytes (VRAM rows
0..255) and differ after that. Rows 0..255 hold the effect textures: the
effect code in `src/main/51268.c` draws with texture pages 0x2E and 0x2F
(4-bit, x = 896 and 960, y = 0; e.g. `51268.c:772`, `:971`). Nothing in
that file uses a page at y = 256, so the per-file part (rows 256..475 and
the 112 x 36 strip at (896, 476)) has no known consumer yet. The old reading of these files as SPU-ADPCM sample data was wrong:
byte 1 of a 16-byte ADPCM block would be a flag value 0..7, and only
4,451..5,298 of each file's 8,120 blocks fit that.

## Tools

`tools/repack_bnk.py` writes each rect as a 16-bit TIM (no CLUT) carrying
its VRAM position, so `tools/repack_tim.py` turns it into a PNG and back,
and rebuilds the BNK from the two TIMs (their rects must stay as the loader
uploads them):

```
python tools/repack_bnk.py unpack disc/TIM2D/GAME.BNK out/   # main.tim, strip.tim
python tools/repack_tim.py decode out/main.tim main.png      # 16-bit RGBA view
python tools/repack_bnk.py pack out/ GAME.BNK                # rebuild
python tools/repack_bnk.py --roundtrip disc/TIM2D            # exit 1 unless every .BNK is byte-identical
```

A 16-bit PNG is a raw view of VRAM: 4- and 8-bit textures stored there look
scrambled in it, but editing it and encoding back is lossless. All five
files round-trip byte-identically, also through TIM -> PNG -> TIM.

`tools/inspect_bnk.py` predates this reading: its BNK mode prints ADPCM
block statistics, which do not apply.

## Verified by

* `eff_LoadImagesAndSetup` (`src/main/51268.c`): the five `cdrom_GetFileIndex`
  reads and the two `LoadImage` rects; 128 x 476 x 2 + 112 x 36 x 2 =
  129,920, the file size.
* The CD-table entries (`g_cd_file_table`, group base `D_8008F12C[2]` = 21)
  hold size 129,920 at those five indices; reading each entry's start
  sector from `Bushido Blade 2 (USA).bin` gives the file named in the table
  above, byte for byte.

## Unverified / TODO

* Who reads rows 256..511 of x = 896..1023 (the per-file part), and in
  which texture mode.
