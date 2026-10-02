# func_80048FFC (0x80048FFC) — evidence

## 2026-10-01 (laneA) — unparked as the first prerequisite of owner ruling Q89 (camera_CalcAngles split)

candidate_region.c replaces src/text1b.c from `extern u8 D_800EF848[];` through this function's INCLUDE_ASM
(func_80048F58 respelled on the record type, byte-identical: score 0). It needs `#include "gpu.h"` in text1b
(OTag, RECT; compiles clean). Scored with tools/ (mk.py splices, fsb.sh/sweep.py build + score with every
rule file empty, the sandbox's honest mode).

Data model (from the code): D_800EF848 is an array of 0x134-byte records — +0 s32 phase, +4 two banks
(frame parity, D_800A36AC & 1) of six PsyQ DR_MOVE packets (0x18 each: three levels x two strips),
+0x124 seven control halfwords copied from D_80099C34 by func_80048F58 (x, y, w, period, dest x, dest y,
step). The loop builds a wrap-around vertical scroll of a VRAM rectangle: strip 1 copies rows
[y, y + period - phase) to dest y + phase, strip 2 rows [.., +phase) to dest y; every level halves the
fractions, w, period and phase. addPrim onto ot[0xFFF] of the OT at D_800A378C (OTag bitfield form).

Floor: 13 (232 = 232 insns). Residual: the loop's four call-saved temporaries are permuted
(target ny s0, nx s1, cy s2, dy s3; ours cy s0, ny s1, nx s2, dy s3) plus one sched2 order hunk.

Deciding pass (BB2_QTY_DEBUG, tools/dump.sh): local-alloc of the loop block (blk 9). qty priority =
floor_log2(refs)*refs/(death-birth): cy+sum (tied) refs 10 len 66 = .4545 beats ny 6/42 = .286 and
nx 6/44 = .273, so cy takes s0. Diagnostic (tools/diag_refs.py, `diag_use_after`, NOT a candidate): one extra
reference to nx and ny after the first call (empty asm use) gives ny/nx refs 8 (.571/.545) and the exact
target allocation; only the `move t1,s7; sra s7` order hunk is left (score 4). The extra-reference
diagnostic establishes one allocation mechanism; it does not prove that the original
source contained those references. A different source structure could yield the same allocation.
cc1psx on the same body makes the same allocation as ours (source-side, not a compiler divergence).

Measured without effect (all 13 unless noted): order of the four temporaries' definitions (24 perms) and
of the rect stores (24 perms; natural order best), position of every post-call statement (360 perms of
old/halvings/i++/addPrim/rect.y/rect.h), for/while/do loops, inline helper for x2+x2f, statement
expression, split assignments, args as expressions with nx/ny assigned after the first call (cse keeps
the arg temporary canonical, copy dies), assignment inside the argument list, operand commutations
(13-17), s16 temporaries (no change), s16 slot variables (halfword spills, 173-176), do-while(0) (110),
inline SetDrawMove+addPrim helper (76), permuter 3 x 9.5 min (randomization and assignment-weighted
passes, base 175, no improvement).

Open data-model debts for the landing: D_800A378C is `s32` in text1b (and defined `s32` in text1a_c) but
is the OT pointer; this body needs it as a pointer (`(OTag *)D_800A378C` is the int->pointer pun the
checklist refuses). All eight value-preserving `(s16)` casts on ctl arithmetic were removed in the manual session below;
the same 13/232 floor remains. The u16-ctl alternative was previously measured at 27.

### Why the residual is a reference count, not an order (sched1 / local-alloc, BB2_RANK_DEBUG + BB2_QTY_DEBUG)
- Lengths cannot flip it: sched1 (bottom-up) ties call 1's a0-a3 setups with the rect stores (same priority,
  class 3) and breaks the tie by LUID, so the setups (emitted at the call, highest LUIDs) always land at the
  block top and nx/ny are born first (birth 8/12). Every statement order measured gives the same quantities.
  With cy's tied quantity at refs 10, nx/ny would need lives under 27 units — not reachable.
- cy cannot drop: the sum ties to cy (cy dies there), so cy's quantity is 5 weighted occurrences in every
  spelling with a register sum; without the tie the sum takes a call-clobbered register, not s2.
