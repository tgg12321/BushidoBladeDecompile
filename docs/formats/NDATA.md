# NDATA — Packed game-data archive

Files: `disc/NDATA/NDATA.DAT` (58,114,048 bytes) and `disc/NDATA/NDATA.INF` (3,604 bytes).

NDATA.DAT is the primary bulk-data archive for Bushido Blade 2. It stores
character animations, motion tables, stage geometry, audio sample banks, and
texture/CLUT pages in a single sector-aligned blob. NDATA.INF is a flat sparse
directory mapping a fixed file-ID (0..900) to a (start-sector, length-sectors)
pair inside NDATA.DAT.

## File layout

```
NDATA.INF :  901 x 4 byte directory entries  (3,604 bytes)
NDATA.DAT :  28,376 x 2,048-byte sectors     (58,114,048 bytes)
```

The sector size is 0x800 (2,048) — the same data sector size used by the PSX
CD-ROM filesystem. NDATA.DAT is read sector-aligned via the same disc-I/O path
used by PSX CdRead.

## NDATA.INF entry format

Each of the 901 entries is 4 bytes, little-endian:

| Offset | Size | Field           | Meaning                                          |
|--------|------|-----------------|--------------------------------------------------|
| 0      | u16  | start_sector    | Offset into NDATA.DAT in 2,048-byte sectors      |
| 2      | u16  | length_sectors  | Length in 2,048-byte sectors (rounded up)        |

Byte offset into NDATA.DAT = `start_sector * 0x800`.
Byte length on disc      = `length_sectors * 0x800`.

Unused / reserved indices are filled with the sentinel pair `0xFFFF, 0xFFFF`.
Of the 901 indices, **762** are non-sentinel entries, of which 760 pack the 28,376
sectors; 139 are sentinels: named files the build left out (138 trailing motions of
the `DATA7` characters, plus `HAND`; see the pool-to-INF rule below).

Of the 762 non-sentinel entries, 760 pack the 28,376 sectors = the entire DAT file. There
are no gaps in the actual on-disc layout; each active entry begins where the
previous active entry ended.

### Trailing entries 898..900 (sentinel + leftovers)

Indices 898, 899, 900 lie at the very end of the INF; 899 and 900 run past the
end of NDATA.DAT (sector 0x6ED8):

```
[898] start=0xFFFF  len=0xFFFF        (sentinel)
[899] start=0x9800  len=0x0308        (would address byte 0x4C00000 — past DAT end)
[900] start=0x1900  len=0x5956        (bytes 0xC80000..0x392B000: overlaps live entries, ends past DAT end)
```

These three slots appear to be footer-padding / abandoned indices left over from
the build pipeline (DAT was finalised earlier in development than INF was
trimmed). The runtime never indexes past 897 in the address-walking code paths
observed in the source. Neither `ndata_Read` nor `ndata_ReadChecked` range-checks
the id (the mirror has 1024 slots). Most callers compute the id at run time from a
base plus an offset (e.g. `playback_index + 0x131` in motion_StartPlayback, `a0 + 0x4D` in
func_80044F80); that every id lands on one of the 760 entries addressing NDATA.DAT
is not established from the code.

## In-RAM mirror

`g_ndata_inf` (`NdataInfEntry[1024]`, 0x800963EC, `.data`, defined at
`src/main/35000.c:1133`) holds the INF: 1024 4-byte
`{s16 start_sector; s16 length_sectors;}` slots. The EXE ships it pre-filled (its
bytes equal NDATA.INF except entries 899-900, then zeros), and `ndata_StartReadInf`
re-reads the file over it at init. Splat had cut it at +2 (`D_800963EE`, where the
original reads each length halfword); the label was merged (wave43 prep), so only
the function listings in `asm/funcs/` still spell `D_800963EE`.

NDATA.INF first 8 bytes: `00 00 49 00 49 00 46 00`, i.e. entry 0 = (0x0000, 0x0049),
entry 1 = (0x0049, 0x0046).

### Loader (`ndata_StartReadInf`)

