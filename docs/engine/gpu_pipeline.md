# GPU and render pipeline

Sony's GPU code is PsyQ 4.0 LIBGPU in `src/main/psxsdk/libgpu/` (`sys.c`,
`prim.c`, `ext.c`). The GTE library is in `src/main/psxsdk/libgte/`. The
game's frame set-up is in `src/main/6CF8.c`. The game's primitive builders are
spread over the game TUs (3AB48.c, 368E4.c, 4D0A8.c, 51268.c, 596CC.c, 63D2C.c, and others).
The frame loop itself is described in [main_loop.md](main_loop.md).

## Double buffer and ordering table

`GpuDb g_gpu_db[2]` (include/bb2.h:11, type at include/game.h:54-63) holds
two 0x4090-byte records, one per frame parity `g_gpu_presented_frame_count & 1`:

```
+0x0000  DRAWENV draw
+0x005C  DISPENV disp
+0x0070  u32 ot[0x1008]   (0x4020 bytes; ends the record)
```

Each frame `main` (src/main/6CF8.c:423) runs these steps:

1. Clears the record's OT with `ClearOTagR(ot, 0x1008)` (src/main/6CF8.c:451)
   and makes it `g_gpu_ot_ptr`.
2. Points `g_prim_buf_cursor` at the parity's primitive buffer
   `D_800A3770[idx]`. `func_80016D78` sets the two buffers to 0x801D8800 and
   0x801EBC00 (src/main/6CF8.c:289-290).
3. After the mode handler, waits for `DrawSync(0)` and the vsync, then sends
   `PutDispEnv` and `PutDrawEnv`.
4. Checks the buffer. Fewer than 0 of its 0x13400 bytes left prints
   `"common prim over flow"` and stops (src/main/6CF8.c:488-499).
5. Draws with `DrawOTag(&ot[0x1007])` (src/main/6CF8.c:504).

`ClearOTagR` links each entry to the one below it, so the GPU walks from
entry 0x1007 down to entry 0. A primitive linked at a higher index is drawn
earlier. `g_main_present_skip_frames` skips presenting for a number of frames.

The two `SetDefDrawEnv` calls in `disp_Init` put buffer 0's draw area at
y = 0 and buffer 1's at y = 0xF0. Each buffer displays the other's area. Both
are 640 x 240.

Two small extra lists use a local 1- or 2-entry OT:

- `gpu_DrawDisplayModeOnLocalOtAndSync` (src/main/6CF8.c:526) draws a full-screen overlay into the
  buffer at `g_shared_buffer`, chosen by `g_local_ot_draw_mode`. It uses `ClearOTagR(ot, 2)`
  and `DrawOTag(ot + 1)`.
- `menu_RunPauseMenu` (src/main/6CF8.c:312), the pause menu loop, uses
  `ClearOTagR(ot, 1)`.

## Display set-up

`disp_Init` (src/main/6CF8.c:157) runs:

- `ResetGraph(0)`, `SetGraphDebug(0)` and `SetDispMask(0)`;
- `InitGeom()`, `SetGeomOffset(0x140, 0x78)` and
  `SetGeomScreen(math_FovToScreenDist(0x2D))`. `math_FovToScreenDist`
  (src/main/6CF8.c:98) is `cos(fov/2) * 320 / sin(fov/2)` from the sine
  table `Judge`;
- `SetDefDrawEnv` and `SetDefDispEnv` for both records;
- `gpu_SetDrawEnvBg(1, 0, 0, 0)`, which sets `isbg` and a black background
  in both DRAWENVs (src/main/6CF8.c:106);
- `ClearImage(&g_gpu_clear_rect, 0, 0, 0)` and `DrawSync(0)`.

`sys_Init` (src/main/6CF8.c:175) calls `disp_Init`. `main` turns the display
on with `SetDispMask(1)` (src/main/6CF8.c:439).

## libgpu SYS internals

`sys.c` (`$Id: sys.c,v 1.129`) reaches the hardware through a device table
`GpuDevTable` (src/main/psxsdk/libgpu/sys.c:14-31). The table is at
`_gpucb`, and the pointer `D_8009BE6C` points to it. Its members:

| Member | Body | Used by |
| --- | --- | --- |
| `addque2` | `_addque2` (:924) | every queued call |
| `clr` | `_clr` (:738) | `ClearImage` / `ClearImage2` (:298, :305) |
| `ctl` / `getctl` | `_ctl` / `_getctl` (:882, :887) | GP1 writes, mirrored in `ctlbuf` |
| `cwb` | `_cwb` (:889) | `DrawPrim` (:363): GP0 word writes |
| `cwc` | `_cwc` (:898) | `DrawOTag`, `PutDrawEnv`, `DrawOTagEnv`, `MoveImage` |
| `dws` / `drs` | `_dws` / `_drs` (:784, :832) | `LoadImage` / `StoreImage` (:312, :317) |
| `otc` | `_otc` (:717) | `ClearOTagR` (:354) |
| `reset` / `sync` | `_reset` / `_sync` (:1003, :1031) | `ResetGraph` / `DrawSync` |

`g_gpu_ctx` (`GpuCtx`, :40-53) is the module state. It holds the GPU type,
the queue mode, the debug level, the VRAM width and height, the
DrawSync callback, and the last DRAWENV and DISPENV.

