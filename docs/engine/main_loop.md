# Boot sequence and main loop

## Entry point

The PS-X EXE header's entry is `0x800836EC`, `__SN_ENTRY_POINT`
(asm/funcs/__SN_ENTRY_POINT.s; `INCLUDE_ASM` in
src/main/psxsdk/libsn/73E98.c:15, PsyQ 4.0 LIBSN module SNMAIN). It is SN's
start-up code, in order:

1. zeroes the words from `D_800A3308` up to `D_801078E0` (the BSS);
2. sets `$sp` to `_ramsize - 8` ORed with `0x80000000` (the high half of
   `D_80000004`);
3. computes the heap: base `D_801078E0` (top bits stripped, then ORed with
   0x80000000) into `__heapbase`, size `_ramsize - 8 - _stacksize - base` into
   `__heapsize`;
4. saves `$ra` in `D_800A3668`, sets `$gp` to `_gp` and `$fp = $sp`;
5. calls `InitHeap(__heapbase + 4, __heapsize)`, then `main`; a `break`
   follows the call.

## `main`

`main` (src/main/6CF8.c:423):

1. Start-up: `__main()` (the SN constructor walker; the game has no
   constructors, src/main/psxsdk/libsn/73E98.c:16-22), `SetSp(0x801FFF00)`,
   `SetMem(2)`, `sys_Init()`, `func_80016D78()`, `SetDispMask(1)`, then
   `func_80016A8C(0x80118800, ...)` (src/main/6CF8.c:188), which reads file
   `cdrom_GetFileIndex(2, 0x61)`, shows it for 150 frames and darkens it over
   the last 29.
2. `D_800A3834 = 0xF` (the first mode, `func_8003BFC4`), `D_800A390D = 0`,
   `D_800A36AC = 0`.
