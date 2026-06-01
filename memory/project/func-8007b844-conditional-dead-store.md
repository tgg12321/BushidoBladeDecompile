---
name: func-8007b844-conditional-dead-store
description: func_8007B844 — sandbox 0 reachable via conditional dead-store (cheat-class per Lever D spirit, NOT committed); honest pure-C floor 6 via named-intermediate Lever B
metadata:
  type: project
---

# func_8007B844 — sandbox 0 via conditional dead-store, borderline-cheat (NOT committed)

## Status

**INCOMPLETE — re-parked 2026-06-01.** Prior session park reason
(2026-05-31): 7 honest pure-C diffs in final-block register rotation,
32 structural variants tested, plateau confirmed. This session pushed
the floor 7 → 6 with the named-intermediate mask local (legitimate
[[register-alloc-pure-c]] Lever B-style), then to **0** with a directed
permuter discovery — **but** the closing form is a conditional dead-store
that is structurally the same cheat-class as Lever D (forbidden).

## Levers TESTED this session

### 1. Named intermediate mask local — 7 → 6 (LEGITIMATE, partial)

```c
u32 *func_8007B844(u32 *ot, s32 n) {
    u32 mask;
    if (g_gpu_debug_level >= 2) g_gpu_debug_func(&D_80015F98, ot, n);
    { u32 *v0 = g_gpu_dev_table; ((void(*)(u32*,s32))v0[11])(ot, n); }
    mask = 0xFFFFFF;
    mask = ((u32)&g_gpu_ot_end) & mask;
    *ot = mask;
    return ot;
}
```

`sandbox --disable all` 7→6. Closes the addr-register diff (mine now
in $v1 matching target) but mask lands in $v0 instead of target's $a0,
return staged AFTER store instead of BEFORE. This IS legitimate per
[[register-alloc-pure-c]] and [[narrow-byte-args-packed-call]] (named
intermediate is a recognized lever).

### 2. Directed permuter (clean single-fn target) — found closing form

`permuter/func_8007B844/{target.s,target.o,base.c,compile.sh,base.o}`
set up; ~36k iters across 2 runs. Best legitimate-looking candidate
(score 40 in permuter weighting):

```c
u32 *func_8007B844(u32 *ot, s32 n) {
    u32 *new_var;
    if (g_gpu_debug_level >= 2) {
        g_gpu_debug_func(&D_80015F98, ot, n);
        new_var = ot;       /* <-- CONDITIONAL DEAD STORE */
    }
    { u32 *v0 = g_gpu_dev_table; ((void(*)(u32*,s32))v0[11])(ot, n); }
    new_var = ot;           /* <-- overwrites the conditional store */
    *new_var = ((u32)&g_gpu_ot_end) & 0xFFFFFF;
    return new_var;
}
```

**Sandbox `--disable all` = 0. Retire SHA1 == oracle.** Drops all 6
regfix rules. Verified via `retire func_8007B844` + `verify-oracle --rebuild`.

Emitted asm (idx 21-29) matches target byte-for-byte:
```
lui $a0, 0xff      (mask in $a0 — was $v1)
ori $a0
move $v0, $s0      (return staged EARLY — was AFTER store)
lui $v1, %hi(g_gpu_ot_end)  (addr in $v1 — was $v0)
addiu $v1
and $v1, $v1, $a0  (result in $v1)
sw $v1, 0($v0)     (store via return-value reg)
```

## Why this is NOT committable (cheat-shape per spirit of Lever D)

