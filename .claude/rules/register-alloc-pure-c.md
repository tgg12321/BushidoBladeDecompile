---
name: register-alloc-pure-c
paths: ["src/*.c"]
description: "Retire a register pin in PURE C. gcc-2.7.2 prefers low regs; if target uses a lower reg than you, YOU are the anomaly (cc1 -da greg dump). Levers: block-local split, narrow type, loop precompute, dead assignment."
metadata:
  type: reference
---

# Retiring a register pin in pure C (no regfix, no inline asm)

When a function matches ONLY via `register T x asm("$N")` pins (or you are
tempted to add one), the pin is almost always papering over a **global
register-allocation tie that pure-C source structure can move.** Do NOT accept
the pin (tier-3 debt) or fall back to a regfix register-rename until you have
run the diagnosis and tried the levers below. This retired ALL 5 pins on
`saTan0Main` (main.c MIDI dispatcher, commit 6dba050) and the last pin on
`InitHiraRmd_80047FBC` (commit 10becc3) — both to 100% pure C (tier-4). Several
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

## Lever D — dead self-assignment to break a value association

A dead `param = 0;` (DCE'd to ZERO emitted instructions, since the param is
unused afterward) can break GCC's `paramreg == value` association so a later
use binds to a callee-save instead of the still-live param register.
**Reference:** `InitHiraRmd_80047FBC` second pointer (`$s4` vs `$a0`) — see
[[dead-vars-local-array]].

## Order of attack

1. **Step 0 diagnosis.** Confirm the target uses the *preferred* (lower) reg →
   you're the anomaly, so it's pure-C-reachable.
2. **Run the vanilla (pin-free) permuter** from a pin-free base — it finds type
   mutations (Lever B) and some structural levers on its own.
3. **If it plateaus 1 register short**, dump `-da .greg`, find the blocker, and
   apply Lever A (block-local split) / C / D by hand.
4. **Verify with `dc.sh build` (full SHA1).** Isolated permuter scores can be
   UNFAITHFUL for functions with many relocations (saTan0Main's matching
   version scored 200 in isolation, while a non-matching variant scored 85) —
   always confirm a candidate in the full build, not just the permuter score.

## Related
- [[register-asm-pins]] — pin reliability; this rule is the pure-C alternative
  to its "regfix the register name" fallback. **Read both when fighting a pin.**
- [[inline-move-aliasing]] — the asm escape valve (tier-3); prefer the pure-C
  levers here first.
- [[dead-vars-local-array]] — Levers C and D worked end-to-end there.
- [[minimize-regfix]] — every pin and rule is debt; these levers retire both.
