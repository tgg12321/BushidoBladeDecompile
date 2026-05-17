# BBM — Character motion bundle ("Marionation" engine)

Files: `disc/MOTION/*.BBM` (18 files, plus `WIN.DAT` which uses a different
related format).

BBM is the bundle format used by Lightweight's "Marionation" engine — the
animation/skeleton runtime that powers all of BB2's character motion and was
reused by the studio in Kengo on PS2. Each BBM file packs the per-character
skeleton, the joint-track tables, and the compressed packed-keyframe payload
for one character (or `COMMON.BBM` for shared/system animations).

## File layout summary

```
+--------------------+---------------------------------------------------+
| 0x00..0x14         | 20-byte fixed header (magic + 4 section ends)     |
| 0x14..end1         | Section 1 — joint default-pose table              |
| end1..end2         | Section 2 — short header / parameter block        |
| end2..end3         | Section 3 — animation segment descriptor table    |
| end3..end4         | Section 4 — segment auxiliary table (or empty)    |
| end4..EOF          | Section 5 — bulk packed keyframe payload (~95%)   |
+--------------------+---------------------------------------------------+
```

## Header (20 bytes, all little-endian)

| Offset | Size | Field        | Notes                                        |
|--------|------|--------------|----------------------------------------------|
| 0x00   | 2    | magic        | `b"MC"` for `COMMON.BBM`, `b"MW"` everywhere |
| 0x02   | 2    | version      | Always `0x000B` in BB2                       |
| 0x04   | 4    | end1         | Byte offset where section 1 ends (= sec2 start) |
| 0x08   | 4    | end2         | end of section 2                             |
| 0x0C   | 4    | end3         | end of section 3                             |
| 0x10   | 4    | end4         | end of section 4 (start of section 5 bulk)   |

All offsets are absolute (from file start). EOF is the implicit end of
section 5.

The two-byte magic + two-byte version pack as the LE u32 `0x0B00434D` (MC) or
`0x0B00574D` (MW). The runtime does not validate the magic — it is purely a
disc-label that distinguishes the two roles.

## Verified section sizes (every BBM in the shipped game)

```
file            size     sec1   sec2  sec3  sec4   sec5(bulk)
COMMON.BBM    165860     2936    472  1464     0      160968   ← MC, sec4=0
GN.BBM         96120     2652    316   856   140       92136
GS.BBM        108468     2668    328   952   140      104360
K123.BBM      284880     4884    632  2500   908      275936
N123.BBM      276596     4800    720  2836   796      267424
ON.BBM          3480      240     12    20     8        3180   ← outlier (placeholder?)
S123.BBM      253672     5236    672  2596  1644      243504
S124.BBM      287740     5564    788  2984  1328      277056
S125.BBM      269308     4920    668  2736  1208      259756
S234.BBM      292724     5616    800  3064  1272      281952
T123.BBM      286120     5056    664  2528  1000      276852
U123.BBM      252816     5148    664  2508  1664      242812
U124.BBM      294060     5512    800  3048  1200      283480
U125.BBM      278076     4888    680  2744  1092      268652
U134.BBM      293716     5684    796  3036  1560      282620
U135.BBM      278972     5060    676  2732  1452      269032
U235.BBM      271600     4792    660  2712  1076      262340
Y123.BBM      282848     4984    740  2884   944      273276
```

Two outliers:
* `COMMON.BBM` (MC magic) — section 4 is empty (sec3 and sec4 share the same
  end offset 0x131C). The shared/common animations don't carry per-segment
  auxiliary data.
* `ON.BBM` — tiny (3,480 B). Likely a stub / placeholder used when no real
  motion bundle exists for a character slot.

## Filename convention

The 17 character-specific bundles encode (character_id, secondary_set) in the
basename. The hundreds-digit identifies the character and the trailing digits
list per-character variants. Character mapping (matched against the source-side
string pool at `D_8001036C..D_80010410` in `asm/data/800.rodata.s:337..410`,
where each string is wrapped in its own `g_str_bbm_*` label):

