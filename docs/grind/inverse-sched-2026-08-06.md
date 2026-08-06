# Inverse solver — scheduler backend (Phase 4, 2026-08-06)

Tool: `tools/ra_solver/inverse_sched.py`. Policy layer: `tools/ra_solver/levers.py`
(extended with the scheduler classes). Nothing under `tools/sched_solver/` was
modified — its atom enumeration, priority recomputation and goal mapping are
imported and reused.

## What this adds over `sched_solver/perturb.py`

`perturb.py` already enumerates the atoms (`add_dep` / `del_dep` / `luid` /
`luid_move` / `cost`), recomputes `priority()` after each perturbation, and
searches to a goal order. That machinery is reused verbatim. Three things are
new:

1. **A policy layer, which matters more here than on the RA side.** The most
   obvious way to create a dependence edge is `volatile`, a `memory` clobber,
   or an `__asm__` barrier — all FORBIDDEN families. A search tool that reports
   "add an anti-dependence 55 <- 16" without saying so is one step away from a
   cheat. Every atom is mapped through `levers.py`; anti-edge atoms carry a
   standing warning that the edge is only real if the two accesses genuinely
   alias; the forbidden block prints on every report.
2. **A plausibility cost model.** A LUID move is an ordinary statement reorder
   (cost 1); an added true edge needs real value flow (3); an anti edge needs
   real aliasing (4); an `INSN_COST` change needs a different instruction to be
   selected (5). Ranking is (atom count, summed cost).
3. **Spellability triage.** Vectors are labelled SPELLABLE (LUID only) or
   CONDITIONAL (anything requiring a precondition about the program). Depth-2
   search seeds its first element from the spellable classes, which is what
   stops a truncated run from reporting only unspellable families — the failure
   mode `perturb.py`'s own docstring warns about.

**Honest-model note.** regfix/asmfix rules run AFTER cc1, so a function whose
only cheats are RULES already has an honest scheduler model on main. Only
cheat-asm in the C source (register pins, `__asm__` barriers) contaminates it;
for those, build from the stripped source first (`mkasm_honest.sh`).
`gnd_init_80041688` is rules-only (3 regfix), so `text1a.sched.json` is honest
as extracted.

---

## Case 1 — `gnd_init_80041688` (src/text1a.c), pass 1 block 18: **LEVER**

Park reason: "sched1 lbu emit-order (3 regfix)".

Goal derivation is clean: `hon -> tgt` alignment is 81 equal + **1 moved**, and
18 of the function's 19 blocks come back `GOAL == OURS (identity)`. The entire
residual is block 18, 5 slots.

The block builds a 24-bit value out of three byte loads and calls a loader:

| uid | instruction |
|---|---|
| 183 | `lbu $4,24($16)` |
| 187 | `lbu $2,25($16)` |
| 191 | `lbu $3,26($16)` |
| 193 | `sll $4,$4,16` |
| 194 | `sll $2,$2,8` |
| 195 | `or $4,$4,$2` |
| 198 | `or $4,$3,$4` |
| 200 | `jal gnd_load_tex` |

In emission order (the pick sequence reversed):

```
ours : 183 187 193 194 191 195 198 200     lbu24, lbu25, sll, sll, lbu26, ...
goal : 191 183 187 193 194 195 198 200     lbu26 FIRST, then lbu24, lbu25
```

Target issues the byte-26 load **first**; our build defers it until after the
two shifts. That is the classic three-parallel-loads shape.

**Four minimal vectors, all single-atom, all CONDITIONAL — and no SPELLABLE
vector exists at depth 1:**

| # | atom | class | cost |
|---|---|---|---|
| 1 | `add_dep 183 <- 191` (true/data) | `dep_add_true` | 3 |
| 2 | `add_dep 187 <- 191` (true/data) | `dep_add_true` | 3 |
| 3 | `add_dep 183 <- 191` (anti/output kind 14) | `dep_add_anti` | 4 |
| 4 | `cost 191 := 12` | `insn_cost` | 5 |

