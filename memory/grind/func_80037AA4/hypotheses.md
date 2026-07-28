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
