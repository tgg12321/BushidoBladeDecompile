# sched_solver — reverse instruction-scheduling solver (GCC 2.7.2 / MIPS)

Phase 6 of the codegen-modelling campaign, built in the `tools/ra_solver`
style: extract the exact scheduler inputs from an instrumented cc1, replicate
`sched.c` faithfully in Python, validate against ground truth, then search for
the minimal input perturbation that produces the TARGET's instruction order.

Where `ra_solver` answers *"which register"*, this answers **"which order"** —
the residual class that shows up as "our instructions are right but two of
them are swapped".

**Application round complete (2026-08-05).** All three banked scheduling
residuals are resolved: `title_mv_exec2` and `func_8002C61C` are **closed** by
ordinary statement moves, and `tslGlobalMemFree_800861BC`'s is **proven
unreachable** by statement order. See "What this toolkit can and cannot
answer" at the end.

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
| code6cac_b | 1686 | 1684 (99.9%) | 1684 (99.9%) | 842/844 | 842/844 |

1234/1234 function-passes fully exact; 42,943 instructions scheduled; largest
block 125 insns; all 413 blocks of ≥20 insns and all 44 of ≥50 insns exact.
The simulator reproduces not just the order but the **virtual clock of every
pick**, which is the stronger claim (it means the queue, the stall search and
the function-unit state are all right, not merely the tie-breaks).

`code6cac_b` was added for the application round and is listed separately
because it is the one TU where **`extract.py` reports `parity=False`**: the
instrumented cc1 differs from `build/cc1` by exactly one instruction
(`or $4,$4,$2` vs `addu $4,$4,$2`, line 9204) inside `func_80030900` — which is
also the only function whose blocks miss (2/1686). Every other function in the
TU, `func_8002C61C` included (74/74), is unaffected.

Named functions, all blocks exact in both passes: `tslGlobalMemFree_800861BC`
(6), `title_mv_exec2` (3), `func_8007C7A0`/`func_8007C86C` (13 each),
`func_8007CE0C` (25), `saTan4FireDisp` (15), `hirahira_w_ctrl` (4),
`camera_set_zoom` (49), `exec_game` (36), `func_8002C61C` (74, in
**`code6cac_b.c`** — not `code6cac.c`, where it is only declared `extern`).

Separately, the function-unit blockage model is checked in isolation against
every `BLOCKAGE` hook observation: **15497/15497 exact**
(`validate.py --blockage`). The figure grows with the extracted corpus — it was
351/351 when only `config` had been extracted.

## Tools

| file | role |
|---|---|
| `extract.py <stem>` | run cpp + the instrumented `tools/gcc-2.7.2/cc1` (`BB2_SCHED_DEBUG=1`, `-da`), parse the SCHEDDBG stream into `tmp/sched_solver_work/<stem>.sched.json`. Byte-parity-checks instrumented cc1 vs `build/cc1` on the TU first and records the verdict |
| `simulate.py <model.json>` | replay `schedule_block` per basic block; score order-exact / clock-exact |
| `validate.py [stems...]` | batch ground-truth table (above). `--blockage` runs the independent machine-model check; `--funcs a,b` details named functions |
| `mkasm.sh <stem>` | emit the three aligned asm texts the goal mapper needs: `<stem>.cc1.s` (raw cc1), `.hon.s` (+ prologue_fix\|maspsx\|multu_pad — OURS, honest), `.tgt.s` (+ regfix\|asmfix — TARGET bytes) into `tmp/sched_map/` |
| `goalmap.py <root> <stem> <func>` | express TARGET's instruction order in our RTL insn UIDs; `--model` prints per-block goal-vs-ours, `--target` pins the target stream |
| `perturb.py <model.json>` | the search. `--goal-from-target <stem>` derives goals automatically; `--atoms`, `--target`, `--self-check`, `--verify-resort` |

Run from the repo root (or a `git archive` snapshot) under WSL with the venv
active. `extract.py`, `simulate.py` and `validate.py` are read-only with respect
to the tree; `mkasm.sh` writes only under `tmp/sched_map/`.

### The whole loop, in order

```bash
python3 tools/sched_solver/extract.py <stem>          # model (re-run per edit)
bash    tools/sched_solver/mkasm.sh   <stem>          # cc1 / honest / target asm
cp tmp/sched_map/<stem>.tgt.s tmp/sched_map/<stem>.tgt.head.s   # pin target ONCE

python3 tools/sched_solver/perturb.py \
    tmp/sched_solver_work/<stem>.sched.json \
    --func <FUNC> --pass 2 --goal-from-target <stem> \
    --target tmp/sched_map/<stem>.tgt.head.s \
    --atoms luid,luid_move --depth 2
```

Blocks already matching print nothing; each differing block prints its goal and
any vectors found. Then spell a vector in C, **recompile, and re-run the whole
loop** — a vector is a hypothesis, not a result.

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
* **Goals are validity-checked, with a fallback.** A goal must be a topological
  order of the block's `LOG_LINKS`. The σ composition above assumes reorg applied
  the *same* positional permutation to both sides; when it did not — our reorg
  pulls an insn into a delay slot and target's leaves it alone — the composition
  scrambles the goal. `goal_for_block` detects that and falls back to target's
  own post-reorg order, which needs no assumption about reorg at all. Only if
  *that* is also non-topological is the block skipped. (`func_8002C61C` block 31
  was unusable until this fallback existed; the first diagnosis, that duplicate
  instruction text had been cross-paired, was wrong — capping the pairing
  distance from 8 to 999 changed nothing.)

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

