# Finished C Subsystem Map - 2026-05-12

Worktree: `C:/tmp/bb2-codex-audit-existing-c`

This is a navigation map for the finished C surface. It is intentionally not a
move/refactor plan. Address ranges come from `build/bb2.map` after a clean
matching build.

## Object-Level Map

| Object | Text range | Current shape |
| --- | --- | --- |
| `src/ings.c` | `0x800164AC-0x80017FA0` | Boot/file load, GPU display enable/disable, system init, RNG, object scratchpad/position primitives. |
| `src/code6cac.c` | `0x80017FA0-0x80026DA4` | Early game/combat/camera logic, practice/menu helpers, ground init, player/game status helpers, camera zoom and direction helpers. |
| `src/code6cac_b.c` | `0x80026DA4-0x80035438` | CPU/combat behavior, collision, weapon/body hit helpers, move-command queues, shadow/draw setup, pad/control, waza data. |
| `src/code6cac_b2.c` | `0x80035438-0x800375EC` | Special/replay camera, frame init/loop, camera data stream reads from `SpecialCam`, camera boot plumbing. |
| `src/code6cac_c.c` | `0x800375EC-0x80037D14` | Small pad/damage/special-camera helpers. |
| `src/code6cac_c0.c` | `0x80037D14-0x80037F08` | Tiny isolated helper block. |
| `src/code6cac_c_mid.c` | `0x80037F08-0x8003AB44` | Motion/precalc/control flow, analog pad handling, hit-stop checks, sound/effect init helpers, CD/init retry helpers. |
| `src/code6cac_c_ab.c` | `0x8003AB44-0x8003B9D0` | Mode/menu/display helpers, mental bar, cursor/state transitions. |
| `src/code6cac_c2.c` | `0x8003B9D0-0x8003F168` | Game mode transitions, menu text/drawing, stage/ground open, line drawing, camera matrix setup. |
| `src/config.c` | `0x8003F168-0x800401CC` | Configuration/options and fade/control helpers. |
| `src/text1a.c` | `0x800401CC-0x80042504` | Effect and gauge-adjacent helpers; several Kengo-derived small display/effect names live here. |
| `src/text1a_c.c` | `0x80042504-0x800460E4` | Effect/channel table management, sound/effect channel helpers, `saTanMainDispGnd_80046020`. |
| `src/text1a_c2.c` | `0x800460E4-0x800466C0` | Small continuation of the effect/channel helper area. |
| `src/text1a_b.c` | `0x800466C0-0x80046780` | Very small wrapper block. |
| `src/sound.c` | `0x80046780-0x80047ED0` | BGM/SE loading and callbacks, game/stage state, camera/bone initialization, fade calculations. |
| `src/text1b.c` | `0x80047ED0-0x8007A28C` | Large mixed block: effect rendering, object/task init, ground/hit/effect helpers, motion/gauge/replay-camera clusters, pad/critical-section/lib wrappers. |
| `src/gpu.c` | `0x8007A28C-0x8007B244` | GPU helper wrappers and low-level draw/list utilities. |
| `src/display.c` | `0x8007B244-0x8008008C` | GPU packet/env helpers, order-table/DMA helpers, math trig, GTE/matrix/vector routines, rotation helpers, callback stubs. |
| `src/system.c` | `0x8008008C-0x8008289C` | CD-ROM, callback, BIOS/std-library, memory, low-level system routines. |
| `src/ings2.c` | `0x8008289C-0x80083BE4` | VSync/timer/IRQ wrappers, matrix conversion, motion open/close, BIOS file read, SPU volume. |
| `src/main.c` | `0x80083BE4-0x8008D120` | High-level main/game loop, pad/memcard/task orchestration and late game plumbing. |

## `src/text1b.c` Landmarks

`text1b.c` is the file most likely to confuse future cleanup because it holds
many unrelated late-stage clusters. Useful landmarks:

| Text range | Line neighborhood | Notes |
| --- | --- | --- |
| `0x80047ED0-0x80049E4C` | lines `15-1179` | RMD/Hira/effect drawing, weapon afterimage/zanzou, ground-close and particle/effect helpers. |
| `0x80049F4C-0x80052720` | lines `1207-10558` | Large inline-asm/GTE and local-matrix interpreter block, including `calc_loc_mat_fw_8004A940` and many generated draw/math helpers. Treat as fragile. |
| `0x80052754-0x800545F4` | lines `10558-11227` | Projection/screen helpers, life gauge no-display checks, fade panel, camera inside-screen checks. |
| `0x800545F4-0x80055B60` | lines `11227-11421` | `myRobGeneiOpen`, status/select/katanuki status helpers, several bridge stubs and status-data setup. |
| `0x80055B60-0x8005B43C` | lines `11421-11673` | Second matrix/motion helper area, mostly asmfix or fragile hand-shaped C. |
| `0x8005B43C-0x8005C8A8` | lines `11673-12463` | Object/task init, allocation/file wrappers, `tslGlobalMemFree_8005C2A8`, `saFidLoad`. |
| `0x8005D46C-0x800620B8` | lines `12463-13840` | Ground/landing/weapon/effect hit helpers, fog, cleanup, action/effect command helpers. |
| `0x8006288C-0x80068F70` | lines `13840-14709` | Motion and ex-motion state tables, gauge init wrappers, small bridge-stub clusters. |
| `0x80069120-0x8006EACC` | lines `14710-15646` | Effect object management, `saTan` gauge/info/motion set, close/free helpers, sound/gauge bridge cluster. |
| `0x8006EC0C-0x80073728` | lines `15644-16266` | Replay camera attack, motion shift control, primitive allocation helpers, gauge and transform bridge cluster. |
| `0x80073C78-0x80077D94` | lines `16266-16985` | Display/gauge/particle subclusters, mirrored `saTan*` init/gauge functions, data queue helpers. |
| `0x800784E4-0x80079A30` | lines `16985-18001` | Camera rotation math, pad/critical-section wrappers, memcpy/debug printf and late utility wrappers. |

## Practical Rules For Future Cleanup

1. Do not move functions between translation units until the rodata/regfix
   dependencies are understood. Several files have per-file pipeline knobs in
   the Makefile.
2. Treat address-suffixed Kengo names as provisional. They are useful search
   handles, but the suffix means the semantic evidence may be weak.
3. For `text1b.c`, prefer local section comments or external docs before
   source movement. The file mixes pure C, asmfix bridge placeholders, inline
   asm scaffolds, and old-GCC-sensitive declarations.
4. For rename-only patches, update all symbol sources together: C declarations,
   `asm/funcs` labels when relevant, `named_syms.txt`, and any generated queue
   notes. Then run `make clean-check`.
