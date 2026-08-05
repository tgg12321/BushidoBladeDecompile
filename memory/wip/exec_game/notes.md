# exec_game — WIP checkpoint (2026-08-05, sessions 3-4)

**Honest pure-C distance 121 → 30; the insn stream is structurally exact.**
Candidates: `candidate.diff` (apply to `src/main.c`) = the 30 state; the
insn-exact 31 state is `tmp/csz/main.c.s31` (194/194 insns) and is the better
substrate for RA work because the streams align 1:1. NOT applied to the tree —
the 104 regfix rules no longer repair this codegen. ASM-SUSPECT, frame-less both
sides (`.frame $sp,0`; no frame lever). Instruments: `tmp/csz/d.sh exec_game main`,
`tmp/csz/align.py exec_game`, `tmp/csz/eg_grid.sh` (quiet score),
`tmp/csz/eg_ps.py` + `eg_pair.py` (pseudo ↔ role ↔ hard reg), `eg_perturb.sh`.

## What carried it: VARIABLE IDENTITY, not spelling

`simulate.py` reproduced our allocation exactly (28/28, later 36/36), so the model
was trustworthy; the role map then showed our `base` was ONE pseudo (livelen 103,
`$t3` in every phase) against a target using a different register per phase. That
is only reachable if the original had per-phase variables. Applying the catalog law
*conflict edges = variable identity* to every reused local produced the whole drop:

| change | score | insns |
|---|---|---|
| session-2 bank | 70 | 195 |
| `base` → `base1/3/4/5` | 73 | 195 |
| `p` → `p1/p2/p4` | 67 | 195 |
| **delete the 11 constant-holder locals, literals inline** | **60** | 195 |
| `cnt` → `cnt2/3/4/5` | **36** | 196 |
| `v` → `v2/v5` | 33 | 196 |
| `i = 0;` hoisted ABOVE the phase-1 and phase-3 guards | **31** | **194** |
| same hoist for phase 2 | **30** | 193 |

Splitting `base` alone is worse (73); it only pays once `p` splits too, because
target's phase-2 `p` and phase-4 `base` are different registers where we shared one
pseudo. Splitting `i` (74) and `j` (58) are KILLS — those are genuinely shared,
matching target's single `i` across `D_800A2D3C = i` in phases 4 and 5.

## The FLAGGED holder spelling is RETIRED — do not reinstate it (phases 1/3/5)

Session 2 banked 11 per-phase constant holders (`flagbit1`, `sentinel1`, …) as its
largest gain and flagged them for layer-2 under [[named-local-fake-exception]].
They were an artifact of the unsplit `base`/`p`: after the splits, deleting all 11
and writing the literals inline scores 60 vs 67. The candidate has no holders, no
FAKE annotations and no dead code. That layer-2 question is moot — **except for
phase 2**, see below.

## Delay slots: the last 2 excess insns