```
prefix   referenced as          character / role            file set
-------  ---------------------  --------------------------  --------
Y        g_str_bbm_y123          Yugiri (ninja-spear girl)   Y123.BBM
N        g_str_bbm_n123          Nightstalker / ninja        N123.BBM
K        g_str_bbm_k123          Kannuki / katana            K123.BBM
T        g_str_bbm_t123          Tatsumi                     T123.BBM
S        g_str_bbm_s123/4/5/234  Sara                        S123, S124, S125, S234
U        g_str_bbm_u123/4/5...   Utsusemi                    U123, U124, U125, U134, U135, U235
G                                Gunner / Black Lotus        GN.BBM, GS.BBM
O                                Other / generic stub        ON.BBM (3.4 KB stub)
```

The trailing digit cluster (`123`, `124`, `234`) appears to encode which weapon
sets are bundled (1=katana, 2=long sword, 3=sledgehammer, 4=naginata,
5=rapier — derived from the in-game weapon roster), but this is unverified.

The 18th file `COMMON.BBM` carries shared/system animations referenced by every
character (idle, hit reactions, victory poses).

## Sections in detail

The Marionation runtime parses BBM as five logical sections. The exact byte
layout of each section is partially understood — what follows is verified by
hex inspection of the smallest representative files; the per-byte interpretation
of section 5 (the bulk payload) is engine-specific and not decoded here.

### Section 1 — joint default-pose / skeleton parameter table

Starts at offset 0x14. Sample bytes from `ON.BBM` (240-byte section 1):

```
0000: 0003 00b2 0090 0090 0090 0090 0090 0090
0010: 0090 0090 0090 0090 0090 0090 0090 0090
...
0030: 0090 0090 0090 0090 0090 000c 000c 000c
0040: 0000 000c 000c 000c 0000 000c 000c 000c
...
0090: 0090 0090 8012 8000 0002 0400 0004 0000
00a0: 8012 8000 0001 2c00 002c 0000 8012 800c
00b0: 0000 1d00 001d 0000 800c 800c 800c 8006
...
```

Structure (partial):
* `0x00`: u16 — usually a small count
* `0x02`: u16 — second small count (segment count?)
* Then a sequence of 16-bit per-joint parameter codes. Values `0x9000`,
  `0x000C`, `0x0006` etc recur as defaults; codes with the top bit set
  (`0x8012`, `0x800C`) appear to mark structured sub-records that carry a
  joint-index byte and a payload byte.

For COMMON.BBM the same `00 03 00 b2 ...` opening repeats — strong evidence
that this section is engine-managed skeleton metadata rather than per-character
data, even in character files.

### Section 2 — short parameter / index block

Tens to ~800 bytes. ON.BBM's section 2 is 12 bytes total:

```
0000 0000 003c 0005 0069 000e
```

This looks like three (u32 timestamp, u16 segment_id, u16 count) descriptors —
a tiny lookup table referencing into section 3.

### Section 3 — animation segment descriptor table

The size of section 3 scales linearly with the size of section 5 in fully-
populated character files (sec3 ≈ 1% of sec5). This is consistent with
section 3 being a per-segment index where each entry points at a region inside
section 5.

K123.BBM section 3 starts with:

```
0000: 0000 0000 0a00 0f00 1400 1e00 1e00 2700
0010: 2800 3000 3200 3f00 3c00 4e00 4600 5d00
...
```

Each 4-byte group looks like `(u16 start_frame, u16 end_frame)` — frame ranges
for animation segments.

### Section 4 — auxiliary segment data

Often a small fraction of section 3's size. Empty (`0`) in `COMMON.BBM`,
present in all per-character bundles.

### Section 5 — bulk packed keyframe payload

