# func_8007B844 — WIP resume notes

## TL;DR

- **Floor 6** (HEAD floor 7). Lever: named-intermediate `u32 mask;` (Lever B
  from `register-alloc-pure-c.md`). Stable across 4 sessions, 55+ structural
  variants, ~50k permuter iters. Workflow round 1 (2026-06-02) added a
  decisive **sibling-evidence finding**: gpu_ClearOTag (matched, display.c:177)
  has the IDENTICAL final-statement C tail and emits return-staging-AFTER-store,
  which is exactly the score-6 shape — so the score-6 emission shape is a
  legitimate matching output for THE SAME C tail in a sibling. Discriminator
  is preceding-body shape, not the tail.

## Session 2026-06-02 (workflow round 1)

5-variant pure-C sweep at the score-6 candidate base + sibling cross-reference.
All 5 confirmed the floor: single-stmt fuse 7, hoist-mask-init 19, ret-variable 6
(copy-prop fold), comma-operator 6 (RTL-gen fold), val-split 7. The sibling
gpu_ClearOTag finding is the new actionable lead — see new top-of-list
next_hypothesis: sibling-mimicry (degenerate-loop wrapping) AND a target.s tail
diff between B844 and gpu_ClearOTag to first confirm whether the masked-12
hides a 0-distance match (run retire/verify-oracle to settle).

## Session 2026-06-02 (workflow round 2)

**Floor unchanged at 6.** Re-verified score-6 baseline; tested 4 NEW pure-C
variants targeting the orchestrator hint's mechanistic finding (target uses
RETURN-STAGED $v0 as store base via `sw $v1,0($v0)`):

- `ot[0] = mask;` — score 6, identical RTL lowering as `*ot = mask;`
- `out_ptr = ot; *out_ptr = mask; return out_ptr;` — score 6, copy-prop fold
- `u32 addr = (u32)&g_gpu_ot_end; *ot = addr & mask;` — score 7, breaks
  named-intermediate addr-register alignment
- `p = ot + 0; *p = mask; return p;` — score 6, combine folds pointer-arithmetic
  identity

**Predecessor-list evidence refined** (corroborates session-3 BB2_SCHED_DEBUG):

- STORE INSN chain depth = 4 (`lui → addiu → and → sw`)
- RETURN-STAGING insn chain depth = 1 (`move v0,s0 → jr ra`)
- Store base allocation: cc1 allocates `ot` ($a0 at entry) to callee-save $s0
  because it must survive the v0[11](ot,n) call; store base pseudo is $s0,
  not $v0

To match target, the store insn would need to depend on the return-staging
output (base = $v0). The only way in C is to make the store base pseudo
equal the return value pseudo as a NEW pseudo (not foldable to ot). Every
tested form folds via copy-prop.

**NO CHEAT-BY-SPELLING SURFACED.** The four FORBIDDEN constructs to avoid
(function-pointer return-type lie, dead-conditional-store, volatile-coercion,
register-asm pin) were not pursued. The volatile-on-local-pointer probe was
rejected at design time per the open-class cheats-by-any-spelling intent test.

