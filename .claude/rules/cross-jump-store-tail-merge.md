---
name: cross-jump-store-tail-merge
paths: ["src/*.c"]
description: "target has more `sw GLOBAL` stores (or more `j SAME_LABEL` error tails) than your build: GCC 2.7.2 jump2 cross_jump merged N identical `[sw GLOBAL; j END]` tails into one block. FIX: give the error paths a MIX of exit forms (distinct `goto endK; ... endK: return G;` labels + one inline `return G;`) so the block ENDINGS differ -> suffixes not rtx_equal -> no merge."
metadata:
  type: reference
---

# The cross-jump STORE-tail merge wall — SOLVED (mix the exit forms)

The store-tail analogue of [[cross-jump-call-merge]] (which uses arg COUNT to keep
CALL suffixes distinct). Here the merged suffix is a global STORE + jump, and the
lever is the **block ending** (jump vs return vs distinct-label jump), not arg count.

## Symptom

A function with several early-exit paths that each write the SAME constant to the
SAME global and then return it through a shared end, e.g.

```c
if (...) { ...; D_GLOBAL = -1; goto end; }   // tail 1
if (...) { ...; D_GLOBAL = -1; goto end; }   // tail 2
...                          D_GLOBAL = -1; goto end;   // tail 3
end: return D_GLOBAL;        // re-reads the global (value unknown -> reload)
```

Your pure-C body is structurally correct but plateaus. **The tell: `target` has MORE
`sw $v0,D_GLOBAL` stores than your build** (target keeps N inline stores; your build
collapsed them to 1 shared store block). Equivalent tells: fewer `j` to the shared
end label; a `.LNNN:`-labelled shared store block in your maspsx that the target
does not have.

## Diagnostic (decisive)

Count the stores to the global in target vs your cheat-free build object:

```python
# target: build/src/<file>.o ; yours: tmp/sandbox/<func>/<file>.o
re.findall(r'sw\t\$\d+,D_GLOBAL', objdump_dis)   # target N, yours < N  => merged
```

Or watch it collapse across RTL passes (`cc1 ... -da`): the N stores survive
`rtl/jump/cse/loop/combine/sched/flow`, then **`jump2` drops them to 1**. That is
`jump_optimize(..., cross_jump=1)` -> `find_cross_jump` (jump.c ~2371) matching the
2-insn suffix `[sw GLOBAL ; j END]` (minimum=2, both insns rtx_equal -> merge).

## Why it merges (the mechanism)

`find_cross_jump` walks backward from each `j END` and matches identical insns; the
shared `[sw $v0,D_GLOBAL ; j END]` is exactly `minimum=2` -> merges into one block,
the others redirected into it. All N tails being byte-identical (`li v0,-1; lui at;
sw v0,%lo(G); j END; nop`) is precisely what triggers it. Even feeding GCC the
TARGET's own m2c-reconstructed structure merges under our fork — so the C must make
the tails *not* mergeable.

You CANNOT defeat it by:
- inserting a dead pure-C statement after the store (DCE'd before jump2 -> no effect);
- a `*(volatile*)` store (stays separate but emits wrong `la`-form addressing -> a
  different diff; this is the tier-3 cheat the de-cheat is removing);
- distinct goto labels that all forward to `return G` (GCC coalesces e1/e2 back into
  one block -> re-merges the pair).

## The lever: MIX the exit forms so block ENDINGS differ

cross_jump matches blocks with rtx_equal suffixes that jump to the SAME label.
If the tails END differently, the suffix match breaks (only the bare jump, or
nothing, matches -> < minimum -> no merge). The combination that keeps ALL N
stores separate AND preserves the shared re-reading end:

```c
if (...) { ...; D_GLOBAL = -1; goto end1; }   // tail 1: distinct-label goto
if (...) { ...; D_GLOBAL = -1; return D_GLOBAL; } // tail 2: INLINE return
...          ...; D_GLOBAL = -1; goto end3;   // tail 3: distinct-label goto
...success...;                    goto end4;
end1: return D_GLOBAL;
end3: return D_GLOBAL;
end4: return D_GLOBAL;
```

The inline `return G` block ends with the epilogue thread; the `goto endK` blocks end
with a jump to a distinct label — the endings are not rtx_equal, so none merge. Each
emits its own `li v0,-1; lui at; sw v0,%lo(G)` (all in `$v0`, matching target) and the
shared `endK: return D_GLOBAL` still RE-READS the global (`lw v0,%lo(G)`), because the
value at the merge point is genuinely unknown (errors store -1, success stores another
value). Pick which path is the inline `return` vs which are distinct-label `goto` by
matching the TARGET's per-tail register usage (all `$v0` is the goal; a tail that comes
out in `$v1` means that path should switch goto<->return).

