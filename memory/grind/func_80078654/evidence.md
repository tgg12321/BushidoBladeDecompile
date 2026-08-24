> **2026-08-24 MIGRATION NOTE:** HEAD is now `INCLUDE_ASM` (migrated in
> a7892ba2 (2026-08-24 sweep 2)); rules retired, in-source cheat-asm removed. "HEAD"
> claims below describe the pre-migration tree (`retired-chassis-2026-08/body.c`).

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

## SESSION 2 (structural) — the global.c model is now CLOSED, and quantified

**Floor unchanged at 19.** candidate.c was re-applied to src/text1b_b.c at the
start of the session (sandbox `--disable all` = 19, build_insns 116 == target
116) and src carries that form at session end.

### The whole of find_reg is now read, not assumed (tools/gcc-2.7.2/global.c)
The s1 model ("higher allocno priority takes $s0, callee-saves handed out in
ascending order") is correct, but it was only half the machine. The complete
decision procedure for our two pointers is:

1. `find_reg` runs TWO passes (global.c:993-1084). Pass 0's candidate set is
   `regs_used_so_far` MINUS `regs_someone_prefers[allocno]` minus conflicts —
   i.e. *only registers that are already in use*, so that no new callee-save
   save/restore is created. Pass 1 (reached only if pass 0 finds nothing) is a
   pure first-fit walk of `reg_alloc_order`, which on MIPS lists $s0..$s7
   ascending.
2. `regs_used_so_far` is seeded (global.c:344-372) from `regs_ever_live` +
   `call_used_regs` + every hard reg already handed out by LOCAL alloc.
   **MEASURED for this function** (`ALLOCDBG seed_used=`):
   `0..15,24..29,31,32..51,64..67` — i.e. every call-clobbered reg and no
   callee-save at all. So pass 0 can never award $s0/$s1 here, and both
   pointers are decided by pass 1's ascending first-fit. There is no
   cost-based choice between two free callee-saves anywhere in find_reg.
3. The copy-preference override (global.c:1097-1166) could move an allocno off
   the first-fit answer, but `prune_preferences` (global.c:899-909) strips
   every call-clobbered reg from the preferences of a call-crossing allocno, so
   arg0's only preference ($a0, from `addu $s1,$a0,$zero`) is removed, and pure
   C has no way to make a *callee-save* a copy preference (that would require a
   register-asm pin — a forbidden construct).
4. The local-alloc eviction path (global.c:1198-1253) only fires when
   `best_reg < 0`, which never happens here.

**Consequence — the "share $s0 with an earlier disjoint allocno" route is dead
too.** In principle the target's allocation could be produced without a
priority flip: a higher-priority allocno X taking $s0 while conflicting with
arg0 (pushing arg0 to $s1) and NOT conflicting with the walk pointer (letting
the walk pointer re-use $s0). That is impossible for this function: the walk
pointer is live across the ENTIRE body — its def must precede block A, because
`D_800A3610` is a global pointer and block A contains four calls, so any
`var_s0 = D_800A3610 + 5` placed after block A forces a SECOND
`lw %gp_rel(D_800A3610)` (the target has exactly one, at insn 5). A pointer
live across the whole body conflicts with every other allocno, so no X can be
disjoint from it.

### live_length provenance, measured (this closes the last modelling gap)
`zero` showing livelen=170 in a ~116-insn function looked like an accumulation
quirk that might make live lengths inflatable by CFG shape. It is not:

* flow.c increments `reg_live_length` exactly once per insn the reg is live
  (flow.c:1685 + :2087). **MEASURED with BB2_FLOW_DEBUG**: pseudo 72 → 104
  increments over 104 DISTINCT insns; pseudo 74 → 103 over 103 distinct insns;
  **zero repeats for either**, so life analysis' fixpoint iteration does NOT
  double-count and CFG shape cannot inflate a live length.
* `sched.c:5106` then OVERWRITES `reg_live_length` with its own
  post-scheduling recount (that is why the allocno values 98 / 91 / 85 differ
  from the flow counts).
* `local-alloc.c:1058-1064` then **DOUBLES** `reg_live_length` for any pseudo
  carrying a `REG_EQUIV` note. That — not a quirk — is the whole explanation of
  `zero`'s 170 (= 2 x 85) and of why it sorts last and lands in $s2.

So a REG_EQUIV note is a genuine 2x priority DEMOTION lever. It is
**unreachable for arg0**: local-alloc only auto-attaches the note to a pseudo
whose single set has a MEM source AND which is local to one basic block
(`reg_basic_block[regno] >= 0`, local-alloc.c:1051-1055), while arg0 is a
register-passed parameter (`addu $s1,$a0,$zero`, a REG source) spanning every
block. GCC only gives a register-passed scalar parm a stack-home REG_EQUIV when
it is addressable / homed, which would change the emitted bytes.

### The residual, now stated as a hard numeric bound
With arg0 pinned at 13 refs / live length 98 (pri 3979) by the target's own
emitted accesses, `allocno_compare` requires the walk pointer to reach
**>= 14 references** at length ~98 to take $s0 (13 refs gives 3900 < 3979).
It has 5. Equivalently arg0 must drop to <= 5 refs. Splitting the 12 buffer
accesses across two disjoint pseudos does not help: a block-A-only holder
(7 refs / ~30 length) scores ~4666 and takes $s0 itself.

### Sibling ground truth (F2, closed)
Census over all 1437 `asm/funcs/*.s` (tmp/grind/func_80078654/s2/census.py):
190 target functions copy the incoming `$a0` into `$s1` or higher. The nearest
COMPLETED-C sibling is **func_80078824** (src/text1b_b.c:1059, same file, same
draw cluster): it holds its parameter in $s1 and the derived pointer
`s0 = arg0 + 0x58` in $s0. Its C shows WHY — the parameter is referenced only
3 times (`arg0 + 0x58`, `D_800A360C = arg0`) while the derived pointer is
referenced 5 times over a shorter range, so the derived value wins the priority
sort. The sibling therefore CONFIRMS the s1 mechanism and shows the target's
shape requires a reference-POOR parameter; it does not supply a new dataflow.
F2 is closed: no sibling exhibits a reference-RICH parameter in $s1.

- [s1] PROCESS NOTE: src/text1b_b.c was reverted to HEAD before the session ended, because with the improved C the six existing regfix rules no longer align to their maspsx indices and a full build would not be SHA1-clean. The improved form is carried in memory/grind/func_80078654/candidate.c and MUST be re-applied at the start of the next session before any measurement — the sandbox will otherwise report the stale floor of 23.

- [s2] candidate.c re-applied to src/text1b_b.c reproduces the s1 floor exactly: sandbox --disable all score 19, build_insns 116 == target_insns 116. src carries that form at session end (git diff: src/text1b_b.c only).

- [s2] MEASURED pass-0 seed for func_80078654 (ALLOCDBG seed_used): 0..15,24..29,31,32..51,64..67 — no callee-save register is in regs_used_so_far, so find_reg's pass 0 cannot award $s0 or $s1 and both pointers fall through to pass 1's ascending first-fit over reg_alloc_order.

- [s2] find_reg contains no cost-based comparison between two FREE callee-saves (global.c:1052-1084 is a plain first-fit break); the only override is the copy/full-preference switch at global.c:1097-1166, and prune_preferences (global.c:899-909) removes every call-clobbered reg from a call-crossing allocno's preference sets, which deletes arg0's sole preference ($a0 from addu $s1,$a0,$zero).

- [s2] The walk pointer's def cannot be moved after block A: D_800A3610 is a global pointer and block A contains four calls, so a later `var_s0 = D_800A3610 + 5` forces a second lw %gp_rel(D_800A3610); the target has exactly one such load (insn 5). Therefore the walk pointer is live across the whole body and conflicts with every other allocno — no higher-priority allocno can take $s0 and leave it shareable.

- [s2] reg_live_length is NOT inflated by flow.c's fixpoint iteration: BB2_FLOW_DEBUG shows 104 increments over 104 distinct insns for pseudo 72 and 103 over 103 for pseudo 74, with zero repeats. sched.c:5106 then overwrites reg_live_length with its post-scheduling recount, and local-alloc.c:1058-1064 doubles it for any pseudo with a REG_EQUIV note — that is the entire explanation of `zero`'s livelen 170 (= 2 x 85) and of its last-place sort into $s2.

- [s2] The REG_EQUIV doubling is a real 2x priority-demotion lever for other functions but is unreachable for this parameter: local-alloc only auto-attaches the note when the pseudo's single set has a MEM source AND the pseudo lives in one basic block (local-alloc.c:1051-1055); arg0 is register-passed (REG source) and spans every block.

- [s2] MEASURED V1 (base/walk-pointer merge): pseudo 73 nrefs 8, livelen 98, pri 2448 vs arg0 pri 3979; sandbox 22 (vs 19 for the best form), insn count still 116. The analytic model predicted 2449, validating it to 0.04% — future variants can be scored from the model before spending a build.

- [s2] Numeric bound for the flip: with arg0 pinned at 13 refs / length 98 (pri 3979), the walk pointer needs >= 14 references at length ~98 (13 refs gives 3900 < 3979). Splitting the 12 buffer accesses across two disjoint pseudos does not help — a block-A-only holder (7 refs / ~30 length) scores ~4666 and takes $s0 itself.

- [s2] All 12 buffer references exist at RA time in the target: no 0xC($s1) or 0x14($s1) insn sits in a branch/jal delay slot in asm/funcs/func_80078654.s, so none of them was created after register allocation by reorg's delay-slot duplication.

- [s2] Sibling census (190 hits, tmp/grind/func_80078654/s2/census.py): the COMPLETED-C sibling func_80078824 in the same file has its parameter in $s1 with only 3 references and its derived pointer in $s0 — confirming the mechanism and that the target shape demands a reference-poor parameter.

## SESSION 3 (structural) - the duplication axis (F4) is measured DEAD

**Floor unchanged at 19.** candidate.c was re-applied to src/text1b_b.c at the
start of the session (HEAD still carried the OLD inline-asm body - see the
process note below) and reproduces the ledger numbers exactly: sandbox
`--disable all` = 19, build_insns 116 == target 116, and the instrumented cc1
reprints the s1/s2 allocno table verbatim
(`tmp/grind/func_80078654/s3/base/alloc.log`):

    ord=3 pseudo=72 hardreg=16 nrefs=13 livelen=98  pri=3979   <- arg0   -> $s0
    ord=4 pseudo=73 hardreg=17 nrefs=5  livelen=91  pri=1098   <- var_s0 -> $s1
    ord=5 pseudo=74 hardreg=18 nrefs=3  livelen=170 pri=176    <- zero   -> $s2

### F4 measured on both of the CFG's available duplication shapes

D2 - the whole walk loop duplicated into both arms of the single junction
(`rejected/dup-loop-into-arms-no-crossjump-merge.c`):

| | nrefs | livelen | pri |
|---|---|---|---|
| arg0 (72) | 13 -> **19** | 98 -> 134 | 3979 -> **5671** |
| var_s0 (73) | 5 -> **9** | 91 -> 127 | 1098 -> **2125** |

Sandbox 19 -> **58**, build_insns 116 -> **155**.

D1 - the walk-pointer-ONLY initialiser duplicated into both arms
(`rejected/dup-walk-init-into-arms-second-gp-load.c`):

| | nrefs | livelen | pri |
|---|---|---|---|
| arg0 (72) | 13 | 98 -> 100 | 3979 -> 3900 |
| var_s0 (73) | 5 -> **6** | 91 -> 85 | 1098 -> **1411** |

Sandbox 19 -> **25**, build_insns 116 -> **118** (a second
`lw %gp_rel(D_800A3610)`, appearing as a new pseudo 76 in $v1).

### Why that closes the axis (two independent kills)

1. **jump2's find_cross_jump re-merges NEITHER shape.** The sanctioned
   [[duplicated-statement-into-arms]] family's byte-neutrality prerequisite
   fails at the first measurement: +39 insns for D2, +2 for D1. The family
   works when two arms present a common SUFFIX reaching a common label; this
   function's arms are separated by the whole of block A (D1) or carry their
   own back-edges and exit tests (D2), so no mergeable suffix exists.
2. **The reference arithmetic runs the wrong way.** The loop body - the only
   region containing walk-pointer references - contains SIX arg0 references
   (`arg0[3]` x2, `arg0[5]` x3, plus the store) against FOUR walk references.
   Every duplication therefore grows the parameter's `reg_n_refs` 1.5x faster
   than the walk pointer's, so the priority ratio has an asymptotic FLOOR of
   1.5 and can never reach 1.0. The measured k=2 point (5671 vs 2125, ratio
   2.67) is the best point on the curve: at k=3 arg0 crosses floor_log2's
   16-reference step (4*25/170 = 0.588 vs 3*13/163 = 0.239) and the ratio
   worsens again. Walk-only duplication yields +1 reference per junction and
   the function has exactly ONE junction - the flip needs +9.

### The bound is 2-D, and the 2-D relaxation is unreachable too
s2 stated the bound one-sided (walk >= 14 refs, or arg0 <= 5). The exact
condition is `floor_log2(w)*w/len_w > floor_log2(a)*a/len_a`, and because
floor_log2 steps, LOWERING arg0 helps disproportionately: at a = 8 refs /
len 98 (pri 2448) a walk pointer with just 8 refs / len 91 (pri 2637) already
wins. So the flip does NOT strictly require 14 walk references - it requires
the PAIR to move. That relaxation is nonetheless unreachable: arg0's 13
references are one def plus the target's own twelve `12($sN)` / `20($sN)`
accesses, and the only way to reduce a single pseudo's count is to split the
accesses across two pseudos - which s2 already measured produces a SHORTER,
HIGHER-priority half (block-A-only holder: 7 refs / ~30 length / pri ~4666,
which takes $s0 itself). Every split makes the situation worse, in both
directions.

**Consequence.** Within the measured global.c model, no C source that emits
the target's own memory accesses can produce the target's allocation: the
byte content pins pri(arg0) ~ 3979 and pri(walk) ~ 1098-1411 in every variant
tried, and the three levers that could move them (reference lift by
duplication, reference reduction by splitting, live-length inflation) are each
measured dead. The remaining question is therefore FORENSIC - what the
ORIGINAL compile's RA-time reference counts were - not structural.

- [s3] PROCESS NOTE (correcting the s2 ledger): HEAD's src/text1b_b.c did NOT
  carry the s2 form - it still carried the inherited inline-asm body (`s32 v;`
  + `__asm__ volatile("move %0, %1" ...)`), because the s2 ledger commit was
  ledger-only. candidate.c MUST be re-applied at the start of every session
  before any measurement; a session that skips this measures the stale floor
  of 23. src carries the candidate form at the end of session 3.

- [s3] Baseline re-measured this session: sandbox --disable all = 19, build_insns 116 == target 116, allocno table identical to s1/s2 (pseudo 72 nrefs 13 / livelen 98 / pri 3979 -> $s0; pseudo 73 nrefs 5 / livelen 91 / pri 1098 -> $s1; pseudo 74 pri 176 -> $s2).

- [s3] MEASURED D2 (whole walk loop duplicated into both arms of the single junction): arg0 nrefs 13 -> 19, livelen 98 -> 134, pri 3979 -> 5671; walk nrefs 5 -> 9, livelen 91 -> 127, pri 1098 -> 2125. Sandbox 19 -> 58, build_insns 116 -> 155 - find_cross_jump does NOT re-merge the copies, so the form is not byte-neutral and the sanctioned duplicated-statement-into-arms prerequisite fails outright.

- [s3] MEASURED D1 (walk-pointer-only initialiser duplicated into both arms): walk nrefs 5 -> 6, livelen 91 -> 85, pri 1098 -> 1411; arg0 unchanged at 13 refs, pri 3900. Sandbox 19 -> 25, build_insns 116 -> 118 (a second lw %gp_rel(D_800A3610) as new pseudo 76 in $v1). Walk-only duplication yields exactly +1 reference per junction and the function has one junction.

- [s3] The loop body contains SIX arg0 references against FOUR walk-pointer references, so duplication grows the parameter's reg_n_refs 1.5x faster than the walk pointer's; the priority ratio therefore has an asymptotic floor of 1.5 and the k=2 measurement (2.67x) is the best point on the curve - at k=3 arg0 crosses floor_log2's 16-reference step and the ratio worsens. No amount of duplication can flip the sort.

- [s3] The flip condition is 2-D, not the one-sided ">= 14 walk refs" of s2: at arg0 = 8 refs / len 98 (pri 2448) a walk pointer with 8 refs / len 91 (pri 2637) already wins, because floor_log2 steps. The relaxation is still unreachable - arg0's 13 refs are one def plus the target's own twelve buffer accesses, and splitting them across pseudos produces a shorter, HIGHER-priority half (s2: 7 refs / ~30 len / pri ~4666), so every split moves the wrong way.

- [s3] PROCESS CORRECTION to the s2 ledger: HEAD's src/text1b_b.c did NOT carry the s2 form — it still had the inherited inline-asm body ('s32 v;' + __asm__ volatile("move %0, %1" ...)), because the s2 ledger commit was ledger-only. memory/grind/func_80078654/candidate.c MUST be re-applied at the start of every session before any measurement, or the session measures the stale floor of 23. src carries the candidate form (floor 19) at the end of session 3.

- [s3] Baseline re-measured this session with candidate.c applied: sandbox --disable all = 19, build_insns 116 == target_insns 116, and the instrumented cc1 reprints the s1/s2 allocno table verbatim (pseudo 72 arg0 nrefs 13 / livelen 98 / pri 3979 -> $s0; pseudo 73 walk nrefs 5 / livelen 91 / pri 1098 -> $s1; pseudo 74 zero pri 176 -> $s2).

- [s3] MEASURED D2 (whole walk loop duplicated into both arms of the single junction): arg0 nrefs 13 -> 19, livelen 98 -> 134, pri 3979 -> 5671; walk nrefs 5 -> 9, livelen 91 -> 127, pri 1098 -> 2125. Sandbox 19 -> 58, build_insns 116 -> 155 — jump2's find_cross_jump does NOT re-merge the two loop copies (they carry their own back-edges and exit tests, so no common mergeable suffix is presented).

- [s3] MEASURED D1 (walk-pointer-only initialiser duplicated into both arms): walk nrefs 5 -> 6, livelen 91 -> 85, pri 1098 -> 1411; arg0 unchanged at 13 refs, livelen 98 -> 100, pri 3900. Sandbox 19 -> 25, build_insns 116 -> 118 — the duplicated initialiser materialises a SECOND lw %gp_rel(D_800A3610) (new pseudo 76, allocated $v1) because the arms are separated by the whole of block A and its four calls; the target contains exactly one such load, at insn 5.

- [s3] The loop body — the only region of the function containing walk-pointer references — holds SIX arg0 references (arg0[3] x2, arg0[5] x3, plus the store) against FOUR walk-pointer references. Duplication therefore grows the parameter's reg_n_refs 1.5x faster than the walk pointer's, so the priority ratio has an asymptotic FLOOR of 1.5 and can never reach 1.0; the measured k=2 point (2.67x) is the best point on the curve and k=3 is worse.

- [s3] The flip condition is a 2-D frontier, not the one-sided '>= 14 walk refs' of s2: at arg0 = 8 refs / len 98 (pri 2448) a walk pointer with 8 refs / len 91 (pri 2637) already wins. The relaxation is unreachable because arg0's 13 refs are one def plus the target's own twelve buffer accesses and every split of them produces a shorter, higher-priority half (s2 measured 7 refs / ~30 len / pri ~4666, which takes $s0 itself).

- [s3] Consequence for the ladder: within the fully-measured global.c model, all three levers that could move the two priorities — reference lift by duplication (this session), reference reduction by splitting (s2), and live-length inflation (s2 H6) — are now measured dead. No C source that emits the target's own memory accesses can produce the target's allocation, so the remaining question is FORENSIC (what the original compile's RA-time reference counts were) or a search question for the permuter, not a hand-derivable structural one.

- [s3] Incidental measurement worth keeping: local-alloc DOES hand out callee-saves in this translation unit for other functions (ALLOCDBG seed_used for func_80077D10 includes 16, for func_800784E4 includes 16 and 17), so the empty callee-save seed measured for func_80078654 in s2 is a consequence of this function's shape, not a property of the target/flags. It cannot be exploited here: any local-alloc pseudo taking $s0 would also conflict with the walk pointer, which is live across the entire body.

## SESSION 4 (permuter) — the random-search axis is measured, with a reusable rig

**Floor unchanged at 19.** candidate.c was re-applied to src/text1b_b.c at the
start of the session (HEAD again carried the OLD inline-asm body — the s3 ledger
commit was ledger-only, exactly as the s3 process note warned) and reproduces the
ledger numbers: sandbox `--disable all` = 19, build_insns 116 == target 116. src
carries that form at session end.

### The rig (reusable — this is the session's most durable artifact)
`tmp/grind/func_80078654/s4/mkws.sh` builds a CLEAN single-function permuter
workspace in one command:
* `target.o` = `tools/decomp-permuter/prelude.inc` with the `.set gp=64` line
  stripped (r3000) + `asm/funcs/func_80078654.s`, so the function sits at
  offset 0 and the permuter score is the REAL weighted diff, not ~340k of
  branch-address noise.
* `compile.sh` reproduces the exact build pipeline: cc1 `-O2 -G0 -funsigned-char
  -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel` → prologue_fix →
  maspsx (`--expand-div --aspsx-version=2.34` + all five gate lists) →
  multu_pad → `as -march=r3000 -no-pad-sections -O1 -G0`.
* `base.c` is a STANDALONE 6-typedef TU carrying only the externs, the `S78654`
  struct and the function, so no other function shares the object. The script
  self-validates by objdump-diffing base.o against target.o.
**Measured base score = 108**, and the validating diff prints exactly the twelve
known `$s0`/`$s1` lines and nothing else — independent confirmation, through a
completely different tool chain than `engine sandbox`, that the residual is the
one allocation inversion and nothing more.

### Campaign A — random from the clean floor-19 base (chassis A)
`s4-random-clean19`, `-j 8 --stop-on-zero --stack-diffs`, workspace
`tmp/grind/func_80078654/s4/ws`. **39,636 iterations / 1005 s.** Best new score
**108 == base**; the two saved finds (`output-108-1`, `output-108-2`) are
equal-score and both are semantics-CHANGING permuter rewrites
(`var_s0 = &arg0[5]; SetDrawMode(*var_s0, ...)` and
`var_s0 = &arg0[3]; s.c = *var_s0;` — they clobber the walk pointer inside the
loop), so neither is a usable form. **Zero score-improving finds.**

### Campaign B — random from the base/walk-pointer merge (chassis B)
`s4-chassisB-basemerge`, same flags, workspace `tmp/grind/func_80078654/s4/ws2`.
Seeded from the s2 H4 form because it is the only measured shape that materially
lifts the walk pointer's allocno priority (nrefs 5 → 8, pri 1098 → 2448) — i.e.
the structurally different chassis nearest the 2-D flip frontier.
**49,448 iterations / ~19 min.** Base score **331**; best find **191**; five
finds total, all still far ABOVE chassis A's 108, and no novel find in the final
9-minute window. The basin is strictly worse and does not tunnel back.

### What this measures (and what it does not)
88,000+ iterations of random structural mutation over the two chassis produced
**no form scoring below the hand-derived base**. The permuter's mutation set
(statement reordering, temp introduction/removal, expression respelling, type
and cast changes, `&x[i]`/`*(x+i)` re-spelling) is exactly the space of edits
that leaves the emitted memory accesses intact — and by the s1-s3 priority
model, every such edit leaves `reg_n_refs(arg0) = 13` and `reg_n_refs(walk) ≤ 8`.
So this is not a lucky miss: the search space the permuter samples is provably
inside the region the model already proves cannot flip the sort. The permuter
axis is therefore CLOSED as a route to the flip, and the residual question is
FORENSIC (what the original compile's RA-time reference counts were) or a
RE-DERIVATION question, exactly as s3 concluded.

- [s4] PROCESS NOTE (third consecutive session): HEAD's src/text1b_b.c again carried the inherited inline-asm body (`s32 v;` + `__asm__ volatile("move %0, %1" ...)`), because every ledger commit so far has been ledger-only. candidate.c MUST be re-applied before any measurement. Applied, re-measured: sandbox --disable all = 19, build_insns 116 == target_insns 116.

- [s4] A CLEAN single-function permuter rig now exists and is reusable: tmp/grind/func_80078654/s4/mkws.sh builds target.o from asm/funcs/func_80078654.s + the r3000-ified prelude (offset 0), a compile.sh reproducing the exact cc1/prologue_fix/maspsx/multu_pad/as pipeline, and a standalone 6-typedef base.c holding only this function. Measured base score 108, and its self-validating objdump diff prints exactly the twelve known $s0/$s1 lines — an independent confirmation of the residual through a different toolchain than engine sandbox.

- [s4] MEASURED campaign A (s4-random-clean19, random mutation from the clean floor-19 base, -j 8, --stop-on-zero, --stack-diffs): 39,636 iterations in 1005 s, ZERO score-improving finds. The two saved finds are equal-score (108 == base) and both are semantics-CHANGING rewrites that clobber the walk pointer inside the loop (var_s0 = &arg0[5] / var_s0 = &arg0[3]), so neither is a usable form.

- [s4] MEASURED campaign B (s4-chassisB-basemerge, random mutation seeded from the s2 H4 base/walk-pointer merge — the only measured shape that lifts the walk pointer's priority, 1098 -> 2448): base score 331, 49,448 iterations, best find 191, no novel find in the final 9-minute window. Every point in that basin scores far above chassis A's 108; the basin is strictly worse and does not tunnel back to the clean base.

- [s4] Interpretation, and why the negative is strong rather than merely unlucky: the permuter's mutation set (statement reordering, temp introduction/removal, expression and cast respelling, &x[i] vs *(x+i)) is precisely the class of edits that preserves the emitted memory accesses, and the s1-s3 priority model proves every such edit leaves reg_n_refs(arg0) = 13 and reg_n_refs(walk) <= 8, which cannot flip allocno_compare. 88,000+ iterations across two chassis confirm the prediction empirically. The permuter axis is closed as a route to the $s0/$s1 flip.

- [s4] PROCESS NOTE, third consecutive session: HEAD's src/text1b_b.c again carried the inherited inline-asm body (`s32 v;` + `__asm__ volatile("move %0, %1" ...)`) because every ledger commit so far has been ledger-only. memory/grind/func_80078654/candidate.c was re-applied before any measurement; re-measured sandbox --disable all = 19, build_insns 116 == target_insns 116. src carries the candidate form at session end.

- [s4] A reusable clean single-function permuter rig now exists: tmp/grind/func_80078654/s4/mkws.sh builds target.o from asm/funcs/func_80078654.s + tools/decomp-permuter/prelude.inc with the '.set gp=64' line stripped (r3000), so the function sits at offset 0; compile.sh reproduces the exact cc1/prologue_fix/maspsx/multu_pad/as pipeline including -mel and all five maspsx gate lists; base.c is a standalone 6-typedef TU carrying only the externs, the S78654 struct and the function.

- [s4] Measured permuter base score for the floor-19 form: 108. The rig's self-validating objdump diff prints exactly the twelve known $s0/$s1 lines and nothing else — independent confirmation, through a different toolchain than engine sandbox, that the residual is the one allocation inversion.

- [s4] MEASURED campaign A (label s4-random-clean19, workspace tmp/grind/func_80078654/s4/ws, -j 8 --stop-on-zero --stack-diffs): 39,636 iterations in 1005 s, ZERO score-improving finds. Both saved finds are equal-score (108 == base) and are semantics-CHANGING rewrites that clobber the walk pointer inside the loop, so neither is usable.

- [s4] MEASURED campaign B (label s4-chassisB-basemerge, workspace tmp/grind/func_80078654/s4/ws2): seeded from the s2 H4 base/walk-pointer merge (walk nrefs 5 -> 8, pri 1098 -> 2448 — the shape nearest the 2-D flip frontier). Base score 331, 49,448 iterations, best find 191, no novel find in the final 9-minute window. Every point in that basin scores far above chassis A's 108 and it does not tunnel back.

- [s4] Interpretation banked in the ledger: the permuter's mutation set (statement reordering, temp introduction/removal, expression and cast respelling, &x[i] vs *(x+i)) is precisely the class of edits that preserves the emitted memory accesses, and the s1-s3 priority model proves every such edit leaves reg_n_refs(arg0) = 13 and reg_n_refs(walk) <= 8, so allocno_compare cannot flip. The 89,084 iterations confirm the prediction empirically rather than merely failing to find something.

- [s4] Both campaigns were harvested with --stop inside the session (permuter_campaign.py status reports alive=false / registered_active=false for both), and the in-turn wait loop exited; no campaign outlives this session.

## SESSION 5 (permuter — directed PERM_* sweep + three-chassis priority census)

- [s5] PROCESS NOTE, fourth consecutive session: HEAD's src/text1b_b.c again carried the
  inherited inline-asm body (`s32 v;` + `__asm__ volatile("move %0, %1" ...)`), because every
  ledger commit so far has been ledger-only. memory/grind/func_80078654/candidate.c was
  re-applied before any measurement and the baseline re-confirmed:
  `sandbox func_80078654 --disable all` = 19, build_insns 116 == target_insns 116,
  rules_dropped 6, cheat_asm_stripped 89.

- [s5] m2c IS NOT INSTALLED in this tree. `python3 -m m2c.main` fails with
  ModuleNotFoundError (no m2c package in .venv, no ~/m2c checkout; only the historical
  outputs in tmp/blitz/ and include/m2c_context.h remain). The ledger's "rederive with m2c"
  next-probe therefore cannot be executed as written — a future rederive session must either
  install decompals/m2c or hand-derive from asm/funcs/func_80078654.s. This is a hard blocker
  discovered by measurement, not an opinion; record it before planning that modality.

- [s5] A new, cheaper instrument exists: tmp/grind/func_80078654/s5/eval.sh <chassis.c> <label>.
  One invocation compiles a STANDALONE single-function chassis with the instrumented cc1
  (tools/gcc-2.7.2/cc1, BB2_ALLOC_DEBUG=1) and prints the full allocno table, then re-compiles
  the same file through the exact build pipeline (build/cc1 -mel | prologue_fix | maspsx with
  all five gate lists | multu_pad | as -march=r3000) and objdump-diffs it against the s4 rig's
  target.o. It costs ~5 s and needs no src/ edit and no sandbox run, so a structural variant can
  now be judged on priorities AND bytes in a single call. Baseline chassis A reproduces the
  known table exactly: pseudo 72 (arg0) nrefs=13 livelen=98 pri=3979 -> $s0; pseudo 73 (walk)
  nrefs=5 livelen=91 pri=1098 -> $s1; pseudo 74 (zero) nrefs=3 livelen=170 pri=176 -> $s2;
  116 == 116 insns, 38 objdump diff lines (the 19 known exchanged instructions, both sides).

- [s5] MEASURED chassis C1 — INDEX-EXPRESSED WALK (`s32 i = 5; ... D_800A3610[i]; i++;
  if (D_800A3610[i + 1] != -1)`), the partition the permuter provably cannot invent because it
  changes the induction variable's type rather than an expression's spelling. KILLED at the
  first measurement: loop strength-reduction re-creates a pointer induction variable anyway but
  keeps the base register live, so the function grows to 120 insns (vs target 116) and 50 diff
  lines, and the walk allocno gets WORSE not better — pseudo 73 nrefs 5 -> 4, livelen 91 -> 94,
  pri 1098 -> 851, while arg0 stays at 13 refs (pri 3861). Banked at
  rejected/index-walk-strength-reduce-plus4-insns.c.

- [s5] MEASURED chassis C2 — SUB-POINTER ARG0-LOWERING (`s32 *ot = arg0 + 5;` hoisted to
  function scope, every `arg0[5]` rewritten as `*ot`), i.e. the arg0-LOWERING quadrant of the s3
  2-D frontier, which no prior session had ever built (s2 measured only a block-scoped split and
  s4 seeded only the walk-RAISING chassis). KILLED, and it produces the single most informative
  number of this session: arg0's references do drop 13 -> 6 and its priority drops 3979 -> 1212,
  but the walk pointer is STILL BELOW IT at 1086 (nrefs 5, livelen 92) — because livelen 99 vs 92
  and floor_log2(6) == floor_log2(5) == 2 make the comparison 2*6/99 vs 2*5/92. So even a
  7-reference reduction of the parameter does not flip the sort. Extrapolating on the same
  measured live lengths, arg0 must reach <= 5 references (2*5/99 = 1010 < 1086) — i.e. ELEVEN of
  its twelve emitted accesses must leave the pseudo — before the walk pointer wins. Worse, the
  extracted `ot` pseudo behaves exactly as s2/s3 predicted for every split: it is shorter-lived
  (85) with more references (9) and therefore OUTRANKS everything at pri 3176, taking $s0 itself
  and forcing a FOURTH callee-save ($s3); 119 insns, 53 diff lines. Banked at
  rejected/ot-subpointer-arg0-lowering-4th-callee-save.c.

- [s5] MEASURED campaign (label s5-directed-perm, workspace tmp/grind/func_80078654/s5/ws,
  -j 8 --stop-on-zero --stack-diffs): the ledger's F-directed probe — hand-written PERM_* macros
  over genuine alternatives rather than s4's uniform random sampling. Six directed axes:
  PERM_GENERAL over three walk-initialiser partitions (`D_800A3610 + 5` / `&D_800A3610[5]` /
  split base-then-`+= 5`); PERM_GENERAL over three block-A shapes including the `s32 *ot =
  &arg0[5]` arg0-lowering sub-pointer; PERM_GENERAL over four loop-head read shapes including
  the double-read `s.b = var_s0[0] + 0xC;` ref-lift candidate; PERM_LINESWAP over the
  `var_s0++` / `arg0[5] += 0xC` pair; PERM_GENERAL over four loop-test spellings; plus two
  PERM_RANDOMIZE regions so random mutation still runs on the prologue and the call block.
  Base score 108 (identical to chassis A, confirming the default expansion is the floor-19 form).
  RESULT: 39,950 iterations in 1152 s, ZERO score-improving finds; two equal-score (108) finds at
  147 s and 584 s, then a full 9-minute window with no novel find at all, at which point the
  fresh-seed rule was honoured and the campaign harvested with --stop (procs_killed 9,
  pid_alive_at_harvest true -> stopped true). No campaign outlives this session.

- [s5] Both s5 finds reproduce s4's failure mode EXACTLY and are unusable for the same reason:
  each one clobbers the walk pointer inside the loop (`var_s0 = &arg0[3];` in output-108-1,
  `var_s0 = &arg0[5];` in output-108-2) and is therefore semantics-CHANGING, not a respelling.
  Two independent campaigns on two different mutation regimes converging on the same
  semantics-breaking equal-score attractor is itself evidence: the permuter's scorer cannot see
  the walk-pointer clobber, so that attractor will absorb any future campaign on this function.
  A third campaign on this chassis family has negative expected value.

- [s5] Cumulative permuter evidence for this function is now 129,034 iterations across THREE
  chassis and TWO mutation regimes (s4 random on the clean base, s4 random on the walk-raising
  merge, s5 directed cross-product on the clean base) with zero score-improving finds, plus two
  additional chassis (C1, C2) killed analytically on their allocno tables without needing a
  campaign at all. Every one of them leaves reg_n_refs(arg0) >= 6 with a live length longer than
  the walk pointer's, so allocno_compare never inverts. The permuter modality is closed for
  func_80078654; the remaining frontier is post-RA reference provenance (forensics) and a
  from-scratch re-derivation (rederive, which now needs m2c installed first).

- [s5] Baseline re-confirmed after re-applying candidate.c over HEAD's stale inline-asm body (fourth consecutive session that HEAD did not carry the candidate): sandbox func_80078654 --disable all = 19, build_insns 116 == target_insns 116, rules_dropped 6, cheat_asm_stripped 89. src/text1b_b.c was restored to HEAD at session end, so the tree carries only ledger changes.

- [s5] New one-call instrument banked: tmp/grind/func_80078654/s5/eval.sh <chassis.c> <label> compiles a STANDALONE single-function chassis with the instrumented cc1 (BB2_ALLOC_DEBUG=1) and prints the allocno table, then re-compiles the same file through the exact build pipeline and objdump-diffs against the s4 rig's target.o. ~5 s, no src/ edit, no sandbox run — a structural variant is now judged on priorities AND bytes in one call.

- [s5] Chassis A (the floor-19 candidate) reproduces the known allocno table exactly through the new instrument: pseudo 72 (arg0) nrefs=13 livelen=98 pri=3979 -> $s0; pseudo 73 (walk) nrefs=5 livelen=91 pri=1098 -> $s1; pseudo 74 (zero) nrefs=3 livelen=170 pri=176 -> $s2; 116 == 116 insns; 38 objdump diff lines (the 19 known exchanged instructions counted on both sides).

- [s5] Directed campaign s5-directed-perm: base 108, 39,950 iterations, 1152 s, zero score-improving finds, two equal-score finds, harvested with --stop after a full no-novel-find window; permuter_campaign.py status reports alive=false / registered_active=false for it and for both s4 campaigns. No campaign outlives this session.

- [s5] Both s5 finds are semantics-CHANGING in exactly the way s4's were — they assign the walk pointer from arg0 inside the loop (`var_s0 = &arg0[3];`, `var_s0 = &arg0[5];`). The permuter's scorer cannot see the clobber, so this equal-score attractor will absorb any future campaign on this chassis family; a third campaign has negative expected value.

- [s5] Cumulative permuter evidence is now 129,034 iterations across THREE chassis and TWO mutation regimes (s4 random on the clean base, s4 random on the walk-raising merge, s5 directed cross-product on the clean base) with zero score-improving finds, plus two further chassis (index-walk, sub-pointer) killed on their allocno tables without a campaign. Every measured form leaves reg_n_refs(arg0) >= 6 with a live length longer than the walk pointer's, so allocno_compare never inverts.

- [s5] SHARPEST BOUND TO DATE (new this session, from the sub-pointer measurement): lowering arg0 from 13 to 6 references is NOT sufficient — the walk pointer is still below it (1212 vs 1086) because floor_log2(6) == floor_log2(5) and the live lengths are 99 vs 92. On the measured live lengths arg0 must reach <= 5 references, i.e. eleven of its twelve emitted accesses must leave the pseudo, and every extraction that achieves this creates a shorter-lived higher-priority pseudo that takes $s0 itself (measured 3176 here, 4666 in s2).

## SESSION 6 (forensics) — the per-insn provenance table, and the exact window in which a "free" reference can exist

- [s6] BASELINE REPRODUCED. The s5 standalone chassis (`tmp/grind/func_80078654/s5/chassisA.c`, byte-identical to `memory/grind/func_80078654/candidate.c`) still measures allocnos `ord=3 pseudo=72 (arg0) nrefs=13 livelen=98 pri=3979`, `ord=4 pseudo=73 (walk) nrefs=5 livelen=91 pri=1098`, `ord=5 pseudo=74 (zero) nrefs=3 livelen=170 pri=176`, 116 insns == target's 116, 38 objdump diff lines. Note the arithmetic identity worth remembering: 38 diff lines == 19 changed instruction pairs == the sandbox floor of 19. The standalone chassis and the full-TU sandbox are measuring the same 19 instructions.

- [s6] THE FULL RTL DUMP SET NOW EXISTS. `tmp/grind/func_80078654/s6/dump.sh` runs the instrumented cc1 (`tools/gcc-2.7.2/cc1`, NOT `build/cc1`) with `-da` from inside the output directory, producing `in.c.rtl .jump .cse .loop .cse2 .flow .combine .sched .lreg .greg .sched2 .jump2 .dbr` in `tmp/grind/func_80078654/s6/base/`. This is reusable for any future forensics session on this function: `bash tmp/grind/func_80078654/s6/dump.sh <chassis.c> <label>`. The per-stage reference counter is `tmp/grind/func_80078654/s6/count.py`, the per-insn provenance dumper is `tmp/grind/func_80078654/s6/prov.py`.

- [s6] **PER-INSN PROVENANCE TABLE (the ledger's F1 named deliverable) — the reference counts are pinned END TO END, from RTL expansion to register allocation.** Pseudo 72 (arg0) is mentioned in exactly 13 insns, uids `4, 118, 129, 142, 164, 169, 173, 203, 214, 227, 249, 257, 261`, one mention each. Pseudo 73 (walk) is mentioned in exactly 4 insns, uids `47, 185, 254, 265`, with uid 254 (`(set 73 (plus 73 4))`) carrying TWO mentions — hence nrefs 5 from 4 insns. **The identical uid sets with identical per-insn counts appear in every one of `.rtl`, `.jump`, `.cse`, `.loop`, `.cse2`, `.flow`, `.combine` and `.sched`.** Not one reference to either pseudo is created or destroyed by any pass between RTL expansion and register allocation. uid 4 is `(set (reg/v:SI 72) (reg:SI 4 a0))` — the parameter copy; the other twelve are exactly the target's `12($sN)` accesses (uids 118, 129, 203, 214) and `20($sN)` accesses (uids 142, 164, 169, 173, 227, 249, 257, 261). **No buffer access is reload-generated, none is post-RA, and there are no reload address reloads at all** (every offset is a 16-bit displacement off a base register). The frontier hypothesis "some reference was created after RA" is therefore KILLED outright. Table banked at `tmp/grind/func_80078654/s6/provenance_table.txt`, per-stage counts at `tmp/grind/func_80078654/s6/refcount_by_stage.txt`.

- [s6] **THE "FREE REFERENCE" WINDOW IS [flow_analysis, global_alloc) AND NOTHING ELSE — mechanism confirmed from the compiler source.** `toplev.c` pass order is `flow_analysis` (line 2983, the ONLY place `reg_n_refs` is accumulated — `flow.c:2081,2329,2515,2725`) then `combine_instructions` (3004) then `schedule_insns` (3033) then `regclass`+`local_alloc` (3051) then `global_alloc` (3080). `reg_n_refs` is never recomputed. `combine.c:56` says so verbatim ("reg_n_refs is not adjusted in the rare case when a register is..."); combine only ZEROES it for a wholly-deleted reg (`combine.c:2313,2336`) and bumps a 2 to a 3 (`combine.c:10745-10754`). `local-alloc.c` is the other writer: `optimize_reg_copy_1/2` TRANSFER references across a register copy (`781-783`, `913-914`), `update_equiv_regs` DOUBLES `reg_live_length` for a REG_EQUIV pseudo (`1064`) and ZEROES `reg_n_refs` for a two-reference replaceable one (`1110`). So a reference that exists at flow time and dies before global_alloc genuinely IS free — this is the real mechanism behind the project's sanctioned-F1 "combine-foldable chain-extender". It is not folklore; it is in the source.

- [s6] **BUT THE MEASURED YIELD OF THAT WINDOW ON THIS FUNCTION IS ZERO OR NEGATIVE.** Three chassis measured with `tmp/grind/func_80078654/s5/eval.sh` (allocno table + full-pipeline objdump diff in one call):
  - V1, split increment `{ s32 *nx = var_s0 + 1; var_s0 = nx; }` — allocno table BYTE-IDENTICAL to base (13/98/3979 and 5/91/1098), 116 insns, 38 diff lines. **Zero yield, and the reason is structural: a SET counts as a reference**, so splitting `73 = 73 + 4` (one insn, two mentions) into `nx = 73 + 4` (one use) plus `73 = nx` (one set) preserves the total at two. Splitting can never raise `reg_n_refs`.
  - V3, address chain-extender `{ s32 *nx = var_s0 + 1; s.a = nx[-1]; }` — walk nrefs UNCHANGED at 5, 117 insns, 41 diff lines. Combine absorbs an extender by SUBSTITUTION, which relocates the pseudo's mention from the deleted insn into the surviving one; it never duplicates it.
  - V2, double read `s.b = var_s0[0] + 0xC;` — the only shape that raises the count: walk nrefs 5 to 6, livelen 91 to 92, pri 1098 to 1304 (arg0 unchanged at 13 refs, pri 3939). But the second load survives cse and cse2 and the form emits 118 insns / 52 diff lines. **Measured price: +2 emitted instructions per +1 walk reference.**
  Banked at `rejected/combine-chain-split-increment-zero-ref-yield.c`, `rejected/nx-chain-address-extender-zero-ref-yield.c`, `rejected/double-read-ref-lift-byte-materializes-2-insns-per-ref.c`.

- [s6] **THE FLIP CONDITION IS STRICT, NOT WEAK — allocno_compare's tie-break structurally favours the parameter.** `allocno_compare` (global.c) returns `pri2 - pri1` and, when those are equal, `return *v1 - *v2` — the LOWER allocno index sorts first. Allocnos are numbered in ascending pseudo order and the parameter's pseudo is created by `assign_parms` before any local's, so arg0 always holds the lower index. **Equal priority does NOT flip the allocation.** Every ">=" bound previously recorded in this ledger must be read as strict. Recomputed exactly on the measured live lengths: the walk pointer needs `floor_log2(w)*w > 3979*91/10000 = 36.2`, i.e. `w >= 13` (`w = 12` gives `3*12/91*10000 = 3956 < 3979`). That is +8 references, or +16 emitted instructions at V2's measured price.

- [s6] **THE find_reg PASS-0 SEEDING ROUTE IS KILLED BY THE TARGET BYTES.** s2 measured that `seed_used` contains no callee-save and concluded pass 0 cannot award $s0/$s1 *for the current C*; it did not ask whether a different C could SEED a callee-save into `regs_used_so_far` (global.c:344-372 seeds it from `regs_ever_live` plus local-alloc's renumberings), which would make pass 0 hand arg0 an already-used $s1 and leave $s0 to the walk pointer under pass 1's ascending first-fit. That route requires `local_alloc` to have placed some fourth, short-lived value in a callee-save. A register census of the target disassembly rules it out: the target mentions `s0` 7 times, `s1` 15 times, `s2` 5 times and **`s3`-`s7` zero times** — exactly the three known values (walk = 1 def + 4 uses + save/restore; arg0 = 1 copy + 12 accesses + save/restore; zero = 1 li + 2 uses + save/restore) and no fourth callee-save resident anywhere. Any C that seeds a callee-save materialises a fourth s-register in the bytes. Census at `tmp/grind/func_80078654/s6/target_calleesave_census.txt`.

- [s6] **THE RESULTING PROOF OBLIGATION, STATED EXACTLY — this is what a future session must satisfy.** Because no pass between expansion and RA creates or destroys a reference (provenance table) and the only free-reference window is [flow, global_alloc), `reg_n_refs` of a pseudo is bounded BELOW by the number of emitted insns mentioning its hard register (excluding prologue save / epilogue restore). The target mentions `$s1` in 13 such insns, so in ANY compile that emits the target bytes `reg_n_refs(arg0) >= 13` and `pri(arg0) >= 3979` at livelen 98 (and >= 3170 even at the maximum possible livelen of ~123). The target mentions `$s0` in 5 such insns. **Therefore the original compile must have carried >= 8 references to the walk pointer that existed at `flow_analysis` time and were gone by `global_alloc` — deleted by combine, or transferred onto it by `local-alloc.c`'s `optimize_reg_copy_1/2`.** That is the single remaining explanation for the residual, and it is now a precise, falsifiable target rather than a vague "the dataflow differed": find a C form in which eight walk-pointer references die inside that window. s6 measured the two obvious mechanisms (chain splitting, combine absorption) at exactly zero yield, so what remains untried is the `optimize_reg_copy_1/2` reference-TRANSFER path — a copy insn `walk = src` whose source dies later and whose post-copy uses local-alloc rewrites onto the walk pseudo, moving their references onto it byte-neutrally.

- [s6] Baseline reproduced unchanged on the s5 standalone chassis: allocnos ord=3 pseudo=72 (arg0) nrefs=13 livelen=98 pri=3979, ord=4 pseudo=73 (walk) nrefs=5 livelen=91 pri=1098, ord=5 pseudo=74 (zero) nrefs=3 livelen=170 pri=176; 116 insns == target's 116; 38 objdump diff lines. Useful identity: 38 diff lines == 19 changed instruction pairs == the sandbox floor of 19, so the standalone chassis and the full-TU sandbox measure the same 19 instructions.

- [s6] A reusable full-RTL-dump rig now exists for this function: `bash tmp/grind/func_80078654/s6/dump.sh <chassis.c> <label>` runs the instrumented cc1 with -da from inside the output directory and emits in.c.rtl/.jump/.cse/.loop/.cse2/.flow/.combine/.sched/.lreg/.greg/.sched2/.jump2/.dbr. Companion analysers: count.py (per-stage reference counts) and prov.py (per-insn provenance).

- [s6] PER-INSN PROVENANCE TABLE (the ledger's outstanding deliverable): pseudo 72 = 13 mentions across uids 4,118,129,142,164,169,173,203,214,227,249,257,261; pseudo 73 = 5 mentions across uids 47,185,254,265 (uid 254 carries two). The same uid sets and per-insn counts hold at .rtl, .jump, .cse, .loop, .cse2, .flow, .combine and .sched - zero references created or destroyed between expansion and RA.

- [s6] No reload-generated address reloads exist in this function at all: every one of the twelve buffer accesses is a plain 16-bit displacement off a base register, present as a MEM (plus reg const_int) from the .rtl dump onward.

- [s6] reg_n_refs is accumulated ONLY in flow.c (2081, 2329, 2515, 2725) during flow_analysis (toplev.c:2983) and is never recomputed before global_alloc (toplev.c:3080), with combine (3004) and sched (3033) running in between; combine.c:56 states verbatim that reg_n_refs is not adjusted. The only later writers are combine.c:2313/2336 (zero a wholly deleted reg), combine.c:10745-10754 (bump 2 to 3), and local-alloc.c's optimize_reg_copy_1/2 (781-783, 913-914, reference TRANSFER across a copy) and update_equiv_regs (1064 doubles reg_live_length, 1110 zeroes reg_n_refs).

- [s6] Measured yield of that free-reference window on this function: split-increment chain-extender = allocno table byte-identical to base (zero yield, because a SET counts as a reference); nx[-1] address chain-extender = walk nrefs unchanged at 5 and +1 emitted insn (combine absorbs by substitution, which relocates rather than duplicates the mention); double-read = walk nrefs 5 -> 6 / pri 1098 -> 1304 but +2 emitted insns. Price of a walk reference is therefore +2 instructions.

- [s6] allocno_compare's tie-break is `return *v1 - *v2` (lower allocno index first) and the parameter's pseudo is created by assign_parms before any local's, so arg0 wins every priority tie. The flip condition is strict: floor_log2(w)*w > 36.2, i.e. w >= 13 walk references (w = 12 yields 3956 < 3979).

- [s6] Target callee-save census: s0 x7, s1 x15, s2 x5, s3-s7 x0 - exactly three callee-save-resident values, all accounted for. No fourth value for local_alloc to have seeded into regs_used_so_far.

- [s6] THE PROOF OBLIGATION, now exact: reg_n_refs of a pseudo is bounded BELOW by the number of emitted insns mentioning its hard register (save/restore excluded), because nothing creates or destroys references after expansion and the only deletion window is [flow, global_alloc). The target mentions $s1 in 13 such insns, so in ANY compile emitting the target bytes reg_n_refs(arg0) >= 13 and pri(arg0) >= 3979 at livelen 98 (>= 3170 even at the maximum possible livelen of ~123). Hence the original compile must have carried >= 8 walk-pointer references that existed at flow_analysis and were gone by global_alloc.

## SESSION 7 (forensics) — the optimize_reg_copy reference-TRANSFER frontier, and allocno sharing

- [s7] Floor unchanged at 19 (`sandbox func_80078654 --disable all`: score 19, target_insns 116, build_insns 116, rules_dropped 6) with the s1 candidate body applied to src/text1b_b.c. HEAD still did NOT carry that body at session start (the inherited `s32 v;` + INLINE_MOVE_ALIASING form was still in the file); it is applied again now, exactly as candidate.c prescribes.

- [s7] THE COPY-INSN CENSUS AT LOCAL_ALLOC TIME (new tool tmp/grind/func_80078654/s7/copyscan.py, which parses an RTL dump and reports every insn whose pattern is a bare `(set (reg X) (reg Y))`). In the RTL that local_alloc consumes (in.c.combine, identical to in.c.lreg for this purpose) the base compile contains exactly FIVE reg<-reg copies, and only ONE of them has a PSEUDO destination: uid 4, `(set (reg/v:SI 72) (reg:SI 4 a0))` — arg0's parameter home. The other four have hard-register destinations (uid 134 and 219 `a1 <- reg/v:SI 74`, uid 148 and 233 `a3 <- reg:SI 2 v0`). There is NO copy insn anywhere in the function whose destination is pseudo 73, the walk pointer. By contrast in.c.rtl (pre-cse) holds sixteen copies, three of them pseudo-to-pseudo (uids 108, 155, 240) — so cse deletes every pseudo-to-pseudo copy this body produces before combine even runs.

- [s7] ZERO reference transfer occurs in the base compile — measured, not argued. New tool tmp/grind/func_80078654/s7/mentions.py counts, per pseudo, the insns and the raw mentions in an RTL dump with the REG_NOTES stripped (notes must be excluded: flow.c counts references from insn PATTERNs only). On in.c.combine AND on in.c.lreg the counts are 72: 13 insns / 13 mentions; 73: 4 / 5; 74: 3 / 3; 75: 2 / 2; 81: 4 / 4; 85: 3 / 3; 91: 2 / 2. The post-local_alloc ALLOCDBG line for every one of those pseudos prints exactly the same nrefs (72 nrefs=13, 73 nrefs=5, 74 nrefs=3, 81 nrefs=4, 85 nrefs=3, 91 nrefs=2). local-alloc.c's optimize_reg_copy_1/2 therefore fired ZERO times on this function, consistent with the copy census above, and there is no hidden reference bookkeeping between flow and global_alloc in the base at all. This also re-confirms loop_depth == 1 everywhere (nrefs == raw mention count, no weighting).

- [s7] The alias round-trip the s6 frontier named — `p = var_s0; s.a = p[0]; ...; p++; var_s0 = p;` inside the loop body, the exact optimize_reg_copy_2 shape (SRC dead at the copy, a later `SRC = DEST` in which DEST dies) — is DELETED BY CSE before it can reach local_alloc. Chassis tmp/grind/func_80078654/s7/v1_alias_writeback.c: three pseudo-to-pseudo copies in in.c.rtl (uids 108, 155, 243), ZERO in in.c.cse, in.c.combine and in.c.lreg. Its allocno table is byte-identical to base (ord=3 pseudo=72 nrefs=13 livelen=98 pri=3979; ord=4 pseudo=73 nrefs=5 livelen=91 pri=1098), it emits 116 insns, and its objdump diff is the same twelve $s0/$s1 lines. Measured yield of the alias route: exactly 0 references, 0 bytes.

- [s7] WHY NO C SHAPE CAN FIX THAT, stated as a bytes-grounded bound. A reg<-reg copy survives cse only when the copied value must live in two registers simultaneously — i.e. only when it is an EMITTED `move` instruction; a copy whose source stays available is copy-propagated away (measured above). So the set of copies available to optimize_reg_copy_1/2 in any compile is a subset of the target's own `move` instructions. The target's complete move census (twelve): `move s1,a0`, `move s2,zero`, `move v1,v0`, `move v0,v1`, `move v1,zero`, `move v0,v1`, `move a1,s2` x2, `move a2,zero` x2, `move a3,v0` x2. NO move has $s0 (the walk pointer) as its destination — so no copy_1 can donate references TO the walk pseudo; and NO move has $s1 (arg0) as its source — so no hard-destination copy_1 can drain references AWAY from the arg0 pseudo either. Both directions of the transfer are closed by the target bytes. Even the one pseudo-destination copy that does exist, `move s1,a0` (uid 4), is ineligible: optimize_reg_copy_1 requires SRC to be NOT dead at the copy (a0 dies there — its only later occurrences are SETs of a0 as a call argument, and `reg_set_p (src, p)` breaks the forward scan at the first of them), and optimize_reg_copy_2 requires BOTH operands to be pseudos.

- [s7] A SECOND, PREVIOUSLY UNKNOWN byte-free reference mechanism was found in the same pass and is also closed. global.c:443-453 builds allocnos such that pseudos SHARING an allocno have their reference counts SUMMED (`allocno_n_refs[allocno] += reg_n_refs[i]`) and their live lengths MAXed — i.e. allocno sharing would lift a priority for free, with no extra instruction. The sharing map comes from `regs_may_share` (global.c:403-411), whose ONLY producer in the entire compiler is loop.c:1659, inside move_movables' `m->partial && m->match` branch. `m->partial` is set at exactly one place, loop.c:838-849: a pseudo whose in-loop set is `SET_SRC == const0_rtx` immediately followed by a STRICT_LOW_PART SUBREG set of the same register, with n_times_set == 2 (the clear-the-high-bytes narrow-load idiom). An SImode POINTER can never be such a movable, so the walk pointer can never join a shared allocno. And the point is moot twice over: this function's RTL contains ZERO NOTE_INSN_LOOP_BEG notes (the body is a goto-loop, so loop.c's movable machinery never runs on it at all), which is also the independent confirmation that loop_depth is 1 in both flow.c and update_equiv_regs.

- [s7] NET EFFECT ON THE PROOF OBLIGATION. s6 proved the original compile must have carried >= 8 walk-pointer references that existed at flow_analysis and were gone by global_alloc, and named two candidate mechanisms: combine deletion (s6 measured at zero yield) and the optimize_reg_copy transfer (this session, unreachable — no eligible copy insn can exist in a compile that emits the target bytes). Both named mechanisms are now dead, and the third route into the sort (allocno sharing) is dead as well. The reference counts are therefore fixed, end to end, by the number of RTL insns that mention each pseudo at expansion: pinned from expansion to RA (s6), un-transferable at local_alloc (s7), un-shareable at global_alloc (s7). The ONLY remaining degree of freedom is a genuinely different DECOMPOSITION of the same dataflow — i.e. the rederive modality — and it now has a cheap pre-screen: any candidate body can be checked in one command with tmp/grind/func_80078654/s7/dump.sh + mentions.py, since `mentions in in.c.combine` == `reg_n_refs at RA` is now a measured identity for this function.

- [s7] Floor unchanged at 19 with the s1 candidate body applied to src/text1b_b.c (sandbox --disable all: score 19, target_insns 116, build_insns 116, rules_dropped 6). HEAD still did not carry that body at session start (the inherited `s32 v;` + INLINE_MOVE_ALIASING form was still in the file); it is applied again now.

- [s7] COPY CENSUS AT LOCAL_ALLOC TIME (s7/copyscan.py): base in.c.combine holds five reg<-reg copies — uid 4 `reg72 <- a0` (only pseudo destination), uid 134 and 219 `a1 <- reg74`, uid 148 and 233 `a3 <- v0`. No copy anywhere in the function has pseudo 73 as its destination. in.c.rtl (pre-cse) holds sixteen copies, three pseudo-to-pseudo — so cse deletes every pseudo-to-pseudo copy this body produces before combine runs.

- [s7] NEW MEASURED IDENTITY (s7/mentions.py, REG_NOTES excluded because flow.c counts references from insn PATTERNs only): mentions in in.c.combine == mentions in in.c.lreg == post-local_alloc ALLOCDBG nrefs, for EVERY pseudo (72:13, 73:5, 74:3, 75:2, 81:4, 85:3, 91:2). optimize_reg_copy_1/2 fired zero times; the base has no reference bookkeeping between flow and global_alloc at all. This identity is now a cheap pre-screen: a future candidate body can be rejected on its reference census (dump.sh + mentions.py) before it is ever assembled.

- [s7] The alias round-trip named by the s6 frontier (`p = var_s0; s.a = p[0]; ...; p++; var_s0 = p;`) is deleted by cse: 3 pseudo-pseudo copies at .rtl, 0 at .cse/.combine/.lreg; allocno table byte-identical to base; 116 insns; same twelve $s0/$s1 diff lines. Banked as rejected/alias-writeback-copy-deleted-by-cse-zero-ref-transfer.c.

- [s7] BYTES-GROUNDED BOUND: a reg<-reg copy survives cse only when the value must live in two registers simultaneously, i.e. only as an emitted `move`. So the copies available to optimize_reg_copy in ANY compile are a subset of the target's own moves. The target's twelve moves have no $s0 destination and no $s1 source, closing both the donate-to-walk and drain-from-arg0 directions of the transfer.

- [s7] loop.c never runs on this body: zero NOTE_INSN_LOOP_BEG notes in in.c.rtl. That closes allocno sharing a second way and confirms loop_depth == 1 everywhere (why nrefs == raw mention count).

- [s7] NET: the reference counts are pinned from RTL expansion to RA (s6), un-transferable at local_alloc (s7) and un-shareable at global_alloc (s7). Every mechanism that could satisfy s6's proof obligation (walk >= 13 flow-time references while emitting only 5 mentions of $s0) inside the CURRENT dataflow is now measured dead; only a different decomposition of the dataflow remains.

## SESSION 8 (rederive) — the from-scratch derivation, the alias-split quadrant, and the forced-decomposition theorem

- [s8] Floor unchanged at 19. src/text1b_b.c at session start AGAIN did not carry the candidate body (HEAD still holds the inherited `s32 v;` + INLINE_MOVE_ALIASING form — the s2..s7 ledger commits are ledger-only), so candidate.c was applied first; `sandbox func_80078654 --disable all` then printed score 19, target_insns 116, build_insns 116, rules_dropped 6, cheat_asm_stripped 89. Every measurement below is against that base.

- [s8] THE RE-DERIVATION WAS DONE FROM THE TARGET BYTES ALONE (asm/funcs/func_80078654.s, 123 lines, read in full this session; m2c is still not installed, per s5, so the derivation was done by hand). Every one of the 116 instructions is accounted for: frame 0x58 with the S78654 descriptor at sp+0x18..sp+0x43 (a=0x18, b=0x1C, c=0x20, e=0x28, f=0x2C, g=0x30, h=0x34, cd_flag byte=0x40, b_/g_/r bytes=0x41/0x42/0x43; 0x24 and 0x38/0x3C are never touched); ONE gp-relative load of D_800A3610 into $v1, used both for `lw $v0,0x3C($v1)` (= tbl[0xF]) and for `addiu $s0,$v1,0x14` (= tbl+5) which sits in the DELAY SLOT of the block-A skip branch — so the walk pointer's definition provably precedes block A in the target itself, and its live range spans the whole body; twelve accesses off the parameter register, four at displacement 0xC and eight at displacement 0x14; and the trailing `j .L800787F8` entry into a bottom test, i.e. the goto-form loop the candidate body already spells. The derivation reproduces the inherited body's structure exactly — there is no second reading of these bytes.

- [s8] THE RE-DERIVED BODY IS CODEGEN-IDENTICAL TO THE INHERITED ONE. The one structural freedom the derivation exposes — giving the single gp-load its own named local (`s32 *tbl = D_800A3610; s.a = tbl[0xF]; var_s0 = tbl + 5;`) instead of two source-level mentions of the global that cse has to fold — was built as tmp/grind/func_80078654/s8/x2.c and measured with s5/eval.sh: 116 insns (== target), 38 diff lines (the same twelve $s0/$s1 exchange as the base), and an allocno table identical up to pseudo renumbering (arg0 pseudo 72 nrefs=13 livelen=98 pri=3979 -> $s0; walk pseudo 74 nrefs=5 livelen=91 pri=1098 -> $s1; zero pseudo 75 nrefs=3 livelen=170 pri=176 -> $s2). The named base local is folded away entirely. Banked as rejected/rederive-explicit-base-local-byte-identical.c.

- [s8] THE LAST UNMEASURED SPLIT SHAPE — a block-scoped PURE ALIAS of the parameter (`s32 *p = arg0;` inside block A and `s32 *q = arg0;` inside the loop, with every access spelled `p[3]`/`p[5]`/`q[3]`/`q[5]`) — was built as tmp/grind/func_80078654/s8/x1.c. This is the only split that can preserve the target's 0xC/0x14 DISPLACEMENTS: s5's sub-pointer form (`ot = arg0 + 5`) lowers the address into the pointer and emits displacement 0, which is why it printed 53 diff lines. RESULT: the alias is NOT deleted by cse (unlike s7's round-trip alias) — one of the two survives as a genuine fourth call-crossing pseudo (83: nrefs 7, livelen 47, pri 2978) and takes $s0; arg0 falls to nrefs 8 / livelen 99 / pri 2424 and takes $s1; the walk pointer (5 / 92 / 1086) takes $s2; `zero` takes $s3. 119 insns against target's 116, frame 0x60 against target's 0x58, 47 diff lines. Banked as rejected/alias-halves-split-adds-fourth-callee-save.c. The measured law is general and matches every earlier split (s2's block-scoped split at pri 4666, s5's ot form at 3176): the half that is peeled off is always SHORTER-LIVED at nearly the same reference count, so it always outranks both pointers and always takes $s0, and it always costs a fourth callee-save.

- [s8] THE JOINT QUADRANT OF THE s3 2-D FRONTIER — lower arg0 AND raise the walk pointer at the same time, the one combination s3 named and no session had built — is now measured. tmp/grind/func_80078654/s8/x3.c applies the alias split (arg0 13 -> 8 references) together with the s2/H4 base merge (walk 5 -> 8 references) in one body. RESULT: the two pointers land in an EXACT PRIORITY TIE — pseudo 72 nrefs=8 livelen=99 pri=2424 and pseudo 73 nrefs=8 livelen=99 pri=2424 — and s6's strict tie-break decides it for the parameter (allocno_compare falls through to `return *v1 - *v2`, lower allocno index first, and the parameter's pseudo is created by assign_parms before any local's). The alias half (2978) still sorts ahead of both and takes $s0, so the final order is 81 -> $s0, 72 -> $s1, 73 -> $s2, 74 -> $s3: 119 insns, 53 diff lines. The joint quadrant is dead, and it is dead in the most instructive way available — the two axes converge on a tie the parameter wins by construction. Banked as rejected/joint-alias-split-plus-base-merge-priority-tie.c.

- [s8] THE FORCED-DECOMPOSITION THEOREM (the rederive modality's product; it discharges the s6/s7 proof obligation from the TARGET BYTES rather than from the compiler's pass order). (1) The target's callee-save census is exactly three resident values — $s0 seven mentions, $s1 fifteen, $s2 five, $s3-$s7 zero (s6). (2) All twelve buffer accesses are emitted as `12($sN)` / `20($sN)`, i.e. as displacements off a register holding arg0's own value; routing any of them through a derived pointer emits displacement 0 instead (s5, measured). So each of the twelve must be a direct dereference of a pseudo holding arg0's value. (3) Every one of the twelve is separated from at least one other by a call, so any pseudo carrying a proper subset of them is live across a call and needs its own callee-save — measured twice this session (X1 and X3 both add $s3 and grow the frame 0x58 -> 0x60) and twice before (s2's block split, s5's ot form). A split therefore contradicts (1). (4) Hence in EVERY compile that emits the target bytes, one pseudo carries all twelve accesses plus the parameter home: reg_n_refs = 13 exactly, live_length <= 116 and measured 98, priority 3979. (5) The walk pointer symmetrically has 5 mentions ($s0 appears in four insns, one of them a use+set), free references are impossible (s6: counts pinned from RTL expansion to RA; s7: no local_alloc transfer, no allocno sharing), and 8 references is the ceiling for any dataflow that emits the target's SINGLE gp-load of D_800A3610 (H4) — priority <= 2448 < 3979, with ties lost to the parameter's lower allocno index. Therefore no pure-C decomposition of this function can present the allocno ordering the target's allocation requires. The rederive modality is CLOSED.

- [s8] ENDGAME GATE 1 PRE-MEASURED for whoever files the escalation: `python3 tools/scan_hand_coded.py --single func_80078654` returns tier=LOW, score 0/8 — S1 multu pacing 0 pairs, S2 no empty-body branches, S3 17 spills over 116 insns / 10 distinct registers, S4 max load burst 3, S5 no high-similarity siblings (jaccard < 0.5), S6 no BIOS jumptable pattern, S7 all callee-save uses have $sp saves, S8 no redundant mask-before-shift. The canonical-asm gate therefore FAILS on evidence, not on presumption. Gate 2 (a citable SOTN-master precedent for an RA-coercion family) has never been found by any session on this function; the sanctioned families that exist (duplicated-statement-into-arms, dead-store/self-assign, constant-holder locals, pointer alias) are all measured INERT or byte-materializing here (s3, s6, s7, s8).

- [s8] Floor re-measured at 19 THIS session with candidate.c applied to src/text1b_b.c (sandbox --disable all: score 19, target_insns 116, build_insns 116, rules_dropped 6, cheat_asm_stripped 89). src at session start again carried the stale inherited `s32 v;` + INLINE_MOVE_ALIASING body, since the s2..s7 ledger commits are ledger-only; it was re-applied before any measurement.

- [s8] The hand re-derivation from asm/funcs/func_80078654.s accounts for all 116 target instructions: frame 0x58; S78654 descriptor at sp+0x18..0x43 (a=0x18, b=0x1C, c=0x20, e=0x28, f=0x2C, g=0x30, h=0x34, cd_flag=0x40, b_/g_/r=0x41/0x42/0x43, with 0x24 and 0x38/0x3C never touched); ONE gp-relative load of D_800A3610 into $v1 feeding both `lw $v0,0x3C($v1)` and `addiu $s0,$v1,0x14`; that walk-pointer definition sits in the DELAY SLOT of the block-A skip branch, so the walk pointer's live range provably spans the whole body in the target itself; twelve parameter accesses, four at displacement 0xC and eight at 0x14; and `j .L800787F8` entering a bottom test (the goto-form loop).

- [s8] x2 (rederived body with the gp-load in its own named local `tbl`): 116 insns == target, 38 diff lines, allocno table identical to base up to renumbering (72 arg0 13/98/3979 -> $s0; 74 walk 5/91/1098 -> $s1; 75 zero 3/170/176 -> $s2). The named base local is folded away by cse; the re-derivation is codegen-identical to the inherited body.

- [s8] x1 (block-scoped pure aliases `s32 *p = arg0;` / `s32 *q = arg0;`): the alias SURVIVES cse and becomes a fourth call-crossing pseudo - ord=3 pseudo=83 nrefs=7 livelen=47 pri=2978 -> $s0; arg0 72 nrefs=8 livelen=99 pri=2424 -> $s1; walk 73 nrefs=5 livelen=92 pri=1086 -> $s2; zero 74 nrefs=3 livelen=172 pri=174 -> $s3. 119 insns vs 116, frame 0x60 vs 0x58, 47 diff lines.

- [s8] x3 (alias split + H4 base merge together, the s3 joint quadrant): pseudo 72 and pseudo 73 land at an EXACT tie, both nrefs=8 livelen=99 pri=2424; the parameter wins the tie on allocno index (allocno_compare's `return *v1 - *v2`), and the alias half (81, pri 2978) takes $s0 regardless. Order 81 -> $s0, 72 -> $s1, 73 -> $s2, 74 -> $s3; 119 insns, 53 diff lines.

- [s8] General law for splits, now measured four times on this function (s2 block split pri 4666, s5 ot sub-pointer 3176, s8 x1 2978, s8 x3 2978): the peeled-off half is always shorter-lived at nearly the same reference count, so it always outranks both pointers, always takes $s0, and always adds a fourth callee-save that the target's three-callee-save census forbids.

- [s8] Displacement argument (from the target bytes): all twelve buffer accesses are `12($sN)` / `20($sN)`, so the holding pseudo carries arg0's own value; a derived sub-pointer lowers the offset and emits `0($sN)` instead (measured in s5's ot form, 53 diff lines). Combined with the split law, this forces all twelve onto ONE pseudo, i.e. reg_n_refs(arg0) = 13 in every conforming compile.

- [s8] Endgame gate 1 is pre-measured for whoever files the escalation: `python3 tools/scan_hand_coded.py --single func_80078654` returns tier=LOW, score 0/8 (S1 0 multu/mflo pairs, S2 no empty-body branches, S3 17 spills over 116 insns / 10 distinct registers, S4 max load burst 3, S5 no high-similarity siblings, S6 no BIOS jumptable pattern, S7 all callee-save uses have $sp saves, S8 no redundant mask-before-shift). The canonical-asm gate fails on evidence, not on presumption.

- [s8] Gate 2 (a citable SOTN-master precedent for an RA-coercion family) remains unfound after eight sessions; the sanctioned families that do exist (duplicated-statement-into-arms, dead-store/self-assign, constant-holder locals, pointer alias) are all measured INERT or byte-materializing on this function (s3 duplication +2/+39 insns, s6 V1 zero yield / V2 +2 insns per reference, s7 alias round-trip zero yield, s8 x1 fourth callee-save).

## SESSION 9 (escalation) — 2026-08-13

**Base re-measured.** candidate.c applied over src/text1b_b.c (HEAD still
carries the inherited inline-asm body that scores 23; the s2-s8 ledger commits
are ledger-only). `sandbox func_80078654 --disable all` =>
`{"score": 19, "target_insns": 116, "build_insns": 116, "scorable": true,
"rules_dropped": 6, "cheat_asm_stripped": 89}`. Ninth consecutive session at
floor 19.

**What holds the byte match on main.** Six regfix rules, zero asmfix rules
(regfix.txt:2556-2566): one register swap `func_80078654: $16 <-> $17`, four
prologue/epilogue save-slot substitutions (`sw $17,72`->`sw $17,76` @1,
`sw $16,76`->`sw $16,72` @9, `lw $16,76`->`lw $16,72` @109,
`lw $17,72`->`lw $17,76` @110) and one `reorder 110,109 @ 109-110`. Every one
of the six is paperwork for the SAME single fact: the two callee-saves are
swapped relative to target. There is no second defect anywhere in the function.

**The RMW-window-only split — the last un-built partition — is KILLED.**
Form: block-scoped PURE aliases of the parameter confined to the call-free tail
RMW window (`p = arg0; p[5] = p[5] + 0xC;` in block A, `q = arg0; q[5] = q[5] +
0xC;` in the loop), pure alias rather than `&arg0[5]` so the target's 0x14
displacement is preserved. Measured with the s5 one-call instrument
(`s5/eval.sh .../s9/r1.c r1`).

    ALLOCDBG ord=3 pseudo=72 hardreg=16 nrefs=13 livelen=98 pri=3979   ($s0)
    ALLOCDBG ord=4 pseudo=73 hardreg=17 nrefs=5  livelen=91 pri=1098   ($s1)
    insns: mine=116 target=116  difflines=38

Byte-identical to the base allocno table and the base diff. cse deletes a
call-free pure alias outright, so it never reaches register allocation and
moves zero references off the parameter.

**Why this closes the theorem.** s8's forced-decomposition theorem rested on
one inductive premise — that every pseudo carrying a proper subset of the twelve
buffer accesses is live across a call and so costs a fourth callee-save. That
premise is now deductive, because the alias axis is closed at BOTH endpoints
with nothing in between:
  * call-CROSSING pure alias (s8a): survives cse, becomes a fourth
    call-crossing pseudo (7 refs / 47 live / pri 2978) that takes $s0 itself;
    119 insns, frame 0x60 vs 0x58, 47 diff lines.
  * call-FREE pure alias (s9, this session): deleted by cse before RA; zero
    references moved, allocno table unchanged.
An alias cheap enough to avoid a callee-save is deleted before it can lower
reg_n_refs; an alias that survives to RA is expensive enough to need one.

**Gate 1 (canonical asm) — FAILS.** `python3 tools/scan_hand_coded.py --single
func_80078654`:

    HAND_CODED: tier=LOW  score=0/8  (func_80078654, 116 insns)
      Reason: no strong hand-coded indicators
      [ ] S1 multu pacing   0 multu/mflo pairs
      [ ] S2 empty branch   no empty-body branches
      [ ] S3 no spills      116 insns, 17 spills, 10 distinct regs
      [ ] S4 front loads    max load burst 3 in any 8-insn window
      [ ] S5 cluster        no high-similarity siblings (jaccard < 0.5)
      [ ] S6 BIOS jumptable no BIOS jumptable call pattern
      [ ] S7 unsaved $sN    all callee-save uses have $sp save
      [ ] S8 redundant mask no redundant mask-before-shift

The STRONG tier needs S1/S2/S6; none fires. Per
.claude/rules/endgame-lock-disposition.md a LOW score is dispositive, and a
register-allocation tiebreak is ordinary compiler output by definition.

**Gate 2 (SOTN precedent for a coercion family) — FAILS.** No precedent can be
exhibited because no closing CONSTRUCT exists to seek one for. Nine sessions
across six modalities measured every sanctioned family this CFG admits and each
leaves allocno_compare's ordering intact: duplication-into-arms raises the
parameter 1.5x faster than the walk pointer (5671 vs 2125 at k=2) and is not
byte-neutral here; the base/walk merge tops out at 2448 against 3979; the
sub-pointer arg0-lowering form forces a fourth callee-save; the joint quadrant
produces an exact 2424/2424 tie that global.c's strict `*v1 - *v2` tie-break
resolves in the parameter's favour; and the window alias above is deleted
before RA. A census that comes back negative is a failed gate, not an open
question.

**Disposition applied.** Both gates fail, so the owner's standing ruling
(2026-07-27) is applied immediately with no owner wait: entry filed at
docs/grind/decisions.md as `OWNER-ESCALATION — RESOLVED BY STANDING RULING
(2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE`. src/text1b_b.c was reverted
to HEAD so the six regfix rules keep holding the byte match and the full-build
oracle stays green, exactly as that ruling's disposition clause requires. The
retained rules are NOT sanctioned as a technique; they survive only to hold the
match, and the function is openly flagged unresolved and remains eligible for
re-attempt if a genuine new pure-C lever or new tooling emerges.

- [s9] Base re-measured this session with candidate.c applied over src/text1b_b.c: sandbox func_80078654 --disable all = {"score": 19, "target_insns": 116, "build_insns": 116, "scorable": true, "rules_dropped": 6, "cheat_asm_stripped": 89}. Ninth consecutive session at floor 19; the build is instruction-exact and the whole residual is one two-way callee-save inversion.

- [s9] What holds the byte match on main: six regfix rules and zero asmfix rules (regfix.txt:2556-2566) — `func_80078654: $16 <-> $17`, four prologue/epilogue save-slot substitutions (sw $17,72->sw $17,76 @1; sw $16,76->sw $16,72 @9; lw $16,76->lw $16,72 @109; lw $17,72->lw $17,76 @110) and `reorder 110,109 @ 109-110`. All six are paperwork for the SAME single inversion; there is no second defect in the function.

- [s9] GATE 1 evidence (verbatim tool output): `HAND_CODED: tier=LOW score=0/8 (func_80078654, 116 insns) / Reason: no strong hand-coded indicators` with S1-S8 all unchecked. The STRONG tier needs S1/S2/S6; none fires.

- [s9] GATE 2 evidence: no SOTN-master (or VS/ESA) precedent is exhibited and none can be, because six modalities across nine sessions have failed to identify ANY closing construct for which a citation could be sought. The blocking mechanism is measured exactly — global.c allocno_compare sorts by floor_log2(n_refs)*n_refs/live_length, giving the parameter 3979 (13 refs / 98 live) against the walk pointer's 1098 (5 refs / 91 live, ceiling 8 refs / 2448), and find_reg hands out free callee-saves by ascending first-fit.

- [s9] New this session: the RMW-window-only alias split is deleted by cse before register allocation — allocno table byte-identical to the base (pseudo 72: 13 refs / 98 live / 3979 / $s0; pseudo 73: 5 refs / 91 live / 1098 / $s1), 116 insns, 38 diff lines, zero references moved off the parameter.

- [s9] Closing the axis: a call-CROSSING pure alias (s8a) survives cse but becomes a fourth call-crossing pseudo (7 refs / 47 live / pri 2978) that takes $s0 itself at 119 insns and frame 0x60 vs 0x58; a call-FREE pure alias (s9) is deleted before RA. There is no middle, so s8's forced-decomposition theorem premise (3) is now deductive rather than inductive.

- [s9] Exhaustion record: 9 sessions; 6 distinct modalities (recon, structural x2, permuter x2, forensics x2, rederive, escalation); 129,034 permuter iterations across three campaigns with ZERO score-improving finds; a per-insn provenance table proving both pseudos' reference counts are pinned from RTL expansion through RA with no post-RA reference; a full hand re-derivation from asm/funcs/func_80078654.s accounting for all 116 target instructions and converging on the current body; 14 disproven forms banked in memory/grind/func_80078654/rejected/.

- [s9] Disposition applied per the standing ruling: src/text1b_b.c reverted to HEAD so the six regfix rules keep holding the byte match and the full-build oracle stays green. The retained rules are NOT sanctioned as a technique — they survive only to hold the match, and the function is openly flagged unresolved and remains eligible for re-attempt. The best honest pure-C form (floor 19, instruction-exact, zero inline asm — materially better than HEAD's inherited floor-23 inline-asm body) is preserved at memory/grind/func_80078654/candidate.c.
