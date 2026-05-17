# Recent naming-pass findings (2026-05-17)

This addendum captures clusters and patterns that emerged from the
data-symbol naming sprint (batches 2-14 + string-literal pass + medium
promotion pass), in a single place.  These were not previously
documented because the constituent symbols were unnamed when the
existing engine docs were written.  Each section names the cluster's
members and explains the role they play together.

> **For each cluster:** the canonical name is what's in
> `named_syms.txt`; original addresses are kept in parens for
> grep-ability.  Subsystem docs that already touch these clusters get
> the same names via the rewriter pass (`tools/...` -- see commit log).

## 1. BIOS event-handle cluster (8 handles)

PSX BIOS event handles, opened in `memcard_event_init`
(`src/code6cac_c.c:175`, the function still mis-named `pad_press_control`
in source -- see [MISNOMERS.md](../naming/MISNOMERS.md)).

| Symbol | Address | Class | Mode | Purpose |
|---|---|---|---|---|
| `g_memcard_event_io_complete` | 0x800A37DC | 0xF4000001 | 4 | Memcard I/O complete |
| `g_memcard_event_disconnect` | 0x800A37F0 | 0xF4000001 | 0x8000 | Memcard disconnect/error |
| `g_memcard_event_count_io` | 0x800A37FC | 0xF4000001 | 0x100 | Memcard count/timeout |
| `g_memcard_event_general_error` | 0x800A3800 | 0xF4000001 | 0x2000 | Memcard general error |
| `g_kernel_event_io_complete` | 0x800A3838 | 0xF0000011 | 4 | Kernel I/O complete |
| `g_kernel_event_disconnect` | 0x800A383C | 0xF0000011 | 0x8000 | Kernel disconnect |
| `g_kernel_event_count_io` | 0x800A3848 | 0xF0000011 | 0x100 | Kernel count/timeout |
| `g_kernel_event_general_error` | 0x800A3850 | 0xF0000011 | 0x2000 | Kernel general error |

Two parallel pairs of 4 events (memcard class + kernel class), each
covering completion / disconnect / count / general error.  After
opening, each handle is `bios_EnableEvent`'d.

Two additional event handles for the root-counter (timer/vblank) class
sit nearby:

| Symbol | Address | Class | Mode |
|---|---|---|---|
| `g_root_counter_event_handle` | 0x800A3810 | 0xF000000B | 0x8000 |
| `g_root_counter_callback_event` | 0x800A3738 | 0xF000000B | 0x400 |

The root counters use mode 0x8000 (error notification) and 0x400
(callback delivery).

**Memcard file handles** (opened by the same init code via
`bios_FileOpen_B` with the filename at `D_800A3210` = `"sio:"`):

| Symbol | Address | Mode |
|---|---|---|
| `g_memcard_file_handle_a` | 0x800A3734 | 0x8001 (write-create-excl) |
| `g_memcard_file_handle_b` | 0x800A373C | 2 (read) |

See also: [psyq_usage.md](psyq_usage.md) for event-pool sizing
discussion.

## 2. Scratchpad RAM pointer cache (7 cached pointers)

PSX has 1 KB of fast on-CPU "scratchpad" SRAM at `0x1F800000`-`0x1F8003FF`.
The `gnd_get_fog` initialiser caches seven specific scratchpad addresses
as ordinary main-RAM globals so that subsequent reads avoid the `lui`/
`addiu` pair for the literal:

| Symbol | Address | Scratchpad target |
|---|---|---|
| `g_scratchpad_ptr_18` | 0x800A346C | 0x1F800018 |
| `g_scratchpad_ptr_20` | 0x800A3470 | 0x1F800020 |
| `g_scratchpad_ptr_30` | 0x800A3474 | 0x1F800030 |
| `g_scratchpad_ptr_a4` | 0x800A34E8 | 0x1F8000A4 |
| `g_scratchpad_ptr_a8` | 0x800A3480 | 0x1F8000A8 |
| `g_scratchpad_ptr_ac` | 0x800A3484 | 0x1F8000AC |
| `g_scratchpad_ptr_b8` | 0x800A34EC | 0x1F8000B8 |

The bulk-init pattern (`func_80060E38`) sets all seven in one pass.
Consumers (`func_80060B70`, `func_80061FAC`, etc.) then `lw` the cached
pointer instead of materialising the literal each time.

There's also a 0x3E0-byte main-RAM mirror at `g_scratchpad_save`
(0x800F5370) used by `scratchpad_Save`/`scratchpad_Restore` for IRQ
context preservation.

See also: [memory_layout.md](memory_layout.md) for the scratchpad
discussion.

## 3. Match-round bookkeeping cluster

