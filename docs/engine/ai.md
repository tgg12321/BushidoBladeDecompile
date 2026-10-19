# CPU opponent: what the code establishes

No routine in `src/` is identified as the CPU opponent's decision logic, i.e. code
that picks moves for a computer-controlled fighter. Both fighters are
`Unk80101EC8Record`s (0x44C bytes, include/game.h:1647) in `D_80101EC8[]`
(include/bb2.h:153). This page describes the routines nearest that topic: the
per-fighter kind queue and its object pool, the shuffled ID list built for a
match sequence, and three standalone helpers. All are C.

## The per-fighter kind queue and the object pool `D_80106A78`

Each fighter record holds a queue of up to 12 kinds: a count `unk_330` and the
entries `unk_332[12]` (include/game.h:1562-1564). `D_80106A78[12]`
(include/bb2.h:191) is a pool of 0x64-byte `Obj80106A78` objects
(include/game.h:1695); a free slot has `kind == -1` and `slot == 0xFF`.

- `func_8003047C(rec)` (src/main/17AFC.c:3728) seeds the queue: it copies the five entries of
  `D_8008E338[rec->unk_0A]` (`s8 [27][5]`, include/bb2.h:193), counts the ones
  other than -1, and stores the first in `D_800A36F2[rec->index]`. Called from
  src/main/9F9C.c:3783.
- `func_80030B10(rec, kind)` (src/main/17AFC.c:3901) appends a kind, or puts it
  first when `rec->unk_88 != -1` and `kind == rec->unk_14`; it returns 0 when
  the queue already holds 12.
- `func_80030580(rec, kind)` (src/main/17AFC.c:3756) allocates the first free
  pool slot and places the object at the fighter (`unk_F4`, raised by
  `unk_1A / 32`). Its velocity comes from `D_8008E194[kind]`
  (`Tbl8008E194`, include/game.h:1661): the horizontal speed `unk4` along the
  fighter's heading `unk_1C8.vy`, the vertical speed `unk6`, and a spin
  `unk8` about Y or X by `unk0`.
- `func_800307D0(rec)` (src/main/17AFC.c:3819) takes an entry off the queue
  (index 1 instead of 0 when it holds two or more, the head equals
  `rec->unk_14` and `rec->unk_88 != -1`), spawns it through `func_80030580`, and plays a cue
  through `func_80032854`. `func_80030900(rec, pos)` (src/main/17AFC.c:3853)
  spawns the head entry at `pos`, and `func_80030A2C(rec, kind, pos)`
  (src/main/17AFC.c:3879) spawns `kind` at `pos`; both give the object a
  random velocity and spin from `rng_Next`.
- `func_80030BA8(rec)` (src/main/17AFC.c:3919) takes a pool object back into
  the queue: the first object whose kind is not -1 or 0x12..0x1D, whose
  `unk_50` is 0, which lies within 100 units of the fighter's `unk_B8.vy`
  vertically and within a squared horizontal distance of 0xF423F of
  `unk_F4`. It frees the slot and returns the kind (-1 if none, or if the
  queue is full).
- The fighter update calls these at src/main/9F9C.c:5101 (`func_80030A2C`),
  :5122 (`func_800307D0`) and :5128 / :5133 (`func_80030BA8`), gated on the
  record's `unk_6A` / `unk_40` values.
- Pool cleanup: `func_8003043C` (src/main/17AFC.c:3719) frees all twelve
  slots (called at src/main/9F9C.c:1769); `func_80030524`
  (src/main/17AFC.c:3745) frees objects with `unk_08 != 0` and `func_80030D04`
  (src/main/17AFC.c:3967) frees kinds 0x12..0x1D (both called at
  src/main/9F9C.c:1180-1181).

## The shuffled ID list `D_801077B0`

`func_800344B4` case 1 (src/main/24BF0.c:54) calls `func_800338CC` and then
`func_80033BC0`.

`func_800338CC` (src/main/17AFC.c:5279):

1. Collects the IDs `i` in 0..26 whose bit is set in
   `D_80106A50.unk_00 & 0x3EF3DF`, minus the bit
   `D_8008D538[D_80102778.unk_4[0]]` (src/main/17AFC.c:5288), into
   `D_801077B0`.
2. Swaps two `rand() % count` positions 108 times (src/main/17AFC.c:5309).
3. If bit 0x20 or 0x10000 of `D_80106A50.unk_00` is set, adds ID 5 (bit 0x20)
   and / or ID 0x10 (bit 0x10000), in random order when both are set: the first
   goes to position 10, shifting the rest up (or at the end of a shorter list),
   and the second is appended.
4. Appends 0x1A or 0x18 when bit 0x04000000 or 0x01000000 is set (which one
   is tested depends on `D_8008D9EC`), stores the count in `D_800A391F`, and
   clears the cursor `D_800A3783`.

`func_80033BC0` (src/main/17AFC.c:5397) steps the cursor: it sets
`D_80102778.unk_4[1] = D_8008D55C[D_801077B0[cursor]]`, or, once the cursor
reaches the count, sets the mode `D_800A3834` to 0x12 (count 20) or 0x20.
The 27 IDs match the row count of the per-character tables (`D_8008E338`,
`D_8008DE34`, include/bb2.h:181-193), which is consistent with the list being
an opponent order; the code does not name it.

## Random numbers

- `rand` (src/main/psxsdk/libc2/rand.c:8) is PsyQ's linear congruential
  generator: `x = x * 0x41C64E6D + 0x3039`, returning bits 16..30.
- `rng_Next` (src/main/6CF8.c:415) is the game's own generator:
  `r = x * 5497 + 0x7FA9`, `x = (x >> 16) ^ r`, returning the low 15 bits;
  `rng_SetSeed` (src/main/6CF8.c:413) sets its state `g_rng_state`.

## Standalone helpers

- `func_8003032C(a0, a1)` (src/main/17AFC.c:3694) takes the angle
  `ratan2(a1[0], a1[2])`, rotates the X/Z pair at `a0[0x11]` / `a0[0x13]`
  into that angle's frame (`>> 12`), negates the second component, rotates back
  with `>> 15` (an eighth of the magnitude), and divides `a0[0x12]` by four. No direct caller appears in `src/`.
- `func_80027640(rec)` (src/main/17AFC.c:178), called from `func_80027AD8`
  (src/main/17AFC.c:370) when `rec->unk_0C == 0xD`, moves the fighter's
  positions (`unk_F4`, `unk_D8`, `unk_B8`) to a target: with
  `D_800A36A4 == 3` a percentage blend from the fighter's position toward the
  point (0x2EE0 or -0x2710, 0x1770 or -0x1770), picked by the other fighter's
  position, with a weight that shrinks from 60 to 10 as the counter `unk_34C`
  (capped at 0x40) grows; otherwise the entry
  `func_80046F14()[D_800A36A4 * 12 + index * 3]`. It then zeroes `unk_104` /
  `unk_134`, switches its move (the script `motion_ResolveMoveData` resolves from
  `unk_50->unk_00` and that record's halfword 0x1D, made current by
  `func_80021A98`) and plays
  cue 0x30.
- `camera_SetFighterFocusView(rec, a1)` (src/main/9F9C.c:980) sets the view target
  `D_800F6608.unk_00` to `rec->unk_174`, its rotation to `(0x120, a1, 0)` and
  its distance `w18` to 0x1162. Its caller is `func_8001EA84`
  (src/main/9F9C.c:2081).
