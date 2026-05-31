---
name: compare-operand-order-register
description: Reversing a comparison's operand order (write `local > GLOBAL` instead of `GLOBAL < local`) can flip which register cc1 picks for the local; one source edit retires a `$X <-> $Y` regfix rule cluster
paths: ["src/*.c"]
---

# Reversing a comparison's operand order can flip which register a value gets

## Symptom

A function carries one or more regfix `$X <-> $Y` register-rename rules over a
range of indices (e.g. `func: $4 <-> $5 @ 14-37`) — possibly with companion
`subst` rules — covering a check block of paired comparisons like:

```c
w = rect[2];
if (D_8009BE78 < w) goto bad;     /* GLOBAL < local */
y = rect[1];
if (D_8009BE7A < y) goto bad;
```

cc1 picks `$a0` for `w` and `$a1` for `D_8009BE78` (the global address goes into
its own register first, the local `w` then takes the LOWER-numbered register
allocator preference). Target has `w` in `$a1` and `D_8009BE78` in `$v1`. The
regfix rules swap the register assignments after the fact.

## The lever — write `local OP global`, not `global OP local`

Flip the operand order so the LOCAL variable is the LEFT-HAND operand and the
global is the RIGHT-HAND. Use `>` / `>=` etc. instead of `<` / `<=` to preserve
semantics:

```c
w = rect[2];
if (w > D_8009BE78) goto bad;          /* local > GLOBAL — same semantics */
```

This changes cc1's RTL evaluation order: the local (`w`) is loaded into a
register first, and the global is loaded into a register second. cc1's
allocator then picks the natural ascending sequence ($a1 for `w`, $v1 for the
global), matching target's layout. **One source edit retires the entire
`$X <-> $Y` register-rename rule cluster** covering that check block.

## Why this works (cc1 RTL gen, not scheduler)

cc1 lowers `D_GLOBAL < w` and `w > D_GLOBAL` differently at the RTL stage:
the LHS gets RTL-allocated first, biasing the register-allocator's
preference. `<` and `>` have different `mips_emit_compare` paths that
materialize the comparison's operands in a different order — even though
the integer result is identical. The downstream allocator then picks
ascending register numbers per the natural pseudo-creation order.

This is NOT a scheduling lever (sched/reorg don't see the swap); it's a
RTL-emission-order lever. So it survives `-O2` + scheduling without being
folded.

## Confirmed case — func_8007B3A8 (display.c, 2026-05-31)

Queue top, verdict C, distance 15 (cheat-free), 5 regfix rules (`$4 <-> $5
@ 14-37` + 4 `subst` companions) + 1 register-asm pin (`register asm("$3")`)
on the second-call fn_ptr. The check block has 4 paired `GLOBAL < local`
comparisons (`D_8009BE78 < w`, `D_8009BE78 < w+x`, `D_8009BE7A < y`,
`D_8009BE7A < y+h`). Flipping all four to `w > D_8009BE78`, `w+x > D_8009BE78`,
`y > D_8009BE7A`, `y+h > D_8009BE7A` — `sandbox --disable all` 15 → 5; then
the remaining 5 was the second-call delay-slot pattern, closed by inlining
the call into both `bad:` and `level_2:` paths (cc1 cross-jump-merges them
back but allocates fn_ptr into `$v1` because rect[3] stays alive in `$v0`,
allowing the `sw v0, 16(sp)` to fill the jalr delay slot, matching target's
[[store-before-jal]] pattern). Net: 5 regfix rules + 1 pin retired.

A trailing `goto label_pad; label_pad: goto end;` dead-pad was added to
restore the global `.L` label count to HEAD's value (the cross-jump merge
shaved off one label number) — see [[global-label-drift-sibling-cheat]].

## When this does NOT apply

- The comparison is genuinely symmetric in target — e.g. both `slt rd, rs, rt`
  and `slt rd, rt, rs` would emit, depending on context. Check target.s to
  confirm the actual operand order.
- The function uses the local as a loop counter or accumulator — swapping
  operands may change loop-form codegen ([[loop-rotation-two-shift]]).
- The comparison is against `0` (e.g. `if (x < 0)` vs `if (0 > x)` — both
  emit `bltz`).

## Related
- [[register-alloc-pure-c]] — the broader RA-via-C-structure playbook; this is
  another lever in that family.
- [[register-asm-pins]] — pins are diagnostic; finding the C structure that
  makes the pin's target register the natural choice IS the work.
- [[global-label-drift-sibling-cheat]] — the dead-label-pad technique for
  restoring HEAD's `.L` count when restructuring a function shifts it.
- [[store-before-jal]] — the related "delay-slot fill for stack arg" pattern
  this fix also enables (by keeping `$v0` busy through the fn ptr load).
