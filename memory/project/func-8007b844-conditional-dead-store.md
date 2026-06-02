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

## Session 3 (2026-06-02) — re-park under SOTN-aligned policy

Today's policy updates (commit 161c6c3) opened six previously-borderline
techniques as ALLOWED based on SOTN master-branch evidence:
- defeat-licm var-reuse
- opaque arithmetic (`s32 one = 1;` and `(R()&3)+1-1;`)
- sub-word param read casts (`*(u16*)&local`)
- mixed exit forms (goto endK mixed with inline return)
- duplicate-read into branch arms
- named-intermediate declaration-order

Re-attempted this function applying each allowed lever. Honest floor
remains 6 (Lever B baseline). 17 new structural variants tested:

| variant | score | notes |
|---|---|---|
| v1: named mask + named addr | 7 | folded (addr inline) |
| v3: u32 *p = ot pointer | 7 | folded (CSE) |
| v4: goto end + mixed-exit on Lever B | 6 | same |
| v5: opaque `*(ot+zero)` | 6 | zero folded |
| v6: duplicate-read into 2 branch arms | 22 | regress (CJM cost) |
| v7: u32 *ret_val = ot; decl early | 6 | same |
| v8: reuse single p for dev_table + ret | 6 | dead store DCE'd |
| v9: opaque 0xFFFFFF+1-1 | 6 | constant folded |
| v10: block-scoped result + mask decl order | 6 | same |
| v11: opaque addr (u32)&end + (u32)zero | 7 | regress |
| v12: hi\|lo mask split | 7 | folded |
| v13: opaque +1-1 on mask | 6 | folded |
| v14: store inside dispatch block | 20 | regress |
| v15: mask declared at top | 19 | regress (spilled) |
| v16: reuse n param for mask (var-reuse) | 17 | regress ($s1 cascade) |
| v17: reuse v for dev_table + mask | 10 | regress BUT move v0,s0 NOW BEFORE store; lever IS real |
| v18: keep vp scope + reuse v | 6 | DCE'd dead store, back to Lever B |

Permuter from Lever B base (`permuter/func_8007B844/base.c` updated
to mask-reuse form): ~12,845 iters at base score 135, zero candidates
beat 135. Lever space is fully exhausted by randomization from this
base.

### v17 is the lever's ghost — close but mis-allocated

`u32 v; v = (u32)g_gpu_dev_table; ((vp_t)((u32*)v)[11])(ot,n); v = 0xFFFFFF; v = addr & v; *ot = v; return ot;`
emitted asm shows `move $v0, $s0` (return staging) at idx 27, BEFORE
the store at idx 28. That's the target's structural order. But mask
landed in $v1 (target $a0), addr in $v0 (target $v1), store via $s0
(target via $v0). The lever shifted ONE cycle in the right direction;
two more cycles short. Adding more reuse hops to extend the chain
either DCE's the dead stores (back to Lever B) or emits real
instructions (regression).

### Mechanism re-confirmed at the cc1 level

Target requires `move $v0, $s0` scheduled at idx 23 BEFORE the
mask/addr chain (idx 24-26). cc1's sched.c gives the return-staging
insn priority 1 (chain depth to `jr ra` is 1). The mask/addr chain
has priority 4 (lui→addiu→and→sw). For sched.c to pick return-staging
first, its priority must be ≥4 — which requires the store to USE the
return-value pseudo (so return-staging is a producer for the store,
extending its chain depth).

In pure C, this means `*p = ...; return p;` where `p` is a SEPARATE
pseudo from `ot`. Every form tested either:
- copy-propagates `p` to `ot` (Lever B and all v3/v7/v8/v18 variants);
- emits real extra instructions for the separation (v6, v14, v15);
- relies on a dead store / conditional store / fn-ptr lie to defeat
  copy-prop (FORBIDDEN — same cheat family as Lever D dead-param-
  assign, archived 2026-05-31).

### Verdict (session 3)

Honest pure-C floor for func_8007B844: **6** (legitimate, via
named-intermediate `u32 mask` Lever B). The SOTN-allowed lever set
opened by today's policy update does NOT contain a construct that
closes the score-6 → 0 gap. All previously-rejected closing forms
(conditional dead-store, function-pointer return-type lie) remain
FORBIDDEN per the expanded cheat catalog.

Function stays INCOMPLETE; no new park category requested per
[[no-new-park-categories]]. Honest floor 6 stable across 3 evidence-
driven C-exhaustion sessions (50+ structural variants + ~50k
permuter iters total).

