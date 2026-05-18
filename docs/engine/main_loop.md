# Boot Sequence and Main Loop

This document traces what happens from power-on through one frame of gameplay.

## Boot

The PS1 BIOS hands control to BB2 at address `0x8008_36EC`. This is declared
in the PS-EXE header (`asm/header.s:6`) and corresponds to the entry point
labelled `ang_hosei` in `asm/funcs/ang_hosei.s` — the label is just whatever
came first in the address range; the entry code is NOT actually `ang_hosei`
the rotation-correction helper. The two share an asm file because they were
adjacent in the binary.

Annotated entry-point code (from `asm/funcs/ang_hosei.s:12-55`):

```
; ------- ENTRY @ 0x800836EC -------
loop:                          ; clear BSS from D_800A3308 up to D_801078E0
    lui   $v0, %hi(D_800A3308) ; v0 = 0x800A3308 (BSS start)
    addiu $v0, $v0, %lo(...)
    lui   $v1, %hi(D_801078E0) ; v1 = 0x801078E0 (BSS end)
    addiu $v1, $v1, %lo(...)
.bss_clear:
    sw    $zero, 0($v0)
    addiu $v0, $v0, 4
    sltu  $at,  $v0, $v1
    bnez  $at,  .bss_clear

    ; Set up stack at top of RAM
    lui   $v0, %hi(D_800A2690)
    lw    $v0, %lo(D_800A2690)($v0)
    addi  $v0, $v0, -8
    lui   $t0, %hi(D_80000004)   ; KSEG1 offset
    or    $sp, $v0, $t0          ; SP = top - 8, with KSEG1 alias
    ...
    or    $a0, $a0, $t0          ; argv[0] in KSEG1
    sw    $a0, %lo(D_800A266C)($at)
    sw    $ra, %lo(D_800A3668)($at)

    ; Set up $gp = 0x800A30CC and $fp = $sp
    lui   $gp, %hi(_gp)
    addiu $gp, $gp, %lo(_gp)
    addu  $fp, $sp, $zero

    jal   func_8008386C          ; BIOS B(39h) - InitHeap
     addi $a0, $a0, %lo(D_80000004)

    lw    $ra, %lo(D_800A3668)($ra)
    jal   cpu_set_move_command_and_dir_for_no_action_2  ; <-- the actual C entry
     nop
    break 0, 1                   ; never returns
```

This is the standard PsyQ crt0 boilerplate: zero BSS, set `$sp`/`$gp`/`$fp`,
call `InitHeap`, then jump to "main". On BB2 the "main" function is named
`cpu_set_move_command_and_dir_for_no_action_2` because the Kengo name table
collided two functions of the same name at different addresses. It is NOT
actually a CPU-AI helper; it's the main entry. See `ings.c:584`.

## C entry point — `cpu_set_move_command_and_dir_for_no_action_2`

Full body at `ings.c:584-675`. Structure:

