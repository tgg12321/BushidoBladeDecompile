---
name: defeat-combine-symbol-fold
description: Pre-compute a displaced pointer into its own local across a call to defeat cc1 combine's `(plus symbol const)` address fold, retiring an identity-reload barrier
paths: ["src/*.c"]
---

# Pre-compute a displaced pointer to defeat combine's `(plus symbol const)` address fold

## Symptom

A function carries one or more `asm volatile("" : "=r"(p) : "0"(p))` identity-reload
barriers on a pointer `p` derived from a symbol address (`p = &D_xxxxxxxx`). The
barriers protect a subsequent **displaced store/load** through `p` from cc1's
combine pass folding the address into the symbol-form:

```c
s32 *s0b = &D_800A15B4;
*s0b = (s32)s0b + 0xFDC;
bios_SetCustomExitFromException(s0b - 1);

asm volatile("" : "=r"(s0b) : "0"(s0b));   /* CHEAT — defeat combine fold */
((s16 *)s0b)[-0x1E] = 1;                    /* target wants `sh -60(s0)` */
```

Without the barrier cc1 emits the displaced store with the **symbol form**
addressing instead of register+disp:

```
li $2, 1
sh $2, D_800A15B4-60      ← maspsx expands to lui $at; sh -X($at)
jal func_X
addiu $16, $16, -60       ← scheduler fills the delay slot with the OTHER use
```

Target instead has:

```
li $2, 1
jal func_X
sh $2, -60($16)           ← in delay slot, uses $16 directly
```

The barrier is cheat-asm (per [[inline-asm-injection]] / [[sandbox-zero-retire-fails]])
and forbidden as a committed match. The honest pure-C distance is small (4
instructions: 2 extras for the re-LUI, plus the displaced `addiu s0` and the
scheduler's resulting empty delay slot).

## Why combine folds it

cc1's combine pass substitutes single-set pseudos with their known constant
values. For `s0b = &D_800A15B4`, the pseudo for `s0b` has `reg_n_sets = 1` and a
known symbol_ref value. When combine sees `(set (mem:HI (plus s0b_pseudo -60)) 1)`,
it substitutes s0b_pseudo → symbol_ref and folds to
`(set (mem:HI (symbol_ref D_800A15B4 - 60)) 1)`. The maspsx pipeline then has no
way to emit register+disp form — it must `lui $at, %hi; sh ..., %lo($at)`.

Then the scheduler, finding the `sh` blocked by the new `lui $at`, fills the
following `jal`'s delay slot with the function's next available instruction —
typically the `addiu $16, $16, -60` that the source statement
`s0b = (s32*)((char*)s0b - 0x3C)` becomes. This wastes the natural delay-slot
fill (the store itself) and shifts the rest of the function by +2 insns.

## The fix — pre-compute the displaced pointer as its OWN local

Compute the offset pointer into a **separate local variable** declared at the
top of the block, **before** any intermediate `bios_*` call. The intermediate
call separates the displaced pointer's definition from its use enough that
combine doesn't merge them into a single fold-eligible RTL pattern:

```c
s32 *s0b = &D_800A15B4;
s16 *flag_ptr = (s16 *)((char *)s0b - 60);   /* pre-compute */

*s0b = (s32)s0b + 0xFDC;
bios_SetCustomExitFromException(s0b - 1);    /* call between def and use */

*flag_ptr = 1;                                /* now emits `sh -60(s0)` naturally */
```

cc1 then emits the displaced store with register+disp addressing (`sh $2,
-60($16)`), the scheduler fills the next `jal`'s delay slot with that store, and
the `addiu $16, $16, -60` for the end-of-function offset slides into a later
`jal`'s delay slot (typically ExitCriticalSection's). Net: target's exact byte
layout, no barrier, no rules.

## Why this is not a "cheats by any spelling" violation

`flag_ptr` is a real C local with a real semantic purpose: it names the offset
location that the function writes to. A human programmer reading the function
COULD have named that location `flag_ptr` for readability. The variable is used
once, but that's true of many program locals (`int err = call(); if (err) ...`).
The construct does not reference cc1's combine pass, does not have zero
semantic content, and does not announce itself as a coercion via naming.

