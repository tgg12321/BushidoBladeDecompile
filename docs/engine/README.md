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
                            |  func_80016D78 -> init      |
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
   |  Dispatch: D_8008D090[D_800A3834]()                        |
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
boundaries; several files are one original file split at a jump-table, rodata or
per-file gp (`-G8`) boundary. `.text` ranges are from the link map
(`build/bb2.map`; link order in `bb2.ld`). Descriptions summarise the names in
each file (`named_syms.txt` aliases included); many game-side names are inferred,
so treat them as hints. The subsystem docs below predate the file splits; their
file citations point at the current files (old `sound.c` is now in `text1b.c`,
`config.c` in `code6cac_c2.c`, `code6cac_b2.c` in `code6cac_b2_post.c`..`b5_post.c`),
but many function names they use were since reset to `func_<addr>` — the
address is the stable key.

| File | `.text` range | Contents |
| --- | --- | --- |
| `ings.c` | 0x800164F8-0x80017FA0 | Boot/init: `main`, `func_80016D78`, `disp_Init`, PC-drv file loaders, RNG, object-position math, scratchpad save/restore |
| `code6cac.c` | 0x80017FA0-0x8001979C | mixed / unclear (11 functions, mostly unnamed) |
| `code6cac_tu2.c` | 0x8001979C-0x80026DA4 | Game-mode handlers and match flow (`D_8008D090` slots 1/12/17: `func_8001E878`, `func_8001EA04`, `func_8001EFA0`), practice-menu code (func_80021424), CPU move-pattern helpers; mixed |
| `code6cac_b.c` | 0x80026DA4-0x800272FC | func_80026DA4 only (CPU/AI helper per its alias) |
| `code6cac_b_tu2.c` | 0x800272FC-0x800343F0 | Fighter combat and CPU/AI: distance/separation, dodge checks, `cpu_set_move_command_and_dir`, `cpu_check_same_dir_timer`, GTE transforms |
| `code6cac_b_tu3.c` | 0x800343F0-0x80034708 | 2 functions; mixed / unclear |
| `code6cac_b3.c` | 0x80034708-0x80034F88 | func_80034708 alone (`-G8` unit); mixed / unclear |
| `code6cac_b3_post.c` | 0x80034F88-0x80035438 | Mode handlers (`mode_handler_14`, mode helpers) |
| `code6cac_b2_post.c` | 0x80035438-0x80035F30 | Mode helpers, scene teardown, bit-mask pack helpers; mixed |
| `code6cac_b4.c` | 0x80035F30-0x80035FA8 | CD-audio mix setters (`cdrom_SetMix`) |
| `code6cac_b4_post.c` | 0x80035FA8-0x80036140 | CD init and callbacks (`cdrom_Init`, `cdrom_ReadyCallback`), `snd_SerialMixOn` |
| `code6cac_b5.c` | 0x80036140-0x80036D88 | The game CD module's two state-machine steppers |
| `code6cac_b5_post.c` | 0x80036D88-0x800375EC | Game-side CD wrappers (`cdrom_StartRead`, `cdrom_LoadExec`), `sys_Exec`, `func_80036F40` |
| `code6cac_c.c` | 0x800375EC-0x80037D14 | Memory-card wrappers (`memcard_Init` .. `memcard_WriteFile`) |
| `code6cac_c0.c` | 0x80037D14-0x80037F08 | func_80037D14 (memory card; calls libcard `_card_*`) |
| `code6cac_c_mid.c` | 0x80037F08-0x8003AB44 | Mixed: game-side link-cable `comb_*` wrappers, motion-shift mode setters, file-I/O state machine, `mode_handler_04` |
| `code6cac_c2.c` | 0x8003AB44-0x800401CC | Menu/mode handlers (VS-mode init, logo init), mode handlers 06-33 (post-battle, teardown, reboot), stage collision/lighting (`stage_*`), bitstream reader |
| `text1a_pre.c` | 0x800401CC-0x80040D48 | Player-model ("rob") init, draw-move list (`gpu_AddDrawMove`) |
| `text1a_pre_tu2.c` | 0x80040D48-0x800414FC | 4 functions; mixed / unclear |
| `text1a_svc.c` | 0x800414FC-0x8004153C | `save_vc_ctrl` alone |
| `text1a_post.c` | 0x8004153C-0x80042504 | Slot management (`func_800415C4`, `func_80041604`), effect dispatch |
| `text1a_c.c` | 0x80042504-0x80044800 | Matrix/colour math (`math_RotMatrix*`, `math_RgbToHsv`), primitive texture-offset helpers (`gpu_OffsetTexPoly*`), prim-buffer slots |
| `text1a_c_tu2.c` | 0x80044800-0x800460E4 | `func_800450BC` block load / copy family and channel helpers |
| `text1b.c` | 0x800460E4-0x8004A348 | Game glue (`func_800460E4`, `game_*Init`, `func_800467A8`), SE allocation/stop, camera bone setup |
| `text1b_tu1b.c` | 0x8004A348-0x80060A68 | 3D render core: GTE transform/clip kernels, stage/ground drawing, collision tests, `func_80054604` / `func_8005490C` (fill and play back the `D_800EFAE8` camera / motion block) |
| `text1b_tu1c.c` | 0x80060A68-0x8006E534 | Effects and ex-motion: the 16 `func_80067200` particle-bank wrappers (`func_80066EC0`..`func_800671CC`), `motion_ex_*`, animated sprite/object drawing |
| `text1b_tu1d.c` | 0x8006E534-0x8007352C | HUD/overlay rendering, replay display setup |
| `text1b_tu1e.c` | 0x8007352C-0x800747D8 | Sprite/animated-object drawing (5 functions) |
| `text1b_b.c` | 0x800747D8-0x80079244 | Game code to 0x80078948 (character-select `SelWork` drawing, display setup, replay camera), then PsyQ libapi/libetc/libc (`Exec`, events, pad, root counters, `memcpy`, `rand`, `printf`) |
| `text1b_b_tu2.c` | 0x80079244-0x80079A30 | PsyQ libc (`prnt`, `toupper`, `tolower`, `memchr`, `putchar`) |
| `text1b_b_tu3.c` | 0x80079A30-0x8007A28C | PsyQ libc `sprintf` |
| `gpu.c` | 0x8007A28C-0x8007B244 | PsyQ: `memmove`, libcard (`_card_*`, `InitCARD`), libgpu primitives (`LoadTPage`, `SetPoly*`, `ResetGraph`) |
| `display.c` | 0x8007B244-0x8008008C | PsyQ libgpu (`DrawSync`, `LoadImage`, `DrawOTag`, draw/disp env), libgte (`InitGeom`, matrix ops, `RotTransPers`), `CdInit` |
| `system.c` | 0x8008008C-0x8008289C | PsyQ libcd (`CdControl`, `CdRead`, `CD_sync`), `DeliverEvent` |
| `ings2.c` | 0x8008289C-0x80083BE4 | PsyQ libcd read callbacks, libetc/libapi (`VSync`, interrupts, `setjmp`, `InitHeap`), libsn (`PCopen`, `__SN_ENTRY_POINT` = EXE entry 0x800836EC, `__main`), libsnd start-up |
| `main.c` | 0x80083BE4-0x8008BE04 | PsyQ libsnd (`Ss*`) and libspu (`Spu*`) |
| `comb.c` | 0x8008BE04-0x8008D050 | PsyQ LIBCOMB link-cable SIO driver |
| `main_post.c` | 0x8008D050-0x8008D120 | PsyQ libapi `AddDrv`/`DelDrv` and the data words ending `.text` |