```c
void cpu_set_move_command_and_dir_for_no_action_2(void) {
    /* phase 1: bring up engine */
    motion_Open();                 // run CTORs in g_module_func_tbl
    func_800789D8(0x801FFF00);     // libapi InitTimer / PadInit
    func_80078968(2);              // VSync wait config
    sys_Init();                    // libgpu/libcd/libspu init + disp_Init
    sys_GameInit();                // load game data, init game subsystems
    gpu_SetDispMask(1);            // turn the display ON
    func_80016A8C(0x80118800);     // load splash/logo bitmap

    /* phase 2: per-frame state */
    tbl = &D_800A3770;             // pointer to OT/buffer-base table
    D_800A3834 = 0xF;              // initial game mode
    D_800A390D = 0;
    D_800A36AC = 0;

loop:
    /* per-frame work */
    idx = D_800A36AC & 1;
    env = &D_800F7438 + idx*0x4090;  // pick draw env A or B
    ot  = env + 0x70;
    func_8007B844(ot, 0x1008);     // clear OT
    D_800A374C = ot;
    D_800A38B4 = tbl[idx];

    gnd_get_fog(idx);              // stage fog (per buffer)
    change_shadow_tex_reg();        // shadow texture page swap
    single_game_VoiceContorol(...);  // SPU voice update
    special_camera_Exec();          // camera per-frame
    func_8005C6D0();                // ground/stage frame update

    /* handle global reset request */
    if (D_800A3928 != 0) {
        func_800372C0();
        D_800A3768 = 0xFF;         // hide display while resetting
        D_800A3928 = 0;
        D_800A31DA = 0;
        D_800A3834 = 8;            // jump to title/menu mode
    }

    /* CALL THE CURRENT GAME MODE */
    ((void (*)(void))(&D_8008D090)[D_800A3834])();  // **MODE DISPATCH**

    ReturnVTMenu();                // late housekeeping

    /* wait for vsync window */
    do {
        if (func_80078B04(0xF2000001) >= ((D_800A36F1-1)<<8) + 0x80) break;
        func_80079154();
    } while (1);

    sys_VSync(1);                  // poll counter
    gpu_DrawSync(0);               // wait for current GPU draw
    sys_VSync(0);                  // wait for next vblank
    func_80078BA8(0xF2000001);

    /* submit frame to GPU */
    voice = D_800A390D;
    if (voice == 0) {
        func_8007BC08(env + 0x5C); // PutDispEnv
        func_8007B9B0(env);        // PutDrawEnv
    }

    /* check OT overflow */
    {
        s32 adj = D_800A38B4 + 0xFFFECC00;
        s32 remaining = tbl[idx] - adj;
        if (remaining < D_800A30DC) D_800A30DC = remaining;
        if (remaining < 0) {
            debug_printf(&D_80010034);  // "common prim over flow"
            while (1) func_800164F8();   // hang (assert)
        }
    }

    /* flip / send OT to GPU */
    if (D_800A390D != 0) {
        D_800A390D--;
    } else {
        gpu_DrawOTag(env + 0x408C);   // **frame submit**
        D_800A36AC++;
    }

    /* event-driven exits to "pause / select menu" mode */
    if (D_800A3834 != 1) goto loop;
    if (voice != 0)      goto loop;
    if (D_80102794 & 0x08000800) goto call_func;  // SELECT pressed
    if (D_800A38DC != 2) goto loop;
    if (D_800A3713 == 0) goto loop;
    D_800A3713--;
    if (D_800A3713 != 0) goto loop;
call_func:
    func_80016E60(env);    // pause/select submenu
    goto loop;
}
```

This is the entire main loop. It's a single forever-loop that:

1. Picks a double-buffer slot from `D_800A36AC & 1`.
2. Updates per-frame "always on" subsystems: fog, voice control, special
   camera, ground/stage.
3. Honors a global reset request (`D_800A3928`).
4. Dispatches to the current mode function via `g_module_func_tbl[D_800A3834]`.
5. Waits for vsync window.
6. Submits the built OT to the GPU.
7. Optionally enters a pause/select submenu (`func_80016E60`).

The mode dispatch in step 4 is the heart of the engine — it's how the game
moves between title screen, character select, gameplay, replay, etc.

## Boot subsystems

### `sys_Init` (`ings.c:300`)
Disables interrupts, initializes the pad buffer (`func_80078C9C` — libapi
`InitPAD`), sets up the controllers (`func_80078D38` — `StartPAD`), the VSync
event (`func_80078A58`), the display (`disp_Init`), camera state
(`func_80035FE0`), pad-press state, then `sys_InitSound`.

### `sys_GameInit` (`ings.c:414`)
- Prints `"LIMIT:%08x"` debug
- `kgm_clamp_patch_init` — zero file/disc flags
- `func_80020D70` — early game data init
- Sets `D_800A3770=0x801D8800`, `D_800A3774=0x801EBC00`, `D_800A3798=0x13400`
  — the overlay-region scratch buffers