Per-round bookkeeping for the match (best-of-N rounds):

| Symbol | Address | Role |
|---|---|---|
| `g_match_round_index` | 0x800A3874 | Monotonic round counter (0, 1, 2, ...) |
| `g_match_round_byte_a` | 0x800F65F8 | Byte array, stride 2, [round*2] = byteA |
| `g_match_round_byte_b` | 0x800A389B | Companion byte array (paired with byte_a) |
| `g_match_round_result_table` | 0x800A377C | Byte array of per-round outcomes (0/1/2 enum) |

The round index is bumped by `func_800340A0` after each round; the
two parallel byte arrays at `g_match_round_byte_a/b` store the
per-round outcome bytes; `g_match_round_result_table` holds the
final outcome enum.  `_GetBattleSwichData` consumes these to decide
whether to advance to the next round or end the match.

See also: [combat.md](combat.md) for the round flow.

## 4. Replay-attack position triple + state

The replay camera selects an "attack number" based on the average
position of two world-space points (probably player + opponent
midpoint).  The (X, Y, Z) triple is computed in `sound.c:func_80046BF4`
and consumed by `replay_camera_get_attack_number`:

| Symbol | Address | Computed as |
|---|---|---|
| `g_replay_attack_pos_x` | 0x80101E3C | `result[0] + a0[0]` |
| `g_replay_attack_pos_y` | 0x80101E40 | `result[1] + a0[1]` |
| `g_replay_attack_pos_z` | 0x80101E44 | `result[2] + a0[2]` |

There's also a separate (X, Y, Z) for the "input position" with
negated components:

| Symbol | Address | Computed as |
|---|---|---|
| `g_replay_pos_x` | 0x80101E02 | `-(s16)a1[1]` |
| `g_replay_pos_y` | 0x80101E04 | `-(s16)(*new_var)` |

State / control:

| Symbol | Address | Role |
|---|---|---|
| `g_replay_camera_state` | 0x80101E70 | Volatile state (concurrent with IRQ?) |
| `g_replay_camera_init_done` | 0x80101E62 | Init-done flag |
| `g_replay_camera_mode` | 0x800A3740 | Mode enum {1, 3, 4} |
| `g_replay_camera_target_a` | 0x80101FBC | Replay-cam target arg A |
| `g_replay_camera_target_ptr_b` | 0x800A370C | Replay-cam target struct ptr (+0x18 offset) |
| `g_replay_frame_offset` | 0x80101E74 | Frame offset (BCD + pre-roll) |
| `g_replay_pos_change_count` | 0x800A38D6 | Position-change tick counter |
| `g_replay_attack_pos_x/y/z` | above | Attack-number selection coords |

See also: [replay.md](replay.md).

## 5. text1b render-state cluster

`src/text1b.c` has a render path that maintains its state in a
struct at `g_text1b_render_state` (0x800F116C).  A pointer to the
struct is cached at `g_text1b_render_state_ptr` (0x800A3468); the
deeper renderer derefs that pointer rather than taking the address
of the struct each time.  Three additional cached pointers point at
specific fields within the struct:

| Symbol | Address | Points to |
|---|---|---|
| `g_text1b_render_state` | 0x800F116C | Struct base |
| `g_text1b_render_state_ptr` | 0x800A3468 | Pointer to above |
| `g_text1b_render_buf_ptr` | 0x800A347C | state + 0x20 (volatile; IRQ shared?) |
| `g_text1b_render_dst_ptr` | 0x800A3478 | state + 0x18 (volatile; sibling of buf_ptr) |
| `g_text1b_glyph_cursor` | 0x800F1180 | Pointer-cursor over glyph data |
| `g_text1b_render_arg_cache` | 0x800F1178 | Caller-arg passthrough |

Three different entry points (`func_800611A4`, `func_80061250`,
`func_8006133C`) initialise the cursor to a different glyph base
(`&D_800F116A`, `&D_800F1159+1`, `&D_800F1154`); the deeper render
walker then advances it as it consumes glyphs.

Glyph buffers in the same address neighborhood:

| Symbol | Address |
|---|---|
| `g_text1b_glyph_buf_b` | 0x800F1154 |
| `g_text1b_glyph_buf_c` | 0x800F1160 |
| `g_text1b_glyph_buf_d` | 0x800F1164 |
| `g_text1b_glyph_buf_e` | 0x800F1168 |

The bit `0x200000` at struct +0x0 is the "render valid" flag; the
byte at +0x14 is the output-cursor pointer.

## 6. GTE register cache

The GTE background color register is set as a triple:

| Symbol | Address | Channel |
|---|---|---|
| `g_gte_back_color_r` | 0x800F6338 | R (u8) |
| `g_gte_back_color_g` | 0x800F6339 | G (u8) |
| `g_gte_back_color_b` | 0x800F633A | B (u8) |

