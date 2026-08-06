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

## Uncommitted draft

The dead-self-assign removal lives in `stash@{0}` ("nearmatch drafts"), score-neutral.
