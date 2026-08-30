# 2026-08-30 — Sched-tie endgame campaign

Owner-approved 2026-08-30 ("go ahead" on the dedicated-campaign proposal).
Companion to `2026-08-06-inverse-solver-design.md` (whose four phases have all
shipped — this campaign APPLIES that tooling; it does not rebuild it).

## Motivation

The grind ledgers show one failure class absorbing wildly disproportionate
effort. Lifetime session counts (2026-08-30):

| function | sessions | residual class |
|---|---|---|
| CD_sync (LIBCD/BIOS) | 105 | 2-insn sched pair (LAUNCH tie, luid-ordered) |
| CD_ready (LIBCD/BIOS) | 58 | masked 4; two root-caused sched/RA residuals |
| func_80045294 | 47 | **distance 2** — one prologue sched2 pair, fully characterized |
| func_80057CC8 | 45 | 3-insn register-coalescing decision |
| get_alarm (libgpu sys.c) | 42 | 1 combine fold + 8-op sched1 reorder (debug_printf setup) |
| func_80017848 / func_80034F88 / CD_datasync | 25 / 23 / 18 | CD_datasync: RA rotation + debug_printf arg-block reorder |

Nearly every residual is a 1–3 insn scheduler-tie or RA-coalescing endgame, and
the ledgers independently rediscovered the same mechanics per function. The
reference-C path is exhausted for the library members (psyz sweep 2026-08-18:
CD_ready is bios.c v1.86 verbatim — residual is codegen reproduction, not
provenance; SOTN/Tomba/Xeno spellings measured worse and banked).

Shared sub-pattern: the debug_printf argument-block schedule appears in the
ledgers of CD_sync, CD_ready, CD_datasync, get_alarm, and sprintf — a lever
found once should transfer.

## Tooling state (recon 2026-08-30)

Forward models are exact (sched 6978/6978 order+clock; RA global 10/10; reload
194/194) and the inverse layer EXISTS: `sched_solver/perturb.py`
(`--goal-from-target`, `--target-object`/`--ours-object` escape),
`ra_solver/inverse.py` / `inverse_sched.py` / `inverse_reload.py`,
`levers.py` (perturbation → C-technique tiers, FORECLOSED reporting),
`goal_from_tgt.py` (object-level goal derivation). Grinder integration is
prompt-text only (solver modality playbook, grindlib.py).

Known honest limit (sched README): model-spellable ≠ C-spellable — address
materialisations and LICM-hoisted constants are inert levers.

## Work items

1. **Ruling-1 repairs** (granted, decisions.md escalation-batch 2026-08-30;
   all diagnostic-only, frozen toolchain untouched):
   - **(b) DONE (this commit):** `inverse_compose.py classify` refuses the
     text path for any zero-rule function — post rules-to-zero that is every
     function, retiring the confident-fiction failure mode (func_80089F3C,
     func_800645B0) in every src state, including mid-session applied bodies.
   - **(a)** extend the instrumented-cc1 `block_alloc` hook to dump AND score
     `qty_phys_copy_sugg` / `qty_phys_sugg` / `qty_size` (named likely cause of
     local_alloc.py's 4–5% order misses).
   - **(c)** flow-analysis→global_alloc deletion-window dump + matching
     ra_solver atom class (func_80078654's gap).
2. **Validation target: func_80045294.** Smallest gap in the cluster (distance
   2, one sched2 pair, ledger has the full chain analysis). Run the full chain
   — `goal_from_tgt.py classify` → `inverse_sched.py --target-object` →
   `levers.py` tiers — and either land the lever or bank a FORECLOSED verdict
   with the atom named.
3. **Cluster diagnostic sweep** (extends `sweep.py`; evidence-prep only, does
   NOT touch queue order — no cherry-picking, no out-of-order completions).
   For each cluster member, bank the classify verdict + ranked lever report
   into `memory/grind/<func>/evidence.md`, so the session that receives the
   item at queue-top starts warm instead of paying the rediscovery tax.
   Order: func_80045294, get_alarm, CD_datasync, func_80057CC8, CD_ready,
   CD_sync. Solve the debug_printf arg-block sub-pattern once, apply across.
4. **Feedback loop.** Sweep results land in the ledgers + `docs/grind/`
   report; mechanically-FORECLOSED residuals become escalation packets per
   standing policy (escalation-not-parked). Judge gates every completion as
   usual — the campaign changes evidence quality, not standards.

## Constraints

COMPLETED-C bar, cheats-by-any-spelling, frozen family list, default-FAIL
Judge, oracle SHA1 — unchanged. All cc1 instrumentation is dump-only and
byte-parity-checked against `build/cc1` by `extract.py` before use. Queue
discipline holds: the cluster items reach completion only when they surface at
queue-top; the campaign pre-banks the evidence they'll need when they do.
