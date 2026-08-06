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
~~`gnd_init_80041688` is rules-only (3 regfix), so `text1a.sched.json` is honest
as extracted.~~

> **CORRECTED 2026-08-06.** That premise was FALSE: `gnd_init_80041688` is
> *not* rules-only. Its body carries an undocumented `volatile s32 sp10[8];` +
> `(void)sp10;` frame cheat (commit `fd1497f7`). The conclusion still holds —
> the block-18 model IS honest, measured three ways below — but it held by
> luck, not by the reasoning given here. The general rule stands for
> *genuinely* rules-only functions; the error was asserting this function
> qualified without checking. **Verify the cheat inventory before invoking
> this note** (`engine/volatile_cheats.py`, or diff the source against
> `engine.inlineasm.write_stripped` output). A park reason is not a cheat
> inventory — this one omitted the frame cheat for 15 sessions.

---

> ## ⚠ CORRECTION 2026-08-06 (commit `fd1497f7`) — READ BEFORE CASE 1
>
> **The Case-1 LEVER verdict below is KILLED.** The mapped lever
> (`walking-pointer-serializes-parallel-loads`) does not apply to this block,
> and the function's recorded debt was incomplete. Case 1 is retained
> unedited as the record of what the tool produced; this box states what is
> now known. Details in the Phase-4 correction section at the end of this
> document.
>
> - **The lever is dead.** A 12-spelling sweep (including the rule's literal
>   post-increment walk) emits byte-identical code. The mechanism requires
>   intervening memory WRITES; three read-only loads provide no edge.
> - **The mechanism is bottom-up readiness, not LUID.** The +26 load's sole
>   consumer is the final `or`, so it is ready earliest, picked early, and
>   emitted last — placing it first *or* last in RTL both emit it last.
> - **The model itself was NOT contaminated** by the undocumented cheat —
>   measured three ways, see the correction section.
> - **The function's real debt is larger** than the 3 regfix rules recorded:
>   a `volatile s32 sp10[8]` frame reservation is load-bearing and belongs to
>   the phantom-slot surface.

## Case 1 — `gnd_init_80041688` (src/text1a.c), pass 1 block 18: **LEVER** *(KILLED — see correction box above)*

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
| `gnd_init_80041688` | 1 / 18 | ~~LEVER~~ → **KILLED** (`fd1497f7`): 4 vectors stand, but the mapped lever needs intervening memory writes and cannot apply to three read-only loads; bottom-up readiness (sole consumer of the final `or`) decides, not LUID. Model verified UNcontaminated. Debt is 3 regfix **+ an undocumented frame cheat**. |
| `func_80072CD4` | 1 / 5 | **UNREACHABLE, diagnosed** — goal now valid after the alignment fix; search negative because the instruction multiset differs (252 materialised twice vs once, `$2`/`$3` roles swapped). Not a scheduling residual. |
| `func_80072CD4` | 2 / 4, 2 / 5 | **GOAL STILL INVALID** — fewer violations after the fix but not zero, same structural cause |

---

# Phase-4 correction (2026-08-06) — Case 1 re-examined after `fd1497f7`

## Item 1 — was the Phase 4 model contaminated? **NO, measured three ways**

The concern was legitimate: Phase 4 asserted an honest model on the strength of
"rules-only", and that premise was false — the body carries a `volatile s32
sp10[8]; (void)sp10;` frame cheat that `engine/volatile_cheats.py` flags and
`write_stripped` strips. So the model had to be re-derived rather than trusted.

The sched model for text1a was re-extracted from three source states and block
18 compared field by field (`n_insns`, `ready0`, node attributes incl. LUID /
unit / icost / priority / ref-count, the full `deps` graph, the pick sequence
and the per-pick clocks):

| source state | block-18 model |
|---|---|
| on-main `src/text1a.c` (full cheat present) — the Phase 4 model | reference |
| `write_stripped` output (`(void)sp10;` discard removed) | **IDENTICAL** |
| `sp10` declaration removed as well (no trace of the cheat) | **IDENTICAL** |

Every field matches: `ready0 [200]`, picks `[200,198,195,191,194,193,187,183]`,
clocks `[1..8]`, 5 dependence entries, 8 nodes. Re-running the inverse search
against the fully-stripped model returns **the same 4 vectors in the same
order**. Both re-extractions reported `parity=True` (instrumented cc1 agrees
with the build compiler).

