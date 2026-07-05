# marionation_Exec — HONEST FLOOR = masked 30; remaining gap is 100% REGISTER ALLOCATION (2026-07-05 handoff)

## TL;DR (read this first — the record was corrected this session)
The committed build's "masked 4" (mh5) was a **register-masked MIRAGE** built on
TWO unsanctioned cheats that a fresh cheat-reviewer FAILED. candidate.c is now
the **HONEST BASELINE = masked 30** (m2c-tail structure, cheats stripped to
natural C). Resume from candidate.c, NOT from the old mirage. The instruction
SHAPES are all free from natural C — **the entire remaining gap is the s-register
allocation** (a knife-edge allocno-priority rotation). No fold, no missing cheat.

## What was PROVEN this session (banked, don't re-derive)
1. **The FAKEs are register-web gaming, NOT symbol-fold defeat** (their comments
   lied). Proof (proto_mh5.py): natural `idx_1495 = 1 + idx_1494` emits the exact
   target `addiu s6,s2,1` (base+offset, no reloc) — identical to the FAKE. The
   FAKE only changes WHICH registers seat. Same for the iq3 `+=1;+=1;` double-
   split (ref-count inflation) and the F19C0 rebase. All three are the same cheat
   class; layer-1 reviewer FAILed them (meta.json.reviewer). DO NOT reintroduce.
2. **The original used explicit STORED pointers** idx_1494/1495/1496 (candidate
   structure is right). Proof: full m2c rebuild with field-style accesses
   `(&D_800A1494)[k]` REGRESSED to masked 65 (GCC re-materializes the base each
   use). m2c full-rebuild is a DEAD END. Target `addiu s2,1/2` off a held base
   confirms stored pointers.
3. **Honest floor = masked 30.** The masked-4/6 forms were all cheat-carrying.

## The register problem, fully quantified (ledger_full.py, honest baseline)
status(s0)/saved(s1)/i1495(s6) already seat CORRECT. The other 5 rotate:
```
       ours(honest)          target wants
  s2   arg1  r4 ll84  p952    i1494
  s3   i1494 r7 ll150 p933    i1496
  s4   i1496 r5 ll150 p666    arg1
  s5   arg0  r2 ll78  p256    tbl
  s7   tbl   r3 ll152 p197    arg0
```
Needed priority order (desc, pri=2*refs*1e4/livelen): i1494 > i1496 > arg1 > tbl
> i1495 > arg0.

## LEVER FOUND (honest, partial): arg1-hold flips the hard pair
`hold = a1;` at top + `hold` in BOTH copy arms (natural C, ~m2c's var_a1)
extends arg1's live range → arg1 drops below i1496 → **i1494→s2 ✓ and i1496→s3 ✓
flip correct** (honest_sweep2 W3). BUT it overshoots the bottom-3 and adds
register pressure (masked rises to 38 until the rest is fixed). It's a real
honest lever; it just isn't the whole answer.

## THE WALL (where the next session starts)
Bottom-3 are REVERSE-ordered: ours arg0(256)>i1495(202)>tbl(197); target needs
tbl>i1495>arg0. tbl is dead-last because its live range spans the whole timeout
loop (ll152). Can't shorten honestly — to stay call-saved tbl must cross a call;
the only pre-use calls are in the loop; crossing the loop = long ll. Moving the
load drops tbl to a t-reg (masked 37). Target has tbl long-lived too (s5), so the
resolution is that the ORIGINAL's arg0/i1495 had EVEN LOWER priority than ours —
i.e. the exact statement structure sets these livelens. Neither candidate-patch
nor m2c-rebuild reproduced it this session.

## NEXT SESSION — concrete plan
1. Apply candidate.c → `sandbox marionation_Exec --disable all` == 30 (confirm).
2. Apply the arg1-hold lever (see honest_sweep2.py W3) → confirm i1494/i1496 flip.
3. Attack the bottom-3: find natural code that raises tbl's priority above
   i1495/arg0 WITHOUT dropping tbl to a t-reg, OR lowers arg0/i1495 below tbl.
   Levers to try: arg0 use-site placement (extend its live range → lower pri);
   whether i1495 (the callback pointer) can live shorter; the || compound loop
   condition from m2c (may reshape livelens). ALL must be natural — no ref
   inflation, no cross-symbol arithmetic, no register pins.
4. End gate: masked 0 → retire all 42 rules → full SHA1 == oracle → dual
   adversarial review → queue done → delete WIP.

## Artifacts (this session's worktree was bb2-work-marproto, now removed)
Harness + full write-up copied to `tmp/marion_handoff/` (gitignored, on-machine):
FINDINGS.md, honest_baseline.py, honest_sweep{,2,3}.py, ledger_full.py,
proto_m2c_rebuild.py, proto_mh5.py. Pattern: splice candidate body into
src/system.c → engine sandbox → objdump tmp/sandbox vs build/src/system.o →
restore. Debug cc1 (ALLOCDBG ledger): tmp/gccdbg/cc1. rejected/ holds the three
dead forms (mh5 mirage, iq3 cross-symbol, v0-idx). Twin cpu_side_move_dir_4 has
the identical structure/problem — the same honest analysis applies.