## Confirmed case — saEft00Add (src/system.c, 2026-05-28)

Three error paths each `D_800A14E4 = -1` then return the re-read global; success path
`D_800A14E4 = D_800A14D0`. Original committed source used `goto end` for all three +
a `*(volatile*)` store on tail 1 + an `__asm__("")` barrier on tail 2 (the barrier is
an opaque insn that survives to jump2 and breaks the suffix) + regfix to fix the
volatile addressing — all tier-3 debt. Pure-C distance with those stripped: 15, of
which the whole gap was the 3-way store-tail merge.

The mix (tail1 `goto end1`, tail2 inline `return`, tail3 `goto end3`, success
`goto end4`, all labels `return D_800A14E4`) produced 4 separate `$v0` stores +
shared `lw` re-read = the target's exact branch-store region. Honest distance 15 -> 6.
(The residual 6 was a separate reorg.c delay-slot eager-fill, not this merge.)

## The residual-6 is a COUPLED FIXPOINT, not two independent fills (saEft00Add, 2026-05-29)

The residual 6 (after the store-merge is defeated) is NOT closable by pure C with the
current toolchain. It is a coupled `reorg.c` ⨯ `jump2 cross_jump` fixpoint. Pinned in
full below so a future resume doesn't re-derive it.

### What the target has that we can't reproduce together (3 facts, all simultaneously)
1. **Three eager delay-slot fills** reorg performs and our build declines:
   - site1: `addiu $a0,1` in the `bnez` delay slot before `func_80080390(1,0)` (mode path)
   - site2: `addiu $a0,6` in the `beqz` delay slot before `func_80080390(6,0)` (success path)
   - tail2: `addiu $v0,-1` in the arg0-error `bnez` delay slot
2. **A single shared end `.L80082610`** (re-reads `$v0,D_800A14E4`) reached by all 3
   error `j .L80082610` + the success fall-through. Build object addr 0x1f4.
3. **Three byte-identical inline error stores** `[li $v0,-1; lui $at; sw $v0,%lo(D_800A14E4); j .L1f4]`
   that did NOT cross-jump-merge (verified: target has 4 distinct `R_MIPS_LO16 D_800A14E4`
   store relocs at 0x2434/0x248c/0x24e4/0x2570).

### Why we can't reproduce all three (the coupling)
- **`reorg` declines each eager fill** because `mark_target_live_regs` reports `$a0` (or
  `$v0`) LIVE at the branch's opposite-thread head. Mechanism (instrumented cc1, `DBG_FILL`/
  `DBG_FWD`/`DBG_JF`): the fall-through head DEFINES the reg first (`head_sets_a0=16`, so it
  is genuinely set-before-use), but the forward set-before-used walk in
  `mark_target_live_regs` RE-LIVES it via the downstream `func_80080390`+`sys_VSync`
  arg-uses, and for site2 never reaches a followable jump (`jump_insn=-1`) so the
  jump-following intersection that would clear it never runs. Result `a0_live=16` ⇒ fill rejected.
- **Site1 fills only with the mode-path inline `return`** (jump-following reaches a sibling
  error tail's `j .L80082610` laid after it ⇒ `a0_live=0`) — but that inline return makes the
  mode error store land in `$v1` + emits an extra `li $v0,-1` (collateral). masked distance 6→8.
- **Site2 fills only with a deferred `D_800A14E4=-1; return` store-block laid PHYSICALLY
  AFTER the success path** (gives the forward-walk a return-bearing block to hit ⇒ `a0_live=0`).
  But deferring a store REMOVES that path's inline `[v0=-1; sw E4; j; nop]` (which the target
  keeps inline) ⇒ −6 insns. The combined both-fill base (mode inline-return + any one error
  store deferred after success) reaches `a0_live=0` at BOTH sites (verified `DBG_FILL`:
  site1 + site2 both `a0_live=0`) but measures **sandbox distance 19–21** (build 127 vs target
  133): the fills compound the cost, they do NOT cancel.
