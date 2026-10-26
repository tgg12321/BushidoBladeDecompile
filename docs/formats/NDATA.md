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
sectors; 139 are sentinels
that break the ID space into named "buckets" (one bucket per `DATA<n>\` prefix
in the source filename table — see below).

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
base plus an offset (e.g. `a0 + 0x131` in func_80054604, `a0 + 0x4D` in
func_80044F80); that every id lands on one of the 760 entries addressing NDATA.DAT
is not established from the code.

## In-RAM mirror

`g_ndata_inf` (`NdataInfEntry[]`, 0x800963EC, one 0x1000-byte `.data` label in
`asm/data/7D920.data.s`; `src/main/35000.c`) holds the INF: 1024 4-byte
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

Source `rodata` at `0x80010DEC..0x80015294` is a contiguous pool of 899
NUL-terminated ASCII strings of the form `DATA<n>\<NAME>.DAT`:

```
DATA0\CHANBARA{0,1}.DAT  2  intro / chambara cinematic
DATA0\Mxxx.DAT           37 character cinematic-motion modules
DATA1\HAND.DAT           1  hand / cursor data (single file)
DATA2\Sxx.DAT            38 stage geometry/data files (S00..S37)
DATA5\Txx.DAT            27 texture-page archives    (T00..T26)
DATA6\TINYMxx.DAT        27 reduced-LOD "tiny" models (TINYM00..TINYM26)
DATA7\MARxx_yy.DAT       593 marionation animation files
                              (character XX, motion YY)
DATA8\...                137 placeholder names — never indexed at runtime
DATA9\WEPccww.DAT        37 per-character weapon data + WEPNULL
                              (character c, weapon w)
```

Total: 899 names. The runtime indexes 762 of them; the 137 `DATA8\*` names
are placeholders that exist for build-pipeline bookkeeping but have no
corresponding INF entry — they were never copied into NDATA.DAT.

The executable does hold the pool's 899 pointers, as `.data` words at 0x800955E0
(`D_800955E0`, `const char *[899]` at the head of 35000.c's `.data`), right after 32D04.c's 44-halfword per-type size
table D_80095588. No code reads them.

The mapping from filename to file-ID is NOT a pointer table at runtime — it
is fixed at build time by the *order* in which the path-string pool appears
in rodata. Code paths like `ndata_LoadPack`'s `ndata_Read(a0 + 0x25, ...)` (`src/main/35000.c`)
pass hard-coded numeric IDs that correspond to specific slots in the on-disc
directory. The string pool exists only for debug / build identification —
the shipped game does not perform name-based lookup.

### Pool-to-INF alignment rule

The 762 active INF slots align 1:1 with the **non-DATA8** strings in the
pool, taken in pool order. To recover a name for INF entry `i`:

1. Walk the INF entries; keep a counter `k` that increments only on
   non-sentinel entries.
2. Walk the string pool; keep a counter `m` that increments only on
   non-`DATA8\` strings.
3. The name for INF entry `i` is the m-th non-`DATA8\` string where m is
   `k` at the moment `i` was visited.

The `tools/extract_ndata_names.py` tool implements this and emits
[`docs/formats/ndata_filemap.csv`](ndata_filemap.csv) — 762 rows with
columns `file_id, name_vaddr, group, filename, content_guess,
start_sector, len_sectors, byte_offset, byte_length`.

### File-ID layout (canonical groups, by ID range)

| File IDs   | Group | Files | Total bytes  | Content                                 |
|-----------:|:------|------:|-------------:|:----------------------------------------|
| `   0`     | DATA1 |     1 |      149,504 | `HAND.DAT` — hand / cursor sprite data  |
| `   1..683`| DATA7 |   593 |   53,776,384 | `MARcc_mm.DAT` marionation animations   |
| ` 684..728`| DATA9 |    37 |      876,544 | `WEPccww.DAT` weapon data + `WEPNULL`   |
| ` 729..763`| DATA6 |    27 |      708,608 | `TINYMcc.DAT` reduced-LOD character models |
| ` 764..798`| DATA5 |    27 |      612,352 | `Tcc.DAT` texture page archives          |
| ` 799..852`| DATA2 |    38 |      997,376 | `Scc.DAT` stage geometry                |
| ` 853..900`| DATA0 |    39 |   49,420,288 | `CHANBARA{0,1}.DAT` + `Mxxx.DAT` cinematics |

(Note the absent ID **898** is a sentinel within the DATA0 range; **899**
and **900** are the leftover slots discussed below. Sentinels also
appear inside DATA0 between `M004` (id 875) and `M003` (id 884), and inside
DATA9 between `WEP0105` (id 708) and `WEP0104` (id 717) — they don't only
fall on group boundaries.)

### Marionation character index

The 593 MAR entries cover 19 character indices (numbered 00..22, with
05, 10, 11, 16 absent — those slots presumably belonged to characters cut
during development). The first ID in each character's block is the
character's *highest-numbered* motion (the build-pipeline order writes each
character's motion table in reverse); within DATA7 the character order is
also reversed (MAR22 first, MAR00 last):

| Character | Motions | Total bytes | First ID | First filename       |
|----------:|--------:|------------:|---------:|:---------------------|
| MAR22     |     14  |   1,980,416 |        1 | `MAR22_13.DAT`       |
| MAR21     |     35  |   4,831,232 |       15 | `MAR21_34.DAT`       |
| MAR20     |     35  |   3,596,288 |       50 | `MAR20_34.DAT`       |
| MAR19     |     31  |     380,928 |       85 | `MAR19_30.DAT`       |
| MAR18     |     31  |   3,964,928 |      116 | `MAR18_30.DAT`       |
| MAR17     |     35  |   8,386,560 |      147 | `MAR17_34.DAT`       |
| MAR15     |     35  |   8,206,336 |      182 | `MAR15_34.DAT`       |
| MAR14     |     35  |   7,849,984 |      217 | `MAR14_34.DAT`       |
| MAR13     |     35  |   5,066,752 |      252 | `MAR13_34.DAT`       |
| MAR12     |     35  |   1,794,048 |      287 | `MAR12_34.DAT`       |
| MAR09     |     31  |   1,026,048 |      322 | `MAR09_30.DAT`       |
| MAR08     |     30  |     786,432 |      353 | `MAR08_29.DAT`       |
| MAR07     |     30  |     772,096 |      383 | `MAR07_29.DAT`       |
| MAR06     |     27  |     710,656 |      413 | `MAR06_26.DAT`       |
| MAR04     |     30  |     786,432 |      440 | `MAR04_29.DAT`       |
| MAR03     |     27  |     761,856 |      470 | `MAR03_26.DAT`       |
| MAR02     |     33  |     978,944 |      497 | `MAR02_32.DAT`       |
| MAR01     |     32  |     808,960 |      530 | `MAR01_31.DAT`       |
| MAR00     |     32  |   1,087,488 |      562 | `MAR00_31.DAT`       |

Per-character size correlates roughly with whether the character is
**playable** (MAR12..MAR21 are all multi-MB) versus an NPC / background
fighter (MAR00..MAR09 are sub-1 MB). The 8 most-populated character slots
(MAR12, 13, 14, 15, 17, 18, 20, 21 — note the 16 / 19 / 22 outliers) total
about 44 MB of motion data and likely correspond to the 8-fighter playable
roster.

### DATA9 weapon-data scheme

Names match `WEPccww.DAT` where the first two digits are the character
slot (00..07) and the last two are the weapon variant (00..07). Of the 64
possible (character, weapon) pairs, 36 are present plus 1 `WEPNULL.DAT` —
characters get between 5 and 8 weapon variants:

```
char 00 : WEP0000..WEP0007  (8 weapons — most-equipped fighter)
char 01 : WEP0101..WEP0107
char 02 : WEP0202..WEP0207
char 03 : WEP0303..WEP0307
char 04 : WEP0404..WEP0407
char 05 : WEP0505..WEP0507
char 06 : WEP0606, WEP0607
char 07 : WEP0707
```

`WEPNULL.DAT` (22,528 bytes) is the empty-handed / unarmed weapon body.
Note the character-index scheme here uses 00..07 (8 fighters) whereas the
MAR animation scheme uses 00..22 (with gaps) — the two indexes are NOT
the same and there is no direct numerical correspondence; the mapping from
MAR-character to WEP-character lives in `motion_make_table` / the
character roster tables (`src/ings2.c:162`).

### DATA0 cinematic-motion scheme

`Mccvv.DAT` where:

* `M0xx` (id 875, 884, 894..897, 899, 900) — 8 names, character-0 cinematic motions
* `M1xx` (id 885..893) — 12 names, character-1 cinematic motions
* `M2xx` (id 862..873) — 12 names, character-2 cinematic motions
* `M3xx` (id 855..861) — 7 names, character-3 cinematic motions
* `CHANBARA0.DAT` (id 854) — shared chambara opening (small, 12 KB)
* `CHANBARA1.DAT` (id 853) — shared chambara opening (large, 120 KB)

The 4 character-cinematic indices `M0..M3` are presumably the 4 ending /
story-mode characters that receive cinematic treatment.

### Leftover slots (file IDs 898, 899, 900)

Entry 898 is a sentinel; 899 and 900 run past the end of NDATA.DAT and are
believed to be build-pipeline residue:

```
[898] start=0xFFFF  len=0xFFFF        (sentinel)
[899] start=0x9800  len=0x0308        addresses byte 0x4C00000 (past DAT end)
[900] start=0x1900  len=0x5956        starts inside the DAT (overlapping live entries), ends past it
```

The pool, however, supplies names for the 762-th and 761-st non-DATA8
slots — `DATA0\M001.DAT` and `DATA0\M000.DAT`. So these *would-be*
character-0 motion-0 and motion-1 cinematics simply never made it into the
final DAT; the runtime never queries IDs 899 or 900 (both would read past
the end of NDATA.DAT if it did).

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
python tools/extract_ndata_names.py --summary  # per-group counts and size
python tools/extract_ndata_names.py --stdout   # write CSV to stdout
```

The tool re-derives the file-ID -> name mapping every time by parsing the
rodata string pool out of the live `disc/SLUS_006.63`. The CSV under
`docs/formats/ndata_filemap.csv` is committed for human review and to make
the mapping greppable, but it is reproducibly regenerable.

## Verified by

* Hex-dump of `disc/NDATA/NDATA.INF` bytes 0x00..0x4F (901 little-endian (u16,
  u16) pairs).
* Cross-reference with the in-RAM mirror `g_ndata_inf` (`asm/data/7D920.data.s`,
  same byte sequence).
* Loader `ndata_StartReadInf`, which CD-reads 2 sectors of NDATA.INF into it.
* Reader `ndata_Read`, which passes `g_ndata_inf[id]` (start_sector,
  length_sectors) to `cdrom_StartReadAt` on NDATA.DAT.
* Filename pool `D_80010DEC..D_80015294` (label `D_80010DEC` in
  `src/main/d_15EC.c`) — 899 `DATA<n>\<NAME>.DAT`
  strings whose order (after dropping the 137 `DATA8\*` placeholders)
  matches the 762 active INF entries 1:1. Independently verified by
  walking both lists with the alignment rule described above.
* Cumulative-sum sanity check: starting at sector 0, walking only the non-
  sentinel entries in INF order, the running total exactly reaches sector
  0x6ED8 = 28,376 = `len(NDATA.DAT) / 0x800`. No gaps.
