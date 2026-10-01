---
name: cross-jump-store-tail-merge
paths: [".claude/rules/cross-jump-store-tail-merge.md"]
description: "Target has more `sw GLOBAL` error-tail stores than your build: jump2 cross-jump merged identical [sw G; j END] tails. Mix exit forms (distinct goto endK labels + one inline return) so endings differ."
metadata:
  type: reference
---

# The cross-jump STORE-tail merge wall — mix the exit forms

The store-tail analogue of [[cross-jump-call-merge]]: here the lever is the **block ending**.

## Symptom

Several early-exit paths each write the SAME constant to the SAME global and return through a shared
end that re-reads it:

```c
if (...) { ...; D_GLOBAL = -1; goto end; }
if (...) { ...; D_GLOBAL = -1; goto end; }
...          D_GLOBAL = -1; goto end;
end: return D_GLOBAL;
```

The tell: **target has MORE `sw $v0,D_GLOBAL` stores than your build** (or fewer `j` to the shared
end; a shared `.LNNN:` store block target lacks). In `cc1 -da` dumps the N stores survive until
`jump2`, which drops them to 1: `find_cross_jump` (jump.c ~2371) matches the 2-insn suffix
`[sw G ; j END]` (minimum 2).

Does NOT work: a dead statement after the store (DCE'd before jump2); a `*(volatile*)` store
(wrong addressing, and volatile coercion is a cheat); distinct labels that all forward to
`return G` (coalesced back).

## The lever: MIX the exit forms so block ENDINGS differ

```c
if (...) { ...; D_GLOBAL = -1; goto end1; }        /* distinct-label goto */
if (...) { ...; D_GLOBAL = -1; return D_GLOBAL; }  /* INLINE return */
...          ...; D_GLOBAL = -1; goto end3;
...success...;                    goto end4;
end1: return D_GLOBAL;
end3: return D_GLOBAL;
end4: return D_GLOBAL;
```

Endings that are not `rtx_equal` cannot merge, so each tail keeps its own `li v0,-1; sw v0,%lo(G)`,
and the ends still re-read the global. Choose which path is the inline `return` by the TARGET's
per-tail register usage (a tail landing in `$v1` should switch goto↔return). SOTN ships mixed exit
forms verbatim (`SsVabOpenHeadWithMode`, `src/main/psxsdk/libsnd/vs_vh.c`) — a frozen-list entry in
[[no-new-park-categories]].

Example: saEft00Add (system.c) — three error tails + success path; the mix took the honest distance
15 → 6. Its remaining residual is a coupled reorg.c × cross-jump fixpoint: read
`pre-slim-2026-10-01:memory/project/cross-jump-store-tail-deep-dive.md` before
re-attempting it.

Related: [[cross-jump-call-merge]] · [[shared-end-label]] (the inverse: ADD a shared end to force a
merge) · [[no-compiler-divergence]]
