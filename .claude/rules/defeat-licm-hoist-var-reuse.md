---
name: defeat-licm-hoist-var-reuse
description: When GCC hoists a loop-invariant (e.g. limit-1) that the target recomputes INLINE, reuse one C variable for a used loop-variant AND the invariant — multi-set pseudo isn't a loop.c movable, so it's not hoisted. Pure C, no asm.
paths: [".claude/rules/defeat-licm-hoist-var-reuse.md"]
# on-demand only: surfaced via codegen-technique-index (auto-loads on src/*.c)

---

# Defeat loop.c invariant-hoisting in pure C by reusing a scratch variable

> **Historical framing note (2026-08-30):** this rule predates the removal of the
> regfix/asmfix rule system (retired at zero rules; machinery deleted). Where the
> symptom text says a function "carries a rule", read it as "the honest build shows
> this diff shape vs target". The technique itself is unchanged.

## Symptom

The target computes a loop-invariant **inline, every iteration** (e.g. `addiu
v0,s5,-1` = `limit-1` recomputed each pass, with `limit` live in a callee-save).
Your pure-C `if (i == limit - 1)` instead makes GCC **hoist** `limit-1` out of the
loop into a fresh callee-save (`s8`) — the frame grows and the whole prologue/
epilogue + branch targets cascade into a large diff. Switching to `i + 1 == limit`
avoids the hoist but emits `addiu i+1` / `bne i+1,limit` — the wrong operands
(target has `addiu limit-1` / `bne i,limit-1`).

## Why GCC hoists it (loop.c, confirmed in `tools/gcc-2.7.2/loop.c`)

`limit - 1` is a **non-trapping, single-set, loop-invariant** pseudo, so `scan_loop`
records it as a *movable* (the `may_trap_p` exclusion at line ~715 only blocks
*trapping* invariants — memory loads, divides — from being speculatively hoisted past
a conditional/call; a plain `addiu` is never excluded). `move_movables` then hoists
it whenever `threshold * savings * lifetime >= insn_count`, which for a small
(~29-insn) loop-with-call is essentially always true. So **no amount of placing the
computation inside an `if` or reordering prevents the hoist** — the value is simply
invariant and cheap.

`volatile`-qualifying the base *does* defeat the hoist (the value becomes non-
invariant) but **spills it to the stack** (an extra `lw` each iteration) — wrong,
the target keeps the base in a register. **Also FORBIDDEN as of 2026-05-31**:
declaring a game-state global as `volatile` (or casting via `*(volatile T *)`) to
coerce GCC's hoist analysis is a cheat per [[inline-asm-policy]] (expanded
catalog). An `__asm__` barrier (`"=r"(x):"0"(x)`) also works but is **cheat-asm**
(never committable); it's what the sibling `func_8003DE14` settled for — that
function's status now needs re-evaluation under the expanded catalog.

## The pure-C fix — make the invariant's pseudo MULTI-SET

A movable must be set **once** in the loop (loop.c line ~705: `n_times_set==1 ||
consec_sets_invariant_p`). If the invariant lands in a pseudo that is **also assigned
a *used* loop-variant value earlier** (non-consecutively), `n_times_set > 1` and it is
**not a movable → not hoisted → recomputed inline.** And it naturally reuses that
variant's register — which is exactly what the target does.

**Read the target for which register it reuses.** In func_8003DBE4 the target's `v0`
holds the `*pal` store result (`sw v0,0(a0)`), then is **immediately reused for
`limit-1`** (`addiu v0,s5,-1`). So route both through one C variable:

```c
s32 tmp;
*colors = (*colors & 0xFF000000) | (*pal & rgb_mask);
tmp = (*pal & 0xFF000000) | ((u32)colors & rgb_mask);  /* variant value, USED next */
*pal = tmp;                                            /* the use — prevents DCE */
colors = (s32 *)((u8 *)colors + 0x30);
tmp = limit - 1;                                       /* REUSE tmp for the invariant */
if (i == tmp) {                                        /* -> addiu limit-1; bne i,limit-1 inline */
    D_800905F8 = idx;
}
```