At 33 we carried 2 instructions target lacks, both a `nop` in a phase guard's
`bltz` delay slot where target fills it with `move t1,zero` (`i = 0`). reorg fills
only from the block BEFORE the branch, so the initializer belongs outside the
guard. Both slots fill → 194/194 exact at 31. (Phase-4's equivalent hoist is inert.)

## The residual 30/31 — THREE two-value swaps, mechanism measured

`eg_pair.py` reduces every RENAME line to three pairs:

| cluster | pseudos (ours → target) | refs | livelen | perturb |
|---|---|---|---|---|
| phase 1 | 104 sentinel1 `$t3`→`$t2`, 112 addrmask1 `$t2`→`$t3` | 5, 5 | **36, 35** | single atom |
| phase 2 | 89 cnt2 `$a1`→`$a0`, 140 sentinel2 `$a0`→`$a1` | 3, 3 | **10, 8** | single atom |
| phase 4 | 76 `$a2`→`$a0`, 80 `$a0`→`$a2` | 3, **12** | 14, 21 | **NO vector** |

**Clusters 1 and 2 are pure live-length near-ties.** Refs are equal within each
pair, so `allocno_compare` is decided by live length alone and the shorter-lived
value picks first — 35 beats 36 by ONE slot, 8 beats 10 by two. Every atom perturb
returns says the same thing (`sentinel refs+1` / `sentinel live-2` / `mask refs-1`
/ `mask live+2`): flip that near-tie.

**Cluster 4 is out of perturb's space** — pseudo 80 has 12 refs against 76's 3, so
it wins by a wide margin; target's assignment needs a refs-level structural change,
and target emits the copy in the opposite direction (`move a0,a2` vs our
`move a2,a0`), so the `base4`/`p4` roles are distributed differently in the
original. C-shape search, not the solver, is the tool there.

## Measured kills (beyond the splits above)

- Phase 1: hoisting the key `(base1[i*2] & 0x0FFFFFFF) + base1[i*2+1]` above the
  scan loop (**54**), after it (**46**), masked `ival` in the outer test (**59**).
  All far worse — each carries a value live across the scan loop that target does
  not. The two constants hoist to the same preheader in first-use order and the
  sentinel's first use (scan compare) is unavoidably before the mask's, so its
  birth cannot be pushed later for free.
- Phase 1 (earlier): equality operand swap (67), scan as a real `while` (37),
  `q = p1;` (38), `0x2FFFFFFF == p1[0]` (inert), store-after-accumulate (41),
  `p1 = (j)*2 + base1;` (inert — re-confirms GCC canonicalizes the pointer PLUS
  chain, so the `subst "addu $3,$8,$2"` rules are NOT source-reachable).
- Phase 2 (five spellings, none moves the pair): drop `v2` and guard on `cnt2`
  (31, inert — the copy is still emitted); `p2 = …` before `cnt2 = v2` (32);
  `i <= cnt2` (inert); `p2 += 2; i++;` body order (inert); `cnt2 = v2` above the
  guard (inert).
- `i = 0;` moved to after `base = …` but still inside the guard: inert. It is the
  before-the-branch position that matters, not statement order.

## Resume here

1. Phase 2's sentinel materialises BEFORE `move a0,v0` in target, which by
   session-2's finding 2 (LICM places hoisted invariants only AFTER the
   preheader's source statements) means that constant WAS source-level in the
   original. Exhaustion is now documented, so a **phase-2-only** named holder is
   the reviewable next step under [[named-local-fake-exception]] — layer-2's call,
   not the agent's. Phases 1/3/5 must NOT get one; it is measurably wrong there.
2. Phase 1 needs a spelling that shortens the sentinel's range or lengthens the
   mask's by ~2 RTL slots without carrying a value across the scan loop. Untried.
3. `tools/sched_solver` has not been run on the MOVED pairs (phase 3's `move
   t0,t2` 3 slots late; phase 4's load 2 slots early and `addiu t1,t1,1` 2 late).
4. Cluster 4 needs a different `base4`/`p4` decomposition, not a perturbation.

## Reusable findings (rule-worthy; layer-2 + owner sign-off before registering)

1. *Reused-local splitting is the primary RA lever when target varies a register
   across a region the source shares.* Read it off the role map, not the score:
   one long-livelen pseudo holding one register everywhere against a rotating
   target. It costs no FAKE construct — it is ordinary C — and was worth 40 points
   here where six sessions of spelling search had been worth 51.
2. *Constant-holder locals can be an ARTIFACT of an unsplit pointer.* Re-test any
   flagged holder after every identity split; here they inverted from +17 to −7.
3. *An unfilled conditional-branch delay slot where target holds a loop-counter
   init means the init was OUTSIDE the guard in the original.*
4. Session 2's finding 1 (struct-vs-scalar pointer typing is an aliasing lever,
   `true_dependence`/`MEM_IN_STRUCT_P`) and finding 2 (LICM preheader placement)
   both still stand and are cited above.
