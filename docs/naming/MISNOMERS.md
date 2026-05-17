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
