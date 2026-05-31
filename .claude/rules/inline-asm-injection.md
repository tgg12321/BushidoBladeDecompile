---
name: inline-asm-injection
paths: ["src/*.c"]
description: "`__asm__ volatile(\\"addu $X, $Y, $0\\")` with hardcoded $N and no %N is the lost-codegen regfix cheat moved into C — same bytes, no GCC tracking. Score-inert under the sandbox; the audit flags it."
metadata:
  type: audit
---


## The trap

When retiring a lost_codegen regfix cheat like:

```
func_XXX: insert_after "addu $t0,$v1,$zero" @ 3
```

it is tempting to "move" the cheat from regfix.txt into the C source as a
single-instruction `__asm__` block:

```c
__asm__ volatile("addu $8, $3, $zero");   // = move $t0, $v1
```

This looks like a §6.1 / [[inline-move-aliasing]] codegen hint and
the audit historically accepted it. **It is not a codegen hint. It is the
exact same cheat in a different file.**

## The semantic difference

| Legitimate codegen hint (allowed) | Asm injection (the cheat) |
|---|---|
| `__asm__("move %0, %1" : "=r"(dst) : "r"(src))` | `__asm__("addu $8, $3, $zero")` |
| `%N` placeholders for every register | Hardcoded `$N` register references |
| `=r` / `r` / `0` constraints binding to `register T x asm("$N")` C variables | No constraints at all |
| GCC tracks the operation; the registers in the emitted instruction are GCC's choice (steered by the pin) | GCC has no view; the bytes come from the template text verbatim, just like a regfix `insert_after` |
| Used to defeat register-allocation cascades or scheduling collapses without changing what GCC understands | Used to emit a literal instruction that GCC's optimizer ate, with the exact registers the target asm uses |

The literal-register asm is byte-identical to the regfix cheat it
"replaces." Whether the bytes come from `regfix.txt` or from a C-source
`__asm__` literal, GCC never produced them. That is the rule the audit
enforces.

## How to verify you are doing the legitimate version

A correct §5.1 / inline_move_aliasing block looks like:

```c
register T dst asm("$RD");
register T src asm("$RS");
__asm__ volatile("move %0, %1" : "=r"(dst) : "r"(src));
//                      ^^^^^^^^^^^^^^^^^^^^^^^
//                      Constraints bind C-level register variables to operands
//                      → GCC chooses RD/RS based on the asm() pins, tracks lifetime
```

If the template has hardcoded `$N` and no `%N`, **GCC does not understand
the operation**. That is the injection signature. Under the engine it is score-inert — the
cheat-invisible sandbox strips it before scoring, so it can't move the score.
`audit_asm_cheats.py --check-new` flags it as a manual detector (not a wired
commit gate):

```
ASM-CHEAT GUARD: new unauthorized asm-cheat patterns since HEAD:
  Inline-asm lost-codegen INJECTION for func_XXX: HEAD had 0, now 1 (+1).
  Single-instruction __asm__ block with hardcoded $N registers (no %N
  placeholders) emitting addu RD,RS,$zero-style move ...
```

## Confirmed cases

Two `/decomp-cheat-cleanup` sessions on 2026-05-16 (4 hours apart) committed
this pattern with the same `§6.1 / feedback_inline_move_aliasing.md`
justification:

- **`c5e11ae`** (kept, on cheat queue as `asm_injection(1)`):
  `src/code6cac.c:703 func_80019310 __asm__("addu $12, $2, $0")`
  Commit message: "Replaced the regfix cheat with a single-instruction
  `__asm__ volatile ("addu $12, $2, $0")` in the C body ... Single-instruction
  inline asm is an allowed codegen-control technique per §6.1." The "literal-
  register asm landed at the exact maspsx position needed" is the giveaway —
  a legitimate use would have placeholders and let GCC choose.

- **`9118925`** (reverted in `53a441c`):
  `src/text1b.c:292 func_80048530 __asm__("addu $8, $3, $zero")` + regfix
  reorder to move the injected instruction into target's position. Same
  shape, same rationale, same wrong.

## Why `reference/inline-move-aliasing.md` got cited but the pattern was wrong

The memory's `move %0, %1` example uses placeholders; the trap is reading
the example as "single-instruction __asm__ for a move is allowed" without
noting that **the operands are placeholders bound to register-asm variables**,
not hardcoded registers. The audit_asm_cheats commit (`705e6ae`) extracted
that distinction explicitly and made it enforceable.

## What to do when you reach for this pattern

You have a lost_codegen cheat in regfix.txt. Naive plan: move the instruction
into a C-source `__asm__`. Per this memory:

1. **STOP** if your draft asm has hardcoded `$N` and no `%N`. That is the
   injection pattern. The audit will block your commit.
2. **Try the legitimate version first**: pin source and destination to
   `register T x asm("$N")` variables, use `%0`/`%1` placeholders bound via
   `=r`/`r` constraints. If GCC honors the pins, GCC emits the move and the
   bytes come from compilation.
3. **If pins are ignored** (GCC's RA fights them — common per
   [[register-asm-pins]]), the answer is NOT to fall back
   to hardcoded-register asm. It is to **restructure the C** so GCC's natural
   choice matches target, OR to genuinely escalate as canonical asm via
   `dc.sh scan-hand-coded` (per [[hand-coded-asm-recognition]] —
   STRONG tier with S1/S2/S6 only, user-authorized only).
4. Asking the user "should I do the hardcoded-register asm anyway" is not
   an option per the skill HARD RULE — but asking "I have exhausted the
   coercion ladder for THIS function, the pins don't stick, here is what I
   tried" is fine and not a §6.1 dodge.

## Sibling cheat family — alias-rename injection (added 2026-05-31)

The 2026-05-31 audit identified an analogue at the DECLARATION level —
providing a SECOND C handle for an existing global under a `_v` / `_u16` /
similar synonym via `asm("OrigSym")`:

```c
/* CHEAT — volatile alias rename; defeats CSE on uses of name_v */
extern volatile s32 D_800A14E4_v asm("D_800A14E4");

/* CHEAT — non-volatile alias rename with a different type, forces a
   width-coerced load (e.g. `lhu` on an `s32`-typed global) */
extern u16 _D_36C0_u16 asm("D_800A36C0");
```

There is no legitimate single-TU reason to provide a second C identifier
for the same memory under a different name. Same intent as the `__asm__`
injection above — coerce GCC's view of the access — different spelling.

The engine's `volatile_cheats` detector catches both variants and
`mark_done` refuses Tier-4 for affected functions. See [[inline-asm-tiers]]
expanded catalog for the full set of coercion patterns the detector covers.

## Related memories

- [[lost-codegen-insert-cheat]] — the regfix form of this cheat
- [[inline-move-aliasing]] — the LEGITIMATE pattern (placeholders,
  not hardcoded registers); make sure you read it for what it actually says
- [[register-asm-pins]] — why pins sometimes don't stick
  (the most common reason you reach for hardcoded asm in the first place)
- [[hand-coded-asm-recognition]] — when asm IS legitimately the
  answer (requires strong signals + user authorization)
