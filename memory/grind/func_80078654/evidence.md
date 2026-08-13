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
