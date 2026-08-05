# exec_game - WIP checkpoint (2026-08-05, sessions 3-5)

**Honest pure-C distance 121 -> 25, insn-exact (194/194).** Three banked states:

| file | score | insns | holders? |
|---|---|---|---|
| `candidate.diff` | 30 (31 at 194/194) | 193 / 194 | **none - clean** |
| `candidate_holder2_25.diff` | **25** | **194** | phase-2 sentinel only |
| `candidate_maskholder_20.diff` | 20 | 192 | + phase-1 addrmask |

NOT applied to the tree - the 104 regfix rules no longer repair this codegen.
ASM-SUSPECT, frame-less both sides (`.frame $sp,0`, no frame lever). Instruments
in `tmp/csz/`: `d.sh`, `align.py`, `eg_grid.sh` (quiet score), `eg_ps.py` +
`eg_pair.py` (pseudo <-> role <-> hard reg), `eg_state.py` (the three residual
pairs), `eg_probe.sh <driver> <variant>` (score AND pairs together - a variant
can flip a pair while the score moves for other reasons), `eg_perturb.sh`.
`--pairs --greedy` on the six-entry spec locks 5/6 then stalls on 104.

## What carried it: VARIABLE IDENTITY, not spelling

`simulate.py` reproduced our allocation exactly (28/28, later 36/36), so the model
was trustworthy; the role map then showed our `base` was ONE pseudo (livelen 103,
`$t3` everywhere) against a target using a different register per phase - only
reachable if the original had per-phase variables. Applying *conflict edges =
variable identity* to every reused local produced the drop:
`base` -> `base1/3/4/5` 73, `p` -> `p1/p2/p4` 67, deleting the 11 constant-holder
locals for literals **60**, `cnt` -> `cnt2/3/4/5` **36**, `v` -> `v2/v5` 33,
`i = 0;` hoisted above the phase-1/3 guards **31 at 194/194**, above phase 2's
30 (193). (Session-2 bank: 70 at 195.)

Splitting `base` alone is worse; it pays only once `p` splits too. Splitting `i`
(74) and `j` (58) are KILLS - genuinely shared, matching target's single `i`
across `D_800A2D3C = i` in phases 4/5. The `i = 0;` hoists fill `bltz` delay
slots we left as `nop`: reorg fills only from the block BEFORE the branch.

## The three pairs (`eg_pair.py`) - refs equal, so live length decides

| cluster | pseudos (ours -> target) | refs | live | perturb |
|---|---|---|---|---|
| 1 | 104 sentinel1 `$t3`->`$t2`, 112 addrmask1 `$t2`->`$t3` | 5, 5 | **36, 35** | 1 atom |
| 2 | 89 cnt2 `$a1`->`$a0`, 140 sentinel2 `$a0`->`$a1` | 3, 3 | **10, 8** | 1 atom |
| 4 | 76 `$a2`->`$a0`, 80 `$a0`->`$a2` | 3, **12** | 14, 21 | **none** |

With refs equal, `allocno_compare` is decided by live length alone and the
shorter-lived value picks first - 35 beats 36 by ONE slot. Cluster 4 is out of
perturb's space (pseudo 80's 12 refs win big); round 6 bounds it.
## Round 5: both near-ties SOLVED by the LICM preheader-placement law

A hoisted invariant lands only AFTER the preheader's source statements, so it is
born late and lives short; making it source-level moves its birth earlier,
lengthening it and demoting the value it was beating. From s31 (31 @ 194, all
three pairs wrong): a phase-2 sentinel holder assigned BEFORE `cnt2 = v2` gives
**s25 = 25 @ 194 with phase 2 MATCHING** (`a0 <- v0`); adding a phase-1 addrmask
holder gives s20 = 20 but at 192 insns, with phase 1's constants matching.