Resume avenues for a future session (BOTH require user policy
decisions — no further worker grinding will close it):
1. **Canonical-asm authorization** (per [[hand-coded-asm-recognition]]):
   scan_hand_coded LOW (0/8 signals) — formally doesn't qualify under
   the gate. Three consecutive sessions of evidence-driven C-exhaustion
   may warrant a policy-level discussion.
2. **Engine-level lever discovery**: an additional SOTN-aligned
   technique not yet identified by the 2026-06-02 borderline-research
   pass. Would need fresh community-master research.

Permuter artifacts preserved at `permuter/func_8007B844/` (base now
reflects Lever B form). Session-3 variant ledger preserved at
`tmp/b844/v*.c` (gitignored).

## Session 2 (2026-06-01) — re-park with NEW evidence

Honest floor confirmed still 6 (Lever B). ~26 additional structural
levers tested this session, none cleanly closing below 6. Detailed log:

### Round 1-2 (baseline + ret_val staging variants, 9 forms)
| Variant | Score | Notes |
|---|---|---|
| `goto end; end: return ret_val;` (per goto-end-prologue-delay-slot rule) | 7 | ret_val folds to ot, no progress |
| `ret_val = ot; mask = ...; *ret_val = mask;` | 6 | identical residual to Lever B |
| `u32 *ret_val = ot;` decl-init form | 7 | folds |
| comma-return `return (*ot = ..., ot);` | 7 | no effect |
| swap arg order `func(s32 n, u32 *ot)` | 12 | ABI changes (cheat, also breaks callers) |
| param-local-alias-reverse `_n=n; _ot=ot;` | 15 | regression |
| `if (g_gpu_debug_level >= 2) ? : (void)0;` ternary | 7 | folds to if-form |
| `void *ot` cast-through-u32 | 7 | no effect on RA |
| `u8 *ot` + `*(u32*)ot` cast | 7 | no effect |

### Round 3-4 (m2c-shape adoption + named intermediates, ~10 forms)
All score 6 or 7. The named-mask Lever B (score 6) emits the addr in $v1
correctly but mask in $v0 (target wants $a0), and the return staging
`move v0, s0` lands LAST instead of EARLY. No clean lever flips this.

### Round 5-6 (type/cast/call-form variation, ~10 forms)
| Variant | Score | Notes |
|---|---|---|
| `mask = 0xFFFF; mask |= 0xFF << 16;` chain | 7 | folds |
| `mask = 0x1000000U - 1U;` form | 7 | folds |
| separate compound block for store | 7 | no effect |
| `mask_at_top` declared as init | 16 | spilled across calls |
| `ot[0] = ...;` indexed form vs `*ot =` | 7 | no effect |
| `(1u << 24) - 1` mask form | 7 | folds |
| `fn` local for dispatcher | 7 | folds |
| `addr_first; mask = addr & 0xFFFFFF;` | 7 | folds |

### NEW CHEAT-FORM IDENTIFIED THIS SESSION — DO NOT COMMIT

```c
u32 *func_8007B844(u32 *ot, s32 n) {
    u32 *result;
    if (g_gpu_debug_level >= 2) {
        g_gpu_debug_func(&D_80015F98, ot, n);
    }
    {
        u32 *(*disp)(u32 *, s32);   /* LYING: actual fn is void-returning */
        u32 *v0 = g_gpu_dev_table;
        disp = (u32 *(*)(u32 *, s32))v0[11];
        result = disp(ot, n);        /* captures undefined $v0 garbage as "return" */
        (void)result;                /* suppresses unused, but result IS used by GCC's view */
    }
    *ot = ((u32)&g_gpu_ot_end) & 0xFFFFFF;
    return ot;
}
```

**Sandbox `--disable all` = 0.** The function-pointer return-type lie
makes GCC believe the dispatch call returns a `u32 *` value in `$v0`,
which it captures into `result`. This pseudo `result` (which holds garbage
post-call) then sits in `$v0`'s live-range through the post-call body,
forcing GCC to schedule the mask into `$a0`, addr into `$v1`, AND in `$v1`,
and store via the `$v0`-resident pointer alias. Exact target match.

**Vetting per [[no-new-park-categories]] cheat-form checklist:**

