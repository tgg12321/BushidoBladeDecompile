# func_80040A78 (text1a.c) — WIP @ floor 1 (HEAD honest: 7)

## TL;DR

**HEAD matches via two cheat-asm `register asm("$N")` pins** on the loop counter
and pointer (`register s32 var_a1 asm("a1"); register s32 var_v1 asm("v1");`).
Pins are forbidden cheat-asm per [[inline-asm-policy]]; function is INCOMPLETE.

**Lever found this session:** change loop counter type `s32 var_a1 → u32 var_a1`.
This defeats GCC 2.7.2's `check_dbra_loop` (loop.c:5849) count-down rewrite,
which only fires on signed-LT comparisons. Without reversal, GCC emits the
natural count-up form with registers matching target. **Floor 7 → 1.**

**Remaining single diff (1-bit):** `sltiu v0,a1,0x14` (ours) vs target's
`slti v0,a1,0x14`. The two requirements (signed LT compare *and* no SR
reversal) are mutually exclusive in natural C without a register pin.

## Resume instructions

1. `git rev-parse --short HEAD` — current candidate was measured against
   `4dd017d8` (queue top: `func_80040A78`, distance 7, rules 0, verdict C).
2. Apply `memory/wip/func_80040A78/candidate.c` to `src/text1a.c` (replaces
   lines 372–404 at HEAD `4dd017d8`).
3. Confirm: `& tools/eng.ps1 sandbox func_80040A78 --disable all` → expect
   `score 1, build_insns 51, target_insns 51`.
4. From this base, attack the remaining `sltiu → slti` gap (see `meta.json`
   `next_hypotheses`).

## Why the natural lever space is exhausted at score 1

The diff is purely the comparison opcode. Achieving `slti` from C requires a
SIGNED comparison code in GCC's RTL. With `s32 var_a1`, the compare is signed
and `check_dbra_loop` rewrites to count-down (`addiu -1; bgez` form) — wrong
shape. With `u32 var_a1`, no reversal but `sltiu` is emitted.

The HEAD cheat-asm pin (`register asm("a1")`) resolves both at once: pinned
pseudos can't be replaced by `check_dbra_loop`'s SR pass (it'd need to
substitute a different reg), so signed semantics survive AND reversal is
inhibited. That's the precise mechanism the pin gates — and exactly what
makes it cheat-asm we cannot commit.

**Variants tested this session (all rejected, see `meta.json` rejected_forms):**

| Form | Score | Verdict |
|------|-------|---------|
| `s32 var_a1` (baseline, no pin) | 7 | SR reverses to count-down |
| `u32 var_a1` (candidate) | **1** | sltiu vs slti — the gap |
| `(s32)var_a1 < 0x14` cast | 7 | cast forces signed compare → SR fires |
| `signed char` / `short` | 5 | sign-extension overhead |
| `char` (unsigned) | 2 | adds `andi 0xFF` mask |
| `register s32` hint | 7 | hint ignored (without `asm("$N")`) |
| `s32 limit = 0x14;` local | 2 | opaque var — cheat-by-spelling |
| `s32 one = 1; var_a1 += one;` | 2 | opaque var, sanction is for shift loops only |
| `var_a1 * 0` in store address | 7 | GCC simplifies, no use of var_a1 |
| `*((s32*)(arg0+0x1994))[i]=...` (drop arg0+=4) | n/a | breaks target structure (no `addiu a0,a0,4`) |

Declaration ORDER also matters: `u32 var_a1` declared first → score 1; second
→ score 6 (registers swap). The candidate uses `var_a1` first.

## What the next session should try

Listed in `meta.json` `next_hypotheses` — summary:

1. **Instrumented cc1** (`BB2_LOOP_DEBUG` if added) — read loop.c more deeply
   to find a fourth path between (a) keeping signed and (b) preventing
   reversal. The candidate next_hypothesis names the specific lines
   (check_dbra_loop conditions at loop.c:5826–5849) and the unexplored
   surface: a way to make `no_use_except_counting` evaluate to 0 (var_a1
   used in body) without adding insns. This is the precise lever-finding
   question.
2. **SOTN / Vagrant Story cross-reference** — search the community master
   trees for count-up signed loops with `slti+bnez` that survive without
   pins. The C structure that ships in the community standard IS the lever
   if such a sibling exists.
3. **Pointer-walk restructure** — replace the counter+pointer-pair with a
   single-biv pointer loop (`while (dst < end)`). May avoid check_dbra_loop
   entirely. Cost: 1–2 prologue insns to compute `end`. Worth measuring.
4. **Sibling check in text1a.c** — line 340's `do { ... off += 0xA; } while
   (off < 0xD2);` is structurally similar (count-up, constant bound, single
   memory store cluster). Disassemble its compiled form and check whether it
   emits `slti` or has been reversed. If it survives as `slti`, the
   structural difference between the two is the lever to copy.
5. **Directed permuter (PERM_GENERAL)** — last resort; the space is small
   and the diff is 1-bit, so likelihood is low but it may surface a hidden
   structural variant.

## Cheat-reviewer verdict

`PASS` (recorded 2026-06-12 in `meta.json.reviewer`). Caveat: the reviewer
explicitly noted that the target's `slti` opcode evidences the **original
source used a signed counter**, so `u32` moves AWAY from the target's
semantic intent. The candidate is a valid floor-lowering WIP step, NOT a
viable closing form. The next session must find a different structural
approach to recover signed-compare semantics without a pin.

## Mechanism reference (GCC 2.7.2 `tools/gcc-2.7.2/loop.c`)

The loop-reversal path is `check_dbra_loop` (lines 5654–5934). The trigger
conditions for the `LT → count-down` rewrite (lines 5846–5852):

```c
if (comparison && bl->initial_value == const0_rtx
    && GET_CODE (XEXP (comparison, 1)) == CONST_INT
    && GET_CODE (comparison) == LT          /* <-- LT only, not LTU */
    && (INTVAL (XEXP (comparison, 1)) % INTVAL (bl->biv->add_val)) == 0)
```

Four conditions:
1. Initial value of the biv is the constant 0
2. Comparison RHS is a constant integer
3. **Comparison code is `LT` (signed less-than)** — NOT `LTU` (unsigned)
4. Final value is divisible by the increment

`u32 var_a1 < 0x14` produces an `LTU` comparison → condition 3 fails → no
reversal. `s32 var_a1 < 0x14` produces an `LT` comparison → all four
conditions hold → reversal fires.

There is no other check_dbra_loop trigger path that produces a count-down
rewrite — but other `strength_reduce` paths (final_giv_value substitution,
etc.) may also affect this loop. The next session's first action should be
reading those paths in detail.

## Related

- [[inline-asm-policy]] — pins are cheat-asm; the bar this candidate is
  trying to clear
- [[register-asm-pins]] — pin mechanics and why they're forbidden
- [[register-alloc-pure-c]] — Lever B precedent (narrow integer type)
- [[difficult-is-not-impossible]] — keep grinding; the matching C exists
- [[no-compiler-divergence]] — compiler is frozen; the lever is C structure
- [[no-new-park-categories]] — cheats-by-spelling; the standard the
  reviewer applied