The ranking is informative in itself. Because LUID atoms cost 1 and are sorted
first, the absence of any cost-1 vector is a positive finding: **this residual
is not a statement reorder.** No amount of moving the three load statements
around produces target's order — the byte-26 load has to genuinely *precede*
the other two in the dependence graph.

Vectors 1 and 2 say exactly that: insn 183 (or 187) must depend on 191. The
lever mapping routes this to **`walking-pointer-serializes-parallel-loads`**,
whose recorded symptom is verbatim this shape — "memory-clobber barriers OR
per-load `register asm("$N")` pins between independent parallel-array element
loads → walk the array(s) with post-increment pointers (`*ap++`); the pointer
dependence serializes the loads". The rule was derived independently, on a
different function; the solver reached it from the model alone.

Vector 4 is correctly labelled unspellable by the cost model: `INSN_COST` 12 is
a multiply, and 191 is an `lbu`. It cannot be reached by any C edit that keeps
the same instruction, and the class note says so.

Vector 3 is the trap the policy layer exists for. An anti-dependence between
two `lbu`s from the same base is trivially spelled with `volatile` — and that
is forbidden. The report prints the standing warning: the edge is only real if
the accesses genuinely alias, and `legitimate-volatile-interrupt-touched` is a
narrow qualification of the SYMBOL (IRQ-touched, or type-level MMIO), never a
lever to reach for because an edge was wanted.

**Recommended next step:** rewrite the three byte reads as a post-increment
pointer walk over the source bytes, per the catalog rule, and re-derive. The
three regfix rules are the thing to retire.

---

## Case 2 — `func_80072CD4` (src/text1b.c), pass 2: **GOAL NOT DERIVABLE** (honest negative)

Park reason: "store-scheduling duplication outside duplicated-statement-into-arms
scope".

Two blocks differ (block 4: 16 slots; block 5: 10 slots), but `goalmap`'s own
topological check **rejects both goals**:

```
block 4: *** GOAL INVALID: 5 dependence violation(s)
         [(95,111,0), (98,111,14), (104,111,14), (111,113,0)]
block 5: *** GOAL INVALID: 3 dependence violation(s)
         [(176,194,15), (178,194,14), (194,196,0)]
```

A goal order must be a topological order of the block's LOG_LINKS — a
predecessor must precede its consumer in emission order. These are not, which
means the target alignment mis-paired duplicate instruction text (the usual
cause: the same `la SYM` / `addu r,K` cluster appearing in two blocks). The
goal is fiction, so the block must not be searched.

`inverse_sched.py` refuses it rather than producing vectors, which was verified
directly: it prints the violation list and stops before the search. **This is
the correct outcome, not a tool failure** — a fabricated goal would have
produced confident, wrong lever advice.

To make this function answerable, the alignment needs disambiguation (block-
scoped matching, or UID-range-restricted pairing of duplicate text) in
`goalmap.align`. That is a real, bounded next task; it was not attempted here
because Phase 4 was scoped to new files plus the policy layer.

---

---

# Phase 5 addendum (2026-08-06) — goalmap alignment fix + func_80072CD4 resolved

## The `goalmap.align` fix

`align()`'s second pass pairs the leftover deletes against the leftover
inserts; those pairs ARE the reordering signal, and a mis-pair is worse than no
pair, because it yields a goal that is not a topological order of the block's
dependences — a schedule the compiler could not have produced.

**The defect was the greedy ORDER, not the matching rule.** The pass walked
`dels` ascending and gave each one its nearest free insert, so with duplicate
instruction text an early delete took a far slot that a later delete needed.
Measured on func_80072CD4: `h36 sb $2,12($17)` was paired to `t55`, 19
positions away, while `h39 li $2,252` got no target slot at all.

Two changes, both conservative:

- **Global assignment order.** Candidate pairs are now scored `(exact-text
  before skeleton, then distance)` and assigned cheapest-first across the whole
  leftover set, instead of per-delete nearest.
