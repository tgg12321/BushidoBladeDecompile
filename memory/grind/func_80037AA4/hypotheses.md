# Hypothesis ledger — func_80037AA4

## s1 (recon, 2026-07-28)
- H1 "a0<->v1 rename needs pins" — KILLED: pointer-decl-first flips it in the
  guarded do-while form; sandbox 14 -> 4. (probe: sweep.py v1_p_first + sandbox)
- H2 "8-byte frame requires a dead local (cheat)" — KILLED: unguarded
  entry-test loops (possibly-zero-trip) naturally allocate a phantom stack
  temp, vars=8 with zero stores. (probe: sweep.py v10/v11, sweep3 t9/t11)
- H3 "original tail is division /0x2000" — KILLED as a spelling (worse
  schedule), but the T0 explicit-if IS division codegen shape; tail register
  pattern proves T0 spelling. (probe: probe_div*.s, sweep5)
- H4 "indexed struct source reproduces full structure" — CONFIRMED: la after
  blez + lw 24 + stride 40 + vars=8 + 21 insns. Remaining: sum<->giv
  allocation order under the T0 tail. (probe: sweep4/sweep6/sweep7)

## Frontier for s2
1. Flip sum<->giv on the unguarded indexed base while keeping the T0 tail.
   Mechanism: global.c allocno ordering (sum's 4 tail refs vs giv priority).
   Next probes: read gcc-2.7.2/global.c allocno_compare to compute the exact
   priority formula and derive the needed ref/length delta; then targeted C
   respellings (e.g. lengthen sum's live range, split sum into loop-local +
   result var, tail temp reuse of var_a1). Then a decomp-permuter campaign
   seeded from the indexed base (score 6/11 forms are excellent seeds).
2. Alternatively: induce vars=8 on the guarded do-while floor-4 form. Next
   probe: pinpoint the pass creating the slot (dump reg numbers: compare
   in.i.loop/jump dumps of v12 vs v13 in tmp/grind/func_80037AA4/s1/dump_*),
   then search for a guarded-compatible natural trigger.
3. If (1) closes: candidate is the unguarded indexed form — verify semantics
   identical (trip-0 equivalence holds: for-loop skips when n<=0, same as
   guard) and vet typedef placement per file conventions.

## [s1] The a0<->v1 accumulator/pointer rename requires register pins
- mechanism: GCC 2.7.2 allocates user pseudos in decl-scan order for tied priorities; pointer-first declaration flips the assignment
- probe: sweep.py 24-variant decl/init/type sweep + sandbox --disable all on v1_p_first form
- result: sandbox 14 -> 4 with s8 *var_v1 declared first, zero pins, zero dummy; only the 8-byte frame remains
- verdict: KILLED

## [s1] The target's 8-byte no-store frame is reproducible only via a dead-local cheat
- mechanism: phantom-frame-slots-gcc272: cc1 reserves a stack temp (vars=8, zero stores) for any loop whose trip count is not provably >=1 (duplicate_loop_exit_test path); guarded do-while forms get vars=0
- probe: cc1 .frame instrument (frameprobe.sh) across 30+ variants: unguarded while/for/indexed all vars=8, every guarded form vars=0
- result: vars=8 arises naturally from unguarded entry-test loops; unguarded indexed struct form reproduces la-after-blez + lw 24(p) + stride 40 + 21 insns = full target structure
- verdict: KILLED

## [s1] Original tail was written as division by 0x2000
- mechanism: signed div-by-2^13 expansion matches target's bgez/addiu/sra shape
- probe: probe_div*.s: /0x2000 spelling on pointer-walk and temp-split bases
- result: division SPELLING is worse (loop schedule breaks: i++ hoists above lw leaving a nop; in-place tail); but tail register pattern (move $2,$4 delay, sra $4,$2,13 quotient-coalesced-into-sum) proves the explicit-if T0 spelling
- verdict: CONFIRMED

## [s1] Unguarded indexed base + T0 tail closes to 0
- mechanism: with T0's 4 tail refs on sum, global-alloc orders sum before the strength-reduction giv: sum=$3/p=$4 (target sum=$4/p=$3); decl order inert because the pointer is a compiler giv
- probe: sweep4 (24 decl perms), sweep6 (8 tail spellings), sweep7 (10 base spellings) + sandbox: indexed+T0=11, indexed+inplace-tail=6
- result: swap robust against all natural spellings tried; only in-place tail flips it and that breaks the tail shape
- verdict: KILLED

## s2 (structural, 2026-07-28)
- H5 "vars=8 can be induced on a guarded form by a natural construct" —
  CONFIRMED: `if (var_a1 < var_a2)` guard (a1 just zeroed) orphans its slt
  pseudo (cse folds operand, combine folds branch to blez, reload alter_reg
  gives the dead pseudo a 4-byte slot). sandbox 11, build insns 23/23 == target.
  (probe: gdb break assign_stack_local -> alter_reg backtrace; sweep_s2.py)
- H6 "the sum<->p swap on the orphan base flips by decl order" — KILLED:
  120/120 permutations invariant. No tie exists to break: the guard's slt
  charge (+1 sum ref via cse longest-lived canonicalization) makes sum pri
  22000 vs p 20000 outright. (probe: sweep_s2b.py)
- H7 "the swap flips by guard operand identity / init order / stmt order" —
  KILLED: cse always canonicalizes the zero-class to sum (longest-lived rule,
  cse.c make_regs_eqv); all spellings charge sum. (probe: sweep_s2c.py + csewho.sh)
- H8 "p's priority can be boosted by an address-temp / split-add extra ref" —
  KILLED: cse folds tp=p+0x18 into the MEM and split addiu chains into one
  insn BEFORE flow counts refs; no flow-time delta. (probe: sweep_s2d/s2e.py)
- H9 "a second combine-foldable guard adds +2 sum live_length" — KILLED as
  bytes: same-expr dedups (cse); different-expr second tests either emit a
  second blez (jump2 never dedups branches) or survive in a second bb where
  combine cannot fold (no cross-bb LOG_LINKS). BUT the bb-separated variant
  (a2>=1 && a1<a2, n=20) empirically confirms the len-17 flip threshold:
  sum 11/17=19411 < 20000 allocates sum=$4/p=$3. (probe: sweep_s2f.py)

## Frontier for s3
1. Find a bb0 construct that survives flow and is combine-deleted, worth +2
   sum live_length (or +1 weighted p ref), with zero byte drift. Mechanism:
   only branch-fed value chains survive cse; only combine deletes post-flow;
   combine needs same-bb LOG_LINKS. Next probe: enumerate GCC 2.7.2 combine.c
   3-insn patterns that fold two flow insns into zero final insns in bb0
   (e.g. double-slt chains feeding one branch, sne/seq expansions).
2. decomp-permuter campaign seeded from BOTH 11-score forms (orphan-guard g1
   and indexed T0) + the floor-4 g0 — the permuter may find an unthought
   spelling that shifts the ref/len arithmetic; the g1 seed is one register-
   allocation event away from 0. (modality: permuter)
3. If a flip form is found: verify semantics (guard `a1<a2` with a1==0 is
   identical to `a2>0`) and re-measure vars=8 + sandbox 0 in the same session.

## [s2] vars=8 can be induced on a guarded form by a natural construct
- mechanism: guard `if (var_a1 < var_a2)` (a1 just zeroed) emits a reg-reg slt pseudo; cse folds the operand, combine folds slt+branch into blez a2, orphaning the pseudo; reload1.c alter_reg allocates the dead pseudo a 4-byte spill slot (gdb-proven backtrace), giving addiu sp,-8/+8 + final jr-delay nop with zero stack stores
- probe: gdb break assign_stack_local on minimal TU + sweep_s2.py; sandbox on g1 form
- result: vars=8, build 23/23 == target insn count, sandbox 11 (only sum/p register swap remains); slot invisible in all -da dumps (allocated at reload, RTX discarded)
- verdict: CONFIRMED

## [s2] the sum<->p swap on the orphan base flips via decl order
- mechanism: s1's pointer-decl-first lever worked on g0 because sum 10/15 and p 7/7 priorities tie at 20000 and allocno_compare tie-breaks by lower pseudo number; the orphan guard's +1 sum ref (11/15=22000) removes the tie
- probe: sweep_s2b.py: all 120 decl permutations on the g1 base
- result: 120/120 invariant sum=$3/p=$4
- verdict: KILLED

## [s2] the swap flips via guard operand identity, init order, or loop stmt order
- mechanism: cse.c make_regs_eqv canonicalizes the zero-equivalence class to its LONGEST-LIVED member; sum's last use is the final subu so sum is always canonical and absorbs the guard slt ref regardless of which zero-var is named
- probe: sweep_s2c.py (10 spellings) + csewho.sh lreg stats: sum always 11 refs
- result: all spellings charge sum; swap invariant
- verdict: KILLED

## [s2] p's priority can be boosted (or sum's length grown) by address-temp / split-add respellings
- mechanism: extra flow-time refs/insns that combine later folds would shift allocno_compare priorities (flow refs are loop-weighted x2)
- probe: sweep_s2d.py (tp=p+0x18 temps), sweep_s2e.py (split addiu chains, sub-sign and le-minus-1 guards) with per-variant priority calculator
- result: cse folds every arithmetic respelling BEFORE flow counts (tp never exists at flow; chains merge; dead inits deleted); zero priority delta in all vars=8 variants
- verdict: KILLED

## [s2] a second combine-foldable guard test adds +2 sum live_length without byte drift
- mechanism: two slt+branch pairs in the guard region would push sum to 11/17=19411 < p 20000
- probe: sweep_s2f.py: nested/&& second guards (a1+1<=a2, a2>=1, a2!=a1, reversed &&)
- result: same-expr dedups (cse); different-expr seconds either emit a second blez (jump2 never dedups identical branches, n=22) or land in a separate bb where combine lacks LOG_LINKS to the zero def (slt/beq survive, n=20, vars=0) -- but that variant's sum 11/17 DOES allocate sum=$4/p=$3, empirically confirming the flip threshold
- verdict: KILLED
