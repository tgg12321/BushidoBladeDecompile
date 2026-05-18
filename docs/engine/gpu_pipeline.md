# GPU / Render Pipeline

BB2 uses the PS1 GPU's standard ordering-table + DMA model. The CPU builds a
linked list of GPU command packets every frame, and the GPU DMA controller
walks the list to feed packets to the GPU's command FIFO. The GTE
(Geometry Transformation Engine, COP2) does perspective projection and
matrix work in parallel with CPU control flow.

GPU helper code is mostly in `gpu.c` (low-level wrappers, 0x8007A28C..0x8007B244)
and `display.c` (frame-environment + DMA helpers, 0x8007B244..0x8008008C),
with the per-fighter bone+vertex code spread across `text1b.c`.

## The Ordering Table (OT)

A PS1 "ordering table" is a backwards linked list. Each entry is 32 bits:
- High 8 bits: the size (in 32-bit words) of the primitive that follows
- Low 24 bits: the address of the next entry

Conceptually it's "draw this primitive, then jump here to draw the next".
The list is **back to front** — entry 0 is the furthest, entry N-1 is
the closest. The GPU walks from entry 0 forward, painting back-to-front
(painter's algorithm). Primitives at the same distance can be linked into
the same OT slot (depth bucket).

BB2 typically uses `0x1008` entries per OT (~4 KB), per `func_8007B844`
calls in `ings.c:608`. That's a depth resolution of 4104 buckets — enough
for 3D scene + UI overlays.

### OT helpers

| Function | Defined in | Purpose |
| --- | --- | --- |
| `gpu_ClearOTag` | `display.c:189` | Initialize OT chain — backward-link all entries to point at the next; tail points at `g_gpu_ot_end` |
| `func_8007B844` | `display.c:211` | Variant that uses libgpu's `ClearOTagR` (reverse-order clear) — most common |
| `gpu_DrawOTag` | `display.c:229` | Kick the GPU DMA to walk the OT (`g_gpu_dev_table[6]`) |
| `gpu_CatPacket` | `gpu.c:556` | Insert a packet into the OT (low-level, prepends to slot) |

The PS1 OT macros are inlined directly in code: `*ot = (next_addr & 0x00FFFFFF) | (size << 24);`

### OT submission per frame

Looking back at the main loop in `ings.c:584`:

1. `func_8007B844(ot, 0x1008)` — clear OT for this frame
2. `D_800A374C = ot;` — make it the "active OT" for sub-engines
3. ... mode dispatch fills it with packets ...
4. `gpu_DrawOTag(env + 0x408C)` — kick GPU DMA

The `env + 0x408C` is the OT's tail entry (the high-numbered, nearest, last-
to-be-drawn slot). DMA walks backward toward entry 0.

## DrawEnv / DispEnv

The DRAWENV (`g_gpu_draw_env`, 0x5C bytes) holds the current draw-area
clip rect, the current texpage, and the active CLUT. The DISPENV
(`g_gpu_disp_env`, 0x14 bytes) tells the GPU which slice of VRAM to scan out.

Per-buffer layout (size `0x4090`, base at `g_disp_fb_base = 0x800F7438`):

```
+0x0000: DRAWENV  (0x5C bytes)
+0x005C: DISPENV  (0x14 bytes)
+0x0070: OT array (0x4000 bytes = 0x1000 entries * 4 bytes)
+0x4070: ... padding ...
+0x4090: end of slot (next slot starts here)
```

There are two of these (buffer A and B), at `g_disp_fb_base + 0` and
`g_disp_fb_base + 0x4090`. The main loop chooses one per frame via
`D_800A36AC & 1`.

Frame submit sequence (`ings.c:638-642`):
- `func_8007BC08(env + 0x5C)` → libgpu `PutDispEnv` (send DISPENV to GPU)
- `func_8007B9B0(env)` → libgpu `PutDrawEnv` (send DRAWENV to GPU)

After both are sent, the GPU is configured to draw and display this slot
on the next vsync.

## GPU command path — the device table

The engine maintains a function pointer table at `g_gpu_dev_table`
(`0x8009BE6C`). The functions are PsyQ libgpu's low-level access primitives:

| Index | Purpose (inferred) |
| --- | --- |
| 0 | GPU reset / init |
| 1 | Display mask (on/off) |
| 2 | Send command (`(cmd, packet, size, extra)`) — main packet sender |
| 3 | Clear-image command (used by `func_8007B4D0`) |
| 4 | Display enable / mask (gpu_SetDispMask helper) |
| 5 | Send variable-length data block |
| 6 | DrawOTag (used by `gpu_DrawOTag`) |
| 7 | StoreImage |
| 8 | LoadImage |
| 9-10 | Mode / state |
| 11 | ClearOTag (reverse-order, used by `func_8007B844`) |
| 14 | IsDrawing |
| 15 | DrawSync |