- **The single shared end with 3 unmerged identical stores is unreachable**: GCC 2.7.2 `jump2`
  `find_cross_jump` (jump.c ~2371) merges the 3 identical `[sw D_800A14E4; j .L1f4]` suffixes
  (4-insn common suffix ≫ `minimum`=2) into one shared store block. The m2c single-return form
  (cand_E) yields 3 LO16 relocs not 4 (one error store merged) under the **canonical cc1** AND
  under **PsyQ cc1psx** — and no `-O`/`-f` flag tested (`-fno-thread-jumps`,
  `-fno-cse-follow-jumps`, `-fno-rerun-cse-after-loop`, `-fno-delayed-branch`, `-fno-reorder-blocks`,
  `-O1`) changes it. The distance-6 mix avoids the merge only by SPLITTING the end into
  `.L1ec` (re-read) + `.L1f4` (epilogue), which itself diverges 1-2 insns from the target's
  single-end and (critically) makes the success path "last" ⇒ site2's forward-walk re-lives `$a0`.

### Evidence ledger (so a resume need not redo it)
- Hand structures: ~25 exit-form / CFG variants; floor = the distance-6 mix (masked 6).
- Permuter: 4 seedings — distance-6 base, site1-fill base (cand_F), both-fill bases
  (cand_I/cand_K), + a `PERM_GENERAL`-directed exit-form base — ~120k+ iters total; best
  saved scores 385 / 405 / 985 / 995 / 465 respectively; NONE approached the fills (385 was
  injected-volatile/reorder noise, not structural). Single-base AND multi-base
  (cross-pollinate) both plateaued.
- Target-replication: target = 4 distinct E4 stores + single end + 3 eager fills; our build
  from every single-end C = ≤3 distinct E4 stores (merge) and 0–1 eager fills.
- cc1psx parity: cc1psx produces the IDENTICAL decline (site1-eager-only on the mix, store-merge
  on the single-end) ⇒ NOT a decompals-vs-PsyQ fork divergence.
- Combined-base result (the last untested avenue, 2026-05-29): both fills achievable together
  (a0_live=0 at both sites) but distance 19–21 — the deferred-store layout cost (−6 insns)
  exceeds the +2 fills. Coupling compounds, does not cancel.

### Status
NOT pure-C-matchable with the current engine/toolchain understanding. Best clean state is the
distance-6 mix (committed via the saEft00Add cheat carrying the two `subst nop addiu`/`.L+4`
regfix + the asmfix copy/draw-skip — these RECREATE the eager fills + the single-end the
compiler won't produce). A genuine close needs a `reorg`-level lever (make `$a0` dead at the
fall-through WITHOUT the deferred-store layout) that none of the explored C structures provide,
or a `cross_jump`-level lever to keep 3 identical store-tails unmerged at a single end — neither
found. Resume here; do not re-run permuters (exhausted) or re-call it a fork wall.

### Final-round la-macro instrumentation: cause pinpointed (2026-05-29)

Hypothesis tested: "the `la` macro `lui $a0,%hi; addiu $a0,$a0,%lo` emits an `addiu` that
READS $a0, polluting the forward set-before-used walk's `needed` before the `lui`'s SET is
recognised — explains `a0_live=16` despite `head_sets_a0=16`." **Instrumented disproof: the
cause is the CALL_INSN's arg USE, not the la macro.**

At the RTL level reorg sees, the la is ONE insn `(set (reg a0) (symbol_ref "D_800162EC"))` —
DBG_LA confirms `ref_a0=0 set_a0=16` (pure SET, no a0-read). The trace at site1 (target uid=59):
```
uid=59 (set a0,symbol)  THIS_insn(ref_a0=0  set_a0=16) → res_a0: 16→0  ✓ cleared
uid=61 (CALL tslTm2LoadImage_2) THIS_insn(ref_a0=16 set_a0=16) → ACCUM needed_a0 := 16
uid=66 (set a0, const 1)   scratch = set_a0 & ~needed_a0 = 16 & ~16 = 0 → res unchanged
```
The poisoning insn is **uid 61, the CALL** — its `CALL_INSN_FUNCTION_USAGE` `(use (reg a0))`
(the standard arg-passing list) adds a0 to accumulated `needed`. From that point, every later
`set a0, const` candidate has `scratch = set & ~needed = empty` and cannot be recognised as
set-before-used. Site2 (target uid=231) shows the identical pattern — uid 233 CALL
`tslTmlGetHeda` is the poisoner.