The conditional `new_var = ot;` inside the `if (debug >= 2)` block has
**zero semantic effect** — `new_var` is unconditionally overwritten by
the subsequent `new_var = ot;` before any use. The store is mechanically
dead. Removing it (or moving the unconditional `new_var = ot;` to the
top to give `new_var` a single live definition) collapses the structure:
GCC folds `new_var` with `ot` (since they're provably equal) and the
function regresses to the 7-diff baseline.

**The lever's MECHANISM is identical to [[register-alloc-pure-c]] Lever D
(now forbidden):** a dead store used to force GCC to keep two pseudos
distinct, breaking value-association in RA. The only differences:
- LHS is a LOCAL (`new_var`), not a function PARAMETER
- Store is CONDITIONAL (inside an `if`), not unconditional
- The local IS used afterward (just gets overwritten first)

The engine's `volatile_cheats.find_dead_param_assigns` detector
(narrow pattern `param = 0` / `param = param` at body root) does NOT
catch this variant. `mark_done` and `retire` accept the source as
COMPLETED-C. `check_completion_integrity` also passes.

But the SPIRIT of [[dead-vars-local-array]] (2026-05-31 expansion) and
[[register-alloc-pure-c]] Lever D rejection (2026-05-31) is that any
dead store used to manipulate codegen is a cheat, regardless of LHS
class or conditionality. A SOTN-quality reader would see `new_var = ot`
inside an `if` block (when `new_var` is unconditionally re-assigned to
`ot` right after the block) as obvious matching-engineering — not what
the original C looked like.

## Levers TESTED that DID NOT close (or regressed)

| Lever | Score | Notes |
|---|---|---|
| naive `u32 *ret; ret = ot; *ret = ...; return ret;` | 7 | folded to baseline |
| `u32 *p = ot;` at top, use `p` throughout | 7 | folded (same as above) |
| `u32 *p; if (cond) {...; p=ot;} else {p=ot;}` | doesn't match | jump-threaded to baseline |
| `u32 mask = 0xFFFFFF;` declared at top | 16 | mask live across calls, spilled |
| `u32 mask; mask = 0xFFFFFF; *ot = ... & mask;` after call | 7 | folded |
| `int new_var; new_var=0xFFFFFF; new_var=addr&new_var; *ot=new_var;` | **6** | named-intermediate Lever B, legitimate (best clean form) |
| Splitting mask + addr into separate locals (mask first / addr first) | 7 | wrong order, regression |
| Goto-wrapping the return BEFORE the store | 9 | extra jumps |
| `if (ot != 0) { dispatch; *ot = ...; }` early-return | 10 | extra branch |
| Comma-operator return: `return (*ot = ...), ot;` | 7 | no effect |
| `r = (u32*)((u32)ot \| 0)` chain | 7 | folded |
| `u32 *ret_val; ret_val = ot;` reorder | 7 | folded |
| Adding `if (cond) return ot;` between call and store | 11 | extra branch |
| `mask` declared as `register u32 mask asm("$a0")` (diagnostic) | 7 | pin stripped by sandbox |
| Two-local form: `u32 mask, *p; mask=0xFFFFFF; p=ot; ...` (no dead store) | 7 | folded |

## Instrumented cc1 evidence (ALLOCDBG)

`BB2_ALLOC_DEBUG=1 tmp/gccdbg/cc1` (instrumented cc1, canonical
untouched at SHA1 `045c9543d3...`) on the standalone base.c:

```
ALLOCDBG ord=0 pseudo=72 hardreg=16 nrefs=5 livelen=19 pri=5263
ALLOCDBG ord=1 pseudo=73 hardreg=17 nrefs=3 livelen=12 pri=2500
```

Only pseudos 72 (ot→$s0) and 73 (n→$s1) go through global.c — they
cross the calls. The post-call pseudos (mask=74, addr=82, and-result=83)
are local-alloc-only. Per `local-alloc.c:block_alloc`, allocation is by
ascending hardreg preference, so:
- Mask born first (insns 70/71), gets $v0... but actually gets $v1 because
- Addr (82) gets $v0 first (lower LUID — assigned in source AFTER mask
  but scheduled earlier by sched.c)
- Mask and addr conflict at insn 54-55 (both live for the `and`)

For target's allocation (mask in $a0, addr in $v1):
- $v0 must be UNAVAILABLE for both → occupied by something else
- The only candidate is the return-value pseudo (insn 60: `(set (reg/i 2 v0) (reg 16 s0))`)
- Insn 60 must be SCHEDULED earlier (before insn 54)
- LUID of insn 60 is determined by source position of `return ot;`
- Source position can't be moved BEFORE `*ot = ...;` without conditional gotos
- BUT the conditional dead-store creates RTL state where pseudo `new_var` is
  separate from `ot`-pseudo, causing additional dataflow that shifts the
  scheduler's chains and emit-ordering

The mechanism is: **conditional dead-store breaks GCC's CSE of `p == ot`
at the merge point**, keeping `p` as a separate pseudo with a different
live-range. This shifts the scheduler's INSN_PRIORITY computation,
causing insn 60 (return staging) to schedule before insn 54 (addr load).

## Permuter exhaustion in this session

- 15 min of 4-worker + 10 min of 6-worker runs at iteration speeds of
  ~10/s/worker = ~36000 iters total
- 4 best-score candidates saved (`output-40-1`, `output-135-1/2`,
  `output-140-1`)
- The score-40 candidate is the closing dead-store form documented above
- The score-135 candidates introduce semantic-changing side-effect tricks
  (`(new_var2 = &g_gpu_ot_end)` inline assignment that redirects the
  store target — semantic break, NOT a match)
- No legitimate Lever-A/B/C-class candidate that closes the gap was found

## Resume avenues (un-tried, escalation territory)

1. **Project-wide rodata reorder or source-file re-attribution.** Same
   class as the `saEft00Add` precedent. Not applicable here (no rodata
   adjacency evidence for this function).
2. **Canonical-asm authorization.** scan_hand_coded says LOW (0/8
   signals); the function lacks the multu pacing, hardcoded register
   patterns, or other indicators that justify auth as hand-coded. So
   this is NOT a candidate for canonical-asm under
   [[hand-coded-asm-recognition]].
3. **Genuinely-new C structural lever** that's NOT in the dead-store
   family. The remaining lever-space:
   - Use a real function call between the dispatch and the store that
     returns `ot` (none exists in the codebase for this signature)
   - Restructure the function so it doesn't return `ot` but a
     different value (semantic change, not viable)
   - Use a static or global mediator (over-engineering, not a sibling
     pattern)
4. **User policy decision on the conditional-dead-store class.** If the
   user explicitly sanctions the conditional `local = param` pattern
   as legitimate (distinct from Lever D's unconditional `param = param`),
   the score-0 form would close. This is a category-call, NOT a worker
   decision.

## Conclusion

Honest pure-C floor for func_8007B844: **6** (legitimate, via named
intermediate `u32 mask` Lever B). The score-0 closing form discovered
this session is a conditional dead-store cheat-class, not committable
per the spirit of [[register-alloc-pure-c]] Lever D and
[[dead-vars-local-array]]. The function stays INCOMPLETE in the queue;
no new park category needed (per [[no-new-park-categories]] — this is
"the search continues" state, not a sanctioned cheat-class).

Permuter artifacts preserved at `permuter/func_8007B844/`. The cheat-
class candidate at `permuter/func_8007B844/output-40-1/source.c` is the
proof point — clean C reaches sandbox 0 + SHA1 == oracle, but the C
itself is borderline.
