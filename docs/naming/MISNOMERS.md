# Known Misnomers

Names in `src/*.c` and `named_syms.txt` that have been identified as
demonstrably incorrect based on body analysis. Each entry lists:

* The current name (what the symbol is called today)
* Where the name is defined
* Evidence the name is wrong
* The proposed canonical name (often added as an additional alias in
  `named_syms.txt` to provide the correct name without disturbing
  src/*.c calls that use the old name)

Two categories:

1. **Audit-found misnomers** -- where body analysis demonstrates the
   current name is wrong. Aliases have been added to `named_syms.txt`.
2. **Kengo / naming-batch conflicts** -- where the decomp agent's
   Kengo-derived renames overlap with proposals from our hand-review
   batches and the interpretations disagree. Reviewer should pick
   one.

---

## Category 1: Audit-found misnomers (aliases added 2026-05-17)

### `pad_press_control` (src/code6cac_c.c:175) -- actually memcard init

Body opens 8 memory-card I/O events and calls `bios__bu_init_A0()`
(BU = "Backup Unit" = memory card). Has nothing to do with pad-press
handling.

```c
void pad_press_control(void) {
    func_8007A370(1);
    func_8007A3C8();
    bios__bu_init_A0();                         // memcard init
    bios_ChangeClearPad(0);
    EnterCriticalSection();
    D_800A37DC = bios_OpenEvent(0xF4000001, 4,      0x2000, 0);
    // ... 7 more bios_OpenEvent calls, 4 each in two classes ...
    ExitCriticalSection();
    bios_EnableEvent(D_800A37DC);
    // ... 7 more EnableEvent ...
}
```

* **Current name in src/**: `pad_press_control`
* **Proposed canonical**: `memcard_event_init` (added as alias
  `memcard_event_init_800375EC = 0x800375EC` in named_syms.txt)
* **Origin of misnomer**: likely a Kengo-derived rename that
  mistranslated -- the original Japanese may have referred to "pad
  card check" (memcard insertion vs pad pressed) and got conflated.

The 3 paired `memcard_event_*` waiters were already refined from
their original `game_event_*` slugs in commit `b6e72b8` based on the
same finding.

### `marionation_camera_Init_80036064` (src/code6cac_b2.c:380) -- actually a CD streaming callback

Body uses `cdrom_SetCallbackB(0)` + `cdrom_BcdToFrames` and reads
0x200 sectors at a time via `func_80080620`. It's a CD streaming
callback that loads NDATA-style sector chunks; not a camera-init
function.

* **Current name in src/**: `marionation_camera_Init_80036064`
* **Proposed canonical**: `cdrom_stream_callback`
* **Alias added**: `cdrom_stream_callback_80036064 = 0x80036064`
* **Discussed in**: [`docs/formats/STAGE_BIN.md`](../formats/STAGE_BIN.md)
  ("Naming caveat" section) and
  [`docs/formats/NDATA.md`](../formats/NDATA.md)

### `marionation_camera_Init_80037468` (src/code6cac_b2.c:651) -- actually a `bios_Exec` wrapper

Body sets up GPU (`gpu_SetDispMask + gpu_EnableDisplay`), interrupts
(`irq_DisableInterrupts + irq_Reset`), critical section, then calls
`bios_Exec(sp, a0, a1)` -- the PSX BIOS system-restart function.
Then re-initialises pad and reloads sound data after `bios_Exec`
returns.

```c
void marionation_camera_Init_80037468(s32 a0, s32 *a1, s32 a2) {
    s32 sp[16];
    sys_VSync(0);
    gpu_SetDispMask(0);
    gpu_EnableDisplay();
    get_point_value();
    func_80037774();                         // memcard_event_pool_close
    irq_DisableInterrupts();
    func_8007FF7C();
    special_camera_get_rot_dir(sp);          // misnomer? possibly
    gpu_DrawSync(0);
    gpu_SetMode(0);
    pad_Init();
    irq_Reset();
    sp[8] = a2;
    sp[9] = 0;
    EnterCriticalSection();
    bios_Exec(sp, a0, a1);                   // **system restart**
    sys_Init();
    file_LoadSoundData();
    sys_VSync(0);
    // ...
}
```

* **Current name in src/**: `marionation_camera_Init_80037468`
* **Proposed canonical**: `bios_exec_wrapper_with_setup`
* **Alias added**: `bios_exec_wrapper_with_setup_80037468 = 0x80037468`

### `special_camera_check_pos_outside_ground_80035DC8` -- actually scene teardown

Body destroys both players, resets DMA + obj state, swaps disp
config, sets a state code. Has nothing to do with position
checking.

```c
void special_camera_check_pos_outside_ground_80035DC8(void) {
    gpu_EnableDisplay();
    gpu_InitDisplay();
    func_80020CDC();
    player_Destroy(0);                       // teardown player 0
    player_Destroy(1);                       // teardown player 1
    file_ResetDmaFlag();
    obj_InitAll();
    func_80077820((s32)0x80118800);          // disp_load_config_from_buf
    D_800A3834 = 0x1B;
    gpu_DisableDisplay();
}
```

* **Current name in src/**: `special_camera_check_pos_outside_ground_80035DC8`
* **Proposed canonical**: `scene_teardown`
* **Alias added**: `scene_teardown_80035DC8 = 0x80035DC8`

### `special_camera_check_pos_outside_ground_80036E34` -- actually NDATA disc-read scheduler

Body is the per-call NDATA file scheduler. Called by `func_800451A0`
to DMA NDATA files into RAM via `cdrom_BcdToFrames` /
`cdrom_FramesToBcd`.

* **Current name in src/**: `special_camera_check_pos_outside_ground_80036E34`
* **Proposed canonical**: `ndata_disc_read_scheduler`
* **Alias added**: `ndata_disc_read_scheduler_80036E34 = 0x80036E34`
* **Discussed in**: [`docs/formats/NDATA.md`](../formats/NDATA.md)
  "Reader (`func_80044E74`)" section.

---

## Category 2: Naming-batch conflicts with Kengo applies

The decomp agent ran a batch of Kengo-derived renames on
2026-05-17 that landed in `named_syms.txt` after our batch-4
hand-review proposals were committed. Several addresses now have
**two different proposed names** -- the Kengo one (applied to
`named_syms.txt`) and our hand-review one (in `docs/naming/evidence/`
+ `docs/naming/proposals.csv`).

Reviewer should examine the evidence for each and pick one. The
Kengo names are mostly `size-only-ambiguous` matches per the
methodology doc, which means their reliability is mixed.

| Address | Kengo (applied) | Our proposal (evidence file) |
|---------|------------------|------------------------------|
| `0x80021280` | `title_mv_exec` | `char_motion_nibble_scan_80021280` |
| `0x80030900` | `myRobGeneiDraw` | `spawn_hit_particle_80030900` |
| `0x80033498` | `ki_attack_ougi` | `get_stage_idx_80033498` |
| `0x80034200` | `_GetBattleSwichData` | `pack_player_input_bitstream_80034200` |
| `0x80080258` | `_DispCharacterName` | `tslTm2_command_with_retry_80080258` |
| `0x80080390` | `myRobGeneiDraw3` | `tslTm2_command_with_retry_no_arg3_80080390` |
| `0x80086130` | `action_check_defense` | `char_store_pos_pair_80086130` |

**Analysis hint per conflict:**

* `0x80021280`: Kengo's `title_mv_exec` (= title-screen MV exec?)
  conflicts with our `char_motion_nibble_scan` (parses motion stage
  nibbles from per-char struct). The body is in `code6cac.c` (motion
  cluster, NOT title-screen). Our reading is better-supported by
  context.
* `0x80030900`: Kengo's `myRobGeneiDraw` (= "my robot afterimage
  draw") and our `spawn_hit_particle` could BOTH be right -- the
  function uses `coli_hit_body_weapon` and `rng_Next` to spawn
  randomised effect particles. "Afterimage" is a specific kind of
  spawned particle; the implementations are compatible.
* `0x80033498`: Kengo's `ki_attack_ougi` (= "spirit attack secret
  move identifier") and our `get_stage_idx` -- the body is a pure
  switch on `D_800A36A4 - 2` returning indices 0..5. Stage IDs are
  more likely than ougi-attack codes (the global is named
  `g_stage_id`-adjacent in the agent's new `g_*` additions).
* `0x80034200`: Kengo's `_GetBattleSwichData` (typo for SwitchData?)
  vs our `pack_player_input_bitstream` -- the body packs N x 2 byte
  values into a u32. "Switch data" might fit if it's compressed
  battle-mode flags.
* `0x80080258`: Kengo's `_DispCharacterName` vs our
  `tslTm2_command_with_retry` -- the body is a 3-retry wrapper
  around `tslTm2LoadImage`. Kengo's name is implausible (this is
  clearly a CD/TIM load wrapper, not a name display).
* `0x80080390`: same pattern; Kengo's `myRobGeneiDraw3` is also
  implausible for what is clearly a 2-arg variant of the
  `tslTm2_command_with_retry` retry wrapper.
* `0x80086130`: Kengo's `action_check_defense` vs our
  `char_store_pos_pair`. The body validates `a0 < 0x18` and stores
  paired values into 16-byte slot records. "Defense check" doesn't
  fit; our reading fits.

**Recommended action**: a reviewer pass that compares each Kengo name
against the body's actual behaviour. Where the Kengo name is
size-only-ambiguous and our hand-review reading is strongly
supported (4 of 7 cases above), the Kengo name should be demoted
(commented out or moved to an "anti-aliases" section) and our name
applied. Where they're compatible (case 2: afterimage particle), no
action needed -- both names can co-exist at the same address.

---

## Future audit candidates

Names worth investigating in subsequent passes:

* ~~All `katinuki_game_get_katinuki_max_num_*` (5 functions)~~
  **Done 2026-05-17** -- see "Audit verdicts" below.
* ~~The `tslSmdSendVu1Code_*` family~~
  **Done 2026-05-17** -- see "Audit verdicts" below.
* All `_helper_<addr>` suffixed names -- these were proposed by the
  `sole_caller_path` heuristic and should be verified against the
  caller's actual usage. Currently none are applied to
  `named_syms.txt` (they live in `proposals.csv` only), so they're
  evidence-only suspect rather than active misnomers. Lower
  priority for audit.

---

## Audit verdicts (2026-05-17 deepen-pass)

### Category 1 audit-found misnomers (5 functions)

All 5 had replacement aliases added in named_syms.txt commit `b6e72b8`.
See the per-function entries above (`pad_press_control` →
`memcard_event_init`, etc.).

### Category 2 Kengo conflict verdicts (named_syms.txt commit `8ef467f`)

Per-conflict resolution after body audit:

| Address | Kengo | Hand-review | Verdict |
|---------|-------|-------------|---------|
| `0x80021280` | `title_mv_exec` | `char_motion_nibble_scan` | **Hand-review wins** -- demoted Kengo, added hand-review alias |
| `0x80030900` | `myRobGeneiDraw` | `spawn_hit_particle` | **Compatible** -- both retained as aliases |
| `0x80033498` | `ki_attack_ougi` | `get_stage_idx` | **Ambiguous** -- kept Kengo; needs caller-side trace |
| `0x80034200` | `_GetBattleSwichData` | `pack_player_input_bitstream` | **Ambiguous** -- kept Kengo; may be same thing |
| `0x80080258` | `_DispCharacterName` | `tslTm2_command_with_retry` | **Hand-review wins** -- demoted Kengo, added hand-review alias |
| `0x80080390` | `myRobGeneiDraw3` | `tslTm2_command_with_retry_no_arg3` | **Hand-review wins** -- demoted Kengo, added hand-review alias |
| `0x80086130` | `action_check_defense` | `char_store_pos_pair` | **Hand-review wins** -- demoted Kengo, added hand-review alias |

### Broader sweep verdicts (named_syms.txt commit `8ef467f`)

7 Kengo aliases demoted with `/* MISNAMED: ... */` inline comments
documenting the body's actual behaviour. 5 replacement aliases
added in a new "Misnomer-replacement aliases" section:

| Wrong Kengo name | Address | Body actually is | Replacement alias |
|------------------|--------:|------------------|-------------------|
| `katinuki_game_get_katinuki_max_num_80016868` | `0x80016868` | `gpu_SetMode(1)` wrapper | `gpu_EnableDisplay` (already in undefined_syms_auto.txt) |
| `katinuki_game_get_katinuki_max_num_800168D0` | `0x800168D0` | `gpu_SetDispMask(1)` wrapper | `gpu_DisableDisplay` (already in undefined_syms_auto.txt) |
| `katinuki_game_get_katinuki_max_num_80046914` | `0x80046914` | `func_800453E0(8)` wrapper | `channel_helper_8_80046914` |
| `katinuki_game_get_katinuki_max_num_80046934` | `0x80046934` | `func_800455AC(9)` wrapper | `channel_helper_9_80046934` |
| `katinuki_game_get_katinuki_max_num_80046A60` | `0x80046A60` | `func_800453E0(0xA)` wrapper | `channel_helper_A_80046A60` |
| `tslSmdSendVu1Code_8007A87C` | `0x8007A87C` | 5-insn: `*a0 & 0xFFFFFF \| 0x80000000` (rel-to-KSEG0) | `ptr_load_kseg0_8007A87C` |
| `tslSmdSendVu1Code_8007A898` | `0x8007A898` | 5-insn: `(*a0 & 0xFFFFFF) == 0xFFFFFF` (OT-sentinel check) | `is_ot_terminator_8007A898` |

### Summary of named_syms.txt state after audit

* 7 entries marked `/* MISNAMED: ... */` (kept as back-compat aliases).
* 9 new replacement aliases added (5 misnomer fixes + 4 Kengo conflict
  resolutions).
* 1 compatible-with-Kengo alias added (`spawn_hit_particle_80030900`
  alongside `myRobGeneiDraw`).
* 2 Kengo aliases left untouched pending further investigation
  (`ki_attack_ougi`, `_GetBattleSwichData`).

The named_syms.txt is now substantially more accurate than before
this audit pass.

## Late finding (2026-05-17): `g_file_disc_size` at 0x80106A50

Discovered during the engine-doc rewrite pass.  The address
`0x80106A50` is mapped to `g_file_disc_size` in `named_syms.txt:152`
(pre-existing name; not from this session's work).

**The name is wrong.**  Actual usage in `src/code6cac_b.c:3524`:

```c
s32 bits = D_80106A50 & mask;
if (D_80106A50 & 0x10020) ...
```

And in `docs/engine/ai.md`:

> "No action" = AI is between moves. Builds a shuffled list of which
> moves are currently legal (from a mask in `D_80106A50` against
> waza-enable bits in `D_8008D538[stance]`).  Walks bits 0..26 of
> `D_80106A50` masked against the stance's enable bitmap; for each
> enabled bit, appends the move ID to a buffer ...

This is a **27-bit move-enable bitmap**, not a disc size.

**Suggested replacement name:** `g_move_enable_bitmap`.

**Why not renamed yet:** the existing name `g_file_disc_size` is
referenced from `src/ings.c`, `src/code6cac_b.c`, `src/code6cac_c_ab.c`,
`src/code6cac_c_mid.c` (as `extern u32` and `extern s32`).  A rename
requires updating these `extern` decls in `src/*.c` files, which
carries SHA1-mismatch risk if any of the using functions get
recompiled differently due to the type change.  The rename is a good
follow-up but deferred to the decomp-agent's normal flow.

For reference until the rename happens: `docs/engine/recent_naming_findings.md`
uses `D_80106A50` directly with the "move-enable bitmap" interpretation
called out inline.

## Audit pass 2 (2026-05-17) -- naming-pattern verification

Ran a heuristic audit (`tools/...` -- in `/c/tmp/bb2_misnomer_audit.py`)
against all 614 semantic names in `named_syms.txt`.  The audit checks
for mismatches between the name's implied role and the actual usage:

- `_counter` / `_count` / `_ticks` -- should be incremented/decremented
- `_timer` / `_countdown` / `_deadline` -- should be decremented
- `_ptr` / `_addr` -- should be deref'd somewhere
- `_mask` / `_bitmask` / `_bits` -- should be bitwise-operated

The audit scanned both `src/*.c` and `asm/funcs/*.s` (asm coverage is
necessary because many counters/timers are modified only in still-
asm functions).

**Results: 22 flagged, mostly false positives.** Verified-true issues:

### True misnomer 1: `g_file_io_state_event_counter_17` (0x800A3338)

I named these "counter" cells in the rodata cluster-consumers pass.
Verification: they're 17 x 4-byte zero-init cells consumed by
`file_io_state_dispatch_80038988` -- the consumer RESETS them (sets
to 0 conditionally) but never increments.  These are state-machine
**event cells** (booleans / state codes), not counters.

**Renamed in this commit** to `g_file_io_state_event_cells_17`.

### True misnomer 2: duplicate name at 0x800A3790

`g_game_timer` (pre-existing, line 130) and `g_round_timer`
(line 468, added 2026-05-17 batch 5) both alias the same address.
Both names describe the same role (the per-round/per-game frame
countdown reset by `game_ResetTimer_80046E8C` to 0x23 = 35 frames).
The linker accepts duplicate names at the same address; this is
benign cosmetic clutter rather than a functional bug.

**Action: left both names.** They're consistent with the role and
having two synonyms doesn't hurt searchability.  A future cleanup
pass could drop the older `g_game_timer` alias since `g_round_timer`
has the more accurate body-derived semantic.

### False positives (20)

Most of the audit's flags were false positives due to:
- "counter" names where the increment is in an asm function and the
  pattern crosses two lines (`addiu` then `sh`/`sw`), which the
  line-based audit doesn't detect.  Example: `g_pad_analog_frame_counter`
  is incremented in `pad_FuncAnalog.s` via:
  ```
  lhu  $v0, %gp_rel(D_800A3814)($gp)
  addiu $v0, $v0, 0x1
  sh   $v0, %gp_rel(D_800A3814)($gp)
  ```
- "ptr" names that are passed by address (e.g., as function arg) but
  not deref'd in C source -- the C-source heuristic misses this.
- `g_root_counter_*` -- the "counter" in the name refers to the PSX
  root-counter subsystem (hardware timer), not the variable being a
  counter.
- `g_bitmask_table_*` -- table of mask VALUES, applied with bitwise
  ops at the consumer, not in the table itself.

The audit is a useful smell-test but its line-based heuristics need
manual triage.  Worth re-running periodically as the symbol table
grows.

---

## Pass 3: Cluster-consumer cross-check (2026-05-17)

While naming the pad-recording cluster at 0x800A36C0-36D4 (commit b230220
neighborhood), discovered an overlap with an existing named symbol.

### `g_memcard_file_write_buf` (0x800A3698) -- actually pad-recording buf

The address `0x800A3698` was named `g_memcard_file_write_buf` because
it gets passed to `bios_FileWrite_B`.  However, the primary use is as
the **current-frame packed pad-record buffer** in
`func_8003A728` (code6cac_c_mid.c:1714):

```c
D_800A3698 = packed;          // store packed pad input
// ... later in same function ...
D_800A36D0 = D_800A3698;      // copy to "previous" slot at end of frame
```

The memcard `bios_FileWrite_B` call reuses the address as a generic
scratch buffer between pad-record updates; it isn't memcard-owned
storage.

**Action: kept the existing name.** Renaming would touch the existing
`g_memcard_file_write_buf` reference in src/code6cac.c, and the role
genuinely is "general-purpose scratch for both memcard and pad-rec".
A future cleanup could rename to `g_scratch_buf_3698` and document
both uses, but that's a higher-touch change with SHA1 risk that's
not worth it for a single name.

Filing this as a soft misnomer (don't act, but document so future
analysts don't get confused).

### `conv_matrix_rotation` (src/ings2.c:372) -- actually CD-ROM IRQ init

The function body has NOTHING to do with matrix rotation conversion:

```c
s32 conv_matrix_rotation(void) {
    sys_MemClear2((s32 *)&D_800A2640, 8);       // clear g_irq_cdrom_callbacks
    *(volatile s32 *)D_800A263C = 0;            // clear CD-ROM IRQ control reg
    ((void (*)(s32, void *))irq_EnableInterrupts)(3, (void *)D_80083418);
                                                // hook IRQ 3 to irq_cdrom_handler
    return (s32)D_8008359C;                     // return registrar
}
```

It's the **CD-ROM IRQ initialization** function (a parallel to `func_800832A0`
which is `vsync_Init`).  The "matrix rotation" name is completely wrong
and may have come from misclassification during early naming.

**Action: left existing name.** Renaming would touch sibling files.
Proposed canonical name: `irq_cdrom_init` (parallel to `vsync_Init`).


## Pass 4: Cluster-trace findings (2026-05-17 end-of-session)

After the placeholder-refinement addendum cluster traces
(see `docs/engine/recent_naming_findings.md` §11-22), several
existing names were found to misrepresent their actual role.

### Fixed in trace commits

#### `g_alarm_armed_flag_plus_4` (0x800A26D4) — was a fn ptr, not a flag

The `_plus_4` placeholder suggested a sibling of `g_alarm_armed_flag`
(0x800A26D0, an actual boolean).  Reading DispStuff.s lines 18-58
(§22 trace) revealed it's actually a **function pointer** invoked
via `jalr` from both IRQ-callback trampolines.

**Action: renamed to `g_alarm_secondary_cb_ptr`** in the trace commit.

**Lesson:** When a `_plus_<N>` sibling is invoked with `jalr` rather
than tested with `beqz`, it's a function pointer, not a flag — needs
a semantic rename promotion.

### Identified but not corrected (informational misnomers)

#### `g_snd_fade_curve` (0x800F0xxx via `g_snd_fade_curve` alias) — generated, not hand-tuned

The name implies a hand-tuned table.  §13/14 trace revealed it's
actually the **8th channel of the synth wavetable** generated by
`func_80047A90` from `g_snd_wave_phase_table` + `g_snd_wave_output_table`.
Changes to `g_snd_voice_init_block` (vol/pitch baselines) propagate
to the fade curve through the synth pipeline.

**Action: not renamed.** `g_snd_fade_curve` IS the fade curve
semantically (it's used as one) — just generated rather than static.
The trace doc (recent_naming_findings.md §13/14) calls this out so
future readers know it's not safe to edit independently of the synth
table parameters.

#### `g_disp_state_buf` (0x800F33D8) — also the memcard save/load payload

The name correctly describes its primary role (display-state struct,
512 bytes, cleared by `func_80038148`).  But §17 trace shows it's
also the **memcard write payload** (passed to `func_80037C34` and
`func_80037B90` with len 0x200).

**Action: not renamed.** The dual role is documented in §17 + the
file_io.md cross-ref.  Future memcard work should be aware that
edits to this buffer affect both the display path AND the saved
data layout.

#### `g_camera_view_state` (0x800FF558) — should be typed as `MATRIX`

The name is fine but the type isn't formalized.  §18 trace confirmed
it's a standard PsyQ `MATRIX` struct (per `include/gte.h`): `s16 m[3][3];
u16 pad; s32 t[3];` = 32 bytes.

**Action: not renamed; trace documents the type.** A future cleanup
could add an `extern MATRIX g_camera_view_state;` declaration in
relevant `.c` files instead of accessing it via `(s32 *)&D_800FF558`,
but that's a typed-extern refactor, not a naming change.

### Bonus discoveries (not misnomers but worth recording)

#### `g_seq_event_handler_{90,B0,C0,E0,FF}_*` dispatch slots — never written

§11 trace verified via byte-level binary scan: the 5 saTan0Main MIDI
dispatch slots at `0x800F3340..0x800F3350` are NEVER WRITTEN by any
code in the main EXE or MOVOVL.EXE overlay.  EXE header declares
`bss_size = 0` so the loader doesn't pre-zero either.  The dispatch
arms in saTan0Main (`if (b & 0x80) { switch (cmd) { ... } }`) are
effectively dead code in the shipped game.

**Implication:** Future cheat-cleanup / pure-C work on saTan0Main can
ignore the dispatch call arms (slots are 0 / unused); only the
running-status path + `func_80084A7C` end-of-track handler are
actually exercised.

#### `g_winanim_*` cluster (§15) — was `g_c2_event_frame_*` placeholders

Reading code6cac_c2.c:870-899 + the .data values revealed these 5
constants are absolute frame counts within the post-fight WIN
animation script (controlled by `g_practice_loop_frame` aka
`D_800A37B8`).  Renamed cluster to `g_winanim_*` (callout_a, callout_b,
special, fanfare, particle).

**No misnomer here** — just an upgrade from address-based to
semantic-based names.


## Pass 5: Kengo `name-unique` mass-rename audit (2026-05-18)

Audited 5 families of Kengo-derived names that had been applied across
multiple addresses by the `name-unique` heuristic (where the same Kengo
PS2 symbol matched multiple BB2 functions by SIZE only). Hypothesis: at
most one address per family is semantically the real function; the
others are unrelated functions that happened to share the same size as
the Kengo entry.

Bodies read directly from `src/*.c` or `asm/funcs/*.s`. **Verdict:
20 of 21 functions are confirmed misnomers** (one match, twenty
misclassifications). Demoted with `/* MISNAMED: <body> */` comments in
`named_syms.txt`; replacement aliases added in a new section.

### Family 1: `Vu0SetLightColMatrix_*` (4 functions — 4 misnomers, 0 matches)

VU0 is PS2-only hardware; none of these touch the GTE color matrix.
All four are unrelated 1-line wrappers in `src/system.c`.

| Address | Actual body | Replacement alias |
|---|---|---|
| 0x800801E8 | `cpu_side_move_dir_4();` | `cpu_side_move_dir_4_wrapper_800801E8` |
| 0x80080208 | `marionation_Exec();` | `marionation_Exec_wrapper_80080208` |
| 0x80080640 | `return cdrom_DmaChain() == 0;` | `cdrom_dma_idle_check_80080640` |
| 0x80080684 | `saEft01Init(a0);` | `saEft01Init_wrapper_80080684` |

### Family 2: `camera_SetMatrix_*` (5 names @ 4 addresses — 1 match, 3 misnomers)

The canonical `camera_SetMatrix = 0x80037F40` is itself misnamed (the
addr-suffixed alias at the same address covers the demotion).

| Address | Actual body | Verdict |
|---|---|---|
| 0x80037F40 | Checksums `g_file_disc_size` (0x24 bytes) then copies Quad records | **MISNOMER** → `file_disc_checksum_copy_80037F40` |
| 0x8001DA8C | Calls `func_80035FA8()` + switches on `D_800A38DC` with camera-perspective selections | **MATCH** (camera_SetMatrix_8001DA8C kept; no demotion) |
| 0x8001DBE4 | Checks `g_disp_enable` + dispatches `func_8003AA78/AA48` (framebuffer ops) | **MISNOMER** → `display_framebuffer_dispatch_8001DBE4` |
| 0x8003E164 | Cache check + `gnd_open()` + load via `func_8007B6C8` | **MISNOMER** → `ground_load_with_cache_8003E164` |

### Family 3: `motion_SavePreCalcData_*` (5 functions — 5 misnomers, 0 matches)

None save motion pre-calc data. Three are duplicated `bios_DeliverEvent`
wrappers (`0xF0000003` event class, sizes 0x20/0x40/0x40 — the last two
are identical bodies at different addresses).

| Address | Actual body | Replacement alias |
|---|---|---|
| 0x8003A3F0 | `func_8003A39C(); D_800A3928 = 1;` | `set_event_flag_8003A3F0` |
| 0x8003A574 | `bios_FileRead_B(buf)` wrapper | `bios_FileRead_B_wrapper_8003A574` |
| 0x80080014 | `bios_DeliverEvent(0xF0000003, 0x20)` | `bios_DeliverEvent_class3_evt20_80080014` |
| 0x8008003C | `bios_DeliverEvent(0xF0000003, 0x40)` | `bios_DeliverEvent_class3_evt40_8008003C` |
| 0x80080064 | duplicate of 0x8008003C | `bios_DeliverEvent_class3_evt40_80080064` |

### Family 4: `motion_LoadPreCalcData_*` (3 functions — 3 misnomers, 0 matches)

None load motion pre-calc data.

| Address | Actual body | Replacement alias |
|---|---|---|
| 0x80037F08 | `func_80079A30(); bios_FormatDevice_B();` | `bios_FormatDevice_B_wrapper_80037F08` |
| 0x8005B98C | `saFidLoad(a0, 8); saFidLoad(a0, 4);` | `saFidLoad_pair_8005B98C` |
| 0x8007DC68 | `g_gpu_vcount = sys_VSync(-1) + 0xF0;` | `gpu_vcount_init_8007DC68` |

### Family 5: `mario_getMarioVoiceData_*` (4 functions — 4 misnomers, 0 matches)

None retrieve Mario voice data. Heterogeneous bodies (view-pos init,
sound dispatch, BGM table, mode change) — same-size collision only.

| Address | Actual body | Replacement alias |
|---|---|---|
| 0x8001B3C0 | Reads obj pos 0x180/184/188 into `D_800F5328/532C/5330` | `view_pos_init_from_obj_8001B3C0` |
| 0x8001C820 | Conditional `func_800325E0()` with sound IDs 0x56..0x58 | `sound_dispatch_56_to_58_8001C820` |
| 0x80048AD0 | Loads BGM and inits 0x11 sound-table entries at offset 0x68 | `bgm_sound_table_init_80048AD0` |
| 0x8005BE84 | `title_mv_exec2() + md_game_check_change_main_mode_katinuki()` | `mode_change_to_katinuki_8005BE84` |

### Summary

- 20 lines in `named_syms.txt` demoted with `/* MISNAMED: <body> */`
- 19 replacement aliases added in `=== MISNOMERS pass 5 replacement aliases (2026-05-18 audit) ===` section
- 1 alias left untouched: `camera_SetMatrix_8001DA8C` (the only true match)
- SHA1 unchanged (alias-only adds; existing Kengo names retained for src/*.c compat)

The pattern observed in pass 4 (`katinuki_game_get_katinuki_max_num_*`,
`tslSmdSendVu1Code_*`) repeats here at much larger scale: the Kengo
`name-unique` heuristic is **highly unreliable** when the matched Kengo
name has multiple BB2 candidates — most of those candidates will be
unrelated functions that happen to share the SIZE. Audit pass should
flag any future Kengo `name-unique` rename that lands on 3+ BB2
addresses for the same source name; the default verdict should be
"suspect" rather than "high confidence".


## Pass 6: Comprehensive multi-address-family audit (2026-05-18)

Continuation of pass 5. Surveyed ALL `named_syms.txt` function entries
for multi-address families (same base name at 2+ distinct addresses,
excluding `g_*`/`D_*`/`jtbl_*` data prefixes). Result: 25 families
covering 60 functions, audited in two parallel sub-passes.

**Methodology:** for each function, read the body in `src/*.c` (or
`asm/funcs/*.s` if not yet decomp'd) and classify against the name's
claim: **MATCH** | **PARTIAL** | **MISNOMER** | **UNKNOWN**.

**Aggregate verdict across 50 audited:**

| Verdict | Count |
|---|---:|
| MISNOMER (demote + replace) | 21 |
| PARTIAL (supplementary alias) | 3 |
| MATCH (leave as-is) | 22 |
| UNKNOWN (asm-only, defer) | 4 |

**Self-correction:** the audit also caught one of MY OWN pass-5
additions as a misnomer:
`syscall_wrapper_break_800164F8` is actually a `break 1`-instruction
**trap loop** (~0x270F iterations), not a syscall wrapper. The high-
confidence proposer's `syscall_wrapper` category is itself unreliable
when the body is a break-loop rather than a thin trap wrapper.

### Pass 6A — 3+ address families

#### `saTan2InfoInit_*` (4 addrs — 0 MATCH, 4 MISNOMER)
All four are 1-line forwarding wrappers with no Satan-2 info init logic.

| Addr | Body | Replacement |
|---|---|---|
| 0x80044F80 | `func_80044E74(a0+0x4D, a1)` | `dispatch_via_4D_offset_80044F80` |
| 0x8006C1FC | `func_8006C168(a0, a1)` | `func_8006C168_wrapper_8006C1FC` |
| 0x80077A60 | `func_8006E068()` | `func_8006E068_wrapper_80077A60` |
| 0x80077AC0 | `func_80077724()` | `func_80077724_wrapper_80077AC0` |

#### `InitHiraRmd_*` (3 addrs — 1 MATCH, 2 PARTIAL)
0x80041AC8 is real stage-Hira init. The other two are weapon-afterimage
draw setup loops (related to Hira's combat but not init proper).
Supplementary aliases added; existing names retained.

#### `ang_hosei_*` (3 addrs — 1 MATCH, 1 MISNOMER, 1 UNKNOWN)
0x8003F62C is a real angle-correction orchestrator. 0x80056FE8 is
`char_disp_offset_80056FE8` (D_8009A830/38/40 lookups). 0x800836C8
unresolved (asm-only).

#### `calc_fc_frame_*` (3 addrs — 1 MATCH, 2 MISNOMER)

| Addr | Body | Verdict |
|---|---|---|
| 0x8007EC5C | GTE-accelerated 3×3 matrix × 3-vec rotation | **MATCH** |
| 0x80044378 | counts `-2` terminators in buffer + builds offset table | → `build_offset_table_80044378` |
| 0x800203B4 | extracts motion matrix + applies GTE rotation to vector | → `apply_motion_matrix_800203B4` |

(Both `calc_loc_mat_fw` and `calc_fc_frame` had BARE canonical names
without addr suffix — demoted those too.)

#### `calc_loc_mat_fw_*` (3 addrs — 0 MATCH, 1 MISNOMER, 1 PARTIAL, 1 UNKNOWN)

| Addr | Body | Verdict |
|---|---|---|
| 0x8002AB08 | 200+ line GPU DMA scheduler with GTE matrix branches | → `gpu_dma_schedule_8002AB08` |
| 0x8004A940 | u16 stream decoder dispatching to `func_8004A76C` | → `stream_decode_via_8004A76C_8004A940` |
| 0x80055B60 | asmfix `replace_with_asmfile` stub | UNKNOWN |

#### `md_option_reset_*` (3 addrs — 2 MATCH, 1 PARTIAL)
0x8004001C (set bitflags to 1) and 0x80040068 (clear to 0) genuinely
are option-reset variants. 0x800400B0 is a mode-SETTER (writes a1, not
0/1) — supplementary `md_mode_set_800400B0` alias added.

#### `saTanMainDispGnd_*` (3 addrs — 0 MATCH, 3 MISNOMER)
All three are indirection/dispatch wrappers, not "main display ground".

| Addr | Body | Replacement |
|---|---|---|
| 0x800444BC | 1-line `func_80044504(D_800A378C)` | `disp_dispatch_via_D_800A378C_800444BC` |
| 0x80046020 | `func_800453E0(6) + func_8005B6AC()` | `display_setup_80046020` |
| 0x80077A04 | `D_800A35E4 = 0; func_8006D74C(a0, a1);` | `motion_state_reset_80077A04` |

### Pass 6B — 2-address families

**Legitimate small-variant families (no changes — 7 families, 14 funcs):**
- `AddTbpOfst` (both validate/apply tex-buf offset)
- `coli_CheckBukiPreHit` (both delegate to `saTan2Main`)
- `leaf_muki_awase` (both initialize leaf orientation)
- `motion_Close` (single-player vs pair-mode variants)
- `saSeMain` (sound-slot search variants)
- `saTan5TakeGetPos` (sound-playback position management)
- `suDraw2DLib` (gpu_LoadImage at different mode flags)

**Misnomers (8 of 14 families had at least one wrong addr — 9 funcs):**

| Addr | Was | Now |
|---|---|---|
| 0x8003FFA8 | `FadeOut_*` | `align_4byte_round_8003FFA8` (alignment, not fade) |
| 0x8003FFC4 | `FadeOut_*` | `set_array_slot3_8003FFC4` (field setter, not fade) |
| 0x8004939C | `gnd_close_*` | `sentinel_array_init_8004939C` (-1/-2 fills, not close) |
| 0x80041688 | `gnd_init_*` | `player_equipment_flag_init_80041688` |
| 0x8003D2C4 | `katinuki_game_setData_*` | `gpu_LoadImage_wrapper_8003D2C4` |
| 0x800548DC | `katinuki_game_setData_*` | `ui_stop_selection_800548DC` |
| 0x80077B20 | `saTan2GaugeInit_*` | `gauge_flag_set_80077B20` |
| 0x80077D00 | `saTan2GaugeInit_*` | `gauge_data_ptr_get_80077D00` |
| 0x800164F8 | `syscall_wrapper_break_*` (MY pass-5) | `breakpoint_trap_loop_800164F8` (break-1 loop, not wrapper) |

**Asm-only UNKNOWN (deferred):** `saTan3GaugeMain_*` (0x8006A564, 0x80073200), `calc_loc_mat_fw_80055B60`, and `ang_hosei_800836C8` are asmfix `replace_with_asmfile` stubs or asm-only entries.

### Summary

- 21 `named_syms.txt` lines demoted with `/* MISNAMED: <body> */`
- 21 replacement aliases added in `=== MISNOMERS pass 6 replacement aliases ===`
- 3 supplementary aliases added in `=== MISNOMERS pass 6 supplementary aliases (PARTIAL verdicts) ===`
- SHA1 unchanged (alias-only adds; demotions are comments only)

**State after pass 6:** all multi-address function families with 2+
distinct addresses have been audited. The remaining naming-confidence
gap is in single-address Kengo-derived names (still many unaudited)
and sole_caller_path "helper" proposals (111 medium-conf, not yet
applied).

**Pass-6 hit-rate observation:** for 3+ address families, the MISNOMER
rate is ~75%. For 2-address families, the entire-family hit-rate is
~50% (7/14 legit families), and the per-function misnomer rate within
mixed families is ~35% (9/26 funcs). Recommendation: audit any 3+
family unconditionally; spot-check 2-address families.

