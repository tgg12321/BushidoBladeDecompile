# exec_game - WIP checkpoint (2026-08-05, sessions 3-5)

**Honest pure-C distance 121 -> 25, insn-exact (194/194).**

| file | score | insns | holders? |
|---|---|---|---|
| **`candidate.diff`** (working) | **25** | **194** | phase-2 sentinel, `/* FAKE */` |
| `candidate_clean31.diff` | 31 | 194 | none |
| `candidate_maskholder_20.diff` | 20 | 192 | + phase-1 mask (NOT landable) |

Layer-2 ruled the phase-2 holder USABLE (2026-08-05): the FAIL was mechanical
only (missing annotation) and the construct fits
[[named-local-fake-exception]]. Annotated at the assignment and promoted to
`candidate.diff`; score unchanged at 25. Completion needs a fresh layer-2.

NOT applied to the tree - the 104 regfix rules no longer repair this codegen.
ASM-SUSPECT, frame-less both sides (`.frame $sp,0`). Instruments in `tmp/csz/`:
`d.sh`, `align.py`, `eg_grid.sh`, `eg_ps.py`+`eg_pair.py` (pseudo<->role<->reg),
`eg_state.py` (the three pairs), `eg_probe.sh <driver> <variant>` (score AND
pairs together), `eg_perturb.sh`, `eg_pin_head.sh` (re-pin the sched goal).

## What carried it: VARIABLE IDENTITY, not spelling

`simulate.py` reproduced our allocation exactly (28/28, later 36/36), so the
model was trustworthy; the role map then showed our `base` was ONE pseudo
(livelen 103, `$t3` everywhere) against a target using a different register per
phase - only reachable if the original had per-phase variables. Applying
*conflict edges = variable identity* to every reused local, from the session-2
bank of 70@195: `base` -> `base1/3/4/5` 73, `p` -> `p1/p2/p4` 67, deleting the 11
constant-holder locals for literals **60**, `cnt` -> `cnt2/3/4/5` **36**, `v` ->
`v2/v5` 33, `i = 0;` hoisted above the phase-1/3 guards **31 at 194/194**, above
phase 2's 30 (193). Splitting `base` alone is worse; it pays only once `p` splits
too. Splitting `i` (74) and `j` (58) are KILLS - genuinely shared, matching
target's single `i` across `D_800A2D3C = i` in phases 4/5. The `i = 0;` hoists
fill `bltz` delay slots we left as `nop`: reorg fills only from the block BEFORE
the branch.

## The three pairs (`eg_pair.py`) - refs equal, so live length decides

| cluster | pseudos (ours -> target) | refs | live | perturb |
|---|---|---|---|---|
| 1 | 104 sentinel1 `$t3`->`$t2`, 112 addrmask1 `$t2`->`$t3` | 5, 5 | **36, 35** | 1 atom |
| 2 | 89 cnt2 `$a1`->`$a0`, 140 sentinel2 `$a0`->`$a1` | 3, 3 | **10, 8** | 1 atom |
| 4 | 76 `$a2`->`$a0`, 80 `$a0`->`$a2` | 3, **12** | 14, 21 | **none** |

Refs are equal in 1 and 2, so `allocno_compare` turns on live length alone and the
shorter-lived value picks first - 35 beats 36 by ONE slot. Cluster 4 is out of
perturb's space (pseudo 80's 12 refs win by a wide margin).

## Clusters 1 + 2: the LICM preheader-placement law

A hoisted invariant lands only AFTER the preheader's source statements, so it is
born late and lives short; making it source-level moves its birth earlier and
demotes the value it was beating. From s31 (31 @ 194, all three pairs wrong): a
phase-2 sentinel holder assigned BEFORE `cnt2 = v2` gives **25 @ 194 with phase 2
MATCHING** (`a0 <- v0`); adding a phase-1 mask holder gives 20, but at 192. The
same holder assigned AFTER `cnt2 = v2` is inert (30), which proves the mechanism.
No cheaper phase-2 spelling exists - `dropv2`/`guardcnt` both DELETE the
`move a0,v0` copy target requires.

**The 20 state is NOT landable.** A full-text multiset diff (not difflib, which
mis-pairs the duplicate constant text) shows ours missing exactly one
`lui t3,0xfff` + `ori t3,t3,0xffff`: the named holder lets CSE SHARE phase 1's
mask with a later phase, so one of target's three materialisations vanishes.
More holders share more, not fewer (`m1+m3` 31@190).