Rodata-only files (no `.text`): `ings_strings.c` (0x80010000-0x80010068, debug
format strings + build date), `code6cac_b_rodata_pre.c` (0x80010868, 4 bytes),
`code6cac_b_rodata_post.c` (0x800109B0-0x800109D8, memory-card path formats),
`text1a_filepaths.c`
(0x80010DEC-0x800152B4, asset file-path table), `text1a_b_pre_rodata.c`
(0x800153F0-0x8001585C) and `text1a_b_pre_rodata_b.c` (0x800158B4-0x800158E0)
(jump tables / strings of the text1b files), `text1a_b_post_rodata.c`
(0x80015D58-0x8001622C) and `text1a_b_tail_rodata.c` (0x80016240-0x800163C0)
(multi-file rodata cluster), `text1a_b_mid_rodata.c` (empty boundary unit).

Subsystem docs:

| Subsystem | Doc |
| --- | --- |
| Boot + main loop | [main_loop.md](main_loop.md) |
| Combat / hit detection | [combat.md](combat.md) |
| CPU / AI | [ai.md](ai.md) |
| Motion / animation | [motion.md](motion.md) |
| GPU pipeline / OT / DMA | [gpu_pipeline.md](gpu_pipeline.md) |
| Sound / SPU | [sound.md](sound.md) |
| File I/O / CD-ROM | [file_io.md](file_io.md) |
| Menus / UI / fades | [menus.md](menus.md) |
| Replay / special camera | [replay.md](replay.md) |
| Memory map; naming-pass data clusters | [memory_layout.md](memory_layout.md) |
| Cross-reference | [cross_reference.md](cross_reference.md) |
| PsyQ library usage | [psyq_usage.md](psyq_usage.md) |