Calls go through this indirection because libgpu has multiple back-ends
(NTSC vs PAL, debug emulator vs real hardware). PsyQ's debug builds
include a syntax checker; the released BB2 build uses the production
back-end.

Examples:
- `gpu_LoadImage(rect, source)` (`display.c:145`) calls
  `g_gpu_dev_table[2](g_gpu_dev_table[8], rect, 8, source)` — i.e., send
  the LoadImage command (index 8) using the generic sender (index 2).

## DMA channels used

BB2 uses three PS1 DMA channels:

- **Channel 2: GPU**. Linked-list mode for OT submission. Triggered by
  `gpu_DrawOTag` writing `0x01000401` (`DMA_GPU_LINKED_LIST`) to
  `*g_gpu_dma_chcr`. Source is the OT's tail entry.
- **Channel 3: CD-ROM**. Used for sector-aligned `bios_FileRead` and
  XA-ADPCM streaming. The GPU shares its OT direction control with the
  CD-ROM DMA — see `gpu.c`'s GP1_DMA_DIR_FIFO toggle.
- **Channel 4: SPU**. Used for sound bank uploads to SPU RAM
  (`spu_DmaTransfer`, `main.c:1949`).

The GPU DMA flag `GP1_DMA_DIR_FIFO` (`0x04000002`, `psx.h:20`) puts the GPU
DMA into "linked list" mode, which is what walks the OT. The alternative
(`GP1_DMA_DIR = 0x04000000`) is plain block transfer for `LoadImage`.

## GTE usage

BB2 uses the standard PS1 GTE (COP2) for matrix math:

- **Rotation matrix setup**: `gte_SetRotMatrix(mat)` loads a 3x3 rotation
  into GTE registers $0-$5 via ctc2.
- **Translation vector**: `gte_SetTransVector(vec)` loads a 3-vec
  translation into GTE registers $5-$7.
- **Single-vertex projection**: `gte_rtps()` (`.word 0x4A180001`, see
  `gte.h:59`) projects one vertex from world → screen space using the
  currently-loaded matrix + translation. Result in screen X/Y/Z registers
  ($14, $19).
- **Triangle projection**: `gte_rtpt()` (`.word 0x4A280030`) projects 3
  vertices at once — used for solid/textured polys.
- **Average Z**: `gte_avsz3()`/`gte_avsz4()` compute the OT slot index from
  the projected Z values of 3 or 4 vertices, for OT insertion.
- **Cross product**: `OP` (`.word 0x4B70000C`) — used in
  `cpu_check_tubazeri` for the blade-vs-blade orientation test
  (see [combat.md](combat.md)).
- **Sqrt distance**: `mtc2 $0, $30; swc2 $31, 0(addr)` — used to compute
  the count-leading-zeros for `Sqrt(distance)` via the scratchpad. The
  encoding `.word 0x488CF000` is `mtc2 $t4, $30`, and `.word 0xE99F0000`
  is `swc2 $31, 0($t4)`. Both BB2 and Kengo use this idiom heavily; see
  `code6cac.c:291-297` for an example.

The GTE has a tight pipeline; back-to-back ops can stall. Most BB2 GTE
sequences interleave `mtc2/ctc2/lwc2` loads with CPU arithmetic between
ops so the GTE is busy when the CPU isn't waiting.

See `include/gte.h` for the full macro set the project uses to wrap GTE ops
in inline asm.

## Camera matrix — `g_cam_matrix` (`0x800EEDB0`)

The camera rotation matrix lives in a small struct starting at
`g_cam_matrix`:

| Offset | Field | Use |
| --- | --- | --- |
| 0x00 | g_cam_matrix (the s16 itself = matrix m[0][0]) | rotation 3x3 |
| 0x02..0x10 | matrix m[0][1..2], m[1][0..2], m[2][0..2] | rotation |
| 0x12..0x1B | translation t[0..2] (s32) | translation |
| 0x20.. | second matrix | secondary (interpolation target) |
| 0x40.. | bone data | per-frame view bone |
| 0x48.. | bone data 2 | secondary view bone (special camera) |

`camera_InitMatrix` (`sound.c:396`) builds an identity rotation with
FOV-scaled X/Z. `camera_Transform` (`sound.c:415`) applies the camera
matrix to a point (using `func_80052930` — the GTE matrix-multiply
wrapper).

## Display setup at boot — `disp_Init`

`disp_Init` (`ings.c:276`) sets up the GPU at boot:

