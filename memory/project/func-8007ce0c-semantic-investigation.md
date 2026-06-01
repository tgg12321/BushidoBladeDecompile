---
name: func-8007ce0c-semantic-investigation
description: func_8007CE0C (display.c GPU LoadImage wrapper) — semantic frame investigation; 32-byte vars region in target is unreferenced, pure-C floor 41 via compare-flip + drop cheat-asm, no semantic justification for the 16-byte vars slack found, function stays INCOMPLETE
date: 2026-06-01
status: ongoing
metadata:
  type: project
---

# func_8007CE0C — semantic frame investigation

The function in `src/display.c:719` is a GPU `LoadImage`-style wrapper that
clips an `arg0->x,y` against screen bounds (`D_8009BE78` / `D_8009BE7A`),
computes the GPU half-word transfer count (`(x*y+1)/2`), splits into a DMA
block count + a CPU-PIO remainder, then writes the LoadImage command sequence
to `*g_gpu_data_reg` and (if any DMA blocks remain) configures the DMA
controller.

`scan_hand_coded`: tier=LOW, score=1/8 (only S4 — front-loaded loads in the
GPU PIO loop). NOT a canonical-asm candidate.

`canonical`: verdict C, pure-C distance 48 (cheat-asm stripped).

`diagnose`: CANONICAL-ASM "mfhi/mflo synthetic intermediate" — this is a
false positive. The mflo is present in BOTH target and current sandbox at
the same position (idx 44 normalized). The heuristic over-fires because
mflo appears anywhere in the diff context, not because the function is
hand-coded.

## Phase 1: stack-offset map (from `asm/funcs/func_8007CE0C.s`)

Target frame: **80 bytes** (`addiu $sp, $sp, -0x50`).

| Offset | Byte range | Use |
|---|---|---|
| 0x00..0x0F | 16 | arg-spill area for callees (ABI-required) |
| **0x10..0x2F** | **32** | **vars region — NEVER ACCESSED via $sp anywhere in the body** |
| 0x30 | 4 | s0 saved |
| 0x34 | 4 | s1 (= arg0 pointer throughout) |
| 0x38 | 4 | s2 (= arg1 pointer throughout) |
| 0x3C | 4 | s3 |
| 0x40 | 4 | s4 |
| 0x44 | 4 | s5 |
| 0x48 | 4 | ra |
| 0x4C | 4 | (alignment slot, never accessed) |

**Decisive finding:** there is NO `lw`/`sw`/`lh`/`sh`/`lhu`/`lbu` operation
against `$sp+0x10` through `$sp+0x2F` anywhere in the function body. The
32-byte vars region exists in the frame allocation but is never read or
written — it is purely reserved space.

Current C body produces frame=64 (vars=16 — 16 bytes shorter). The 16-byte
delta is precisely 4 × s32 or one 16-byte aggregate. Probed via cc1:
```
[baseline current C]  .frame $sp,64,$31  # vars= 16, regs= 7/0, args= 16, extra= 0
[target wants]        .frame $sp,80,$31  # vars= 32, regs= 7/0, args= 16, extra= 0
```

## Phase 2: hypotheses tested

