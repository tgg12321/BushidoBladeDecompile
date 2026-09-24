# Campaign 7 — the reload / retry_global_alloc solver (2026-08-06)

Current state. Goal: model GCC 2.7.2's reload-pass interaction with global
allocation — the `retry=` path in `find_reg` — exactly enough to answer "which
C-visible perturbation makes retry give pseudo X hard-reg Y".

**Status: the model is CLOSED, validated exactly, and the answer for the three
target functions is negative.** Details below.

## What the mechanism actually is (read from the source, then measured)

`reload()` (reload1.c) loops until no more hard regs need spilling. Each pass:

1. `order_regs_for_reload()` ranks hard regs by how much trouble spilling one
   would cause (`uses` = summed `reg_n_refs` of the pseudos living there; fixed
   / explicitly-used / eliminable regs get `2*large+2` and land in
   `bad_spill_regs`). Result: `potential_reload_regs`, most-spillable first.
2. Per-insn reload needs are accumulated into `max_needs[class]`.
3. Unmet needs are satisfied by `new_spill_reg()`, which takes the next
   `potential_reload_regs` entry and calls `spill_hard_reg()`.
4. `spill_hard_reg()` sets the reg's bit in the cumulative `forbidden_regs`,
   then for **every pseudo whose `reg_renumber` is that reg** it clears the
   assignment and calls `retry_global_alloc(pseudo, forbidden_regs)`.
5. `retry_global_alloc` re-enters `find_reg` with `losers = forbidden_regs` and
   `retrying = 1` — first with the pseudo's preferred class, then (if that
   fails and the alternate class is not NO_REGS) with the alternate class.

`retrying = 1` disables exactly one thing: the "kick out regs local-alloc used
poorly" tail of `find_reg`. Everything else is the ordinary scan.

## The closed-form law (the whole point of the campaign)

Measured over **194 retry `find_reg` calls in 55 functions across 8 TUs**:

| observation | count |
|---|---|
| `own_copy_prefs` non-empty at retry time | **0 / 194** |
| `own_full_prefs` non-empty at retry time | **0 / 194** |
| `best_reg` == lowest free register in the pass-1 set | **97 / 97** (of the calls that found one) |

Preferences are *destructively consumed* during `global_alloc` (`find_reg` does
`AND_COMPL_HARD_REG_SET (hard_reg_copy_preferences[allocno], used)` in place),
so by reload time every pref set is empty. The pass-0 / pass-1 split never
changes the answer either. So the retry outcome collapses to

```
got = min { r : r ∉ base ∪ forbidden_regs ∪ ~reg_class_contents[class] ∪ hard_reg_conflicts }
base = fixed_reg_set          if allocno_calls_crossed == 0
     = call_used_reg_set      otherwise
     = call_fixed_reg_set     if accept_call_clobbered
```

**The preference lever — the dominant lever pre-reload — is completely inert at
retry.** The only retry inputs are the conflict set, the forbidden set, the
class, and calls-crossed.

## Validation

`python3 tools/ra_solver/reload_sim.py --check` (tree-wide, 8 TUs, 55 funcs):

```
SCAN level: 194/194 find_reg retry calls reproduce best_reg exactly
SETS level: 194/194 exclusion-set reconstructions are bit-identical
```

* **SCAN** replays the register scan + both preference-upgrade stages from the
  dumped exclusion sets.
* **SETS** rebuilds `used2` / `used1` / `used0` from find_reg's own recipe out
  of the RETRYDBG primitives (conflicts, forbidden, used_so_far,
  someone_prefers, calls-crossed, class) and compares bit-for-bit.

That is well past the stated bar (3 targets + 5 matched controls).

## Two pre-existing pipeline defects this surfaced — both fixed

1. **`simulate.py` was validating against the wrong ground truth.**
   `model["dispositions"]` is parsed from the `.greg` file, which cc1 writes
   **after** reload. For any pseudo reload kicked out, `.greg` records the
   POST-retry register. `simulate.py` now compares against the ALLOCDBG stream
   (printed inside `global_alloc`) and prints a `note:` line naming every
   pseudo whose `.greg` value differs.

