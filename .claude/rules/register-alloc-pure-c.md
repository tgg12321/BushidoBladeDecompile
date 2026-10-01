---
name: register-alloc-pure-c
paths: [".claude/rules/register-alloc-pure-c.md"]
description: "Register diff vs target (or a pin you want to add): diagnose with cc1 -da .greg/.lreg; levers are block-local split, narrow type, loop precompute; dead stores only last-resort per dead-store-fake-exception."
metadata:
  type: reference
---

# Retiring a register difference in pure C

A register diff (or the urge to add a `register T x asm("$N")` pin — forbidden, [[register-asm-pins]]) is
almost always a global-allocation tie that C source structure can move.

## Step 0 — diagnose: is your build or the target the anomaly?

MIPS gcc-2.7.2 defines no `REG_ALLOC_ORDER`, so lower-numbered hard regs are PREFERRED (`$v0`=2, `$v1`=3,
`$a0..$a3`=4-7, `$t0..$t7`=8-15, `$s0..$s7`=16-23, `$t8/$t9`=24/25). If the **target uses a LOWER register**
than your build for the same value, your build is the anomaly: something blocks the preferred register — find
and clear the blocker.

Dump the allocation: `cc1 <build flags> -da standalone.c` → `.lreg` / `.greg`; `;; Register dispositions:`
maps `pseudo -> hardreg`. Build the standalone with the function's real extern decls (arg counts and
pointer-ness affect codegen). The instrumented cc1 (`tools/gcc-2.7.2/cc1`, `BB2_ALLOC_DEBUG` /
`BB2_SCHED_DEBUG` / `BB2_PRIO_DEBUG`) prints the allocno priority table directly.

## Which wall class is it? (read `.lreg`: `Register N used K times ... dies in D places`)

| tell | class | go to |
|---|---|---|
| the losing pseudo `dies in D places` with D > 1 | local-vs-global class wall (`local-alloc.c:472`) | [[local-alloc-death-count-class-wall]] — the levers below won't move it |
| both pseudos in `.greg` "N regs to allocate", near-tied | allocno-priority tiebreaker (`global.c:624`) | levers below + `pre-slim-2026-10-01:memory/project/register-alloc-deep-dive.md` |
| register diff downstream of a mis-scheduled insn | not RA at all | [[sched-rank-class-tie-wall]] |

## Lever A — block-local variable split

One variable reused across many sites (e.g. loaded in several `switch` cases) gets ONE register for the whole
pseudo, often a soon-to-be argument register. Read the value into a fresh block-local in one (or a few) sites;
the shared pseudo's shorter range lets it take the preferred register elsewhere.

```c
case 0x90: { u8 *cp = *state; ...; handler(a0, a1, b, next); goto end; }   /* saTan0Main: last pin retired */
```

## Lever B — narrow integer type

Width changes value tracking and allocation: `u32 b` → `char b` emits/elides the target's masks and can yield
the target's callee-save choice (saTan0Main). The build uses `-funsigned-char`. The permuter's type-mutation
pass finds this; run it from a pin-free base.

## Lever C — whole-function reallocation via loop-local / precompute

The allocator is global: precomputing a call argument into a loop-local can move a prologue copy GCC otherwise
copy-prop-folds away.

## Other closing levers seen

- **Explicit status flag** for an exit accumulator the target carries in `$v0` (`status = 0` / `status = -1`,
  then `if (status != 0)`) — real meaning, not a cheat ([[exit-path-return-set-cse-join]]).
- **Explicit base pointer** (`s32 *base = arr; elem = base + idx;`) flips the base-vs-shift evaluation order.

## Lever D — dead stores: LAST RESORT

Only after A/B/C are measured dead and the blocking mechanism is named from a dump; then exactly per
[[dead-store-fake-exception]] (FAKE annotation, exhaustion, layer-1 + layer-2). Un-annotated dead param
assigns / dead conditional stores are flagged by `find_dead_param_assigns` / `find_dead_conditional_stores`.
A dead store paired with a pin still fails on the pin. FORBIDDEN in this family: combine-foldable
chain-extenders and DImode round-trips used to bump pseudo refs/numbering.

## Order of attack

1. Step 0 diagnosis. 2. Pin-free permuter. 3. If it plateaus a register short, dump `.greg`, find the
blocker, apply A/C by hand. 4. Verify with the full SHA1 gate — isolated scores can mislead for
relocation-heavy functions.

Known measured walls (read the deep dive before re-testing any lever): `cpu_side_move_dir_4`,
`marionation_Exec` (global.c:624 class); `func_800611A4` / `func_80061658` / `func_80061710` /
`func_80057CC8` (local-alloc class).

## Related

[[local-alloc-death-count-class-wall]] · [[sched-rank-class-tie-wall]] · `pre-slim-2026-10-01:memory/project/register-alloc-deep-dive.md` ·
[[register-asm-pins]] · [[dead-vars-local-array]] · [[no-compiler-divergence]]
