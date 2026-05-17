# Menus, UI, Fades, and Mode Transitions

BB2's menu and mode-transition logic is a state machine driven by
`D_800A3834` (the main game-mode value) and a set of per-mode sub-state
variables. The transitions are scattered across `code6cac_c2.c`,
`code6cac_c_ab.c`, `code6cac_c_mid.c`, and `config.c`.

## Mode dispatch recap

Every frame, the main loop calls
`g_module_func_tbl[D_800A3834]()`. The functions in `g_module_func_tbl`
range from "title FMV" through "character select" to "fight" to "results
screen" to "training mode". Each handler is responsible for:

1. Drawing its own UI (HUD, menu text, cursor)
2. Reading the pad (`D_80102794`)
3. Mutating `D_800A3834` to transition to the next mode

See [main_loop.md](main_loop.md) for the mode value table.

## Menu transition: a typical mode handler

`md_game_check_change_sub_mode` (`code6cac_c2.c:252`) is a model
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

## Fade-in / fade-out — `g_disp_fade`

`g_disp_fade` (`0x800A36A8`) is a 0..255 fade-to-black amount applied to
the framebuffer. The fade is implemented as a fullscreen black quad
inserted into the OT at increasing alpha.

Helpers:
- `FadeOut_8003FFA8` / `FadeOut_8003FFC4` (asm-only) — animate fade out
- `CheckFadeEnd` (asm-only) — query fade complete flag
- `InitFadePanel` (asm-only) — set up the fullscreen black quad in the OT

`g_disp_fade` is checked at the start of most menu handlers; the menu is
not interactive while a fade is in progress.

## Configuration / options — `config.c`

`config.c` (`0x8003F168..0x800401CC`) handles game options:

- `game_GetMode` / `game_SetControllerPorts` / `game_SetPlayerCount` —
  basic options.
- `stage_ExecInitFunc` — picks the per-stage initialization function from
  `g_stage_init_tbl[stage_id]`.
- `stage_InitCollision` — builds the 32x32 collision grid for the
  current stage.
- `md_option_reset_*` family (lines 518-535) — reset options to defaults.

The stage init table `g_stage_init_tbl` (`0x800948BC`) is an array of
function pointers — one per stage, called when entering that stage.

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

- `chractar_use_pset_combo_id_table` (`0x8009BA7C`) — per-character pset
  combo table
- `g_player_char_ids` (`0x80094B88`) — final selected char IDs [P1, P2, AI]

The mode handler for character select sets one entry of `g_player_char_ids`
per frame as the user moves the cursor and presses CROSS to commit.

## Stage select

`g_stage_id` (`0x80099478`) is the selected stage. The stage-select menu
dispatches the per-stage initialization via `g_stage_init_tbl`.

## Mode entries (handlers that set up modes)

`func_8003BE10` (`code6cac_c2.c:315`) — "enter mode 0xB" handler:
1. Enable display, init display
2. Reset players, file DMA, all objects
3. Bring in setup table (`func_80078824(0x80118800)`)
4. Set up camera matrix (`func_80035FA8`)
5. Load mode-specific motion (`func_80036FD4`)
6. `D_800A3834 = 0xB` (transition complete)
7. Disable display (for next frame's fade-in)

`func_8003BFC4` (`code6cac_c2.c:397`) — "go to mode 8" handler:
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

`game_FrameLoop` (`code6cac_b2.c:471`) is a smaller per-frame "play a
canned animation" loop used by title and demo modes — it spins until
either a fixed frame count or a button press.

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
| `g_menuctl_pad_result_buf` | `0x800A350C` | s16 pad-decode result base (4 entries: 350C/350E/3510/3512) |
| `g_menuctl_frame_counter` | `0x800A3514` | Per-frame tick counter |

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

Several menu-related mode handlers exist in `g_module_func_tbl`
(see [main_loop.md](main_loop.md) for full table):

- Mode 30 (`SetCurrentCursor`, `0x8003C714`) — menu cursor positioning
- Mode 11 (`mode_handler_11_PadInputCheck`, `0x8003BEA8`) — pad input
  check (mask `0x40` = action button)
- Mode 7 (`mode_handler_07_SubModeTransition`, `0x8003BCB4`) —
  sub-mode transition triggered by cross+circle (`0x400040`)
- Mode 21 (`mode_handler_21_FrameTimerSfx`, `0x8003C560`) — plays
  SFX 0xA4/0xA7 at frame counter == 0x1E

## Fade / transition state machine (2026-05-17)

The fade-in/fade-out state machine at `func_8006EC0C` (`text1b.c:16108`)
ramps a value (0..0x1E8 in 0x20 steps) under control of a dispatch
table:

| Symbol | Address | Role |
|--------|---------|------|
| `g_fade_value` | `0x800A3570` | s16 ramp value, 0..0x1E8 (488) |
| `g_fade_dispatch_idx` | `0x800A3580` | Index into g_fade_dispatch_table |
| `g_fade_next_dispatch_idx` | `0x800A3584` | Deferred next (loaded when ramp completes) |
| `g_fade_dispatch_table` | `0x8009BC1C` | 5+ function-pointer entries |

State values (1-4):
- 1 = ramp up
- 2 = fade out
- 3 = ramp up alt
- 4 = fade out alt (plays sfx 5 at peak)

Note: `D_800A3578` (fade state value) is also named
`g_replay_motion_shared_state_a` — the address is reused across the
fade + replay-camera + motion-shift subsystems.
