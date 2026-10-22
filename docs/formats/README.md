# BB2 Asset Formats — Index

Reverse-engineered specifications for every binary asset format shipped on
the Bushido Blade 2 (SLUS-00663) disc.

## File-format catalogue

| Format     | Spec                  | Inspector tool             | Status                | Files on disc            |
|------------|-----------------------|----------------------------|-----------------------|--------------------------|
| **NDATA**  | [NDATA.md](NDATA.md)  | `tools/inspect_ndata.py`, `tools/extract_ndata_names.py`, `tools/repack_ndata.py` (round-trip) | Fully reversed; file-ID -> name map in `ndata_filemap.csv` | `disc/NDATA/NDATA.{DAT,INF}` |
| **BBM**    | [BBM.md](BBM.md)      | `tools/inspect_bbm.py`     | Partially reversed (containers, not section-5 keyframe encoding) | `disc/MOTION/*.BBM` |
| **WIN.DAT**| [BBM.md#WIN.DAT](BBM.md#windat) | `tools/inspect_windat.py` | Structure reversed (27×0x3800 records, i16 fixed-point); per-frame schema TBD | `disc/MOTION/WIN.DAT` |
| **TIM**    | [TIM.md](TIM.md)      | `tools/inspect_tim.py`, `tools/repack_tim.py` (PNG round-trip) | Standard PSX format — fully implemented (PNG export) | `disc/TIM2D/*.TIM`, `disc/STR/TITLE.TIM` |
| **STAGE_BIN** | [STAGE_BIN.md](STAGE_BIN.md) | `tools/inspect_stage.py`, `tools/repack_stage.py` (`STAGE*.BIN` round-trip) | Container reversed (incl. wrap5 family TIM/VAB embeds); `STAGE*.BIN` = 6 VAB packs ([SE.md](SE.md)) + billboard list | `disc/{LOADSE,LOADSE1}/STAGE*.BIN`, `disc/U_PIC/STG*.BIN`, `disc/TIM2D/STG*.BIN`, plus SEL/NAR/MOD/STAFF/D_SEL BINs |
| **BNK**    | [BNK.md](BNK.md)      | `tools/repack_bnk.py` (round-trip, via TIM) | Reversed: raw VRAM data for two rects at x = 896, effect textures and more (not sound) | `disc/TIM2D/GAME*.BNK`   |
| **SE**     | [SE.md](SE.md)        | `tools/repack_vab.py` (round-trip) | VAB pack reversed (head, event table, VH, VB; = one STAGE record) | `disc/LOADSE/*.SE` |
| **STR**    | [STR.md](STR.md)      | `tools/inspect_str.py`     | Standard PSX format — documented and inspected | `disc/STR/OPENING.STR` |
| **MOVOVL** | [MOVOVL.md](MOVOVL.md)| `tools/inspect_movovl.py`  | PS-X EXE overlay characterised (header, BIOS, libs); decoder internals from PsyQ library | `disc/STR/MOVOVL.EXE` |
| **XA**     | [XA.md](XA.md)        | (external tools)           | Standard PSX format — documented | `disc/XA_{0,1}/*.XA`, `disc/LOADSE/BB2_M*.XA` |

## Bird's-eye view of the disc

```
disc/
├── SLUS_006.63                  PSX main EXE (606 KB)
├── SYSTEM.CNF                   PSX boot config
├── NDATA/
│   ├── NDATA.DAT (55 MB)        Primary packed archive (901 entries)
│   └── NDATA.INF (3.6 KB)       Sector directory
├── MOTION/
│   ├── COMMON.BBM (162 KB)      System animations (MC magic)
│   ├── {GN,GS,K123,...}.BBM     Per-character motion bundles (MW magic)
│   └── WIN.DAT (378 KB)         Victory-pose tables: 27 x 0x3800 fixed-stride records
├── LOADSE/                      Stage 00–09 bundle + system audio
│   ├── STAGE{00..09}.BIN        6-record container per stage
│   ├── BB2_M{1,2,3}.XA          Music banks
│   └── {SYSTEM,JADGE,NINJA_C,OTHER_C}.SE   SFX banks with embedded VH
├── LOADSE1/                     Stage 10–37 continuation
│   └── STAGE{10..37}.BIN
├── TIM2D/
│   ├── {LAS_*,LINK,LINK_M,SEA2_16}.TIM     Textures
│   ├── GAME{,CHAN,DAN,STO,TRA}.BNK         VRAM blocks, not sound (BNK.md)
│   ├── STG{30,31}.BIN                      Stage-thumbnail TIM containers
│   └── {SEL,SEL1,SEL2,D_SEL,MOD,NAR,STAFF}.BIN   Container files
├── U_PIC/
│   └── STG{00..29}.BIN          Per-stage select-screen thumbnails
├── XA_0/, XA_1/                 In-game music streams (MAR_XA*.XA)
└── STR/
    ├── OPENING.STR (40 MB)      Studio logo / opening cinematic
    ├── TITLE.TIM (150 KB)       Title screen (320×240 16-bit)
    └── MOVOVL.EXE (122 KB)      MDEC playback overlay (standalone PS-X EXE)
```

## Disc image and rebuild

The retail image (`Bushido Blade 2 (USA).bin` + `.cue`) is one MODE2/2352
data track of 298,348 sectors: ISO 9660 with XA extensions, each directory
sector just ahead of its files, the 162 files back to back, then 150 empty
sectors (header only). Plain data files use Form 1 sectors with subheader
submode 0x08 (0x89 on a file's last sector); the `.XA` files are Form 2
audio streams and `STR/OPENING.STR` is real-time video sectors.

The game finds files through `g_cd_file_table` in the EXE (159
`CdFileEntry {CdlLOC, u32 size}` entries, [file_io.md](../engine/file_io.md)),
not through the ISO directory; every disc file has an entry except
`SLUS_006.63`, `SYSTEM.CNF` and `TIM2D/LAS_DD.TIM`.

`tools/build_disc.py` builds a modded image from the retail one. Replaced
files may change size: one that still fits its sectors is written in place,
a bigger one moves to the end of the files (before the 150 empty sectors).
For each file that moves or changes size it rewrites the ISO directory
record and the file's table entry in the EXE that goes on the disc (found
by its old 8 bytes, so a rebuilt or shifted EXE works as long as its
table still describes the input image; an entry it cannot find is an
error), grows the volume size, and writes every touched sector with its
address, subheader and EDC/ECC. Only plain data files can be replaced.
A 74-minute CD-R holds 333,000 sectors, 34,652 more (about 71 MB of
data) than the retail image.

```
python tools/build_disc.py --image "Bushido Blade 2 (USA).bin" --out mod.bin --verify   # no-op: must equal the retail image
python tools/build_disc.py --image "Bushido Blade 2 (USA).bin" --out mod.bin \
    --exe build/bb2.exe --dir mods/        # mods/TIM2D/GAME.BNK replaces TIM2D/GAME.BNK, etc.
```

Checked: the no-op build is byte-identical to the retail image; the sector
writer reproduces retail sectors exactly; a build that grows two files and
shrinks one re-parses with every file's bytes, sector EDC/ECC and
subheaders, directory records and all 159 table entries consistent, and
only table bytes changed in the EXE. Not yet run in an emulator.
Per-format limits still apply: `NDATA.INF` is read as 2 sectors (the tool
refuses one over 4,096 bytes), and `STAGE*.BIN` has its own
([STAGE_BIN.md](STAGE_BIN.md)).

## Unverified / not yet documented

* Internal byte encoding of BBM section 5 (the bulk packed keyframe
  payload) — requires Marionation runtime reverse-engineering.
* Inner sub-section layout of SEL/NAR/MOD/STAFF/D_SEL containers beyond the
  5-u32 outer TOC.

## Tools summary

All inspectors live in `tools/inspect_<format>.py` and follow the same UX
conventions:
* Run on a single file → print a summary.
* Run on a directory → tabulate every matching file inside it.
* `--hex N` / `--dump OUT` flags to extract or hex-dump specific records.
* `--png OUT` for TIM (image export via Pillow, if available).

A top-level driver `tools/unpack_all.py` runs the appropriate inspector on
every BB2 asset file and prints a unified asset-inventory summary.

## How this was reverse-engineered

For every format the reversing workflow was:

1. **Hex inspect** representative files to find magic bytes / header
   constants.
2. **Cross-reference** against source code (`src/*.c`, `asm/funcs/*.s`) to
   identify loader functions and confirm field interpretations.
3. **Sanity-check** by parsing every file on disc with the proposed schema
   and verifying counts / offsets are consistent.
4. **Cite evidence** in the docs: each verified claim is followed by either
   a hex snippet, a source line reference, or both.

Where formats are PSX-SDK standard (TIM, VAB, STR, XA) the external
specification was fetched and cited; per-format docs note any
BB2-specific quirks (e.g. `TITLE.TIM`'s anomalous `block_length`).
