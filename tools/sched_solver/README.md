# sched_solver — reverse instruction-scheduling solver (GCC 2.7.2 / MIPS)

Phase 6 of the codegen-modelling campaign, built in the `tools/ra_solver`
style: extract the exact scheduler inputs from an instrumented cc1, replicate
`sched.c` faithfully in Python, validate against ground truth, then search for
the minimal input perturbation that produces the TARGET's instruction order.

Where `ra_solver` answers *"which register"*, this answers **"which order"** —
the residual class that shows up as "our instructions are right but two of
them are swapped", including the scheduling residuals banked on
`tslGlobalMemFree_800861BC`, `func_8002C61C` and `title_mv_exec2`.

## Status: the model is exact

| TU | blocks | order-exact | clock-exact | sched1 | sched2 |
|---|---|---|---|---|---|
| config | 230 | 230 (100%) | 230 (100%) | 115/115 | 115/115 |
| main | 1992 | 1992 (100%) | 1992 (100%) | 996/996 | 996/996 |
| code6cac | 1470 | 1470 (100%) | 1470 (100%) | 735/735 | 735/735 |
| display | 690 | 690 (100%) | 690 (100%) | 345/345 | 345/345 |
| text1a | 528 | 528 (100%) | 528 (100%) | 264/264 | 264/264 |
| text1b | 2068 | 2068 (100%) | 2068 (100%) | 1034/1034 | 1034/1034 |
| **TOTAL** | **6978** | **6978 (100%)** | **6978 (100%)** | | |

1234/1234 function-passes fully exact; 42,943 instructions scheduled; largest
block 125 insns; all 413 blocks of ≥20 insns and all 44 of ≥50 insns exact.
The simulator reproduces not just the order but the **virtual clock of every
pick**, which is the stronger claim (it means the queue, the stall search and
the function-unit state are all right, not merely the tie-breaks).

Named functions, all blocks exact in both passes: `tslGlobalMemFree_800861BC`
(6), `title_mv_exec2` (3), `func_8007C7A0`/`func_8007C86C` (13 each),
`func_8007CE0C` (25), `saTan4FireDisp` (15), `hirahira_w_ctrl` (4),
`camera_set_zoom` (49), `exec_game` (36). `func_8002C61C` lives in
`code6cac.c`, exact as part of that TU's 1470/1470.

Separately, the function-unit blockage model is checked in isolation against
every `BLOCKAGE` hook observation: **351/351 exact** (`validate.py --blockage`).

## Tools

| file | role |
|---|---|
| `extract.py <stem>` | run cpp + the instrumented `tools/gcc-2.7.2/cc1` (`BB2_SCHED_DEBUG=1`, `-da`), parse the SCHEDDBG stream into `tmp/sched_solver_work/<stem>.sched.json`. Byte-parity-checks instrumented cc1 vs `build/cc1` on the TU first and records the verdict |
| `simulate.py <model.json>` | replay `schedule_block` per basic block; score order-exact / clock-exact |
| `validate.py [stems...]` | batch ground-truth table (above). `--blockage` runs the independent machine-model check; `--funcs a,b` details named functions |
| `mkasm.sh <stem>` | emit the three aligned asm texts the goal mapper needs: `<stem>.cc1.s` (raw cc1), `.hon.s` (+ prologue_fix\|maspsx\|multu_pad — OURS, honest), `.tgt.s` (+ regfix\|asmfix — TARGET bytes) into `tmp/sched_map/` |
| `goalmap.py <root> <stem> <func>` | express TARGET's instruction order in our RTL insn UIDs (the piece the "What is still missing" section below used to describe) |

Run from the repo root (or a `git archive` snapshot) under WSL with the venv
active. The tools are read-only with respect to the tree.

## Which passes shape the final order

Both. `toplev.c` sets `flag_schedule_insns` **and**
`flag_schedule_insns_after_reload` at `optimize >= 2`, and MIPS defines
`INSN_SCHEDULING`, so at this build's `-O2` every function is scheduled twice:
**sched1** before register allocation (dump `.sched`, `reload_completed == 0`)
and **sched2** after reload (dump `.sched2`). They differ in two modelled
ways: `adjust_priority` is a no-op in sched2, and sched1 additionally
maintains the live-range bookkeeping that feeds `global_alloc`. Models are
tagged `pass=1` / `pass=2`; validate scores them separately.

Everything after sched2 (delayed-branch scheduling in `reorg.c`, and maspsx's
own reordering) is downstream of this model and out of its scope.

## The algorithm, as replicated

`schedule_block` builds each basic block **backwards**: the ready list starts
from the insns with no successors (`INSN_REF_COUNT == 0`, i.e. the block tail)
and each pick is *prepended* to the output. So a high priority means
"schedule me late". Confusions this resolves: `INSN_PRIORITY` is the longest
dependence chain *from the block head*, and `LOG_LINKS(insn)` are the insns
`insn` depends on, released when `insn` is picked.