## Cluster 4 bounded; the sched goal source

Phase 4 uses the SAME registers on both sides - walker `$a0`, base `$a2`. The
residual is only the copy DIRECTION (target `move a0,a2`: the load defines the
BASE, the walker is the copy; ours is the reverse), `addu v0,v0,a2` operand
order, and the `D_800A2D40` load 2 slots early. Four decompositions are ALL
inert at 25 (`p4 = load; base4 = p4;`, two independent loads which CSE re-merges,
swapping which variable walks, `e = (cnt4)*2 + base4`) - GCC canonicalizes the
copy direction, so naming cannot reach it. Phase-4 holders are worse and delete
insns (`d4` 26@193, `s4`/`d4+s4` 28@192): it is scheduling, not the LICM law.

**`sched_solver` works here.** Pin `<stem>.tgt.head.s` once from clean HEAD (its
docstring says so); pinning from an edited tree makes the rules reconstruct
fiction. Re-pinned via `eg_pin_head.sh`, `hon->tgt` goes 16/16 replace/delete ->
27/6 and block 20 of 36 yields vectors; only block 1 skips (duplicate text).
**`asm/funcs/exec_game.s` is NOT a drop-in goal source**: `.tgt.s` is cc1-level
text with unexpanded macros (174 insns, `lw $2,D_800A2D3C`) against splat's
expanded machine code (195, `lui`+`lw`, named regs; mnemonic agreement 14/174).

Block 20 is phase 3's inner preheader (goal `[644,642,270,663]`; 270 =
`icnt = D_800A2D3C;`, 663 = the LICM `move t0,t2`); all three vectors say "move
`icnt` earlier" and all cost more than the pair is worth - above the
`if (cnt3 >= j)` guard 27, above `j = i + 1;` 27, at the loop top 35@193.

## Measured kills

- Phase 1, all worse and none flips the pair: equality operand swap 67, scan as
  `while` 37, `q = p1;` 38, store-after-accumulate 41, key hoisted above/after the
  scan loop 54/46, masked `ival` in the outer test 59 (every hoist carries a value
  live across the scan loop target lacks), five named-intermediate
  merge-condition spellings 42-48; `p1 = (j)*2 + base1;` inert (GCC canonicalizes
  the pointer PLUS chain, so `subst "addu $3,$8,$2"` is dead).
- Per-phase holders from s25: `s1` 25, `f1` 26, `m1+s1` 22, `m1+f1` 25, `m1+m3` 31,
  `m1+d3` 21; only `m1` (20) flips phase 1.

## Resume here

1. Phase 1 needs a mask-lengthening spelling that does NOT create a named value
   later phases can CSE-share (that sharing costs the 2 insns). The one untried
   perturb atom is `112: pref+r11` - a copy relationship toward `$t3`.
2. Cluster 4: bounded, unsolved. Block 1 is the last skipped sched block.
3. A normalised `asm/funcs` goal source would make `sched_solver` robust against
   source drift for every function, not just this one.

## Reusable findings (rule-worthy; layer-2 + owner sign-off before registering)

1. *Reused-local splitting is the primary RA lever when target varies a register
   across a region the source shares* - read it off the role map, not the score.
   No FAKE construct, ordinary C, worth 40 points here.
2. *Constant holders are per-phase decisions, not a set* - and can be an artifact
   of an unsplit pointer; re-test each after every identity split.
3. *An unfilled conditional-branch delay slot where target holds a loop-counter
   init means that init was OUTSIDE the guard in the original.*
4. *The LICM preheader-placement law is a two-way instrument* - making a constant
   source-level LENGTHENS its live range and flips an equal-refs allocno tie;
   cost check, a named holder is CSE-shareable across later phases.
5. *Pin `sched_solver`'s `<stem>.tgt.head.s` from clean HEAD, once, before
   editing.* `asm/funcs/<f>.s` is rule-independent but machine-level: normalise.
6. *A scheduler vector is a hypothesis about ONE block* - all three of block 20's
   were reachable and all cost more elsewhere than the pair was worth.
7. *Use a full-text multiset diff, not difflib, to count insn deltas* - difflib
   mis-pairs duplicate constant text and invented a phantom deletion here.