`cdrom_StartReadAt(1, g_ndata_inf, 0, 2)`: file 1 of `g_cd_file_table` (3,604
bytes, NDATA.INF), 2 sectors (the INF rounded up), into the mirror. It only starts
the read; the next `cdrom_WaitIdle` (e.g. `ndata_Read`'s) waits for it.

### Reader (`ndata_Read`)

`ndata_Read(id, dst)`: `cdrom_WaitIdle`; `cdrom_StartReadAt(0, dst,
g_ndata_inf[id].start_sector, g_ndata_inf[id].length_sectors)` (file 0 =
NDATA.DAT, 58,114,048 bytes); `cdrom_WaitIdle`. `ndata_ReadChecked` first checks
that an arena destination has room ("Marionation over flow" otherwise);
`ndata_GetArenaRoomAfter(id)` is the arena's free bytes minus the entry's size.

### Pack cache

`ndata_LoadPack(n, dst)` reads entry `0x25 + n` into `dst`, keeps `dst` in
`g_ndata_pack` and sets `g_ndata_pack_loaded`. While it is set,
`ndata_ReadFromPack(id, dst)` serves ids 0x1E..0x24 from that block:
`g_ndata_pack_index[id - 0x1E]` (s16 {0, 1, 2, 3, 4, -1, 0}; -1 = not in the pack)
picks sub-block k; the block must start with the count word 5 (otherwise the
flag is cleared), and sub-block k lies at `block + (block[1 + k] & ~3)`, length
`block[2 + k] - block[1 + k]`, copied with `mem_MoveBlockRoundedUpToWords`. It returns 1 when it
served the read. `ndata_InvalidatePack` clears the flag (memory untouched) and
`ndata_IsPackLoaded` returns it. `ndata_ReadRemappedIndexPackFirstElseRead` remaps ids >= 0x1F to `id - 0x1B`,
tries the pack, and falls back to `ndata_Read`.

## File-ID → name mapping (source-side filename tables)

Source `rodata` at `0x80010DEC..0x8001528C` (`D_80010DEC[17568]`, `src/main/d_15EC.c:7`) is
a contiguous pool of 899 `DATA<n>\<NAME>.DAT` strings, each NUL-padded to a 4-byte
boundary (count strings, not NUL-separated pieces), followed by the
`Marionation over flow` format string. The executable also holds the pool's 899
pointers as `.data` words at 0x800955E0 (`D_800955E0`, `const char *[899]` at the head
of 35000.c's `.data`, right after 32D04.c's g_gpu_prim_type_halfword_sizes). No code
reads them: the shipped game does no name lookup, and every caller passes a numeric id.
The table is in INF order: `D_800955E0[i]` points at slot `i`'s name for all 899
entries (`src/main/35000.c:831`), the direct statement of the rule below.

### Pool-to-INF rule: one name per slot, in reverse

**INF slot `i` (0..898) is named by pool string `898 - i`.** The pool is written
last-id-first, so walking it backwards walks the INF forwards. Every slot has a name,
sentinels included (a sentinel is a named file the DAT does not carry); slots 899 and 900
have none. Evidence (read from NDATA.INF, NDATA.DAT and the EXE pool):

* Each of the 139 sentinels falls on a name the build left out: 138 are the
  highest-numbered motions of one `MARcc` character, each run ending exactly on that
  character's last slot (slots 328..336 = `MAR00_23..MAR00_31`, 361..368 =
  `MAR01_24..MAR01_31`, and so on); the 139th is slot 898 = `DATA1\HAND.DAT`.
* Group boundaries match the header shapes in the DAT (the 38 stage files carry the
  multi-section header `resource_LoadOrReuseSelectedBlock` parses, the 54 node assets are
  12,288-byte files with a 1-entry header), and the group byte totals in the table below
  sum to exactly 58,114,048, the DAT size.
* The code's base offsets land on group starts: `0x25` (`ndata_LoadPack`,
  `src/main/35000.c:294`) on `CHANBARA0`, `0x27` on `S00`, `0x4D` on `T00`, `0x4D + 0x1B`
  on `TINYM00`, `0x83` and `0x10C` on the two `WEP0000`s, `0x131`
  (`src/main/43500.c:780`) on `MAR00_00`.

[`ndata_filemap.csv`](ndata_filemap.csv) (from `tools/extract_ndata_names.py`) applies this
rule: one row per slot 0..900, sentinels flagged. The tool refuses to write unless the
pool holds 899 strings, the id bases above land on their names, and the EXE's pointer
table `D_800955E0` agrees with every slot's name.

### File-ID layout (by ID range)

