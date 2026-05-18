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

## 12. Sound data buffer pointer cluster (6 pointers + relocator)

A cluster of 6 pointers at `0x800EFB14..0x800EFB28`, all relocated
together by `func_80054FDC(a0)` (text1b.c:11363-11378) when the
underlying sound-data buffer is moved.  The function adds `a0`
(the relocation delta) to every pointer; the trailing 4 are gated
by non-zero check (NULL ptrs are not relocated).

| Symbol | Address | Role |
|---|---|---|
| `g_snd_data_buf_base`       | 0x800EFB14 | Buffer base / header struct ptr (set by `func_80054604`) |
| `g_snd_data_subblock_0_ptr` | 0x800EFB18 | base + *(base+0x0C) — first subblock |
| `g_snd_data_subblock_1_ptr` | 0x800EFB1C | base + *(base+0x10) — second subblock (NULL if absent) |
| `g_snd_data_subblock_2_ptr` | 0x800EFB20 | base + *(base+0x14) — third subblock (NULL if absent) |
| `g_snd_data_subblock_3_ptr` | 0x800EFB24 | fourth subblock (NULL if absent) |
| `g_snd_data_subblock_4_ptr` | 0x800EFB28 | fifth subblock (NULL if absent) |

**Trace evidence (writers other than the relocator):**
- `func_80054604` (asm-only) sets `g_snd_data_buf_base` from either
  `func_80044FA0(...)` arg or `snd_LoadSelection(...)` return value.
  Path: `if (s2 != 0) { func_80044FA0(s0, s2); D_800EFB14 = s2; }
  else { func_80045080(s0); snd_StopBgm(); snd_LoadSelection(s0);
  D_800EFB14 = ret; }`.  So `g_snd_data_buf_base` always holds a
  pointer to a freshly loaded sound-data buffer or its returned size.
- `func_8005490C` (asm-only) reads `g_snd_data_buf_base`, then
  populates `g_snd_data_subblock_0_ptr` / `_1_ptr` / `_2_ptr` from
  header offsets at `*(base + 0xC/0x10/0x14)` added to the base.

`func_8005507C` returns `&g_snd_data_header_FB0C` (0x800EFB0C), which
is 8 bytes earlier than `g_snd_data_buf_base` — it's the start of a
small header struct that includes the buffer base pointer.

This cluster ties into the saTan family sound system; the relocator
allows the sound buffer to be moved (e.g., during BGM loading) without
invalidating all the cached subblock pointers.

## 13/14. SPU voice0E setup cluster (D_800EF070..0x800EF59C)

Tracing the writers of the voice-init constants (sound.c:780-826 +
asm-only func_800477E8 alt-init path + func_80047A90 synth-table
generator + func_80047BE0 scratchpad-DMA loop) revealed a complete
SPU voice setup cluster, spanning ~0x52C bytes from base.

### Voice state struct base — `g_snd_voice_init_block` (0x800EF070)

The struct holds setup parameters for SPU voice ID 0xE.  Fields known
so far (set at sound.c:783-799 and also at func_800477E8.s:110-138):

| Offset | Symbol | Type | Init value | Role |
|---|---|---|---|---|
| +0x00 | `g_snd_voice_init_block` | u8 | 0xE | voice ID |
| +0x01 | `g_snd_voice_init_byte_1` | u8 | 0 | sub-byte |
| +0x06 | `g_snd_voice_init_byte_6` | s16 | 0 | s16 field |
| +0x08 | `g_snd_voice_init_byte_8` | s16 | 0 | s16 field |
| +0x0A | `g_snd_voice_init_byte_A` | s16 | 4 | priority |
| +0x0C | `g_snd_voice_init_byte_C` | s32 | 0 | s32 field |
| +0x10 | `g_snd_voice_init_byte_C_plus_4..8` | s16 × 3 | 0 | 3 × s16 |
| +0x4C | `g_snd_voice_init_vol_baseline` | s32 | -0x2EE0 | initial volume baseline (-12000) |
| +0x50 | `g_snd_voice_init_field_at_50` | s32 | 0 | zero-init |
| +0x54 | `g_snd_voice_init_pitch_baseline` | s32 | -0xFA0 | initial pitch baseline (-4000) |