`ResetGraph(0)` (:164) works as follows:

- prints `jtb=` (the `_gpucb` address) and `env=` (`&g_gpu_ctx`);
- clears `g_gpu_ctx`;
- sets the type from `_reset`, which calls `_version` (:1090);
- sets `width = D_8009BEF4[type]` and `height = D_8009BF08[type]`.

**Queue.** `_que[64]` (:63-70) is a ring of 0x60-byte `GpuQueueItem`s. Each
holds a function, an argument pointer, a second argument and 21 data words,
indexed by `_qin` / `_qout`. `_addque2(func, arg, len, cb_arg)` runs `func`
at once when the queue is empty, DMA 2 is idle and no DrawSync callback is
set. Otherwise it copies `len` bytes of `arg` into the slot and queues it,
with `_exeque` as the DMA-2 callback. `_exeque` (:967) runs queued calls
while DMA 2 is idle. It calls the DrawSync callback once the queue drains.
The 16-byte `_qlog` record (words 0-2) logs the last call. The GPU
`get_alarm` (:1069) prints them on a timeout and resets the queue. The
timeout is 0xF0 vsyncs, set by `set_alarm` (:1064).

**DMA.**

- `_cwc` (:898) sends a packet chain on DMA channel 2 in linked-list mode.
  It writes GP1 `0x04000002` (`GP1_DMA_DIR_FIFO`, include/psx.h:26), then
  `DMA2_MADR = chain`, `DMA2_BCR = 0` and `DMA2_CHCR = DMA_GPU_LINKED_LIST`.
- `_dws` / `_drs` move image data. They send the copy command and the leading
  `n % 16` words through GP0, then the remaining 16-word blocks on DMA 2 in
  block mode (`DMA2_CHCR = 0x01000201` / `0x01000200`, GP1 direction 2 / 3).
- `_otc` (src/main/psxsdk/libgpu/sys.c:717) builds the reverse OT on DMA
  channel 6. It writes `DMA6_MADR = ot + n*4 - 4`, `DMA6_BCR = n` and
  `DMA6_CHCR = 0x11000002`, then polls bit 24.

The GP1 DMA direction (GP1 04h) and the DMA channel's mode bits in CHCR are
separate settings.

## GTE

The game uses the GTE through Sony's macros: `include/psxsdk/inline_o.h`,
`inline_c.h` and `gtemac.h`, with DMPSX command words substituted. A
`MATRIX` (include/psxsdk/libgte.h:29-33) is `s16 m[3][3]`, `u16 pad` at
+0x12, and `s32 t[3]` at +0x14. `SetRotMatrix` loads `m` into GTE control
registers 0-4. `SetTransMatrix` loads `t` into 5-7. Library entry points are
the LIBGTE modules in `src/main/psxsdk/libgte/`.

**Integer square root.** Functions in 87A0.c, 9F9C.c and 17AFC.c use
`gte_Lzc` (include/psxsdk/gtemac.h:174), the LZCS / LZCR leading-zero count,
to normalise a value above 0x400 before reading the square-root table
`g_sqrt_table_u8`. Values below 0x400 index the table directly. For an
example, see `softbody_SolveSprings` (src/main/87A0.c:174, the `gte_Lzc` at :284).

**`g_math_light_plane_projection`** (`MATRIX`, declared at src/main/368E4.c:296) is built by
`math_SetLightPlaneProjection` (src/main/368E4.c:555) from row 0 `(l0, l1, l2)` of
`g_light_setups[0].lmat`. The matrix has `m[0][0] = m[2][2] = 0x1000`, `m[0][1] = -l0/l1`,
`m[2][1] = -l2/l1` (4.12), and every other entry 0, including `m[1][1]`. `math_ApplyLightPlaneProjection(in, out, y)` (src/main/368E4.c:573)
multiplies `in` by it and moves the translation to height `y` along the same
direction. The result maps a point onto the plane at height `y` along the
direction `(l0, l1, l2)`. `math_LightPlaneProject` (:580) does the same from row 0
of `g_light_setups[arg0].lmat`, without storing the matrix.

## 2D sprites

The sprite sheets are 12-byte headers `Unk8009B0E0Record`
(include/game.h:231) followed by 8-byte cells `Unk8009B400Record`
(include/game.h:202-209). The draw walkers are in src/main/63D2C.c:

- `gpu_AddSpriteCells(arg0)` (src/main/63D2C.c:11) emits one `SPRT` per cell
  inside 640 x 240. It applies the header's CLUT and texel base and the
  optional colour and semi-transparency. It links the `SPRT` at
  `g_gpu_ot_ptr + arg0->ot_idx`. An index of 0x1006 or more is reset to 1
  with a printed warning.
- `gpu_AddScaledSpriteQuads(arg0, mode)` (:55) is the `POLY_FT4` form.

`gpu_DrawSpriteAtPackedTablePosition` (4D0A8.c) takes the screen position from the packed table
`D_8009B850`, picks one of the `D_8009B7AC` sheets, and draws it with
`gpu_AddSpriteCells` plus a `DR_MODE`. `gpu_DrawDisplayModeOnLocalOtAndSync` calls
it in overlay mode 20.