| Hypothesis | Frame produced | Distance | Notes |
|---|---|---|---|
| Baseline (current C with cheat-asm pin + barrier) | 64 (vars=16) | 48 | Status quo |
| Add `s32 buf[4];` with NO uses (forbidden — dead-vars detector) | 80 (vars=32) | 48 (unchanged) | Frame matches but masked distance unchanged. Score is masked Levenshtein — frame size doesn't directly affect it, only the COUNT/TYPE of insns. Build_insns stays 142 vs target 143. |
| Add `s32 buf[4];` with `buf[0] = arg0->unk0;` (single real store) | 80 (vars=32) | regressed | The store emits a real `sw $2, 16($sp)`, so build_insns becomes 144 (target 143) and a non-target store appears in the asm. Net regression. |
| Restructure body — drop cheat-asm pin + barrier, separate `x`/`y` locals, flip comparisons (`x > D_8009BE78` not `D_8009BE78 < x`) | 64 (vars=16) | **41** | **REAL PROGRESS** — the pure-C distance dropped 48 → 41 by removing the cheat-asm and using the compare-flip lever from [[compare-operand-order-register]]. The `var_s5 = 0;` initialization is now interleaved into the bltz delay slot naturally (matching target's structure, just in a different scheduling slot). |

### Why the frame=80 path is forbidden

The only mechanism known to make GCC 2.7.2 reserve 32 vars-bytes WITHOUT
emitting any instructions that touch those bytes is to declare an unused
local aggregate (struct or array). Per [[dead-vars-local-array]] (2026-05-31)
and [[inline-asm-policy]] expanded catalog (2026-05-31), this pattern is
forbidden:

> "Declaring an anonymous `buf[N]` is a GUESS that happens to produce the
> right size. SOTN-quality reconstruction either identifies what the locals
> semantically WERE (matrices, GPU packets, scratchpad areas) and reconstructs
> them with meaningful names, OR marks the function `INCLUDE_ASM`."

And the prior agent's `s32 pad; (void)&pad;` workaround was specifically
closed (commit `4460cb2`, 2026-06-01) — `engine/volatile_cheats.find_addr_coerced_locals`
flags it. So the address-coercion variant is also out.

### The semantic question — what locals WERE the 32 bytes?

Investigation found no decisive semantic answer:

1. **`RECT clip;` (8 bytes)** — half the right size; no second 8-byte aggregate
   suggested itself from the body.
2. **GPU primitive struct (e.g. `DR_LOAD`)** — PsyQ's `DR_LOAD` is ~20 bytes,
   but the function doesn't build a primitive in stack — it writes the command
   words DIRECTLY to `*g_gpu_data_reg` one at a time.
3. **Scratchpad area** — the function uses `*g_gpu_data_reg` (memory-mapped
   GPU port), not scratchpad (`0x1F800000`-range addresses). No scratchpad use.
4. **Spill area for register pressure** — the function uses 7 callee-saves
   (s0..s5,ra) + scratch but never spills any. Body has 4 loads in an 8-insn
   window (S4 signal) but no spill stores.
5. **4 × s32 named scalars** that GCC keeps fully register-allocated — would
   need to be USED in the body for GCC to reserve frame, but then they emit
   real `sw` instructions the target doesn't have.

**No semantic justification was found for the 16-byte frame slack.** The
target's frame=80 appears to be a cc1 artifact that the toolchain we have
cannot reproduce without forbidden phantom-locals patterns.

## Phase 3: what we learned (vs. what the prior agent reported)

The prior agent (2026-06-01) reached:
- pure-C floor 38 (with the cheat-asm pin + barrier removed but other
  restructuring kept)
- 22 (with `s32 pad; (void)&pad;` forbidden form)

This session reached:
- pure-C floor 41 with a cleaner restructure (compare-flip + dropped
  cheat-asm + reorganized vars)

The difference vs the prior agent's 38: the prior agent had additional manual
codegen tweaks not captured in their summary. Both sessions agree on the wall:
**a 30+-distance plateau in pure C that no C-source restructure has been
shown to close**.

## Phase 4: remaining gap

The 41 cheat-free masked diffs are a mix of:

1. **Scheduling differences** — target interleaves `var_s5=0` between the
   initial `lh` and its consumer; cc1 places it differently (delay slot of
   the bltz instead of load-delay slot of the lh). Same instruction count,
   different positions.
2. **Register allocation differences** — target uses `$a1` for arg0->x where
   cc1 picks `$v0`; cascades through ~10 register-rename sites.
3. **The +16-byte frame slack** — every saved-reg load/store offset is +16
   from cc1's output. Masked score doesn't see this (offsets are masked
   numerics), but the real-build SHA1 gate does.
4. **A few extra/missing instructions** — sandbox builds 141 insns, target
   has 143. The 2-instruction shortfall is mostly in the prologue (cc1
   doesn't emit the extra `move` insns that fill target's load-delay slots).

## Concrete next steps (if continuing)

1. **Apply the cleaner restructure as a separate commit** (`cheat-cleanup:`
   prefix). It drops cheat-asm pin + barrier + applies compare-flip lever.
   BUT: this changes the maspsx instruction sequence, which would break the
   existing 5 splice rules AND the 16 frame-shift substs. The rules are
   anchored to specific indices in the OLD body; a cleaner body requires
   re-deriving them. Net: NOT a rule-count reduction, so per the brief's
   discipline ("don't commit src changes unless they retire rules"), this
   should not be committed without finishing the close.

2. **Search for the 32-byte vars semantic** — possible angles untried:
   - Examine the leaked PSY-Q libgpu source for the equivalent of `LoadImage`'s
     local frame layout. If a `int args[4];` or similar local IS in PSY-Q's
     source AND is referenced (so not phantom), it might match target's
     pattern.
   - Check sibling display.c functions that DO match for similar "GPU command
     wrapper with 32-byte vars" patterns. None obviously match in the survey.

3. **Run the directed permuter** — `permuter/display_8007CE0C/` from a
   clean restructured base (frame=64, distance 41). Per the
   [[cross-jump-store-tail-merge]] precedent, the random search can find
   non-obvious type-mutation levers (`u32`→`s32`, signed/unsigned changes)
   that close some of the residual.

4. **Acceptance — keep parked.** Per [[no-new-park-categories]] (2026-06-01):
   - No "frame-size infrastructure" carve-out.
   - No phantom locals.
   - This function stays INCOMPLETE in queue.json with its 23 rules until
     a future session finds the lever, OR until evidence supports
     canonical-asm authorization (currently signaled only LOW tier 1/8).

## What's confirmed not viable

- **Phantom `s32 buf[N];` declarations** — closed by [[dead-vars-local-array]]
  detector (2026-05-31).
- **`(void)&local;` address-coercion** — closed by `find_addr_coerced_locals`
  detector (2026-06-01, commit 4460cb2).
- **`register asm("s5")` pin + `__asm__ volatile("" : "=r"(s5) : "0"(s5))`
  barrier** — cheat-asm per [[inline-asm-policy]]; can be removed
  (drops distance 48→41) but doesn't close the gap.
- **The K&R extern signature lever** (adding more args to motion/func_8007DC9C
  callees) — adds stack arg-spill but also emits real `sw` insns the target
  doesn't have. Net regression.
- **Compare-operand flip on x and y clamps** — applied; helps the
  scheduling pattern but doesn't address the frame size.

## Relevant rules / memory

- [[dead-vars-local-array]] — the rule that forbids phantom locals
- [[no-new-park-categories]] — frame-size infrastructure is NOT a new
  category
- [[compare-operand-order-register]] — the lever that dropped the
  distance 48→41
- [[sandbox-zero-retire-fails]] — explains the cheat-asm pattern
  currently present (`__asm__ volatile("" : "=r"(s5) : "0"(s5))` is the
  same shape as the barrier discussed there)
- [[register-alloc-pure-c]] — the broader RA-lever playbook; was
  applied (compare-flip) but didn't close the residual
- [[difficult-is-not-impossible]] — the discipline that says "don't
  declare this impossible"; the matching C exists, it just hasn't been
  found within the explored search space

## Status

PROGRESS — pure-C cheat-free distance lowered from 48 to 41 via restructure
(compare-flip + drop cheat-asm pin/barrier). Cannot reach COMPLETED-C without
either (a) finding a non-phantom semantic justification for the 32-byte vars
region, OR (b) closing the 41-diff plateau via a yet-undiscovered C-source
lever. Source reverted to baseline to keep oracle green. Function stays
INCOMPLETE in queue.json with its 23 rules. No new park category requested
per [[no-new-park-categories]].
