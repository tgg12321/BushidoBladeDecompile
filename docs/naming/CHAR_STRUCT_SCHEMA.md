# Per-character struct schema (`g_practice_menu_table`)

The per-character struct is an array indexed by character/slot id.
Each record is **0x44C bytes (1100)**.  The array base is at
`g_practice_menu_table = 0x80101EC8`.  Record N spans
`0x80101EC8 + N * 0x44C` to `0x80101EC8 + (N+1) * 0x44C - 1`.

| Record | Start | End |
|---|---|---|
| 0 | 0x80101EC8 | 0x80102313 |
| 1 | 0x80102314 | 0x8010275F |
| 2 | 0x80102760 | 0x80102BAB |
| 3 | 0x80102BAC | 0x80102FF7 |

A 4th and 5th record (if they exist) would continue at 0x80102FF8
and 0x80103444 respectively; only 4 records show observable
accesses across the codebase.

## How this schema was derived

Scanned all `D_<addr>` references in `src/*.c` and `asm/funcs/*.s`.
For each address that fell within `0x80101EC8 + r*0x44C` for some r,
computed its offset within the record.  Offsets that appeared in
**at least 2 different records** were classified as true struct
fields (rather than coincidental hits on isolated globals near the
same address).

Result: **27 confirmed struct fields** out of 141 unique offsets
within the address range.  The other 114 are globals that happen to
fall in the address range but are not per-character struct fields
(e.g., the `g_se_voice_*` cluster at record 2's first ~0x40 bytes,
which only has record-2 references).

## Field layout

| Offset | Width | Records | Refs | Sample callsite | Role hint |
|--:|:-:|:-:|--:|---|---|
| +0x000 | (base) | 0,1,2 | 104 | `u8 *base = (u8 *)&g_practice_menu_table + idx * 0x44C` | record base (table indexing entry point) |
| +0x006 | s16 | 0,1 | 5 | `if (D_8010231A != 0)` | flag/state at offset 6 |
| +0x00E | s16 | 0,1 | 34 | `v3r[0x9] = (u8)D_80101ED6;` | byte/halfword cell read into v3r[9] |
| +0x012 | s16 | 0,1 | 28 | `s16 *eda = &D_80101EDA;` | pointer-taken s16 field |
| +0x020 | s8  | 0,1,2 | 20 | `func_80022580(arg0, ((s8 *)&D_80102780)[arg0], ...)` | indexed byte (within sub-array) |
| +0x03C | s32 | 0,1,2 | 12 | `D_8010279C = -1;` then `~sp.voice_mask` | voice/state mask (-1 sentinel) |
| +0x040 | u8  | 0,2   | 10 | `u8 *a1 = &D_801027A0;` | byte-array start |
| +0x048 | ?   | 0,1   | 4  | (asm-only refs) | -- |
| +0x04C | s32 | 0,3   | 4  | (asm-only refs) | -- |
| +0x054 | s32 | 2,3   | 24 | `s32 v0 = (&D_801027B4)[idx] + (v1 * 4);` | base of indexed table |
| +0x05E | ?   | 0,1   | 22 | `D_80101F26 = 0;` and `s0 = &D_80102372` | cleared cell + base-of-array |
| **+0x06A** | s16 | 0,1   | 57 | `if ((u16)D_80101F32 == 0x11 \|\| (u16)D_8010237E == 0x11)` | **SEQ subsystem state code** (formerly named `g_seq_state_p1`; really per-character) |
| +0x07A | ?   | 0,1   | 4  | (asm-only refs) | -- |
| +0x084 | s16 | 0,2   | 12 | `*(u16 *)((u8 *)&D_80101F4E + offset) = *(u16 *)((u8 *)&D_80101F4C + offset);` | **previous-frame keyframe field** (copy-old-to-new pattern) |
| +0x096 | s16 | 0,1,2 | 55 | `if (D_80101F5E != 0 \|\| D_801023AA != 0)` | flag tested across records (player-1 vs player-2 OR) |
| +0x0AD | s8  | 0,1   | 12 | `if (D_80101F75 != 0 \|\| D_801023C1 != 0); D_801023C1 = 0; D_80101F75 = 0;` | one-shot trigger flag |
| +0x0B1 | s8  | 0,1   | 4  | (asm-only refs) | -- |
| +0x0D8 | s32 | 0,1   | 6  | `temp_a3 = D_80101FA0 - D_801023EC` | **inter-player X-delta** (subtraction between records) |
| +0x0E0 | s32 | 0,1   | 7  | `temp_t1 = D_80101FA8 - D_801023F4` | **inter-player Z-delta** (paired with +0xD8) |
| **+0x0F4** | s32 | 0,1   | 36 | `D_80101FBC = a1; func_8003E6A0(D_80101FBC, D_80101FC4)` | **position arg A** (formerly `g_replay_camera_target_a`) |
| **+0x0FC** | s32 | 0,1   | 21 | `D_80101FC4 = a0_val; func_8003E6A0(D_80101FBC, D_80101FC4)` | **position arg B** (passed paired with +0x0F4) |
| +0x134 | s32 | 0,1   | 7  | `D_80101FFC = 0;` | cleared cell |
| +0x13C | s32 | 0,1   | 9  | `D_80102004 = 0;` | cleared cell |
| +0x14E | s16 | 0,1   | 5  | `D_80102016 = 0;` | cleared cell |
| +0x286 | s16 | 0,1   | 24 | `lui $at, %hi(D_8010214E)` | (asm-only; needs caller decode) |
| +0x28C | s32 | 0,1   | 4  | `lui $a0, %hi(D_80102154)` | (asm-only) |
| +0x31A | s16 | 0,1,2 | 21 | `D_8010262E = 0; D_801021E2 = 0;` | cleared cell across all records |

