# system.c sibling cluster — handoff (2026-05-30)

Status of three parked functions in `src/system.c`, after multi-session deep investigation.
Read this BEFORE re-attempting any of them.

## TL;DR
- **tslPolyF4Init** — MATCHED (Tier-4, commit `acc208b`). Reference for the rest.
- **saEft00Add, cpu_side_move_dir_4, marionation_Exec** — parked at technique-class
  exhaustion (every reachable pure-C avenue empirically tested with instrumented evidence).
  Remaining avenues are policy-class only.

## What was already executed (do NOT re-derive)
- ~14 worker sessions, ~7M+ tokens cumulative across the three.
- 4 instrumented cc1 hooks built and preserved in `tmp/gccdbg/cc1` (canonical
  `tools/gcc-2.7.2/build/cc1` SHA1 `045c9543d3...` is untouched and the oracle dep):
  - `BB2_ALLOC_DEBUG` (global.c allocno-priority dump)
  - `BB2_SCHED_DEBUG` (sched.c ready-queue dump)
  - `BB2_PRIO_DEBUG` (sched.c compute_insn_priority dep walk dump)
  - `BB2_REORG_DEBUG` (reorg.c delay-slot fill / mark_target_live_regs dump)
- Random + PERM_*-directed permuter from clean offset-0 targets, multiple bases.
- cc1psx cross-check via `tools/cc1psx_wrapper.sh` (restored commit `d10d34b`).
  Confirmed cc1psx is NOT closer to target on these functions (parity record holds).
- m2c, manual structural variants (~50+ across all three), lever-carryover tests.

## Per-function state

### saEft00Add (src/system.c:980)
- **Mechanism (deepest):** reorg.c `mark_target_live_regs` walk has TWO pollution sources
  for the bnez/beqz eager-fill candidates: (1) JF-LOOP recursive seed via
  `basic_block_live_at_start[bb_of(j_target)]`; (2) USE-INSN-wrapper insertions by
  `update_block` that DIRECTLY add set bits to res (violating monotonic-decrease).
  See commit `bab7361` rule update for full dump excerpts.
- **Partial pure-C lever found:** `cand_inline_ret` (inline-return at end1, files at
  `tmp/cand_*.c`) closes site1's eager fill but not site2. The 3-way trilemma —
  unmerged tails ∧ JF-LOOP-safe ∧ v0-reuse — has goals 1+3 aligning and 2 contradicting.
- **Full match:** not reachable in pure C with current toolchain understanding.
- **Rule:** `.claude/rules/cross-jump-store-tail-merge.md` "Session 12" section.

### cpu_side_move_dir_4 (src/system.c:386)
- **Real progress:** sandbox-distance 20 → 15 via session-8 lever (commit `31ad63b`):
  ```c
  D_800F19C0 = (void *)((u8 *)tbl_125c + ((s32)&D_80016240 - (s32)D_800A125C));
  ```
  Combine-foldable symbol_ref delta chain — bumps `tbl_125c`'s reg_n_refs 3→4, flips
  allocno priority 197→526, lands tbl_125c in $s3 matching target. **Zero emitted bytes.**
  Result: 5 of 6 callee-saves now match target ($s1 saved, $s2 idx_1494, $s3 tbl_125c).
- **Residual 15-diff:** idx_1495 needs to flip $s6→$s4. Mechanism is the 3-pseudo coupling
  (idx_1494/tbl_125c/idx_1495 sharing ref-count from same indexing ops; bumping one
  drops another). Random permuter plateau 505 over 80 candidates; PERM_*-directed
  plateau 505 over 336 candidates. Parallel-method validation of the structural ceiling.
- **Rule:** `.claude/rules/register-alloc-pure-c.md` "Confirmed limit" + Sessions 5-11.
- **Workspace preserved:** `permuter/csmd4_v8/` (lever-applied base.c, target.o, candidates).

### marionation_Exec (src/system.c:507)
- 44 regfix rules. Same mechanism class as cpu_side_move_dir_4 at LARGER surface
  (6-way rotation across $s1-$s7 vs cpu_side's single rotation).
- **Session-8 lever-carryover empirically falsified** (regresses 55→57; instrumented
  measurement, commit before `feaa946`).
- Random permuter plateau 1125 (270 candidates, commit `feaa946`). PERM_*-directed
  run was launched but worker terminated mid-wait — workspace `permuter/marionation/`
  preserved.
- **Rule:** `.claude/rules/register-alloc-pure-c.md` "Confirmed limit — marionation_Exec".

## What works (lever class that landed tslPolyF4Init + cpu_side 20→15)
**Combine-foldable symbol_ref delta chains:** an expression of the form
`(s32)&symA - (s32)&symB` where both A and B are symbol_refs the compiler can
subtract at link time. Combine folds the subtraction to a constant offset.
Useful for bumping a pseudo's ref count without emitting bytes. See `register-alloc-pure-c.md`
session-8 entry for the full recipe. **Applies broadly across the codebase** — try first on
any function whose only remaining gap is allocno-priority-driven register rotation.

## What does NOT work (don't re-try)
- `unsigned long long new_var2` chain (DImode pseudo for doubled allocno priority) —
  works for the matched siblings (they have a real long-long loop counter) but
  doesn't apply where no natural use exists; either folds away or emits real DI arith.
- `register T x asm("$N")` pins — silently ignored when fighting RA preference.
- `volatile` cast on tbl/idx — emits wrong addressing form (extra `lui+addiu` vs `%lo`).
- cc1psx as the build's compiler — empirically verified NOT closer on these
  functions (23-diff measurement at lever-applied base for cpu_side; SOTN parity
  record holds). Per `cc1psx-calibration-only.md` it's a self-disproof tool only.
- Project-wide rodata reorder — see `community-standard.md` (corrected 2026-05-30):
  no reference PS1 decomp project does this; fallback for unmatchable is INCLUDE_ASM.

## What remains (genuine choices, not unrun avenues)
1. **Canonical-asm (tier-1) authorization** per `canonical-asm-retirement.md` for one
   or more of the three. SOTN-equivalent fallback (their `INCLUDE_ASM`). User policy.
2. **Wait for toolchain improvements** — these functions may yield to a future
   `reorg`-permissiveness fix or a different allocator. The evidence ledger is
   structured for clean resume.

## Standing directive (READ if pursuing further)
`.claude/rules/difficult-is-not-impossible.md` includes a 2026-05-29 directive:
**do NOT park with documented unrun resume avenues.** Execute every named technical
lever before any park/exhaustion commit. The 3 parked functions reached their
current state by exhaustively honoring this — every "Resume here" pointer in prior
session commits was subsequently executed. Don't list new resume avenues without
executing them.

## Quick orientation commands
```
pwsh.exe tools/eng.ps1 sandbox <func> --disable all     # honest pure-C distance
git log --oneline -- .claude/rules/register-alloc-pure-c.md .claude/rules/cross-jump-store-tail-merge.md
BB2_ALLOC_DEBUG=1 tmp/gccdbg/cc1 <flags> minimal.i      # priority/conflict dump
BB2_REORG_DEBUG=1 tmp/gccdbg/cc1 <flags> minimal.i      # delay-slot fill dump
```
