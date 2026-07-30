# Hypothesis ledger — cpu_set_move_command_and_dir_for_no_action

## Session 1 (2026-07-30, recon) — 4 hypotheses raised, 4 CONFIRMED, floor 18 → 0

### H1 — CONFIRMED (18 → 12). The 1-instruction deficit is a CSE-merged duplicate constant.
Statement: the build is 188 insns vs target 189 because target materializes the
constant `1` twice (`li a2,1` for the loop's opaque `one`; `li v0,1` for the
`~(1 << shift)` mask) while our build materializes it once and reuses the
register. The fix is statement order, not a barrier.
Mechanism: cse.c reuses a live pseudo already holding the same constant. With
`one = 1;` placed BEFORE the mask expression, `one`'s pseudo is live at the
mask's expansion point and cse rewrites the mask shift to use it
(`sllv v0,a2,v0`), deleting one `li`. Placed AFTER, the mask's own constant
register has already been clobbered by the `sllv` result it feeds, so cse has
nothing to reuse and both `li 1`s survive. The second `li 1` additionally
fills the `lbu v1` load-delay slot, exactly as target does.
Probe: delete the `__asm__ volatile("" : "=r"(one) : "0"(one));` barrier and
move `one = 1;` below `bits = D_80106A50 & mask;`.
Result: `sandbox --disable all` 18 → 12; `build_insns` 188 → 189; cluster B
(target idx 16-22) matched exactly including registers.

### H2a — CONFIRMED (12 → 8). Prologue insn order follows source statement order.
Statement: target emits `li a2,1` before the `la D_801077B0` pair; our build
emitted them in the opposite order because `ptr = &D_801077B0;` preceded
`one = 1;` in the source.
Mechanism: sched1 ready-list priority for these independent single-def chains
follows source LUID order; there is no dependence to break the tie.
Probe: swap the two statements so `one = 1;` comes first.
Result: 12 → 8.

### H2b — CONFIRMED (8 → 4). The 0x3EF3DF constant hoists above the callee-save stores only if the mask is the FIRST body statement.
Statement: target puts `lui a0`/`ori a0` at idx 1-2, immediately after
`addiu sp,sp,-0x28` and ahead of `sw s2`/`move s2,zero`/`sw s1`/`move s1,zero`;
our build put the pair 4 slots later because `count = 0; i = 0;` preceded the
mask computation in source.
Mechanism: same sched1 priority-by-source-order effect, but strong enough here
to let the constant chain interleave ahead of the prologue's save stores.
Probe: declare `s32 bits;` separately, compute `mask` as the first statement of
the body, then `count = 0; i = 0;`, then `bits = D_80106A50 & mask;`.
Result: 8 → 4; cluster A empty; the `reorder ... @ 1-11` regfix rule is now
redundant.

### H3 — CONFIRMED (4 → 0). The tail's branch sense and `val`'s register are ONE defect: the shared-tail label placement.
Statement: target idx 162 `beqz v0,<end>` (branch AWAY from the append work)
plus `val` in `v0`; our build had `bnez v0,<work>` plus `val` in `a0`. Both
follow from the `append_last:` label being nested inside the else-arm's `if`
body instead of sitting in a shared trailing block.
Mechanism: with the label inside one arm, jump.c/reorg.c lay the then-arm out
as a branch TO the work with a `j` to the end, and `val` becomes a fresh pseudo
that lands in `a0`. With a shared tail, both arms become branch-to-end
(fallthrough into the work in the else-arm, `j` into it from the then-arm) and
`val` reuses the just-tested, now-dead `and` result register `v0`.
Probe: hoist `append_last:` out of both arms; assign `val` inside each arm's
`if`; add an explicit `goto set_count;` for the append-nothing path.
Result: 4 → **0**. All four residual diffs closed simultaneously, confirming
they were a single shape defect rather than four independent ones.

## Frontier for any follow-up session
**Empty for pure-C purposes — the function is at distance 0 and the candidate
is banked.** The only remaining work is the operator/driver step this session's
contract forbids: retire regfix.txt lines 2018-2021 (`engine retire
cpu_set_move_command_and_dir_for_no_action`) and confirm
`verify-oracle` SHA1 == `62efab4f73f992798c43e8c730aa43baa10bb4fa`. If retire
rolls back despite sandbox 0, read `.claude/rules/sandbox-zero-retire-fails.md`
— but note this session verified zero real diffs instruction-for-instruction
against `asm/funcs/<func>.s`, not merely a masked score of 0, so a masked-zero
false positive is unlikely.

## Rejected forms bank
None. Every form probed this session lowered the floor; no disproven variants
were produced, so `rejected/` is intentionally empty.
