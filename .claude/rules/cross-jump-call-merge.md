---
name: cross-jump-call-merge
paths: [".claude/rules/cross-jump-call-merge.md"]
# on-demand only: surfaced via codegen-technique-index (auto-loads on src/*.c)
description: "target.s has more jalr call-sites than your build: GCC cross-jumping merged calls with equal CALL_INSN_FUNCTION_USAGE. FIX: give each fn-ptr its REAL arg COUNT (count differentiates the calls; arg modes do NOT)."
metadata:
  type: reference
---

# The cross-jump call-merge wall — SOLVED (arg count is the lever)

## Symptom

A dispatch/handler function (status-byte interpreter, vtable-style call, multi-case
switch each calling a function pointer) where your pure-C body is structurally and
semantically correct but plateaus at a high diff. The tell: **target.s has more
`jalr` (or `jal`) call sites than your build.** Handler arms that the original kept
as distinct call sites collapsed into ONE in your output.

Diagnostic (decisive, ~2s):

```
mipsel-linux-gnu-objdump -d <yourbuild>.o      | grep -wc jalr
mipsel-linux-gnu-objdump -d permuter/<func>/target.o | grep -wc jalr
```

If build < target, you are on this wall. Register/scheduling substs will NOT close
it — the gap is call-site count.

## Why (validated against the GCC 2.7.2 source)

`toplev.c:3142` runs `jump_optimize(insns, 1, 1, 0)` (cross_jump=1) at any `-O>0` —
**no disable flag** (that is GCC 3.x). `find_cross_jump` (jump.c:2371) merges two
blocks that jump to the same label when they share an instruction **suffix** of
length >= `minimum` (2). For two `jalr` (CALL_INSN) to count as matching, jump.c:2426
requires their **`CALL_INSN_FUNCTION_USAGE` to be `rtx_equal`** — the explicit comment
says this guards delay-slot filling from clobbering a parameter. FUNCTION_USAGE is the
list of `(use (reg argN))` / `(clobber ...)` for the call.

**The decisive fact (isolation-tested):**

| two fn-ptr calls converging to one return | jalr |
|---|---|
| `(s16,s16,u8,u8)` vs `(s32,s32,u8,void*)` — different **modes** | 1 (MERGED) |
| `(s16,s16,s16,s16)` vs `(s32,s32,s32,s32)` — all different modes | 1 (MERGED) |
| `(s16,s16)` 2-arg vs `(s32,s32,u8,void*)` 4-arg — different **count** | **2 (SPLIT)** |

**Argument MODES are promoted to a uniform register mode → they do NOT differentiate
the calls.** (This is why "just give them distinct signatures" fails.) **Argument
COUNT genuinely changes FUNCTION_USAGE (different set of arg registers used) → it
blocks the merge.** Calls that jump vs fall-through to the epilogue also stay split
(different block ending, not a shared suffix).

## The fix (recipe)

1. Run the jalr diagnostic. If build < target, do NOT chase register diffs.
2. For each handler call, read the target and count **how many argument registers
   (a0-a3) are actually set/live into that call.** That is the function's real arg
   count. m2c routinely OVER-infers a trailing `void*`/extra arg — trust the asm, not
   m2c's signature.
3. Declare each fn-ptr with its real arg count and call it with exactly that many
   args. Calls with different counts will not merge; calls with the same count + same
   epilogue path will (which is correct when the original merged them).
4. Verify with the **jalr count**, not the diff. Once it matches, the residual is
   ordinary ordering/scheduling/register work (the diff drops to a normal cascade).

## Confirmed result — saTan0Main (main.c, MIDI sequence interpreter)

Target = 3 jalr; every uniform-4-arg attempt = 1 jalr (cross-jump merged all 5
handlers). Reading real arg counts from target.s:

| status | handler | real arg count |
|---|---|---|
| 0x90 | D_800F3340 | **4** (a0,a1,b,next) |
| 0xE0 | D_800F3348 | **2** (a0,a1) |
| B0/C0/F0/FF | D_800F3344/334C/3350 | **3** (a0,a1,data) — merge into one (correct) |

Declaring those counts → **3 jalr, matching target** (alignment-immune edit distance
collapsed from a structural wall to ~70 ordering/scheduling edits). decomp-permuter
had failed 67k+ attempts from the 1-jalr seed because no local mutation changes a
call's arg count — it is a signature/declaration change, not a body mutation.

## What does NOT defeat the merge

- Distinct arg **modes/types** (promoted — see table above).
- Any `-O1/-O2/-O3` + scheduling/CSE/thread-jump/delayed-branch flag (see
  [[compiler-flags-canonical]]); only `-O0` disables cross-jump and it over-splits.
- m2c's faithful reconstruction, cc1psx on the same C, register pins, the permuter
  from a 1-jalr seed — all 1 jalr.

## Apply (manual pipeline)

This is a DECLARATION-surface fix; neither the body permuter nor regfix can find it
(they mutate the body, not the signatures). The retired `dc.sh sig-reconcile` /
`sig-search` tools that automated this are gone as of 2026-07 (dc.sh workflow
retirement). Work it manually:

1. **DETECT.** Compare build vs target `jalr` counts in the affected function.
   `objdump -d build/src/<file>.o` for build; `asm/funcs/<func>.s` for target.
   If build < target, you are on this wall — GCC's `jump2` cross-merged
   identical CALL suffixes in your build that target compiled distinctly.
2. **RESOLVE.** For each fn-ptr handler called by the function, read the
   target's per-call arg setup (which of `$a0..$a3` are live into each call).
   The arg count comes from THE TARGET — not from m2c (which over-infers
   trailing args) and not from blind diff-minimising. Update the C-side
   handler declarations so build jalr == target jalr.
3. Close the residual ordering/scheduling with the body permuter / manual work.

The arg count comes from the TARGET (which a0-a3 are live into each call), never from
m2c (which over-infers trailing args) and never from blind diff-minimising.

## Related
- [[shared-end-label]] — a different convergence-merge (per-case return folding)
- [[store-before-jal]] — delay-slot scheduling from C structure
- [[compiler-flags-canonical]] — flags are not the lever; arg count is
- [[minimize-regfix]] — this closes in pure C, no regfix needed for the structure