| File IDs (hex) | Group | Names | Live / slots | Bytes | Read by |
|----------------|-------|-------|-------------:|------:|---------|
| `0x000..0x026` | DATA0 | `M000..M306`, `CHANBARA0/1` | 39 / 39 | 5,410,816 | slot assets (`ndata_ReadRemappedIndexPackFirstElseRead`, `src/main/35000.c:227-233`, blocks 3..5), served from the pack for ids `0x1E..0x24`; packs `0x25 + n` (`ndata_LoadPack`) |
| `0x027..0x04C` | DATA2 | `S00..S37` (stages) | 38 / 38 | 5,048,320 | `func_80044F30` (`0x27 + selector`) |
| `0x04D..0x067` | DATA5 | `T00..T26` | 27 / 27 | 331,776 | `func_80044F80` (`0x4D + id`) |
| `0x068..0x082` | DATA6 | `TINYM00..TINYM26` | 27 / 27 | 331,776 | `func_80044F80` (`0x4D + id + 0x1B`, grayscale mode) |
| `0x083..0x10B` | DATA8 | `WEP0000..WEP1515`, `WEPNULL` | 137 / 137 | 31,516,672 | `ndata_ReadIndexInRangeSelectedByFlag(0, k)` (`0x83 + k`) |
| `0x10C..0x130` | DATA9 | `WEP0000..WEP0707`, `WEPNULL` | 37 / 37 | 3,182,592 | `ndata_ReadIndexInRangeSelectedByFlag(!0, k)` (`0x10C + k`) |
| `0x131..0x381` | DATA7 | `MAR00_00..MAR22_13` | 455 / 593 | 12,292,096 | `motion_StartPlayback` (`0x131 + index`) |
| `0x382` (898) | DATA1 | `HAND` | 0 / 1 | 0 | sentinel |
| `0x383..0x384` | — | (no name) | leftovers | — | never |

Within a group the ids run in name order (`S00` = `0x27`, `MAR00_00` = `0x131`). DATA0's
order is `M000..M002`, `M100..M111`, `M003..M005`, `M200..M211`, `M301..M305`, `M300`,
`M306`, `CHANBARA0`, `CHANBARA1`: the pack ids `0x1E..0x24` are the `M3xx` set, and the two
pack blocks `0x25`/`0x26` (`CHANBARA0/1`) have header count word 5, the value
`ndata_ReadFromPack` requires (so do `M302..M305`, ids `0x1F..0x22`). Each `T<n>`/`TINYM<n>` pair has the same
header and size and differs in a few hundred bytes (colour vs grayscale). DATA8 names are
the 136 unordered weapon pairs `WEPaabb` (`aa <= bb`, both 00..15) plus `WEPNULL`; DATA9 the
36 pairs over 00..07 plus `WEPNULL`.

DATA7 by character (19 characters 00..22; 05, 10, 11, 16 absent). Each block lists motions
`_00` upwards and ends with its sentinels:

| Char | IDs (dec) | Live | Sentinel tail | Char | IDs (dec) | Live | Sentinel tail |
|-----:|-----------|-----:|--------------:|-----:|-----------|-----:|--------------:|
| 00 | 305..336 | 23 | 9 | 13 | 612..646 | 27 | 8 |
| 01 | 337..368 | 24 | 8 | 14 | 647..681 | 27 | 8 |
| 02 | 369..401 | 23 | 10 | 15 | 682..716 | 27 | 8 |
| 03 | 402..428 | 20 | 7 | 17 | 717..751 | 27 | 8 |
| 04 | 429..458 | 24 | 6 | 18 | 752..782 | 23 | 8 |
| 06 | 459..485 | 19 | 8 | 19 | 783..813 | 23 | 8 |
| 07 | 486..515 | 24 | 6 | 20 | 814..848 | 27 | 8 |
| 08 | 516..545 | 24 | 6 | 21 | 849..883 | 27 | 8 |
| 09 | 546..576 | 24 | 7 | 22 | 884..897 | 14 | 0 |
| 12 | 577..611 | 28 | 7 | | | | |

### Runtime selectors and the arena blocks they fill

