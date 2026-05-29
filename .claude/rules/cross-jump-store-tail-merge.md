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

## Related

- [[cross-jump-call-merge]] — the CALL-suffix analogue (arg count is the lever there).
- [[shared-end-label]] — the INVERSE situation (you ADD a shared end to force a merge
  / defeat per-case constant folding); read it to keep the two straight.
- [[difficult-is-not-impossible]] — this was first (wrongly) called a "fork wall";
  the matching C existed. Don't certify a cross_jump merge as impossible.
