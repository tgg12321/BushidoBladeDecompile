# Bushido Blade 2 Engine Documentation

This is design-level engine documentation for Bushido Blade 2 (SLUS-00663), a
1998 PlayStation 1 fighting game by Lightweight, published by SquareSoft. The
purpose of these docs is to describe **what the engine does** so future
modders, decompilers, and contributors can understand and modify the game.

The engine is internally referred to as **Marionation** (or "Marionation
Engine"), a proprietary engine written by Lightweight. The same engine —
modified and expanded — was later reused for Lightweight's PS2 game
*Kengo: Master of Bushido*. The Kengo source has leaked, and many BB2 function
names in this project were recovered by name-matching against Kengo.

Companion documents:

- `named_syms.txt` and `symbol_addrs.txt` (project root) — the authoritative
  name→address vocabulary used throughout these docs. When a name here has
  drifted, these files are ground truth.
- The per-subsystem tables below (and each linked subsystem doc) give the
  object-level address ranges; this README documents what each subsystem
  actually does.
- `CLAUDE.md` (project root) — toolchain, decomp workflow, splat config.

## High-level architecture

```
                            +-----------------------------+
                            |  EXE entry @ 0x800836EC     |
                            |  (BSS clear + boot)         |
                            +--------------+--------------+
                                           |
                                           v
                            +-----------------------------+
                            |  cpu_set_move_command_and_  |
                            |    dir_for_no_action_2      |
                            |  (ings.c — the C entry      |
                            |   point; misnamed by Kengo  |
                            |   collision)                |
                            |                             |
                            |  motion_Open  -> CTORs      |
                            |  sys_Init     -> hardware   |
                            |  sys_GameInit -> game data  |
                            |  main loop (label `loop:`)  |
                            +--------------+--------------+
                                           |
                  per frame                v
   +------------------------------------------------------------+
   |  D_800A3834 = current game mode/state                      |
   |    0  -> title FMV / mode select                           |
   |    1  -> game gameplay (saRobDraw / motion_SetMotion)      |
   |    3  -> mode reset                                        |
   |    7..0x20+ -> menus, options, character select, replay,   |
   |               training (cpu_practice_*), katinuki, etc.    |
   |                                                            |
   |  Dispatch: g_module_func_tbl[D_800A3834]()                 |
   |  Camera:   special_camera_Exec(), special_camera_get_*()   |
   |  GPU:      change_shadow_tex_reg, gnd_get_fog              |
   |  Render:   gnd_disp_loop_ctrl() (per-mode draw)            |
   |  Sync:     sys_VSync(0/1/2)                                |
   +------------------------------------------------------------+
                                           |
                                           v
                +-----------------------------------------------+
                |  Subsystems (per-frame service work)          |
                |  -- combat / hit detection                    |
                |  -- AI command stream evaluation              |
                |  -- motion playback / animation               |
                |  -- camera                                    |
                |  -- GPU packet/OT build + DMA submit          |
                |  -- SPU sound / BGM streaming                 |
                |  -- CD-ROM file loading                       |
                +-----------------------------------------------+
```

## Where each subsystem lives

The names `code6cac*`, `text1a*`, `text1b*`, and `ings*` are **splat/pipeline
artifacts** named after disassembly section labels — they are **not** semantic
boundaries. Treat them as containers for the address ranges given in the
per-subsystem table below.

| Subsystem | Primary files | Address range | Doc |
| --- | --- | --- | --- |
| Boot + main loop | `ings.c`, `main.c`, `ings2.c` | 0x800164AC-0x8008D120 | [main_loop.md](main_loop.md) |
| Combat / hit detection | `code6cac_b.c`, `text1b.c` (12463-13840), `main.c` | 0x80026DA4-0x80035438, etc. | [combat.md](combat.md) |
| CPU / AI | `code6cac_b.c` (cpu_*) | 0x80026DA4-0x80035438 | [ai.md](ai.md) |
| Motion / animation | `text1b.c` (11421-11673, 13840-14709), `code6cac_c_mid.c`, `ings2.c` | scattered | [motion.md](motion.md) |
| GPU pipeline / OT / DMA | `gpu.c`, `display.c`, `text1b.c` calc_loc_mat_* | 0x8007A28C-0x8008008C | [gpu_pipeline.md](gpu_pipeline.md) |
| Sound / SPU | `sound.c`, `main.c` (spu_*), `text1a_c.c` (seq_*) | 0x80046780-0x80047ED0 + scattered | [sound.md](sound.md) |
| File I/O / CD-ROM | `ings.c` (file_*), `system.c` (cdrom_*), `ings2.c` (bios_*) | 0x800800CC-0x8008289C | [file_io.md](file_io.md) |
| Menus / UI / fades | `code6cac_c2.c`, `code6cac_c_ab.c`, `config.c` | 0x8003B9D0-0x8004019C | [menus.md](menus.md) |
| Replay / special camera | `code6cac_b2.c`, `text1b.c` (15644-16266) | 0x80035438-0x800375EC, 0x8006EC0C-0x80073728 | [replay.md](replay.md) |
| Memory map | (data symbols project-wide) | n/a | [memory_layout.md](memory_layout.md) |
| Cross-reference | (this doc + above) | n/a | [cross_reference.md](cross_reference.md) |
| PsyQ library usage | (libgpu, libcd, libapi, libspu, MDEC) | n/a | [psyq_usage.md](psyq_usage.md) |
| Recent naming-pass findings | (BIOS events, scratchpad cache, match-round, replay-pos, text1b render-state, GTE/SPU/IRQ clusters) | n/a | [recent_naming_findings.md](recent_naming_findings.md) |

## Key globals to know

These are the load-bearing globals; if you understand them, the engine starts
to make sense. Full vocabulary in `symbol_addrs.txt`.

### Game-state dispatch
- `D_800A3834` — **the** main game-mode register (0..0x20+). Each value is an
  index into `g_module_func_tbl` (0x8008D090, an array of function pointers
  paired with cleanup pointers). The main loop calls
  `((void (*)(void))(&D_8008D090)[D_800A3834])()` every frame
  (`ings.c:625`).
- `g_game_mode` (0x800A336C) — coarse game state (set up by `game_Init`).
- `g_game_pause` (0x800F6654) — pause flag.
- `g_game_timer` (0x800A3790) — global game timer (frames or ticks).

### Display / frame
- `g_disp_enable` (0x800A3768) — 0xFF = display off / loading; 0x14 = active;
  10 = loading. See `bb2_const.h`.
- `g_disp_fade` (0x800A36A8) — 0..255 fade-to-black amount.
- `g_frame_parity` (D_800A36AC) — frame counter, low bit selects the active double-buffer slot.
- `g_disp_fb_base` (0x800F7438) — base of the two 0x4090-byte
  drawenv+dispenv+OT structures (one per buffer).

### Sound
- `g_snd_bgm_id` (0x800A33B0), `g_snd_se_id` (0x800A33B4),
  `g_snd_stage_bgm` (0x800A33C0), `g_snd_volume` (0x800A33D0)
- `SND_CHANNEL_BGM=8`, `SND_CHANNEL_SE=9`, `SND_CHANNEL_UI=0xA`
  (`bb2_const.h`)

### Stage / character
- `g_stage_id` (0x80099478), `g_stage_variant` (0x8009947A),
  `g_stage_collision` (0x800A8FB0) — 32x32 collision grid
- `g_player_ptrs` (0x800A9A10) — array of player-object pointers (3 slots:
  P1, P2, prop/AI)
- `g_player_char_ids` (0x80094B88) — character IDs per slot
- `g_char_data` (0x800A6690) — large character data region
- `g_cam_matrix` (0x800EEDB0) — camera rotation matrix base

### Pad / controller
- `g_pad_data` (0x800FF580) — raw pad buffer (libapi `InitPAD`)
- `D_80102794` — decoded current+previous pad state; the low 16 bits are P1
  current, high 16 bits are P2 current (the various `0x100010`, `0x10001000`
  masks in code are `[P2-bit | P1-bit]` pairs — Triangle is `0x10|0x100000`).

### Memory regions used by the engine
- 0x80010000-0x800A3800 — main EXE text + data (606 KB, from PS-EXE header)
- 0x801D8800 — sound/overlay scratch base (`D_800A3770`)
- 0x801EBC00 — second scratch base (`D_800A3774`)
- 0x801FFFF0 — top of stack (initial `$sp`)
- 0x1F800000-0x1F8003FF — PS1 scratchpad SRAM, used as fast per-frame
  workspace (see [memory_layout.md](memory_layout.md))

## Decomp status caveat

The project reached zero-stub completion on 2026-04-27: essentially every
function is decompiled to C (only a handful of canonical-asm bodies remain as
`INCLUDE_ASM`). The remaining work is not "asm-only" functions but
**byte-matching** them without cheats — ~422 functions still carry a
regfix/asmfix rule or cheat-asm and are tracked in `engine/queue.json` (the
single worklist). Some subsystem docs still cite bare `func_8XXXXXXX` names
where `named_syms.txt` has since assigned a semantic name — trust
`named_syms.txt` when they disagree.

`engine/queue.json` is the ordered worklist (pre-sorted easiest-first by honest
pure-C distance); `python3 -m engine.cli queue next` prints the current top.
The autonomous pipeline that grinds it is the **Grinder** (`tools/grinder/`,
the `decomp-grind` skill). See `CLAUDE.md` for the full workflow. (The old
`WORK_QUEUE.md` file and `dc.sh classify` tool are retired.)
