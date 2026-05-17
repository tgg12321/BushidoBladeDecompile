# Rodata + .data constant-table catalog

Generated 2026-05-17 by the rodata-classifier pass.

## Why this catalog exists

After 14 batches of `D_*` data-symbol naming, 89 string literal names,
and the medium-promotion passes, the remaining unnamed labels in the
rodata + .data sections are mostly **structured constant tables**
(jump tables, monotonic lookups, palettes, bitmask sets, zero pads,
and per-record pointer arrays).

Classifier pass found **904 unique unnamed non-string labels** across
all `asm/data/*.s` files; of these:

| Class | Count | Notes |
|---|---:|---|
| `zero_pad` | 369 | `.word 0` cells -- typically alignment / padding between records |
| `jump_table` | 17 | Function-pointer arrays (some applied as named callback tables) |
| `monotonic` | 7 | Strictly-increasing lookups (pitch curves, step tables) |
| `bitmask` | 4 | Power-of-2 .word arrays |
| `palette` | 3 | 32-entry CLUT palettes |
| `unclassified` | 504 | Mostly fixed-stride per-character / per-stage records |

## Highs landed (21 entries in `named_syms.txt`)

See the "rodata constant-table pass" block in `named_syms.txt`. The
21 names span:

- **5 function-pointer / callback tables** (seq_start, seq_step,
  packed_handler_8, two 42-entry uniform handler arrays)
- **3 per-record pointer arrays** (per-record_ptr at 0x8008D86C,
  per-char descriptors at 0x800900EC, stage lighting at 0x80094DF0)
- **5 lookup / step tables** (128-entry pitch curve at 0x80015620,
  byte index map at 0x8008EB28, step lookup at 0x8008EBCC,
  lookup_8/14/4 entries)
- **3 CLUT palettes** (32 entries each at 0x800A127C / 12FC / 137C,
  spaced 0x80 apart)
- **4 bitmask tables** (4-5 power-of-2 entries each)

## Why the rest isn't named

**Zero pads (369)**: These are 1-2 word blocks of `.word 0` between
records.  They're not symbols anyone reads; they're alignment
artefacts.  Naming each one `g_zero_pad_N_<addr>` would add 369 lines
to `named_syms.txt` without enabling any new lookup.  Skipped.

**Unclassified (504)**: Mostly fixed-stride per-character / per-stage
record data.  These need per-record schema analysis to name
meaningfully (i.e., decoding the record structure first).  Without
the schema, a name like `g_record_table_<addr>` would be no more
useful than the raw `D_<addr>`.  The catalog records their existence
but defers naming until structural analysis happens.

**Remaining jump tables (12 of 17)**: Some are referenced from
specific functions whose role we haven't pinned down.  Without
caller context, the table could be a state-machine dispatch, an
SPU command table, a CDROM op table, etc.  Defer until callsite
function is itself named.

## How to use this catalog

For a specific D_* label not in `named_syms.txt`:

1. `grep "dlabel D_<addr>" asm/data/*.s` to find the file.
2. Look at the content shape (`.word` count, value pattern).
3. Look at how the label is referenced in `src/*.c` or `asm/*.s`.
4. If the role is clear, add a name to `named_syms.txt` and remove
   the entry from this catalog.

Continued progress here pays off only as the surrounding code gets
decompiled; the constant tables alone don't reveal their semantic
without the consumers.

## Classification machinery

The pass used `tools/...` (in `/c/tmp/bb2_rodata_scan.py` during the
session; not committed -- can be regenerated).  Patterns matched:

- **jump_table** -- block contains symbol references mixed with code
  addresses (0x80010000-0x800A0000 range)
- **palette** -- 16/32/64/128/256 .word entries, all with bytes
  matching RGB-A layout
- **sin_cos** -- 64+ .short entries oscillating in `[-0x1FFF, 0x1FFF]`
  (the BB2 fixed-point sin/cos range)
- **monotonic** -- sequence is strictly increasing
- **bitmask** -- all values are powers of 2 or zero
- **zero_pad** -- all values are zero

Future passes could add: 256-entry table = u8 lookup, 0x44C-stride =
per-character record, etc.
