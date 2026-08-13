# EVIDENCE — func_80078654 (src/text1b_b.c)

## Floor history
| session | modality | floor | note |
|---|---|---|---|
| (inherited) | — | 23 | body carried an INLINE_MOVE_ALIASING cheat; 115 insns vs target 116 |
| s1 | recon | **19** | cheat removed + short-typed clamp; insn count now exactly 116 |

`canonical func_80078654` → verdict **C**, 0 asm insns, 116 target insns.
`diagnose` → LARGE (its d28 label is stale/unmasked; the honest sandbox
`--disable all` distance is what matters).

## Rules currently carried (regfix.txt lines 2550-2566, 6 rules)
    func_80078654: $16 <-> $17
    func_80078654: subst "sw\s+\$17,72"  "sw  $17,76"  @ 1
    func_80078654: subst "sw\s+\$16,76"  "sw  $16,72"  @ 9
    func_80078654: subst "lw\s+\$16,76"  "lw  $16,72"  @ 109
    func_80078654: subst "lw\s+\$17,72"  "lw  $17,76"  @ 110
    func_80078654: reorder 110,109 @ 109-110
All six exist to paper over ONE thing: the $s0/$s1 allocation inversion
described below. There are no asmfix.txt entries for this function.

## What the function is
A per-frame draw routine. `arg0` is an 8-word scratch block built by the
caller `func_800788B0` (`s32 buf[8]`, fields 0,2..7 filled). It builds a
`S78654` sprite/primitive descriptor on the stack (frame 0x58 = 88 bytes,
matched exactly), optionally fades it in when the global frame counter
`D_800A3608` is past 0xB04, submits it via
`func_8007352C` / `func_8006E480` / `SetDrawMode` / `AddPrim`, then walks a
`-1`-terminated table at `D_800A3610 + 5` doing the same for each entry.

## THE ONE REMAINING DIFF (the whole 19-point residual)
A single two-way callee-save register inversion, repeated at every use site:

| role | target | our build |
|---|---|---|
| `arg0` (the s32* parameter) | **$s1** (saved at 0x4C) | $s0 (saved at 0x48) |
| `var_s0` (the table walk pointer) | **$s0** (saved at 0x48) | $s1 (saved at 0x4C) |
| `zero` (the s32 0 held across calls) | $s2 | $s2 (matches) |

Nothing else differs. Instruction count, instruction selection, scheduling,
delay-slot fills, the frame size, the `mult`-by-0x88888889 divide-by-15
sequence, and the three-way `sb` colour splat are all identical. Verified
with `tmp/grind/func_80078654/s1/diffasm.py`, which disassembles the sandbox
`.o` and the target bytes through the SAME objdump and normalizes relocs and
branch targets, so only real codegen deltas print.

## THE MECHANISM, MEASURED (this is the load-bearing finding)
The inversion is decided in `global_alloc`. GCC 2.7.2's `allocno_compare`
(tools/gcc-2.7.2/global.c) sorts allocnos by

    pri = floor_log2(n_refs) * n_refs / live_length * 10000 * size

and `find_reg` then hands out callee-saves in ascending hard-reg order, so
**the higher-priority allocno gets $s0**. Instrumented cc1
(`tools/gcc-2.7.2/cc1`, NOT `build/cc1`) with `BB2_ALLOC_DEBUG=1` prints the
actual numbers for our current best form:

    ord=0 pseudo=81 hardreg=4  nrefs=4  livelen=9   pri=8888
    ord=1 pseudo=85 hardreg=3  nrefs=3  livelen=5   pri=6000
    ord=2 pseudo=91 hardreg=64 nrefs=2  livelen=4   pri=5000
    ord=3 pseudo=72 hardreg=16 nrefs=13 livelen=98  pri=3979   <- arg0    -> $s0
    ord=4 pseudo=73 hardreg=17 nrefs=5  livelen=91  pri=1098   <- var_s0  -> $s1
    ord=5 pseudo=74 hardreg=18 nrefs=3  livelen=170 pri=176    <- zero    -> $s2

Target requires pseudo 73 to sort BEFORE pseudo 72. The gap is 3.6x. Solving
`allocno_compare` for what would flip it, holding the other side fixed:

  * raise var_s0: need `floor_log2(n)*n > 36.2` at livelen 91, i.e.
    **n_refs >= 13** (it has 5, and only 5 memory references exist in the
    target bytes for that pointer).
  * lower arg0: need `floor_log2(n)*n < 10.8` at livelen 98, i.e.
    **n_refs <= 5** (it has 13, and the target bytes contain all 13:
    one def plus `12(sN)` x2 and `20(sN)` x4 in each of the two blocks).

Both bounds are outside what the target's own emitted instructions allow —
the ref counts are pinned by the bytes we are trying to reproduce. The
`.greg` conflict sets are symmetric (72 and 73 conflict with an identical
set), so there is no conflict-driven route either, and neither pseudo has a
usable copy preference (arg0's only preference is hard reg $a0, which is
excluded because the pseudo crosses calls).