1. `gpu_SetMode(0)` — initialize PsyQ libgpu
2. `gpu_SetDebugLevel(0)` — disable verbose debug
3. `gpu_SetDispMask(0)` — display off during setup
4. `func_8007E094()` — InitGeom (init the GTE)
5. `gte_SetScreenOffset(0x140, 0x78)` — center of screen at (320, 120)
6. `tslDmaDrawListDelAll(disp_CalcFov(0x2D))` — set FOV to 45 degrees
   (`disp_CalcFov(0x2D) = cos(22.5°)*320/sin(22.5°)`)
7. `gpu_InitDrawEnv` for both buffers (640x240 each)
8. `gpu_InitDispEnv` for both buffers (640x240 display window)
9. `disp_SetFramebufferMode(1, 0, 0, 0)` — enable display mask, no dither
10. `func_8007B4D0(&g_disp_gp_base, 0, 0, 0)` — clear screen to black
11. `gpu_DrawSync(0)` — wait for clear to complete

After `disp_Init`, `gpu_SetDispMask(1)` is called from
`cpu_set_move_command_and_dir_for_no_action_2` (the main loop entry) to
turn on the display.

## Special render features

### Zanzou (afterimage)

`efc_buki_draw_zanzou` (`text1b.c:189`) draws the weapon's afterimage trail
— a sequence of fading silhouettes of recent weapon positions. Inputs:
weapon mesh pointer, three rotation params, alpha.

Used during attack animations to give the visual "speed-blur" effect that
makes BB2's combat read clearly despite its slow per-frame pace.

### Particles / Effects

`text1a*.c` files contain `efc_*` (effect) functions for blood spray, sparks,
dust, etc. Most of these are still asm-only. The effect-channel table is
managed by `text1a_c.c` (the channel/SE helpers, `0x80042504..0x800460E4`).

### Fog

`gnd_get_fog` (called every frame from `ings.c:611`) sets the per-frame fog
color/distance for the stage. The actual data is in two helper functions
`gnd_set_fog` (`0x80043C7C`) and `gnd_set_fog2` (`0x80043DE0`).

### UI / Life Gauge

`game_2d_CheckLifeGaugeNoDisp` (`text1b.c:10798`) and the `saTan*Gauge*`
family render the in-game HUD. Each gauge has init / main pair functions
(see [combat.md](combat.md) under "Mental gauge").

### Stage background

Loaded from `disc/U_PIC/STGNN.BIN`. Each STG file is a TIM-format texture
backdrop for one stage. `tslDrTex1Init*` family functions
(`named_syms.txt`) initializes the background texture quads.

## Where unmatched render asm still hides

- `func_8007BC08` — `PutDispEnv` wrapper (asmfix-bridged in `display.c:303`)
- `func_8007D048` — image-LoadImage continuation (asmfix-bridged)
- All `calc_loc_mat_fw_*` variants (the bone-matrix builders)
- Many `Vu0SetLightColMatrix*` functions in `system.c` — light-color matrix
  setup
- Most of the `text1b.c` 10558-11227 range (projection/screen helpers)

## PSX libgpu wrappers identified (2026-05-17)

The engine wraps several PSX libgpu primitive submission routines in named
helper functions, identified by their debug-trace strings:

| Helper | Address | Body / debug string |
|--------|---------|---------------------|
| `gpu_DrawSyncCallback` | `0x8007B244` | Save old `g_gpu_draw_mode`, set new, return old. Debug trace: `"DrawSyncCallback(%08x)"`. PSX libgpu DrawSyncCallback equivalent. |
| `gpu_DebugCheckRect` | `0x8007B3A8` | Validates rect against `D_8009BE78/7A` (screen width/height). Debug-logs `"%s:bad RECT"` and `"(%d,%d)-(%d,%d)"`. Called by other primitive wrappers as a pre-check. |
| `gpu_MoveImage` | `0x8007B6C8` | `MoveImage(RECT *rect, int x, int y)` — VRAM rect-copy primitive. Submits cmd 0x14 via `g_gpu_dev_table[2]`. |
| `gpu_DrawOTagEnv` | `0x8007BAB4` | `DrawOTagEnv(u_long *ot, DRAWENV *env)` — submits cmd 0x40 (= 64-byte DRAWENV size). Copies `_drawenv_q` struct. |

## GPU packet queue — async draw infrastructure

The engine maintains a 64-slot circular queue of GPU "draw packets" submitted
via callbacks, located at `g_gpu_packet_queue_base` (`0x80103680`):

