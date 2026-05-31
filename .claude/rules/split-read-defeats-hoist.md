---
name: split-read-defeats-hoist
paths: ["src/*.c"]
description: "Duplicate a post-branch read into each flag arm (direct symbol in the known arm) to stop GCC hoisting per-arg offsets across a switch; regs fall out pin-free. Meta: register-rename plateau = restructure, not pins."
metadata:
  type: reference
---

## Symptom

A function selects a base (a runtime pointer, or one of two globals) from a flag,
does a shared read, then dispatches on a `mode` switch. The natural C computes the
read ONCE after the flag branch, through the selected base:

```c
if (flag & 1) base = GLOBAL_A; else base = (u16 *)GLOBAL_B;
t = ((base[arg_hi] & 0xFF) << 16) | base[arg_lo];   /* ONE read, post-branch, via base */
switch (mode) { /* cases index through base[...] */ }
```

GCC 2.7.2 hoists the `arg_hi`/`arg_lo` halfword-offset address computations up and
*across* the switch, and picks a register allocation that does not match target.
Register `asm()` pins do NOT fix it: pinning one register cascades and breaks two
others (a "cluster coupling"). You grind diffs down with the permuter + micro-levers
(dead-var scratch, chained copies, temp reuse) but hit a hard floor of pure
REGISTER-RENAME diffs (0 structural) that will not close.

## Fix -- duplicate the read into the branch arms

Move the read INTO each arm of the flag branch. In the arm where the base is a known
symbol, index the symbol DIRECTLY (not through `base`); use the pointer only in the
other arm:

```c
if (flag & 1) {
    t = ((GLOBAL_A[arg_hi] & 0xFF) << 16) | GLOBAL_A[arg_lo];   /* direct symbol */
} else {
    base = (u16 *)GLOBAL_B;
    t = ((base[arg_hi] & 0xFF) << 16) | base[arg_lo];           /* pointer, only here */
}
switch (mode) { /* cases index arg_lo / arg_hi DIRECTLY, no intermediate var */ }
```

Three coupled effects, all from this one structural change:

1. **Duplicating the read** pins the `arg_hi`/`arg_lo` offset computations inside their
   branch, so GCC can't hoist them across the switch. The args stay live as direct
   index registers, and the accumulator lands in its target register.
2. **Direct symbol indexing** (`GLOBAL_A[i]`, not `base[i]`) in the symbol-known arm
   makes GCC materialize the case base-pointer as `lui $aN, %hi(GLOBAL_A)` naturally,
   so `base` lands in target's register, which in turn forces the remaining regs.
3. ~~Mark flags re-read across branches/cases `volatile` so GCC re-loads them each time
   (matching target's repeated loads) instead of caching the value in a register.~~
   **❌ FORBIDDEN as of 2026-05-31.** Marking plain game-state globals
   `extern volatile T G;` or via `*(volatile T *)&G` casts is a codegen-coercion
   cheat — same kind as a regfix subst rule, just spelled in C. The engine's
   `volatile_cheats` detector flags every such usage and refuses Tier-4. If
   restructuring the C around the branch (#1 + #2) doesn't yield the target's
   register allocation without the volatile coercion, the function is genuinely
   not pure-C-matchable and should be canonical-asm authorized or parked.

Pins, synthetic switch assignments, and `__asm__` barriers were always tier-3
debt. With the volatile lever (#3) now also forbidden, the remaining legit moves
are: (1) duplicate-the-read structural rewrite, and (2) direct-symbol indexing
in the symbol-known arm. If those don't close it, the function is not Tier-4.

## The meta-lesson -- a register-rename plateau is a STRUCTURAL signal

The permuter and lever-grinding optimize register allocation *within a fixed
control-flow shape*. They cannot discover a different shape. So:

> When a grind plateaus on a hard floor of pure REGISTER-RENAME diffs (0 structural)
> with a documented "cluster coupling" (pinning A breaks B, fixing B breaks C), STOP
> grinding. The C STRUCTURE is wrong, not the allocation. Step back and change how the
> computation is split across control flow.

The "floor" is an artifact of the wrong structure, not a real floor. More pins, levers,
or permuter iterations past this point are wasted effort -- see [[escalation-ladder]]
(switch technique, not target) and `docs/PERMUTER_PIPELINE.md` (isolated score != full
build).

## Worked example: coli_HitPauseKatana_2 (b233af8)

178-insn `replace_with_asmfile` bridge in main.c. A prior session ground it 32 -> 12
diffs over 60+ source variants, a directed search, and 13k+ permuter iterations using
levers (dead-mode-as-`0xFF` scratch, chained `new_var3` copies, temp reuse), then
documented a "4-register-cluster coupling" floor and proposed a CU split. All 12
remaining diffs were pure register-renames.

The fix was the structural rethink above (commit technique = `split-initial-read`):
duplicate the `((base[arg3] & 0xFF) << 16) | base[arg2]` read into the two
`D_800A2CD4 & 1` arms -- direct `D_800F7298[...]` in the set arm, `base[...]` (pointing
at `D_800A2CDC`) in the other -- and **❌ FORBIDDEN PORTION** mark `D_800A2CD4` /
`D_800A28A0` volatile. Result at the time: 0 diffs, 0 pins, 0 regfix, 0 asm.

**Re-evaluation needed (2026-05-31):** with `extern volatile T D_x;` and
`*(volatile T *)&D_x` casts on game-state globals now forbidden,
`coli_HitPauseKatana_2`'s match is NO LONGER VALID under the new bar. The
function needs re-derivation: either find a structural lever that doesn't
require volatile globals, OR canonical-asm authorize. Queue regen will
re-route it to active.

## Related

- [[escalation-ladder]] -- switch technique not target; this is the canonical instance
- [[voice-control-playbook]] / [[gte-3x3]] -- defeat hoisting of CONSTANTS via asm
  barriers (different: those ADD a barrier; this REMOVES the hoist by restructuring)
- [[cheat-cleanup-techniques]] -- statement-vs-branch placement; its "place init after
  the early-exit branch to avoid duplication" note is the inverse of this technique
- [[register-asm-pins]] -- why the pins you reach for first don't stick (cluster coupling)
- [[strength-reduce-defeat]] -- another "the structure forces the codegen" case