| Reader | NDATA ids | Group | Arena block | Evidence |
|--------|-----------|-------|-------------|----------|
| `func_80044F30(selector, dst)` = `ndata_Read(selector + 0x27)` | `0x27..0x4C` for selector 0..37 | DATA2 stage `S<selector>` | 7 (stage) | `src/main/35000.c:238`; `resource_LoadOrReuseSelectedBlock` opens block 7 and reads into it or into the staging buffer (`src/main/368E4.c:84-89`); selector = `g_fight_stage_index` (`src/main/9F9C.c:1752-1753`) or `(control_word & 0x3F) - 1` of a motion block (`src/main/43500.c:833-842`) |
| `func_80044F80(id, dst)` = `ndata_Read(id + 0x4D)` | `0x4D..0x67`, or `0x68..0x82` in grayscale mode | DATA5 `T<id>` / DATA6 `TINYM<id>` (tiny models) | 8 (node asset) | `src/main/35000.c:248`; caller `src/main/368E4.c:335-366` adds `0x1B` when `gpu_RefreshGrayscaleMode()`; ids from `g_node_retained_draw_asset_ids` (`src/main/d_89C78.c:201`: 0x00..0x1A, 0xFF = none; `src/main/368E4.c:1590-1593`); `motion_StartPlayback` frees block 8 with "Destruction tiny model." when the arena is short (`src/main/43500.c:774-792`) |
| `motion_LoadPlaybackBlock(file_id)` = `ndata_ReadChecked(file_id)` | `0x131..0x381` (`0x131 + playback_index`) | DATA7 `MARcc_mm` | 0xA (motion playback) | `src/main/368E4.c:394-409`, caller `src/main/43500.c:780-794`; move callback `func_80046A80` -> `motion_RebasePlayback` (`src/main/368E4.c:413-415`) |

The selector ranges are data facts, not code bounds: no reader range-checks its argument.
Stage selectors 0..37 are exactly the 38 files whose header the stage loader parses. Its
per-selector cases read header words past the common five only for selectors 3, 4, 7, 13,
18 and 34 (`src/main/368E4.c:140-175`), and those six files (`0x2A`, `0x2B`, `0x2E`,
`0x34`, `0x39`, `0x49`) are the only stage files whose header count word exceeds 6 (8, 8,
8, 9, 8, 21). Selector 38 or more would read `T00` and the later tiny models, whose 1-word
header the stage loader does not fit.

**Arena block 9 has no reader and no caller.** Its three accessors, `func_80046934`
(`arena_OpenBlock(9)`), `func_8004695C` (peak, end, no-op move callback `func_80046954`) and
`func_800469A0` (`arena_Resize(9, n)`) (`src/main/368E4.c:380-390`), are referenced by no
`jal`, `j`, data word or `%hi/%lo` pair in `SLUS_006.63` or `MOVOVL.EXE` (word scan of
both images for the call encodings and the addresses; `%hi/%lo` and
named references searched in `asm/funcs/` and `src/`). No
other arena call passes 9: every `arena_*` call in `asm/funcs/` and `src/` takes a constant
6, 7, 8 or 0xA, or slot `n` / `n + 3` for slots 0..2. No `ndata_Read*` reads into it. The
shipped game never fills block 9, so this executable doesn't show what it held.

Arena blocks in use: 0..2 slot objects, 3..5 their asset blocks (slot + 3;
`src/main/35000.c:530-560`), 6 selected entries (`resource_LoadSelectedEntriesTexturesAndSound`),
7 stage, 8 tiny-model node asset, 9 unused, 0xA motion playback.

## Contents of a typical entry

Entries hold opaque data of various formats. The first 64 bytes of entry 0:

```
06 00 00 00  20 00 00 00  bc 61 00 00  e0 61 00 00
10 62 00 00  54 67 00 00  1c 62 01 00  f8 46 02 00
09 00 00 00  2c 00 00 00  f0 47 00 00  a0 4a 00 00
50 4d 00 00  b8 52 00 00  fc 55 00 00  80 57 00 00
```

This looks like a small TOC: a u32 record count (6), a u32 header size (0x20),
then 6 u32 offsets growing monotonically. Most NDATA entries appear to be small
nested archives of this kind — the inspector tool below reports the TOC where
the leading word is plausibly a record count, otherwise dumps a hex preview.

Decoding individual record types (geometry, animations, audio) is out of scope
for this archive document; see the per-format docs for BBM (motion), STAGE_BIN
(stage geometry), etc.

## Inspector tools

### `tools/inspect_ndata.py` — list / extract NDATA entries

```
python tools/inspect_ndata.py <NDATA.INF> (--list | --extract <ID> [--out FILE]
                                           | --dump-all <OUTDIR>
                                           | --hex <ID> [--bytes N]
                                           | --stats)
```

(Exactly one of `--list` / `--extract` / `--hex` / `--dump-all` / `--stats` is
required. The output filename for `--extract` is given via `--out`, and the
`--hex` byte count via `--bytes` — they are NOT bare positional arguments.)