**Conclusion: the Phase 4 block-18 derivation was NOT contaminated.** The cheat
reserves 32 frame bytes; it emits no instructions into this block and adds no
edges to its dependence graph, so the scheduler model never sees it.

Two things worth carrying forward, because the *reasoning* was wrong even
though the *answer* was right:

1. **`write_stripped` was not a complete strip — FIXED 2026-08-06.** It removed
   the `(void)sp10;` discard but left `volatile s32 sp10[8];` standing, so the
   third source state above had to be built by hand. `engine/volatile_cheats.py`
   now closes that gap generally (`find_orphaned_local_decls`): a local
   declaration whose every remaining reference lies inside a span the stripper
   already removes is stripped with it. `sandbox gnd_init_80041688 --disable all`
   reports **8** directly, matching the hand-built state and confirming the 2 → 8
   measurement below. The general caution still stands for constructs outside the
   detector roster — verify per-construct rather than assume.
2. **Frame-reserving cheats are invisible to the sched and RA graphs but not
   to the byte score.** Removing sp10 regresses the honest distance 2 → 8 at an
   unchanged 82 insns (per `fd1497f7`). A model can be perfectly honest for the
   question you are asking while the function still carries load-bearing debt.

## Item 2 — the LEVER verdict is KILLED

Per `fd1497f7`, a 12-spelling sweep (the rule's literal post-increment walk, a
walk visiting +26 first, a split-pointer form, both local orderings, two
fully-inlined forms, three named-intermediate forms) emitted **byte-identical
code** in every case — `lbu +24, +25, +26`, sandbox 2 at 82 insns.

**Why the mapped lever cannot work here.**
`walking-pointer-serializes-parallel-loads` derives its dependence from
intervening memory **writes**; both of its confirmed cases have stores between
the loads. Block 18 is three reads feeding one expression with no store between
them, and reads do not anti-depend on reads. GCC additionally folds every
constant byte offset into the `lbu` displacement, so no pointer register even
survives to carry a dependence.

**What actually decides the order — and it is not LUID.** The C statement order
*does* reach sched1: spelling the locals `b,r,g` produces target's exact
pre-scheduling RTL order (+26, +24, +25) in `.combine`, and sched1 alone reverts
it. All three loads carry equal priority so the LUID tiebreak nominally
decides — but LUID is not binding: placing the +26 load first *and* last in RTL
both emit it last. The cause is **bottom-up readiness**: the +26 load's sole
consumer is the final `or`, so it becomes ready strictly earlier than the
+24/+25 loads (which wait on their `sll`s and the inner `or`), and is therefore
picked early and emitted late. Only a genuine dependence edge delaying its
readiness changes that.

**The tool's report was not wrong about the space** — the absence of any
cost-1 LUID vector was already the finding that "this is not a statement
reorder", and the other two vectors were labelled forbidden (volatile-spelled
anti-dep) and unspellable (insn_cost). What was wrong was the lever the
`dep_add_true` class mapped to. That mapping is now corrected in `levers.py`
with the precondition and both measured negatives recorded, so it cannot
over-promise this again.

## Item 3 — corrected function state

`gnd_init_80041688` debt is **larger than recorded**:

- 3 regfix rules (the recorded park reason), plus
- an undocumented `volatile s32 sp10[8]; (void)sp10;` frame reservation —
  cheat-asm by the engine's own classification, load-bearing (removing it costs
  6 diffs), absent from the s1–s15 ledger, the park reason and the filed owner
  escalation.

So closing the lbu order would **not** have reached COMPLETED-C. The frame
reservation is a separate residual belonging to the
[[phantom-slot-frame-lever]] diagnosis surface.

## Reproduce the correction

```bash
# re-extract the sched model from an arbitrary source (tmp scratch)
python3 tmp/_honest_sched.py tmp/inverse_work/src/text1a.c \
        tmp/sched_solver_work/text1a.honest.sched.json
python3 tmp/_diff_blk18.py tmp/sched_solver_work/text1a.sched.json \
        tmp/sched_solver_work/text1a.honest.sched.json
```

---

## Reproduce

```bash
bash tools/sched_solver/mkasm.sh text1a
python3 tools/sched_solver/goalmap.py . text1a gnd_init_80041688 \
        --model tmp/sched_solver_work/text1a.sched.json --pass 1
python3 tools/ra_solver/inverse_sched.py tmp/sched_solver_work/text1a.sched.json \
        --func gnd_init_80041688 --block 18 --pass 1 \
        --goal-from-target text1a --depth 2 --top 8
```