**Sibling-evidence reconfirmed**: gpu_ClearOTag has IDENTICAL tail C emitting
the score-6 shape (sw-then-move). The discriminating factor IS preceding-body
structure (single vtable call vs gpu_ClearOTag's do-while loop) — NOT the tail.

New top next_hypothesis: synthetic degenerate loop wrapping the vtable-call
where the loop ITSELF re-binds ot (not just exists), matching gpu_ClearOTag's
loop-rebinding pattern. Risk-vetted carefully against no-new-park-categories
(an `ot = ot` rebinding is borderline). Alternative: BB2_SCHED_DEBUG dump on
gpu_ClearOTag's tail vs B844's tail to compare predecessor lists at .greg/.sched
dump level — if the lists differ, the structural cause is identifiable.

## Session 2026-06-03 (workflow round 3)

**Floor unchanged at 6.** Re-verified score-6 baseline (Lever B named-intermediate
u32 mask). Investigated per round-3 orchestrator hint whether the vtable
v0[11](ot,n) call's actual semantic could be leveraged to legitimately rebind
`ot` post-call.

**C semantic finding:** C semantics forbid a callee from rebinding a pass-by-value
pointer arg; the call can only modify what ot POINTS to, not ot itself. So GCC
always keeps `ot` in callee-save $s0 across the call. This closes the
post-call-rebinding avenue without UB.

**Three new structural variants tested at the score-6 base — all measured negative:**

1. `u32 *ret; ret = ot; *ret = mask; return ret;` declaration-first variant —
   copy-prop folds ret to ot, score 6 (same lowering as prior `u32 *ret = ot;` form)
2. `mask = (u32)ot; return (u32 *)mask;` SOTN variable-reuse — score 7 regression
   (no loop-invariant hoist mechanism exists in this function; technique doesn't apply)
3. `volatile u32 mask;` local volatile — rejected at design time per
   cheats-by-any-spelling intent test (no semantic purpose for a volatile local
   holding a constant 0xFFFFFF mask)

**Decisive byte-comparison gpu_ClearOTag vs func_8007B844** (both byte-matched
sibs): same C tail produces different asm — gpu_ClearOTag emits store-then-move
(5e0/5e4), func_8007B844 target emits move-then-store via return-staged base
(66c/670-67c). For cc1 to schedule move-then-store, store base pseudo must
depend on return-staged $v0 pseudo. No pure-C SOTN-allowed construct creates
this dependency without (a) extra bytes or (b) copy-prop fold to ot.

**Cumulative evidence across 5 distinct grinding contexts** (3 sessions + 2
workflow rounds + round-3): ~70+ structural variants and ~50k+ permuter iters,
ZERO progress beyond floor 6. **ESCALATION CANDIDATE** — empirically exhausted
under explored lever class; structural ceiling identified mechanistically.

Standing user_policy_note preserved: canonical-asm authorization NOT
evidence-backed for this function (S1=S2=S6=S8=0). B844 joins the "documented
structural ceiling at sched.c priority wall" cluster as cpu_side_move_dir_4,
marionation_Exec, saEft00Add. Round 4 recommended next steps: directed-PERM
permuter (`PERM_*` macros) from score-6 base targeting return-staging
REG_DEP_TRUE list — untried in any prior round.
- **Closing forms found are FORBIDDEN cheats** — preserved at `rejected/`:
  - `rejected/conditional_dead_store.c` — score 0 + SHA1 == oracle, but
    Lever D family (find_dead_conditional_stores detector catches it).
  - Function-pointer return-type lie (described in `meta.json`
    rejected_forms[1] but not preserved as a file — different cheat shape).
- **Hand-coded asm: ZERO signals.** S1 (handwritten annotations) = 0; S2
  (hardcoded register signatures) = none; S6 (custom ABI) = none; S8
  (packed-multiply pattern) = no. Every instruction has direct C analog;
  no GTE/BIOS/scratchpad. **Canonical-asm authorization is NOT
  evidence-backed** despite the worker's session-3 framing — the
  structural ceiling is a `sched.c` INSN_PRIORITY wall, same class as
  cpu_side_move_dir_4 / marionation_Exec.

## Session 2026-06-04 (round 5, SOTN-research-driven)

**Floor unchanged at 6. PARK_CANDIDATE escalation.** Executed the fresh-SOTN-
research lever set from `tmp/sotn_research_func_8007B844.md`. The research file
gathered direct evidence from the SOTN master tree and concluded: NO SOTN
precedent exists for the BB2 target shape (vtable dispatch + post-call
`*ot = mask & 0xFFFFFF; return ot;`). SOTN's analogs either return immediately
after the vtable call (DMA path — terminator written by callee) or wrap the
tail in a real `while (--n)` loop (non-DMA path — loop body provides chain
depth naturally).

**Tested research-proposed lever 1 (two variants):**
- `ot[n-1] = mask;` — sandbox 9, build 40 vs target 38 (+2 insns for
  `(n-1)*4` offset arithmetic; semantically writes wrong offset).
- `ot[n] = mask;` — sandbox 8, build 40 vs target 38 (+1 insn for `n*4`
  arithmetic; same semantic mismatch).

Target asm `sw $v1, 0x0($v0)` writes to offset 0; both lever-1 variants
write to a different slot AND require extra offset insns. They confirm the
research file's analytical hypothesis (different pseudo gets scheduled) but
cannot close the gap — the store base still depends on `$ot` (callee-save
`$s0`), not the return-staged `$v0`.

**Research-proposed lever 2 (degenerate do-while) REJECTED ON POLICY:**
The literal `do { } while (0);` form is sanctioned by `do-while-zero-exception`
ONLY for the LABEL_OUTSIDE_LOOP_P / reorg.c invert-jump peephole mechanism.
The documented mechanism here is `sched.c` INSN_PRIORITY (chain depth). The
rule explicitly states using do-while-zero to bend other GCC passes is forbidden.
The non-literal `while (--_n);` form either unrolls (no effect) or emits a
back-branch (regression). The introduced `_n = 1;` has no semantic purpose
(would fail the cheat-reviewer's family check).

**Cumulative across 8 sessions:** ~73+ structural variants, ~50k+ permuter
iters, ZERO progress beyond floor 6. Every documented SOTN-allowed catalog
technique exhausted; instrumented-cc1-derived levers exhausted; SOTN-master-
tree-precedented levers exhausted as of this round. Single un-attempted lever:
directed-PERM_GENERAL permuter — randomization permuter already at 50k iters
with zero gains, directed search would be over mutations within the exhausted
space.

**Escalation surfaced in `meta.json.next_hypotheses[0]`** — user policy decision
needed: (a) accept indefinite park under documented mechanistic ceilings
[default — joins the cpu_side_move_dir_4 / marionation_Exec / saEft00Add
cluster]; (b) NEW exception class sanction — agent does NOT recommend; SOTN
research found no master-tree evidence supporting this; (c) authorize one
final directed-PERM permuter run before park-acceptance.

## Session 2026-06-05 (round 7, v2-research-driven)

**Floor unchanged at 6. NO_PROGRESS.** Executed the v2 research memo
(`tmp/sotn_research_func_8007B844_v2.md`) which enumerated 5 RTL-derived
angles cross-referenced against rejected_forms. Only Angle 1 (`&ot` in
debug helper call) was net-new + self-vet-survivable at design time;
Angles 2/4 were combine-foldable chain-extender cheats (forbidden per
.claude/rules/register-alloc-pure-c.md session-5 update); Angle 5 was
fn-ptr return-type lie (already rejected_forms[1]); Angle 3 was
research-only (no new lever).

**Two NEW variants measured this round, both negative:**

1. Angle 1: `g_gpu_debug_func(&D_80015F98, &ot, n)` — **regressed 6 → 19**,
   build_insns 38 → 37 (−1). Mechanism: addressable `ot` gets a stack
   home, `move v0,s0` return-staging disappears (−1 insn), but the whole
   schedule cascades and masked diff explodes. Also fails cheats-by-any-
   spelling intent check (same family as `(void)&local;` from
   [[dead-vars-local-array]] scalar — passing `&ot` (u32**) to a debug
   formatter expecting `ot` (u32*) has no human-programmer purpose).
2. Post-increment + arithmetic rebinding: `u32 *ret = ot; *ret++ = mask;
   return ret - 1;` — score 6 (combine folds +1/−1 pointer arithmetic
   identity + copy-prop folds `ret` to `ot`).

**Permuter (~8 min wallclock, j=4, --stop-on-zero from score-6 base via
existing permuter/func_8007B844/ workspace, Lever B pre-loaded as
base.c):** ZERO sub-baseline candidates saved. Consistent with the
cumulative ~50.5k iters across sessions 5-10 finding zero improvements
on this function.

**Cumulative across 9 grinding contexts** (3 sessions + 2 workflow rounds
+ rounds 3-7): ~82+ structural variants, ~50.5k+ permuter iters, ZERO
progress beyond floor 6 in any context.

Next-session concrete actions (per [[difficult-is-not-impossible]] —
matching C exists, keep grinding; per [[no-new-park-categories]] —
no new park category for register-rotation walls):
1. BB2_SCHED_DEBUG instrumented dump on the Angle-1 (`&ot`) form —
   even though the form is a cheat, the SCHED dump may reveal whether
   the addressable-ot rearrangement crosses a chain a non-cheat
   construct could exploit (the `move v0,s0` insn disappearing is a
   structural shift not seen in prior session's RTL dumps).
2. Sibling cross-reference of gpu_SendPacket (display.c:210,
   byte-matched, has a post-call store) — diff its .sched dump against
   B844's to test for any transferable structural pattern.
3. PERM_GENERAL directed permuter with explicit `// PERM_GENERAL(stmts)`
   macro annotations around the post-call statements.

## Session 2026-06-05 (round 8, instrumented cc1 dump diagnostic)

**Floor unchanged at 6. NO_PROGRESS — diagnostic-only round.**

Three-pass campaign produced ALLOCDBG/SCHEDDBG/PRIODBG trace on the score-6
candidate via `tmp/gccdbg/cc1` (canonical SHA1 untouched; instrumented .s
byte-identical to canonical .s — sanity-check OK).

**Per-insn priority table** (block=2, post-CALL tail, 12 insns):

```
insn=46 CALL  pri=3 refcnt=7   = jalr $v0       (vtable call)
insn=71 INSN  pri=3 refcnt=1   = li $v0,0xff0000
insn=72 INSN  pri=3 refcnt=1   = ori $v0,$v0,0xffff
insn=54 INSN  pri=3 refcnt=1   = la $v1,g_gpu_ot_end
insn=55 INSN  pri=3 refcnt=2   = and $v0,$v1,$v0
insn=58 INSN  pri=3 refcnt=2   = sw $v0,0($s0)
insn=61 INSN  pri=3 refcnt=1   = move $v0,$s0   (return-staging)
insn=62 JUMP  pri=inf            = jr $ra
```

LUID order = C-statement order: 71(5), 72(6), 54(7), 55(8), 58(9), 61(10),
62(11). rank_for_schedule LUID-tiebreak among pri=3 ties picks higher LUID
first (reverse-emission) → 61 picked at clock=2, emits at position 11.
Target wants 61 at position 8.

**Insn 61 (return-staging) has ONLY ANTI predecessors** (55, 58, 46) and
only one TRUE successor (62 = jr ra reads $v0). The TRUE-producer of $s0
(`ot` pseudo) was defined in the prologue block; LOG_LINKS is intra-block,
so that producer is invisible to block=2 priority calc.

**Four shift mechanisms — all rejected at design-time pre-vet:**

1. Lower 61's pri below 3 → requires removing ANTI preds, all inherent to
   function semantics. Impossible without semantic change.
2. Raise 71/54's pri above 3 → requires extending CALL-rooted TRUE chain.
   Every form is either combine-foldable chain-extender (FORBIDDEN per
   `.claude/rules/register-alloc-pure-c.md` §FORBIDDEN as of 2026-06-02)
   OR emits bytes (regression).
3. Lower 61's LUID below 71/54 → LUID = C-statement order; `return ot;`
   MUST come last in C. Impossible.
4. Add a TRUE successor between 61 and 62 → requires an insn reading $v0
   that doesn't fold. Either emits bytes (regression) or is identity-fold
   cheat.

**Lever A/B/C surface assessment (per the campaign brief):**

- Lever A (block-local var split): `ot` is the only candidate; all splits
  (`u32 *p = ot;` permutations across rounds 1-7) copy-prop-fold. 10
  `rejected_forms` entries already document the exhaustion.
- Lever B (narrow type): applied at baseline as `u32 mask`. Cannot narrow
  further — `g_gpu_ot_end` is `u32`, so `u16 mask` truncates semantically.
- Lever C (loop-local precompute): function has NO LOOP. Does not apply.

**No candidate measured this round.** Per the brief: "If any test fails
[the 6-test checklist], drop the candidate. Do NOT code it just to confirm;
SKIP forms that would fail the reviewer."

**Cumulative** (10 grinding contexts): ~82+ variants, ~50.5k+ permuter
iters, ZERO progress beyond floor 6. The structural ceiling is now
demonstrated at the per-insn priority level.

Diagnosis: `tmp/gccdbg_func_8007B844/diagnosis.md` (gitignored).

Next-session un-attempted avenues (from round 7 + 8):
1. PERM_GENERAL directed permuter with explicit `// PERM_GENERAL(stmts)`
   macro annotations around post-call statements (narrower search-space
   than the exhausted ~50.5k-iter randomization permuter).
2. Sibling cross-reference: gpu_SendPacket (display.c:210, byte-matched,
   has a post-call store). Diff its block=2 SCHED dump against B844's.

## How to resume in one read

1. Read `meta.json` — note `instrumented_evidence` (the BB2_SCHED_DEBUG
   finding pins the exact mechanism), `rejected_forms` (do NOT re-derive
   these), `next_hypotheses`.
2. Read `rejected/conditional_dead_store.c` to understand the cheat
   shape (so you don't re-derive it under a different variable name like
   `_pre`, `stage`, `tmp`, etc.).
3. Apply `candidate.c`'s body to `src/display.c`. Confirm score 6 with
   `& tools/eng.ps1 sandbox func_8007B844 --disable all`.
4. Pick a `next_hypothesis`. Top candidate: fresh SOTN borderline-research
   pass focused on return-staging priority patterns (the 2026-06-02
   research found 6 new SOTN-allowed techniques; another pass may surface
   patterns specific to vtable-dispatch wrappers).

## The mechanism (instrumented, decisive)

Target asm (40 lines, src/display.c offset 199):
```
0x6C0A8: lui  $a0, 0xFF        ; mask = 0xFFFFFF
0x6C0AC: ori  $a0, $a0, 0xFFFF
0x6C0B0: addu $v0, $s0, $zero  ; <- return-staging (move v0,s0) BEFORE store
0x6C0B4: lui  $v1, %hi(D_8009BF30)
0x6C0B8: addiu $v1, $v1, %lo(D_8009BF30)
0x6C0BC: and  $v1, $v1, $a0    ; addr & mask
0x6C0C0: sw   $v1, 0($v0)      ; *ot = result, via the return-staged $v0
```

The candidate's score-6 form emits the same instructions but in a
different order: store first, return-staging after. Reason:

> cc1 sched.c gives return-staging INSN_PRIORITY = 1 (chain depth to
> `jr ra` is 1); the mask/addr chain has priority 4 (lui->addiu->and->sw,
> depth 4). `rank_for_schedule` picks higher-priority first; mask/addr
> chain emits before return-staging.

To match target, return-staging's chain depth must extend to >=4. The
only way in C is to make a later instruction consume `$v0` — but every
tested `u32 *p = ot; *p = ...; return p;` form folds to `ot` via
copy-prop. No SOTN-allowed construct prevents the fold.

## What's NOT canonical-asm

The worker's session-3 framing suggested "canonical-asm authorization OR
a fresh SOTN borderline-research pass" as next-step options. The
canonical-asm option fails the evidence threshold for THIS function:

| Signal | Threshold | B844 finding |
|---|---|---|
| S1 — `/* handwritten instruction */` annotations | grep > 0 | 0 |
| S2 — hardcoded source registers (SOTN signature) | yes | no — all ABI standard |
| S6 — custom-ABI / non-standard arg passing | yes | no — `$a0..$a3` |
| S8 — redundant low-bits mask before discarding shift | yes | no — mask is real (24-bit OT pointer) |
| no-C-form constructs | GTE/BIOS/scratchpad | none — all standard MIPS |

Per `.claude/rules/hand-coded-asm-recognition.md` strict gate, B844 fails
the canonical-asm test. Per `.claude/rules/difficult-is-not-impossible.md`,
the structural ceiling is "unfinished work," not "proven impossible."

The honest next-step menu is therefore:
1. Fresh SOTN borderline-research pass (genuine option)
2. Sibling cross-reference (look for a matched gpu_LinkList-style wrapper)
3. Instrumented BB2_SCHED_DEBUG re-probe on the candidate
4. ~~Canonical-asm authorization~~ — not evidence-backed

## Related

- `memory/project/func-8007b844-conditional-dead-store.md` — full
  session-2 ledger documenting the conditional dead-store discovery and
  its rejection
- `.claude/rules/register-alloc-pure-c.md` — Lever B (named intermediate)
  used to reach the score-6 base; Lever D (dead self-assign) is the
  forbidden closing form's family
- `.claude/rules/no-new-park-categories.md` — the cheats-by-any-spelling
  policy that catches the rejected conditional dead-store
- `.claude/rules/difficult-is-not-impossible.md` — the cardinal rule
  that says "stuck on a sched.c priority is unfinished work, never
  proven-impossible"
- `.claude/rules/hand-coded-asm-recognition.md` — the strict gate that
  this function fails (no S1/S2/S6/S8 signals; no no-C-form constructs)