* **Priorities.** `priority(insn) = max over preds of (priority(pred) +
  insn_cost(pred, link, insn) - 1)`, floor 1. The trailing run of
  CALL/JUMP/USE/cc0-setter insns is pinned in place: the last gets
  `TAIL_PRIORITY - i` (0x7ffffffe - i) and each earlier one an anti-dependence
  on its successor. Harvested from the dump rather than recomputed — the
  critical path is an *input* to the pick order, not part of it.
* **`insn_cost`.** `INSN_COST(pred)` (the machine description's ready cost),
  except that MIPS's `ADJUST_COST` zeroes the cost of any anti/output
  dependence — so **only true data dependences (kind 0) can carry latency
  above 1** — and a `USE` consumer never forces a wait.
* **`rank_for_schedule`.** Priority descending; then dependence class relative
  to `last_scheduled_insn` (3 = independent or latency-1, 2 = anti/output,
  1 = data — highest class wins, i.e. *prefer the insn least entangled with
  what was just placed*); then `INSN_LUID` **descending**.
* **`SCHED_SORT`.** `swap_sort` (insertion of the single new element) when
  exactly one insn was added since last cycle, `qsort` when more than one, and
  **nothing at all when none were added**. That third case is load-bearing:
  the ready list can carry a stale order across a cycle, so the scheduler is
  not a pure function of the current priorities — it has history.
* **The sliding window.** C consumes the chosen insn with `ready += 1`, so the
  scheduled insn is not removed from the array, the base just advances; new
  insns are appended at `n_ready`, overwriting whatever stale entries sit
  there. Python models this as a list with `pop(0)` / `append`.
* **`schedule_select`** (runs because `MAX_BLOCKAGE` is 112 > 1). Walking the
  ready list in equal-priority groups: queue every insn currently blocked by a
  function-unit hazard (`actual_hazard`), and if more than one survives the
  group, promote the one with the largest `potential_hazard` to the front.
  `potential_hazard = (minb * 0x40 + maxb) * ((unit_n_insns[unit] - 1) * 0x1000 +
  unit)` — note the multiply: a unit with only one insn in the block scores 0,
  so this term only ever breaks ties on a *contended* unit.
* **`schedule_insn`.** Occupy the function unit, then release predecessors: on
  the last unsatisfied requirement, either make the pred ready (cost ≤ 1) or
  queue it `cost` cycles out in a 128-slot circular `insn_queue`. `INSN_TICK`
  carries the earliest-fire time across partial releases.
* **The clock.** One tick per loop iteration; when nothing is ready, the stall
  search scans forward through the queue and jumps the clock to the first
  non-empty slot.

## Machine model (MIPS r3000)

From `config/mips/mips.md` + `insn-attrtab.c`. Only two units are reachable in
this build: **memory** (unit 0, `max_blockage` 3; load ready 2, store 1, xfer
2 on r3000) and **imuldiv** (unit 1, `max_blockage` 69; hilo ready 1 issue 3,
imul 12, idiv 35). Units 2–4 are FP and unused. `unit == -1` means the insn
occupies no unit at all.

The generated `*_unit_blockage` pairwise functions were **measured, not
re-derived** — the `BLOCKAGE` hook prints `raw_tick`, `adj_tick` and
`max_blockage`, so the compiler's own value is exactly
`adj_tick + max_blockage - raw_tick`. The fit:

* memory: `max (1, bmax(last) - bmax(exec) + 1)`
* imuldiv: the executing insn's own issue delay — its ready cost when > 1,
  else 3

`validate.py --blockage` re-checks this against every observation, so it stays
falsifiable. This was the *entire* residual: before it, config.c sat at
93.0% blocks exact; with it, 100%.

## Two harvested inputs

Consistent with `ra_solver` harvesting `seed_used`, two values are read from
the compiler instead of derived, because no dump carries what they depend on:

1. **`INSN_PRIORITY` after `priority()`** — deliberate. The critical-path
   computation is an input to the search, not a thing the search reasons about.
2. **`adjust_priority`'s `birthing_insn_p` flag** (pass 1 only) — it reads
   `bb_live_regs` and `reg_n_sets`, i.e. liveness state the dump does not
   carry. `n_deaths` is always 0 (sched.c notes REG_DEAD notes are already
   gone), so only the birthing branch can fire.

Both are per-block in the model, so a perturbation layer that wants to move
them must state the change explicitly rather than get it for free.

## The perturbation layer (`perturb.py`)

The model being exact means the question can be asked in reverse: what minimal
change to a block's *inputs* produces TARGET's order?

**Priorities are never perturbed directly.** `priority()` is a pure function of
the dependence graph and the instruction costs, so `perturb.py` *recomputes*
it after every atom (`recompute_priorities`). Perturbing a priority on its own
would model a change no C edit can make in isolation. The recomputation is
validated against every dumped `INSN_PRIORITY`: **21,828/21,828 exact** across
config/main/code6cac/text1a (`perturb.py <model> --self-check`).

Goals are stated over the output order — `--goal-order u1,u2,...` for an exact
sequence, or the usually more practical `--goal-before A:B` (repeatable) when
the residual is "these two are the wrong way round". `--depth 2` searches
pairs when no single atom suffices.

Worked example — forcing insns 44 and 157 to swap in `title_mv_exec2` pass 2
block 0 (22 insns, 998 single atoms searched) returns six vectors, of which

    add_dep 157 <- 44 (anti-output)

reproduces our order with *only* those two exchanged and nothing else moved:
the cleanest possible answer, and one that maps to a specific C claim (insn 44
writes a location that 157 reads or writes, so the two must be ordered).

### The atoms, and their C-level meanings

* **Added dependence edge (pred, insn, kind).** The most direct atom: a true
  dependence is a value flowing between two statements, an anti/output
  dependence is a write ordered against a read/write of the same location.
  C-controllable via aliasing, via splitting or merging expressions, and via
  memory-access ordering. Note the asymmetry the model makes explicit: an
  anti/output edge costs 1 cycle and only constrains order, while a true edge
  carries the unit's real latency.
* **Removed dependence edge** — the same lever pulled the other way (the
  classic being a memory dependence that a `restrict`-shaped rewrite drops).
* **Priority change.** Because priority is longest-chain-to-here, it moves
  when the chain moves: lengthening or shortening the dependence path, not by
  local edits. Worth searching as `delta on one insn` first, then asking which
  chain edit realises it.
* **LUID change** = source order of the two insns' generating statements —
  the cheapest atom to spell in C, and the final tie-break, so it decides
  every case where priority and dependence class tie.
* **Unit/cost change** = instruction selection (a load vs a move), reachable
  by type and addressing-mode changes.

### The goal mapper (`goalmap.py`, `mkasm.sh`) — BUILT 2026-08-05

`perturb.py --goal-from-target <stem>` now derives the goal from the target
binary itself, per block, for every block of a function at once. The chain:

```
target .s  --difflib+move-pairing-->  honest .s  --difflib-->  cc1 .s  --index-->  .dbr UIDs
```

* **`.tgt.s` really is target.** The tree builds SHA1-identical to the original
  EXE, so running the honest stream through `regfix | regfix_stage2 | asmfix`
  produces target's byte order *as text*, at the same granularity as our own
  output. That removes the objdump/macro-expansion hop entirely — no collapsing
  of `lui`/`addiu` pairs, no nop bookkeeping.
* **`.dbr` index-aligns 1:1 with the cc1 `.s` body.** The post-reorg RTL dump
  lists exactly the body instructions in emission order; the epilogue is a
  UID-less suffix (GCC emits it from `function_epilogue`, not from RTL).
* **difflib alone is not enough.** `SequenceMatcher` only ever produces
  *monotone* alignments, so a MOVED instruction is reported as a delete plus an
  insert — and a monotone map is exactly what a scheduling difference is not.
  Without the second move-pairing pass the mapper reports every block as
  already matching. Pairing is exact text first, then operand skeleton.
* **reorg.c cancels rather than being modelled.** Delay-slot filling runs after
  sched2, so both sides are compared in POST-reorg space and the result is
  carried back through our own known reorg permutation
  (`goal_pre[i] = T[sigma(i)]`). Guessing the un-fill directly is wrong: reorg
  often lifts an insn from several positions back, not just from before the
  branch.
* **Goals are validity-checked.** A goal must be a topological order of the
  block's `LOG_LINKS`. When the same instruction text occurs in two blocks (the
  usual `la SYM` / `addu r,1100` cluster) the move-pairer can cross-pair them;
  the topo check catches it and the block is skipped rather than searched
  against an impossible goal.

