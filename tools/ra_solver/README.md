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
