# Menus, UI, Fades, and Mode Transitions

BB2's menu and mode-transition logic is a state machine driven by
`D_800A3834` (the main game-mode value) and a set of per-mode sub-state
variables. The transitions are scattered across `code6cac_c2.c` and
`code6cac_c_mid.c` (the former `code6cac_c_ab.c` is now the head of
`code6cac_c2.c`, the former `config.c` its tail).

## Mode dispatch recap

Every frame, the main loop calls
`D_8008D090[D_800A3834]()`. The functions in `D_8008D090`
range from "title FMV" through "character select" to "fight" to "results
screen" to "training mode". Each handler is responsible for:

1. Drawing its own UI (HUD, menu text, cursor)
2. Reading the pad (`D_80102794`)
3. Mutating `D_800A3834` to transition to the next mode

See [main_loop.md](main_loop.md) for the mode value table.

## Menu transition: a typical mode handler

`md_game_check_change_sub_mode` (`code6cac_c2.c:234`) is a model
sub-mode handler. Annotated:

```c
void md_game_check_change_sub_mode(void) {
    D_800A37B8++;                           // frame counter

    if (func_80054F68() != 0) {              // some external block?
        if ((D_80102794 & 0x400040) == 0) {  // no CROSS button
            return;                          // ... keep waiting
        }
    }

    func_800372C0();                         // cancel-and-cleanup
    katinuki_game_setData_800548DC();        // commit menu choice

    if (D_800A38DC != 0) return;             // sub-mode busy

    if (D_800A3894 != 0) {                   // some pending action
        D_800A3834 = 0;                      // go to "no mode" (idle)
        switch (D_800A37B0) {                // sub-state
            case 1: case 2: case 4: case 5:
                D_800A3907++;
                return;
            case 3:
                func_8003AF40(0);
                md_menu_logo_exec();
                /* fall through */
            case 6:
                D_800A3894 = 0;
                goto call_bar;
            default:
                return;
        }
    }

    if (D_800A385C != 0) {                   // sub-mode finalize
        s32 val = D_800A390C;
        if (val == 1) { D_800A3834 = 0; return; }
        if (val == 0 || val >= 4) return;
        D_800A385C = 0;
    }

call_bar:
    func_8003B5A4();                          // post-transition setup
}
```

Note `0x400040` = CROSS button (both P1 and P2) — the magic pad-input
masks are everywhere in the menu code:

- `0x100010` = SELECT
- `0x400040` = CROSS (confirm)
- `0x200020` = SQUARE
- `0x800080` = CIRCLE
- `0x10001000` = UP
- `0x20002000` = RIGHT (with hint about another button)
- `0x40004000` = DOWN
- `0x80008000` = LEFT

(Each mask is `(P2 button) | (P1 button)` because `D_80102794` packs both
players in one 32-bit word.)

## Fade-in / fade-out

`D_800A36A8` (`0x800A36A8`) is not a fade amount: it is a 0/1 switch that
multiplies the `func_8005D554` sprite passes (3AB48.c:4307-4310, :4349).

Helpers:
- `FadeOut_8003FFA8` / `FadeOut_8003FFC4` (asm-only) — animate fade out
- `CheckFadeEnd` (asm-only) — query fade complete flag
- `InitFadePanel` (asm-only) — set up the fullscreen black quad in the OT

## Configuration / options — tail of `code6cac_c2.c`

The former `config.c` (`0x8003F168..0x800401CC`, now the tail of
`code6cac_c2.c`) handles game options:

- `game_GetMode` / `game_SetControllerPorts` / `game_SetPlayerCount` —
  basic options.
- `func_8003F168` — calls the `.init` hook of `D_800948BC[func_80046798()]`
  when it is non-null.
- `func_8003F274` — rebuilds the 32x32 grid `D_800A8FB0` around the view
  node; the draw walks read it.
- `md_option_reset_*` family (lines 518-535) — reset options to defaults.

`D_800948BC` is an array of `{init, unk4}` function-pointer pairs indexed by
`func_80046798()`: `func_8003F168` calls `.init`, the draw walk `func_8003E6D8`
calls `.unk4`.