### Why no pure-C escape exists for the actual cause
The `(use (reg a0))` is **intrinsic to any function call** (the ABI requires a0 to hold the
first arg AT the call). No C-level reformulation removes it:
- **Single-insn-load lever** (the brief's option 1 — put `&D_800162EC` in a sdata global loaded
  via `lw $a0,%gp_rel(...)($gp)`): would replace `(set a0,symbol)` with `(set a0,(mem ...))` —
  still ONE RTL insn, still SET-only. The CALL right after still has `(use (reg a0))`.
  Adding such a global would also alter the .data layout (SHA1-breaking) and `D_800162EC`
  is not in `sdata_syms.txt`.
- **Invert the bnez sense / pick which path is fall-through** (options 2 + 3): tested in
  `tmp/inv1.c`, `tmp/inv2.c`, `tmp/inv3.c`, `tmp/flip1.c` against the distance-6 base and the
  cand_F inline-return mode. GCC's RTL is invariant under the C-level inversions
  (jump-threading collapses them to the SAME bnez). Result: site1 fills iff mode-path
  inline-return is used (with the same v0/v1 collateral as before); the inversion is
  byte-for-byte equivalent to its un-inverted form. No new lever.
- **Deferred store-block (the only `a0_live=0` structural lever found)**: costs the −6 inline-
  store insns; combined-base distance 19–21 (above).

The fall-through head is ALWAYS an arg-setup-then-call sequence for the call the eager fill is
meant to feed. The CALL's `(use a0)` is what poisons `needed`; nothing C-level removes it. The
coupled fixpoint stands.

### Session-12 (2026-05-30) — BB2_REORG_DEBUG instrumented reorg.c walk; finer-grained mechanism + partial site1 lever found

Built the BB2_REORG_DEBUG hooks (DBG_OPP, DBG_FILL, DBG_LA, DBG_BRK, DBG_WLK, DBG_MTLR,
DBG_BBLAS) into `tmp/gccdbg/reorg.c` covering the `mark_target_live_regs` walk's full state
at the per-insn level + the `fill_slots_from_thread`'s opposite_thread head live check. Per
session-11's tooling guarantee, canonical `tools/gcc-2.7.2/build/cc1` SHA1
`045c9543d39ab8109583b92137c7adde084f7a25` (May 18 00:23) — **UNTOUCHED**; instrumented
`tmp/gccdbg/cc1` rebuilt to expose tgt=59 (site1), tgt=231 (legacy site2 uid), tgt=237 (site2
in cand_inline_ret) traces.

**Refined the mechanism three layers deeper than session-11's `(use a0)` finding:**