- **A distance bound** (`MAX_MOVE = 24`). A scheduling move is a within-block
  displacement; a pairing spanning far more than a block is duplicate text.
  Beyond the bound the instruction is left unmapped, and the caller's
  interpolation states the weaker, honest claim.

**Measured, corpus-wide** (702 functions / 3650 blocks across text1a, text1b,
main), old pairing vs new:

| | GOAL-DIFFERS blocks | topo violations |
|---|---|---|
| old | 52 | 126 |
| new | 52 | **97** |

Same set of differing blocks (the fix invents no differences and loses none),
**23% fewer violations**. Per-block: **10 blocks better, 4 blocks worse by +1
each** — so this is a net improvement, not a strict dominance, and that is
stated rather than smoothed over. The four regressions are in two functions
that improve more elsewhere (`func_80048530` block 0: 3→1; `exec_game` block 2:
1→0). `gnd_init_80041688` block 18 reproduces its Phase 4 goal **identically**.

Honest note on the bound: the sweep below shows `MAX_MOVE` made **no
difference** on the measured case — violations were 6 at every window from 30
down to 6. The global-ordering change is what produced the improvement; the
bound is a guard that did not bind here.

## func_80072CD4 — resolved verdict: **NOT A SCHEDULING RESIDUAL**

The fix improved both blocks (block 4: 16→4 differing slots, 5→4 violations;
block 5 pass 1: 3→**0** violations, i.e. a now-VALID goal). Searching the
now-valid block 5 returns a **NEGATIVE**: no perturbation of dependence edges,
LUID order or instruction costs reaches target's order at depth 2.

Reading the two streams directly explains why, and it is not an alignment
problem at all:

```
ours  h35..h40          target t35..t40
addiu $2,252            addiu $2,70      <- a constant ours doesn't have here
sb $2,12($17)           sb $3,4($17)
sb $3,14($17)           sb $3,12($17)    <- offset 12 from $3, not $2
sb $2,20($17)           sb $2,14($17)    <- offset 14 from $2, not $3
addiu $2,252   <- AGAIN addiu $2,252
sb $2,4($17)            sb $2,20($17)
```

Target holds 252 in `$3` (materialised once, earlier — where our build's `$3`
holds 70) and stores offsets 4 and 12 from it. Our build materialises 252
**twice** into `$2` and swaps the `$2`/`$3` roles. The instruction MULTISET
differs, so no dependence-graph perturbation can produce target's order: the
insns themselves are not the same insns.

That matches the park reason ("store-scheduling **duplication**") — the
duplication is the double constant materialisation. **This is a
constant-CSE + register-allocation residual wearing scheduling clothes.** The
productive next step is the RA/CSE side, not the scheduler.

## Verdicts

| function | pass/block | verdict |
|---|---|---|
| `gnd_init_80041688` | 1 / 18 | **LEVER** — 4 single-atom vectors; no statement-reorder solution exists; routes to `walking-pointer-serializes-parallel-loads` |
| `func_80072CD4` | 1 / 5 | **UNREACHABLE, diagnosed** — goal now valid after the alignment fix; search negative because the instruction multiset differs (252 materialised twice vs once, `$2`/`$3` roles swapped). Not a scheduling residual. |
| `func_80072CD4` | 2 / 4, 2 / 5 | **GOAL STILL INVALID** — fewer violations after the fix but not zero, same structural cause |

## Reproduce

```bash
bash tools/sched_solver/mkasm.sh text1a
python3 tools/sched_solver/goalmap.py . text1a gnd_init_80041688 \
        --model tmp/sched_solver_work/text1a.sched.json --pass 1
python3 tools/ra_solver/inverse_sched.py tmp/sched_solver_work/text1a.sched.json \
        --func gnd_init_80041688 --block 18 --pass 1 \
        --goal-from-target text1a --depth 2 --top 8
```
