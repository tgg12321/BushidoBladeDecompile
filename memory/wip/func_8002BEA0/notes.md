# func_8002BEA0 WIP — floor 9 → 4

**Status:** WIP checkpoint, 2026-06-13. Cheat-reviewer PASS on candidate.

## TL;DR

Cluster sibling of **func_8001A67C / func_800274BC** (both user-
authorized 2026-06-10 for the GTE LZCS/LZCR block: $t4 reused for two
unrelated values, 2 unfilled GTE delay nops, splat tags cop2 ops
"handwritten instruction"). This session mirrored the sibling pattern in
pure C + canonical inline asm and retired 5/7 cheat constructs, dropping
honest cheat-free pure-C distance **9 → 4** (sandbox --disable all,
build_insns 131 == target_insns 131, 0 regfix rules in effect on the
candidate's build). Remaining 4 is a single coupled cluster in the
`/100` strength-reduce subtree.

## Changes vs HEAD

| HEAD | Candidate | Status |
|---|---|---|
| `register s32 t4_v asm("t4");` | `"$12"` clobber inside the consolidated asm block | Pin removed |
| Two `__asm__ volatile("nop");` standalone | Nops inside the consolidated GTE block | Cheat-asm absorbed |
| Two separate `.word` GTE blocks | One `__asm__` block (sibling form) | Sibling pattern |
| `__asm__ volatile("" : "=r"(temp_v0) : "0"(temp_v0));` | Block-scoped `s32 temp_v0 = 0x3E8` grouping the 4 divisions | Barrier removed |
| `do { v0_m = ...; var_t0 = (hi << 16) >> v0_m; } while (0);` | Direct expression (sibling form) | do-while-0 retired |
| Function-scope `temp_v0`, `temp_v1_3` | Block-scoped to the divisions block | Live ranges narrowed |

**Transparency note (reviewer flag):** the trailing block-scope on
`temp_v0` / `temp_v1_3` IS the replacement for the removed scheduling
barrier. The motive is grouping the 4 divisions that share both operands;
the live-range narrowing is a side effect. Both variables are genuinely
used in 4 active arithmetic operations.

## Remaining 4 — the `/100` strength-reduce cluster

Target at insns 54-64:

```
54:  lui   $a0, hi(0x51EB851F)   <-- delay slot
55:  ori   $a0, $a0, lo(0x51EB851F)
56:  addiu $v1, $zero, 0x44C
... subu/sll/addu/sll/mult/sra ...
63:  mfhi  $t8                    <-- target picks $t8
64:  sra   $v1, $t8, 5
```

Candidate at the same positions:

```
48:  li    $v1, 1100              <-- delay slot picks addiu
49:  lui   $a0, 0x51eb
50:  ori   $a0, $a0, 0x851f
... mfhi  $t5                      <-- our build picks $t5
    sra   $v1, $t5, 5
```

Two separable diffs in the same RTL subtree:

1. **Delay-slot fill order** (3 reordered). reorg.c picks the FIRST
   candidate from the fall-through BB; build's first is `addiu $v1,
   0x44C`, target's first is the synthetic magic-constant `lui $a0`.
2. **mfhi destination RA** (1 reg diff). The synthetic intermediate
   holding `mfhi`'s output has no C variable; local-alloc picks from
   the free list. Build's $t0..$t4 live → $t5 free; target's $t0..$t7
   live → $t8 free. Difference is one extra register live across the
   strength-reduce.

7 in-C variants tried this session (see meta.json `rejected_forms`);
none shifted either diff.

## Why this isn't park material

[[difficult-is-not-impossible]] applies. Concrete unrun probes:

- **ALLOCDBG-instrumented cc1** (`tmp/gccdbg/cc1` with BB2_ALLOC_DEBUG=1)
  on the mfhi pseudo — reveals which conflicts forced $t5.
- **BB2_SCHED_DEBUG** on the if-branch BB — reveals reorg.c's
  candidate list and why addiu wins over lui.
- **Sibling cross-ref** with `func_8002BC68` (same file, same cluster
  with $2↔$3 swap + same mfhi $13→$24 rename) — paired greg dumps may
  surface a shared lever.
- **Variants that raise register pressure before the divide** so
  `mfhi` lands at $t8 naturally.

If the in-C levers measurably exhaust, this is the textbook cluster
sibling of the two already-authorized LZCS functions — surface to user
for canonical-asm cluster authorization per HARD RULES (no self-resolve;
LZCS family is not in the orchestrator auto-auth bucket, only GTE leaves
are).

## Notes for the next agent

- **Resume from `candidate.c`, NOT HEAD.** Apply candidate.c (function
  span lines 811-872 in HEAD; verify with `grep -n "^s32 func_8002BEA0"
  src/code6cac_b.c`), confirm `sandbox --disable all` returns 4, then
  iterate.
- **Don't re-derive `rejected_forms`** — 7 variants with measured
  negative deltas; pick from `next_hypotheses` instead.
- **Cheat-reviewer PASS** recorded with one transparency note (see
  meta.json `reviewer.evidence`).

## Related

- [[difficult-is-not-impossible]] · [[canonical-gate-distance-not-evidence]]
- [[register-alloc-pure-c]] · [[no-new-park-categories]]