## Notable findings

### 1. Existing global names that are actually per-character fields

Two names in `named_syms.txt` turn out to be record-0 field accesses,
not standalone globals:

- **`g_seq_state_p1 = 0x80101F32`** is `record[0] + 0x6A`.  The
  corresponding fields in records 1, 2, 3 are at 0x8010237E, 0x801027CA,
  0x80102C16.  The current name is fine but slightly misleading -- it's
  not just "player 1's seq state", it's the SEQ field of record 0.
- **`g_replay_camera_target_a = 0x80101FBC`** is `record[0] + 0xF4`.
  Same situation -- it's record 0's "position arg A", not a single global.

These names are kept as-is because (a) they're already in code, and
(b) the record-0 interpretation makes sense for the single-player flow
where record 0 is always the active player.

### 2. World-position vec3 at +0xD8 (with stored copy at +0xF4) -- DECODED

Found via code6cac_b.c:765-771:

```c
temp_a3 = D_80101FA0 - D_801023EC;             // r[0]+0xD8 - r[1]+0xD8
temp_t1 = D_80101FA8 - D_801023F4;             // r[0]+0xE0 - r[1]+0xE0
temp_a0 = (temp_a3 * temp_a3) + (temp_t1 * temp_t1);   // squared distance
var_t0 = ((u32) (*((&D_8008D118) + temp_a0))) >> 3;    // -> g_isqrt_lut!
```

And vec3 layout confirmed via code6cac.c:1407-1413 which reads all
three components together and stores a biased copy:

```c
v1 = D_80101FA4;             // +0xDC -- middle (Y)
a0_val = D_80101FA8;         // +0xE0
D_80101FBC = a1;             // +0xF4 -- stored copy of X
D_80101FC0 = v1 - 0x384;     // +0xF8 -- stored Y with -0x384 bias
D_80101FC4 = a0_val;         // +0xFC -- stored Z
```

**Decoded fields:**

| Offset | Field | Role |
|---|---|---|
| +0xD8 | `world_pos.x` (`g_char_world_pos_x` @ 0x80101FA0) | World-space X |
| +0xDC | `world_pos.y` (`g_char_world_pos_y` @ 0x80101FA4) | World-space Y |
| +0xE0 | `world_pos.z` (`g_char_world_pos_z` @ 0x80101FA8) | World-space Z |
| +0xF4 | `stored_pos.x` (`g_replay_camera_target_a` @ 0x80101FBC) | Stored X (mirror) |
| +0xF8 | `stored_pos.y` (`g_char_stored_pos_y` @ 0x80101FC0) | Stored Y with -0x384 bias |
| +0xFC | `stored_pos.z` (paired with stored.x) | Stored Z |