- Extra references are one demonstrated route to the target allocation, not proof of the original source
  spelling. The diagnostic empty asm use supplies the measured 8-reference quantities; a dead
  pre-loop initializer plus call-1 expression args with nx/ny assigned after the call (probe, a dead-store
  device) does not (13: cse still keeps the argument temporary canonical).
- cc1psx on candidate_region.c allocates exactly like ours (cy s0, ny s1, nx s2): source-side.
- do-while(0) (sanctioned family 7, loop-note ref weighting): every single-level wrap of a contiguous
  statement range of the loop body (378 ranges, tools/sweep_dowhile0.py) — best 15 (233 insns); the loop
  notes act as sched1 barriers and cost instructions before they can reweight nx/ny. s16 cy/dy/old: 13-15.

Frontier: identify a truthful source shape or an admissible annotated construct reaching the target
allocation and schedule. The diagnostic reference counts do not prove the original source spelling.
Also owed before landing: a truthful shared D_800A378C pointer and SetDrawMove interface. The redundant
width casts are avoidable without changing the floor, as measured below.

## 2026-10-01 — Codex manual dependency session (checkpoint only)

Main stayed at INCLUDE_ASM. Re-baseline: plain region 13/232; deleting all eight ctl result casts
preserves 13/232. rejected/castfree-plain-floor13.c carries the simpler form; it remains rejected
on the shared-interface debts. The 256 selective s16-local
combinations (casts absent) did not beat 13; all named measurements are in tools/codex-probe-receipts.json.

Frozen-family F6: one exact adjacent nx++/nx-- pair and two separate adjacent ny++/ny-- pairs after
call one reduce the codegen score to 4/232. Zero pairs: 13; one pair each: 9; x1/y2: 4; x1/y3: 4;
x2/y2: 9; x2/y3: 4. Thus x1/y2 is the smallest measured pair cluster with this floor. The entire
cluster ablates to 13. tools/plain-qty.txt and f6-qty.txt (instrumented diagnostic cc1, commands in
tools/dump.sh) show loop local-alloc: plain cy refs10->s0, ny6->s1, nx6->s2; F6 ny22->s0,
nx14->s1, cy10->s2. No instruction is added (232 each). Bounds of these s32 coordinate sums
are within [-33023,33022], so each adjacent ++/-- pair is value-neutral without signed overflow.

This is NOT a completion or an accepted checkpoint: rejected/f6-scheduling-floor4.c still has the
integer-to-OT pointer debt and a TU-local SetDrawMove signature inconsistent with its definition
and other callers. Fresh read-only checkpoint review is banked there. The remaining scored hunk is
the phase move/shift before, rather than after, call-two's a1/a2/a3 setup. Moving the pair cluster,
delaying the shift, computing next_phase, using a real first_y intermediate, and omitting old did
not close it. Completed sweep counts/results are in the receipts; compiler-error variants are
excluded from counts. The single-level loop-wrapper sweep is recorded there too.

No asm pins, empty-asm diagnostic candidates, output rewriting, flag change, source landing,
queue rotation, or completion record was made. Frontier: fix the shared packet/RECT/OT interfaces
truthfully and resolve the phase/call scheduling gap; Q89's file split remains staged behind this.

One type-repair lead is measured: tools/gpu-typed-proposal.c uses DR_MOVE/RECT fields in
SetDrawMove, with the two packed RECT reads citing matched PS1 SOTN src/main/psxsdk/libgpu/sys.c
at db41b28eee52969244a52cc269c8163d1ed8826a (MoveImage uses LOW(rect->x)/LOW(rect->w), common.h
defines LOW as a signed word read; config/splat.us.main.yaml includes that C file). It scores
0/24, only masked branch-position hunks (tools/gpu-typed-proposal.out). This is a scratch proposal,
not a shared-interface migration, not a full oracle proof, and not reviewed for landing.

Reproduction (run from repo root under WSL with .venv active):
`python3 memory/grind/func_80048FFC/tools/codex_probe.py` remeasured plain 13/232,
F6 4/232 and typed SetDrawMove 0/24 from the banked files after their annotations were finalized.
All 465 contiguous single-level wrapper ranges on the F6 body were measured; none beat 4.