The first `tmp = …; *pal = tmp;` must genuinely **use** `tmp` (here, the store),
otherwise DCE removes the variant assignment and `tmp` collapses back to a single-set
invariant that hoists again. Pick the variable the target actually reuses (diff the
target asm), so the register matches too.

## Confirmed case — func_8003DBE4 (code6cac_c2.c, 2026-05-28)

Honest distance 35 → 0; **36 regfix rules dropped** (incl. a brittle hardcoded-global-
label `subst ".L280-4"`), 100% pure C, SHA1 == oracle. The full match needed three
pure-C levers found by reading the RTL/target, not the compiler:
1. **dead local** `s32 buf[2];` for the target's 8 extra frame bytes ([[dead-vars-local-array]]).
2. **redundant dead store** — `step = base_val - arg0;` duplicated into one branch arm
   — to steer the constant into `v0` (a scheduling/RA tie diagnosed from the `.greg`
   dump; see [[register-alloc-pure-c]]).
3. **this rule** — reuse `tmp` for the `*pal` value and `limit-1` to defeat the
   loop-invariant hoist.

The hoist had looked like a "compiler-fork wall"; it was not — see
[[difficult-is-not-impossible]]. The matching C existed; it just required reading the
target's exact register reuse and the loop.c movable rules.

### The threshold, priced (measured, not inferred)

`move_movables` (loop.c:1631) moves a movable when

    already_moved || (threshold * savings * m->lifetime) >= insn_count
                  || (m->forces && m->forces->done && n_times_used[...] == 1)

with `threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)` (loop.c:532).

**The number on this target: `n_non_fixed_regs == 60`.** `FIRST_PSEUDO_REGISTER` is 68
(mips.h:1181) and `FIXED_REGISTERS` (mips.h:1188) has exactly 8 ones — `$0`, `$at`,
`$k0`, `$k1`, `$gp`, `$sp`, `$ra`, and reg 67 (fp status). `n_non_fixed_regs` is counted
in `init_reg_sets_1` (regclass.c:380-387) as `FIRST_PSEUDO_REGISTER` minus the fixed
count, **after** `CONDITIONAL_REGISTER_USAGE`. That macro (mips.h:524-535) would fix the
32 FP registers under `!TARGET_HARD_FLOAT` — but our build never passes `-msoft-float`
and `TARGET_CPU_DEFAULT=16` is `MASK_GAS` only, so **hard float is on and the 32 unused
FP regs are counted as allocatable**. Nothing else moves `fixed_regs` (regclass.c:499 is
`-ffixed-REG`, :529 is `globalize_reg`; neither fires here).

So:

| loop | threshold |
|---|---|
| call-free   | `2 * (1 + 60)` = **122** |
| with a call | `1 * (1 + 60)` = **61**  |

(The separate giv threshold at loop.c:3241 uses `(3 + n_non_fixed_regs)` → **126 / 63**.)

**Measured confirmation** — `func_800335D8`
(`tmp/grind/func_800335D8/dumps/code6cac_b.loop`) brackets the with-call number from both
sides in one dump:

    Loop from 199 to 423: 71 real insns.
    Insn 216: regno 129 (life 1), move-insn savings 1 not desirable      <- 61*1*1 < 71
    Loop from 25 to 176: 46 real insns.
    Insn  52: regno  91 (life 1), move-insn savings 1  moved to 434      <- 61*1*1 >= 46

Both loops contain calls. A `savings 1, life 1` movable is refused at 71 insns and taken
at 46, so the with-call threshold is in `[46, 71)` — **61 fits; the ~31 of the earlier
draft is refuted by the 46-insn loop.**

### Direction matters: hoisting is defeated by a BIGGER loop, not a smaller one

The test fires when `insn_count` is **small**. Removing insns from the loop body makes
the inequality *more* true and hoists *more*. The only size-based way to suppress a hoist
is to push the body **past** `threshold * savings * lifetime` — and with a call-free
threshold of 122 and `lifetime` typically well above 1, that product is normally in the
hundreds. **Treat the desirability test as unwinnable and use the multi-set-reuse lever
(below), which attacks movable *admission* in `scan_loop`, not desirability.**

