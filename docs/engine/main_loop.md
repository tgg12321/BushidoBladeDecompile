# Boot Sequence and Main Loop

This document traces what happens from power-on through one frame of gameplay.

## Boot

The PS1 BIOS hands control to BB2 at address `0x8008_36EC`. This is declared
in the PS-EXE header (`asm/header.s:6`) and corresponds to the entry point
`__SN_ENTRY_POINT` (`asm/funcs/__SN_ENTRY_POINT.s`, `INCLUDE_ASM` in `ings2.c`).
It was once mislabelled `ang_hosei` because it shared an asm file with the
adjacent lseek trampoline (now `PClseek`); it is NOT the rotation-correction
helper.

Annotated entry-point code (from `asm/funcs/__SN_ENTRY_POINT.s`):

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
    motion_Open();                 // run CTORs
    func_800789D8(0x801FFF00);     // libapi InitTimer / PadInit
    func_80078968(2);              // VSync wait config
    sys_Init();                    // libgpu/libcd/libspu init + disp_Init
    func_80016D78();               // start-up clears and loads (sound bank, pad state)
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
4. Dispatches to the current mode function via `D_8008D090[D_800A3834]`.
5. Waits for vsync window.
6. Submits the built OT to the GPU.
7. Optionally enters a pause/select submenu (`func_80016E60`).

The mode dispatch in step 4 is the heart of the engine — it's how the game
moves between title screen, character select, gameplay, replay, etc.

## Boot subsystems

### `sys_Init` (`ings.c:276`)
Disables interrupts, initializes the pad buffer (`func_80078C9C` — libapi
`InitPAD`), sets up the controllers (`func_80078D38` — `StartPAD`), the VSync
event (`func_80078A58`), the display (`disp_Init`), camera state
(`func_80035FE0`), pad-press state, then `sys_InitSound`.

### `func_80016D78` (`src/main/6CF8.c:287`)
- Prints `g_str_limit` with `0x8010DB00`
- `func_800167EC`, `func_80020D70`
- Sets `D_800A3770[0]=0x801D8800`, `D_800A3770[1]=0x801EBC00`, `D_800A3798=0x13400`
  — the overlay-region scratch buffers
- Clears `D_800A3716` and `D_800A3906`, then `snd_InitAndLoadCommonVab`,
  `pad_ResetStateMarkValid`, `func_8003D2C4`, `func_8001C444`
- Zeroes `D_800A36F9`, `D_800A3690`, `D_800A3744..46`, runs `func_80046B44`,
  sets `D_800A36F1 = 2`, and zeroes `D_800A38C4[1]`, `D_800A36B0`, `D_800A3928`.

### `motion_Open` (`ings2.c:501`)
Runs an array of "constructor" function pointers stored at `D_8008D070`
through the end-marker `D_00000000`. The loop is hand-coded asm because it
modifies `$s1` (the counter) as a side effect of each call. See also
`motion_Close` (`ings2.c:502`), which is the symmetric tear-down.

This is the engine's module init/cleanup mechanism — every subsystem that
needs early initialization adds itself to the `D_8008D070`-rooted array at
link time, and is run automatically at boot. The mechanism is NOT used for
the per-frame mode dispatch, which is a separate array starting at
`D_8008D090`.

## Mode dispatch — `D_8008D090`