The vast majority of the file. Contains the actual per-frame skeletal data
(joint rotations / translations) in a tightly packed encoding. Bit-rate analysis
suggests one frame per joint per segment in approximately 12-16 bytes — typical
for a quaternion-based keyframe + delta encoding.

K123.BBM section 5 start:

```
0000: c000 b0b9 d009 be1f 0000 0057 c878 0500
0010: ef03 8414 0000 2d91 b100 0000 a34a b10c
```

Not interpreted further by this document — the encoding requires reverse
engineering the Marionation runtime functions. **Caveat:** the relevant
runtime is *not* the obviously-named functions in `src/code6cac_c_mid.c`:

* `motion_LoadPreCalcData_80037F08` is a 3-line wrapper around
  `func_80079A30` + `bios_FormatDevice_B` — it doesn't touch BBM bytes.
* `motion_SetMotion` is a game-state machine that selects motion IDs
  based on input + character state — it never parses bytes either.
* `motion_make_table` (in `src/ings2.c:162`) builds the global motion
  index table.

The actual section-5 decoder is likely a separate per-frame interpreter
called from the main game loop after the BBM has been loaded into RAM;
finding it requires either dynamic tracing or hunting for the function
that reads from `D_800A38xx` motion-pointer storage with a per-joint
stride that matches the K123.BBM section-5 byte pattern shown above.

## WIN.DAT

`disc/MOTION/WIN.DAT` (387,072 = 0x5E800 bytes) is a fixed-stride table of
**27 records of 0x3800 (14336) bytes each**. It does NOT carry the BBM `MC`/
`MW` magic and uses a different (related) on-disc layout for victory-pose data
consumed by the win-animation system.

### File layout

```
+0x00000  record  0  (0x3800 bytes)
+0x03800  record  1
+0x07000  record  2
   ...
+0x5B000  record 26
=0x5E800  EOF
```

### Per-record layout

Each 0x3800 record uses only the first `0x30F0` (12528) bytes; the trailing
0x710 bytes are zero padding. (A few records pad starting one or two halfwords
later — see `inspect_windat.py` summary column `used_bytes` for the exact
boundary.)

The used region is an array of 16-bit signed values. Their range across all
27 records is `[-2048, +2048]` (one outlier of 5035 in records 8, 11, 23),
matching the PSX SDK's standard `1.0 = 4096` (Q3.12) / `1.0 = 1024` (Q5.10)
fixed-point conventions used by `gte_*` and `SVECTOR` data.

A small set of field positions hold near-constant values across records,
indicating they are anchored slots (not free-form):

| Halfword offset | Typical value(s) | Likely role                          |
|-----------------|------------------|--------------------------------------|
| `0..1`          | (~1100, ±~1500)  | Per-pose tunable (varies per record) |
| `5`             | `(0, -1023..-1024)` | Constant world-down unit vector?  |
| `9`             | `(0, 0)` (~22/27 recs) | Sentinel / zero padding slot   |

The combination of (a) fixed record stride, (b) tight i16 range typical of
fixed-point geometry, and (c) recurring `0, -1024` slot strongly suggests
each record is a sequence of `SVECTOR`/`MATRIX`-style transform keyframes for
one victory pose. Exact per-frame schema (frame stride, packed component
ordering, channel mapping) is not yet decoded.

### Why 27 records?

The number 27 has no obvious correspondence with the game's 8-character
roster. Plausible (unverified) breakdowns: 8 characters × 3 win poses + 3
shared cinematic poses; or 9 win-camera setups × 3 environments. Confirmation
would require tracing the loader from `motion_*` runtime calls.

### Related code

* `src/code6cac_c_mid.c:184` — `motion_LoadPreCalcData_80037F08(a0, a1)`
* `src/code6cac_c_mid.c:607` — `motion_shift_check_m_hit_stop`
* `src/code6cac_c_mid.c:883` — `motion_SetMotion`
* `src/ings2.c:162`  — `motion_make_table`
* `src/ings2.c:525`  — `motion_Open`

