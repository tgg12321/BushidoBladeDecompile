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

Source `rodata` at `0x80010DEC..0x80015288` is a contiguous pool of NUL-terminated
ASCII strings of the form `DATA<n>\<NAME>.DAT`:

```
DATA0\CHANBARA0.DAT      "common chambara intro" intro group
DATA0\CHANBARA1.DAT
DATA0\Mxxx.DAT           37 character motion modules (M000..M306)
DATA1\HAND.DAT           hand / cursor data (a single file)
DATA2\Sxx.DAT            38 stage geometry/data files (S00..S37)
DATA5\Txx.DAT            27 texture-page archives    (T00..T26)
DATA6\TINYMxx.DAT        27 reduced-LOD "tiny" model files (TINYM00..TINYM26)
DATA7\MARxx_yy.DAT       593 marionation (animation) files
                         MAR00_00 .. MAR22_13, indexed by character & motion
DATA8\xxxx.DAT           137 entries  (auxiliary / sound)
DATA9\xxxx.DAT           37  entries  (auxiliary)
```

Total: 899 named records (the 762 active NDATA entries + 137 "DATA8" gap-fillers
the runtime never actually indexes — those map onto the 0xFFFF sentinel slots in
INF). The `DATA<n>\` prefix is purely descriptive; the on-disc archive is flat
and addressed only by integer file-ID.

The mapping from filename to file-ID is NOT a pointer table at runtime — it is
fixed at build time by the order in which the path-string pool appears in
rodata. Code paths like `seq_Start(0x25, ...)` (`src/text1a_c.c:1480`) pass
hard-coded numeric IDs that correspond to specific slots in the on-disc
directory. The string pool exists only for debug / build identification — the
shipped game does not perform name-based lookup.

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

## Inspector tool

`tools/inspect_ndata.py` — list / extract NDATA entries.

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
* Filename pool `D_80010DEC` (`asm/data/800.rodata.s:1346..4944`) — 899
  `DATA<n>\<NAME>.DAT` strings whose count and ordering corresponds 1:1 with
  the active INF entries.
* Cumulative-sum sanity check: starting at sector 0, walking only the non-
  sentinel entries in INF order, the running total exactly reaches sector
  0x6ED8 = 28,376 = `len(NDATA.DAT) / 0x800`. No gaps.