The main loop's `((void (*)(void))(&D_8008D090)[D_800A3834])()` call uses
`D_800A3834` (the main loop's handler index)
as an index into a 34-entry table of function pointers. Each function is one
frame of that mode.

**The table is statically initialized**, not BSS as previously thought — it's
defined inline in `src/main/d_7D870.c:22` as a `.global D_8008D090` block of 34
`.word` entries. Earlier docs assumed BSS-init based on the entries being
zero in `asm/data/7D920.data.s`; the actual definition is in the inline asm
of `src/main/d_7D870.c`.

### Complete 34-mode dispatch table (decoded 2026-05-17)

| Mode | Address | Handler | Role |
|------|---------|---------|------|
| 0  (0x00) | 0x8001DCB0 | `mode_handler_00_GameInit` | **Game init**: obj_InitChars + gpu_InitDisplay/Disable + gnd_disp_loop_ctrl + gnd_open |
| 1  (0x01) | 0x8001E878 | `func_8001E878` | **Main per-frame fight**: camera, characters, collision, motion, stage tick |
| 2  (0x02) | 0x80033898 | `func_80033898` | Display reset + transitions to mode 3 |
| 3  (0x03) | 0x80034708 | `mode_handler_03_NoOp` | Empty (no-op placeholder) |
| 4  (0x04) | 0x800397D4 | `func_800397D4` | **Full game setup**: gpu_EnableDisplay + gnd_open + player count + DMA list |
| 5  (0x05) | 0x8003993C | `mode_handler_05_NoOp` | Empty |
| 6  (0x06) | 0x8003B9D0 | `func_8003B9D0` | func_8001DA2C + func_80061178 + conditional GPU |
| 7  (0x07) | 0x8003BCB4 | `mode_handler_07_SubModeTransition` | md_game_check_change_sub_mode + pad input check |
| 8  (0x08) | 0x80035480 | `func_80035480` | Cleanup + set-up, selects mode 9, display on; also the **global-reset target** (D_800A3928 trigger) |
| 9  (0x09) | 0x80035828 | `mode_handler_09_NoOp` | Empty |
| 10 (0x0A) | 0x8003BE10 | `func_8003BE10` | gpu_ResetGraphMode1 + func_80016888 + func_80020CDC + func_800415C4(0/1) + eff_ClearInitFlag + func_8005B72C + func_80078824 + snd_SerialMixOn + cdrom_StartAudio + func_80037260, sets D_800A3834 = 0xB, display on (2B344.c:618-635) |
| 11 (0x0B) | 0x8003BEA8 | `func_8003BEA8` | Checks pad input mask 0x40 (action button) |
| 12 (0x0C) | 0x8001EA04 | `func_8001EA04` | func_80041688(0/1) + func_80061178; end-of-round |
| 13 (0x0D) | 0x8001EA84 | `cpu_get_move_pattern_table_number` | CPU AI move-pattern lookup |
| 14 (0x0E) | 0x80035430 | `mode_handler_14_NoOp` | Empty |
| 15 (0x0F) | 0x8003BFC4 | `func_8003BFC4` | gpu_ResetGraphMode1 + func_800415C4(0/1) + ... + func_80046B44 (start-up init), then selects mode 8; main selects this slot first |
| 16 (0x10) | 0x8001EEB4 | `hirahira_w_frie2` | "Falling/particles 2" — likely petal/snow effect |
| 17 (0x11) | 0x8001EFA0 | `func_8001EFA0` | Increments D_800A37B8, calls func_800472B0 |
| 18 (0x12) | 0x8003C040 | `func_8003C040` | Branches on D_800A38A4 (4..9), indexes D_8009016C / D_8008EA70 |
| 19 (0x13) | 0x8003C2C0 | `cpu_side_move_dir_2` | CPU AI sidestep direction |
| 20 (0x14) | 0x8003C42C | `func_8003C42C` | Counts D_800A377C[] entries into 8-cell histogram |
| 21 (0x15) | 0x8003C560 | `func_8003C560` | Plays SFX 0xA4/0xA7 at counter==30 frames |
| 22 (0x16) | 0x8003B870 | `func_8003B870` | **VS mode init**: func_80041604(0/1) + obj_InitChars + disp_SetFramebufferMode(1) |
| 23 (0x17) | 0x8003B8E4 | `func_8003B8E4` | Returns until frame counter >= 3 |
| 24 (0x18) | 0x8003C958 | `func_8003C958` | func_80016888 + clears D_800A3817 / D_800A3929 / D_800A37B8, sets D_800A3834 = 0x19, display on |
| 25 (0x19) | 0x8003C9A4 | `func_8003C9A4` | func_8003F1E4(0), then writes D_800F6608 fields (2B344.c:993-1000) |
| 26 (0x1A) | 0x80035DC8 | `func_80035DC8` | gpu_ResetGraphMode1 + func_80016888 + func_80020CDC + func_800415C4(0/1) + eff_ClearInitFlag + func_8005B72C + func_80077820, sets D_800A3834 = 0x1B, display on (25C38.c:394-405) |
| 27 (0x1B) | 0x80035E38 | `saRobDraw` | Draws robot AI (saRob = "sa" team rob) |
| 28 (0x1C) | 0x8003CE18 | `func_8003CE18` | func_8001DA2C + func_800372C0 |
| 29 (0x1D) | 0x8003CF84 | `func_8003CF84` | mk_leaf_newpos + reads char struct fields |
| 30 (0x1E) | 0x8003C714 | `SetCurrentCursor` | Menu cursor positioning |
| 31 (0x1F) | 0x8003C8B4 | `func_8003C8B4` | D_800A37B8++ and func_80060768 primitives; func_80033FE4 on pad 0x400040 or D_800A37B8 >= 0xF1 (241); no loop |
| 32 (0x20) | 0x8003CCCC | `func_8003CCCC` | gpu_InitDisplay + func_80061178 + dispatch to 0x21 |
| 33 (0x21) | 0x8003CD10 | `func_8003CD10` | Mirrors mode_25 setup; final teardown |

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

**func_80033DF4 / func_80033FE4 flow:**
```
func_80033DF4:
  if (D_800A38E2 == 0x64):
      D_800A36F0 / D_800A3781 = chosen D_80106A50.unk_00 bit was clear
      then sets that bit
  else: table row [D_800A38E2], D_800A38E2 += 1

func_80033FE4:
  D_800A36F0  --> D_800A38A4 = 6 or 7, D_800A3834 = 0x12 (func_8003C040)
  D_800A3781  --> D_800A38A4 = 8 or 9, D_800A3834 = 0x12
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
   global reset, the main loop forces `D_800A3834 = 8` and
   thus jumps to `func_80035480`. Many other paths also
   end at mode 8 to reach the title.

3. **Mode 18 (`func_8003C040`) dispatches on `D_800A38A4`.** `func_80033FE4`
   selects it (`D_800A3834 = 0x12`) after setting `D_800A38A4` to 6..9.

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
5. **Mode dispatch** — call `D_8008D090[D_800A3834]()`. This is where
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

- Returns immediately when `D_800A3768 == DISP_DISABLED` (0xFF); `D_800A3768`
  is the five-valued mode this function switches on.
- Uses its own internal s0/s1 register pinning to set up two GPU packet
  buffers.
- Walks the ground/scene draw pipeline using `s2` as a pointer to
  `D_800F33D8` (a 512-byte scratch region that is also the memcard save/load
  image).

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

## Cross-references (naming pass 2026-05-17; full traces at `pre-slim-2026-10-01:docs/engine/recent_naming_findings.md`)

Three clusters from the placeholder-refinement pass interact with the main loop:

- §11 Sequence-event handler table (MIDI-style dispatch)
  — `g_seq_event_handler_{90_NoteOn, B0_CtrlChange, C0_PgmChange,
  E0_PitchBend, FF_Meta}` at `0x800F3340..0x800F3350`, invoked from
  `saTan0Main` (main.c:334-454) — per-character MIDI-style command-stream
  event dispatcher.  See [sound.md](sound.md) for the sequencer context.
- §12 Sound data buffer pointer cluster
  — `g_snd_data_buf_base` + `g_snd_data_subblock_0_ptr..4_ptr` at
  `0x800EFB14`.  Not main-loop ticked — relocated when the sound buffer
  moves via `func_80054FDC(delta)` (text1b_tu1b.c:1017).  Owned by the
  sound subsystem.
- §22 IRQ-callback trampolines
  — `g_irq_handler_entry_no_pri` (0x80083EDC) fires the pending primary
  callback (if armed) + always-secondary; `g_irq_handler_entry_with_pri`
  (0x80083F1C) implements a one-shot deferred-fire using
  `g_alarm_pending_priority_flag` (0x800A26E0). Both are dispatch
  trampolines for the alarm cluster at `D_800A26D0..0x800A26E0`.

These are all canonical (BIOS syscall) or pure-data forms — not gameplay
logic.
