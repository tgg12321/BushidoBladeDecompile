# Hypothesis ledger — func_80087D10

## s1 (recon, 2026-07-21) — floor 8 → 6

- H1 CONFIRMED: fresh single-set `p`/`base` locals give the base lw the scheduler's load-late
  launch priority; folding `p` into `base` (multi-set `base += slot*176`) removes it.
  Measured 8 → 6; index reg also flips v1 → v0 matching target.
- H2 KILLED: single-use pointer store (`s16 *ptr = &D_80102806; *ptr = a0;`) serializes the
  sh against the base lw the way sibling CAC's two-use pointer does. — cse folds the pointer
  into the MEM before sched1; bytes identical to direct store; still 6.
- H3 KILLED: C-level param reuse for slot (`a0 = (a0 & 0xff00) >> 8`) reproduces target's
  a0-clobber and pins order. — scores 7 (worse); the clobber raises sh priority even higher.
- H4 KILLED: C statement order controls sh-vs-lw emission order. — all three orderings
  (load/store/slot, load/slot/store, store/load/slot) score 6 with identical bytes.

## Frontier (for s2)

- F1 (primary): **sched dump forensics.** cc1 `-da` (.sched/.sched2 dumps) on the V1 .i file;
  read the dependence lists + INSN_PRIORITY that place sh/andi/sra ahead of the lw macro, and
  which pass does it (sched1 vs sched2). Then enumerate C spellings that lower the sh-chain
  priority or raise the lw's (e.g. reshaping what feeds the mul chain) — the whole residual is
  ONE 3-insn block move.
- F2: **legitimate alias-opacity spelling.** Find a C spelling where the store address is
  reg-opaque at sched time yet emits the $at macro after reload (REG_EQUIV substitution path).
  Plain single-use pointer measured dead (H2); candidates: multi-set pointer, pointer whose
  init cse can't fold, address passed through a live computation. Any candidate must clear the
  cheat catalog (pointer-alias family = sanctioned-with-prereqs; sibling CAC precedent).
- F3: **permuter campaign from V1 base** (score 6, 18-insn leaf — cheap). Sweep spellings of
  the *176 computation, base/slot decls, and read expression; fresh-seed discipline per
  [[permuter-fresh-seed-discipline]].

## [s1] Fresh single-set locals (p, base) give the base-table lw the scheduler's load-late launch priority; making base multi-set (base += slot*176) removes it
- mechanism: GCC 2.7.2 reg_n_sets==1 dest gets load-late launch priority (staged-value-reused-variable rule mechanism); multi-set base also shifts the index into v0, whose reuse by the mul chain creates a post-RA anti-dep that stops the lw sink 4 slots earlier
- probe: Replaced p with base += slot*176; return *(s16 *)(base + 0x58); ran sandbox --disable all
- result: score 8 -> 6; index reg v1 -> v0 matching target; lw sink shrank from 7 slots to 3 (built_v1_multiset_base.s)
- verdict: CONFIRMED

## [s1] A single-use pointer store (s16 *ptr = &D_80102806; *ptr = a0;) serializes the sh against the base lw like sibling func_80087CAC's sanctioned two-use pointer
- mechanism: Hypothesized reload-time REG_EQUIV substitution would keep the store address reg-opaque through sched1 (blocking alias disambiguation) yet emit the target's direct $at-macro form
- probe: Built the single-use pointer form; sandbox + objdump of the sandbox object
- result: cse folds the pointer into the MEM before sched1; bytes byte-identical to the direct store, still 6 (built_v3_single_use_ptr.s) - mechanism does not transfer to single-access stores
- verdict: KILLED

## [s1] C-level param reuse for slot (a0 = (a0 & 0xFF00) >> 8) reproduces target's a0-clobber and pins the target order
- mechanism: Explicit clobber creates the sh->andi anti-dependence at the pseudo level from sched1 onward
- probe: Built the param-reuse form; sandbox
- result: score 7 (worse than 6): the anti-dep chain RAISES the store cluster's priority, moving it even earlier - wrong direction
- verdict: KILLED

## [s1] C statement order (store vs load vs slot) controls the emitted sh/lw order
- mechanism: If sched preserved source order the barrier-free source order would land the match
- probe: Measured all three orderings: load/store/slot, load/slot/store, store/load/slot
- result: all score 6 with identical bytes - emission order is entirely scheduler-determined, statement order is inert
- verdict: KILLED
