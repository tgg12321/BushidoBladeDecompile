# ra_solver — reverse register-allocation solver (GCC 2.7.2 / MIPS)

Purpose: turn the register-allocation endgame ("our allocation differs from
target's by a small permutation") from blind C-spelling search into a
directed one: extract the exact allocation inputs, simulate `global.c`
faithfully, then search for minimal input perturbations that produce the
TARGET's assignment — each surviving vector maps to known C-level phenomena
(refs = uses, livelen = statement span, birth order = first-def order,
conflicts = live-range overlap, prefs = copy relationships).

## Tools

| file | role |
|---|---|
| `extract.py <func> <stem>` | run cpp + the instrumented cc1 (`tools/gcc-2.7.2/cc1`, BB2_ALLOC_DEBUG/BB2_FINDREG_DEBUG env hooks, print-only, parity-verified vs `build/cc1`) with `-da`; parse `.greg` (order/conflicts/prefs/dispositions), `.flow` (nrefs/livelen/calls-crossed incl. negatives), `.lreg` (per-function banners; modes; use-only + MD-class detection) + the func-tagged ALLOCDBG/seed/FINDREGDBG stderr → `tmp/ra_solver_work/<func>.model.json` |
| `simulate.py <model.json> [--trace]` | Python replication of `allocno_compare` sort (pri desc, tie → lower pseudo = birth order), `prune_preferences` (someone_prefers from FULL pref sets), `find_reg` (used1 assembly, pass0 with the call-used+ever-live+local-renumber seed, pass1, copy- then full-pref upgrades), post-assign conflict propagation. Skips: reg_live_length<0, use-only (NO_REGS), MD-class → $lo |
| `validate.py` | batch ground-truth check over the banked six + completed controls |
| `local_extract.py <stem> [--func N]` | **Phase 5.** LOCAL-alloc ground truth: runs the instrumented cc1 with `BB2_QTY_DEBUG=1`, whose `block_alloc` hook prints one line per quantity per block for both allocation loops (`QTYDBG-SUGG` = suggested-register pass, `QTYDBG` = main pass) with `blk/ord/qty/reg1/birth/death/refs/got`. QTYDBG is **not** function-tagged, so rows are segmented by the fact that `local_alloc` runs before `global_alloc` per function: every QTYDBG line belongs to the function named by the NEXT `ALLOCDBG func=` line. → `tmp/ra_solver_work/<stem>.local.json` |
| `local_alloc.py <stem> [--func N]` | **Phase 5.** Replication of `block_alloc`: ORDER check recomputes `qty_compare` (`floor_log2(refs)*refs*size/(death-birth)*10000`, tie → lower qty) against the `ord` column; ASSIGN check replays `find_free_reg` (`used = fixed_reg_set | union(regs_live_at[birth..death))`, ascending scan since MIPS defines no `REG_ALLOC_ORDER` here, then `post_mark_life`) against `got` |

## Phase 5 — local-alloc status (2026-08-04)

Validated across five TUs, from an **empty** initial hard-reg liveness map (the
model covers qty-vs-qty interference only):

| TU | order | assign (main pass, GR_REGS) |
|---|---|---|
| main | 617/642 (96.1%) | 1052/1129 (93.2%) |
| code6cac | 544/568 (95.8%) | 1054/1149 (91.7%) |
| display | 209/221 (94.6%) | 365/415 (88.0%) |
| text1a | 163/172 (94.8%) | 321/359 (89.4%) |
| config | 67/68 (98.5%) | 93/108 (86.1%) |

Banked functions: `title_mv_exec2` **2/2 order, 10/10 assign**;
`func_8007C7A0`/`func_8007C86C` **6/6, 4/4**; `func_8007CE0C` **9/9, 30/30**;
`saTan4FireDisp` 7/8, 18/19; `tslGlobalMemFree_800861BC` 5/6, 31/33.

**Two categories are reported, not scored**, because the hook does not dump the
inputs they need: `sugg` rows (the suggested-register pass scans
`qty_phys_copy_sugg`/`qty_phys_sugg`, not ascending order) and `mdreg` rows
(class was MD_REGS, landing in `$hi`/`$lo`). The residual ~7% of scored misses
are blocks with live hard registers — incoming argument regs, the call-clobbered
set around a `CALL_INSN`, the return value — which an empty liveness map cannot
know.

**Known hook gaps for the next extension** (both in `local-alloc.c` `block_alloc`,
same env-gated-fprintf pattern): the QTYDBG line does not print `qty_size` (so
DImode qtys are mispriced in the ORDER check — the likeliest cause of the ~4-5%
order misses) nor the suggestion sets.

### Applying it — worked example

`title_mv_exec2`'s residual was closed out in Phase 4 as "a local-alloc decision
the solver cannot see". With this model it becomes a spec. Block 2 in allocation
order is qty 2 (pri 60000) → `$v0`, qty 1 (pri 50000) → `$v0`, qty 0 (the
sign-extend, span [6,38), pri 22500) → `$v1`, qty 3 (pri 20000) → `$v1`. Target
holds the sign-extend in `$a1`, which needs `$v0`, `$v1` *and* `$a0` occupied
across [6,38). Replaying the block (`tmp/ra/tmv_local.py`) shows a **two-part**
requirement: one extra qty overlapping [6,38) with pri > 22500 moves it only to
`$a0`; hard `$a0` live across that range alone leaves it in `$v1`; **both
together** give `$a1`. That is why the call-argument family, which supplies only
part 2, was uniformly inert.

## Phase 5 stage 2 — reload spill-retry (NOT STARTED)

The other unmodeled mechanism, and the one live in `saTan4FireDisp`'s own dump
(pseudos 100/106/112). Entry points for whoever picks it up:

- `global.c` `retry_global_alloc` and the `losers` / `alt_regs_p` arguments to
  `find_reg` — the tail of `find_reg` already contains the "kick out regs that
  local-alloc used poorly" path, currently unmodeled by `simulate.py`.
- `reload1.c` — the spill loop that re-enters `retry_global_alloc` with grown
  conflict sets. The `ra_solver` README's original validation note recorded that
  `saTan4FireDisp`'s dump shows `retry=1` with conflicts 2-7,17,18,29,30 against
  the dump's 2-5, and that the simulator matches the `retry=0` initial call.
- The BB2_FINDREG_DEBUG hook already prints `retry=`, so retry occurrences are
  observable today without touching cc1; what is missing is the *conflict set at
  retry time*, which would need one more env-gated fprintf.

Ask the solver's question in reverse once modelled: what spill/loser pattern
rotates `saTan4FireDisp`'s `$s`-registers to target, and what body property
produces that pattern. Note the standing constraint from the else-arm round —
target's frame has no spill slots, so any answer must rotate registers *without*
spilling.

## Validation status (2026-08-04)

9/10 functions reproduce the dump's dispositions EXACTLY (order 10/10).
The 10th (saTan4FireDisp, both HEAD and candidate forms) differs on one
allocno triple because **reload's spill-retry** re-enters `find_reg` with
grown conflict sets (verified: the FINDREGDBG block shows `retry=1` with
conflicts 2-7,17,18,29,30 vs the dump's 2-5) and the dump records the
post-retry state; the simulator's answer matches the `retry=0` initial
call exactly. Pre-reload fidelity is the search layer; when a target
function's model shows retry blocks, treat sim output as the pre-reload
allocation and verify candidates with the hook.

## Semantics notes (hard-won; don't re-derive)

- `regs_used_so_far` is SEEDED with call-used + regs_ever_live +
  local-alloc renumberings ("no cost in using them") — pass 0 packs
  caller-saved regs from the start. The tagged seed line dumps it exactly.
- The dump's `;; N preferences:` line is `hard_reg_preferences` only;
  `someone_prefers` needs `hard_reg_full_preferences` → harvested per
  pseudo via BB2_FINDREG_DEBUG.
- Allocno ties break by allocno index = pseudo number = BIRTH ORDER —
  a directly spellable C lever (first-definition order).
- Orphan/folded pseudos (zero real-operand RTL appearances at lreg time)
  keep class NO_REGS and are never allocated: the phantom-slot class.
- `.flow` "across -1 insns" is real and meaningful (reg_live_length<0 ⇒
  global skips the allocno).
- lreg spells user variables `(reg/v:SI n)` — regexes must allow flags.

## Instrumented-cc1 note

The hooks live in `tools/gcc-2.7.2/global.c` (env-gated fprintf only) and
are compiled into `tools/gcc-2.7.2/cc1` via `TMPDIR=/dev/shm make cc1` in
that directory. `tools/gcc-2.7.2/build/cc1` (the build compiler) is NEVER
rebuilt by this; parity is asserted by compiling a TU with both and
comparing output (tmp/parity_check.sh pattern). This is diagnostic
instrumentation under the established BB2_*_DEBUG practice, not a
compiler change ([[no-compiler-divergence]] untouched).