1. **JF-LOOP recursive seed**: For site1's `j .L14` (`goto end1`), the walk reaches the
   simple-jump and FOLLOWS via `mark_target_live_regs(next_active_insn(jump_target))`. That
   recursive call returns `new_resources` SEEDED with the jump-target block's
   `basic_block_live_at_start` — which, by flow.c's worklist back-prop from successors that
   eventually reach calls using a0, has a0=16. The JF-LOOP `IOR`s scratch into new_resources,
   then `AND`s into res. Net: res.a0 = 16 EVEN THOUGH the direct forward walk would have set
   it to 0 (via the LA's set-before-used). The recursive call is the actual poisoner — not the
   in-block CALL_INSN that session-11 identified.

2. **USE-INSN-wrapper pollution**: Between the in-block insns visited by the walk, GCC inserts
   `(insn (use <inner-insn>))` USE markers (residue of earlier reorg/sched moves via
   `update_block`). When the walk hits such a USE with an `'i'`-class inner expression, line
   2747 of reorg.c calls `mark_set_resources(inner, res, 0, 1)` — DIRECTLY ADDING set bits to
   `res`. The forward walk is therefore NOT monotonically decreasing (as the algorithm intends);
   USE-INSN wrappers RE-LIVEN previously-cleared regs. Site1's trace shows a0 going from 0
   (after uid 334) to 16 (entering uid 336) via an interposed USE insn at uid 335 (only visible
   in DBG_WLK; filtered from DBG_LA because USE has its own `continue` path before the LA
   print). This is the REAL reason a conditional `break` inserted between the merge and the
   downstream calls doesn't help — pollution happens BEFORE the break.

3. **Conditional-break breaks the walk for site2 but not for site1**: A conditional
   `if (arg0 < 0) D_800A14E4 = -1;` placed between the success-path post-call code and
   `sys_VSync(-1)` (cand_site2_brk) emits a `bgez s1, .L; li v0,-1; lui at; sw v0,(at)` (4
   insns). The bgez is a non-simple JUMP_INSN → walk BREAKS at it. For site2 (where the
   opposite_thread head is the la for `&D_80082320`), the walk breaks BEFORE hitting any USE
   wrapper that adds a0; result: site2 EAGER-FILLS (`li a0,6` in beqz delay slot). For site1
   (where the opposite_thread head is the la for `&D_800162EC`), the walk visits MORE
   intermediate insns including a USE wrapper that re-livens a0 BEFORE the bgez break. The
   conditional doesn't help site1.

### The partial lever: inline-return at end1 unlocks site1

`cand_inline_ret.c`: replace `goto end1; ... end1: return D_800A14E4;` (the mode-error tail)
with a direct `return D_800A14E4;`. This REMOVES the `j .L<end1>` simple jump from site1's
opposite_thread walk. The walk now reaches the function's `jr ra` (RETURN) instead — which is
"will follow" but `JUMP_LABEL(jr_ra) = NULL` so `next_active_insn(NULL) = NULL` →
`mark_target_live_regs(NULL) = end_of_function_needs` (a0 NOT in it for MIPS). JF-LOOP's
new_resources stays clean for a0. **Site1 eager-fills**. Measured: 127 insns, site1 EAGER
(`li a0,1` in bnez delay slot), site2 still nop.

Trade-off (the reason it's PARTIAL): inline-return at end1 means the mode error tail does NOT
share the `[lw v0, D_800A14E4; epilogue]` re-read with target's `.L80082610` shared end. The
target uses `goto end + shared end re-read` which:
- Allows v0 reuse (target stages `addiu v0,$0,-1` right after sw $v0 (D_800A14EC), reusing v0
  for the next sw — matching its EXACT bytes);
- Inline-return forces GCC to pick $v1 for one of the two adjacent -1 stores (because v0 is
  busy with the sys_VSync return), then `li v0,-1` in the j's delay slot for the return value.
- Net: distance-cleanup of 2 insns SAVED on the tail BUT +1 reg-mismatched store (v1 not v0)
  cascading into branch-offset shifts.

### Combined lever (cand_site2_brk): both eager fills, 131 insns, 75 normalized diffs

`cand_site2_brk.c` = inline-return at end1 + dummy conditional in success path. Result: 131
insns, BOTH site1 AND site2 eager-filled. Vs target's 133: short by 2 insns (the missing
shared-end re-read). Normalized diff (via `tmp/saeft_diff_target.py`): 75/133 (most are
1-insn cascade offsets from the inline-return form + the v0/v1 register diff).

The dummy `if (arg0 < 0) D_800A14E4 = -1;` is a SEMANTIC CHANGE (when arg0 < 0, returns -1
instead of D_800A14D0). NOT a candidate for actual commit — only a proof that the conditional
break MECHANISM works for site2.

### Why neither path closes the residual

| Form | Insns | Site1 eager | Site2 eager | v0/v1 match | Diffs |
|---|---|---|---|---|---|
| distance-6 mix (goto+inline+goto+goto) | 131 | NO | NO | YES | 6 |
| target asm | 133 | YES | YES | YES | 0 |
| cand_inline_ret (inline ret at end1) | 127 | YES | NO | NO (v1 collision) | many |
| cand_site2_brk (+ semantic if) | 131 | YES | YES | NO (v1 collision) | 75 (cascade) |
| cand_jalr (function-pointer indirect call) | 136 | NO | NO | n/a | many |
| cand_call_postdom (do-while-0 boundary) | 131 | NO | NO | YES | 6 (same as baseline) |
| cand_mix_both_brk (goto+brk for both) | 135 | NO (USE pollution) | YES | n/a | many |

The **3-way trilemma**:
1. Want unmerged tails for site1's `[sw D_800A14E4; j shared_end]` (so target's 4-store
   pattern emerges) → need goto + shared end + a way to defeat cross-jump (= the MIX).
