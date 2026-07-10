# CLOSER MISSION — Phase 5: CD_ready/CD_sync final close-out

The last two contested libcd functions. Phase 2 (memory/closer/
phase2-closeout.md) proved the faithful v1.86 source reproduces the contested
window standalone and left a concrete port roadmap. Phase 3 adoptions have
since landed many sibling libcd/libspu functions and several volatile grants —
the in-context environment has CHANGED since Phase 2 measured; re-measure
before trusting any Phase 2 in-context numbers.

Read FIRST: memory/closer/phase2-closeout.md (full), docs/closer/rulings.md
(Rulings 1-3), memory/closer/candidates/marionation_vAT1_notailwrap.c,
tmp/marion_faith2/ if present, memory/closer/libcd-groundtruth.md,
memory/closer/phase3-progress.md (what landed around these functions).

## W1 — marionation_Exec (= CD_ready) → 0

Start from the banked best (vAT1_notailwrap, masked 2, 1-pair residual,
179/179). The Phase 2 frontier roadmap:
1. In-context faithful port (per-member Alarm stores, extern volatile CD_intr
   struct, while(1) loop) — port iteratively from tmp/marion_faith2/, diffing
   against the standalone proof at each step; measure csmd4 collateral each
   change.
2. The fp-allocatability fork divergence (SN cc1psx keeps $fp non-allocatable;
   our cc1 allocates it, shifting loop.c's move threshold): if the comstr
   movable keeps winning s8 in-context, count the exact threshold
   (n_non_fixed_regs) and test +2..+8 insn_count respellings that combine
   folds back. If NO honest spelling clears it, file a ruling-request under
   Ruling 2 with the cc1psx exhibit — that is the documented endgame, not a
   failure.
3. Volatile grants: if the faithful port needs `extern volatile CD_intr
   D_800A1494;` (it does — Intr is volatile in Sony source), propose the grant
   in memory/closer/volatile-grant-proposals.md (two-prong + ground truth is
   already fully documented in libcd-identity.md; cite it) and list the
   blocked func in the outcome. The operator grants between sessions.

## W2 — cpu_side_move_dir_4 (= CD_sync) → honest close

After (or in parallel with) W1's port: write CD_sync from the same faithful
helpers (sync arm: temp==2||temp==5 → Intr.sync=2 → _memcpy(result,
D_800F19A0, 8)), replacing the h5 fakematch initializer (Ruling 1: h5 is a
fakematch — the function CANNOT complete with it). Same volatile-grant note
as W1. Target: sandbox 0, or the minimal measured floor with the full
partition evidence for a Ruling-2 request.

## Completion discipline

- These are the two most-audited functions in the project. Bank every
  measured form. If either reaches sandbox 0 (--disable all) with edits in
  place: list it in completed_funcs (the driver strips its rules — CD_sync
  has 5, CD_ready 42 — re-proves the oracle, and runs the queue-done gate).
- If blocked on a grant: result "ruling-request", precise question, and STOP
  on that function — do not invent workarounds for a gated decision.
- Update memory/closer/phase5-twins.md with the session ledger.