- `file_LoadSoundData` — loads SE bank from disc to RAM
- `func_80019534`, `katinuki_game_setData_8003D2C4`, `func_8001C444` —
  game-data tables setup
- `game_Init` (`sound.c:242`) — initializes effect, gauge, character, and
  pause state. Default state: `g_game_pause = 1`, `g_game_p1_ctrl = 0`,
  `g_game_p2_ctrl = 2`, `g_game_timer = 0x23`.

### `motion_Open` (`ings2.c:525`)
Runs an array of "constructor" function pointers stored at `D_8008D070`
through the end-marker `D_00000000`. The loop is hand-coded asm because it
modifies `$s1` (the counter) as a side effect of each call. See also
`motion_Close` (`ings2.c:543`), which is the symmetric tear-down.

This is the engine's module init/cleanup mechanism — every subsystem that
needs early initialization adds itself to the `D_8008D070`-rooted array at
link time, and is run automatically at boot. The mechanism is NOT used for
the per-frame mode dispatch, which is a separate array starting at
`D_8008D090`.

## Mode dispatch — `g_module_func_tbl` at `D_8008D090`

The main loop's `((void (*)(void))(&D_8008D090)[D_800A3834])()` call uses
`D_800A3834` (the current "main game mode" / `g_practice_mode_dispatch_idx`)
as an index into a 34-entry table of function pointers. Each function is one
frame of that mode.

**The table is statically initialized**, not BSS as previously thought — it's
defined inline in `main.c:3055` as a `.global g_module_func_tbl` block of 34
`.word` entries. Earlier docs assumed BSS-init based on the entries being
zero in `asm/data/7D920.data.s`; the actual definition is in the inline asm
of `main.c`.

### Complete 34-mode dispatch table (decoded 2026-05-17)