3. The loop (src/main/6CF8.c:447):
   - picks the buffer `g_gpu_db[D_800A36AC & 1]`, clears its OT
     (`ClearOTagR(ot, 0x1008)`), sets `g_gpu_ot_ptr` and the primitive cursor
     `g_prim_buf_cursor = D_800A3770[idx]`;
   - `func_80060E04(idx)` (src/main/51268.c), `font_BeginFrame()`
     (src/main/2B344.c:1131), `pad_Update(skip)` (last frame's `D_800A390D`),
     `cdrom_Update()`,
     `func_8005C6D0()` (src/main/3AB48.c);
   - if the link-abort request `g_comb_abort_request` is set (its one setter is
     `comb_Abort`, src/main/28708.c:1605): `cdrom_PauseAndWait()`, display off
     (`D_800A3768 = DISP_DISABLED`), clears the request and `D_800A31DA`, and
     sets mode 8;
   - calls the mode handler `D_8008D090[D_800A3834]()` (src/main/6CF8.c:468),
     then `font_AddTpage()` (src/main/2B344.c:1146);
   - waits until root counter 2 (`GetRCnt(0xF2000001)`) reaches
     `((D_800A36F1 - 1) << 8) + 0x80`, calling `rand()` while it waits; then
     `VSync(1)`, `DrawSync(0)`, `VSync(0)`, `ResetRCnt(0xF2000001)`;
   - unless frames are being skipped (`D_800A390D != 0`), `PutDispEnv` /
     `PutDrawEnv` for the buffer;
   - checks the primitive buffer: the space left
     (`D_800A3770[idx] + 0x13400 - g_prim_buf_cursor`) updates the low-water
     mark `D_800A30DC`; below 0 it prints `g_str_prim_overflow` and stops in
     `func_800164F8` (src/main/6CF8.c:488-499);
   - decrements `D_800A390D`, or else submits the OT (`DrawOTag`) and
     increments `D_800A36AC`;
   - in mode 1, when no frame was skipped, Start (`0x08000800`) or the
     `D_800A3713` countdown (with `D_800A38DC == 2`) opens the pause menu
     `func_80016E60` (src/main/6CF8.c:508-522).

## Start-up routines

- `sys_Init` (src/main/6CF8.c:175): `ResetCallback`, `InitPAD` on
  `g_pad_buf`, `StartPAD`, `ChangeClearPAD(0)`, `disp_Init`, display off,
  `D_800A36A8 = 0`, `cdrom_Init`, `memcard_Init`, `rcnt_StartCnt1Wrapper`.
- `func_80016D78` (src/main/6CF8.c:285): sets the two primitive buffers
  `D_800A3770[0..1] = 0x801D8800 / 0x801EBC00` and their size `D_800A3798 =
  0x13400`, loads the common sound bank (`snd_InitAndLoadCommonVab`,
  src/main/6CF8.c:265), `pad_ResetStateMarkValid`, `font_LoadImage`,
  `func_8001C444`, `func_80046B44`, sets `D_800A36F1 = 2` and clears the
  start-up flags and `g_comb_abort_request`.
- `sys_Panic` (src/main/6CF8.c:239) prints `g_str_overflow` and stops in
  `func_800164F8`; `eff_Init` and `snd_InitAndLoadCommonVab` call it when
  their data is too large.

## The pause menu: `func_80016E60`

`func_80016E60` (src/main/6CF8.c:312) runs its own frame loop (OT, `pad_Update`,
`func_8005C8A8` to draw, `cdrom_Update`, `func_8005C6D0`, present) with 3
choices, or 6 when `D_800A38DC == 2` and `D_800A389A` is 0. Up / Down move
the choice, Left / Right toggle bits of `D_800A3788` on choices 3..5, Cross
confirms and Triangle cancels. Choice 1 sets `D_800A31DA = 1` and mode 8;
choice 2 sets mode 8.

## The mode table `D_8008D090`

`D_8008D090` is a 34-entry table of handlers, defined in the inline
assembly of src/main/d_7D870.c:24. "Sets" lists the modes each handler
assigns to `D_800A3834` itself.

| Mode | Handler | Sets | What it does |
| --- | --- | --- | --- |
| 0 | `func_8001DCB0` (src/main/9F9C.c:1757) | 1 | sets up the fight (fighters through `func_80022580`, the object pool, the view) |
| 1 | `func_8001E878` (src/main/9F9C.c:2028) | | per-frame fight: `func_8001BE20` / `func_80023F08` per fighter, `func_8002C61C`, `func_80030D7C` ([combat.md](combat.md), [motion.md](motion.md)) |
| 2 | `func_80033898` (src/main/17AFC.c:5273) | 3 | resets graphics, clears `D_800A37B8` |
| 3 | `func_80034708` (src/main/24F08.c:28) | | prints the `D_80102778` settings with `font_Printf` and steps the two cursors `D_800A3174`; calls `func_800344B4` |
| 4 | `func_800397D4` (src/main/28708.c:1287) | 5 | view and fighter setup |
| 5 | `func_8003993C` (src/main/28708.c:1338) | 0xC, 0x10, 0x1E | rebuilds the fighters from the per-frame records `D_800A36EC` ([motion.md](motion.md)) |
| 6 | `func_8003B9D0` (src/main/2B344.c:478) | 7 | loads a script scene ([menus.md](menus.md)) |
| 7 | `func_8003BCB4` (src/main/2B344.c:550) | 0 | waits, then the next script step |
| 8 | `func_80035480` (src/main/25C38.c:34) | 9 | resets the scene, requests the link close (`comb_RequestClose`, unless `D_800A31DA` is set), sets up the row menu (`func_80077820`) |
| 9 | `func_80035828` (src/main/25C38.c:195) | 0, 0xF, 8 | the row-menu flow (`func_80077894`) with CD audio and memory-card calls |
| 10 | `func_8003BE10` (src/main/2B344.c:595) | 0xB | resets the scene, starts CD audio track `(5, 0x20)` |
| 11 | `func_8003BEA8` (src/main/2B344.c:614) | 0x1A, 8 | waits for Cross, merges unlock bits into `D_80106A50` |
| 12 | `func_8001EA04` (src/main/9F9C.c:2060) | 0xD | |
| 13 | `func_8001EA84` (src/main/9F9C.c:2075) | 0, 8 | per-frame fight variant with the view `func_8001BC70` and the cursor `D_800A3817` |
| 14 | `func_80035430` (src/main/25788.c:202) | | empty |
| 15 | `func_8003BFC4` (src/main/2B344.c:651) | 8 | runs the FMV overlay (`sys_ExecMovieOverlay`) |
| 16 | `func_8001EEB4` (src/main/9F9C.c:2162) | 0x11 | sets both fighters' moves (`func_80021A98`) |
| 17 | `func_8001EFA0` (src/main/9F9C.c:2184) | 0xC, 2 | per-frame fight variant with the view `func_8001BCF0`; after frame 0x69 or on Cross it sets 0xC (`D_800A38DC` 4 / 6, or 1 with `D_800A3748 != 0`), calls `func_80033BC0` (`D_800A38DC` 1, `D_800A3748 == 0`), or sets 2 |
| 18 | `func_8003C040` (src/main/2B344.c:666) | 0x13 | scene setup chosen by `D_800A38A4` |
| 19 | `func_8003C2C0` (src/main/2B344.c:725) | 0x12, 0x18, 0x1A, 0xA, 8 | waits on `func_80054F68`, then picks the next mode |
| 20 | `func_8003C42C` (src/main/2B344.c:754) | 0x15 | |
| 21 | `func_8003C560` (src/main/2B344.c:786) | 0x12, 0x18 | timed sounds, ends on Cross or frame 0xF1 |
| 22 | `func_8003B870` (src/main/2B344.c:421) | 0x17 | |
| 23 | `func_8003B8E4` (src/main/2B344.c:432) | | on its third frame runs the next script step (`func_8003B5A4`) |
| 24 | `func_8003C958` (src/main/2B344.c:877) | 0x19 | |
| 25 | `func_8003C9A4` (src/main/2B344.c:886) | 0, 8 | view (`func_80046BF4`) and its own per-frame work |
| 26 | `func_80035DC8` (src/main/25C38.c:394) | 0x1B | resets the scene, sets up the row menu (`func_80077820`) |
| 27 | `func_80035E38` (src/main/25C38.c:407) | 8 | screen gradient and the memory-card flow `func_80038C70` |
| 28 | `func_8003CE18` (src/main/2B344.c:990) | 0x1D | view and scene setup |
| 29 | `func_8003CF84` (src/main/2B344.c:1039) | 0x18 | frame-threshold cues on `D_800A37B8` |
| 30 | `func_8003C714` (src/main/2B344.c:822) | 0x1F | stores the three clock records |
| 31 | `func_8003C8B4` (src/main/2B344.c:868) | | on Cross or frame 0xF1 calls `func_80033FE4` (src/main/17AFC.c:5542), which sets 0x12, 0x1A or 8 |
| 32 | `func_8003CCCC` (src/main/2B344.c:959) | 0x21 | |
| 33 | `func_8003CD10` (src/main/2B344.c:967) | 8 | |

Modes the main loop itself sets: 0xF at start, 8 on a link abort. Many
handlers draw through `func_800174F4` (src/main/6CF8.c:526), which returns at
once while `D_800A3768` is `DISP_DISABLED` and otherwise switches on it.
