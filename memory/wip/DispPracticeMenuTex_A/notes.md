# DispPracticeMenuTex_A (src/code6cac.c) — WIP checkpoint

**Floor: 32. RA component is VALIDATED UNREACHABLE; only the SCHED component is open.**
Wiring: `asmfix.txt` `DispPracticeMenuTex_A: replace_with_asmfile "asm/funcs/DispPracticeMenuTex_A.s"`.
Body is cheat-clean (`volatile_cheat_count` 0).

## Residual (via `tools/pairdiff.py code6cac DispPracticeMenuTex_A`)

Two components, per `goal_from_tgt.py classify`:

1. **RA** — a uniform `$v0`/`$v1` exchange: `$v0->$v1` x30, `$v1->$v0` x4, `$a0->$v0` x2.
   Every "delta then round-shift" block puts the intermediate in `$v0` for us and `$v1` for
   target. Representative (asm 112-119):
   ```
   ours   : sra v0,v0,0xc | lh v1,0(gp) | subu v0,v0,a0 | slti v1,v1,11 | bgez v0 | ...
   target : sra v1,v0,0xc | lh v0,0(gp) | subu v1,v1,a0 | slti v0,v0,11 | bgez v1 | ...
   ```
2. **SCHED** — 1 nop. Untouched, and the ONLY part not yet ruled on.

## RA component — UNREACHABLE, with evidence

Attribution closed via `goal --scope` (`tools/ra_solver/pseudo_scope.py`, narrows by
`.lreg` block scope): `$v1->$v0` narrows 27 holders to **UNIQUE pseudo 100**; `$v0->$v1`
narrows 85 to 3 (`83, 84, 98`); `$a0->$v0` narrows 5 to 3.

Derived goal `{100: $v0}` and all three full-exchange variants (`+{83|84|98: $v1}`) run
through `inverse.py global` at depth 2: **all four NEGATIVE**, with 5-17 preference atoms
FORECLOSED — `prune_preferences` (global.c:897) strips `$v0` preferences from allocnos that
cross calls, and these do.

So the exchange is not produced by refs, live span, birth order, conflicts, preferences or
calls-crossed. **Further spelling searches on the RA component are provably zero-yield.**
Next move is INSTRUMENTATION, not spelling: the local-alloc SUGGESTED-REGISTER pass
(`qty_phys_copy_sugg` / `qty_phys_sugg`), reported-but-not-scored today.