(The same set of motion-system callers consume both BBM and WIN.DAT.)

### Inspector

```
python tools/inspect_windat.py disc/MOTION/WIN.DAT            # summary table
python tools/inspect_windat.py disc/MOTION/WIN.DAT --rec 0    # hex + i16 preview
python tools/inspect_windat.py disc/MOTION/WIN.DAT --dump OUT # extract recNN.bin
python tools/inspect_windat.py disc/MOTION/WIN.DAT --csv      # all values as CSV
```

### Verified by

* All 27 records are exactly 0x3800 bytes (file size 0x5E800 / 0x3800 = 27.0).
* Zero-run analysis: largest zero runs across the file occur every 0x3800
  bytes (at file offsets 0x30F0, 0x68F0, 0xA0F0, 0xD8F0, …), confirming the
  record boundary.
* i16 range across all 27 records is `[-2048, +2048]` with three single-record
  outliers (5035) — consistent with PS1 fixed-point geometry data, not RGBA,
  audio samples, or compressed text.
* Halfword position 5 contains `(0, -1023)` or `(0, -1024)` in 19 of 27
  records and stays within `±32` of zero / `±64` of -1024 in the remaining
  records (no record contains a wildly different value at this slot).

### Unverified / TODO

* Per-frame structure of each record (frame stride, channel order).
* Mapping of record index → character / win pose / camera setup.
* Whether WIN.DAT is a stripped variant of BBM section 5 or a fully distinct
  schema (the absence of any per-record header argues for the latter).

## Related code

* `src/code6cac_c_mid.c:184` — `motion_LoadPreCalcData_80037F08(a0, a1)` —
  Marionation motion loader (parses one entry from a BBM).
* `src/code6cac_c_mid.c:607` — `motion_shift_check_m_hit_stop` — frame-shift
  detection.
* `src/code6cac_c_mid.c:883` — `motion_SetMotion` — applies a parsed motion to
  a character entity.
* `src/ings2.c:162`  — `motion_make_table` — global motion table builder.
* `src/ings2.c:525`  — `motion_Open` — initialises the motion subsystem (runs a
  vector of init thunks from `D_8008D070`).

The strings `Y123.BBM`, `K123.BBM`, etc. live as discrete labels in
`asm/data/800.rodata.s:335..414` (one `g_str_bbm_*` per string).

## Inspector tool

`tools/inspect_bbm.py` — parse and report on a BBM file.

```
python tools/inspect_bbm.py <file.bbm>           # summary + section table
python tools/inspect_bbm.py <file.bbm> --dump 1  # raw dump of section 1
python tools/inspect_bbm.py <file.bbm> --hex 1   # hex preview of section 1
python tools/inspect_bbm.py <dir>/               # iterate every BBM in a dir
```

## Verified by

* Hex-dump of the first 256 bytes of every file in `disc/MOTION/*.BBM`.
* Identical 20-byte header structure (magic + 4 u32) across all 18 files.
* Magic is `MC` only for `COMMON.BBM`, `MW` for the 17 character-specific
  files.
* Section end-offsets validate: `0x14 < end1 < end2 < end3 ≤ end4 < EOF`
  for every file.
* Filename cross-reference: every `*.BBM` on disc has a matching
  `g_str_bbm_*` label in `asm/data/800.rodata.s` lines 335–414.
* Related motion-runtime functions cross-referenced (`motion_Open`,
  `motion_SetMotion`, `motion_LoadPreCalcData_*`) — these all read packed
  records that match the section 5 layout.

## Unverified / TODO

* Per-byte semantics of section 1's skeleton parameter codes (`0x9000` /
  `0x000C` / `0x80xx` series).
* Section 5 keyframe encoding (would require dynamic analysis of
  `motion_SetMotion` and the Marionation step functions).
* See "Unverified / TODO" under the WIN.DAT section above for that file's
  open items.
