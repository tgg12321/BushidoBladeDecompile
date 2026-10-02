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
target allocation; only the `move t1,s7; sra s7` order hunk is left (score 4). So the original source
references nx and ny once more each (weighted refs 8) without extra code — or gives them shorter lives.
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
checklist refuses). The `(s16)(ctl[i] & ~0x3F)` / `(s16)(ctl[i] % 64)` forms reproduce the target's
lhu/and/sll/sra but are value-preserving casts on s16 data; with u16 ctl the mask becomes andi (27).

### Why the residual is a reference count, not an order (sched1 / local-alloc, BB2_RANK_DEBUG + BB2_QTY_DEBUG)
- Lengths cannot flip it: sched1 (bottom-up) ties call 1's a0-a3 setups with the rect stores (same priority,
  class 3) and breaks the tie by LUID, so the setups (emitted at the call, highest LUIDs) always land at the
  block top and nx/ny are born first (birth 8/12). Every statement order measured gives the same quantities.
  With cy's tied quantity at refs 10, nx/ny would need lives under 27 units — not reachable.
- cy cannot drop: the sum ties to cy (cy dies there), so cy's quantity is 5 weighted occurrences in every
  spelling with a register sum; without the tie the sum takes a call-clobbered register, not s2.
- So the original source references nx and ny once more each (4 in-loop occurrences, weighted 8) with no
  extra code, by a construct not yet identified. Only the diagnostic empty asm use supplies it; a dead
  pre-loop initializer plus call-1 expression args with nx/ny assigned after the call (probe, a dead-store
  device) does not (13: cse still keeps the argument temporary canonical).
- cc1psx on candidate_region.c allocates exactly like ours (cy s0, ny s1, nx s2): source-side.
- do-while(0) (sanctioned family 7, loop-note ref weighting): every single-level wrap of a contiguous
  statement range of the loop body (378 ranges, tools/sweep_dowhile0.py) — best 15 (233 insns); the loop
  notes act as sched1 barriers and cost instructions before they can reweight nx/ny. s16 cy/dy/old: 13-15.

Frontier: identify the construct that gives nx and ny one more in-loop reference without code (the target's
own allocation proves it exists: tools/diag_refs.py). Also owed before any landing: D_800A378C as a real
pointer type in text1b (and its other consumers), and a decision on the `(s16)(...)` value-preserving
casts (checklist item 1: disclose with the u16-ctl measurement, 27).