After the field assignments, sound.c:798 calls `func_800417D0(a0p)`
passing the struct pointer — the registered "init the SPU voice with
these parameters" call.

### Envelope-source data blocks (scratchpad-DMA targets)

Two 72-byte (0x48) data blocks copied to PS1 scratchpad each frame for
fast SPU access:

| Symbol | Address | Scratchpad target | Iteration |
|---|---|---|---|
| `g_snd_voice_envelope_block_a` | 0x800EF0D8 | 0x1F800020 + 0x1F800110 | even (i & 1 == 0) |
| `g_snd_voice_envelope_block_b` | 0x800EF168 | 0x1F800068 + 0x1F800134 | odd  (i & 1 != 0) |

Copied by `func_80047BE0` (asm-only) via a per-frame loop.  Source data
comes from synth tables below.

### Synth-table generation — `g_snd_wave_phase_table` + `g_snd_wave_output_table`

Initialized by sound.c:801-816 (filling) and then `func_80047A90`
processes them every call:

| Symbol | Address | Size | Role |
|---|---|---|---|
| `g_snd_wave_phase_table`  | 0x800EF558 | 17 × s32 (68 bytes) | Phase indices, stepped by 0x12 per call, used as `&0xFFF` index into `Judge` (sin table at 0x800973FC) |
| `g_snd_wave_output_table` | 0x800EF59C | 9 channels × 17 s32 (612 bytes, stride 0x44) | Wave samples computed via `(Judge[phase & 0xFFF] * 0x271) >> 10` |

`func_80047A90`:
1. For each of 17 phase indices, look up `Judge[phase & 0xFFF]` and
   compute `(sample * 0x271) >> 10` → write to channel 0 of
   `g_snd_wave_output_table`.  Step each phase by 0x12.
2. For channels 1-8, compute volume envelope from channel-0 baseline:
   `delta = (channel_n_sample - channel_n-1_sample); curve_step =
   (0x7D0 - delta) / 10` (or `>> 4` if negative).
3. **Special**: the LAST channel's curve gets written to
   `g_snd_fade_curve` (already named) — so the fade envelope IS the
   8th-channel ramp from the synth wavetable.

This connects the synthesis tables to the fade-curve subsystem
documented in sound.md.

### Implications

