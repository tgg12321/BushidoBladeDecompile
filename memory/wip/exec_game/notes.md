# exec_game — WIP checkpoint (current state 2026-08-05, session 3)

**Honest pure-C distance 70 → 30, and the insn stream is structurally exact
(194 target / 193-194 ours).** Candidate: `candidate.diff` (apply to `src/main.c`);
also banked verbatim as `tmp/csz/main.c.s30`. NOT applied to the tree — the 104
regfix rules no longer repair the new codegen. Verdict ASM-SUSPECT, frame-less both
sides (`.frame $sp,0`, no frame lever). Instruments: `wsl bash tmp/csz/d.sh exec_game main`,
`python tmp/csz/align.py exec_game`, `wsl bash tmp/csz/eg_grid.sh` (quiet score).

## The session's finding: VARIABLE IDENTITY was the whole residual

Session 2 closed at 70 with "60 RENAME + 12 MOVED, 0 structural — next: ra_solver on
the preheader allocnos." The solver's first output settled it: `simulate.py` reproduced
our allocation 28/28 exactly, so the model was trustworthy, and the role map
(`tmp/csz/eg_roles.py` → `.lreg`/`.greg` insn pairing) showed our `base` was ONE pseudo
(#74, livelen 103, `$t3` in every phase) while target used a DIFFERENT register per
phase. Target's per-phase registers are only reachable if the original had per-phase
variables. Applying the catalog law *conflict edges = variable identity* to every reused
local produced the whole drop.

| # | change | score | insns |
|---|---|---|---|
| 0 | session-2 bank (levers 1-6) | 70 | 195 |
| 1 | `base` → `base1/base3/base4/base5` | 73 | 195 |
| 2 | `p` → `p1/p2/p4` | 67 | 195 |
| 3 | **constant-holder locals DELETED, literals inline** | **60** | 195 |
| 4 | `cnt` → `cnt2/cnt3/cnt4/cnt5` | **36** | 196 |
| 5 | `v` → `v2/v5` | 33 | 196 |
| 6 | `i = 0;` hoisted ABOVE the phase-1 and phase-3 guards | **31** | **194** |
| 7 | same hoist for phase 2 | **30** | 193 |

Step 1 alone is worse (73) — the split only pays once `p` is split too, because target's
phase-2 `p` and phase-4 `base` are different registers and ours shared pseudo #78.

## Lever 6's FLAGGED spelling is RETIRED — do not reinstate it

Session 2 banked 11 per-phase constant-holder locals (`flagbit1`, `sentinel1`,
`addrmask1`, …) as the largest MOVED fix and flagged them for layer-2 under
[[named-local-fake-exception]]. **They were an artifact of the un-split `base`/`p`.**
Once steps 1-2 land, deleting all 11 and writing the literals inline scores 60 vs 67
— strictly better AND uncontroversial. The candidate contains no holders, no FAKE
annotations, no dead code: just per-phase locals and literals. The layer-2 question
session 2 raised is moot.

## Step 6/7: the two extra insns were unfilled `bltz` delay slots

At 33 our stream carried 2 instructions target does not have, both a `nop` in a phase
guard's `bltz` delay slot (align idx 4 and 78) where target fills the slot with
`move t1,zero` = `i = 0;`. reorg can only pull the initializer into the slot from the
block BEFORE the branch, so writing

```c
i = 0;
if (D_800A2D3C >= 0) { base1 = (s32 *)D_800A2D40; do { … } while (…); }
```

instead of putting `i = 0;` inside the guard fills both slots. Insn count became exact
(194/194) at 31. `i` stays a single shared pseudo — splitting `i` per phase is a
measured KILL (74), consistent with target keeping one `i` across `D_800A2D3C = i`
in phases 4 and 5. Hoisting phase 2's `i = 0` as well gives 30 at 193 insns (one
short) — 30 is the better honest score but 31 is the insn-exact state; both banked
(`main.c.s31`, `main.c.s30`).

## Measured kills this session

- Split `i` per phase: 74. Split `j` per phase: 58. Both much worse — genuinely shared.
- Phase-1 equality operand swap (`(base&mask)+w1 == q[0]&mask`): 67 (from 60).
- Phase-1 scan loop as a real `while`: 37. `q = p1;` instead of recomputing: 38.
  `0x2FFFFFFF == p1[0]`: inert. `q[0] = …` after the accumulate: 41.
  `p1 = (j)*2 + base1;` (source operand order): inert — re-confirms GCC canonicalizes
  the pointer PLUS chain, so the `subst "addu $3,$8,$2"` rules are not source-reachable.
- Moving `i = 0;` to AFTER `base = …` inside the guard: inert (33). It is the
  before-the-branch position that matters, not the statement order.
- Hoisting phase 4's `i = 0`: inert (31 → 31).

## The residual 30 — four small RA/scheduling clusters, all located

1. **Phase 1, 6 lines**: `0x2FFFFFFF` and `0x0FFFFFFF` are register-SWAPPED
   (target sentinel `$t2` / mask `$t3`; ours reversed). `perturb.py` on the current
   model gives single-atom solutions — pseudo 100 (sentinel) `refs+1` or `live-2/-4/-8`;
   pseudo 108 (mask) `refs-1`, `live+2/+4`, or `pref+r11`. Reproduce with
   `wsl bash tmp/csz/eg_perturb.sh '{"100": 10, "108": 11}'` (pseudo numbers from
   `tmp/csz/eg_ps.py`; re-derive after any source change). No natural spelling found
   yet that supplies one of those atoms — target also uses the mask twice, so `refs-1`
   is not honest; the live-range atoms are the untried surface.
2. **Phase 2, 5 lines**: `$a0`/`$a1` swapped on `cnt2 = v2`, plus target materializes
   the `0x2FFFFFFF` pair BEFORE `move a0,v0` while ours emits it after. Per session 2's
   finding #2 (LICM places hoisted invariants only AFTER the preheader's source
   statements) that ordering means phase 2's constant WAS source-level in the original —
   i.e. phase 2, unlike phases 1/3/5, may genuinely have had a named holder. Worth one
   targeted test (phase 2 only), and it would need layer-2 as a named-local.
3. **Phase 3, 1 pair**: `move t0,t2` sits 3 slots late.
4. **Phase 4, 4 lines**: the `D_800A2D40` load is 2 slots early, `move a0,a2` is
   reversed, `addu v0,v0,a2` operand order, `addiu t1,t1,1` 2 slots late.

`tools/sched_solver` has not been run on clusters 3/4 yet — that is the next step
alongside the phase-1 live-range atoms.

## Reusable findings (rule-worthy; layer-2 + owner sign-off before registering)

1. *Reused-local splitting is the primary RA lever when target varies a register
   across regions the source shares.* Read it off the role map, not the score: one
   pseudo with a long livelen holding one register everywhere, against a target that
   rotates. Splitting is free of any FAKE construct — it is ordinary C — and here it
   was worth 40 points where six sessions of spelling search had been worth 51.
2. *Constant-holder locals can be an ARTIFACT of an unsplit pointer.* Before flagging
   named-local holders for layer-2, re-test them after every identity split; here they
   inverted from +17 to −7.
3. *An unfilled conditional-branch delay slot where target holds a loop-counter init
   means the init was OUTSIDE the guard in the original.* reorg fills from the block
   before the branch only.
4. Session 2's finding 1 (struct-vs-scalar typing is an aliasing lever) and finding 2
   (LICM preheader placement) both still stand and are still cited above.