1. **Catalog construct check:** the `(void)result;` superficially looks
   like the forbidden `(void)&local;` form but is DIFFERENT — it's
   `(void)<value>` not `(void)<address-of>`. Detector
   `find_addr_coerced_locals` doesn't fire (verified — sandbox completes).
2. **No semantic purpose check:** **FAILS.** The `(u32 *(*)(...))` cast
   declares the function pointer as returning `u32 *`, but the actual
   `g_gpu_dev_table[11]` slot is the canonical "ClearOTagR" handler that
   is `void`-returning (per [[gpu_ClearOTagR]] sibling at `display.c:177`,
   and all other dev_table[N] uses in display.c). The TYPE LIE has no
   semantic purpose — `result` holds undefined garbage.
3. **Natural-programmer check:** **FAILS.** No programmer writing this
   GPU clear-OT wrapper would declare the dispatcher as `u32 *`-returning;
   they'd declare it `void` (as gpu_ClearOTagR does). The form exists ONLY
   to manipulate `$v0`'s allocator/scheduler view.
4. **GCC-internals justification check:** **FAILS.** The form's "function"
   is to make GCC schedule `$v0`'s pseudo earlier so it's the natural
   pointer for the final store. That IS describing a GCC internal effect.

**VERDICT: CHEAT-FORM, not committable.** Same intent as the prior
session's conditional dead-store: lie to GCC about a value's existence to
manipulate `$v0`-liveness/allocation. Just spelled differently. Per
[[no-new-park-categories]] "cheats by any spelling are forbidden, full
stop": rejected by the worker (this session) without surfacing to the
user.

**Detector recommendation for future sessions:** the form is a
"function-pointer return-type promotion" pattern — declaring a void-
returning function pointer as returning a non-void type, then capturing
the result with `(void)<result>;` to suppress unused. A potential
detector would look for `register T *result; ... result = (T *(*)(...))FP(...);
(void)result;` in body scope where the FP's actual callees are
void-returning (cross-TU signature inference required, hard to
automate; flag for manual review).

### Re-park rationale

This session executed 26+ new structural levers (NOT in the prior
session's enumeration) at the score-6 base, including:
- All the goto-end / ret_val-decl / ret_val-staging permutations
- Named intermediate + addr/mask ordering swaps
- Cast and type variations (void*, u8*, u32 hex shifts)
- Dispatcher fn-pointer forms

None reach below 6 cleanly. The new cheat-form found (function-pointer
return-type lie) is documented as forbidden-by-policy.

**Next-session concrete hypothesis (not yet executed):** the
post-call `$v0` is undefined (function returns void). Target reuses
`$v0` for the early-staged return. The only way to force GCC to
naturally stage `$v0 = ot` before the addr chain in pure C would be
if some downstream operation BEYOND the AND/store consumed a value
that has $v0 as its preferred reg, biasing the local-alloc pass.
The function body has no such downstream consumer (the chain is
addr→and→store→return, with AND result going to $v0 naturally).
**The only un-tried structural lever class is**: introduce a *real*,
*semantically meaningful* operation after the AND that genuinely
references the return pointer (e.g. a real second dereference like
`*((u32 *)ot + 1) = something;` if that matches target's bytes —
but it doesn't, target has only ONE store). So this avenue is also
effectively closed unless the target's expected behavior allows
adding a second-store. Strong recommendation: the function is best
left parked pending user policy decision on category.

### Status (session 2)

**STILL PARKED.** Honest floor 6 confirmed via 26+ new lever tests.
NEW cheat-form catalog entry recorded (function-pointer return-type lie).
No clean lever found that closes the score-6 → 0 gap without falling
into the dead-store / mis-typed-return-value cheat family.

Resume avenues for a future session:
1. **User policy:** sanction this function for canonical-asm
   authorization (per [[no-new-park-categories]] section "If the
   evidence is absent or weak: keep parked"). scan_hand_coded LOW (0/8
   signals) still says it doesn't qualify, but two consecutive
   sessions of evidence-driven C-source exhaustion may warrant a
   policy-level conversation.
2. **Wait for a sibling unblock:** if `func_8007B564` /
   `func_8007B4D0` cluster develops a new structural lever that
   applies to this function's RA tie (e.g. a generalization of
   [[narrow-byte-args-packed-call]] for fn-ptr dispatchers),
   re-attempt.
3. **Do NOT** re-attempt directed permuter (~36k iters already
   exhausted prior session + this session's 26 levers cover most of
   the local mutation neighborhood).