| Mode | Address | Handler | Role |
|------|---------|---------|------|
| 0  (0x00) | 0x8001DCB0 | `mode_handler_00_GameInit` | **Game init**: obj_InitChars + gpu_InitDisplay/Disable + gnd_disp_loop_ctrl + gnd_open |
| 1  (0x01) | 0x8001E878 | `mode_handler_01_GameFrameUpdate` | **Main per-frame fight**: camera, characters, collision, motion, stage tick |
| 2  (0x02) | 0x80033898 | `gpu_enable_and_state_reset_80033898` | Display reset + transitions to mode 3 |
| 3  (0x03) | 0x80034708 | `mode_handler_03_NoOp` | Empty (no-op placeholder) |
| 4  (0x04) | 0x800397D4 | `mode_handler_04_GameSetup` | **Full game setup**: gpu_EnableDisplay + gnd_open + player count + DMA list |
| 5  (0x05) | 0x8003993C | `mode_handler_05_NoOp` | Empty |
| 6  (0x06) | 0x8003B9D0 | `mode_handler_06_GameTeardownVariant` | func_8001DA2C + game_Cleanup + conditional GPU |
| 7  (0x07) | 0x8003BCB4 | `mode_handler_07_SubModeTransition` | md_game_check_change_sub_mode + pad input check |
| 8  (0x08) | 0x80035480 | `scene_teardown_variant_80035480` | Scene cleanup variant; also the **global-reset target** (D_800A3928 trigger) |
| 9  (0x09) | 0x80035828 | `mode_handler_09_NoOp` | Empty |
| 10 (0x0A) | 0x8003BE10 | `mode_handler_10_GameTeardown` | gpu_EnableDisplay + player_Destroy(0/1) |
| 11 (0x0B) | 0x8003BEA8 | `mode_handler_11_PadInputCheck` | Checks pad input mask 0x40 (action button) |
| 12 (0x0C) | 0x8001EA04 | `mode_handler_12_RoundCleanup` | gnd_init_80041688(0/1) + game_Cleanup; end-of-round |
| 13 (0x0D) | 0x8001EA84 | `cpu_get_move_pattern_table_number` | CPU AI move-pattern lookup |
| 14 (0x0E) | 0x80035430 | `mode_handler_14_NoOp` | Empty |
| 15 (0x0F) | 0x8003BFC4 | `mode_handler_15_TeardownVariant` | Variant of mode 10 |
| 16 (0x10) | 0x8001EEB4 | `hirahira_w_frie2` | "Falling/particles 2" — likely petal/snow effect |
| 17 (0x11) | 0x8001EFA0 | `mode_handler_17_GameContinueFrame` | Increments g_practice_loop_frame, camera_GetBoneData |
| 18 (0x12) | 0x8003C040 | `mode_handler_18_UnlockAnimDispatch` | **Unlock-celebration dispatch**: reads g_practice_unlock_anim_id (6/7=P1, 8/9=P2) |
| 19 (0x13) | 0x8003C2C0 | `cpu_side_move_dir_2` | CPU AI sidestep direction |
| 20 (0x14) | 0x8003C42C | `mode_handler_20_CountIterator` | Counts D_800A377C[] entries into 8-cell histogram |
| 21 (0x15) | 0x8003C560 | `mode_handler_21_FrameTimerSfx` | Plays SFX 0xA4/0xA7 at counter==30 frames |
| 22 (0x16) | 0x8003B870 | `mode_handler_22_VsModeInit` | **VS mode init**: player_SetCharId(0/1) + obj_InitChars + disp_SetFramebufferMode(1) |
| 23 (0x17) | 0x8003B8E4 | `mode_handler_23_FrameDelay3` | Returns until frame counter >= 3 |
| 24 (0x18) | 0x8003C958 | `mode_handler_24_DispatchToMode25` | gpu_InitDisplay + reset state + sets dispatch_idx = 0x19 |
| 25 (0x19) | 0x8003C9A4 | `mode_handler_25_PostBattleSetup` | Reads g_gnd_midpoint_x, game_SetControllerPorts(0) |
| 26 (0x1A) | 0x80035DC8 | `scene_teardown_80035DC8` | Scene cleanup |
| 27 (0x1B) | 0x80035E38 | `saRobDraw` | Draws robot AI (saRob = "sa" team rob) |
| 28 (0x1C) | 0x8003CE18 | `mode_handler_28_PostBattleMisc` | func_8001DA2C + func_800372C0 |
| 29 (0x1D) | 0x8003CF84 | `mode_handler_29_StageLeafUpdate` | mk_leaf_newpos + reads char struct fields |
| 30 (0x1E) | 0x8003C714 | `SetCurrentCursor` | Menu cursor positioning |
| 31 (0x1F) | 0x8003C8B4 | `mode_handler_31_TimerLoop` | Counts up to 241 frames or pad-input exit |
| 32 (0x20) | 0x8003CCCC | `mode_handler_32_RebootDispatch` | gpu_InitDisplay + game_Cleanup + dispatch to 0x21 |
| 33 (0x21) | 0x8003CD10 | `mode_handler_33_RebootBegin` | Mirrors mode_25 setup; final teardown |

### Observed mode transitions and state flow

From dispatch-assignment evidence in the source, several mode-flow chains
are visible:

**Game flow (main fight):**
```
mode 4 (GameSetup) --> mode 1 (GameFrameUpdate, looped)
mode 1 --> mode 18 (when lesson completes + P1/P2 unlock pending)
        --> mode 12 (round cleanup)
        --> mode 8 (title -- via global reset)
```

**Lesson / practice flow:**
```
func_80033DF4 (lesson check):
  if (D_800A38E2 == 0x64 && unlocks pending):
      sets g_practice_unlock_anim_id = 6/7/8/9
      sets mode_dispatch_idx = 0x12  --> mode_handler_18_UnlockAnimDispatch
      mode 18 reads unlock_anim_id and runs the right celebration

func_80033FE4 (unlock dispatch):
  P1 unlocked  --> sets unlock_id = 6 or 7
  P2 unlocked  --> sets unlock_id = 8 or 9
  else if D_800A38E9 < 3  --> mode 0x1A (scene_teardown_80035DC8)
  else                    --> mode 8 (title)
```