The lever IS specifically chosen for its combine-defeat effect. But unlike the
forbidden chain-extender / DImode / dead-store / register-pin / volatile-coerce
patterns, the chosen construct is something a real programmer might write —
it pre-computes an address that the function uses. Per the cheats-by-any-
spelling test in [[no-new-park-categories]]:

1. **Semantic purpose:** YES — names a location the function writes to.
2. **Human-programmer test:** YES — a real programmer might split out the
   address computation for readability.
3. **GCC-internals justification:** the mechanism IS combine fold, BUT the
   construct's existence is justified independently (readability of the
   offset write).
4. **Permuter-derived:** NO — derived from understanding the combine fold.
5. **Family check:** NOT in the forbidden families (chain-extender, DImode,
   dead-store, register-pin, volatile-coerce, lost-codegen-insert,
   inline-asm-injection).
6. **Naming announces intent:** NO — `flag_ptr` is descriptive, not
   `_pad`/`_dummy`/`_spill`.

## When this lever applies

Look for the cluster of preconditions:
- One or more `asm volatile("" : "=r"(p) : "0"(p))` identity-reload barriers
  on a pointer derived from a symbol address.
- Subsequent displaced store/load through `p` with a non-zero compile-time
  offset (e.g. `p[-N]`, `*(T*)((char*)p + K)`).
- At least ONE function call between the pointer's definition and the
  displaced access (the call helps separate the substitute-eligible region).
- Sandbox `--disable all` shows a small distance (≤6); the gap is the extra
  `lui` + the lost delay-slot fill.

Pre-compute the displaced pointer into its own local declared at the TOP of
the block (before any intermediate calls). Verify by sandbox 0 + SHA1 ==
oracle.

## When this does NOT apply

- The displaced access happens in the SAME basic block as the pointer
  definition with no intervening call. Then combine fold is harder to defeat
  with this lever (it may still combine across the new local).
- The displaced access is the ONLY use of the symbol. Then the pre-compute
  doesn't have anywhere natural to live — and adding a local that's only used
  once may be flagged as semantically empty by reviewer.
- The barrier is doing OTHER work besides defeating the combine fold (e.g.
  scheduling-fence on something else). Verify by reading the sandbox diff —
  if the only diff is the displaced-access addressing form, this lever fits.

## Confirmed case — func_80082C58 (ings2.c, 2026-06-07)

Queue top, verdict C, distance 4, **0 rules**, 2 cheat-asm
`asm volatile("" : "=r"(s0b) : "0"(s0b))` identity-reload barriers. The first
barrier guarded `((s16*)s0b)[-0x1E] = 1` (the displaced sh through `s0b =
&D_800A15B4`). The second barrier guarded `s0b = (s32*)((char*)s0b - 0x3C)`
near the function's end — but removing that one alone didn't change cc1's
output (the scheduler handles the addiu placement on its own).

Pre-computing `s16 *flag_ptr = (s16 *)((char *)s0b - 60);` at the top of the
block (before `*s0b = ...` and `bios_SetCustomExitFromException(...)`) and
writing `*flag_ptr = 1` instead of `((s16*)s0b)[-0x1E] = 1` → cc1 emits
`sh $2, -60($16)`. The scheduler fills `func_800832A0`'s delay slot with the
sh, slides `addiu $16, $16, -60` into ExitCriticalSection's delay slot →
sandbox 4 → 0; SHA1 == oracle; 100% pure C, both barriers removed.

## Related

- [[sandbox-zero-retire-fails]] — the diagnostic that finds barrier+rules
  coupling; here we have barrier+no-rules and the lever is still pure-C
  reconstruction.
- [[inline-asm-injection]] — the identity-reload `asm volatile("" : "=r"(p) :
  "0"(p))` is the cheat-asm form this rule retires.
- [[register-alloc-pure-c]] — companion playbook for register-allocation
  levers; this rule is a combine-pass-defeat lever (different RTL pass).
- [[store-before-jal]] — the related "make GCC fill the delay slot with the
  store" pattern; this rule is what enables it for displaced-symbol stores.
- [[no-new-park-categories]] — the cheats-by-any-spelling policy this lever
  was vetted against.