Loaded as a triple via `gte_SetBackColor(D_800F6338, D_800F6339, D_800F633A)`.

See also: [gpu_pipeline.md](gpu_pipeline.md) for the GTE setup
sequence.

## 7. SPU register cluster

| Symbol | Address | Role |
|---|---|---|
| `g_spu_reg_base_ptr` | 0x800A3044 | SPU register block pointer |
| `g_spu_reg_pair` | 0x800F7420 | volatile u16[2] register pair A |
| `g_spu_reg_pair_b` | 0x800F7424 | volatile u16[2] register pair B (adjacent) |
| `g_spu_reg_voice_array` | 0x800F7298 | volatile u16[] voice register array |
| `g_spu_voice_mask` | 0x800A28A0 | voice key-on/off mask (bit = (regaddr - 0xC6) >> 1) |
| `g_spu_xfer_mask` | 0x800A2874 | transfer channel mask |
| `g_spu_xfer_callback_arg` | 0x800A287C | stashed transfer callback arg |
| `g_spu_xfer_pending` | 0x800A2D2C | transfer-pending flag (0 -> 1 around xfer) |
| `g_spu_pending_irq_mask` | 0x800A2CD4 | pending IRQ mask (& 1 test) |

See also: [sound.md](sound.md).

## 8. IRQ / alarm state cluster

| Symbol | Address | Role |
|---|---|---|
| `g_alarm_armed_flag` | 0x800A26D0 | armed boolean |
| `g_alarm_callback_ptr` | 0x800A26D8 | function-pointer / arg for callback |
| `g_alarm_callback_pending` | 0x800A26DC | callback-pending boolean |
| `g_alarm_active_sentinel` | 0x800A26DE | 0x7F sentinel = alarm pending |
| `g_irq_alarm_handle` | 0x800A14E4 | volatile alarm handle (-1 = none) |
| `g_psyq_io_vec_table` | 0x800A2600 | PsyQ IRQ dispatch vector table |
| `g_vblank_count` | 0x800A157A | halfword incremented per vblank |
| `g_irq_dispatch_counter` | 0x800F19BC | IRQ dispatch sequence counter |
| `g_vsync_timeout_deadline` | 0x800F19B8 | sys_VSync(-1) + 0x3C0 timeout target |

`irq_ProcessPending` walks the entire cluster: enters critical
section, checks `g_alarm_armed_flag`, dispatches `g_alarm_callback_ptr`
via `irq_EnableInterrupts`, writes `g_alarm_active_sentinel = 0x7F`,
exits critical section.

See also: [main_loop.md](main_loop.md).

## 9. CDROM callback state

| Symbol | Address | Role |
|---|---|---|
| `g_cdrom_callback_buf_a` | 0x800F19A0 | 8-byte payload buffer for `cdrom_SetCallbackA` |
| `g_cdrom_callback_buf_b` | 0x800F19A8 | 8-byte payload buffer for `cdrom_SetCallbackB` |
| `g_panic_msg_format_ptr` | 0x800F19C0 | cached debug_printf format string ptr |

The two callbacks are wired via the `cdrom_SetCallbackA/B` functions
(already named) which install callback function pointers; when the
IRQ fires, the BIOS calls the callback passing one of these buffers.

See also: [file_io.md](file_io.md).

## 10. Discovered misnomer: `g_file_disc_size` (0x80106A50)

The pre-existing name `g_file_disc_size = 0x80106A50` is **wrong**.
Actual usage in `src/code6cac_b.c:3524` and `:3548`:

```c
s32 bits = D_80106A50 & mask;
if (D_80106A50 & 0x10020) ...
```

And documented in [ai.md](ai.md):

> Walks bits 0..26 of `D_80106A50` masked against the stance's enable
> bitmap; for each enabled bit, appends the move ID to a buffer ...

This is a **move-enable bitmap**, not a disc size.  A better name would
be `g_move_enable_bitmap` or similar.  See [MISNOMERS.md](../naming/MISNOMERS.md)
for the misnomer audit log.  Renaming is deferred because the existing
name is referenced from `src/*.c` and changing it requires a
src/ rename pass (which carries SHA1-mismatch risk if done casually
during the parallel decomp work).

## Cross-reference index

For a full lookup of name → address → primary file, see
[cross_reference.md](cross_reference.md).  The most-recently-added
symbols (this addendum) are folded into the canonical map in
`named_syms.txt`; the per-symbol prose evidence lives in
[../naming/data_evidence/](../naming/data_evidence/) and
[../naming/evidence/](../naming/evidence/).
