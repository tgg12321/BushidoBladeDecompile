---
name: register-alloc-pure-c
paths: [".claude/rules/register-alloc-pure-c.md"]
# on-demand only: surfaced via codegen-technique-index (auto-loads on src/*.c)
description: "Retire a register pin in PURE C. gcc-2.7.2 prefers low regs; if target uses a lower reg than you, YOU are the anomaly (cc1 -da greg dump). Levers: block-local split, narrow type, loop precompute (dead stores are FORBIDDEN). Full session logs: [[register-alloc-deep-dive]]."
metadata:
  type: reference
---

# Retiring a register pin in pure C (no regfix, no inline asm)

When a function matches ONLY via `register T x asm("$N")` pins (or you are
tempted to add one), the pin is almost always papering over a **global
register-allocation tie that pure-C source structure can move.** Do NOT accept
the pin (cheat-asm) or fall back to a regfix register-rename until you have
run the diagnosis and tried the levers below. This retired ALL 5 pins on
`saTan0Main` (main.c MIDI dispatcher, commit 6dba050) and the last pin on
`InitHiraRmd_80047FBC` (commit 10becc3) — both to 100% pure C (COMPLETED-C). Several
prior agents had given up on saTan0Main and left a pin.

## Step 0 — diagnose: is YOUR build or the target the anomaly?

MIPS gcc-2.7.2 defines **no `REG_ALLOC_ORDER`** (confirm:
`grep -r REG_ALLOC_ORDER tools/gcc-2.7.2/*/config/mips/mips.h` → empty), so the
allocator uses **default ascending register order** — lower-numbered hard regs
are PREFERRED:

| reg | num | reg | num |
|---|---|---|---|
| `$v0` | 2 | `$a3` | 7 |
| `$v1` | 3 | `$t0..$t9` | 8-15,24,25 |
| `$a0..$a2` | 4-6 | `$s0..$s7` | 16-23 |

**The key inference:** if the **target uses a LOWER-numbered register** than
your build for the same value (e.g. target `$v1`, you `$a2`), the target is
following the natural preference and **YOUR build is the anomaly** — something
is *blocking* the preferred register in your codegen. The pin is not
fundamental; find and clear the blocker. (If the target uses a HIGHER reg than
you, the situation is rarer and a pin may be closer to justified.)

See the actual allocation with an isolated `-da` (dump-all-RTL) compile:
```
cc1 <build-flags from compile.sh> -da /tmp/standalone.c -o /tmp/x.s
```
This writes `/tmp/standalone.c.greg` (post-global-alloc) and `.lreg`. The
`;; Register dispositions:` section maps `pseudo -> hardreg` (`77 in 6` =
pseudo 77 in `$a2`). Find your value's pseudo, see which reg it got and which
pseudo holds the preferred reg across its live range. (Build the standalone =
typedefs + the function's real `extern` decls + the body; copy the exact decls
from the .c so call arg-counts/pointer-ness match — they affect codegen.)

## Lever A — block-local variable split (shrink live range / conflicts)

**Symptom:** one variable (often a pointer or byte temp) is reused across many
sites — e.g. loaded in several `switch` cases. The global allocator assigns the
WHOLE pseudo ONE register and frequently reuses a soon-to-be-argument register
(`$a0..$a3`) because the temp dies just before the call's arg setup — instead of
the lower-numbered scratch the target uses.

**Fix:** read the value into a **fresh block-local variable in ONE (or a few) of
the sites** instead of the shared variable. Shrinking the shared pseudo's live
range / conflict graph frees the allocator to give it its preferred
(lower-numbered) register across the remaining sites.

```c
/* before: shared `u8 *cmd_ptr` reused in every case -> GCC parks it in $a2 */
case 0x90: cmd_ptr = *state; ...; handler(a0, a1, b, next); goto end;
/* after: ONE case reads into a block-local -> shared cmd_ptr now gets $v1 */
case 0x90: { u8 *cp = *state; ...; handler(a0, a1, b, next); goto end; }
```

**Reference:** `saTan0Main` (6dba050) — splitting ONE switch case's `cmd_ptr`
use into a block-local `cp` retired the last pin; the shared `cmd_ptr` then
allocated to its preferred `$v1` across the other cases. A `goto` out of the
block is fine in C.

## Lever B — narrow integer type (`char`/`short` vs `int`/`u32`)