Honesty note: `{100: $v0}` is one side of an exchange and the faithfulness guard warns a
partial goal can flatter a result. Here the result is NEGATIVE and the component is
NECESSARY (pseudo 100 must reach `$v0` for target's `lh v0,0(gp)`), so it stands; the three
full-exchange variants were run precisely to close that gap.

## SCHED component (the 1 nop) — ruling: NOT DERIVABLE, and not an independent lever

**Where it is.** Target index 207, immediately after `lw a0,24(s0)` at 206 — it is that
load's delay slot:
```
target : addu v1,v1,v0 | sra v1,v1,0xc | lw a0,24(s0) | nop | subu v0,v1,a0
ours   : addu v0,v1,v0 | sra v0,v0,0xc |              ...  | subu v0,v0,a0
```
Target loads `dst+0x18` LATE and eats the delay; we hoist the load to index 197 so no nop
is needed. The nop is a CONSEQUENCE of the load placement, not a separate phenomenon.

**Mechanically NOT DERIVABLE today (measured, not inferred).** `inverse_sched`'s
`--goal-from-target` reads target's order through `goalmap.asm_body`, probed directly for
this function:
```
honest  code6cac.hon.s: OK, 228 insns
target  code6cac.tgt.s: KeyError: 'DispPracticeMenuTex_A not found in code6cac.tgt.s'
```
The block IS present, as `glabel DispPracticeMenuTex_A` + `/* off addr bytes */` lines —
the exact form `asm_body` skips. The `--goal-order` / `--goal-before` alternatives need RTL
insn UIDs, which needs the four-hop object -> hon.s -> cc1.s -> dbr-uid chain that was
deliberately not built. Note the RA side escaped this because register substitutions are
read off ALIGNED PAIRS; the scheduler side is keyed by UID and cannot avoid it.

**Not an independent lever either way.** The load placement is in the same block-3/4 region
whose RA exchange is validated UNREACHABLE, and a source reorder to defer the load was
measured NOT to move it (the scheduler re-hoists). So even with a derivable goal, this is
coupled to a residual already ruled out — not a standalone opportunity.

**Verified honest** (per the corrected doctrine — measured, not assumed): the function is
NOT rules-free (it still carries `asmfix.txt:50 replace_with_asmfile`, `rules_dropped: 1`),
but the model is honest regardless, because that rule is a post-cc1 TEXT substitution and
cannot reach the RTL. The body itself carries zero cheat constructs: `volatile_cheat_count`
0, zero detector hits inside the body span, and zero occurrences of `__asm__`, `asm(`,
`register `, or `volatile`. The file-wide `cheat_asm_stripped: 139` is siblings.

**Full diagnosis is now complete: every residual component of this function is ruled on.**

## Measured negatives — do NOT re-run

| lever | result |
|---|---|
| remove dead self-assign `inv_s1 = inv_s1;` | inert (32 -> 32); removed anyway, it was catalog debt |
| move `D_800A3310 = 0;` above `new_var` | store moved index 64 -> 53 (target wants 56); 32 -> 32 |
| reorder to load `dst+0x18` later | scheduler re-hoists it; 32 -> 32 |
| local-alloc vectors on **block 12** | wrong block entirely — block 12 is `asm[174:181]`; the contested hunk is block 3 (`asm[98:118]`) |
| `inverse.py global`, 4 goals, depth 2 | all NEGATIVE + foreclosures (above) |

The block-12 error is worth remembering: the contested exchange is **global-vs-local** (the
`use_high` value is block-local; the `dx` chain crosses into the next block, so it is a
global allocno). `inverse.py local` cannot express it — it permutes quantities inside one
block and one participant is not one.

## Tooling

- `tools/pairdiff.py code6cac DispPracticeMenuTex_A` — the scored object-level diff.
- `tools/blockmap.py code6cac DispPracticeMenuTex_A <idx>` — asm index -> basic block.
- `tools/ra_solver/goal_from_tgt.py classify|goal --scope` — verdict + attribution.
- `inverse_compose.py classify` does NOT work here (guarded since f68a9115): its
  `replace_with_asmfile` target is disassembly text it cannot read.

## Stash disposition — `stash@{0}` "nearmatch drafts: SetPacketData + DispPracticeMenuTex_A"

Two uncommitted drafts, held OUT of the tree because both functions still carry
`replace_with_asmfile` wiring, so their edits shift maspsx indices for sibling index-based
regfix rules in the same TU. Neither can be committed without a full-build verify.

| file | draft | worth |
|---|---|---|
| `src/code6cac.c` | removes the dead self-assign `inv_s1 = inv_s1;` from this function | **score-neutral** (32 -> 32). Pure catalog-debt cleanup; it would block `queue done` but buys no distance. Safe to drop and redo if the stash is ever lost. |
| `src/main.c` | SetPacketData cheat removal — redundant volatile casts/aliases, `la` inline asm replaced by honest `&D_800F1AE2`, `register asm("s4")` pin, param-alias local | **34 -> 27, the real value.** Losing it costs a re-derivation; the `la`-to-honest-C step (30 -> 27) is the non-obvious one. |

Recovery if the stash is lost: both are reconstructible from this checkpoint and
`memory/wip/SetPacketData/notes.md`, which record every step with its measurement.
SetPacketData cannot reach COMPLETED-C regardless until the owner grants the D_800F1AEC
IRQ-touched volatile carve-out (`volatile s32 *loop_flag` is load-bearing, 27 -> 39
without it).
