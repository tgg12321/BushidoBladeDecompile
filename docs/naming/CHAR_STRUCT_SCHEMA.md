# Per-character struct schema (`D_80101EC8`, typedef `Unk80101EC8Record`)

The per-character struct is an array indexed by character/slot id.
Each record is **0x44C bytes (1100)**.  The array base is at
`D_80101EC8 = 0x80101EC8` (named `g_practice_menu_table` until owner ruling Q103 reset it).  Q103 item 3
keeps the record neutral: "per-character" in this file is its old reading, not a
landed name, and the current layout is `Unk80101EC8Record` (include/game.h:1377).  Record N spans
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
| +0x000 | (base) | 0,1,2 | 104 | `u8 *base = (u8 *)&D_80101EC8 + idx * 0x44C` | record base (table indexing entry point) |
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
| **+0x06A** | u16 | 0,1   | 57 | `if ((u16)D_80101EC8[0].unk_6A == 0x11 \|\| (u16)D_80101EC8[1].unk_6A == 0x11)` (9F9C.c:2046-2047) | `unk_6A` (include/game.h:1424): compared with many constants (6, 5, 0xF, 0x11, 0x13, 0x15, 0x1C, 0x21, 0x28: 9F9C.c:1656, 175A4.c:34/:52/:88, 17AFC.c:913/:982/:1010/:2409); no role established, no SEQ use |
| +0x07A | ?   | 0,1   | 4  | (asm-only refs) | -- |
| +0x084 | s16 | 0,2   | 12 | `*(u16 *)((u8 *)&D_80101F4E + offset) = *(u16 *)((u8 *)&D_80101F4C + offset);` | **previous-frame keyframe field** (copy-old-to-new pattern) |
| +0x096 | s16 | 0,1,2 | 55 | `if (D_80101F5E != 0 \|\| D_801023AA != 0)` | flag tested across records (player-1 vs player-2 OR) |
| +0x0AD | s8  | 0,1   | 12 | `if (D_80101F75 != 0 \|\| D_801023C1 != 0); D_801023C1 = 0; D_80101F75 = 0;` | one-shot trigger flag |
| +0x0B1 | s8  | 0,1   | 4  | (asm-only refs) | -- |
| +0x0D8 | s32 | 0,1   | 6  | `temp_a3 = t2_base->unk_D8.x - t3_base->unk_D8.x` (17AFC.c:2104) | `unk_D8.x`, differenced between records 0 and 1 |
| +0x0E0 | s32 | 0,1   | 7  | `temp_t1 = t2_base->unk_D8.z - t3_base->unk_D8.z` (17AFC.c:2105) | `unk_D8.z`, differenced between records 0 and 1 |
| **+0x0F4** | s32 | 0,1   | 36 | `e->unk_F4.x = x;` (9F9C.c:1305); `func_8003E6A0(D_80101EC8[0].unk_F4.x, D_80101EC8[0].unk_F4.z)` (:2174) | `unk_F4.x` of the Vec3i32 `unk_F4` (include/game.h:1474); no role established |
| **+0x0FC** | s32 | 0,1   | 21 | `e->unk_F4.z = z;` (9F9C.c:1307); the same `func_8003E6A0` call (:2174) | `unk_F4.z`; one read of many (9F9C.c:2580, 175A4.c:171, 17AFC.c:2169) |
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

- **`0x80101F32`** is `D_80101EC8[0].unk_6A` (u16, include/game.h:1424).  The
  same member of records 1, 2, 3 is at 0x8010237E, 0x801027CA,
  0x80102C16.  Its old registry names are retired: wave16 reset it to
  `D_80101F32`.
- **`0x80101FBC`** is `D_80101EC8[0].unk_F4.x` (include/game.h:1474); wave05
  reset it to `D_80101FBC`.

The C spells both as record members, so a role found for them names
the member (`unk_6A`, `unk_F4`), not an interior global; no access ties
record 0 to one player.

### 2. Triples at +0xD8 (`unk_D8`) and +0xF4 (`unk_F4`)

src/main/17AFC.c:2102-2108 differences records 0 and 1:

```c
temp_a3 = t2_base->unk_D8.x - t3_base->unk_D8.x;   // t2_base = D_80101EC8
temp_t1 = t2_base->unk_D8.z - t3_base->unk_D8.z;   // t3_base = t2_base + 1
temp_a0 = (temp_a3 * temp_a3) + (temp_t1 * temp_t1);
if (temp_a0 < 0x400U) {
    var_t0 = ((u32)(g_sqrt_table_u8[temp_a0])) >> 3;
```

and func_8001C624 (src/main/9F9C.c:1300-1307) copies `unk_D8` into
`unk_F4` with a -0x384 bias on y:

```c
x = e->unk_D8.x;
y = e->unk_D8.y;
z = e->unk_D8.z;
e->unk_F4.x = x;
e->unk_F4.y = y - 0x384;
e->unk_F4.z = z;
```

**Members (record 0 addresses; neutral `D_` names since waves 05, 15 and 16):**

