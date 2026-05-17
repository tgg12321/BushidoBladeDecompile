# D_80101F00 -- per-character state struct base (1100 bytes / 0x44C)

- Address: `0x80101F00` (inferred -- the struct base; no direct
  symbol exists for byte 0)
- Element size: `1100` bytes (= `0x44C`) per character
- Element count: at least **2** (the only observed `a0` index values
  are 0 and 1, consistent with the 2-player fighting game)
- Touchers: dozens of accessors throughout `src/code6cac.c` (each
  accessor names the specific field, e.g. `D_80101F12`,
  `D_80101F26`, `D_80101F4C`, `D_80101F4E`, ...)
- Proposed name: `g_char_state[]` (struct array)
- Per-element field-set proposed type: `CharState` (see below)
- Confidence: **medium** -- the struct itself is well-attested by the
  stride arithmetic, but very few fields are decoded yet.

## Why this entry doesn't propose a `D_80101F00` rename

Splat doesn't auto-detect a symbol at `0x80101F00` (no asm
references the byte-0 address directly -- code only ever accesses
specific fields like `D_80101F12`). Renaming the *base* of an array
that doesn't have an auto-detected label requires either:

* Adding an explicit declaration in `named_syms.txt` for
  `g_char_state = 0x80101F00`, then having splat / linker honour it.
* Or naming the individual fields and letting the struct base remain
  unnamed.

This evidence file documents the struct *shape* so that a future
reviewer can name either the struct base or all the field-level
symbols consistently.

## Stride evidence

The constant `1100` (= `0x44C`) appears explicitly in 4 places in
`src/code6cac.c`:

```c
// src/code6cac.c:2502, 2513, 2524, 2531 (all the same form)
s32 offset = a0 * 1100;
s16 v1 = *(s16 *)((u8 *)&D_80101F12 + offset);
...
```

Where `a0` is a small integer (player slot 0 or 1 -- the only values
observed at call sites). This means the per-character record is
exactly 1100 bytes wide, with field `f12` (at struct offset 0x12) as
the most-accessed field.

Verifying: `D_80101F12 + 1100` = `0x80101F12 + 0x44C` = `0x8010235E`,
which is in the range covered by the adjacent auto-detected symbols
(`D_80102350`, `D_8010235C`, `D_80102372`, ...) -- consistent with
per-character record 1 starting around `0x8010234C`.

## Known fields

Decoded from the access patterns in `src/code6cac.c`:

| Offset  | Width | Field name (proposed)             | Notes                                                                 |
|---------|-------|-----------------------------------|-----------------------------------------------------------------------|
| `0x12`  | s16   | `motion_data_idx`                 | Index into [`g_motion_data_base_ptr`](D_800A3860.md) (D_800A3860)     |
| `0x26`  | u8/u16| `state_flag_26`                   | Cleared to 0 at fight-start (`D_80101F26 = 0;`)                        |
| `0x4A`  | s16   | `motion_data_idx_alt`             | Used the same way as `+0x12` in line 2491; possibly the alternative   |
|         |       |                                   | "next motion" slot vs. `+0x12` being "current motion"                 |
| `0x4C`  | u16   | `keyframe_idx_curr`               | Current-frame index, used in lookup as `idx` for the motion table     |
| `0x4E`  | u16   | `keyframe_idx_prev`               | Previous-frame index (set by `*+0x4E = *+0x4C` snapshot at line 2499) |
| `0x86`  | s16   | `frame_counter_or_segment`        | Modulo-clamped against `v[0x14/2]` -- looks like a frame-counter      |

Many other fields are accessed in nearby code and elsewhere but
haven't been traced in this batch. The 1100-byte struct almost
certainly holds:

* Motion-state machine (current motion ID, current keyframe, next-
  motion request, transition timer).
* Per-character entity state (position, velocity, life, stamina --
  these would be the higher offsets we haven't decoded).
* Maybe some scratch buffer for per-frame computation.

## Element-1 base

For `a0 = 1` callers, the struct base is `0x80101F00 + 1 * 0x44C =
0x8010234C`. Adjacent auto-symbols `D_80102350` (offset 4 from struct
base) and `D_8010235C` (offset 0x10) are consistent with this layout.

## Why this name

* `g_char_state[]` -- captures (a) the per-character (i.e.,
  per-fighter) role, (b) the broad "state machine + everything else
  about a character" content, and (c) the array shape (indexed by
  `a0 = player slot`).
* Confidence **medium** because:
  1. The 1100-byte stride is mathematically certain.
  2. The per-character (2-element) indexing is unambiguous in 4
     places.
  3. Many of the included fields haven't been decoded -- a more
     specific name like `g_char_motion_state` (if it turns out to be
     purely motion-related) might fit better, OR if the struct also
     holds combat state, `g_char_full_state` would be more honest.

## Reviewer next steps

* Decode the higher-offset fields (around `+0x100`, `+0x200`, etc.)
  by reading more `code6cac.c` functions that access this region.
* If the struct turns out to be purely motion-state, rename to
  `g_motion_state[]` and keep `g_char_state` reserved for a
  different (combat / entity) struct.
* Once the field set is documented, propose a `CharState` struct
  typedef in `include/bb2/types.h` (or similar) and progressively
  retype the accessors to use field names instead of magic offsets.

## Reviewer alternatives

* `g_char_motion_state[]` -- if the struct is purely motion.
* `g_player_state[]` -- if "char" is reserved for char characters
  in the source.
* `g_entity_state[]` -- if the struct is a generic per-entity state
  used for non-player characters too.