- The voice0E setup is a complete SPU "instrument" definition that
  the engine uses for one specific audio class (likely UI sounds or a
  global SFX bus, since it's a single voice initialized once at boot).
- The "envelope blocks A/B" double-buffer pattern is for **double-
  buffered scratchpad DMA** — while the SPU consumes one block, the
  CPU updates the other.
- `g_snd_fade_curve` is not a hand-tuned table — it's the 8th
  generated channel of the wave-synth output, so changes to
  voice0E parameters propagate to the fade behavior.

## 15. Win-animation sound trigger cluster (D_8008EAC0..0x8008EB28)

Per-frame scripted event handler in code6cac_c2.c:870-899.  Drives the
WIN/LOSE animation sequence after a fight ends — `katinuki_game_setData_…`
("katinuki" = Japanese "katte iku" = "win out") commits the result and
transitions back to the title menu.  Frame counter is `g_practice_loop_frame`
(D_800A37B8); winner character index is `D_800A3748` (called `p` in the
C body).

All frame constants are absolute frame counts within the win-animation
playback (post-fight); compared against `g_practice_loop_frame == D_x`
to fire a single SFX or particle effect at that exact frame.

### Per-stage intro frame (D_8008EAC0)

`g_winanim_per_stage_intro_frame[34]` — one frame target per stage,
fires SFX `40*p + 0x2D` (the per-character "intro" SFX class).

| Stage range | Value | Meaning |
|---|---|---|
| Most stages | 0x82 (130f) | Intro fires at ~2.17s |
| Stage 6 | 0x87 (135f) | Slight delay |
| Stages 7, 10 | 0xE6 (230f) | Long delay (3.83s) — likely cinematic stages |
| Several stages | 0xFFFF | Disabled (no intro SFX for this stage) |

### Per-character single-event frame triggers

| Symbol | Address | Frame | What it does |
|---|---|---|---|
| `g_winanim_callout_a_frame` | 0x8008EB04 | 0x9B (155f) | SFX `40*p + 0x31` — per-character callout A |
| `g_winanim_callout_b_frame` | 0x8008EB06 | 0x9F (159f) | SFX `40*p + 0x36` — per-character callout B |
| `g_winanim_special_frame`   | 0x8008EB08 | 0xA0 (160f) | SFX 0x53 (if p==0) or 0x2B (otherwise) — player-0 special |
| `g_winanim_fanfare_frame`   | 0x8008EB0A | 0xC6 (198f) | SFX 0x71 — closing fanfare |
| `g_winanim_particle_frame`  | 0x8008EB0C | 0x9F (159f) | Spawns particle at offset (see below) |

### Particle XYZ offset (added to D_800A3818 base)

When `g_winanim_particle_frame` fires, computes `vp = D_800A3818;
vp += (offset_x, offset_y, offset_z); func_800618B4(vp, &D_800A312C)`
to spawn the win-celebration particle effect:

| Symbol | Address | Value |
|---|---|---|
| `g_winanim_particle_offset_x` | 0x8008EB10 | 0 |
| `g_winanim_particle_offset_y` | 0x8008EB14 | -0x320 (-800) — upward (Y is downward in PSX/BB2 coord system, so −Y = up) |
| `g_winanim_particle_offset_z` | 0x8008EB18 | 0 |

### Sub-event tables (D_8008EB1C / D_8008EB28)

- `g_winanim_event_subtable_eb1c` (12 bytes) — indexed by
  `D_800A384C * 2` to fetch per-sub-event timing or type
- D_8008EB28 (16 bytes, unnamed) — frame-index sequence pattern
  `00 01 02 03 04 05 06 07 08 09 08 09 2E 2F 2E 2F` — looks like
  an animation-frame playback order with a stationary loop at the end

### Implications

- Total win-animation length is ~200 frames (~3.3 seconds at 60fps)
- Per-character SFX banks are 40-entry (0x28) blocks starting at
  SFX 0x2D, 0x55, 0x7D, … — character N's callouts live at
  `0x2D + 40*N` through `0x2D + 40*(N+1) − 1`.
- Stages 7/10 have longer intro delay (230f vs 130f) suggesting
  cinematic camera intros that take longer to settle before the
  winner's callout is appropriate.

## 16. Walk-direction packed cos/sin table (D_8009C928, 16384 bytes)

A 4096-entry packed trig LUT used by `motutil_GetWalkDir`
(display.c:2565, hand-coded asm in the original PSY-Q source) to
build a 3×3 Euler rotation matrix from XYZ angle components.

### Bit layout per entry (verified from .data values)

| Bits | Field | Notes |
|---|---|---|
| 31..16 | `cos(angle)` (s16, unity = 0x1000) | high halfword, sign-extended via `sra $t0, $t9, 16` |
| 15..0  | `sin(angle)` (s16, unity = 0x1000) | low halfword, sign-extended via `sll/sra 16` |

Entry 0 at idx 0 (angle = 0°): `0x10000000` → cos=0x1000 (unity), sin=0. ✓
Entry 1 at idx 1 (angle = 360°/4096 ≈ 0.088°): `0x10000006` → cos≈unity, sin=6. ✓

| Symbol | Address | Geometry |
|---|---|---|
| `g_trig_sin_cos_table_packed` | 0x8009C928 | 4096 × 4 bytes = 16384 bytes; range 0x8009C928..0x800A0928 |

### Indexing convention (motutil_GetWalkDir)

```
angle = lh 0(a0)              // s16 input
if (angle < 0) {
    angle = -angle;
    idx = angle & 0xFFF        // 12-bit angle (4096 ticks/rev)
    entry = D_8009C928[idx]
    sin_out = -(entry & 0xFFFF)  // negate sin for negative angle
    cos_out =  (entry >> 16)     // cos unchanged (cos(-x) = cos(x))
} else {
    idx = angle & 0xFFF
    entry = D_8009C928[idx]
    sin_out = (s16)(entry & 0xFFFF)
    cos_out = entry >> 16
}
```

The negative-angle branch uses sin(-x) = -sin(x), cos(-x) = cos(x)
identities — saving 2048 table entries vs storing both signs.

### Use case

`motutil_GetWalkDir(s16 *euler_xyz, u8 *mat3x3_out)`:
1. Loads X angle from `a0+0`, builds first row's (cos_x, sin_x)
2. Loads Y angle from `a0+2`, builds second row's (cos_y, sin_y)
3. Loads Z angle from `a0+4`, builds third row's (cos_z, sin_z)
4. Computes 9 matrix entries via `multu` of pairwise (cos/sin)
   products, writes halfwords to `mat3x3_out` at +4, +10, +16, etc.
5. Result is a Tait-Bryan ZYX rotation matrix for character walk
   direction or animation orientation.

Callers in text1b.c:
- text1b.c:13882 — `motutil_GetWalkDir(sp18, sp30)` (stack-local)
- text1b.c:13936 — same pattern
- text1b.c:14007 — `motutil_GetWalkDir(dest, s0)`

### Distinct from `Judge`

`Judge` (0x800973FC) is the canonical sin-only PSX trig table
(4096 × s16 = 8192 bytes), used for fade curves and SPU envelope
generation (see §13/14).

`g_trig_sin_cos_table_packed` (0x8009C928) is the **paired** form —
both sin and cos in one 32-bit load — used specifically by walk-
direction matrix builders where you need both trig components
simultaneously and don't want the 2 loads + 90° offset arithmetic.

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

## 18. Camera view-state — PsyQ MATRIX struct (D_800FF558, 32 bytes)

Camera/view state at `0x800FF558` is a standard PsyQ **`MATRIX`** struct
(per `include/gte.h`):

```c
typedef struct MATRIX {
    s16 m[3][3];   // +0x00..+0x11 (18 bytes) — 3x3 rotation matrix
    u16 pad;       // +0x12        (2 bytes)  — padding to align t[]
    s32 t[3];      // +0x14..+0x1F (12 bytes) — translation TRX/TRY/TRZ
} MATRIX;          // total: 32 bytes
```

Layout of `g_camera_view_state`:

| Offset | Field | Type | Role |
|---|---|---|---|
| +0x00..+0x11 | `m[3][3]` | 9 × s16 | 3×3 rotation matrix (R11/R12/R13/R21/R22/R23/R31/R32/R33) |
| +0x12..+0x13 | `pad` | u16 | alignment padding (unused) |
| +0x14..+0x17 | `t[0]` (TRX) | s32 | camera X position |
| +0x18..+0x1B | `t[1]` (TRY) | s32 | camera Y position |
| +0x1C..+0x1F | `t[2]` (TRZ) | s32 | camera Z position |

(Bytes +0x20..+0x27 between this struct and `g_voice_packet_base_0` at
0x800FF580 appear to be 8 bytes of unused padding or a small unknown
extension.)

### Builder — `func_80048BA4` (asm-only, asmfix-bridged)

Called from `code6cac.c:1773` as `func_80048BA4(D_800F5344, a1, flags)`
where `D_800F5344 = g_code6cac_state_F5344` holds 0x800 (the standard PSX
half-FOV scale).  Writes:
- 9 halfwords at +0x00..+0x10 (the full 3×3 rotation matrix)
- s32 at +0x18 (TRY) and +0x1C (TRZ)

TRX (+0x14) is set elsewhere — likely in a sibling helper or before this
function is called.

### Consumer — `func_80052930` (text1b.c:10798)

The GTE matrix×vector transformation routine:
- Loads `mat[0..4]` (5 s32 = 9 halfwords) into GTE control registers
  $0..$4 (R11/R12, R13/R21, R22/R23, R31/R32, R33+pad)
- Zeros GTE control $5/$6/$7 (TRX/TRY/TRZ) — so translation is ignored
- Loads `vec[0..4]` into GTE data $0, $1 (vertex XYZ)
- Executes `cop2 0x4A480012` — MVMVA: Multiply Vector by Matrix and Add
  zeroed translation Vector
- Reads result from $9/$10/$11 (IR1/IR2/IR3) → writes `out[0..2]`

So `func_80052930` applies ONLY the rotation portion of the matrix to a
vector — it's a pure rotation, not a full transformation.  Callers pass
`&g_camera_view_state` as the matrix, a vertex from another struct as
the vector, and write the rotated result back into the per-actor data.

### Used by

Inline-asm callers in text1b.c (1896, 1920, 2364, 2474, 2499, 5495,
11270) and asm-only `calc_loc_mat_fw_8004A940`.  This is the camera's
**view rotation** applied to per-actor vertex coordinates — distinct
from `g_cam_matrix` (`0x800EEDB0`, the world/projection matrix).

### Position-delta threshold check (text1b.c:2359-2435)

A guard pattern in text1b's draw path:
```
v0 = &g_camera_view_state
t3..t5 = read TRX/TRY/TRZ (camera pos)
delta = abs(player_pos - camera_pos) per component, OR'd
if delta < 18944 (0x4A00):       LOD level A
else if delta < 42240 (0xA500):  LOD level B
else:                             LOD level C
```
The thresholds 0x4A00 / 0xA500 are LOD-cutoff distances measured in
world units.

## 19. HUD sprite size packed lookup (D_8009B850)

Per-sprite dimension table consumed by `func_80060414(s16 arg0,
s32 arg1, s32 arg2)` (text1b.c:12963), the HUD-sprite draw helper.

### Encoding

```c
u16 entry = *((&D_8009B850) + (arg0 & 0x7FFF));
s.width  = (entry >> 7) + 0x37;          // 9-bit high field -> width 55..566
s.height = (entry & 0x7F) + 0x2A;        // 7-bit low field  -> height 42..169
```

| Symbol | Address | Encoding |
|---|---|---|
| `g_text1b_sprite_size_packed_lookup` | 0x8009B850 | u16 per entry; high 9 bits = (width - 0x37), low 7 bits = (height - 0x2A) |

Bias constants `+0x37` (width) and `+0x2A` (height) save bits by
storing only the delta from minimum dimensions.

### arg0 bit semantics

| Bits | Meaning |
|---|---|
| bit 15 (0x8000) | "Highlight/match" flag - selects alt geometry `D_8009B7AC` |
| bits 14..0 (0x7FFF) | Sprite size table index |

When the highlight bit is clear, geometry is `D_8009B7B8` if
`g_disp_config < 0xC`, else `D_8009B7C4` -- i.e., PAL/NTSC or
video-mode-aware geometry selection.

### Caller chain (HUD render)

`ings.c:759` (inside the per-frame draw path):
```c
s1_var = D_800A37A8[a0_val];          // per-character size index
if ((new_val & 0xFF) == a1_val) {
    s1_var |= 0x8000;                  // mark as "current/active"
}
func_80060414(s1_var, s2_var, ot_slot);
```

So this is a per-character HUD element (likely portrait or status
icon).  The 0x8000 flag highlights the "current/active" character
slot using a distinct geometry, while inactive slots use the
video-mode-appropriate dimmer geometry.

### Output struct `S414` (text1b.c:12950)

```c
typedef struct {
    s32 *p_geom;       // selected by highlight + disp_config
    s32 *p_static;     // always &D_800A328C
    s32 arg1_field;    // base draw-primitive pointer
    s32 pad0C, zero10;
    s32 arg2_field;    // OT slot
    s32 width, height; // from D_8009B850 lookup
    s32 pad20, pad24;
    s8  byte28;
} S414;
```

After fill, `func_8007352C(&s)` submits the primitive, then
`saMotionSet(s.p_geom)` + `ot_Link` add it to the ordering table.

## 20. Motion-ex pool B (12-slot effect-spawn pool, D_800F0E38..0x800F0BEC)

Allocated by `func_80062FEC` (text1b.c:14112) via the
`g_particle_slot_bitmap_plus_4` busy bitmap (0x800A3448, 12 bits used).
On allocation, writes XYZ from `src = (s32*)D_800A347C` into the slot,
no random perturbation (cf. pool A below, which does add spread).

| Symbol | Address | Stride/size | Role |
|---|---|---|---|
| `g_motion_ex_pool_b_xyz_x` | 0x800F0E38 | s32, stride 12 | XYZ X column (12 slots × 4 bytes) |
| `g_motion_ex_pool_b_xyz_y` | 0x800F0E3C | s32, stride 12 | Y column |
| `g_motion_ex_pool_b_xyz_z` | 0x800F0E40 | s32, stride 12 | Z column |
| `g_motion_ex_pool_b_flag`  | 0x800F0BEC | s16 × 12 | per-slot flag (cleared on allocate) |
| `g_particle_slot_bitmap_plus_4` | 0x800A3448 | u32 bitfield | busy bitmap (bits 0-11) |

### Pool A vs Pool B (two parallel effect-spawn pools)

| Bitmap | Slots | Data cluster | Allocator | Spread |
|---|---|---|---|---|
| `g_particle_slot_bitmap` (0x800A3444) | 32 | D_800F0D78 / 0x0D7C / `videoDec`(0x0D80) | text1b.c:14236 | Random ±0x7F per axis (via `func_80079154()`) |
| `g_particle_slot_bitmap_plus_4` (0x800A3448) | 12 | D_800F0E38 / 0xE3C / 0xE40 | text1b.c:14112 (`func_80062FEC`) | **None** — direct copy |

Pool A uses random per-axis offset for particles that spread (sparks,
debris, splatter).  Pool B uses unmodified position — likely for
**precision effects** like sword-impact glints, damage numbers, or
focus markers that need to spawn at exact world positions.

### Related motion-ex tables in the same address range

- `g_motion_ex_state_block_table_12` (0x800F0CA0, 8 entries × 12 bytes)
  — primary per-motion-id state (pos/extra/data_ptr)
- `g_motion_ex_extra_at_0/4/8` (0x800F0FB8..C0) — extra XYZ data
- `g_motion_ex_state_aux_at_0/2/4` (0x800F10A0..A4) — u16 aux state

This is the motion-ex subsystem's **effect-spawn substrate**: per-entity
state tables (block_table_12), extra/aux fields (extra_*, state_aux_*),
and two slot pools for transient spawned effects.

## 21. text1b 2D-draw helper geometry banks (0x8009B340..0x8009B850)

Bank of 20+ GPU draw-primitive geometry tables at `0x8009B340..0x8009B850`,
each consumed by a specific text1b 2D-sprite draw helper.  Helpers all
share the same `S414`-style draw-submission struct (see §19 for the layout)
with fields `s.p0` (geometry vertices), `s.p1` (texture/params), `s.p_geom`
(per-mode geometry), `s.p_static` (static draw-list data).

Each helper is a self-contained 2D-sprite renderer for a distinct UI/HUD
element.  Cross-reference of tables to consumers:

| Helper function | text1b.c lines | Tables used | Likely role |
|---|---|---|---|
| `func_8005D46C` | 12625-12640 | `g_text1b_draw_template_b2c8` (stride), `..._p1_b340`, `..._p1_b358` | Small 2-variant sprite (B2C8 base + alt at +0xC) |
| `gnd_land_hit_char_tsuba` | 12690-12715 | D_8009B2E0 (u8 base), `..._draw_data_b388`, `..._draw_p1_b390` | Sword-clash hit visualization (named function) |
| `func_8005FA98` | 12777-12797 | `..._draw_p0_b610_stride12`, `..._b63C_stride12`, `..._p1_b660`, `..._b670`, `..._b678`, `..._b634` | 4-variant sprite (4 p1 alts × 2 geometries) |
| `func_800600C8` | 12871-12906 | `..._draw_primitive_b6f0`, `..._p0_b6FC`, `..._p1_b708_stride8`, `..._p1_b758` | **Digit/counter** draw (stride-8 indexed by `s.d0[i]` — score or timer digits) |
| `func_80060414` | 12963-12988 | D_8009B7AC, `..._geom_b7B8`, `..._geom_b7C4`, `..._sprite_size_packed_lookup` | **HUD per-character sprite** (see §19) |
| `func_80060544` | 13044-13085 | `..._geom_b770_offset`, D_8009B7A0, `..._static_b7D0/b7D8/b800/b820/b840`, D_8009B3B0 | **Multi-mode menu draw** (5 static variants for 5 menu modes) |
| `func_80060768` | 13133 | D_8009B0C0 | Passes geometry to `func_8006D808` (sub-helper) |

### Naming conventions

| Prefix | Field bound | Role |
|---|---|---|
| `g_text1b_draw_p0_*` | `s.p0` | Geometry/vertex array base (often stride-12 indexed) |
| `g_text1b_draw_p1_*` | `s.p1` | Primitive params / texture pointer |
| `g_text1b_draw_data_*`, `..._template_*`, `..._primitive_*` | mixed | Catch-all for tables used in less standard slots |
| `g_text1b_geom_*` | `s.p_geom` | Per-display-mode geometry (PAL/NTSC variants) |
| `g_text1b_static_*` | `s.p_static` | Static draw-list data attached to OT |

Strided variants encode stride in name:
- `_stride8` (e.g. `b708_stride8`): 8 bytes per record (digit cells)
- `_stride12` (e.g. `b610_stride12`, `b63C_stride12`): 12 bytes per record (XYZ triplets or 4-corner UV)
- `_offset` (e.g. `b770_offset`): indexed by raw byte offset added to base

### Cluster summary

The whole 0x8009B340..0x8009B850 block is the **text1b 2D-UI render
substrate** — geometry/texture/static-data pools for 6+ distinct UI
helpers (sword-clash effect, digits, HUD portraits, menu screens, etc.).
Use `git grep g_text1b_draw_…_<suffix>` to find the specific consumer
when a unfamiliar table appears in new decomp work.

## 22. IRQ-callback trampolines (D_80083EDC + D_80083F1C)

Two `alabel`s inside DispStuff.s used as `irq_EnableInterrupts(de, ptr)`
callbacks from main.c:177/180.  Read line-by-line, they're alarm-
callback dispatch trampolines that consume the alarm-cluster state at
`D_800A26D0..D_800A26E0`.

### `g_irq_handler_entry_no_pri` (D_80083EDC) — "fire pending + secondary"

```
v0 = g_alarm_callback_ptr             // D_800A26D8
if (v0 != 0) jalr v0                  // call pending primary cb if set
v0 = g_alarm_secondary_cb_ptr         // D_800A26D4 (renamed from _plus_4)
jalr v0                                // ALWAYS call secondary
return
```

Used when the `de` arg is 0 (no priority pending) — fires both the
pending alarm callback (if armed) and the always-running secondary.

### `g_irq_handler_entry_with_pri` (D_80083F1C) — "one-shot deferred"

```
v0 = g_alarm_pending_priority_flag    // D_800A26E0 (newly named)
if (v0 != 0) {                         // already armed → fire now
    cb = g_alarm_secondary_cb_ptr
    g_alarm_pending_priority_flag = 0  // disarm
    jalr cb
} else {
    g_alarm_pending_priority_flag = 1  // arm for next entry, skip call
}
return
```

Used when `de != 0` (priority pending) — implements a **one-shot
deferred-fire** scheme using `D_800A26E0` as a single-bit edge-trigger:
first entry arms; second entry fires.  This is the classic "wait for
two ticks before invoking" pattern for IRQ-deferred work.

### Alarm callback cluster (D_800A26D0..D_800A26E0)

After this trace, the cluster names are:

| Symbol | Address | Role |
|---|---|---|
| `g_alarm_armed_flag` | 0x800A26D0 | armed boolean |
| `g_alarm_secondary_cb_ptr` | 0x800A26D4 | always-call secondary callback ptr (renamed from `_plus_4`) |
| `g_alarm_callback_ptr` | 0x800A26D8 | primary callback ptr (call if non-null) |
| `g_alarm_callback_pending` | 0x800A26DC | boolean: pending |
| `g_alarm_callback_pending_plus_1` | 0x800A26DD | (sibling) |
| `g_alarm_active_sentinel` | 0x800A26DE | active sentinel |
| `g_alarm_pending_priority_flag` | 0x800A26E0 | one-shot deferred-fire flag (added by §22 trace) |
