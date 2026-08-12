# func_8003A5A0 — hypothesis frontier

## Status: CLOSED at honest distance 0 (session 1, recon)

All four hypotheses raised this session were measured and CONFIRMED; none remain
open. Full measurements in evidence.md.

| # | Hypothesis | Mechanism | Verdict |
|---|---|---|---|
| H1 | The 21-rule `$2`↔`$3` cluster is ONE allocation decision, caused by a single local being reused as poll temp / timeout temp / return value | `local_alloc` assigns block-local temps before `global_alloc` sees the multi-block pseudo, so the temps take $v0 and the shared local is pushed to $v1, mirroring the function | CONFIRMED — 21 → 16, poll+timeout regions byte-exact |
| H2 | The extra instruction is the shared `epilogue: return v0;` join's `move $v0,$v1` copy | target's `.L8003A6E4` is a bare epilogue, so the return pseudo must already be $v0; per-exit returns materialize it in $v0 directly | CONFIRMED — 16 → 6, 88 → 87 insns |
| H3 | The two `if (s1 >= 5)` exits must share ONE label, not be two inline returns nor two distinct labels | jump2 `find_cross_jump` merges identical `[set $v0,0; jump END]` blocks (and coalesces distinct labels forwarding to the same return); one shared label leaves a block with two predecessors, which reorg can copy-into-delay-slot for the poll branch without deleting it for the loop_check branch | CONFIRMED — 6 → 1 (competing spellings measured at 6, 6, 7) |
| H4 | The final `xor` operand order is `expand_binop`'s commutative swap, not source order | `expand_binop` swaps commutative operands when the destination rtx IS operand 1, to avoid a copy; a one-expression fold gives an anonymous temp as op1 so no swap fires | CONFIRMED — source-order change measured INERT (still 1); one-expression form → 0 |

## Killed / do-not-retry
- Reordering the xor at the SOURCE level (`v0 = a1 ^ v0;`) — measured, no effect.
- Two distinct `return 0;` exit labels to defeat the cross-jump merge — measured 7,
  WORSE than the shared-label form, and it displaces the `match` block's fall-through.
- Both retry exits as inline `return 0;` — measured 6 at 85 insns (2 short).

## If a future session reopens this function
The only work left is integration, which this session is not permitted to do:
retire the 21 now-redundant rules at `regfix.txt:140-160`, run `verify-oracle`, and
`queue done`. If the full-build SHA1 does NOT match after retiring, the first thing to
check is [[global-label-drift-sibling-cheat]] — this edit changes the `.L` label count
in `src/code6cac_c_mid.c`, which can break a LATER sibling's hardcoded-label rule in
the same translation unit.
