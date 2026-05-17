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
`D_800A3834` (the current "main game mode") as an index into a table of
function pointers. Each function is one frame of that mode.

Mode values observed in `src/`:

| Mode | Set by | Meaning (inferred) |
| --- | --- | --- |
| 0 | `cpu_get_move_pattern_table_number` (code6cac.c:1889), various | "menu/title" — null mode, advances to next |
| 1 | (initial in saRobDraw, code6cac_b2.c:296) | game-running, motion playback (`motion_SetMotion`) |
| 2 | code6cac_b.c:3756 | game-running variant |
| 3 | `cpu_set_move_command_and_dir_for_no_action` (code6cac_b.c:3510) | reset main loop |
| 4 | code6cac_b.c:3756 | gameplay |
| 7 | code6cac_c2.c:247 | sub-mode transition |
| 8 | many — global reset uses this | title / menu logo |
| 9 | code6cac_b2.c:146,250 | special-camera variant |
| 0xB | code6cac_c2.c:330 | menu |
| 0xD | code6cac.c:1812 (func_8001EA04) | post-cleanup mode |
| 0xF | initial value set by `cpu_set_move_command_and_dir_for_no_action_2` | **boot mode** — what the engine starts in |
| 0x11 | func_8001EEB4 (code6cac.c:1924) | "after-fight" cleanup mode |
| 0x12, 0x14, 0x1A, 0x1B, 0x1C, 0x20 | various combat / menu transitions | scenario-specific |

Most of the actual function pointer values are not yet labeled in
`asm/data/7D920.data.s` (the data file starts at `D_8008D120` with what looks
like a curve table, suggesting `D_8008D070..D_8008D120` is in BSS and zero-
initialized at runtime).

The implication: each mode's function pointer is INSTALLED at runtime — it is
not statically linked at load time. The "module init" array at `D_8008D070`
likely populates the dispatch table. This is consistent with how Marionation
loads sub-engines (e.g., the replay-camera module sets its own per-mode
function on init).

Looking ahead to a research item: nobody has yet enumerated which function
each `D_800A3834` value points to at runtime. This would require tracing each
"`D_8008D090[N] = func_XXX`" assignment in the bring-up path. A few examples
that ARE visible in `src/`:

- `D_8008D090[N]` is reassigned during `cpu_get_move_pattern_table_number`
  via `D_800A3834 = ...` followed by an immediate flow change.
- `D_800A3834 = 8` after a global reset jumps to "title/menu logo" — the
  Kengo function `md_menu_logo_exec` (renamed in `named_syms.txt`) is at
  `0x8003AFFC` and is one of these handlers.
- `md_game_end` (`main.c:997`) is one mode handler — currently empty in
  the matched code, indicating it was inlined or moved.
- `md_game_check_change_main_mode_default` at `0x80083A48` and
  `md_game_check_change_main_mode_katinuki` at `main.c:2240` are both
  mode-transition handlers.

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
  `D_800F33D8` (a global draw-state struct).

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

These are all canonical (BIOS syscall) or pure-data forms — not gameplay
logic.
