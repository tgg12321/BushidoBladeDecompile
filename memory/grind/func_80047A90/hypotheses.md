# Hypothesis ledger — func_80047A90

## Session 1 (recon) — tested

### H1: pins removable, structure is right — CONFIRMED
Statement: the pinned body's structure (pointer-walk both loops) is byte-structure-correct;
the 20 is purely a register permutation.
Probe: strip pins, sandbox. Result: 20, 84/84 insns, all diffs are renames. CONFIRMED.

### H2: original loop 1 was array-indexed for-loop (loop.c produces the pointers) — CONFIRMED
Mechanism: LICM hoists Judge base (late pseudo → low alloc pressure), strength reduction
creates the walking givs, biv keeps slti.
Probe: full nested-for version. Result: loop 1 matched exactly incl. counter→t0. CONFIRMED.

### H3: original loop 2 was ALSO array-indexed for-for — KILLED
Probe: same full nested-for version. Result: 41 (87 insns) — LICM hoisted 0x7D0/0x66666667/8
which target keeps inline; sll-index addressing instead of pointers. Loop 2 in the original
was compiled WITHOUT loop notes ⇒ goto-form source. KILLED (do not re-propose for-loops or
do-while for loop 2 — do{}while(0..) also emits loop notes).

### H4: counter-init-last order sinks pt1/pt2 below a3off — CONFIRMED (floor 20→14)
Probe: `pt2; pt1; k = 1;` vs `k; pt2; pt1` (21) vs `pt2; k; pt1` (18). Result: 14 with
pt1→t1 pt2→t2 pt3→t3 pa1→a1 pa2→a2 all target. CONFIRMED, load-bearing.

### H5: both counters are one variable (target t0 in both loops) — KILLED as spelled
Probe: merged i, order pt2;pt1;i=1. Result: 17. Also merged + pt3=pa1+0x11: 17.
KILLED for these spellings — but the CONFLICT argument still favors a merged counter
(only a merged counter conflicts with BOTH a2/a3 holders forcing t0 in both loops);
the failure is priority (merged i ~11-12 refs → allocated first → a2). A merged-counter
retry must FIRST shave i's refs to ≤7 (floor_log2 drops to 2 ⇒ prio .222 < a3off .276
< pa2 .286). Do not re-run the naive merged form.

## Live frontier (for session 2)

### F1 (primary): shave ref-counts so alloc order becomes pa2 > a3off > [pt1,pt2 sunk] > counter(s)
Mechanism: global.c allocno_compare prio = floor_log2(refs)*refs/live_length. Exact current
numbers in evidence.md. The two remaining 2-cycles both need "counter allocated last".
Loop 2: k (5 refs/36 = .278) vs a3off (4/29 = .276) — shave ONE k ref or ONE k live-insn
worth of margin: candidate spellings — `k = 8` compare via a temp? `if (k == 8)` is required
semantics; try `s32 last = 8;`-free forms like comparing a3 (`a3 == 0x1C`?? NO — target
compares k reg vs 8: addiu v0,zero,8; bne t0,v0). Try: k as s16? (narrow type changes refs? no —
same). Try moving `k++` earlier/later in tail (live-length ±1-2 insns flips .278 vs .276).
Loop 1: i (7/25 = .56) must fall below judge (.12) — impossible by refs alone with 4+ refs;
instead make i live LONGER (merged counter: 7 refs/63 = .222) — see F2.

### F2 (the closing shape, likely): merged counter with exactly ≤7 total refs
i refs today when merged: init0, <17 test, ++ (loop1) + =1, ==8, ++, <9 (loop2) ≈ 11-12
(each ++ counts set+use). To reach ≤7 counted refs the loop-1 side must lose refs to
strength reduction: loop 1's biv test `i < 17` could be replaced by giv test if i were
otherwise unused — but target KEEPS slti on the counter, so i must keep exactly those refs.
Alternative: recount what lreg reports for the merged form (session 1 never dumped it —
FIRST ACTION session 2: rebuild merged form, run greg.sh, read pseudo numbers instead of
guessing). Then tune ++/test spellings (e.g. `i += 1` vs `i++`, test `i < 9` vs `i != 9`)
to hit the order pa2 > a3off > i, with pt1/pt2 sunk (init before k? — measured 14-winner
order pt2;pt1;counter-last already sinks them).

### F3 (fallback): permuter campaign seeded from the 14-floor form
The space is now small and pure-rename: permuter with register-diff scoring from the
14 base, sweeping statement order / counter spellings / init placements. Use a clean
single-function target.o (see difficult-is-not-impossible §3). Only after F1/F2 measured.

## [s1] The inherited pinned body's pointer-walk structure is byte-structure-correct; the distance is purely a register permutation
- mechanism: sandbox strips the register-asm pins; 84/84 insns match, all diffs are renames (4-cycle: counter a2->t0, judge-base a3->a2, pt1 t0->t1, a3off t1->a3)
- probe: strip all 10 pins, sandbox --disable all
- result: 20, 84/84, renames only
- verdict: CONFIRMED

## [s1] Original loop 1 was an array-indexed for-loop; the walking pointers in target are loop.c strength-reduction givs, and the Judge base is a LICM-hoisted pseudo
- mechanism: loop.c creates the givs late (low pseudo pressure) and LICM hoists the invariant base; biv keeps the slti counter test
- probe: full nested-for version, sandbox + insn diff
- result: loop 1 matched target exactly including counter->t0
- verdict: CONFIRMED

## [s1] Original loop 2 was ALSO an array-indexed for-loop
- mechanism: if loop.c saw loop 2 it would LICM-hoist the 0x7D0 / 0x66666667 / 8 constants; target keeps them inline in the inner loop
- probe: same full nested-for version
- result: 41 (87 insns): constants hoisted, sll-index addressing - loop 2 must be goto-form (no loop notes), source-level pointer walk
- verdict: KILLED

## [s1] Counter-init-LAST statement order (pt2; pt1; k=1;) sinks pt1/pt2 allocno priority below a3off, landing pt1/pt2/pt3/pa1/pa2 on target registers
- mechanism: global.c allocno_compare prio = floor_log2(refs)*refs/live_length; earlier pointer inits lengthen pt1/pt2 live ranges (37/38 insns) dropping them to .270/.263, below a3off .276
- probe: three orderings measured: k-first=21, k-middle=18, k-last=14
- result: floor 14 with pa1->a1 pa2->a2 pt1->t1 pt2->t2 pt3->t3 all target
- verdict: CONFIRMED

## [s1] Both loops' counters are one merged variable (target uses t0 in both loops)
- mechanism: only a merged counter conflicts with both loops' a2/a3 holders, forcing it into t0 twice - but merged refs ~11-12 give it TOP priority so it allocates first and takes a2
- probe: merged i, winner init order; also merged + pt3=pa1+0x11
- result: 17 both - naive merge killed; merge-with-refs-shaved-to-<=7 remains the F2 frontier
- verdict: KILLED
