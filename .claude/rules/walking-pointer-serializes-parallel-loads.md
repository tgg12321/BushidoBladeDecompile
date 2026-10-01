---
name: walking-pointer-serializes-parallel-loads
paths: [".claude/rules/walking-pointer-serializes-parallel-loads.md"]
description: "Independent parallel-array element stores (G0=r[0]+a[0]; G1=r[1]+a[1]...) whose later loads GCC hoists into delay slots: walk the arrays with post-increment pointers (`*rp++ + *ap++`). Needs intervening stores."
metadata:
  type: reference
---

# Walking pointers serialize parallel-array loads

## Symptom

A straight-line run of independent element computations stored to separate globals:

```c
D_G0 = result[0] + a0[0];
D_G1 = result[1] + a0[1];
D_G2 = result[2] + a0[2];
```

Target emits a strict per-element `lw; lw; nop; addu; sw` with a genuine load-delay `nop`; ours hoists the later
elements' loads into the delay slots (fewer nops, ~1 insn short per element, extra registers). Historically
faked with `asm volatile("" ::: "memory")` barriers or per-load register pins — both forbidden
([[inline-asm-policy]]).

## Fix

```c
{
    s32 *rp = result;
    s32 *ap = a0;
    D_G0 = *rp++ + *ap++;
    D_G1 = *rp++ + *ap++;
    D_G2 = *rp++ + *ap++;
}
```

The post-increment threads a dependence through the pointer, so element i+1's load can't be hoisted above
element i's store. Idiomatic parallel-array C, no coercion. A per-element temp block or operand swap does NOT
serialize (measured). Example: func_80046BF4, 10 → 0.

**PRECONDITION (measured 2026-08-06):** the serialization comes from the intervening memory WRITES between the
loads. Read-only parallel loads feeding one expression get no edge from this (reads don't anti-depend on reads,
and GCC folds constant offsets into the displacement) — the lever cannot apply there.

## Extensions

- **Re-read batch:** use a second walking pointer (`s32 *ap = p, *bp = p;`) for a later re-read of the same
  elements; intervening global stores already block CSE — no volatile needed (func_8006517C).
- **Independent store into a load-delay slot:** load into a named temp, then the required independent store,
  then the dependent store (`t = *ap; D_800F0BC0 = 0; D_800F0D38 = t;`). Legitimate only when the interleaved
  store is a real, always-required instruction — the load-delay analog of
  [[defer-store-past-later-compute-into-jal-delay]].
- **Constant store materialized too early:** move the independent constant store after the last `*ap++` load so
  its `lui/ori` fills the freed delay slots (func_800613C8).

Not this rule: a loop whose barrier blocks a delay-slot steal off a loop exit ([[loop-note-fixes-delay-slot-steal]]).

## Related

[[loop-note-fixes-delay-slot-steal]] · [[store-before-jal]] · [[defer-store-past-later-compute-into-jal-delay]]
