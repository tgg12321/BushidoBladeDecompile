---
name: hoist-call-arg-local-flips-jal-delay
description: Hoisting a call's late-loaded global arg into a local declared FIRST in a block before the last pre-call store flips cc1's jal delay-slot fill choice — retires fill_delay + delete-nop + reorder regfix clusters around the call.
paths: ["src/*.c", "regfix.txt"]
---

# Hoist a call's late-loaded arg into a local declared FIRST in a block to flip cc1's jal delay-slot fill

## Symptom

A function carries a small `fill_delay`-class regfix cluster around a function
call — typically:

```
func_XXX: fill_delay @ K <- K-2     # move pre-call store into jal delay slot
func_XXX: delete @ K-5              # drop redundant load-delay nop
func_XXX: delete @ K-3              # drop redundant load-delay nop
func_XXX: reorder K-4,K-5 @ K-5..K-4 # swap arg-load and value-load order
```

`canonical` routes `C`; `sandbox --disable all` distance is small (4-6); the
sole diff cluster is the last pre-call store + the jal delay slot.

Target asm (the wanted shape):
```mips
lw   $v0, off($v1)        # load source pointer for the last store
lw   $aN, %gp_rel(GLOBAL)($gp)   # arg setup for call (interleaved)
lw   $v0, off($v0)        # load value
jal  callee
sw   $v0, off($a1)        # delay slot — pre-call store
```