The WIDTH of an integer variable changes GCC's value-width tracking AND its
register allocation. A narrow `char`/`short` (vs `u32`/`int`):
- emits byte/half masks (`andi …,0xFF`, sign-extends) the target has where a
  wide type elided them as redundant — **retiring `andi`-style regfix rules**;
- can yield the target's callee-save allocation for the variable naturally.

**Reference:** `saTan0Main` — `u32 b` → `char b` retired an `andi` regfix rule
+ 4 register pins in a single token. NB: the build uses `-funsigned-char`, so
`char` is *unsigned* — the lever is the narrow **width**, not signedness. The
vanilla decomp-permuter's type-mutation pass finds this automatically; run it
from a pin-free base and check whether a score drop came from a type change.

## Lever C — whole-function reallocation via a loop-local / precompute

The allocator is GLOBAL, so a change deep in a loop body can shift the WHOLE
function's allocation, including the prologue. Precomputing a call argument into
a loop-local can move a prologue copy GCC otherwise copy-prop-folds away.
**Reference:** `InitHiraRmd_80047FBC` prologue staging — see
[[dead-vars-local-array]].

## Lever D — dead stores (all forms) ❌ FORBIDDEN as of 2026-05-31 / expanded 2026-06-01

> **All forms of this lever are no longer allowed.** Per user policy
> ("cheats by any spelling are forbidden, full stop" — [[no-new-park-categories]]),
> dead stores written to influence RA / scheduling / DCE-flow analysis
> are codegen-coercion cheats regardless of WHO the LHS is or HOW the
> store is wrapped. SOTN's bar rejects them.
>
> Detectors:
> - **2026-05-31** (commit `44ef3df`): `find_dead_param_assigns` —
>   `arg0 = 0;` / `param = param;` where `param` is a function parameter
>   and is never referenced after the statement.
> - **2026-06-01** (this commit): `find_dead_conditional_stores` —
>   `if (cond) { ...; v = x; } ...; v = x;` style dead conditional stores
>   to local variables, where the inner store is dead because the outer
>   overwrites it before any use. Caught the `func_8007B844` directed-
>   permuter find verbatim.
>
> `mark_done` refuses completion for any function with either pattern.
> The catalog is open: future variants identified by the same intent
> (DCE'd store written to influence pre-DCE analysis) will be added.

### Why this changed (short form)

A dead store's ONLY effect is on GCC's pre-DCE analysis — that is the
definition of a codegen-coercion cheat, same kind as a register-asm pin
(which also emits nothing and is also forbidden). A real programmer
wouldn't write it, and SOTN's bar rejects it.

### What to do instead

When natural C doesn't reach the target register allocation:

1. Try Lever A (block-local var split), Lever B (narrow integer type), and
   Lever C (whole-function reallocation via loop-local) — these are
   legitimate codegen levers, NOT coercion: they restructure the source's
   liveness/conflict graph without injecting dead stores.
2. If none of those reach target RA, the function is genuinely not
   pure-C-matchable. `queue park` it for canonical-asm review; do NOT
   add a dead `arg0 = 0;`.

### Currently affected — re-evaluation needed

`InitHiraRmd_80047FBC` (commit `10becc3`) used this lever to fix the
second pointer's `$s4` vs `$a0` allocation. With Lever D forbidden, that
function's COMPLETED-C status needs re-evaluation. Queue regen will re-route it.
The dead-vars-local-array cluster has the same dependency — see
[[dead-vars-local-array]] for the cluster's re-evaluation note.

## Order of attack

1. **Step 0 diagnosis.** Confirm the target uses the *preferred* (lower) reg →
   you're the anomaly, so it's pure-C-reachable.
2. **Run the vanilla (pin-free) permuter** from a pin-free base — it finds type
   mutations (Lever B) and some structural levers on its own.
3. **If it plateaus 1 register short**, dump `-da .greg`, find the blocker, and
   apply Lever A (block-local split) / C by hand. The instrumented-cc1 dumps
   (`BB2_ALLOC_DEBUG` / `BB2_SCHED_DEBUG` / `BB2_PRIO_DEBUG` hooks, separate
   `tmp/gccdbg/cc1` build — canonical cc1 untouched) expose the allocno
   priority table directly; see [[register-alloc-deep-dive]] for the recipe.
4. **Verify with the full SHA1 gate** (`retire` / `verify-oracle`). Isolated
   permuter scores can be UNFAITHFUL for functions with many relocations
   (saTan0Main's matching version scored 200 in isolation, while a
   non-matching variant scored 85) — always confirm in the full build.

## Confirmed CLOSURES — levers that retired a wall in this family

- **`tslPolyF4Init`** (system.c, 2026-06-14): retired a 4-regfix + forbidden
  DImode-chain cheat to **COMPLETED-C** (clean floor was sandbox-9). The wall
  was a register rotation (cc1 produced saved@s3/idx@s4/arg0@s5; target wants
  idx@s3/arg0@s4/saved@s5) PLUS a loop-exit accumulator the regfix rules faked.
  TWO pure-C levers closed it, in this order:
  1. **Explicit status flag for the exit accumulator.** The natural
     `if (count == -1) return 0` compiles to `li v0,-1; beq count,v0` —
     but the target carries a flag in `$v0` (0 on success path, -1 when
     retries exhaust) and tests `bnez $v0`. Introduce `s32 status; status=0`
     on the success branch, `status=-1` on the exhaust branch, then
     `if (status != 0)`. This reproduces the `move v0,zero` (delay slot) +
     `li v0,-1` + `bnez v0` accumulator exactly. (Floor 9→7. Sibling of
     [[exit-path-return-set-cse-join]].) NOT a cheat-by-spelling — `status`
     carries real success/exhausted meaning.
  2. **Explicit base-pointer variable for `&arr[idx]`.** `elem = &g_cd_sector_buf[idx]`
     made cc1 emit `sll idx*4 → $v1; lui/addiu base → $v0; addu s6,$v1,$v0`
     (shift first). Target wants `sll → $v0; base → $v1; addu s6,$v0,$v1`
     (base materialized first). Splitting to `s32 *base = g_cd_sector_buf;
     elem = base + idx;` evaluates the base address before the shift, flipping
     the operand order to match. (Floor 7→0.) Ordinary pointer arithmetic.
  The prologue save-ORDER is then handled by the existing `prologue_config.json`
  entry (prologue_fix reorders GCC's natural save sequence to the target's —
  a general build pass, not a per-function cheat; verify it still maps your new
  source's actions: a clean `make` SHA1 is the proof). **Siblings
  `func_80080258` / `func_80080390` (system.c) carry the IDENTICAL DImode-chain
  cheat — try these same two levers on them.**

## Confirmed limits — do NOT re-derive (full logs in [[register-alloc-deep-dive]])

Two functions hit a measured wall behind GCC 2.7.2's `global.c:624`
allocno-priority tiebreaker coupled with the `sched.c:2385` list-scheduler
priority — every named lever plus large permuter runs were executed and
recorded in the deep dive:

- **`cpu_side_move_dir_4`** (system.c): floor sandbox-15 via two
  combine-foldable chain levers; 19+ manual levers and 5000+ permuter iters
  measured negative; the chain-extender sub-technique is FORBIDDEN
  (2026-06-02, cheats-by-any-spelling). cc1psx hits the same wall (23 diffs).
- **`marionation_Exec`** (system.c): 44-rule cluster, two coupled
  register-rotation cycles; clean-target permuter plateau 1125 over 16,800
  iters (residual = pure register rotation at matched insn count).

Read the deep dive BEFORE attempting either function or re-testing any lever
it names. Remaining avenues there are non-local/policy-grade (directed
PERM_* permuter; rodata-reorder policy question).

## Related
- [[register-alloc-deep-dive]] (`memory/project/register-alloc-deep-dive.md`) —
  the full instrumented session logs (ALLOCDBG / SCHEDDBG / PRIODBG, per-lever
  measurements) behind this rule.
- [[register-asm-pins]] — pin reliability; this rule is the pure-C alternative
  to its "regfix the register name" fallback. **Read both when fighting a pin.**
- [[inline-move-aliasing]] — the asm escape valve (cheat-asm); prefer the pure-C
  levers here first.
- [[dead-vars-local-array]] — Levers C and D worked end-to-end there.
- [[minimize-regfix]] — every pin and rule is debt; these levers retire both.
- [[difficult-is-not-impossible]] — the matching C exists; coupling, not impossibility.
- [[cc1psx-calibration-only]] — the cross-check this section ran against (cc1psx
  matches target's scheduling, not target's allocation — confirms C-structure gap).