**Therefore: no statement-reordering / declaration-order / live-range
shortening variant of the CURRENT dataflow can produce the target's
allocation.** The original C must have had a materially different dataflow
shape for one of these two values. Finding that shape is the job.

## Artifacts (session 1)
* `tmp/grind/func_80078654/s1/diffasm.py` — the normalized target-vs-build
  disassembly differ. Re-run after every edit; it is the real gradient.
* `tmp/grind/func_80078654/s1/greg.sh` — regenerates the full cc1 `-da` RTL
  dump set into `tmp/grind/func_80078654/s1/da/` with the exact build flags
  (`-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls
  -fno-builtin -w -mel`; text1b_b is NOT in GP_FILES or NO_SR_FILES).
* `tmp/grind/func_80078654/s1/da/text1b_b.i.greg` — the register dispositions
  and conflict sets quoted above.
* For the priority numbers use the INSTRUMENTED cc1 at
  `tools/gcc-2.7.2/cc1` with `BB2_ALLOC_DEBUG=1` (build/cc1 is not
  instrumented). Other hooks present in that binary: BB2_FINDREG_DEBUG,
  BB2_FLOW_DEBUG, BB2_SUGG_DEBUG, BB2_QTY_DEBUG, BB2_FRAME_DEBUG,
  BB2_XJUMP_DEBUG, BB2_SCHED_DEBUG, BB2_PRIO_DEBUG, BB2_RANK_DEBUG,
  BB2_RELOAD_DEBUG, BB2_DBR_DEBUG.

- [s1] canonical func_80078654 -> verdict C, 0 asm insns, 116 target insns, pure-C distance 23 at session start. No asmfix.txt entries; six regfix.txt rules at lines 2550-2566 ($16 <-> $17 swap, four sw/lw offset substs at maspsx idx 1/9/109/110, and a reorder 110,109) which ALL exist to paper over the single $s0/$s1 inversion.

- [s1] The inherited src body carried a forbidden INLINE_MOVE_ALIASING construct: __asm__ volatile("move %0, %1" : "=r"(v_copy) : "r"(v)). Removing it did not cost anything — it IMPROVED the honest floor 23 -> 19 and fixed a one-instruction shortfall (115 -> 116). The cheat was worse than the honest form, not merely score-inert.

- [s1] After the session's edit the ONLY remaining difference between our build and the target is a two-way callee-save inversion repeated at every use site: target holds the s32* parameter arg0 in $s1 (saved 0x4C) and the table-walk pointer in $s0 (saved 0x48); our build holds them the other way round. The `zero` value is in $s2 on both sides. Instruction selection, count, scheduling, delay-slot fills, frame size (0x58), the mult-by-0x88888889 divide-by-15 sequence and the three-byte colour splat all match.

- [s1] MEASURED MECHANISM (instrumented cc1 at tools/gcc-2.7.2/cc1 with BB2_ALLOC_DEBUG=1 — note build/cc1 is NOT instrumented): ord=3 pseudo=72 hardreg=16 nrefs=13 livelen=98 pri=3979 (arg0 -> $s0); ord=4 pseudo=73 hardreg=17 nrefs=5 livelen=91 pri=1098 (walk pointer -> $s1); ord=5 pseudo=74 hardreg=18 nrefs=3 livelen=170 pri=176 (zero -> $s2). The target requires pseudo 73 to sort before pseudo 72.

- [s1] Solving allocno_compare for the flip, holding each side fixed: the walk pointer needs floor_log2(n)*n > 36.2 at live length 91, i.e. n_refs >= 13 (it has 5); or the parameter needs floor_log2(n)*n < 10.8 at live length 98, i.e. n_refs <= 5 (it has 13). Both counts are pinned by the target's own emitted memory accesses — the target bytes literally contain 13 references through the parameter and 5 through the walk pointer. Therefore NO statement-reordering, declaration-order, or live-range-shortening variant of the current dataflow can produce the target's allocation; the original C's dataflow partition must have differed.

- [s1] The .greg conflict sets for pseudos 72 and 73 are identical (both conflict with 72 73 74 81 85 91 2 3 4 5 6 7 29 65 66), so there is no conflict-driven route to the flip, and neither pseudo carries a usable copy preference — arg0's only preference is hard reg $a0, excluded because the pseudo crosses calls.

- [s1] PROCESS NOTE: src/text1b_b.c was reverted to HEAD before the session ended, because with the improved C the six existing regfix rules no longer align to their maspsx indices and a full build would not be SHA1-clean. The improved form is carried in memory/grind/func_80078654/candidate.c and MUST be re-applied at the start of the next session before any measurement — the sandbox will otherwise report the stale floor of 23.