**Post-battle reboot sequence:**
```
mode 22 (VsModeInit) --> mode 23 (FrameDelay3, wait 3 frames)
                     --> mode 24 (DispatchToMode25)
                     --> mode 25 (PostBattleSetup)
                     --> mode 31 (TimerLoop, up to 241 frames or pad)
                     --> mode 32 (RebootDispatch, sets dispatch to 0x21)
                     --> mode 33 (RebootBegin, final teardown)
```

### Notable observations from the table

1. **5 modes (3, 5, 9, 14) are empty placeholders.** These are no-op handlers
   reserved for state transitions where the dispatch mechanism is used but no
   per-frame work happens. The next frame's dispatch handles the actual logic.

2. **Mode 8 is the "global reset target".** When `D_800A3928 != 0` triggers a
   global reset, the main loop forces `g_practice_mode_dispatch_idx = 8` and
   thus jumps to `scene_teardown_variant_80035480`. Many other paths also
   end at mode 8 to reach the title.

3. **Mode 18 is the unlock-celebration dispatcher.** It bridges the practice-
   mode lesson-progress flow (modes 1/4/etc.) to the unlock-animation modes
   selected via `g_practice_unlock_anim_id`.

4. **Modes 16, 19, 27, 30** are Kengo-derived names that don't fit the
   `mode_handler_NN_*` pattern but still map to specific game features
   (particles, CPU AI, robot draw, menu cursor).

5. **The table is statically defined** in `main.c:3055` inline asm. This
   contradicts an earlier note that suggested runtime population — the
   "module init" mechanism at `D_8008D070` is separate (CTOR pointers, not
   the dispatch table).

## Per-frame order of operations (one iteration of `loop:`)

1. **Frame slot pick** — `D_800A36AC & 1` selects buffer A or B.
2. **OT clear** — `func_8007B844(ot, 0x1008)` resets the ordering table to
   end-terminators.
3. **Per-frame "always on" subsystems** (regardless of mode):
   - `gnd_get_fog(idx)` — set fog color/distance based on stage
   - `change_shadow_tex_reg()` — update shadow texture page
   - `single_game_VoiceContorol()` — feed SPU voices
   - `special_camera_Exec()` — special-camera position update
   - `func_8005C6D0()` — ground-state per-frame update
4. **Global reset** — `D_800A3928 != 0` triggers fade-to-black and reroutes
   to mode 8 (title).
5. **Mode dispatch** — call `g_module_func_tbl[D_800A3834]()`. This is where
   gameplay logic runs.
6. **Late housekeeping** — `ReturnVTMenu()`.
7. **Wait for vsync window** — polled loop on timer counter.
8. **GPU sync** — `sys_VSync(1)`, `gpu_DrawSync(0)`, `sys_VSync(0)`.
9. **Submit env to GPU** — `PutDispEnv`/`PutDrawEnv` via
   `func_8007BC08`/`func_8007B9B0`.
10. **OT overflow check** — assert if the primitive pool didn't fit.
11. **Send OT** — `gpu_DrawOTag(env + 0x408C)` kicks the GPU DMA.
12. **Frame counter advance** — `D_800A36AC++`.
13. **Optional pause/select** — if `D_800A3834 == 1` and SELECT is pressed,
    detour to `func_80016E60(env)`.

Steps 5 and 11 are the only ones that vary by game state. Everything else is
the same code path every frame.

## Per-mode draw path — `gnd_disp_loop_ctrl`

The mode-1 (gameplay) handler is part of `gnd_disp_loop_ctrl`, defined at
`ings.c:678`. It's the "actually draw a frame of gameplay" function:

- Checks `g_disp_enable == DISP_DISABLED` (0xFF) and returns immediately if
  the display is hidden.