Caveat: regenerate `mkasm.sh` output whenever `src/` changes, and treat
`.tgt.s` as valid **only at HEAD** — regfix rules are calibrated to HEAD's
instruction indices, so after a source edit the "target" stream is fiction.

### `ready0` was not being re-sorted — fixed 2026-08-05

`extract.py` harvests `ready0` **already sorted** by the compiler, and
`perturb.py` never touched it. But `schedule_block` sorts that list with
`rank_for_schedule` before the loop, so any LUID or priority change moves it —
and because SCHED_SORT does nothing on a cycle where no insn was added, the
harvested order survived the opening cycles verbatim. The effect: **every LUID
atom was inert for exactly the picks it should decide**, and the search
reported "no vector" for goals that are one ordinary source-order edit away.
All three banked residuals hit this.

`apply_priorities` now ends with `resort_ready0`. The falsification test is
`perturb.py <model> --verify-resort`: re-sorting an *unperturbed* block must
reproduce the compiler's own order, and it does — **8664/8664 blocks across all
seven TUs, ready0 unchanged and still order-exact**.

Because of this, the `add_dep 157 <- 44` worked example above was found under
the old (inert-LUID) atom set and should be re-derived before being relied on.

### New atom: `luid_move`

A pairwise LUID swap says "exchange two statements". It cannot say "move this
statement to there" — one insn travelling several positions while the rest
close up behind it — which is both the cheaper C edit and the shape the banked
residuals actually have. `luid_move a -> b` renumbers the block so A sits
immediately before B, others keeping their relative order and the block keeping
its LUID value multiset.

### The standing caveat

The stale-order (`no sort`) case means two input states that differ only in
*when* an insn entered the ready list can schedule differently. Perturbations
must be applied to the block inputs and replayed, never patched onto an output
order.
