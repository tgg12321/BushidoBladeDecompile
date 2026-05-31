---
name: narrow-byte-args-packed-call
paths: ["src/*.c", "regfix.txt"]
description: "GPU/SPU command-wrapper function with 4+ s32 args where 3 are byte-truncated (`arg & 0xFF`) and packed into one 32-bit slot for a function-pointer call, carrying a pin+regfix cluster (4 register-asm pins + 4 `$a3→$s0` regfix substs on the packing accumulator). Fix: declare the 3 byte-packed params as `u8` and drop the `& 0xFF` masks — the pins, regfix, AND masks all retire."
metadata:
  type: reference
---

# Narrow `u8` parameters retire pin+regfix cluster on byte-packed-arg call wrappers

## Symptom

A "GPU/SPU command wrapper" function that:
1. Takes 4+ s32 args, 3 of which are byte-truncated (`arg & 0xFF`) and packed
   into one 32-bit slot for a downstream function-pointer call.
2. Carries a `register T x asm("$N")` pin cluster forcing a specific callee-save
   allocation, PLUS a regfix `subst` cluster renaming `$a3` (GCC's target-reg-
   propagation pick for the packing accumulator) to `$s0` (target's choice —
   the arg3-holding callee-save).

The pin cluster typically pins all 4 args to consecutive callee-saves IN
REVERSE of natural ascending: `arg0 asm("$19")`, `arg1 asm("$18")`, `arg2
asm("$17")`, `arg3 asm("$16")`. That order is unnatural per
[[register-alloc-pure-c]] Step 0 — target uses HIGHER-numbered regs than
natural ascending would pick — so the pins are doing real allocation work
that's score-inert under the sandbox.

## The fix — declare the byte-packed params as `u8`, drop the `& 0xFF` masks

```c
/* before: pins + masks + 4 regfix substs */
void func(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    register s32 a asm("$19") = arg0;
    register s32 b asm("$18") = arg1;
    register s32 c asm("$17") = arg2;
    register s32 d asm("$16") = arg3;
    s32 packed;
    func_helper(&str, a);
    packed = ((d & 0xFF) << 16) | ((c & 0xFF) << 8);
    fn(p[3], a, 8, packed | (b & 0xFF));
}

/* after: pure C — pins + masks gone, regfix retires */
void func(s32 arg0, u8 arg1, u8 arg2, u8 arg3) {
    func_helper(&str, arg0);
    fn(p[3], arg0, 8, ((u32)arg3 << 16) | ((u32)arg2 << 8) | (u32)arg1);
}
```

## Why both the pins AND the masks were debt

The pins forced target's reverse-ascending callee-save allocation
(arg0→$s3, arg3→$s0). With **`u8` typed params**, GCC's `PROMOTE_ARGS` ABI
treats the args as already-narrowed at function entry — no `andi
$rN,$rN,0xFF` instructions are needed in the body. The widening cast `(u32)`
is a no-op (the value is already in a wider reg). The natural code is
shorter AND naturally schedules the way the target wants.

The original regfix's `$a3→$s0` rename — GCC's "target reg propagation"
picking `$a3` (the call's 4th-arg slot) for the OR-chain accumulator — also
goes away: with the `andi` removed, GCC's RA recognises that the
intermediate OR can stay in arg3's already-allocated reg ($s0) until the
final OR targets $a3.

In short: target was compiled from C where the 3 packed args were declared
**`u8` (or `unsigned char`)**, NOT `s32` with byte masks. The decompilation's
`s32 + (x & 0xFF)` form is m2c's faithful but byte-noisy reconstruction —
correct semantics, wrong type signatures.

## Confirmed case — func_8007B4D0 (display.c, 2026-05-31)

Queue top, verdict C, distance 7 (pin+rule-stripped). 4 pins on
$s3/$s2/$s1/$s0 + 4 regfix substs ($a3→$s0 in packing region). Body is a
`gpu_ClearImage`-style wrapper:

```
func_8007B3A8(&g_str_clearimage, arg0);
fn(p[3], arg0, 8, ((arg3 & 0xFF) << 16) | ((arg2 & 0xFF) << 8) | (arg1 & 0xFF));
```

Pure-C grind with `s32` params + explicit `& 0xFF` masks: bottomed at score 7
(packing region matched naturally in $s0, but prologue scheduling diverged —
lui/addiu materialisation order + jal delay-slot fill). No structural lever
(in-place writeback, single-expression form, statement reordering,
block-local var split) closed the 7-diff prologue scheduling.

Changing the 3 packed params to `u8`: **sandbox 7 → 0 first try.** `retire`
dropped all 4 regfix rules; SHA1 == oracle; 100% pure C. The pins AND the
rules AND the masks were all simultaneously retired by one type change.

## The signature change is safe (caller-side)

Existing externs (`extern void func(void *, s32, s32, s32)`) and call sites
(`func(&base, 0, 0, 0)`) work without modification — K&R-style ABI promotion
passes the s32 args, callee sees the lower 8 bits. Build still matches oracle.
Updating the externs for clarity is optional but recommended.

## When this lever applies

Look for the cluster of preconditions together:
- A function-pointer or direct call where ONE of the args is computed as
  `((a3 & 0xFF) << 16) | ((a2 & 0xFF) << 8) | (a1 & 0xFF)` (or a permutation
  thereof — RGBA-style packed bytes).
- The function takes 4+ s32 args where 3+ of them are only used via
  `arg & 0xFF` (the byte masks ARE the only uses).
- Existing cheats: 4-pin `register asm` cluster + 4-rule `$a3→$s0` regfix
  cluster (or similar register-rename cheats around the packed-arg
  computation).

If any of the "byte-packed" args ALSO has a non-byte use elsewhere in the
function (e.g. `arg3 + 1` or a comparison `arg3 == 5`), you cannot declare
it `u8` — the wider arithmetic / comparison form needs the full s32. In
that case the masks are load-bearing and this lever does not apply; fall
back to the broader [[register-alloc-pure-c]] playbook.

The sibling `func_8007B564` (same display.c cluster, identical shape +
`| 0x80000000` set bit) is a candidate for the same lever.

## Related
- [[register-alloc-pure-c]] — the parent playbook; this is Lever B (narrow
  integer type) applied to the byte-packed-arg-call pattern specifically.
- [[narrow-stack-param-subword-offset]] — sibling sub-word param technique,
  there for stack-passed params at offset confusion.
- [[u16-global-lhu-lbu-low-byte]] — sibling "read the narrow half explicitly"
  lever for globals.
- [[inline-asm-policy]] — pins + regfix renames are cheat-asm / regfix
  cheats; this rule retires both via type change.
