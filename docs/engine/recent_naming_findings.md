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

> **Note on dual naming**: a second naming convention exists for these
> same 8 handles in `named_syms.txt`: `g_memcard1_event_ioe/err/new/timeout`
> (for class 0xF4000001) and `g_rcnt0_event_ioe/err/new/timeout` (for
> class 0xF0000011).  Both sets are valid aliases for the same addresses;
> the `_ioe/err/new/timeout` names are derived from PSX BIOS event-type
> bit codes (0x0004 = IOE, 0x8000 = ERROR, 0x0100 = NEW, 0x2000 = TIMOUT)
> and match the BIOS docs more directly.  Use either set; the build
> resolves both.

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
| `g_replay_pos_z` | 0x80101E44 | `result[2] + a0[2]` (canonical name in named_syms.txt; doc previously called this `g_replay_attack_pos_z`) |

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

Glyph buffer / slot-status entries in the same address neighborhood:

| Symbol | Address | Notes |
|---|---|---|
| `g_text1b_glyph_buf_b` | 0x800F1154 | |
| `g_text1b_slot_a_flags` | 0x800F1160 | u8[2] busy-flag pair (was `g_text1b_glyph_buf_c` in earlier doc; renamed after body re-analysis showed it's used as a 2-byte status check) |
| `g_text1b_slot_c_flags` | 0x800F1164 | u8[2] busy-flag pair (was `g_text1b_glyph_buf_d`) |
| `g_text1b_glyph_buf_e` | 0x800F1168 | |

Note: `g_text1b_slot_b_flags` (0x800F1152) is a sibling 2-byte busy-flag
pair from func_800618B4's slot family.  The bytes act as a "buffer
status pair": if `[0]` and `[1]` are both set, the consumer treats the
slot as fully-used and clears both; otherwise it submits a primitive
referencing the slot.

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

# Placeholder-refinement pass addendum (2026-05-17, batch 17-30)

This addendum captures cluster discoveries from the placeholder-refinement
sprint that converted ~170 `g_high_ref_*` / `g_*_addr_*` placeholders into
semantic names.  Patterns surfaced from re-reading the C bodies that
already referenced these addresses; each cluster is named by what the C
code does with it (array stride, indexed call pattern, init constants,
struct field offsets).

## 11. Sequence-event handler table (MIDI-style dispatch, 5 slots)

A 5-entry function-pointer table at `0x800F3340..0x800F3350` invoked from
`saTan0Main` (main.c:334-454) — the **per-character sequence-stream event
interpreter** that parses a MIDI-encoded command stream and dispatches
each status byte to its handler:

| Symbol | Address | Status | Handler role |
|---|---|---|---|
| `g_seq_event_handler_90_NoteOn`    | 0x800F3340 | 0x90 | Note On — reads {prev=note, next=velocity}; also triggers `spu_ReadMotionFrame()` |
| `g_seq_event_handler_B0_CtrlChange`| 0x800F3350 | 0xB0 | Control Change — reads `next=controller_byte` |
| `g_seq_event_handler_C0_PgmChange` | 0x800F3344 | 0xC0 | Program Change — reads `next=program_byte` |
| `g_seq_event_handler_E0_PitchBend` | 0x800F3348 | 0xE0 | Pitch Bend |
| `g_seq_event_handler_FF_Meta`      | 0x800F334C | 0xFF | Meta Event (except 0x2F end-of-track, which goes to `func_80084A7C`) |

`saTan0Main(a0=char_id, a1=channel)` reads the next byte from the stream
at `((void**)&D_80106F28)[a0] + (a1 * 0xB0)` (per-character × per-channel
state struct, 0xB0 bytes/channel).  It implements MIDI "running status"
(if the next byte's high bit is clear, re-use the last status byte stored
at `state[0x16]`).

Note `g_seq_event_handler_90_NoteOn` has signature `void (s16, s16, u8, u8)`
— the trailing arg is the *velocity byte*.  The other 4 handlers have
signature `void (s16, s16, u8, u8 *)` — the trailing arg is the
*remaining-stream cursor* for handlers that need to consume additional
bytes (e.g., variable-length Meta events).

**Init site — verified NOT EXIST.** A byte-level scan of the main EXE
+ MOVOVL.EXE overlay found ZERO instructions that write to any of these
5 slot addresses:
- 0 sw/sh/sb with offset 0x3340..0x3350 (any base register)
- 0 `ori`/`addi`/`addiu` with immediate 0x3340..0x3350
- 0 literal 32-bit value of 0x800F3340/4/8/C/50 anywhere in the binary
- 0 references in any asm/data, asm/funcs, or src C/H/.s file

The 5 LOADS in saTan0Main (one per slot) are the ONLY references in the
codebase.  Since the EXE header declares `bss_size = 0`, the loader does
not pre-zero these addresses — they hold whatever was in PSX RAM at boot.

This means either: (a) the `D_800F33xx(...)` call arms in saTan0Main are
effectively unreachable in practice (the `if (b & 0x80)` branch + switch
arms are never hit because the stream byte's high bit is never set), or
(b) some PSX BIOS / kernel mechanism populates the slots at boot
(unlikely — BIOS does not know about game-specific dispatch tables).
Most likely interpretation: **the dispatch table was implemented for a
debug build / future SDK feature, but never populated in the shipped
ROM**.  See [project_satan0main_midi_dispatch.md] (memory) for
investigation details.

## 12. text1b accumulator cluster (6 s32 accumulators)

A bank of six `s32` accumulators at `0x800EFB14..0x800EFB28`, updated by
`func_80054FDC(a0)` in text1b.c:11363-11378.  All six receive `+= a0`;
the first two are unconditional, the latter four gated by their own
non-zero check (allowing per-slot enable/disable).

| Symbol | Address | Update gate |
|---|---|---|
| `g_text1b_accum_0` | 0x800EFB14 | unconditional (via `*p` indirection) |
| `g_text1b_accum_1` | 0x800EFB18 | unconditional |
| `g_text1b_accum_2` | 0x800EFB1C | only if non-zero |
| `g_text1b_accum_3` | 0x800EFB20 | only if non-zero |
| `g_text1b_accum_4` | 0x800EFB24 | only if non-zero |
| `g_text1b_accum_5` | 0x800EFB28 | only if non-zero |

`func_8005507C` returns `&g_text1b_accum_base_ptr_FB0C` (0x800EFB0C),
which is 8 bytes earlier than accum_0.  Layout suggests accum_0..5
follow a header block at 0x800EFB0C-0x800EFB13.  Role: likely
time-since-event timers or per-effect duration counters that one global
tick function increments together each frame.

## 13. sound.c voice-init constants (D_800EF070-0xC4)

The init block at sound.c:780-798 sets up voice ID 0xE with these
constants:

| Symbol | Address | Value | Role |
|---|---|---|---|
| `g_voice_init_vol_offset` | 0x800EF0BC | -0x2EE0 (-12000) | starting volume base |
| `g_voice_init_pitch_offset` | 0x800EF0C4 | -0xFA0 (-4000) | starting pitch base |
| `g_voice_init_field_C0` | 0x800EF0C0 | 0 | zero-init field |

Both magic numbers reappear in the envelope generator (sound.c:920-950):
`vol = -0x2EE0` per-iteration init, and `val = *rp - 0xFA0` per-word
subtraction.  Thus they document the **envelope baseline** that pitch &
volume start from before the per-frame ramp:

```
vol += 0x7D0  (2000 per frame)  starting from -0x2EE0
pitch = src - 0xFA0             (subtract baseline from raw input)
```

## 14. Voice envelope/parameter blocks (scratchpad-DMA pair)

Two 72-byte (0x48) data blocks copied to PS1 scratchpad each frame for
fast SPU access:

| Symbol | Address | Scratchpad target | Iteration |
|---|---|---|---|
| `g_voice_envelope_block_a` | 0x800EF0D8 | 0x1F800020 + 0x1F800110 | even (i & 1 == 0) |
| `g_voice_envelope_block_b` | 0x800EF168 | 0x1F800068 + 0x1F800134 | odd  (i & 1 != 0) |

The 0x90-byte gap (0x800EF0D8 -> 0x800EF168) implies each block is
~72 bytes long (8 active + padding, copied to two scratchpad regions
0x48 bytes apart at +0x20/+0x68 main + +0x110/+0x134 control).  The
scratchpad copy is per-iteration of the inner SPU update loop.

## 15. code6cac_c2 frame-event triggers + position offset (D_8008EB04..0x1C)

A small event/parameter cluster in code6cac_c2.c:873-897, all consumed
by the per-frame state machine driven by `g_practice_loop_frame`
(0x800A37B8) — i.e., this is **practice-mode scripted event handling**:

| Symbol | Address | Role |
|---|---|---|
| `g_c2_event_frame_a` | 0x8008EB04 | trigger event: `if (g_practice_loop_frame == D_...)` |
| `g_c2_event_frame_b` | 0x8008EB06 | same pattern, next slot |
| `g_c2_event_frame_c` | 0x8008EB08 | ... |
| `g_c2_event_frame_d` | 0x8008EB0A | ... |
| `g_c2_event_frame_e` | 0x8008EB0C | ... |
| `g_c2_pos_xyz_offset_x` | 0x8008EB10 | `vp[0] += D_...` (vertex X offset) |
| `g_c2_pos_xyz_offset_y` | 0x8008EB14 | `vp[1] += D_...` (vertex Y offset) |
| `g_c2_pos_xyz_offset_z` | 0x8008EB18 | `vp[2] += D_...` (vertex Z offset) |
| `g_per_frame_data_tbl_eb1c` | 0x8008EB1C | s16 stride table indexed by D_800A384C * 2 |

The 5 event-frame triggers fire single sub-handlers; the XYZ offset
triplet is added to a vertex position (likely a per-stage camera or
debug-cam shift).

## 16. 4096-entry packed sin/cos lookup (D_8009C928)

A trig lookup table used inside `display.c:2570-2599` GTE inline asm.
Each entry is 4 bytes packing `(sin << 16) | cos` so a single 32-bit
load delivers both components.  Indexing: `idx = (t7 << 2) & 0xFFC`
where `t7` is masked with `0xFFF` (= 4096 entries x 4-byte stride).

| Symbol | Address | Geometry |
|---|---|---|
| `g_trig_sin_cos_table_packed` | 0x8009C928 | 4096 x 4 bytes |

Distinct from `Judge` (0x800973FC, the canonical 4096-entry x 2-byte
sin-only PSX trig table). `g_trig_sin_cos_table_packed` is used in
texture/lighting paths needing both sin and cos for a single angle
without two loads.

## 17. Display-state buffer + cursor (D_800F33D8 + D_800A36EC)

A 0x200-byte (512) buffer that both fits the memcard save-slot frame
and serves as a display-state struct:

| Symbol | Address | Role |
|---|---|---|
| `g_disp_state_buf` | 0x800F33D8 | 512-byte buffer, zeroed by `func_80038148` |
| `g_disp_state_buf_cursor` | 0x800A36EC | `= &g_disp_state_buf` ptr cursor (indexed x 56) |

Usage cluster (code6cac_c_mid.c:322-524):
1. `func_80038170(&g_disp_state_buf)` -- populate display state
2. `camera_SetMatrix(&g_disp_state_buf + 0x100)` -- matrix at byte 256
3. `func_80037C34(0, 0, D_800A31F0, &g_disp_state_buf, 1, 0x200, ...)` -- memcard write 0x200 bytes
4. `func_80037B90(0, 0, D_800A31F0, &g_disp_state_buf, 0x200)` -- memcard read 0x200 bytes

The cursor at +0x36EC indexes through 56-byte records inside the
buffer (likely per-entry display-state records during enumeration).

## 18. Camera view-state struct (D_800FF558, +0x14..+0x1C is position)

Camera state struct at `0x800FF558`, identified by the position-delta
threshold check in text1b.c:2359-2435:

```
v0 = &g_camera_view_state
t0 = lw 44($s2)    // player_pos_x
t1 = lw 48($s2)    // player_pos_y
t2 = lw 52($s2)    // player_pos_z
t3 = lw 20(v0)     // camera_pos_x  (offset +0x14)
t4 = lw 24(v0)     // camera_pos_y  (offset +0x18)
t5 = lw 28(v0)     // camera_pos_z  (offset +0x1C)
// compute abs(player - camera) per component, OR together,
// then threshold-compare against 18944 (0x4A00) and 42240 (0xA500)
```

After the position check the struct is passed to a matrix helper
`func_80052930(&g_camera_view_state, $s2+24, $s1+72)` twice -- likely
building a model-view matrix from camera+player into the player struct.

Use named base + offset (e.g., `g_camera_view_state_plus_14`) to refer
to the XYZ position fields; the existing C body still uses the
raw `lw 20(v0)` form via inline asm.

## 19. Sprite size packed lookup (D_8009B850)

A per-sprite size-record lookup at text1b.c:12907-12908:

```c
s.width  = (((u16)(*((&D_8009B850) + (arg0 & 0x7FFF)))) >> 7) + 0x37;
s.height = (             *((&D_8009B850) + (arg0 & 0x7FFF))) & 0x7F  + 0x2A;
```

| Symbol | Address | Encoding |
|---|---|---|
| `g_text1b_sprite_size_packed_lookup` | 0x8009B850 | per-entry: low 7 bits = (height - 0x2A); top 9 bits = (width - 0x37) |

The bias constants (+0x37/+0x2A) suggest the table only stores
deltas from a min-size baseline.  arg0 & 0x7FFF gives 32768 max
entries (probably far fewer used in practice).

## 20. Motion-ex stride-12 XYZ triplet array (D_800F0E38)

Three separate base addresses (0x800F0E38, 0xE3C, 0xE40) used as
columns of a single stride-12 XYZ array.  text1b.c:14067-14070:

```c
*(s32 *)((u8 *)(&g_text1b_xyz_arr_E38_x) + word_off) = src[0];
*(s32 *)((u8 *)(&g_text1b_xyz_arr_E38_y) + word_off) = src[1];
*(s32 *)((u8 *)(&g_text1b_xyz_arr_E38_z) + word_off) = src[2];
```

`word_off` is a multiple of 12 (one element of the array = 12 bytes =
XYZ triplet).  Three "columns" indexed by the same offset.

| Symbol | Address | Role |
|---|---|---|
| `g_text1b_xyz_arr_E38_x` | 0x800F0E38 | X column |
| `g_text1b_xyz_arr_E38_y` | 0x800F0E3C | Y column |
| `g_text1b_xyz_arr_E38_z` | 0x800F0E40 | Z column |

Identical layout to `g_motion_ex_state_block_table_12` (0x800F0CA0)
but at a different anchor -- likely a parallel state array for a
separate motion-ex entity class.

## 21. text1b draw-primitive data cluster (0x8009B340..0x8009B850)

A large bank of GPU draw-primitive data tables at 0x8009B340 onward,
referenced as `s.p0` (geometry/vertex pointer), `s.p1` (texture or
primitive params), `s.p_geom`, `s.p_static` fields of a draw-submission
struct.  19+ tables identified, each named with its role prefix and
address suffix:

- `g_text1b_draw_p0_*` -- geometry buffer base
- `g_text1b_draw_p1_*` -- primitive params / texture base
- `g_text1b_geom_*` -- bigger geometry tables
- `g_text1b_static_*` -- static draw-list data

Strided variants encode their stride in the name
(`g_text1b_draw_p0_b610_stride12` = 0xC bytes per record, indexed by
arg0).  Use `git log` or `git grep` on the address suffix to find the
specific caller.

## 22. IRQ handler entry alabels in DispStuff.s (D_80083EDC + D_80083F1C)

Two `alabel`s inside DispStuff.s (an asm-only function) used as
2nd-argument callbacks for `irq_EnableInterrupts()` (main.c:177/180):

| Symbol | Address | Call site condition |
|---|---|---|
| `g_irq_handler_entry_no_pri` | 0x80083EDC | `de == 0` branch (no pending priority dispatch) |
| `g_irq_handler_entry_with_pri` | 0x80083F1C | `de != 0` branch (priority dispatch active) |

Both are part of the IRQ-enable-after-critical-section idiom in the
event-system initialization paths.
