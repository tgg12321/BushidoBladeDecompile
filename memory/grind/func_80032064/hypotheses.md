# Hypothesis ledger — func_80032064

## s2 (structural, 2026-07-13)

### H1 — CONFIRMED (was the judge's prescribed probe)
**Statement:** The literal `-0xC8` at the store site cannot reproduce the target,
because the addiu lands in the store's basic block instead of the entry BB.
**Mechanism:** GCC 2.7.2 sched.c list scheduling is basic-block-local — no
cross-BB code motion. `expand` emits a constant adjacent to its use. The store's
BB (.L800320C4) is separated from the entry BB by the .L8003208C search loop.
**Probe:** replaced the holder with the literal → `sandbox --disable all`.
**Result:** score **7** (97/97 insns). objdump: `li v0,-200` at 0x42e4, inside the
mult window. Target has `addiu $t0,$zero,-0xC8` at instruction 5, entry BB.
**Verdict: CONFIRMED.** The literal form is DEAD.

### H2 — CONFIRMED (the decisive finding)
**Statement:** The audit's flagged `sw_val` and the audit's UNFLAGGED `mul` are
the same construct; single-use-ness is not what makes the holder load-bearing.
**Mechanism:** identical — a mutable local initialized at function entry is the
only C form whose constant materializes in the entry BB (sched.c is BB-local).
**Probe:** inline the unflagged `mul = 0x50` as a literal at both use sites.
**Result:** score **12**, build_insns **95** vs target 97. Also: dropping `i = 0`
scores **2**. Three of the four entry-BB init-cluster members measured
load-bearing.
**Verdict: CONFIRMED.** Any rule calling `vel_y` a scheduling coercion must call
`speed`, `i`, and `ptr` coercions too — i.e. must call every local a coercion.

### H3 — CONFIRMED (closes the naming axis)
**Statement:** A named constant that GCC can see as compile-time-constant cannot
substitute for the mutable holder.
**Mechanism:** cc1 const-propagates it; the store expands with a literal operand.
**Probe:** `const s32 vel_y = -0xC8;` → sandbox.
**Result:** score **7** — byte-identical to the bare literal.
**Verdict: CONFIRMED.** `const` local / `#define` / `enum` / `static const` are
all one thing to cc1. The entire "name it without a mutable pseudo" axis is dead.

### H4 — CONFIRMED (closes s1 frontier item 3)
**Statement:** No legitimate restructure creates a second `-0xC8` use or a natural
hoist.
**Mechanism:** loop.c LICM inapplicable (the store is in straight-line code, not a
loop); cse never moves insns.
**Probe:** grep the file for `-0xC8` / `0xFFFFFF38`.
**Result:** the only occurrence is the one in this function. No second use exists.
**Verdict: CONFIRMED.**

### H5 — CONFIRMED (the floor-0 form)
**Statement:** The natural human C shape — four locals with initializers at the
top of the function, semantically named — reaches the target.
**Probe:** rename + fold initializers into declarations + remove the redundant
`i = 0;` statement (empty for-init clause instead).
**Result:** score **0**. Zero dead code, zero redundancy.
**Verdict: CONFIRMED.**

## Open — the classification question (escalated as ruling-request)

Is a mutable, single-use, constant-initialized local (`s32 vel_y = -0xC8;`) whose
value is genuinely STORED — i.e. fully live, not dead — a cheat, given that the
judge's completion bar says "the match must land WITHOUT this construct family"
and s2 has now proven the match cannot land without it? See the outcome JSON's
`ruling_question`. This session did NOT self-approve.
