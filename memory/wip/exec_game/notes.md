# exec_game - WIP checkpoint (2026-08-05, sessions 3-5)

**Honest pure-C distance 121 -> 25, insn-exact (194/194).** Three banked states:

| file | score | insns | holders? |
|---|---|---|---|
| `candidate.diff` | 30 (31 at 194/194) | 193 / 194 | **none - clean** |
| `candidate_holder2_25.diff` | **25** | **194** | phase-2 sentinel only |
| `candidate_maskholder_20.diff` | 20 | 192 | + phase-1 addrmask |

NOT applied to the tree - the 104 regfix rules no longer repair this codegen.
ASM-SUSPECT, frame-less both sides (`.frame $sp,0`, no frame lever). Instruments
in `tmp/csz/`: `d.sh exec_game main`, `align.py`, `eg_grid.sh` (quiet score),
`eg_ps.py` + `eg_pair.py` (pseudo <-> role <-> hard reg), `eg_state.py` (the
three residual pairs), `eg_probe.sh <driver> <variant>` (score AND pairs
together - a variant can flip a pair while the score moves for other reasons),
`eg_perturb.sh <spec>`. `--pairs --greedy` on the combined six-entry spec locks
5/6 via a five-atom stack and stalls on 104: direction, not a spelling.

## What carried it: VARIABLE IDENTITY, not spelling

`simulate.py` reproduced our allocation exactly (28/28, later 36/36), so the model
was trustworthy; the role map then showed our `base` was ONE pseudo (livelen 103,
`$t3` in every phase) against a target using a different register per phase - only
reachable if the original had per-phase variables. Applying the catalog law
*conflict edges = variable identity* to every reused local produced the drop:

| change | score | insns |
|---|---|---|
| session-2 bank | 70 | 195 |
| `base` -> `base1/3/4/5` | 73 | 195 |
| `p` -> `p1/p2/p4` | 67 | 195 |
| **delete the 11 constant-holder locals, literals inline** | **60** | 195 |
| `cnt` -> `cnt2/3/4/5` | **36** | 196 |
| `v` -> `v2/v5` | 33 | 196 |
| `i = 0;` hoisted ABOVE the phase-1 and phase-3 guards | **31** | **194** |
| same hoist for phase 2 | 30 | 193 |

Splitting `base` alone is worse (73); it only pays once `p` splits too. Splitting
`i` (74) and `j` (58) are KILLS - genuinely shared, matching target's single `i`
across `D_800A2D3C = i` in phases 4/5. The `i = 0;` hoists fill `bltz` delay
slots we left as `nop`: reorg fills only from the block BEFORE the branch.

## The three pairs (`eg_pair.py`) - refs equal, so live length decides

| cluster | pseudos (ours -> target) | refs | live | perturb |
|---|---|---|---|---|
| 1 | 104 sentinel1 `$t3`->`$t2`, 112 addrmask1 `$t2`->`$t3` | 5, 5 | **36, 35** | 1 atom |
| 2 | 89 cnt2 `$a1`->`$a0`, 140 sentinel2 `$a0`->`$a1` | 3, 3 | **10, 8** | 1 atom |
| 4 | 76 `$a2`->`$a0`, 80 `$a0`->`$a2` | 3, **12** | 14, 21 | **none** |

Refs are equal in clusters 1 and 2, so `allocno_compare` is decided by live
length alone and the shorter-lived value picks first (35 beats 36 by ONE slot).
Cluster 4 is out of perturb's space - pseudo 80's 12 refs win by a wide margin,
and target emits that copy in the opposite direction, so `base4`/`p4` are
decomposed differently in the original.

## Round 5: both near-ties SOLVED by the LICM preheader-placement law

A hoisted invariant lands only AFTER the preheader's source statements, so it is
born late and lives short; making it source-level moves its birth earlier,
lengthening it and demoting the value it was beating:

| state | change | score | insns | pair result |
|---|---|---|---|---|
| s31 | clean baseline | 31 | 194 | all three wrong |
| **s25** | phase-2 sentinel holder, assigned BEFORE `cnt2 = v2` | **25** | **194** | phase 2 MATCHES (`a0 <- v0`) |
| s20 | + phase-1 addrmask holder | 20 | **192** | phase-1 constants MATCH |

Placement proves the mechanism: the same holder assigned AFTER `cnt2 = v2` is
inert (30). **s20 is NOT a landing candidate**: the phase-1 mask holder costs the mask's own
`lui`/`ori` materialisation, leaving 192 insns against 194 (align idx 7-8).
Score can prefer a shorter mis-aligned stream; insn-exactness is harder. s25 is
the best insn-exact state.

**Both holders are the [[named-local-fake-exception]] construct and NEITHER is
committed.** Layer-2's question: *is a phase-2-only constant holder acceptable,
given target's `move a0,v0` cannot coexist with a late-born constant under the
LICM law?* `dropv2`/`guardcnt` both DELETE that copy, so dropping the `v2`/`cnt2`
split is not an escape.

## Measured kills

- Phase 1: equality operand swap (67), scan as a real `while` (37), `q = p1;`
  (38), `0x2FFFFFFF == p1[0]` (inert), store-after-accumulate (41), key hoisted
  above/after the scan loop (54/46), masked `ival` in the outer test (59) - each
  hoist carries a value live across the scan loop that target does not; five
  named-intermediate spellings of the merge condition 42-48, none flips the pair.
  `p1 = (j)*2 + base1;` inert - GCC canonicalizes the pointer PLUS chain, so the
  `subst "addu $3,$8,$2"` rules are NOT source-reachable.
- Phase 2, five spellings, none moves the pair: drop `v2` / guard on `cnt2`;
  `p2 = ...` before `cnt2 = v2` (32); `i <= cnt2`; `p2 += 2; i++;`; `cnt2 = v2`
  above the guard. `i = 0;` after `base = ...` but inside the guard: inert - the
  before-the-branch position is what matters, not statement order.
- Per-phase holders individually from s25: `s1` 25, `f1` 26, `m1+s1` 22, `m1+f1`
  25, `m1+m3` 31, `m1+d3` 21. Only `m1` (20) flips phase 1.

## Resume here

1. **Layer-2 must rule on the phase-2 holder** - the only known route to target's
   `move a0,v0`, insn-exact, worth 6 points.
2. Phase 1 needs a mask-lengthening spelling that does NOT delete the mask's
   materialisation (the holder does, -2 insns).
3. `tools/sched_solver` is unrun on the MOVED pairs (phase 3's `move t0,t2` 3
   slots late; phase 4's load 2 early, `addiu t1,t1,1` 2 late).
4. Cluster 4 needs a different `base4`/`p4` decomposition, not a perturbation.

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
   source-level is the LEVER that lengthens its live range and flips an
   equal-refs allocno tie. Check the cost: it can delete its own materialisation.