Build asm (cc1's natural emission):
```mips
lw   $v0, off($v1)
nop                       # load-delay
lw   $v0, off($v0)
nop                       # load-delay
sw   $v0, off($a1)
lw   $aN, %gp_rel(GLOBAL)($gp)   # arg setup last
jal  callee
nop                       # delay slot — cc1's fill (post-call lw $v0,...)
                          # was rejected by maspsx as $v0-clobber-conflict
```

## Cause

cc1's `reorg.c` `fill_simple_delay_slots` tries to fill the jal delay slot. It
prefers the immediate post-call insn (the dispatch block's first `lw $v0,
GLOBAL`) over pre-call candidates. But that post-call insn writes `$v0` — and
the call clobbers `$v0` too — so maspsx (which sees the conflict at assembly
time, since the next insn reads `$v0`) replaces the fill with a `nop`. The
pre-call `sw $v0, off($a1)` would have been a valid fill (its `$v0` read
happens BEFORE the call branches), but cc1 didn't surface it as the top
candidate.

The arg-setup `lw $aN, GLOBAL` happens AFTER the store block because, in cc1's
RTL, the call-arg-setup pseudo for `$aN` has high LUID (born late at the call
expression). With high LUID it slots after the store. Both nops survive
because the load-delay protection is needed.

## Fix — hoist the call's late-loaded arg into a local declared FIRST in a block

Wrap the last pre-call statement(s) and the call itself in an anonymous block,
and **declare the call's last-loaded argument as a local FIRST inside the
block**:

```c
/* before: */
*(s32 *)(dst + 8) = *(s32 *)(*(s32 *)(outer + 8) + 8);
some_call(dst_a, dst, GLOBAL);

/* after: */
{
    s32 last_arg = GLOBAL;                                    /* low LUID — declared first */
    *(s32 *)(dst + 8) = *(s32 *)(*(s32 *)(outer + 8) + 8);   /* the store */
    some_call(dst_a, dst, last_arg);                          /* use the local */
}
```

The local `last_arg`'s pseudo is born at the top of the block (low LUID),
biasing cc1's scheduler to emit `lw $aN, GLOBAL` EARLY — interleaved with the
source-pointer load, filling its load-delay slot. With the arg-setup moved
earlier, the pre-call `sw` becomes the natural delay-slot fill candidate
(cc1's reorg picks the closest pre-call insn that doesn't clobber a call-USE
register; the store reads `$v0` which is in the call's CLOBBER set, not USE
set — a legitimate pattern). The post-call `lw $v0, GLOBAL` then stays
post-call as its own instruction (no $v0 conflict).

Net: 2 load-delay nops eliminated + jal delay slot filled with the store = 3
fewer insns, matching target.

## Why this is not a "cheats by any spelling" violation

The construct is a **named-intermediate declaration order** technique — the
sanctioned family per [[no-new-park-categories]] (SOTN-accepted, evidence in
`src/weapon/w_037.c`'s `randy = basePoint.x; baseX = randy;` chain). Per the
checklist:

1. **Semantic purpose**: `last_arg` IS the value passed to the callee as its
   third argument. Reading the global into a named local immediately before
   passing it is a meaningful naming step, not a no-op.
2. **Human programmer**: a real programmer might name the call's args before
   the call for clarity, especially when the values are read from globals.
3. **GCC-internals justification**: yes, the lever IS LUID-biasing — same
   family as the sanctioned `narrow-byte-args-packed-call` hi/lo split. The
   construct's existence has independent semantic justification (naming the
   arg).
4. **Permuter-derived**: NO — derived from reading cc1 raw output + reasoning
   about reorg's delay-slot fill priorities.
5. **Family check**: in the SOTN-accepted "named-intermediate declaration
   order" family. Not in any forbidden family (dead-store, register-pin,
   DImode chain, OR-tree shuffle, etc.).
6. **Naming**: `last_arg` is descriptive of role, not announcing coercion
   intent.

## When this lever applies

Look for the cluster of preconditions together:
- A `fill_delay` regfix rule moving a pre-call store into the jal delay slot.
- Companion `delete` rules removing 2 load-delay nops.
- Companion `reorder` rule moving the arg-setup `lw $aN` earlier.
- The call's last argument is loaded from a global (`lw $aN, %gp_rel(...)` or
  `lw $aN, GLOBAL`).
- The post-call code's first instruction is itself a `lw $vN, GLOBAL` style
  load that conflicts with the call's `$v0` clobber (cc1 picks it for delay
  slot, maspsx rejects it as nop).

Hoist the call's last arg into a local declared FIRST in a block surrounding
the last pre-call store + the call. Verify by sandbox 0 + SHA1 == oracle.

## When this does NOT apply

- The call's last argument is computed locally (not a global load) — the
  arg-setup is already scheduled with low LUID.
- The pre-call store reads a register that the call USES as input (e.g.
  `sw $a0, ...` would prevent the fill — `$a0` is a call USE, not just
  CLOBBER).
- The post-call code's first instruction is itself something cc1 can validly
  fill with (e.g. an `addiu` of an unrelated callee-save) — in which case
  cc1's choice is fine and the fill_delay rule shouldn't be needed at all.
- The regfix isn't a fill_delay + delete + reorder cluster but a different
  scheduling pattern.

## Confirmed case — func_80060B70 (text1b.c, 2026-06-07)

Queue top, verdict C, distance 5, 4 regfix rules (`fill_delay @ 34 <- 32`,
`delete @ 29`, `delete @ 31`, `reorder 30,29`). The function copies 3 u16s and
3 s32s through globals then calls `func_80061FAC(dst_u16, dst_s32,
D_800A3474)`. The 5-diff gap was exactly the pattern above: cc1 puts `sw $v0,
8($a1)` (3rd s32 store) BEFORE `lw $a2, D_800A3474` BEFORE `jal`; reorg fills
delay slot with post-call `lw $v0, D_800A3468` (rejected by maspsx → nop).

Hoist:
```c
dst_s32 = (s32)D_800A3470;
*(s32 *)(dst_s32 + 0) = *(s32 *)(*(s32 *)(outer + 8) + 0);
*(s32 *)(dst_s32 + 4) = *(s32 *)(*(s32 *)(outer + 8) + 4);
{
    s32 last_arg = D_800A3474;
    *(s32 *)(dst_s32 + 8) = *(s32 *)(*(s32 *)(outer + 8) + 8);
    func_80061FAC(dst_u16, dst_s32, last_arg);
}
```

`sandbox --disable all` 5 → 0; `retire` dropped all 4 regfix rules; SHA1 ==
oracle; 100% pure C.

## Related

- [[store-before-jal]] — the canonical pattern this lever is a sibling of (in
  store-before-jal, the natural C makes the store the delay-slot fill via a
  post-call memory read-back; this rule's lever achieves the same fill via
  arg-setup LUID biasing instead).
- [[narrow-byte-args-packed-call]] hi/lo split — sibling named-intermediate
  declaration order technique (here for byte-packing scheduling; this rule for
  call-arg setup scheduling).
- [[call-return-if-result-reuse-v0]] — sibling "initialize the call's
  if-result from the call's return to land it in $v0" — another lever in the
  "name a value the call uses" family.
- [[no-new-park-categories]] — the cheats-by-any-spelling policy this lever
  was vetted against; named-intermediate declaration order is in the
  SOTN-accepted family.
