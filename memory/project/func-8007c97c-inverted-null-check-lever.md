---
name: func-8007c97c-inverted-null-check-lever
description: func_8007C97C (display.c RGB-packer) RESOLVED to COMPLETED-C via inverted null-check (`if (arg0 != NULL) { body } return 0;`) combined with m2c-shape body + r_e2 named intermediate + sll-reassignment. The lever replaces the prior-session's FORBIDDEN goto-end-with-ret_val accumulator (commit bc346de reverted) without any forbidden construct. Retired 9 regfix rules; SHA1 == oracle.
date: 2026-06-02
status: completed-c
metadata:
  type: project
---

# func_8007C97C — inverted null-check lever closes m2c-shape body to COMPLETED-C

`src/display.c:620`. Small 33-insn RGB→GPU-command packer:

```c
s32 func_8007C97C(u8 *arg0) {
    /* if arg0==NULL return 0; else read 4 byte/halfword channels from
     * arg0[0,2] and arg0[4..7]; >>=3 each; pack into 32-bit GPU command
     * 0xE2NNNNNN; spill each pre-shift channel to sp[0..3]. */
}
```

## The lever — invert the null-check sense

The prior committed match (commit `bc346de`, REVERTED) used a forbidden
return-value accumulator + goto-end form:

```c
/* FORBIDDEN — goto-end-prologue-delay-slot per .claude/rules/ */
s32 ret_val;
if (arg0 == NULL) { ret_val = 0; goto end; }
... body ...
ret_val = expr;
end:
return ret_val;
```

The CLEAN form — same body, NO ret_val accumulator, NO goto-end —
is **inverting the null-check sense**:

```c
s32 func_8007C97C(u8 *arg0) {
    s32 sp[4];
    u32 temp_a1, temp_v0;
    s32 temp_a2, temp_v1;

    if (arg0 != NULL) {            /* INVERTED — was `if (arg0 == NULL) return 0;` */
        temp_a1 = (u8) arg0[0] >> 3;
        sp[0] = temp_a1;
        temp_a2 = (s32) (-*(s16 *)(arg0 + 4) & 0xFF) >> 3;
        sp[2] = temp_a2;
        temp_v0 = (u8) arg0[2] >> 3;
        sp[1] = temp_v0;
        temp_v0 = temp_v0 << 0xF;            /* sll-reassign (SOTN-allowed) */
        temp_v1 = (s32) (-*(s16 *)(arg0 + 6) & 0xFF) >> 3;
        sp[3] = temp_v1;
        {
            u32 r_e2 = (temp_a1 << 0xA) | 0xE2000000;   /* named intermediate (SOTN-allowed) */
            return temp_v0 | r_e2 | (temp_v1 << 5) | temp_a2;
        }
    }
    return 0;
}
```

`sandbox --disable all`: 24 → 5 (with the m2c-shape body using
`if (arg0 == NULL) return 0;`) → **0** (after inverting to
`if (arg0 != NULL) { body... return X; } return 0;`).
`retire` dropped all 9 regfix rules. SHA1 == oracle. COMPLETED-C.

## Why the inversion works (RTL/reorg mechanism)

The target has a "shared-epilogue with delay-slot-filled early-exit"
shape:

```
bnez   $a0, .L_body          # if nonzero -> body
addiu  $sp, $sp, -0x10        # delay slot: prologue (taken either way)
j      .L_end                  # else -> jump to shared epilogue
addu   $v0, $zero, $zero       # delay slot: return 0
.L_body:
... body ...
sw     $v1, 0xC($sp)
.L_end:
addiu  $sp, $sp, 0x10           # shared epilogue
jr     $ra
nop
```

The `if (arg0 == NULL) return 0;` form makes GCC emit `beqz $a0, end;
addiu sp,-16` (branch-IF-ZERO to a tail epilogue that includes
`move $v0,zero`). Because the body's fall-through then has to JUMP
OVER the `move $v0,zero` to reach the addiu/jr, GCC emits a `j` from
the body's last sw → score 5 (5-instruction structural shape mismatch
across the entry/exit region).

The INVERTED form `if (arg0 != NULL) { body... return X; } return 0;`
is what GCC's tree → RTL lowering naturally compiles as:
- `bnez $a0, .L_body` (positive sense — branch to taken-block)
- fall-through to the trailing `return 0;`: `j .L_end; move $v0,zero`
  (with prologue's `addiu sp,-16` filling the bnez delay slot, and
  `move $v0,zero` filling the j delay slot)
- `.L_body:` is the if's body, ending with the computed-return
- `.L_end:` is the shared epilogue (`addiu sp,16; jr ra; nop`)