## Cursor / menu state primitives

| Function | Purpose |
| --- | --- |
| `SetCurrentCursor` (asm-only) | Update the menu cursor sprite position |
| `Pad_Prs` (asm-only) | Press-event helper (debounced pad input) |
| `pad_press_control` (asm-only) | Per-frame pad-press accumulator |
| `pad_main_control` (asm-only) | Top-level pad input dispatch |
| `pad_button_info_clear` | Clear per-frame pad state |

These are called from the menu handlers to manage cursor movement and
"confirm/cancel" responses.

## Mental bar HUD

`saTan*GaugeInit` / `saTan*GaugeMain` family (see [combat.md](combat.md))
renders the in-game mental gauge. The "mental" stat is BB2's spirit/focus
meter that controls some advanced moves' availability.

## Character select

The character select screen uses `selCharaID` (`0x80102092`) as the
selected-character index. The per-character setup table is at
`g_char_setup_tbl` (`0x80094E48`). Selection is driven by:

- `D_8009BA7C` (`0x8009BA7C`) — 62 function pointers called through by
  `func_80060A68` / `func_80060B70` (no character role shown)
- `D_80094B88` (`0x80094B88`) — per-slot 5-bit codes stored by `func_80041604`
  (callers pass `D_800A36C8` / `D_800A36F4` / `D_800A376A`); not shown to be
  character ids

## `D_80099478`

`D_80099478` caches the id `func_800460E4` was last called with (-1 =
none); `func_80046798` returns it, and `func_8003F168` / `func_8003E6D8` index
`D_800948BC` with it.

## Mode entries (handlers that set up modes)

