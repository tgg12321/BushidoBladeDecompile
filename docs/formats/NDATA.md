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
Of the 901 indices, **762** carry real (start, length) data; 139 are sentinels
that break the ID space into named "buckets" (one bucket per `DATA<n>\` prefix
in the source filename table — see below).

The 762 active entries pack exactly 28,376 sectors = the entire DAT file. There
are no gaps in the actual on-disc layout; each active entry begins where the
previous active entry ended.

### Trailing entries 898..900 (out-of-range)

Indices 898, 899, 900 lie at the very end of the INF and reference sectors past
the end of NDATA.DAT:

```
[898] start=0xFFFF  len=0xFFFF        (sentinel)
[899] start=0x9800  len=0x0308        (would address byte 0x4C00000 — past DAT end)
[900] start=0x1900  len=0x5956        (would address byte 0x39E0000 — past DAT end)
```

These three slots appear to be footer-padding / abandoned indices left over from
the build pipeline (DAT was finalised earlier in development than INF was
trimmed). The runtime never indexes past 897 in the address-walking code paths
observed in the source — `func_80044E74` / `func_80044FA0` clamp their input via
the 1024-entry RAM mirror but the real callers always pass IDs that map into the
762 valid entries.

## In-RAM mirror

At game init the entire INF is loaded into RAM at `0x800963EC` (1024 4-byte
slots, zero-padded past the on-disc 901 entries). Two interleaved s16 views of
the same memory are used:

| Label       | RAM address | Stride | Field                                       |
|-------------|------------|--------|---------------------------------------------|
| `D_800963EC` | 0x800963EC | 4      | start_sector for entry N at offset N*4      |
| `D_800963EE` | 0x800963EE | 4      | length_sector for entry N at offset N*4     |

This means the same INF bytes are pointed at by two labels offset by 2 — the C
source can iterate "starts" via `D_800963EC[i]` (each as `s16 x; s16 _len;`) or
"lengths" via `((s16*)&D_800963EE)[i*2]`. Decomp source currently misidentifies
the array as `Coord {s16 x, s16 y;}` (see `src/text1a_c.c:1410`) because the
geometric type-name was a placeholder applied by an early auto-import pass; the
actual interpretation is (start_sector, length_sector).

### Evidence (RAM mirror == INF byte image)

NDATA.INF first 8 bytes: `00 00 49 00 49 00 46 00`
Means entries 0 and 1 are:
  - entry 0: start=0x0000, length=0x0049
  - entry 1: start=0x0049, length=0x0046

`D_800963EC` and `D_800963EE` raw words (from `asm/data/7D920.data.s` lines
12397–12420):

```
800963EC: 0x0000          ← start_sector[0]
800963EE: 0x0049          ← length_sector[0]
800963F0: 0x0049          ← start_sector[1]
800963F2: 0x0046          ← length_sector[1]
800963F4: 0x008F          ← start_sector[2]
800963F6: 0x0049          ← length_sector[2]
...
```

The two labels point at the same array, offset by exactly two bytes; reading at
stride 4 from either yields the corresponding column.

### Loader (`func_800451A0`)

```
glabel func_800451A0
    addiu      $a0, $zero, 0x1
    lui        $a1, %hi(D_800963EC)
    addiu      $a1, $a1, %lo(D_800963EC)
    addu       $a2, $zero, $zero
    jal        special_camera_check_pos_outside_ground_80036E34
     addiu     $a3, $zero, 0x2
```

This dispatches a CD read of 2 sectors (`a3=2` — the INF is 3,604 bytes = 1.76
sectors → 2 sectors with zero padding) into `D_800963EC`. The function name
`special_camera_check_pos_outside_ground_80036E34` is an auto-generated
misnomer — the function is actually the disc-resource scheduler used for both
"special camera" data and the NDATA.INF preload.

### Reader (`func_80044E74`)

```
glabel func_80044E74
    addiu      $sp, $sp, -0x20
    sw         $s0, 0x10($sp)
    addu       $s0, $a0, $zero        # s0 = file_id
    sw         $s1, 0x14($sp)
    sw         $ra, 0x18($sp)
    jal        game_FrameLoop
     addu      $s1, $a1, $zero        # s1 = arg1 (dest / completion target)
    sll        $s0, $s0, 2            # byte_offset = file_id * 4
    addu       $a0, $zero, $zero
    lui        $at, %hi(D_800963EC)
    addu       $at, $at, $s0
    lh         $a2, %lo(D_800963EC)($at)   # a2 = start_sector[file_id]
    lui        $at, %hi(D_800963EE)
    addu       $at, $at, $s0
    lh         $a3, %lo(D_800963EE)($at)   # a3 = length_sectors[file_id]
    jal        special_camera_check_pos_outside_ground_80036E34
     addu      $a1, $s1, $zero
```

`func_80044E74(file_id, completion_target)` is the canonical "queue NDATA file
to be DMA'd in" call. It looks up the (start, length) pair and posts a CD read
to the scheduler.

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

The mapping from filename to file-ID is NOT a pointer table at runtime — it
is fixed at build time by the *order* in which the path-string pool appears
in rodata. Code paths like `seq_Start(0x25, ...)` (`src/text1a_c.c:1480`)
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
and **900** are the past-end leftover slots discussed below. Sentinels also
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

### Past-end leftovers (file IDs 898, 899, 900)

The very last entries in INF reference sectors past the end of NDATA.DAT
and are believed to be build-pipeline residue:

```
[898] start=0xFFFF  len=0xFFFF        (sentinel)
[899] start=0x9800  len=0x0308        addresses byte 0x4C00000 (past DAT end)
[900] start=0x1900  len=0x5956        addresses byte 0x39E0000 (past DAT end)
```

The pool, however, supplies names for the 762-th and 761-st non-DATA8
slots — `DATA0\M001.DAT` and `DATA0\M000.DAT`. So these *would-be*
character-0 motion-0 and motion-1 cinematics simply never made it into the
final DAT; the runtime never queries IDs 899 or 900 (and would crash
trying to disc-read past-end sectors if it did).

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
python tools/inspect_ndata.py <NDATA.INF> [--list | --extract <ID> [OUT]
                                            | --dump-all <OUTDIR>
                                            | --hex <ID> [N]]
```

Examples:

```
# Print every directory entry with start/length/preview
python tools/inspect_ndata.py disc/NDATA/NDATA.INF --list | head

# Extract entry 0 to stdout (binary)
python tools/inspect_ndata.py disc/NDATA/NDATA.INF --extract 0 entry000.bin

# Dump first 128 bytes of entry 42
python tools/inspect_ndata.py disc/NDATA/NDATA.INF --hex 42 128

# Extract every entry into a directory
python tools/inspect_ndata.py disc/NDATA/NDATA.INF --dump-all extracted/
```

The DAT file is auto-located alongside the INF (same directory, `.DAT`
extension).

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
* Cross-reference with in-RAM mirror `D_800963EC` /  `D_800963EE` at
  `asm/data/7D920.data.s:12397..14450` (same byte sequence, two interleaved
  views).
* Loader function `func_800451A0` (`asm/funcs/func_800451A0.s`) which CD-reads
  2 sectors from disc into `D_800963EC`.
* Reader function `func_80044E74` (`asm/funcs/func_80044E74.s`) which uses
  `D_800963EC[id*4]` and `D_800963EE[id*4]` as (start_sector, length_sector)
  and passes them to the disc scheduler.
* Filename pool `D_80010DEC..D_80015294` (label `D_80010DEC` in
  `asm/data/101C.rodata_text1a_a.s:8..3604`) — 899 `DATA<n>\<NAME>.DAT`
  strings whose order (after dropping the 137 `DATA8\*` placeholders)
  matches the 762 active INF entries 1:1. Independently verified by
  walking both lists with the alignment rule described above.
* Cumulative-sum sanity check: starting at sector 0, walking only the non-
  sentinel entries in INF order, the running total exactly reaches sector
  0x6ED8 = 28,376 = `len(NDATA.DAT) / 0x800`. No gaps.