The world_pos triple drives the 2D distance computation that gates
CPU AI range checks (via `g_isqrt_lut`).  The stored_pos triple is
a snapshot used by replay-camera / time-bonus calculations
(the -0x384 = -900 frame Y bias matches the `g_round_frame_counter`
chunking unit -- 30-second segments).

### 4. Two-arm flag pattern (`X || sibling`)

The pattern at `+0x96` and `+0xAD`:

```c
if (D_80101F5E != 0 || D_801023AA != 0)   // either record's +0x96 set
if (D_80101F75 != 0 || D_801023C1 != 0)   // either record's +0xAD set
```

These are flags that trigger global behavior when *any* character has
them set.  Cleared together in one-shot trigger fashion.

## How to use this schema

When decompiling a function that accesses a `D_80101XXX` or
`D_80102XXX` address within the table range:

1. Compute the offset: `(addr - 0x80101EC8) % 0x44C`.
2. Look up the offset in the table above.
3. If it's a confirmed field, use the role hint to interpret the access.
4. If it's not in the table, check whether the access is via the
   `&D_80101EC8 + N * 0x44C` indexing pattern -- if so, it's a struct
   field; if not, it might be a separate global.

## Caveats

- Width inferences are heuristic (based on extern decls and asm
  load instructions).
- Some "fields" at offsets like +0x048 and +0x048 have only asm-only
  references with no readable C body; their roles are inferred from
  the records pattern alone.
- Record 3 has very few field accesses observed, suggesting it may
  be a less-used slot (e.g., a tournament-mode tag-team partner or
  spectator record).
- The 504 "unclassified" rodata labels from
  [RODATA_CATALOG.md](RODATA_CATALOG.md) are mostly **fixed-stride
  per-character record DATA** (the actual record content for each
  character), separate from this schema (which describes the per-game
  state struct).

## Second-half decode (2026-05-17 follow-up)

The second half of the record (+0x100..+0x44B) is sparsely accessed
in the observable C source (record 0 mostly) but exposed enough
structure to identify several sub-fields and clusters:

### Position-pair sub-structs (+0x174 and +0x18C)

Two adjacent vec3 sub-structs, both **computed averages** of nearby
caller-supplied vectors:

| Offset | Field | Computation |
|---|---|---|
| +0x174 | `pos_midpoint.x` (`g_char_pos_midpoint_x` @ rec[0]: 0x8010203C) | `(a1[-5] + a1[-2]) / 2` |
| +0x178 | `pos_midpoint.y` (rec[0]: 0x80102040) | `(a1[-4] + a1[-1]) / 2` |
| +0x17C | `pos_midpoint.z` (rec[0]: 0x80102044) | `(a1[-3] + a1[0]) / 2` |
| +0x18C | `pos_centroid.x` (`g_char_pos_centroid_x` @ rec[0]: 0x80102054) | `(a1[-14] + a1[-11] + a1[-8]) / 3` |
| +0x190 | `pos_centroid.y` (rec[0]: 0x80102058) | `(a1[-13] + a1[-10] + a1[-7]) / 3` |
| +0x194 | `pos_centroid.z` (rec[0]: 0x8010205C) | `(a1[-12] + a1[-9] + a1[-6]) / 3` |