## Key globals to know

These are the load-bearing globals; if you understand them, the engine starts
to make sense. Full vocabulary in `symbol_addrs.txt`.

### Game-state dispatch
- `D_800A3834` — **the** main game-mode register (0..0x20+). Each value is an
  index into `D_8008D090` (0x8008D090, an array of 34 handler function
  pointers). The main loop calls
  `((void (*)(void))(&D_8008D090)[D_800A3834])()` every frame
  (`ings.c:625`).
- `g_game_mode` (0x800A336C) — coarse game state (set up by `game_Init`).
- `g_game_pause` (0x800F6654) — pause flag.
- `g_game_timer` (0x800A3790) — global game timer (frames or ticks).

### Display / frame
- `D_800A3768` — a five-valued mode (0xFF, 1, 2, 10, 0x14) that selects what
  `func_800174F4` draws each frame (6CF8.c:540-590); `bb2_const.h` names
  three of the values.
- `D_800A36A8` — a 0/1 switch that multiplies the `func_8005D554` sprite passes.
- `D_800A36AC` — presented-frame counter (++ after each DrawOTag); most readers take its low bit as the
  double-buffer index, `func_8003C9A4` / `func_8003CD10` use the whole value.
- `g_disp_fb_base` (0x800F7438) — base of the two 0x4090-byte
  drawenv+dispenv+OT structures (one per buffer).

### Sound
- `g_snd_bgm_id` (0x800A33B0), `g_snd_se_id` (0x800A33B4),
  `g_snd_stage_bgm` (0x800A33C0), `g_snd_volume` (0x800A33D0)
- `SND_CHANNEL_BGM=8`, `SND_CHANNEL_SE=9`, `SND_CHANNEL_UI=0xA`
  (`bb2_const.h`)

### Stage / character
- `D_80099478` (id cached by `func_800460E4`), `D_8009947A` (0/1 flag),
  `D_800A8FB0` — 32x32 cell grid read by the draw walks (no collision use)
- `D_800A9A10` — three slots for the model objects `func_80045878` builds
  (`func_80041584` scans i < 3)
- `D_80094B88` — per-slot 5-bit codes `func_80041604` stores and
  `func_80040594` packs into the model object's flag word
- `D_800A6690` — transform-node records (0x68 bytes) unpacked from a u16
  stream and queued by the grid draw walks
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

Every function is complete (queue empty since 2026-10-02); the `INCLUDE_ASM` functions that remain
are canonical hand-written / PsyQ-library asm. The subsystem docs describe behaviour, not decomp state. Some docs still cite
bare `func_8XXXXXXX` names where `named_syms.txt` has since assigned a semantic name — trust
`named_syms.txt` when they disagree.
