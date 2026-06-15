# func_8003B20C (code6cac_c_ab.c) — WIP (floor 6)

**HEAD:** carries `register s32 one asm("v1") = 1;` pin AND a dead
`u8 *new_var; ...(new_var = &D_800900EC)[arg0]` alias. Both cheats
([[inline-asm-policy]] pin + [[inline-asm-injection]] dead-alias). Removing
both gives the honest pure-C floor **6** (the cheats make HEAD oracle-clean,
not pure-C-clean).

## TL;DR for resume
1. Apply `candidate.c` verbatim into `src/code6cac_c_ab.c` (replace the
   `func_8003B20C` body). It is pure C, no cheats.
2. Confirm `& tools/eng.ps1 sandbox func_8003B20C --disable all` = **6**
   (build_insns = target_insns = 47).
3. The residual 6 diffs are **pure scheduling** — GCC's sched.c INSN_PRIORITY
   hoists the array load 3 positions above the `D_80102780/D_80102781` byte
   stores. All registers match target ($v0=0xFF, $v1=1, $a0=arg0_new); only a
   position swap of two clusters. Diff (target left, mine right):

```
   target            mine
   sb zero (780)     addu at,at,a0
   lui at, (781)     lw a0, 0(at)     <- array load hoisted
   sb v1, 0(at)      li v0, 0xFF
   lui at, (900EC)   lui at, hi(780)
   addu at, at, a0   sb zero, 0(at)
   lw a0, 0(at)      lui at, hi(781)
   li v0, 0xFF       sb v1, 0(at)
```

## The gap mechanism
The array load `D_800A3844 = ((s32*)&D_800900EC)[arg0]` feeds a long chain
(store to D_800A3844 -> reload -> `func_8003AE5C` across `file_LoadOverlay`),
so sched.c gives it higher priority than the constant byte-stores (whose chain
dies at the call's memory clobber) and emits it earlier. Target has source
order (stores, then load). HEAD's pin + dead alias anchor the load at a low
LUID to defeat the hoist; both forbidden.

## Why no pure-C lever closes it
All tried forms HOLD at 6 or REGRESS. Full enumeration + scores in
`meta.json:rejected_forms` (git history has the per-session prose). Summary of
the families ruled out across sessions 1-3:
- **Source-statement reorder** (load moved to position 3): HELD 6 — sched.c
  ignores source position for this hoist.
- **Fresh local / const local pointer / const FILE-SCOPE pointer**: HELD 6 —
  combine folds them all back to the symbol-ref (identical asm).
- **Byte-pointer math** `*(s32*)(base+arg0*4)`: REGRESS 9 (extra addressing).
- **Explicit arg0-reassign / func_8003AE5C(arg0) local**: 9 / 14 (pseudo
  competition + callee-save reservation).
- **Random-mode permuter** (5000+ iter): only cheat-class candidates
  (do-while-zero misapplied, u8/short truncation, no-op XOR, DImode chain,
  caller-breaking signature change). Random mode EXHAUSTED.

## Resume avenues (un-mined)
1. **Directed PERM_GENERAL** — hand-author statement-permutation alternatives
   in base.c (the 9 byte-store statements + the array-load statement are all
   semantically commutative — disjoint globals) and let permuter explore the
   legitimate space. Random mode is exhausted; this is the un-mined surface.
2. **Instrument cc1 sched.c** (BB2_SCHED_DEBUG, `tmp/gccdbg/cc1` from the
   saEft00Add work, [[cross-jump-store-tail-merge]]) — dump INSN_PRIORITY/LUID
   of `lw $a0` (array load) vs `sb $zero` (D_80102780). Concrete question: does
   the byte-store lose on chain-cost or on the LUID tiebreaker? That tells you
   what a legitimate C construct would have to change.
3. **cc1psx calibration** — NOT yet run here. saEft00Add_sub (same sched.c
   hoist family) showed BOTH decompals cc1 AND PsyQ cc1psx produce the same
   reorder. If that holds here too, the original used an unreconstructed source
   idiom (or the divergence is pipeline-stage) — worth confirming.
4. **SOTN/community cross-ref** — overlay-load funcs (src/main/, src/dra/,
   src/st/) for `byte-stores-then-array-load-then-call` shapes.

DONE-NEGATIVE: const file-scope alias (#3 old list, closed 2026-06-14).

## Cheat-reviewer status
NOT invoked — candidate is honest pure-C (would PASS) but isn't committed to
`src/` (breaks oracle by 6 insns). Invoke at close-out if a sandbox-0 form is
found.

## Related
- [[no-new-park-categories]] / [[inline-asm-policy]] / [[inline-asm-injection]]
  — the cheats-by-spelling policy and the HEAD pin + dead-alias categories.
- [[store-before-jal]] — confirms the `D_800A3844 = ...; func_X(D_800A3844)`
  memory-reload is the correct target pattern (don't carry arg0 across the call).
- [[difficult-is-not-impossible]] — floor WAS lowered (cheats stripped); the
  residual 6 is genuinely unfinished, not impossible.