The midpoint is averaged from 2 vectors (player + opponent?  body
joint pair?); the centroid is averaged from 3 vectors (probably the
body's 3 control points -- waist + head + feet or similar).  Both are
stored in `char_state[N]`, computed per-frame.

### Reset block (+0x104..+0x14F)

A large clear-to-zero block: 17+ separate `D_8010xxxx = 0;`
assignments in func init code, spanning offsets +0x104, +0x108,
+0x10C, +0x114, +0x118, +0x11C, +0x124, +0x128, +0x12C, +0x134
(2-record), +0x138, +0x13C (2-record), +0x144, +0x148, +0x14C, +0x14E,
+0x150, +0x152.

This is a **per-frame state reset zone** (called e.g. by
`func_8005B43C` / `func_8005B6FC` audio init).  Likely the working
buffer for one tick of the character's combat-state machine.

### Sequential s32 blocks (+0x210..+0x224, +0x234..+0x24F) -- DECODED

Found via `(Vec3i *)&D_801020D8` cast pattern in code6cac_b.c:1070:

```c
Vec3i *dst_a = (Vec3i *)&D_801020D8;        // +0x210 in record[0]
Vec3i *dst_b = (Vec3i *)((u8 *)&D_801020D8 + 0x44C);  // +0x210 in record[1]
```

And usage pattern at code6cac_b.c:944-969:

```c
*(volatile s32 *)0x1F800370 = D_801020D8;                      // pos written to scratchpad
*(volatile s32 *)0x1F800370 = D_801020D8 + D_801020E4;          // pos + delta
```

This decodes to **two pairs of (position, delta) vec3 sub-structs**:

| Offset | Field (record 0) | Role |
|---|---|---|
| +0x210 | `vec3_a_pos.x` (0x801020D8) | vec3 A position X |
| +0x214 | `vec3_a_pos.y` (0x801020DC) | vec3 A position Y |
| +0x218 | `vec3_a_pos.z` (0x801020E0) | vec3 A position Z |
| +0x21C | `vec3_a_delta.x` (0x801020E4) | vec3 A delta X |
| +0x220 | `vec3_a_delta.y` (0x801020E8) | vec3 A delta Y |
| +0x224 | `vec3_a_delta.z` (0x801020EC) | vec3 A delta Z |
| +0x234 | `vec3_b_pos.x` (0x801020FC) | vec3 B position X |
| +0x238 | `vec3_b_pos.y` (0x80102100) | vec3 B position Y |
| +0x23C | `vec3_b_pos.z` (0x80102104) | vec3 B position Z |
| +0x240 | `vec3_b_delta.x` (0x80102108) | vec3 B delta X |
| +0x244 | `vec3_b_delta.y` (0x8010210C) | vec3 B delta Y |
| +0x248 | `vec3_b_delta.z` (0x80102110) | vec3 B delta Z |
| +0x24C | `vec3_b_w_or_alpha` (0x80102114) | separately-accessed 7th word (W or alpha?) |

The fact that `pos` and `delta` are summed in the scratchpad write
strongly suggests these are **animation vec3 keyframes with delta
interpolation** -- the current position is `pos + delta * t` style.

The +0x230 word between the two blocks (offset +0x230..+0x233) isn't
referenced separately, may be a 4-byte gap or part of vec3 B.

13 new field names landed in named_syms.txt as `g_char_vec3_{a,b}_{pos,delta}_{x,y,z}`
plus `g_char_vec3_b_w_or_alpha`.

### Record-2-only sub-arrays

| Offset | Size | Use |
|---|---|---|
| +0x308 | u8[N] | `extern u8 D_80102A68[];` -- per-record byte buffer (34 refs) |
| +0x318 | s16[N] | `extern s16 D_80102A78[];` -- per-record halfword buffer (19 refs) |

These are buffers carved out within the second half of record 2 only
(possibly a debug/log buffer that only player 2's record uses).

## Future work

1. **Decode the remaining 114 single-record offsets** -- many are
   probably true fields whose accesses are localized to one record
   in the observable C source.  As more functions get decompiled,
   more cross-record accesses will appear.
2. **Type each field** more precisely -- the width inferences are
   heuristic; reading the GENERATED `.h` from the per-character
   struct typedef (if any) would give exact types.
3. **Sub-struct identification** -- the +0x040..+0x064 block looks
   like an SE voice descriptor (matches the g_se_voice_* cluster
   semantics); the +0x0D8..+0x100 block looks like position+velocity
   state.  Could be broken into named sub-structs.
4. **+0x308 / +0x318 record-2 buffers** -- inspect record 2's
   consumer functions to identify what's being logged/buffered.
