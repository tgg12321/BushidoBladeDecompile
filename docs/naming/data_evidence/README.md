# Data symbol evidence

Per-symbol prose evidence files for `D_*` globals that have been
hand-reviewed and assigned a proposed name. Parallel to the
function-side `docs/naming/evidence/` directory.

## How this differs from `data_symbols_quick_wins.md`

`docs/naming/data_symbols_quick_wins.md` is a mechanical
classification of every D_* symbol by access-width pattern (lw/lh/lb
only, function-pointer call shape, GTE direct reference, etc.). It
produces type-style names like `wtbl_*` / `stbl_*` / `btbl_*` /
`fp_*` -- useful as quick hints, but explicitly NOT semantically
meaningful and explicitly NOT for bulk-renaming.

The files in this directory take the opposite approach: pick one
data symbol, read every function that touches it, document the
access pattern, and propose a *semantically meaningful* name (e.g.,
`g_rand_state`, `g_disp_config`, `g_tpage_slot_ptr`). One file per
symbol, more discursive than mechanical.

## What's here

Initial batch (2026-05-17), six symbols referenced by the
already-named functions from the 10-function batch in `evidence/`:

| File | Proposed name | Confidence | Why |
|------|---------------|-----------:|-----|
| [D_800F1848.md](D_800F1848.md) | `g_rand_state` | high | LCG state owned exclusively by `bb2_rand` |
| [D_8008F13C.md](D_8008F13C.md) | `g_special_cam_command_table` | high | 8-byte-stride command-descriptor table touched only by `special_cam_issue_command` |
| [D_80103608.md](D_80103608.md) | `g_tpage_slot_ptr` | medium-high | Paired-array slot pointer pool (manager pair + 10 consumers) |
| [D_80103658.md](D_80103658.md) | `g_tpage_slot_count` | medium-high | Partner count array to the above |
| [D_8009BD24.md](D_8009BD24.md) | `g_disp_config` | medium-high | Display-config struct loaded by `disp_load_config_from_buf` |
| [D_800A379E.md](D_800A379E.md) | `g_motion_state_code` | medium-high | Motion-state s16 returned by `game_state_advance` |

## Workflow

Each evidence file follows the same skeleton:

1. **Header** -- address, access width / size, touchers (count +
   files), proposed name, confidence.
2. **Sole / primary toucher** -- a short prose paragraph + the C
   body of the function that most clearly establishes the role.
3. **Readers / writers** -- enumeration of the other touchers and
   what they do with the symbol.
4. **Why this name** -- the reasoning + the confidence rationale.
5. **Confidence-raising next steps** -- what a reviewer could do to
   lift the proposal from its current confidence to higher.
6. **Reviewer alternatives** -- plausible alternative names with
   when each would be preferred.

The format mirrors the function evidence in `../evidence/` so a
reviewer who has read those will find the structure familiar.

## Application gating

These are proposals only. To actually rename a symbol in
`named_syms.txt`, follow the same gating as function renames:

1. Move the line from `undefined_syms_auto.txt` to
   `named_syms.txt`, replacing `D_<addr>` with the proposed name.
2. Re-run splat so the new name propagates to the generated header.
3. `make` and verify SHA1 still matches the original.
4. Spot-check that the new symbol name reads naturally in 2-3
   touching functions.

A failed step 3 means the proposed name conflicts with something
unexpected (e.g., a function with the same name) -- back out.

A passing step 4 means the rename is a net readability improvement.
