# BB2 Asset Formats — Index

Reverse-engineered specifications for every binary asset format shipped on
the Bushido Blade 2 (SLUS-00663) disc.

## File-format catalogue

| Format     | Spec                  | Inspector tool             | Status                | Files on disc            |
|------------|-----------------------|----------------------------|-----------------------|--------------------------|
| **NDATA**  | [NDATA.md](NDATA.md)  | `tools/inspect_ndata.py`, `tools/extract_ndata_names.py` | Fully reversed; file-ID -> name map in `ndata_filemap.csv` | `disc/NDATA/NDATA.{DAT,INF}` |
| **BBM**    | [BBM.md](BBM.md)      | `tools/inspect_bbm.py`     | Partially reversed (containers, not section-5 keyframe encoding) | `disc/MOTION/*.BBM` |
| **WIN.DAT**| [BBM.md#WIN.DAT](BBM.md#windat) | `tools/inspect_windat.py` | Structure reversed (27×0x3800 records, i16 fixed-point); per-frame schema TBD | `disc/MOTION/WIN.DAT` |
| **TIM**    | [TIM.md](TIM.md)      | `tools/inspect_tim.py`     | Standard PSX format — fully implemented (PNG export) | `disc/TIM2D/*.TIM`, `disc/STR/TITLE.TIM` |
| **STAGE_BIN** | [STAGE_BIN.md](STAGE_BIN.md) | `tools/inspect_stage.py`, `tools/scan_container.py` | Container reversed (incl. wrap5 family TIM/VAB embeds); inner Marionation geometry partial | `disc/{LOADSE,LOADSE1}/STAGE*.BIN`, `disc/U_PIC/STG*.BIN`, `disc/TIM2D/STG*.BIN`, plus SEL/NAR/MOD/STAFF/D_SEL BINs |
| **BNK**    | [BNK.md](BNK.md)      | `tools/inspect_bnk.py`     | Reversed (VAB body)   | `disc/TIM2D/GAME*.BNK`   |
| **SE**     | [SE.md](SE.md)        | (uses `inspect_bnk.py` / `inspect_stage.py`) | Container reversed (= one STAGE record) | `disc/LOADSE/*.SE` |
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
│   ├── GAME{,CHAN,DAN,STO,TRA}.BNK         VAB sample-bank bodies
│   ├── STG{30,31}.BIN                      Stage-thumbnail TIM containers
│   └── {SEL,SEL1,SEL2,D_SEL,MOD,NAR,STAFF}.BIN   Container files
├── U_PIC/
│   └── STG{00..29}.BIN          Per-stage select-screen thumbnails
├── XA_0/, XA_1/                 In-game music streams (MAR_XA*.XA)
└── STR/
    ├── OPENING.STR (40 MB)      Studio logo / opening cinematic
    ├── TITLE.TIM (150 KB)       Title screen (320×240 16-bit)
    └── MOVOVL.EXE (120 KB)      MDEC playback overlay (standalone PS-X EXE)
```

## Unverified / not yet documented

* Internal byte encoding of BBM section 5 (the bulk packed keyframe
  payload) — requires Marionation runtime reverse-engineering.
* Per-record Marionation geometry inside `STAGE\*.BIN` records (after the
  VAB body).
* Inner sub-section layout of SEL/NAR/MOD/STAFF/D_SEL containers beyond the
  5-u32 outer TOC.
* Whether the 5 BNK files share VAGs or hold completely distinct samples.

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