Examples:

```
# Print every directory entry with start/length/preview
python tools/inspect_ndata.py disc/NDATA/NDATA.INF --list | head

# Extract entry 0 to a named file (default name: ndata_0000.bin)
python tools/inspect_ndata.py disc/NDATA/NDATA.INF --extract 0 --out entry000.bin

# Dump first 128 bytes of entry 42
python tools/inspect_ndata.py disc/NDATA/NDATA.INF --hex 42 --bytes 128

# Extract every entry into a directory
python tools/inspect_ndata.py disc/NDATA/NDATA.INF --dump-all extracted/

# Directory-level summary statistics
python tools/inspect_ndata.py disc/NDATA/NDATA.INF --stats
```

The DAT file is auto-located alongside the INF (same directory, `.DAT`
extension).

### `tools/repack_ndata.py` — rebuild INF + DAT from entries

```
python tools/repack_ndata.py --template disc/NDATA/NDATA.INF --entries extracted/ --out rebuilt/
python tools/repack_ndata.py --roundtrip disc/NDATA/NDATA.INF   # extract+repack in memory, exit 1 unless identical
```

The inverse of `--dump-all`. The template INF supplies the 901 slots. The
760 chained slots (every non-sentinel slot except the leftovers 899/900; the
template must have them back to back from sector 0) are laid out again in id
order, each `ndata_NNNN.bin` rounded up to whole sectors with a zero-padded
tail, so an entry can change size and every later entry moves. Sentinels and
the leftovers keep their INF words verbatim; leftover entry files are ignored.
A file for a sentinel slot, an empty file for a chained slot, or any file in
the directory not named `ndata_NNNN.bin` for a slot is an error. Repacking the
unmodified dump reproduces both files byte-identically, and a repacked INF
works as the next template. A resized NDATA.DAT also needs the disc's ISO
directory and the EXE's static `g_cd_file_table` entry 0 (disc position +
size; declared in `include/bb2.h`, values in `src/main/cdrom.c`) updated, and
every file after it moved (row `disc-build` in `docs/foundation/backlog.tsv`);
the EXE's pre-filled `g_ndata_inf` copy is overwritten by `ndata_StartReadInf`
at init.

### `tools/extract_ndata_names.py` — build the file-ID -> name map

```
python tools/extract_ndata_names.py            # write docs/formats/ndata_filemap.csv
python tools/extract_ndata_names.py --summary  # slot / group / class counts, checks
python tools/extract_ndata_names.py --stdout   # write CSV to stdout
```

The tool re-derives the file-ID -> name mapping every time by parsing the
rodata string pool out of the live `disc/SLUS_006.63`. The CSV under
`docs/formats/ndata_filemap.csv` is committed for human review and to make
the mapping greppable, but it is reproducibly regenerable. Columns: `file_id` (INF slot),
`name_vaddr`, `group`, `filename`, `content_guess`, `sentinel` (1 = `0xFFFF,0xFFFF`, a
named file the build left out; no byte range), `start_sector`, `len_sectors`,
`byte_offset`, `byte_length`. Slots 899 and 900 have no name.

## Verified by

* Hex-dump of all 3,604 bytes of `disc/NDATA/NDATA.INF` (901 little-endian (u16,
  u16) pairs).
* Cross-reference with the in-RAM mirror `g_ndata_inf` (`src/main/35000.c:1133`,
  same byte sequence).
* Loader `ndata_StartReadInf`, which CD-reads 2 sectors of NDATA.INF into it.
* Reader `ndata_Read`, which passes `g_ndata_inf[id]` (start_sector,
  length_sectors) to `cdrom_StartReadAt` on NDATA.DAT.
* Filename pool `D_80010DEC..0x8001528C` (label `D_80010DEC` in
  `src/main/d_15EC.c`) — 899 `DATA<n>\<NAME>.DAT`
  strings, string `898 - i` naming slot `i`: the 139 sentinels fall exactly on
  the left-out names, and the code's id bases (`0x25`, `0x27`, `0x4D`, `0x83`,
  `0x10C`, `0x131`) land on group starts.
* Cumulative-sum sanity check: starting at sector 0, walking only the non-
  sentinel entries in INF order, the running total exactly reaches sector
  0x6ED8 = 28,376 = `len(NDATA.DAT) / 0x800`. No gaps.