| Offset | Member | Record 0 address |
|---|---|---|
| +0xD8 | `unk_D8.x` | `D_80101FA0` |
| +0xDC | `unk_D8.y` | `D_80101FA4` |
| +0xE0 | `unk_D8.z` | `D_80101FA8` |
| +0xF4 | `unk_F4.x` | `D_80101FBC` |
| +0xF8 | `unk_F4.y` | `D_80101FC0` |
| +0xFC | `unk_F4.z` | `D_80101FC4` |

What the triples hold is not established: `unk_D8` is passed by address
to func_80021D10 / func_80021DB0 / func_80022224 (9F9C.c:1297, :4029-4051),
stepped by `unk_104` and `unk_134` (:5228-5230, :5264-5266) and differenced
between records 0 and 1 (17AFC.c:2104-2105, :2227-2228); `unk_F4` is also
set to `unk_D8 + unk_E8` (9F9C.c:4060, :5286).

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
  RODATA_CATALOG.md are mostly **fixed-stride
  per-character record DATA** (the actual record content for each
  character), separate from this schema (which describes the per-game
  state struct).

## Second-half decode (2026-05-17 follow-up)

The second half of the record (+0x100..+0x44B) is sparsely accessed
in the observable C source (record 0 mostly) but exposed enough
structure to identify several sub-fields and clusters:

### Vec3i32 members at +0x174 and +0x18C

Two adjacent Vec3i32 members that func_8002C61C fills with averages of
scratchpad points (17AFC.c:2428-2442) and func_80022580 sets to `unk_F4`
(9F9C.c:4100, :4102):

| Offset | Member (rec[0] address) | func_8002C61C computation (record i) |
|---|---|---|
| +0x174 | `unk_174.x` (`D_8010203C`) | `(SPAD->unkA8[i][4].x + SPAD->unkA8[i][5].x) / 2` |
| +0x178 | `unk_174.y` (`D_80102040`) | same, `.y` |
| +0x17C | `unk_174.z` (`D_80102044`) | same, `.z` |
| +0x18C | `unk_18C.x` (`D_80102054`) | `(SPAD->unkA8[i][1].x + [2].x + [3].x) / 3` |
| +0x190 | `unk_18C.y` (`D_80102058`) | same, `.y` |
| +0x194 | `unk_18C.z` (`D_8010205C`) | same, `.z` |

What the scratchpad points `SPAD->unkA8[i][k]` hold is not shown by
these uses; wave15 reset the old registry names of these words.

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

The 13 `g_char_vec3_*` field names once proposed here are no longer in
named_syms.txt; the last one noted there, at 0x80102114, was reset to
`D_80102114` (data-wave 2026-10-08).

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


## Pointer-access-derived fields (added 2026-05-20)

The original schema was built from direct `D_<base+off>` references only.
But the struct is mostly accessed via a **pointer argument** (`lw $v0,
0xNN($a0)`), which never appears as a `D_` symbol. Recovered these by
**fingerprinting**: any register that accesses >=2 known fields
(`0x6A`,`0xF4`,`0xD8`,`0xE0`,`0x96`,`0x134`...) within a function is a
char-struct pointer, so its *other* offset accesses are also fields.

**Result: 63 functions confirmed to take a char-struct pointer** (39 of
them still `func_XXXX` -- they are per-character processors and prime
naming targets; list in `tmp/charptr.json`).

New fields (structural facts are high-confidence from access shape; the
role column is *inferred* and should be confirmed before relying on it):

| Offset | Width | Access pattern | Inferred role |
|--:|:-:|---|---|
| +0x00C | s16 | read-heavy; `xori 0x1F`, cmp `0x1D` | direction/angle index (32-entry table; 0x1F mask) |
| +0x060 | s32 | read-only, sign-tested (`bgez`), paired with +0x064 | signed value/bound pair (read-only) |
| +0x0A8 / +0x0AC / +0x0B0 | s32 x3 | rw; scaled via `mult`; also byte-read in some fns | **vec3** (coordinate/scale vector) |
| +0x0B8..+0x0C4 | s32 x4 | set/read together as a block | 4-word state block (vector or saved-input quad) |
| +0x0CC / +0x0D0 / +0x0D4 | s32 x3 | set together; +0xD0 feeds a `mult` | **vec3** (mutable coordinate) |
| +0x0DA / +0x0DE / +0x0E2 / +0x0E6 | s16 x4 | write-only `sh $zero` (init) | halfword array, cleared at init |
| +0x0E8 | s32 | read then `addiu -0x384` | frame/Y-segment field (-0x384 = 30s round-segment bias, cf. +0xF8) |
| +0x104 / +0x108 / +0x10C | s32 x3 | cleared at init, read together | **vec3 accumulator** (reset on init; near stored_pos +0xF4) |
| +0x1CA | s16 | differenced across the two records (`r0.1CA - r1.1CA`) in `coli_hit_body_weap` | per-character body coordinate (collision distance, like the +0xD8/+0xE0 inter-player deltas) |

The +0xA8..+0xD4 span is the struct's **physics/transform region** (several
vec3s) leading into the documented world_pos vec3 at +0xD8. The +0x104..
+0x10C accumulator and the +0xDA..+0xE6 cleared array sit between the
stored_pos snapshot (+0xF4) and the action-state region.
