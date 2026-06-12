---
name: narrow-stack-param-subword-offset
paths: ["regfix.txt"]
# broad src/*.c glob removed 2026-06-11: surfaced via codegen-technique-index
description: "A single regfix `subst` correcting a sub-word stack-parameter load offset by ±2 (e.g. `lhu $18,1090($sp)`→`1088`). Cause: this GCC-2.7.2 fork right-justifies a narrow (u16/s16) 5th+ stack parameter (lhu reads slot+2); cc1psx used slot+0. Fix: declare the param as a 4-byte type (home at offset 0) and read its low half explicitly — `u16 lo = *(u16*)&arg;` — emitting lhu from slot+0 with no ABI change."
metadata:
  type: reference
---

# Narrow stack parameter loaded at the wrong sub-word offset — read the low half explicitly

## Symptom

A function carries a single regfix `subst` that corrects the **offset** of a
sub-word stack-parameter load by ±2:

```
func_XXX: subst "lhu\t\$18,1090\(\$sp\)" "lhu\t$18,1088($sp)" @ 10
```

The function takes a narrow (`u16`/`s16`) parameter **beyond the 4 register args**
(the 5th+ argument, passed on the stack). Target loads it from the **low** half of
its 4-byte home slot (`lhu $s2, 0x440($sp)`); the cheat-free build loads from the
**high** half (`lhu $s2, 0x442($sp)`) — off by 2.

## Cause

This GCC-2.7.2 fork **right-justifies** a narrow (HImode/QImode) parameter passed
on the stack: it places the value in the high bytes of its word slot, so the
`lhu` reads `slot+2` on little-endian. The original PsyQ `cc1psx` left-justified
it (offset 0). `FUNCTION_ARG_PADDING` is not overridden in the fork's `mips.h`,
so this is a deterministic placement quirk for the narrow-stack-param case — not
something any register pin / scheduling lever can move while the parameter stays
HImode. (Confirmed via the GCC source: a HImode stack parm is passed in memory at
the padded offset; only an SImode parm stays at offset 0.)

## Why the obvious type change fails

- Declaring the param `s32`/`int` fixes the **offset** (SImode home at `slot+0`)
  but changes the **opcode** to `lw` (full-word load) and shuffles the
  register allocation — because the value must survive a call, it is loaded once
  into a callee-save reg up front, so `combine` can't narrow the `lw` back to
  `lhu`. Net: same-or-worse distance, different diffs (measured: 1 → 4).

## The fix — 4-byte param + explicit low-half read

Declare the parameter as a **4-byte type** (so its stack home is at offset 0)
and read its **low halfword explicitly** via a pointer cast:

```c
void f(u8 *a0, s16 a1, s16 a2, s16 a3, s32 arg4) {  /* was u16 arg4 */
    u16 arg4_lo = *(u16 *)&arg4;   /* lhu from slot+0 (low half of the SImode home) */
    ...
    rect[1] = arg4_lo;             /* was rect[1] = arg4; */
}
```

Taking `&arg4` makes the parameter addressable, so GCC keeps it in its incoming
SImode slot (no extra store) at offset 0; `*(u16 *)&arg4` is then a plain HImode
load = `lhu $sN, 0x440($sp)` — the target opcode AND offset, into the target
register. The caller passes a full promoted `int` either way (default arg
promotion of the narrow value), so widening the declared type does **not** change
any call site or the ABI.

Use `s16`/`*(s16*)` if the target load is `lh` (signed); `u16`/`*(u16*)` for `lhu`.

## Confirmed case — efc_buki_draw_zanzou (text1b.c, 2026-05-26)

Queue top, verdict C, distance 1, one regfix `subst "lhu\t$18,1090($sp)"
"lhu\t$18,1088($sp)" @ 10`. The 5th param `u16 arg4` loaded at `0x442`; target
`0x440`. `s32 arg4` alone → distance 4 (`lw`@0x440 + s2/s3 reg swap). `s32 arg4`
+ `u16 arg4_lo = *(u16 *)&arg4;` → `sandbox --disable all` 1→0; `retire` dropped
the rule; SHA1 == oracle. 100% pure C.

## Related
- [[halfword-index-srl-sra]] — another single-`subst` opcode/offset fork quirk
  closed by a direct byte-offset cast (sibling lever: index the bytes you want)
- [[register-alloc-pure-c]] — when an off-by-N is a register diff, not an offset
