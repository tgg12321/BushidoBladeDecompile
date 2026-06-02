---
name: func-8007c938-investigation
description: func_8007C938 (display.c GPU draw-area / coord-packing leaf) — RESOLVED 2026-06-01 via the sibling-pattern variable-reuse lever (new_var2 holds arg0 then later the const). The matched siblings func_8007C7A0 / func_8007C86C use this exact idiom (clamp-value then const reuse on new_var2); applying it here lands the var_v0 pseudo in $v0 (target's allocation) by routing arg0 through new_var2's allocno, which breaks the in-place $a0 coalescing.
date: 2026-06-01
status: completed-c
metadata:
  type: project
---

# func_8007C938 — RESOLVED via sibling-pattern variable reuse (new_var2 holds arg0 then const)

`src/display.c:606`. Tiny 17-insn GPU coord-packing leaf:

```c
extern u8 g_gpu_type;
s32 func_8007C938(s32 arg0, s32 arg1) {
    /* return = (mask(arg1) << shift) | (arg0 & arg_mask) | 0xE5000000
     * shift  = 11, arg_mask = 0x7FF if g_gpu_type in {0,3+} (else 12, 0xFFF). */
}
```

## The lever — variable reuse on `new_var2`

The matched siblings `func_8007C7A0` and `func_8007C86C` use `int new_var2;`
**reused** — first holds the clamp-value, then reassigned to the const for the
return. The same idiom applied to 8007C938 lands the match:

```c
extern u8 g_gpu_type;
s32 func_8007C938(s32 arg0, s32 arg1) {
    s32 var_v0;
    s32 var_v1;
    int new_var2;
    var_v1 = arg1 & 0xFFF;
    new_var2 = arg0;                          /* reuse #1: holds arg0 */
    if ((u32) (g_gpu_type - 1) >= 2U) {
        var_v1 = arg1 & 0x7FF;
        var_v1 = var_v1 << 0xB;
        var_v0 = new_var2 & 0x7FF;            /* AND-source: new_var2, NOT arg0 */
    } else {
        var_v1 = var_v1 << 0xC;
        var_v0 = new_var2 & 0xFFF;
    }
    new_var2 = 0xE5000000;                    /* reuse #2: holds const */
    return var_v1 | (var_v0 | new_var2);
}
```

Sandbox `--disable all`: **5 → 4 → 0** (the 4-stage `new_var2 = arg0` reuse +
chained-OR is the closing combination). `retire` dropped the 2 register-asm pins;
SHA1 == oracle; `queue done` → COMPLETED-C.

## Why it works (RTL mechanism, instrumented via tmp/gccdbg/cc1 ALLOCDBG)

Prior session's instrumented diagnosis: pseudo 74 (var_v0) was COALESCED with
pseudo 72 (arg0) because GCC's tree → RTL saw `var_v0 = arg0 & MASK`, and since
arg0 dies at this point and var_v0 starts here, the RTL pass merged them into a
single pseudo whose preference was $a0 (reusing the source register).

**The lever breaks the coalescing.** `new_var2 = arg0; ...; var_v0 = new_var2 & MASK`
makes the AND's SOURCE be `new_var2` (its own pseudo with its own allocno),
NOT arg0 directly. var_v0's pseudo no longer gets coalesced with arg0's; it
gets a fresh allocno with no $a0 preference.

The second reuse (`new_var2 = 0xE5000000`) extends new_var2's lifetime past the
AND, giving it a long live range. When the const-OR happens at the merge,
new_var2's pseudo has long-living refs, and the const (in new_var2's pseudo)
gets allocated to a register that ISN'T $v0 — freeing $v0 for var_v0's mask
result. Target's `lui $a0, 0xE500; or $v0, $v0, $a0` falls out naturally.

The pattern's load-bearing elements:
1. **`new_var2 = arg0`** — breaks var_v0/arg0 coalescing. Without this, var_v0
   inherits arg0's $a0 preference.
2. **`new_var2` reused for the const** — extends new_var2's live range across
   the whole function, giving its pseudo a long-term allocno that doesn't
   conflict with var_v0 at the merge.
3. **`var_v1 | (var_v0 | new_var2)` parenthesization** — the inner OR groups
   var_v0 with new_var2, materializing the inner OR result as a temp pseudo
   that GCC's return-reg propagation places in $v0. (Without parens, the
   left-assoc `var_v1 | new_var2 | var_v0` regresses to score 5.)

## The lever is in the sibling rule set (canonical)

This isn't a new technique — it's the SAME pattern matched siblings 8007C7A0
and 8007C86C already use:

```c
/* func_8007C7A0 / func_8007C86C — COMPLETED-C, same structure */
int new_var2;
...
new_var2 = var_v0_2;          /* reuse #1: clamp value */
if (!D_8009BE7A) { }
var_v1 = var_a1 & 0x3FF;
var_v0 = new_var2 & 0x3FF;    /* AND source: new_var2 */
...
new_var2 = 0xE3000000;        /* reuse #2: const */
return var_v1 | (var_v0 | new_var2);
```

The siblings have the clamp logic as the "natural reason" for `new_var2`'s
first assignment. 8007C938 has no clamp (the function has no bounds check),
so the first reuse is just `new_var2 = arg0` — semantically a temporary
holding arg0. This is a legitimate pattern (not a cheat-by-spelling) per
[[no-new-park-categories]] vetting:
- `new_var2` holds a live value (used in the AND), then reassigned for the
  return. No semantic-purpose-free statements.
- A human writing this code would naturally use a temp variable for arg0 if
  the code style elsewhere does (as the siblings show).
- The justification references program logic (variable reuse mirroring matched
  siblings), not GCC internals.

## Total cheats retired

- 2 register-asm pins (`register s32 var_v0 asm("$2")`, `register s32 var_v1 asm("$3")`).
- 0 regfix rules (had none).
- 0 asmfix rules (had none).

## Prior session levers (now historical — superseded by this finding)

The prior session (2026-06-01 earlier) reached sandbox 4 via a `chained-OR
r_e5` form (`int new_var2; ...; r_e5 = var_v0 | const; return v1 | r_e5;`),
but did NOT try the `new_var2 = arg0` reuse on the AND source. That single
step closes the residual 4 to 0.

Resume-avenue #4 in the prior memory note was "Examine just-completed siblings
(func_8007C97C, func_8007C748)". This session followed it more thoroughly —
also examining func_8007C7A0/8007C86C which use the variable-reuse idiom — and
that examination produced the lever.

## Related

- [[register-alloc-pure-c]] — the playbook this fits into; the variable-reuse
  lever is a NEW lever in that family (alongside block-local var split, narrow
  integer type, loop precompute). Worth adding as Lever E.
- [[register-asm-pins]] — the pins that the lever retires.
- [[difficult-is-not-impossible]] — the cardinal rule; the matching C existed,
  just outside the prior session's explored search space. The "Resume here"
  avenue executed this session was load-bearing.
- [[no-new-park-categories]] — the policy this lever respects (legitimate
  variable reuse pattern from matched siblings, not a coercion cheat).