2. **`extract.py` mis-typed the `mulhi` idiom.** The MD-class regex only looked
   at the outermost RHS operator, so
   `(set (reg:SI 99) (truncate:SI (lshiftrt:DI (mult:DI …) 32)))` — what GCC
   emits for division by a constant — read as `truncate` and was typed
   GR_REGS. `_md_kind()` now recognises all three shapes and records **which
   half** (`md_reg`: `$lo` = 65 for mult/div, `$hi` = 64 for mod and mulhi).
   `simulate.py` is now class-aware: an MD pseudo tries its single-register
   preferred class and falls back to GR_REGS via the alternate class exactly
   as `global.c:585` does.

**Result: `validate.py` is now 10/10 exact, up from 9/10.** The tenth,
saTan4FireDisp, reproduces **17/17**.

## The three target functions — measured answers

### saTan4FireDisp (text1a, floor 29)

The WIP note recorded "three mismatches (pseudos 100/106/112, all `$t`
registers) are the documented reload spill-retry divergence". **That was two
thirds wrong, and the correction is a simulator fix, not a new mechanism:**

* All three are `mulhi` pseudos — the three `channel / 255` colour conversions.
  Only the highest-priority one (`99` at HEAD) actually gets `$hi`; the other
  two lose it to a conflict and fall back to GR_REGS. They were **never**
  divergent; the old simulator just mis-typed them.
* Exactly **one** pseudo is retry-affected: 99, `$hi` → `$t1`. reload's pass-1
  needs are `GR_REGS n=1` and `HI_REG n=1`; it spills `$t0` (top of
  `potential_reload_regs`) and `$hi`; spilling `$hi` evicts 99, whose retry
  finds the lowest free GR register — `$t1`, because `$t0` is now forbidden and
  2-7/17/18/23/29 are conflicts.

`reload_sim.py --target text1a saTan4FireDisp 99 8` reports the only lever:
`$t0` would have to stop being a spill register. **Nothing here touches the
`$s2`/`$s3`/`$s4` rotation that is saTan4FireDisp's actual byte residual**, so
this confirms — now with a complete model rather than a partial one — the
existing WIP conclusion. The note's open item ("pseudos 100/106/112 are where
retry lives — the only part a reload model could address") is hereby answered
and closed.

### func_8007C2A0 / func_8007C4B8 (display, floor 4)

**`display.c` has ZERO `retry_global_alloc` calls.** The only spill activity in
the whole TU is `func_8007CE0C` spilling `$a3` twice, evicting no pseudo. The
twins never enter reload's spill loop at all.

This is consistent with the twins' diagnosis and independent of it: their wall
is the **emit order of the two `sw $sN` / `move $sN,$aN` prologue pairs**,
produced by `save_restore_insns` in LUID order from `expand_function_start` —
a `final`/prologue-emission phenomenon, not register allocation. **The reload
model is measured dead for them.** Do not re-open this route.

## Tooling banked (committed under tools/ra_solver/)

| file | role |
|---|---|
| `reload_harvest.sh` | tree-wide `BB2_RELOAD_DEBUG=1` run → `tmp/reload_work/<stem>.reload.log` |
| `reload_extract.py` | parse the stream → `<stem>.reload.json` (order / needs / spills / kickouts / retries, each retry carrying its find_reg calls and their exclusion sets) |
| `reload_sim.py` | `--check` (SCAN + SETS validation), `--show <stem> <func>`, `--target <stem> <func> <pseudo> <reg>` (the inverse solver) |
| `cc1_hooks.patch.md` §6 | the new print-only cc1 hooks, verbatim, for reproduction |

Parity: `tmp/parity_check_multi.sh` — the instrumented `tools/gcc-2.7.2/cc1`
is byte-identical to `build/cc1` on 5 TUs, and setting `BB2_RELOAD_DEBUG=1`
does not change its output.

## Where the residual modelling gap is (if anyone resumes)

Everything downstream of `forbidden_regs` and the conflict set is exact. What
is still *dumped rather than derived* is the reload **need** computation —
`max_needs[class]` per insn, which decides how many hard regs get spilled and
therefore what lands in `forbidden_regs`. That is `reload.c`'s
`find_reloads`, a much larger surface. It is only worth modelling if a target
function ever needs a *different set of registers spilled*; for the three
functions this campaign was aimed at, it does not.

## Related

[[ra-solver-campaign-2026-08-04]] · [[phantom-frame-slots-gcc272]] ·
[[no-compiler-divergence]] (the hooks are print-only diagnostics, not a
compiler change) · `memory/wip/saTan4FireDisp/notes.md`
