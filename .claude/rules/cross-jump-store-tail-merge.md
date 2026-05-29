---
name: cross-jump-store-tail-merge
paths: ["src/*.c"]
description: "target has more `sw GLOBAL` stores (or more `j SAME_LABEL` error tails) than your build: GCC 2.7.2 jump2 cross_jump merged N identical `[sw GLOBAL; j END]` tails into one block. FIX: give the error paths a MIX of exit forms (distinct `goto endK; ... endK: return G;` labels + one inline `return G;`) so the block ENDINGS differ -> suffixes not rtx_equal -> no merge."
metadata:
  type: reference
---

# The cross-jump STORE-tail merge wall — SOLVED (mix the exit forms)

The store-tail analogue of [[cross-jump-call-merge]] (which uses arg COUNT to keep
CALL suffixes distinct). Here the merged suffix is a global STORE + jump, and the
lever is the **block ending** (jump vs return vs distinct-label jump), not arg count.

## Symptom

A function with several early-exit paths that each write the SAME constant to the
SAME global and then return it through a shared end, e.g.

```c
if (...) { ...; D_GLOBAL = -1; goto end; }   // tail 1
if (...) { ...; D_GLOBAL = -1; goto end; }   // tail 2
...                          D_GLOBAL = -1; goto end;   // tail 3
end: return D_GLOBAL;        // re-reads the global (value unknown -> reload)
```

Your pure-C body is structurally correct but plateaus. **The tell: `target` has MORE
`sw $v0,D_GLOBAL` stores than your build** (target keeps N inline stores; your build
collapsed them to 1 shared store block). Equivalent tells: fewer `j` to the shared
end label; a `.LNNN:`-labelled shared store block in your maspsx that the target
does not have.

## Diagnostic (decisive)

Count the stores to the global in target vs your cheat-free build object:

```python
# target: build/src/<file>.o ; yours: tmp/sandbox/<func>/<file>.o
re.findall(r'sw\t\$\d+,D_GLOBAL', objdump_dis)   # target N, yours < N  => merged
```

Or watch it collapse across RTL passes (`cc1 ... -da`): the N stores survive
`rtl/jump/cse/loop/combine/sched/flow`, then **`jump2` drops them to 1**. That is
`jump_optimize(..., cross_jump=1)` -> `find_cross_jump` (jump.c ~2371) matching the
2-insn suffix `[sw GLOBAL ; j END]` (minimum=2, both insns rtx_equal -> merge).

## Why it merges (the mechanism)

`find_cross_jump` walks backward from each `j END` and matches identical insns; the
shared `[sw $v0,D_GLOBAL ; j END]` is exactly `minimum=2` -> merges into one block,
the others redirected into it. All N tails being byte-identical (`li v0,-1; lui at;
sw v0,%lo(G); j END; nop`) is precisely what triggers it. Even feeding GCC the
TARGET's own m2c-reconstructed structure merges under our fork — so the C must make
the tails *not* mergeable.

You CANNOT defeat it by:
- inserting a dead pure-C statement after the store (DCE'd before jump2 -> no effect);
- a `*(volatile*)` store (stays separate but emits wrong `la`-form addressing -> a
  different diff; this is the tier-3 cheat the de-cheat is removing);
- distinct goto labels that all forward to `return G` (GCC coalesces e1/e2 back into
  one block -> re-merges the pair).

## The lever: MIX the exit forms so block ENDINGS differ

cross_jump matches blocks with rtx_equal suffixes that jump to the SAME label.
If the tails END differently, the suffix match breaks (only the bare jump, or
nothing, matches -> < minimum -> no merge). The combination that keeps ALL N
stores separate AND preserves the shared re-reading end:

```c
if (...) { ...; D_GLOBAL = -1; goto end1; }   // tail 1: distinct-label goto
if (...) { ...; D_GLOBAL = -1; return D_GLOBAL; } // tail 2: INLINE return
...          ...; D_GLOBAL = -1; goto end3;   // tail 3: distinct-label goto
...success...;                    goto end4;
end1: return D_GLOBAL;
end3: return D_GLOBAL;
end4: return D_GLOBAL;
```

The inline `return G` block ends with the epilogue thread; the `goto endK` blocks end
with a jump to a distinct label — the endings are not rtx_equal, so none merge. Each
emits its own `li v0,-1; lui at; sw v0,%lo(G)` (all in `$v0`, matching target) and the
shared `endK: return D_GLOBAL` still RE-READS the global (`lw v0,%lo(G)`), because the
value at the merge point is genuinely unknown (errors store -1, success stores another
value). Pick which path is the inline `return` vs which are distinct-label `goto` by
matching the TARGET's per-tail register usage (all `$v0` is the goal; a tail that comes
out in `$v1` means that path should switch goto<->return).

## Confirmed case — saEft00Add (src/system.c, 2026-05-28)

Three error paths each `D_800A14E4 = -1` then return the re-read global; success path
`D_800A14E4 = D_800A14D0`. Original committed source used `goto end` for all three +
a `*(volatile*)` store on tail 1 + an `__asm__("")` barrier on tail 2 (the barrier is
an opaque insn that survives to jump2 and breaks the suffix) + regfix to fix the
volatile addressing — all tier-3 debt. Pure-C distance with those stripped: 15, of
which the whole gap was the 3-way store-tail merge.

The mix (tail1 `goto end1`, tail2 inline `return`, tail3 `goto end3`, success
`goto end4`, all labels `return D_800A14E4`) produced 4 separate `$v0` stores +
shared `lw` re-read = the target's exact branch-store region. Honest distance 15 -> 6.
(The residual 6 was a separate reorg.c delay-slot eager-fill, not this merge.)

## Related

- [[cross-jump-call-merge]] — the CALL-suffix analogue (arg count is the lever there).
- [[shared-end-label]] — the INVERSE situation (you ADD a shared end to force a merge
  / defeat per-case constant folding); read it to keep the two straight.
- [[difficult-is-not-impossible]] — this was first (wrongly) called a "fork wall";
  the matching C existed. Don't certify a cross_jump merge as impossible.
