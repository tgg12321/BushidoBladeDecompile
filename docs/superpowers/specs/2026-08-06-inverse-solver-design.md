# Inverse solver (lever synthesizer) — design

**Status:** prototype landed (`tools/ra_solver/inverse.py`, `tools/ra_solver/levers.py`),
RA side only, validated on one banked case.
**Date:** 2026-08-06 · Campaign 1 of the strategic plan.

## 1. What this is

The project has three validated FORWARD models of GCC 2.7.2's backend:

| model | scope | validation |
|---|---|---|
| `tools/ra_solver/simulate.py` | `global.c` — allocno priority sort, `prune_preferences`, `find_reg`, conflict propagation | 10/10 exact |
| `tools/ra_solver/local_alloc.py` | `local-alloc.c` `block_alloc` — `qty_compare` order + `find_free_reg` | order 94-98%, assign 86-93% across 5 TUs |
| `tools/sched_solver/simulate.py` | `sched.c` list scheduler | 6978/6978 |
| `tools/ra_solver/reload_sim.py` | reload's spill-retry (`retry_global_alloc`) | 194/194, closed-form |

They all answer the same question: **why did our build diverge from target here?**
The inverse solver answers the next one:

> Given (our allocation, target's allocation), which perturbation of the
> solver's INPUTS flips our outcome to target's — and which C technique from
> the catalog produces that perturbation?

Concretely it turns "the residual is a `$v0`/`$v1` swap" into "qty 0 must be
born at 6 instead of 4, **or** carry 4 references instead of 2 — here are the
catalog levers that do that, and here is the measured negative that kills the
obvious one."

### Why it is not just `perturb.py`

`tools/ra_solver/perturb.py` (Phase 4) already searches single/pair/greedy
perturbations of the GLOBAL model. It stays; the inverse solver supersedes it
on four axes:

1. **Typed perturbations.** Atoms carry a *perturbation class*, not a string.
   The class is what the lever mapping keys on.
2. **A cost model.** Ranking is `(atom count, plausibility cost)` — a
   birth-order swap is a cheaper C change than a class change, and the report
   says so.
3. **A policy-aware lever layer.** Every class maps to catalog techniques
   tagged PLAIN / SANCTIONED (with prerequisites attached) / FORBIDDEN
   (never suggested; printed only as a "do not reach for this" block).
4. **Local-alloc coverage.** The dominant residual class in the banked
   endgames is a two-quantity block decision, which the global model cannot
   see at all.

## 2. The perturbation space

Each atom perturbs exactly one input of one forward model. This is the full
enumerable space today.

### Global (`global.c`)

| class | perturbation | C meaning |
|---|---|---|
| `refs_up` / `refs_down` | `reg_n_refs` ±1..3 | a real extra/removed use of the value |
| `live_shrink` / `live_extend` | `reg_live_length` ±2/4/8 | statement span of the variable |
| `birth_order` | swap two allocnos in the priority sort's tie-break | first-definition order (allocno index = pseudo number = birth order) |
| `conflict_add` / `conflict_drop` | one conflict-graph edge | variable identity (split removes, reuse adds) |
| `pref_add` / `pref_reroute` / `pref_clear` | `hard_reg_preferences` / `hard_reg_copy_preferences` | copy relationships: argument flow, return capture, aliases |
| `calls_crossed` | 0 ↔ nonzero | whether the value survives a call (caller- vs callee-saved decision) |
| `class_change` | preferred register class | RTL idiom (MD_REGS from mult/div/`mulhi`) |

### Local (`block_alloc`)

| class | perturbation | C meaning |
|---|---|---|
| `refs_up` / `refs_down` | qty `refs` | as above, per block |
| `live_shrink` / `live_extend` | qty `birth` / `death` (each enumerated exhaustively over the block's index range, not by fixed deltas) | where in the block the value is defined and last used |
| `hardreg_live` | a hard register occupied across a span | a real argument / return value genuinely live there |
| `extra_qty` | one more competing quantity with a given priority and span | one more real named intermediate |
| `alloc_order` | the allocation order forced directly, with no input change | **deliberately worst-ranked** — see §4 |

### Deliberately NOT in the space

- Anything a regfix/asmfix rule would express. The engine's sandbox strips
  those before scoring; they are inert by construction.
- Dependence edges on the scheduler side that only `volatile` or a memory
  clobber could create. Those map to forbidden levers, so synthesizing them
  would be synthesizing a cheat. When the sched backend lands, added
  dependence edges map only to legitimate *ordering* restructures
  (`loop-exit-work-inside-loop-sched-fence`, `store-before-jal`,
  `defer-store-past-later-compute-into-jal-delay`) — never to a barrier.

## 3. The search

Breadth-first over combination size: all singles, then all pairs, stopping at
the first depth that yields a hit — so the reported vectors are *minimal by
construction*, and within a depth they are ordered by summed plausibility
cost. Duplicate vectors (same atom set in a different order) are collapsed.

Atom generation is focused, not global: for the global backend the focus set
is the constrained pseudos **plus everyone they conflict with**, because a
flip is produced either by moving the pseudo itself or by moving whoever is
sitting in the register it wants.

Composition rules: overrides merge per-unit; two order-forcing atoms in one
combination are rejected (not composable).

## 4. Negative results are first-class

If no combination up to the search depth reaches the goal, the tool prints a
NEGATIVE RESULT block, not a failure. It means the flip is not produced by
refs / span / birth / conflicts / preferences / calls-crossed at all, so **no
C spelling that only moves those will ever close it**, and the next move is
instrumentation rather than another spelling search. The named candidates are
the known model gaps: the local-alloc suggested-register pass
(`qty_phys_copy_sugg` / `qty_phys_sugg`, reported-not-scored today), `qty_size`
for DImode quantities, and reload's spill-retry.

The `alloc_order` atom exists to make this measurable rather than binary: if a
goal is reachable *only* by forcing the order directly, the answer is
"reachable in the allocator, not reachable from any modelled C-visible input" —
a strictly more informative negative.

## 5. The lever-mapping layer (`levers.py`)

Class → candidate C techniques from `.claude/rules/`, in three tiers:

- **PLAIN** — ordinary C restructuring, normal review bar.
- **SANCTIONED** — a narrow carve-out with strict prerequisites (documented
  lever exhaustion, `/* FAKE */` annotation, layer-1 + layer-2 cheat-reviewer).
  Emitted with its prerequisites attached, never as the first suggestion.
- **FORBIDDEN** — never emitted as a suggestion. Printed in a standing
  "NEVER" block on every report, because these are exactly the constructs an
  agent reaches for when the honest lever looks unreachable: register-asm
  pins, hardcoded-`$N` `__asm__`, `asm("Sym")` alias renames, lost-codegen
  regfix inserts, dead-variable frame coercion, volatile-as-coercion, and the
  archived tombstones.

The mapping also carries **measured negatives** — facts that kill a
plausible-looking lever choice, so the same negative is not re-derived:

- A dead store is inert for a `reg_n_refs` lift: `flow.c` deletes it *before*
  the references are counted. It is both a cheat and ineffective.
- An added preference numerically *above* an existing one is inert:
  `find_reg` takes the lowest preferred register. Preferences must be
  **rerouted**, not added. (The tool distinguishes the two cases: with no
  existing preference, acquiring one is a genuine `pref_add`.)
- Local mode carries a standing caveat: a birth/span perturbation is a claim
  about **alloc-time** order, and alloc-time order is not known to equal final
  **emission** order — camera_set_zoom variant B produced target's emission
  order and the assignment did not flip. Birth/span vectors are therefore
  reported as necessary, not sufficient.

## 6. Validation — camera_set_zoom, block 41

The banked case (`memory/wip/camera_set_zoom/notes.md`, floor 3 of 48). The
residual is three instructions:

```
T: lw v0,0(s0)     O: lw v1,0(s0)
T: li v1,2         O: li v0,2
T: sh v1,646(v0)   O: sh v0,646(v1)
```

Local-alloc block 41 holds two quantities — qty 1 (the constant `2`, born 6,
span 2, refs 2, priority 10000, gets `$v0`) and qty 0 (the owner pointer, born
4, span 4, refs 2, priority 5000, gets `$v1`). Target wants them exchanged.

```
python3 tools/ra_solver/inverse.py local tmp/ra_solver_work/code6cac.local.json \
        --func camera_set_zoom --block 41 --swap 0,1 --depth 1
```

Twelve distinct single-atom vectors, minimal by construction. The two the
human derivation banked appear at **#1** and **#9**:

| rank | atom | banked spec |
|---|---|---|
| 1 | `live_shrink` qty 0 born later (4→6), span 4→2 | "the pointer's def is emitted **after** the constant's" |
| 9 | `refs_up` qty 0 refs 2→4 | "the pointer's refs ≥ 4 at span 4" |

Both sufficient conditions recovered independently, from the model alone, with
no access to the notes. The other ten vectors are new — in particular a whole
family the manual derivation did not enumerate, *lengthening the constant's*
range (`live_extend` on qty 1) rather than shortening the pointer's, which is
a different C restructure of the same block.

The `alloc_order` probe ranks last (#12), correctly reporting that the flip
*is* explicable from C-visible inputs and does not require the unmodelled
suggested-register pass.

**Honest counter-note.** Session 2 of the banked work measured variant B, which
produced the target emission order in this block, and the assignment did **not**
flip. So the top-ranked family is empirically dead *for this spelling attempt*
— which is exactly why the local-mode caveat is printed on every run. The
inverse solver's contribution here is that it recovers the full necessary-
condition set in seconds and shows nine untried vectors beyond the two the
manual session found; it does not by itself settle alloc-time vs emission
order. That gap is a hook extension (QTYDBG on the candidate), already named
as the resume point in the WIP notes.

A negative-path check is also validated: goal `{qty 0: $s0, qty 1: $s1}` is
correctly reported UNREACHABLE at depth 2 (the ascending scan cannot reach the
callee-saved bank while `$v0`/`$v1` are free), with the instrumentation
next-move block instead of a fabricated lever.

Global-mode smoke test on the same function (`--goal '{"74": 18, "73": 17}'`,
an `$s1`/`$s2` exchange) returns six single-atom vectors led by a
`live_shrink` on pseudo 73 and three preference reroutes.

## 7. Next phases (not built)

1. **Goal derivation from asm.** Today the target allocation is supplied
   (`--goal` / `--swap`). It can be derived the way `sched_solver/goalmap.py`
   derives target instruction order: align our `.s` against target's, read off
   the register substitution on each aligned pair, and invert through the
   dispositions to get `{pseudo: target hardreg}`. `goalmap.align()` already
   does the hard part (its second pass deliberately pairs leftover
   deletes/inserts, which is what a rename looks like to difflib).
2. **Scheduler backend.** Same `Atom` / search / lever structure over
   `sched_solver/simulate.py`, with the dependence-edge classes mapped only to
   legitimate ordering restructures.
3. **Reload backend.** Phase 6 established the retry outcome is closed-form
   (`got = min{r ∉ base ∪ forbidden ∪ ~class ∪ hard_conflicts}`) and that
   preferences are inert at retry — a small, exactly-invertible space:
   conflicts, forbidden set, class, calls-crossed.
4. **Cross-model composition.** A residual that is an RA flip *caused by* a
   scheduling difference needs both models in one search.

## 8. Files

- `tools/ra_solver/inverse.py` — backends, atom generation, search, CLI.
- `tools/ra_solver/levers.py` — perturbation class → C-lever mapping, policy
  tiers, forbidden families, measured negatives.

No existing solver file was modified.
