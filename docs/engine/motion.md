# Motion: move scripts, the frame decoder and the pose

A fighter's animation is driven by two things: the move script it is in
(`MoveScript`, include/game.h:1371; `Unk80101EC8Record.unk_50`), which says
which frames to play and what may follow, and the motion bitstream those
frames are decoded from (`func_800198D0`). Each frame two decoded
`MotionFrame`s are blended into the model's joint matrices. The bundle files
the data comes from are described in [../formats/BBM.md](../formats/BBM.md).

## Loading

- `func_80020DDC` (src/main/9F9C.c:2991) reads file `cdrom_GetFileIndex(1, 1)`
  to `D_800A3830` and points the common record `D_80102760` (include/bb2.h:479)
  at its sections: the move streams at +0x14, then the sections named by the
  header words at +4, +8 and +0x10 (`unk_04`, `unk_08`, `unk_10`).
- `func_80020E74` (src/main/9F9C.c:3016) loads up to two character files
  (`cdrom_GetFileIndex(1, j + 2)`, `j` the model id's index in `D_8008DCCC`)
  into `D_800A3860[0..1]` (`Tbl800A3860Entry`, include/game.h:720-734) and points
  `D_801027B0[i]` (include/bb2.h:482) at their sections, as the comment on
  `Unk801027B0Rec` describes (include/game.h:736). In modes 1, 4 and 6
  (`D_800A38DC`) it also reads seven sectors per fighter from
  `cdrom_GetFileIndex(1, 0)` into `D_800A3888[i]`, a table of ready
  `MotionFrame`s.
- `func_80021210` (src/main/9F9C.c:3107) hands each loaded file's bitstream
  (`unk_10`) to `func_8001979C`, decoder slots 0 (common), 1 and 2.

## Move scripts

`func_80021424(rec, id, out)` (src/main/9F9C.c:3231) turns a move id into a
`MoveScript *`:

- ids 0x7FF0..0x7FFF select through the character file's tables (`f4E`,
  `f54`, `f66` of `D_800A3860[rec->unk_4A]`) using the record's cycling index
  `unk_86` (wrapped by `f14`; for some ids it skips the entries `unk_88` /
  `unk_8E` while their flags `unk_8A` / `unk_90` are clear);
- other ids with bit 15 set index the character file's streams (the
  opponent's when `unk_4C` is set);
- the rest index the common file's streams, and `*out` is set to 1.

`func_80021A98(i, script, common)` (src/main/9F9C.c:3311) makes `script` the
current move of fighter `i`: `unk_50 = script`, `unk_5C = script->unk_04`, and
`unk_54` / `unk_58` point at the script's entry in the file's `unk_04` table
and at the byte stream it names in `unk_08` (the frame-window stream that
[combat.md](combat.md) describes). It also resets the frame counter `unk_40`
from `script->unk_06` and records the previous state in `unk_6C`.

The script's own fields (`unk_00` / `unk_02` follow-up ids, `unk_07` /
`unk_08` frame bounds, `unk_09` flags, and the command list from `unk_0A`)
are walked by `func_80023F08`, as the comment on `MoveScript` records
(include/game.h:1364).

## The frame decoder

`D_800F1B18[slot]` (`Unk800F1B18Rec`, include/game.h:1356) is one decoder per
loaded bitstream.

- `func_8001979C(slot, stream)` (src/main/9F9C.c:64) reads the 63 initial
  12-bit channel values and their 2-bit channel codes and empties the four
  cache slots (`frame = -2`).
- `func_800198D0(slot, n, out, work)` (src/main/9F9C.c:99) decodes frame `n`
  into a 0x84-byte `MotionFrame` (include/game.h:1327). Frames come in groups
  of eight: a 24-bit table at stream + 0x70 gives each group's bit offset;
  the group start holds three optional 16-bit root values and the 63
  channels, and each later frame applies per-channel deltas coded by the
  channel's 2-bit code. Four cache slots keep recent frames and the reader
  state after them: a cached frame is copied out, and the frame after a cached
  one is decoded from it instead of from the group start (the comment above the function,
  src/main/9F9C.c:88, details it).

`MotionFrame.unk_00` is the root offset, `unk_02` / `unk_04` the root heading
and distance, `unk_06..unk_0A` the root rotation angles, and `unk_0C[]` the
per-joint channels (the comment at include/game.h:1317).

## Per-frame update: `func_80023F08`

The fighter loop calls `func_8001BE20(i, pad)` and then `func_80023F08(i, pad)`
for each fighter (src/main/9F9C.c:2040-2043); the variants at :2086 and :2195
call only `func_80023F08`, with a pad cleared by `pad_ClearStateBits`.
`func_80023F08` (src/main/9F9C.c:4337; its comment lists the whole sequence):

1. converts the pad bits, advances the frame `unk_40` and runs the move
   script's command list, switching moves through `func_80021424` /
   `func_80021A98`;
2. decodes the frames at `unk_40` and the next one with `func_800198D0`
   (src/main/9F9C.c:4704-4705), or copies `D_800A3888[i][unk_40]` in state
   0x33; the frame keys go to `unk_64` / `unk_66` as `(slot << 14) | frame`
   (src/main/9F9C.c:4698);
3. turns the two root offsets into a position delta (heading and distance
   through `Judge`, scaled by `unk_1A`), blended by `unk_42` or by `unk_318`
   against the held frame `unk_290`; the weight goes to `unk_68`;
4. integrates the velocities `unk_104` / `unk_134`, resolves the position
   (`func_8002304C`, src/main/9F9C.c:3868), and builds the pose:
   `func_80041188(i, &f0, &f1, unk_68, ...)` (src/main/9F9C.c:4898) and
   `func_80040D48` (src/main/9F9C.c:4901).

## Building the pose

- `func_80041188(i, f0, f1, w, m)` (src/main/31548.c:194): for joints 1..17
  it builds a rotation matrix from each frame's angle triple (the joint's
  offset into the frame comes from `D_80094CFC`) and blends the pair by `w`
  with `func_800523E0` into the model object's node `work` matrices
  (`D_800A9A10[i]`); joints 18 and 19 also blend a translation
  (`func_80044DE4`).
- `func_80040D48(i, mode, pos, rot, ...)` (src/main/31548.c:10) sets the root
  node's position and rotation, completes the joint nodes (mode 0 takes the
  angles from a channel array), runs `func_800417D0` on each of the 18 nodes,
  and queues the nodes on `g_draw_queue_cursor`.

## Other users of the decoder

- `func_8003993C` (src/main/28708.c:1338) rebuilds both fighters from the
  per-frame records `D_800A36EC` (`Rec1C`, include/game.h:1807, written by
  `func_80039680` from `unk_50`, `unk_F4`, `unk_64..unk_68` and others): it
  decodes the two stored frame keys and calls `func_80041188` /
  `func_80040D48` as the live path does.
- `func_8005490C` (src/main/3AB48.c) plays a loaded sequence: a camera
  stream plus per-object motion streams it registers with `func_8001979C`
  and decodes each frame with `func_800198D0`.