This is exactly target's layout — `bnez + prologue-in-delay + j-to-shared-end + v0=0-in-delay + body + shared-epilogue`. GCC's reorg.c
fills the bnez's delay slot with the sp-adjust (it's a no-side-effect
in the taken thread because the taken thread also needs the frame),
and the j's delay slot with the `move v0,zero` (the if's else-side
return-value computation hoisted into the j's delay).

## Vetted: not a cheat

Per the cheat-reviewer 6-test checklist
([[review-discipline-before-commit]] / [[no-new-park-categories]]):

1. **Semantic purpose**: `if (arg0 != NULL) { compute; return X; } return 0;` is
   idiomatic defensive C — exactly how a human would write a null-tolerant
   accessor. No construct lacks a semantic role.
2. **Human-programmer test**: a programmer writing this from spec
   ("pack RGB+intensity into a GPU command, return 0 if no input") would
   naturally write `if (arg0) { compute and return; } return 0;`.
3. **GCC-internals justification**: the lever's WHY references the
   if-vs-if-not control structure — observable C, not RTL coercion.
4. **"Permuter said so" test**: this was DERIVED by reading target asm
   and recognizing the entry/exit shape, then hand-flipping the null
   check. No permuter involvement.
5. **Family check**: NO dead-goto label-pad, NO DImode chain, NO
   goto-end-with-ret_val, NO param-local-alias, NO dead-conditional-
   store, NO empty-if dead-read, NO volatile-T-local, NO `(void)local;`,
   NO register-asm pin, NO INLINE_MOVE_ALIASING, NO lost-codegen
   insert. SOTN-allowed: variable-reuse (`temp_v0 = temp_v0 << 0xF;`)
   and named-intermediate (`r_e2`).
6. **Naming-announces-intent test**: `temp_a1/a2/v0/v1` are m2c-style
   intermediate names tracking the target's $a1/$a2/$v0/$v1 dataflow
   — natural decompilation naming. `r_e2` semantically names the
   0xE2-tagged GPU command intermediate. `sp[4]` is an array that IS
   referenced (4 writes); the detector's reference-rule excludes it.
   The bc346de commit ALSO used `sp[4]`; the issue with bc346de was
   the goto-end+ret_val, not sp[4].

## The 5 cheat-asm pins in HEAD were not load-bearing

The committed HEAD source carried:
- `register u8 *p asm("$4") = arg0;`
- `register s32 r asm("$5"); register s32 b1 asm("$6");`
- `register s32 g asm("$2"); register s32 b2 asm("$3");`
- inner blocks: `register s32 e2 asm("$4"); register s32 b2sh asm("$4");`

All 5 pins were STRIPPED by the cheat-invisible sandbox to give the
honest distance of 24 (with the cheat-asm-pinned HEAD body). Removing
them + rewriting to the m2c-shape inverted-null form lands sandbox 0
without ANY pins.

## Confirms `if`-sense as a lever for early-exit-then-shared-epilogue

`if (ptr == NULL) return 0;` and `if (ptr != NULL) { body; return X; } return 0;`
are semantically equivalent but produce DIFFERENT RTL layouts:
- `==NULL`+early-return ⇒ branch-if-zero, body fall-through, tail epilogue
  inline; if the function has a frame, the early-exit must jump OVER the
  body's epilogue or carry its own.
- `!=NULL`+enclosed-body ⇒ branch-if-nonzero TO the body block, the trailing
  `return 0;` is the fall-through (with j-to-shared-end and v0=0 in delay slot).

For functions that NEED a frame (locals, spills) AND have a single
early-exit-with-constant-return, target's shared-epilogue layout is
reachable via the inverted form. The `if (NULL) return 0;` form is
NOT reachable because GCC won't synthesize the j-to-shared-end +
v0=0-in-delay-slot pattern from the natural early-return.

## Related

- [[goto-end-prologue-delay-slot]] — the FORBIDDEN technique this
  lever replaces. The same 4-insn entry pattern (bnez/prologue-delay/
  j/v0=0-delay) is reachable WITHOUT a return-value accumulator by
  flipping the null-check sense.
- [[register-alloc-pure-c]] — the m2c-shape body's `temp_v0 = temp_v0
  << 0xF;` is variable reuse (now an explicit SOTN-allowed lever per
  commit 161c6c3, complementing this rule's named-intermediate `r_e2`).
- [[no-new-park-categories]] — the policy this lever respects. The
  inverted-null-check construct has clear semantic purpose (idiomatic
  null-tolerant accessor), would be naturally written by a human, and
  its justification references program logic (if/else fall-through),
  not RTL coercion.
- [[difficult-is-not-impossible]] — the matching C existed. Prior
  session reached 19 with cheat-asm pins + 9 rules; this session
  found the clean pure-C closing form by flipping the null check.