Placement proves the mechanism: the same holder assigned AFTER `cnt2 = v2` is
inert (30). **s20 is NOT a landing candidate** - the phase-1 mask holder costs
the mask's own `lui`/`ori`, leaving 192 insns against 194; score can prefer a
shorter mis-aligned stream, insn-exactness is harder.

**Both holders are the [[named-local-fake-exception]] construct and NEITHER is
committed.** Layer-2's question: *is a phase-2-only constant holder acceptable,
given target's `move a0,v0` cannot coexist with a late-born constant under the
LICM law?* `dropv2`/`guardcnt` both DELETE that copy - not an escape.

## Round 6: cluster 4 bounded; sched_solver's goal mapper is UNUSABLE here

Phase 4 uses the SAME registers on both sides - walker `$a0`, base `$a2`. The
residual is only the copy DIRECTION (target `move a0,a2`: the load defines the
BASE, the walker is the copy; ours is the reverse), `addu v0,v0,a2` operand
order, and the `D_800A2D40` load 2 slots early. Four decompositions are ALL
inert at 25: `p4 = load; base4 = p4;`, two independent loads (CSE re-merges
them), swapping which variable walks, `e = (cnt4)*2 + base4` - GCC canonicalizes
the copy direction, so it is not source-reachable by naming. Target's constants
precede the load but this is NOT the LICM law: phase-4 holders are worse and
delete insns (`d4` 26@193, `s4`/`d4+s4` 28@192). It is scheduling.

**`sched_solver` cannot adjudicate it.** The model is fine (36 blocks exact,
parity=True) and only 2 differ, but `--goal-from-target` reconstructs the target
by applying `regfix`/`asmfix` to OUR asm - and these 104 rules were written
against the pre-split source, so the goal is invalid (`hon->tgt` 16 replace / 16
delete) and both blocks are SKIPPED as "not a topological order".

## Measured kills

- Phase 1: equality operand swap (67), scan as `while` (37), `q = p1;` (38),
  store-after-accumulate (41), key hoisted above/after the scan loop (54/46),
  masked `ival` in the outer test (59) - every hoist carries a value live across
  the scan loop that target does not; five named-intermediate merge-condition
  spellings 42-48, none flips the pair. `p1 = (j)*2 + base1;` inert - GCC
  canonicalizes the pointer PLUS chain, so the `subst "addu $3,$8,$2"` rules are
  NOT source-reachable.
- Per-phase holders from s25: `s1` 25, `f1` 26, `m1+s1` 22, `m1+f1` 25, `m1+m3`
  31, `m1+d3` 21; only `m1` (20) flips phase 1.

## Resume here

1. **Layer-2 must rule on the phase-2 holder** - the only known route to target's
   `move a0,v0`, insn-exact, worth 6 points.
2. Phase 1 needs a mask-lengthening spelling that does NOT delete the mask's
   materialisation (the holder does, -2 insns).
3. Cluster 4 is UNSOLVED but bounded (round 6); the MOVED pairs need a
   rule-independent goal source before `sched_solver` can help.

## Reusable findings (rule-worthy; layer-2 + owner sign-off before registering)

1. *Reused-local splitting is the primary RA lever when target varies a register
   across a region the source shares.* Read it off the role map, not the score.
   It costs no FAKE construct - it is ordinary C - and was worth 40 points here.
2. *Constant holders are per-phase decisions, not a set, and can be an artifact
   of an unsplit pointer.* Re-test each individually after every identity split;
   here the set inverted from +17 to -7 while two members stayed load-bearing.
3. *An unfilled conditional-branch delay slot where target holds a loop-counter
   init means that init was OUTSIDE the guard in the original.*
4. *The LICM preheader-placement law is a two-way instrument* - making a constant
   source-level LENGTHENS its live range and flips an equal-refs allocno tie.
   Cost check: it can delete its own materialisation.
5. *`sched_solver --goal-from-target` is only valid while a function's regfix
   rules still describe its current source* - after a restructuring they
   reconstruct a bogus target and every block is skipped.