`func_8003BE10` (`code6cac_c2.c:294`) — "enter mode 0xB" handler:
1. Enable display, init display
2. Reset players, file DMA, all objects
3. Bring in setup table (`func_80078824(0x80118800)`)
4. Set up camera matrix (`func_80035FA8`)
5. Load mode-specific motion (`func_80036FD4`)
6. `D_800A3834 = 0xB` (transition complete)
7. Disable display (for next frame's fade-in)

`func_8003BFC4` (`code6cac_c2.c:376`) — "go to mode 8" handler:
1. Enable display
2. Reset players, file DMA
3. `func_80045814()` — get next-stage param
4. `func_80037540(v, 0x80118000, 1, 0xCF8, 0xB01)` — submit setup command
5. `game_Init()` — re-init game state (resets pause, mirror, P1/P2 ctrl)
6. `D_800A3834 = 8`

This pattern (enable display + reset state + load assets + set mode value)
is the standard mode-transition idiom.

## Menu text rendering

`DispPracticeMenuTex_A/B/C` (asm-only at `0x80017FA0` etc.) render the
practice-mode text. They're 16x16 character tile renderers using TIM
textures loaded into VRAM.

`DispSleepMenuTex` (asm-only) — "press start" sleeping screen text.

`DispUpdateStatusMessage` (asm-only) — overlay messages like "DRAW!" /
"FINISH!" / "VICTORY!".

`disp_mario_jimaku` / `disp_mario_jimaku2` (`main.c`, asm-only) —
"super-cam dialogue" subtitles. "Jimaku" = subtitle. "Mario" is Kengo's
naming for "Marionation" — i.e., the Marionation engine's subtitle system.

## Mode select / title

The title-screen logo handler is `md_menu_logo_exec` (`0x8003AFFC`,
asm-only). It runs the boot-screen logo animation, then transitions to
the mode-select menu.

`title_mv_exec` / `title_mv_exec2` (referenced) handle the title FMV
(plays OPENING.STR via the MOVOVL.EXE overlay).

`func_80036F40` (`code6cac_b5_post.c`) is a wait entered around CD reads:
it runs per-frame work and `VSync(2)` until `cdrom_IsIdle()` returns
non-zero.

## Replay / training overlap

Many menu handlers also drive training/replay modes. The `cpu_*` AI
functions (see [ai.md](ai.md)) handle the training-mode AI; the menu
layer just chooses which CPU mode to enable.

## Where unmatched menu asm still hides

- Most `md_*` mode handlers in `main.c` (asm-only at 0x80083A48 family)
- Most of the `DispXxx` text renderers (asm-only)
- All `pad_*` controller helpers (asm-only)
- `_DispCharacterName` (asm-only at 0x80080258)
- `md_game_check_change_main_mode_default` (asm-only at 0x80083A48)

## Menu-control state cluster (2026-05-17)

Discovered via cluster-consumer analysis — 6+ consumer functions all
reference these together (highest "naming multiplier" cluster).

| Symbol | Address | Role |
|--------|---------|------|
| `g_menuctl_state_bitfield` | `0x800A34F8` | Packed cursor/slot state |
| `g_menuctl_mode_state_ptr` | `0x800A34FC` | Current screen's state struct ptr (from `func_8006E49C`) |
| `D_800A350C` | `0x800A350C` | s16[2]: `func_800692C0`'s per-axis step (1 / -1 / -6 / 0), added to `D_800A34FC->unk_0C[i]` |
| `D_800A3514` | `0x800A3514` | Counter: += 1 per handler call or cursor-row draw; the rsin phase of a pulsing colour |

### Bitfield layout of `g_menuctl_state_bitfield`

- **bits 0-3**: rotating slot index (cycles 0..15 via the bit-cache
  at `D_8009BC04`)
- **bits 10-12**: sub-cursor (advanced by case-2 action handlers)
- **bits 13-15**: main cursor position (incremented/decremented by
  directional input)

### Menu input handler — `func_8006B92C`

Per-frame menu input handler:
1. Reads pad input (combines P1/P2 controllers high/low halves)
2. Calls `func_800692C0` to decode direction (code 1 = down, 2 = up)
3. Direction code → navigate cursor (incr/decr bits 13-15)
4. Each cursor position has its own action when cross/circle pressed
   (mask `0x400040`)
5. Plays menu sounds via `func_8005C650(N, 0x7F, 0x7F)`

### Cross-reference with mode handlers

Several menu-related mode handlers exist in `D_8008D090`
(see [main_loop.md](main_loop.md) for full table):

- Mode 30 (`SetCurrentCursor`, `0x8003C714`) — menu cursor positioning
- Mode 11 (`func_8003BEA8`, `0x8003BEA8`) — pad input
  check (mask `0x40` = action button)
- Mode 7 (`mode_handler_07_SubModeTransition`, `0x8003BCB4`) —
  sub-mode transition triggered by cross+circle (`0x400040`)
- Mode 21 (`func_8003C560`, `0x8003C560`) — plays
  SFX 0xA4/0xA7 at frame counter == 0x1E

## Fade / transition state machine (2026-05-17)

The fade-in/fade-out state machine at `func_8006EC0C` (`text1b_tu1d.c:362`)
ramps a value (0..0x1E8 in 0x20 steps) under control of a dispatch
table:

| Symbol | Address | Role |
|--------|---------|------|
| `D_800A3570` | `0x800A3570` | s16 slide offset, 0..0x1E8 (488); an x displacement / width in `func_8006F528` |
| `D_800A3580` | `0x800A3580` | The module's current mode; indexes D_8009BC1C, D_8009BCC4 and the menu table |
| `D_800A3584` | `0x800A3584` | The next mode, copied into D_800A3580 when D_800A3570 reaches 0x1E8 |
| `D_8009BC1C` | `0x8009BC1C` | 7 handler entries, called by `func_8006EACC` with `&sp10`; [1..3] drive the full-screen tile's grey level, [0] and [4..6] do not |

State values (1-4):
- 1 = ramp up
- 2 = fade out
- 3 = ramp up alt
- 4 = fade out alt (plays sfx 5 at peak)

Note: `D_800A3578` (fade state value) is a `5ED34.c` static: this fade
reads its low byte, and the rest of the file stores and tests the whole
halfword as its selection-screen state.