Two things still worth knowing:

1. **`savings` and `lifetime` multiply the threshold**, so the practical bound is far
   above 122. `life 1, savings 1` is the weakest possible movable and is the only case
   where the raw threshold is the bound.
2. **`insn_count` doubles — cumulatively — once any movable's regno was already moved**
   (loop.c:1612, dumped as `halved since already moved`; the local `insn_count` is never
   restored, so the doubling persists across the remaining movables in that pass). This
   is the one mechanism that can flip a marginal decision, and it is not under your
   control from C.

### Read the pass's own decisions instead of inferring them

`cc1 -da` emits `<dumpbase>.loop` with one line per movable:

    Loop from A to B: N real insns.
    Insn N: regno R (life L), [move-insn] savings S  moved to M      <- taken
    Insn N: regno R (life L), [move-insn] savings S  not desirable   <- refused

`pwsh tools/grinder/dump.ps1 <func>` produces it under `tmp/grind/<func>/dumps/<tu>.loop`
using the project's exact flags. To read one function's decisions:

    awk '/;; Function <func>/{s=1} s&&/^;; Function/&&!/<func>/{exit} s' \
        tmp/grind/<func>/dumps/<tu>.loop | grep -E '^(Loop from|Insn [0-9]+:)'

(`tmp/loopdec.py` is the older equivalent, hardcoded to `tmp/dump_<tag>/pre.i.loop`.)
Grep the same slice for `call_insn` to settle `loop_has_call` — do not assume it from the
target asm, since inlining and the C's current shape decide it.

Corollary — hoists cost **callee-saved registers**, and that is usually the real diff.
`func_8003EB84` was saving `s0`-`s4` (frame 0xA0) against a target that is a leaf with
`.mask 0x0` (frame 0x88), because loop.c hoisted five invariants out of the innermost
loop where the target hoists three; the extra symbol addresses, a constant `1`, and a
`t4 << 5` giv became function-long lives and pushed the allocator into `s0`-`s4`. Three
applied multi-set-reuse levers took the score 98 -> 86 -> 83 -> **81**, freeing `s4` and
`s3` in turn:

  1. one `u8 *base` reused for the variant `&D_800A87E0[vidx]` and then for both
     table bases, so the two `lui/addiu` pairs are emitted INSIDE the loop as the
     target has them (98 -> 86, `s4` freed);
  2. `vflag = 1; e2[0x58] = vflag;` — kills the hoisted constant (86 -> 83, `s3` freed);
  3. `v1 = t4 << 5; a3 = D_800A8FB0[v1 + t1];` — kills the giv hoist (83 -> 81).

Note what did the work: each lever made the pseudo **multi-set** so `scan_loop` never
admitted it as a movable. None of them changed the loop's size decision — the loop is
call-free at 61 insns against a threshold of 122, i.e. the desirability test was never
close.

The residual there is a *different* mechanism and should not be attacked with this
lever: two hoisted bases land in `s0`/`s1` instead of the target's `t8`/`t7` because MIPS
defines no `REG_ALLOC_ORDER`, so `s0`-`s7` are tried before `t8`/`t9` — closing it needs
~2 more concurrently-live registers in the loop nest, not fewer hoists. See
[[local-alloc-death-count-class-wall]] for the ascending-scan machinery.

## Related
- [[difficult-is-not-impossible]] — this is the case study; "hard" ≠ "impossible"
- [[register-alloc-pure-c]] — companion RA levers (dead store, block-local split)
- [[split-read-defeats-hoist]] — the *trapping*-invariant hoist case (memory address);
  defeated by branch-duplication. This rule is the *non-trapping* case (register
  arithmetic), defeated by multi-set var reuse.
- [[inline-asm-policy]] — why the `__asm__` barrier alternative is cheat-asm, not a match
- [[local-alloc-death-count-class-wall]] — the ascending first-free scan that
  decides WHICH register a hoisted invariant lands in once it is hoisted.
