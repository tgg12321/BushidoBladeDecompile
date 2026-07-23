# Hypothesis ledger — func_800692C0

## Live frontier (from s1 recon; ordered)

1. **greg-dump read of the sum/bitpos allocno tie** (mechanism: local-alloc.c
   qty priority = n_refs/live_length; only these 2 allocnos disagree with target).
   Probe: `cc1 <flags> -da base.i`, read ";; Register dispositions" + qty ordering
   for the sum/bitpos pseudos; find the exact n_refs / live_length delta driving
   ours to assign sum→$t1 instead of $t2. Map = destination.
2. **Live-range restructure to flip the tie** (mechanism: sum is return value =>
   longest live range => lower priority => higher reg in ours; target gives sum
   the higher reg $t2, so the goal is to KEEP sum lower-priority than bitpos OR
   flip which one dies first). Probe: introduce an explicit return temp that ends
   sum's live range before bitpos's final `+=0x10` (e.g. accumulate then
   `ret=sum; return ret;` placed to shorten/lengthen the right range); measure
   each. UNTRIED — prior work permuted decl/init ORDER only, never live-range span.
3. **Permuter with hand-trimmed single-fn target** (mechanism: prior import pulled
   sibling func_8004A348 inline-asm => maspsx "too many values to unpack"). Probe:
   build target.o from asm/funcs/func_800692C0.s + prelude.inc at offset 0 (per
   difficult-is-not-impossible §3), seed from candidate.c, random + directed PERM,
   --stop-on-zero. Note the two root causes are COUPLED (li-slot + t1/t2 tie) —
   permuter is the tool for a coupled sched+RA fixpoint.

## [s1] The honest floor for func_800692C0 is 9 with the pin-free candidate.c form.
- mechanism: cheat-invisible sandbox strips the register-asm pins + 1 regfix reorder rule (355 cheat-asm bytes, 1 rule dropped) and scores the pure-C form.
- probe: canonical func_800692C0 (verdict=C, distance=9) + sandbox func_800692C0 --disable all (score=9, 67/67 insns) THIS session
- result: canonical distance=9, sandbox score=9
- verdict: CONFIRMED

## [s1] The 9 diffs reduce to exactly two coupled root causes, and only two allocnos (sum, bitpos) disagree with target.
- mechanism: Register-name diff isolation: target sum=$t2($10)/bitpos=$t1($9), ours reversed; every other allocno (i=$t3, a3_off=$t5, p=$t4, const1=$t6) and the whole diamond body are byte-identical. Second cause: LICM-hoisted `li t6,1` placed at preheader slot 4 (target) vs slot 8 (ours).
- probe: objdump of tmp/sandbox/func_800692C0/text1b.o instruction-aligned against asm/funcs/func_800692C0.s (artifact ours_disasm.txt)
- result: confirmed: sum/bitpos swap accounts for the 5 loop-body diffs + 1 prologue init; li-t6 displacement accounts for the prologue-slot diffs
- verdict: CONFIRMED

## [s1] No sibling/duplicate function provides a transplantable matched analog.
- mechanism: find_duplicates leads list has no entry for this function.
- probe: grep 800692C0 tmp/duplicates_leads.txt
- result: 0 hits
- verdict: CONFIRMED
