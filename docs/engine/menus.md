# Menus, mode handlers and the scene script

The game's top level is the mode handler table `D_8008D090` (34 entries,
src/main/d_7D870.c:24), called with the mode value `D_800A3834`; see
[main_loop.md](main_loop.md) for the table and the loop. This page covers the
pad masks the handlers test, the handlers in src/main/2B344.c that run the
scene script, and the two menu modules (src/main/596CC.c and
src/main/5ED34.c).

## Pad masks

`pad_Update` (src/main/87A0.c:626) builds each player's 16 button bits as
`~((byte2 << 8) | byte3)` of the controller packet (src/main/87A0.c:659) and
packs the two players into one word, player 0 in the low half
(src/main/87A0.c:674). `g_pad_state.pressed` is the newly pressed bits
(`held & ~previous`, src/main/87A0.c:712; `PadState`, include/game.h:839).
With the standard digital-pad bit order, the masks the handlers use are:

| Mask | Button (either player) |
| --- | --- |
| `0x00100010` | Triangle |
| `0x00200020` | Circle |
| `0x00400040` | Cross |
| `0x00800080` | Square |
| `0x08000800` | Start |
| `0x10001000` / `0x40004000` | Up / Down |
| `0x20002000` / `0x80008000` | Right / Left |

## Mode handlers and the scene script (src/main/2B344.c)

`func_8003B5A4` (src/main/2B344.c:321) steps a byte script at `D_800A3844`
until an opcode ends the step. The opcodes and what they do:

- 0: mode 18 (with `D_800A38A4` from `D_8008D9EC[fighter 0's unk_0A]`), or
  mode 10 when `D_800A380C` is set.
- 1: one operand byte into `D_800A36A4` and the history `D_800A37A8[]`;
  mode 22.
- 2: keeps a 0x1D-byte block in `D_800A3894`, sets up fighter 0
  (`func_8003AF40(0)`), then as 18.
- 3: two operand bytes into `D_80102778.unk_4[1]` / `[3]`; sets up fighter 1
  (`func_8003AF40(1)`, `func_8003AFFC`).
- 16 / 17: `func_8003B3A4` / `func_8003B484` on the operands, which return
  how many bytes they used.
- 18: mode 0.
- 19: keeps a 4-byte block in `D_800A3878`; mode 6.
- 20: keeps a 12-byte block in `D_800A385C` (`func_8003B4DC`,
  `func_8003B56C(1)`).

Handlers that the script's modes reach:

- `func_8003B9D0` (src/main/2B344.c:478, mode 6) loads the scene the
  `D_800A3878` block describes (`func_80054884` with its byte 0 and the
  fighters its flag byte 3 selects), starts its CD audio track (`cdrom_GetFileIndex(5, D_800A3878[1])`) and sets mode 7.
- `func_8003BCB4` (src/main/2B344.c:550, mode 7) counts `D_800A37B8`; while
  `func_80054F68()` is nonzero it waits for Cross. It then pauses the CD and,
  depending on the pending `D_800A3894` / `D_800A385C` blocks, sets mode 0
  or calls `func_8003B5A4` for the next script step.
- `func_8003BE10` (src/main/2B344.c:595, mode 10) resets the scene state
  (`func_80020CDC`, `func_800415C4`, `eff_ClearInitFlag`, `func_8005B72C`,
  `func_80078824(0x80118800)`), starts CD audio
  (`cdrom_GetFileIndex(5, 0x20)`), waits for it, sets mode 0xB and turns the
  display on.
- `func_8003BEA8` (src/main/2B344.c:614, mode 11) waits for Cross (0x40,
  player 0) or `func_800788B0()`, merges `D_800A37A4` into
  `D_80106A50.unk_00` and a per-character bit into `unk_04`, and sets mode
  0x1A when either changed, else 8.
- `func_8003BFC4` (src/main/2B344.c:651, mode 15) runs the FMV overlay
  through `sys_ExecMovieOverlay` and sets mode 8.
- `func_8003C560` (src/main/2B344.c:786, mode 21) plays sounds at frames
  0x1E / 0x43 and, on Cross or after frame 0xF1, sets mode 0x18 or 0x12.
- `func_8003C714` (src/main/2B344.c:822, mode 30) copies the three clock
  records `D_80106A50.times` into the record `func_80077D00()` returns, as
  minutes, seconds and hundredths (its comment, src/main/2B344.c:817).

## The row menu (src/main/596CC.c)

Cited by function name.

- `func_80068F70` sets up the menu block `D_800A34FC`, clears the row index,
  steps it to the first enabled row, and sets the cursor field to 4 (its
  caller is `func_80077820`, src/main/64FD8.c:1249).
- `D_800A34F8` packs three cursors: bits 0-3 the row index 0..7, which skips
  rows whose bit in `D_8009BC04.rows_on` is clear; bits 10-12 a cursor
  0..5; bits 13-15 a cursor 0..2.
- `func_800692C0` reads the pad word and returns, in its high half, 1 or 2
  for a step one way or the other.
- `func_800693CC` (called from `func_80077894`, src/main/64FD8.c:1268) moves
  the row index with that result, applies the row's `opt[].mode` to
  `D_800A3524->unk20_3`, and on Cross (with per-player conditions on the
  row's `state`) returns that `state`; Triangle returns -2, otherwise -1.
- `func_8006B578` steps the bits 10-12 cursor; in positions 0..2 the
  low-half result toggles `D_800A3524->unk20_0..2`, and Cross advances the
  cursor; positions 3, 4, 5 return 2, 3, 1 on Cross, and Triangle returns 1.
- `func_8006B92C` steps the bits 13-15 cursor and returns 2, 3 or 1 on Cross
  (position 2 also advances the bits 10-12 cursor); Triangle returns 1.

## The selection screen (src/main/5ED34.c)

- `func_8006EACC(arg0, pad)` (src/main/5ED34.c:216) keeps the pad word in
  `D_800A354C`, runs `func_8006EC0C`, calls `func_8006F528`
  (src/main/5ED34.c:503) unless the screen mode
  `D_800A3580` is 2 or 3, and calls `D_8009BC1C[D_800A3580]`; it returns
  `D_800A35A0`. Its caller is `func_800779C8` (src/main/64FD8.c:1297).
- `func_8006EC0C` (src/main/5ED34.c:249) slides `D_800A3570` by 0x20 a frame:
  states 1 / 3 (low byte of `D_800A3578`) slide in to 0x1E8 and then switch
  `D_800A3580` to `D_800A3584`; states 2 / 4 slide back to 0, playing sound 5
  as they start.
- `menu_UpdateEntryGridCursors` (src/main/5ED34.c:826) is the grid cursor of that screen
  (see [replay.md](replay.md)).
