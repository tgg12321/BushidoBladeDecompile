# Rodata record-cluster catalog

Sequential-label-stride scan of `asm/data/*.s` identified **114
record clusters** (sequences of consecutive `D_<addr>` labels with
constant stride between them, indicating fixed-size record arrays).
Total labels covered: 530.

Generated 2026-05-17.

## Method

For each `.s` data file, walked the dlabel sequence in address order.
Detected runs of 3+ consecutive labels where:
- Stride between consecutive labels is constant
- All labels use the same data kind (`.word` / `.short` / `.byte`)

A cluster is identified by its first label and stride.

## Highs landed in `named_syms.txt` (9 clusters)

Clusters where the record format + size make the role clear:

| Symbol | Address | Stride | #Records | Notes |
|---|---|---|---|---|
| `g_motion_descriptor_table_5x16` | 0x8008D754 | 16 | 5 | referenced from g_per_record_ptr_table_8008D86C |
| `g_motion_descriptor_table_10x16` | 0x8008D7C4 | 16 | 10 | companion cluster |
| `g_char_pose_data_table_3x68` | 0x8008DA50 | 68 | 3 | .short data; per-character pose |
| `g_char_anim_curve_table_3x324` | 0x8008DE34 | 324 | 3 | .word data; per-character anim curves |
| `g_move_pattern_table_a_4x204` | 0x8008F220 | 204 | 4 | per-character move pattern A |
| `g_move_pattern_table_b_4x204` | 0x8008F6C8 | 204 | 4 | move pattern B (same header as A) |
| `g_move_pattern_table_c_5x224` | 0x8008FA0C | 224 | 5 | move pattern C |
| `g_move_pattern_table_d_3x224` | 0x8008FF2C | 224 | 3 | move pattern D |
| `g_char_sprite_bitmap_block_3x8320` | 0x80090740 | 8320 | 3 | per-character sprite/font bitmap (16-bit row data) |

### How the move-pattern tables were identified

The clusters at 0x8008F220, 0x8008F6C8, 0x8008FA0C, 0x8008FF2C all
have **identical first-8-bytes headers** between their records (and
across clusters that share the same stride):

```
0x07100801, 0x24330310, 0x19001312, 0x0F01010A, ...
```

The 204-byte records (clusters A and B) share the same opening bytes.
The 224-byte records (clusters C and D) share a different opening:

```
0x05100201, 0x2D115010, 0x01180013, 0x01131203, ...
```

This is a struct-header pattern: a 204-byte and 224-byte variant of
the same logical "move pattern descriptor" struct, applied to
different character groups (probably weapon-style vs unarmed-style
fighters, or short-range vs long-range characters).

The pointer table `g_per_char_descriptor_table_32` (32 entries at
0x800900EC) feeds these blocks via runtime indexing.

## Catalog (full 114 clusters)

The 105 unnamed clusters fall into rough size tiers:

### Large per-record blocks (> 100 bytes per record)

These are likely per-character or per-stage data blocks; each has a
specific role tied to a consumer function.  Most are unnamed pending
caller-function decompilation.

| First Label | Stride | #Records | Total bytes |
|---|---|---|---|
| (named above) |  |  |  |

### Medium records (16-64 bytes per record)

These look like per-something descriptor arrays (16-byte = pointer
quads, 32-byte = packet/instruction blocks).  Most are referenced
only via inline asm / specific consumer functions.

### Small records (1-16 bytes per record)

These are mostly tight lookup tables, enum mapping arrays, or
fixed-size constant tuples used by specific functions.

## How to use this catalog

1. If you encounter `D_<addr>` access in a function with stride-N
   indexing (e.g., `base + idx * 16`), grep this catalog for the
   stride to identify which cluster you're indexing into.
2. The cluster's `#Records` value tells you how many distinct
   character/stage/mode slots the array supports.
3. For the still-unnamed clusters: pick one whose consumer function
   you're decompiling, read the consumer to infer the record format,
   then add a name to `named_syms.txt` and remove from this catalog.

## Future work

The 105 remaining unnamed clusters are mostly small-stride (4-16 byte)
record arrays without clear consumer functions identified yet.  Three
ways to drill in:

1. **Match clusters to known consumers**: for each named function
   that uses runtime-indexed addressing into a table, identify which
   cluster is the table.
2. **Stride-pattern grouping**: many small-stride clusters within a
   short address range may be parts of a larger composite struct
   (e.g., 3 adjacent stride-4 clusters might be one stride-12
   record with field types).
3. **Per-character struct cross-reference**: many clusters in the
   `0x8009Axxx` range may be sub-tables of the per-character struct
   documented at `CHAR_STRUCT_SCHEMA.md`.

The full 114-cluster catalog (with stride + record count + file) is
available by running `tools/bb2_record_scan.py` (uncommitted; in
`/c/tmp/`).  Future research can re-run this and prune the named
clusters from the report.