- Uses its own internal s0/s1 register pinning to set up two GPU packet
  buffers.
- Walks the ground/scene draw pipeline using `s2` as a pointer to
  `g_disp_state_buf` (`0x800F33D8`, a 512-byte draw-state struct; see
  [recent_naming_findings.md §17](recent_naming_findings.md#17-display-state-buffer--cursor-d_800f33d8--d_800a36ec)).

This function works closely with the OT in `g_dma_buf_base` (D_800A374C) (the live OT pointer)
and `g_cpu_move_pattern_cursor` (D_800A38B4) (the live primitive heap pointer).

## Pause / select submenu — `func_80016E60`

Triggered by SELECT during gameplay (`D_80102794 & 0x08000800`). Defined at
`ings.c:443`. It's a self-contained inner loop:

- Saves arg0 (the env pointer) to s5
- Runs a multi-choice menu: `limit = special ? 6 : 3` choices
- Reads pad input via the standard `D_80102794` mask
- Returns selection in `select`; values 1 and 2 set `D_800A3834 = 8` (jump
  back to title)

This is the in-game pause menu. The choices are inferred from the masking
bit-tests at lines 525-555 (mode bits, choice modifiers).

## Hard panic — `sys_Panic`

`ings.c:360`. Prints `g_str_overflow` ("OVER FLOW") via `debug_printf` and
spins on `func_800164F8()` — which is hand-coded asm that issues `break` to
halt the CPU. Called when the file-loader gets a sound-data overflow, the
overlay overflow, or the OT overflow check at main-loop step 10.

## Where unmatched asm still hides

The main loop is fully decompiled in C — `ings.c:584-675`. The mode dispatch
table is in zero-initialized BSS so it's not in `asm/data/`. What IS still
asm:

- `func_8008386C` — InitHeap shim (1-instruction stub in `ings2.c:560`,
  matches by being canonical PsyQ BIOS-call form).
- `bios_FileReadRaw` — BIOS A(4Dh) syscall trampoline (`ings2.c:601`).
- `func_800164F8` — the panic-spin (`ings.c:117`).
- `func_800164AC` — the boot data dispatch table (`ings.c:91`, also asm
  because it's pure data, 19 function-pointer words).

## Cross-references (recent_naming_findings.md addendum 2026-05-17)

Three clusters from the placeholder-refinement pass interact with the main loop:

- [§11 Sequence-event handler table (MIDI-style dispatch)](recent_naming_findings.md#11-sequence-event-handler-table-midi-style-dispatch-5-slots)
  — `g_seq_event_handler_{90_NoteOn, B0_CtrlChange, C0_PgmChange,
  E0_PitchBend, FF_Meta}` at `0x800F3340..0x800F3350`, invoked from
  `saTan0Main` (main.c:334-454) — per-character MIDI-style command-stream
  event dispatcher.  See [sound.md](sound.md) for the sequencer context.
- [§12 Sound data buffer pointer cluster](recent_naming_findings.md#12-sound-data-buffer-pointer-cluster-6-pointers--relocator)
  — `g_snd_data_buf_base` + `g_snd_data_subblock_0_ptr..4_ptr` at
  `0x800EFB14`.  Not main-loop ticked — relocated when the sound buffer
  moves via `func_80054FDC(delta)` (text1b.c:11363).  Owned by the
  sound subsystem.
- [§22 IRQ-callback trampolines](recent_naming_findings.md#22-irq-callback-trampolines-d_80083edc--d_80083f1c)
  — `g_irq_handler_entry_no_pri` (0x80083EDC) fires the pending primary
  callback (if armed) + always-secondary; `g_irq_handler_entry_with_pri`
  (0x80083F1C) implements a one-shot deferred-fire using
  `g_alarm_pending_priority_flag` (0x800A26E0). Both are dispatch
  trampolines for the alarm cluster at `D_800A26D0..0x800A26E0`.

These are all canonical (BIOS syscall) or pure-data forms — not gameplay
logic.