| Field | Address | Type | Purpose |
|-------|---------|------|---------|
| `g_gpu_packet_queue_base` | `0x80103680` | array | Slot[i] = callback fn ptr (offset 0 in 0x60-byte slot) |
| `g_gpu_packet_buffer_64x96` | `0x80103684` | array | Slot[i] = arg1 (offset 4); 64 slots × 96 bytes total |
| | `0x80103688` | array | Slot[i] = arg3 (offset 8) |
| | `0x8010368C` | array | Slot[i] = data buffer (offset 0xC) |
| `g_gpu_packet_write_idx` | `0x8009BF78` | s32 | Head pointer (mod 0x40 = 64) |
| `g_gpu_packet_read_idx` | `0x8009BF7C` | volatile s32 | Tail pointer (drained by handler) |
| `g_gpu_packet_pending_fn` | `0x8009BF68` | fn ptr | Last submitted callback fn |
| `g_gpu_packet_pending_arg1` | `0x8009BF6C` | s32 | Last submitted arg1 |
| `g_gpu_packet_pending_target` | `0x8009BF70` | s32 | Last submitted arg3 |
| `g_gpu_motion_save_a` | `0x8009BF80` | s32 | `motion_make_table(0)` state saved during async draw |
| `g_gpu_motion_save_b` | `0x8009BF88` | s32 | Parallel save for outer caller |
| `g_gpu_loop_flag` | `0x8009BE7C` | s32 | Set to 1 during successful packet submit |
| `g_gpu_status_alt_reg` | `0x8009BF60` | u32* | Parallel status reg; written `0x11000002` (GPU primitive header) |

Each queue entry is a 0x60-byte slot containing a callback function and its
saved args. When the GPU is ready, the handler dispatches the queued callback
to actually emit primitives. See `display.c:880-960` for the enqueue path
(`func_8007D9C4` and family).

## GPU mode tables

The engine supports multiple video modes (height/width combinations):

| Symbol | Address | Purpose |
|--------|---------|---------|
| `g_gpu_mode_heights_table` | `0x8009BF08` | 5 s16/s32 height values indexed by mode |
| `g_bitmask_table_5_8009BEF4` | `0x8009BEF4` | 5 width values (MISNAMED — actually `g_gpu_mode_widths_table`, see MISNOMERS.md) |
| `g_gpu_init_msg_buf` | `0x8009BE2C` | Buffer passed to debug_printf with `g_gpu_type` at gpu init |

The `s0->width = D_8009BEF4[idx]` / `s0->height = D_8009BF08[idx]` pattern
in `gpu.c:608-610` confirms the tables are paired and idx is the mode code.

## Cross-references (recent_naming_findings.md addendum 2026-05-17)

Five clusters from the placeholder-refinement pass document render-path
data more directly:

- [§16 4096-entry packed sin/cos lookup](recent_naming_findings.md#16-4096-entry-packed-sincos-lookup-d_8009c928)
  — `g_trig_sin_cos_table_packed` at `0x8009C928`, packed `(sin << 16) | cos`
  for one-load trig in `display.c:2570-2599` GTE inline asm. Distinct from
  the canonical `Judge` table (`0x800973FC`, sin-only).
- [§17 Display-state buffer + cursor](recent_naming_findings.md#17-display-state-buffer--cursor-d_800f33d8--d_800a36ec)
  — `g_disp_state_buf` (`0x800F33D8`, 512 bytes) + `g_disp_state_buf_cursor`
  (`0x800A36EC`, +0x100 = matrix region). Doubles as the memcard save/load
  payload (see `code6cac_c_mid.c:507/524`).
- [§18 Camera view-state struct](recent_naming_findings.md#18-camera-view-state-struct-d_800ff558-0x140x1c-is-position)
  — `g_camera_view_state` at `0x800FF558`. Distinct from `g_cam_matrix`
  (`0x800EEDB0`); position fields at +0x14/+0x18/+0x1C, distance-threshold
  check against player pos (text1b.c:2359-2435).
- [§19 Sprite size packed lookup](recent_naming_findings.md#19-sprite-size-packed-lookup-d_8009b850)
  — `g_text1b_sprite_size_packed_lookup` at `0x8009B850`. Per-entry encoding:
  low 7 bits = (height − 0x2A), top 9 bits = (width − 0x37).
- [§21 text1b draw-primitive data cluster](recent_naming_findings.md#21-text1b-draw-primitive-data-cluster-0x8009b3400x8009b850)
  — 19+ tables at `0x8009B340..0x8009B850` used as `s.p0` / `s.p1` /
  `s.p_geom` / `s.p_static` fields of a draw-submission struct, named with
  role prefixes (`g_text1b_draw_p0_*`, `_p1_*`, `_geom_*`, `_static_*`).
