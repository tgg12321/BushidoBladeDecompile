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

## 2026-10-02 — shared GPU interfaces, manual continuation

Prerequisite type repair, not completion of func_80048FFC or camera_CalcAngles (both INCLUDE_ASM).

- DR_MOVE is the PsyQ layout `{ u32 tag; u32 code[5]; }` (0x18). SetDrawMove(DR_MOVE *, RECT *, u32,
  u32) writes the length through the SDK setlen P_TAG view (SOTN include/psxsdk/libgpu.h:87, PS1 use
  sys.c:287) and packs the RECT as MoveImage does (LOW(rect->x)/LOW(rect->w), sys.c:275/277, all
  @db41b28). Component-wise packing measured 23/31.
- Strides from the code: light_effect_col[31][2] (0x5D0 from 0x800A3D70), D_800A4340[19][2],
  D_800A9830[2][10] (two 0xF0 banks). D_800A9920 (= D_800A9830+0xF0) was a second handle; it is
  removed and gpu_AddDrawMove tests the bank's one-past bound.
- D_800A378C is the SDK OT pointer (u32 *, OT_TYPE); definition and consumers agree. g_gpu_ot_ptr is
  `u8 *` in text1b only (its definition, ings.c); other TUs keep their older `s32` externs.
- First layer-2 (2026-10-02) FAILed five bodies; fixes, each re-measured 0: func_8003DBE4 walks a
  `DR_MOVE *` cursor (`pkt = *arg2; pkt += parity; ... pkt += 2`) instead of a FAKE u32 storage view
  with integer stepping; gpu_AddDrawMove has its real two parameters (callers pass two; the
  `(void (*)())` call casts are gone); gpu_SetDrawMoveArray walks a2 (no parameter copy);
  func_8003D91C uses literals (no constant holders); SetDrawMove's SOTN lines corrected.
- Second layer-2 FAILed func_8003DBE4 (integer OT-entry address kept only for its `+` operand
  order; constant holder rgb_mask; no-op (s32) casts) and SetDrawMove (duplicated `size = 0` arm).
  Fixed: SetDrawMove `if (w == 0 || h == 0)` 0/24. func_8003DBE4 now links with the SDK addPrim
  shape, `setaddr(pkt, getaddr(&ot[idx])); setaddr(&ot[idx], pkt)` written out on OTag views
  (libgpu.h:88), literal masks from the bitfield: 0/133. Same with an `OTag *ot = &...[idx]` local
  3 (a0/v0 swap at target[75]); `ot = (OTag *)D_800A378C; ot[idx]` 0; integer forms 3; literal-mask
  u32 forms 5. Third layer-2: on this body the F6 pair is no longer needed. The two-arm
  `step = 0x6590 - arg0;` / `step = 0x55F0 - arg0;` (and the ternary) score 0/133 with an empty
  unmasked diff, so the pair is gone. The 3/4/13 ordinary-form receipts in tools/interfaces/
  (ordinary_consumer.*, ablations.json) were measured on the pre-cursor body and are stale. The
  inherited dead conditional store to step and the tmp/limit-1 reuse are removed.
- func_80048FFC on the repaired types: F6 cluster 4/232 (spelling the OT entry `ot =
  (OTag *)&D_800A378C[0xFFF]`); plain 13. Residual unchanged: the old-phase copy/shift is scheduled
  before call two's a1-a3 setup. sched1 (bottom-up, LUID tie-break, all priority 1) keeps the
  arg setups below the copy/shift, so sched2 inherits that order. Swept all 2240 orders of
  {old=phase, phase>>=1, other halvings, i++, addPrim 1, rect.y, rect.h} between the calls (halvings
  and i++ also after call two): best 4 (168 orders), none lower. Halvings written after call two are
  hoisted above it by sched1 (the post-call addPrim chain outranks them): also 4.

## 2026-10-02 — after the interface landing (e558afab7); manual session (Claude)

Interface step landed (e558afab7, fourth layer-2 PASS on 22 bodies). Candidate on the landed types:
rejected/f6-typed-floor4.c (F6 cluster, OT link `ot = (OTag *)&D_800A378C[0xFFF]` + setaddr views),
4/232; plain 13. The one scored hunk is `move t1,s7; sra s7,s7,1` above, not below, call two's
a1/a2/a3 setup.

Mechanism (dumps of the instrumented cc1, `.sched`/`.sched2`, RANKDBG): in sched1 the copy (old =
phase), the shift and the three arg setups are all priority 1 (no in-block load feeds them), so
rank_for_schedule falls to LUID and the arg setups (born at the call) are placed lowest. In sched2
they are all priority 6 and LUID again keeps sched1's order. For the target order, the copy/shift
must be deeper than the arg setups in sched1, or have a higher LUID. Neither holds for any source
shape measured so far.