**What it invalidated, and the status of each.** Any *scheduling* "no vector"
result from before 2026-08-05 was searched under this defect. That is:

* the `add_dep 157 <- 44` worked example below — **still not re-derived**; treat
  it as illustrative of the atom vocabulary, not as a current finding;
* the first-pass "no single-atom vector" on all three application functions —
  **re-run**, and they remain true negatives at depth 1;
* the first-pass depth-2 families that reported an unspellable `add_dep` half —
  **re-run**, and superseded: with `--atoms luid,luid_move` two of the three
  functions turned out to have fully spellable pairs.

It does **not** touch anything from `ra_solver`, which has no `ready0`.

### Search spellable atoms FIRST (`--atoms luid,luid_move`)

The full enumeration lists `add_dep` atoms before the LUID ones, so a depth-2
run that stops at `--max N` can report only families whose halves are
dependence edges — several of which have no C spelling at all — while a
**fully spellable** pair sits further down the list, never reached. This
happened on both `tslGlobalMemFree_800861BC` and `title_mv_exec2`: the first
report for each was "one half has no natural C form", and restricting to
`--atoms luid,luid_move` found pure statement-order pairs for both.

Run the restricted search first. It is also far cheaper.

**But model-spellable is not C-spellable**, and the dividing line is now
measured:

* **Stores are spellable.** `title_mv_exec2`'s pair is two real stores; moving
  them worked, 19 → 14 differing instructions, and re-goaling from the new form
  shows all three blocks at goal == identity — the scheduling class closed.
* **Address materialisations and hoisted constants are usually inert.**
  `tslGlobalMemFree`'s pair needs `luid swap 9 <-> 290`, where 9/12/18 are
  `x = &GLOBAL;` statements and 290 is a **LICM-hoisted constant**. All six
  permutations of those three statements, with and without the loop counter
  hoisted, emit **bit-identically**. GCC's earlier passes re-derive where an
  address materialisation lands, so its LUID does not follow source order.

Always compile the TU and measure. The model proposes, the compiler disposes.

### Iterating: PIN THE TARGET (`--target`)

regfix rules are indexed to HEAD's instruction positions, so regenerating
`<stem>.tgt.s` from an EDITED source produces fiction — and it looks like
progress, because the rules land on whatever now sits at those indices. Capture
target once from clean HEAD:

```
git show HEAD:src/<stem>.c > src/<stem>.c
bash tools/sched_solver/mkasm.sh <stem>
cp tmp/sched_map/<stem>.tgt.s tmp/sched_map/<stem>.tgt.head.s
```

then pass `--target tmp/sched_map/<stem>.tgt.head.s` on every later round.
Target is the original binary; it does not change when we edit. Re-run
`extract.py <stem>` after each edit though — the RTL UIDs do change.

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

## What this toolkit can and cannot answer

**Can:**

* *Is any part of this function's residual a scheduling problem at all?* This is
  the highest-value question and it is answered cheaply and definitively — every
  block reports goal == identity or not. On the application round it removed
  scheduling from consideration for 5 of 6 `tslGlobalMemFree` blocks and both
  `title_mv_exec2` loop blocks, redirecting that work to register allocation.
* *Which minimal input change produces target's order?* Restricted to
  `--atoms luid,luid_move`, any vector returned is spellable as a statement move.
* *Is a proposed order even possible?* The topological check rejects goals no
  compiler could emit.

**Cannot:**

* *Which register.* That is `ra_solver`. Order and allocation are separate axes,
  and fixing order can make the register diff slightly worse (`func_8002C61C`:
  `replace` 26 → 28 while the order went fully correct). Never judge a
  scheduling edit by the whole-function differing count.
* *Anything after sched2.* `reorg.c`'s delay-slot filling and maspsx's own
  reordering are downstream. The mapper cancels reorg rather than modelling it,
  and falls back to target's own order when the two sides' reorg permutations
  disagree — a slightly weaker claim, and the reason a closed function can still
  show one `moved` in the alignment.
* *Whether a vector is spellable.* The model can only say which LUID would do
  it. Whether C controls that LUID is a separate question with a measured
  answer: **stores yes, address materialisations and hoisted constants usually
  no** (see above). Always recompile and re-run the loop.
* *Frame size, instruction count, or phantom slots.* Out of scope entirely.

**Application results (2026-08-05).** `title_mv_exec2`: closed, two statement
moves, 19 → 14 differing, all 3 blocks identity. `func_8002C61C`: closed, the
`t1 = 0` hoist in **both** copy loops, all blocks identity (whole-function count
only 34 → 33 — the residual there is the OFFSET/RA families). 
`tslGlobalMemFree_800861BC`: not closed and not closable this way — all six
permutations of its three leading statements emit bit-identically.