2. Want site1 eager-fill → need NO `j .L<x>` in opposite_thread that triggers JF-LOOP with
   a polluting block-live-in (= inline-return for end1, or some structural lever that makes
   the jump-target block's `current_live_regs.a0 = 0`).
3. Want v0 reuse for the -1 stores at the mode-error tail → need goto+shared-end form so
   GCC reuses v0 across the `sw v0,D_800A14EC; addiu v0,-1; sw v0,D_800A14E4` sequence
   (inline-return form forces v1 because the inline jr ra's return-value carry conflicts).

Goals 1 + 3 align (goto+shared-end+mix); goal 2 contradicts them in our pipeline.

### What no C construct breaks

To reach the matching state, EITHER:
- (a) The JF-LOOP's recursive `mark_target_live_regs(next_active_insn(.L<end>))` must return
  with a0=0. That requires `basic_block_live_at_start[bb_of(.L<end>+next_active)].a0 = 0`.
  Flow.c computes this from successors' live-out minus block's defs. If ANY successor has a0
  live and the block doesn't kill a0, a0 propagates. The shared end `.L80082610` block is
  `lw v0, D_800A14E4` (kills v0, doesn't touch a0) → successor `.L<epilogue>` (`lw ra; lw s1;
  lw s0; addiu sp; jr ra`) doesn't use a0 → a0 should NOT propagate. But flow.c is conservative
  about register-clobber sets at function-exit boundaries; its actual computed value depends on
  GCC's flow.c implementation details that no C-level change can override.
- (b) The USE-INSN wrappers from reorg/sched's earlier passes (`update_block`) must not
  insert a0-touching USE markers in opposite_thread's walk path. These are inserted whenever a
  sched move crosses a block boundary or affects a register's last-use. C-level structure
  affects sched's choices indirectly (via instruction count / dependency depth) but no C
  expression directly suppresses USE-INSN insertion.

Both (a) and (b) are **compiler-internal data-flow decisions made AFTER C source consumption**.
The C-source space's degrees of freedom (which the prior sessions + this session explored
across ~120k permuter iterations + 6 hand-crafted structural variants) does not span the
direction needed to flip them.

### Genuine resume avenues from this session

1. **GCC-level: patch reorg.c to recompute `new_resources` from a CLEAN walk** (not seeded by
   `basic_block_live_at_start`). User policy decision: compiler patches at this depth alter
   target match for OTHER functions and require a project-wide regression sweep.
2. **Move sys_VSync(-1) earlier**: place `D_800A14E8 = sys_VSync(-1);` BEFORE site2's
   `if(D_800A1500 & 1)` block. Removes the post-site2 a0 use; site2's walk no longer
   accumulates needed_a0 from a downstream call. Untested in this session because semantically
   it reorders a syscall (potential VSync-timing implication for the game's runtime behavior).
   Worth empirical test in a future session.
3. **Combined approach**: cand_inline_ret structure + sys_VSync-moved-earlier in success path.
   Likely outcome: both sites eager-fill via different mechanisms (site1 = inline-return
   avoidance of JF-LOOP; site2 = no downstream a0 use → walk's needed_a0 stays clean). Net
   insns: ~127 (cand_inline_ret) + whatever the move costs. Worth a controlled test.

### Status (session 12)
PARTIAL pure-C lever proven (`cand_inline_ret` unlocks site1). Site2 closeable via either
SEMANTIC-changing conditional (cand_site2_brk, not viable) or call-reordering (untested).
Full match still blocked by the 3-way coupling (unmerged tails ⨯ JF-LOOP-safe ⨯ v0-reuse).

Canonical cc1 untouched (SHA1 `045c9543d3...` May 18 00:23). Instrumented `tmp/gccdbg/cc1`
preserved with BB2_REORG_DEBUG hooks (DBG_OPP/FILL/LA/BRK/WLK/MTLR/BBLAS) for resume. All
candidate `.c` files preserved in `tmp/cand_*.c`. Test harness: `tmp/saeft_test.sh` + diff
tool `tmp/saeft_diff_target.py`.

### Session-13 (2026-05-30) — NEW FORM: V2b + inline_ret_mode. Honest distance 15 → 8.

Per [[difficult-is-not-impossible]] § "Do NOT stop with documented unrun resume avenues",
session 13 executed session-12's named untried avenue (move sys_VSync(-1) earlier in
success path) AND a sweep of inline-return variants. Result: the documented sys_VSync
move REGRESSES (session-12's hypothesis was wrong — the post-site2 a0 use is from
func_80080390, not sys_VSync). But a NEW lever was found.

**The lever: dual inline-return + drop tier-3 `__asm__("")` barrier.** Two forms tested
this session, combined score-8 vs the committed honest-distance-15 baseline:

```c
/* V2b: inline-return at tail2 (arg0 != 0 path) drops the __asm__("") barrier */
if (arg0 != 0) {
    ...
    if (func_80080258(2, temp_s0, 0) != 0) goto common_path;
    D_800A14E4 = -1;
    return D_800A14E4;   /* was: D_800A14E4 = -1; __asm__(""); goto end; */
}
/* V2b + inline_ret_mode: also inline-return at mode-path end1 */
if (cdrom_GetMode() & 0x10) {
    ...
    func_80080390(1, 0);
    D_800A14EC = sys_VSync(-1);
    D_800A14E4 = -1;
    return D_800A14E4;   /* was: *(volatile s32*)&D_800A14E4 = -1; goto end; */
}
```

This form is NOT in session-12's enumeration (cand_inline_ret = inline-ret end1 ONLY,
cand_site2_brk = goto-all-tails + semantic break). V2b + inline_ret_mode = inline-ret at
BOTH mode-path AND tail2, goto at tail3, fall-through at success. Result:
- build_insns 131 (vs target 133, 2 short)
- masked Levenshtein **distance 8** (vs distance-6-mix's ~15 stripped)
- site1 EAGER-FILLS ✓ (mode-path inline-ret removes the JF-LOOP poisoner)
- site2 still NOT filling (no JF-LOOP path on success — same as session-12)
- v0/v1 collision in mode-path tail (mine: `li v1,-1; sw v1; j; li v0` ≠ target's
  `li v0; sw v0; j; nop`)
- Drops 1 of 3 tier-3 cheats (`__asm__("")` gone; volatiles on D_800A1500 and D_800A14E4 remain)

### Variants tested this session (all measured negative for further improvement)

| Variant from V2b + inline_ret_mode base | Score | Notes |
|---|---|---|
| baseline (HEAD, all cheats present) | 15 | reference |
| V2b alone (inline-ret tail2 only) | 10 | drops one cheat |
| V2b + inline_ret_mode | **8** | session-13's new best |
| V2b + inline_ret_tail3 (all 3 error tails inline) | 15 | tail3 inline regresses |
| V2b + all 3 inline-rets | 13 | tail3 inline regresses |
| V2b + vsync moved before D_800A1500 check | 19 | session-12's hypothesis: regresses |
| V2b + vsync moved after cdrom_SetCallbackB | 23 | regresses harder |
| V2b + vsync moved before func_80080390(6,0) | 18 | regresses |
| V2b + inline_ret_mode + 14E4=-1 first | 10 | swap stores: -2 worse |
| V2b + inline_ret_mode + 14E4=-1 BEFORE 390 | 12 | even earlier: -4 worse |
| V2b + inline_ret_mode + `return (D_800A14E4 = -1)` | 8 | chained assign: no change |
| V2b + inline_ret_mode + `return -1;` explicit | 8 | identical to D_800A14E4 |

### Site2-break variants (all FAIL — empty conditionals DCE'd, real ones cost more than save)

Session-12's cand_site2_brk used `if (arg0 < 0) D_800A14E4 = -1;` (semantic) to break
the walk. Session-13 tested 12 non-semantic break forms; all measured score 8-13:

| Break form (between site2 and func_80080390(6,0)) | Score from V2b+ret_mode base |
|---|---|
| `if (arg0 < 0) {}` | 8 (empty — DCE'd) |
| `if (arg0 < 0) ;` | 8 (DCE'd) |
| `if (arg0 < 0) arg0 = arg0;` | 8 (self-assign DCE'd) |
| `if (arg0 < 0) (void)0;` | 8 (DCE'd) |
| `if (arg0 < 0) goto L; L: ;` | 8 (DCE'd) |
| `if (arg0 < 0) cdrom_SetCallbackB(0);` | 12 (call adds bytes) |
| `if (arg0 > 0) cdrom_SetCallbackB(0);` | 12 |
| `if (arg0 == -1) cdrom_SetCallbackB(0);` | 13 |
| `if (arg0 < 0) D_800A14E4 = -1;` (cand_site2_brk) | 12 (session-12's lever) |
| `if (*(volatile s32 *)&D_800A1500 < 0) {}` | 10 (volatile load adds bytes) |
| `goto L; L: ;` (simple unconditional) | 8 (no break — simple jump) |
| `if (D_800A14D0 < 0) D_800A14D8 = D_800A14D8;` | 8 (self-assign DCE'd) |

**GCC 2.7.2 DCEs every empty-body `if`.** Bodies that survive cost more than the
eager-fill gains. The fundamental cost ceiling stands: any non-semantic break that
GCC can't optimize away costs ≥ 4 insns; eager fills gain ≤ 2 insns.

### Why session-12's vsync-move hypothesis was wrong

Session-12 wrote:
> Move sys_VSync(-1) earlier: place D_800A14E8 = sys_VSync(-1); BEFORE site2's
> if(D_800A1500 & 1) block. Removes the post-site2 a0 use; site2's walk no longer
> accumulates needed_a0 from a downstream call.

Empirically: the post-site2 a0 USE is `func_80080390(6, 0)` (which needs $a0=6 for
its first arg), NOT `sys_VSync(-1)` (which needs $a0=-1 for its arg). Moving
sys_VSync earlier does NOT remove the func_80080390 a0 use. Test results: all
vsync-move forms regress (score 18-29).

### Permuter from V2b base (`permuter/saeft_v2b/`)

Set up + ran ~1264 iters. Best legitimate score 555 (permuter weighting). Candidate
`output-555-1` is essentially V2b + inline_ret_mode + `D_800A14E4=-1` before
`D_800A14EC = sys_VSync(-1)` in mode path (a swap of two stores). When tested via
sandbox (against the full system.o context), that form scores **10**, not better
than V2b + inline_ret_mode's 8. Permuter's score function and sandbox masked
Levenshtein measure different things; permuter's "improvement" of 555 vs 615
baseline doesn't translate to honest sandbox progress.

### What the session-13 V2b finding implies for commit strategy

The V2b + inline_ret_mode form is a real ~50% honest-distance reduction (15 → 8) AND
drops 1 of 3 tier-3 cheats. But:
- The 13 existing regfix/asmfix rules are calibrated for the committed form's
  maspsx indices; V2b shifts them.
- Committing V2b requires REWRITING the rules for the new structure.
- Result would NOT be Tier-4 (still has 2 tier-3 volatiles + rules); just fewer cheats.

The user policy is "Tier 4 SOTN standard moving forward". A partial cheat reduction
that doesn't reach Tier-4 doesn't satisfy that policy, so V2b is best documented as
WIP-progress, not committed.

### Status (session 13)

Honest pure-C floor LOWERED 15 → 8 via V2b + inline_ret_mode. Still NOT Tier-4 (2
missing eager fills + v0/v1 cascade remain — same structural ceiling session-12
documented but at a different masked-distance floor).

Resume artifacts:
- `permuter/saeft_v2b/base.c` — V2b + inline_ret_mode base, ~1264 iters, best 555
- `tmp/saeft_variants.py`, `tmp/saeft_variants2.py`, `tmp/saeft_variants3.py`,
  `tmp/saeft_brk_variants.py`, `tmp/saeft_mode_first_minus1.py`,
  `tmp/saeft_mode_reorder.py` — variant sweepers
- `tmp/diff_saeft_aligned.py`, `tmp/diff_saeft.py` — side-by-side diff tools
- `tmp/dump_saeft.sh` — objdump dump helper

## Related

- [[cross-jump-call-merge]] — the CALL-suffix analogue (arg count is the lever there).
- [[shared-end-label]] — the INVERSE situation (you ADD a shared end to force a merge
  / defeat per-case constant folding); read it to keep the two straight.
- [[difficult-is-not-impossible]] — this was first (wrongly) called a "fork wall";
  the matching C existed. Don't certify a cross_jump merge as impossible.