Killed (all on the F6 chassis unless noted):
- Order sweep, tmp-side generator (perm.py, receipts not banked): every order of {old=phase,
  phase>>=1, other halvings, i++, addPrim 1, rect.y, rect.h} between the calls, with halvings and
  i++ also after call two, 2240 orders: best 4 (168 orders tie).
- Birth/shape: `phase = old >> 1`; `s16 old`; old at the loop top; old outside the loop; shift last
  among the halvings; shift after call two (no copy: 52-53, 231 insns); `next = phase >> 1` with
  `phase = next` before or after call two (53); `rect.h = phase` early or late (53-54); F6 pair on
  phase (21) or on old (4); `cy += dy` (17); rect.h before rect.y (6).
- OT link spellings (addPrim views with and without a local; `((OTag *)D_800A378C)[0xFFF]`): 4
  with F6, 13 plain.
- decomp-permuter, campaign f6c-typed (typed base, -j 8, 60k iterations, ~28 min): no find below
  base.
- cc1psx on the same body: also emits the copy before the arg setup (and schedules the halvings
  worse), so the gap is on the source side.

Frontier: a source structure that makes the copy and shift deeper than the call-two argument
setups in sched1. One option is a true dependence on an in-block load. Another is a different
basic-block or loop-note structure around the second strip. Neither is identified yet.

## 2026-10-02 — oct2-a1 (camera_CalcAngles's Q89 prerequisite) — the sched tie, and the next batch of variants

Re-baseline on main (typed interfaces): F6 chassis 4/232, plain 13 at -G0; the same at -G8 (variants-oct2/
g8fn.sh with G=8: the head part's -G8 does not touch this function). Mechanism (sched.c 2.7.2):
- priority() runs over LOG_LINKS (predecessors), and mips ADJUST_COST makes anti/output deps cost-free.
- In sched1, the copy (prio(call one)), the shift (max of its anti preds: the pre-call phase readers and the
  copy) and the arg moves (output deps on call one) all equal prio(call one) = 1. Nothing before call one is
  a load at sched1.
- rank_for_schedule falls through to LUID, and the higher LUID is placed lower.
- calls.c emits the hard-reg moves after all argument expressions (emit_queue only adds post-increments
  after them).
- So the shift, which must sit between the copy and the rect.h store or cse/combine kills the copy
  (231 insns), always lands above the moves.
- The copy is single-set, so adjust_priority launches it right above the shift.
- sched2 ties again at prio(call one) = 6 (the loop's spill chains) and keeps sched1's order.
Variants (variants-oct2/gen48.py on ffc_f6.c), with diff lines vs the target: x_a 4, x_b 53, x_d 93, x_e 78,
x_f 4, x_g 56, x_h 4, x_j 4, x_k 4, x_l 53.
- x_a: copy+shift inside the rect.h store (comma expression).
- x_b: shift inside the call's argument.
- x_d: RECT pointer for the second strip.
- x_e: destination recomputed from x2/x2f, y2/y2f, halved after call two.
- x_f: halvings and i++ after call two.
- x_g: second strip's rect stores before addPrim one.
- x_h: pre-increment p in the call.
- x_j: s16 copy.
- x_k: copy+shift right before call two.
- x_l: `phase = old >> 1` after the store.
The policy question (the Q89 cut before this function) is logged in docs/grind/borderline.md 2026-10-02;
the orchestrator refused it under its delegation. Frontier: a source construct that emits the copy or the
shift after call two's argument moves — none is known in GCC 2.7.2 beyond queued post-increments.

## 2026-10-02 — oct2-a1 BANKED: frontier

Floor 4/232 (F6 chassis, rejected/f6-typed-floor4.c); plain 13. The one hunk is decided by a sched1 LUID tie,
mechanism above. Every spelling found so far emits the copy/shift before call two's argument moves.
The frontier is a construct whose copy or shift is emitted after those moves. In GCC 2.7.2 the only known
emitter there is emit_queue (queued post-increments), and neither insn is an increment. A second route
would give the copy a costly true predecessor in sched1, without an extra instruction or a register
shared with addPrim's temporaries.
Not tried yet:
- a different loop skeleton: the two strips as separate loops, or a peeled first iteration;
- typing the call two arguments through a struct;
- a permuter campaign seeded with variants-oct2/x_*.c.
Q89 depends on this function; the policy question about a cut before it is in docs/grind/borderline.md
2026-10-02.
