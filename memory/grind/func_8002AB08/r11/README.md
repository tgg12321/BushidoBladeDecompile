# func_8002AB08 — Ruling 11 package (laneC, 2026-10-01)

Ruling: `.claude/rules/reused-local-necessity.md` § Ruling 11. Body measured: `memory/grind/func_8002AB08/candidate.c`
(sandbox --disable all = 0). NOTE: the record reads still use the TU-local stand-in view `R8002AB08`; the landing
respells them on `PracticeMenuRec` (orchestrator, 2026-10-01) and this package is RE-RUN on that exact body
(`dumpall.sh`, `run.py`) before READY_FOR_REVIEW. Record typing does not change allocation (laneB measured the
same on func_80058580, its r11/README.md).

Reused locals (9): `dx`, `dy`, `dz`, `temp1`, `temp2`, `temp3`, `idx`, `alt`, `work`. Why Ruling 11: the
values feed different consumers through different templates (not Ruling 5), are not one record pointer
per region (6), not `VAR = BASE + K` (9), no public original source (10), no SOTN function reuses a
variable in these roles (Q51 needs the same roles at corresponding statements).

## Values (`roles.py <cand> list`; occurrence indices exclude comments; 0 = declaration)
- dx / dz: `seg` segment delta (pass loop), `hit` contact delta (22-loop), `off0` / `off1` offset of the
  blade's two points from the opponent (knockback, three branches), `dir` blade direction (incl. the
  negation), `push` scaled push added to the nudge. dy: `seg`, `hit`.
- temp1 / temp2: `pt0` / `pt1` the two point indices of a pass (per-branch constants 0/0/1 and 1/1/2,
  Q20), `dsq0` / `dsq1` squared distance of the opponent to each point (knockback).
- temp3: `hn` the 22-contact counter, `side` hit-on-the-alternate-blade flag (knockback).
- idx: `seg` triangle index of the segment loop, `nearest` index of the nearest contact.
- alt: `blade` pass selector (0 = primary 0x210 / unk00 points, 1 = alternate 0x234 / unk48), read stale by
  the knockback's velocity index exactly as the target does (fp, 0x8002B8C0); `hitalt` the same meaning
  recomputed for the hit (`(mask_a & (1 << idx)) != 0`).
- work: `lensq` segment length^2, `dist` contact distance minus radius, `ang` facing difference, `weight`
  push weight (`0x400 - ang`, clamped at 0).
Every value has a real computation (load / arithmetic / call result); temp1/temp2 `pt*` are Q20 per-branch
constants. The min/max loop counter was a temp3 value in s2; its split matched (scores.txt r_mm, s2), so it
is its own local `j` (not reused).

## (C)(2) statement lists
`stmtcheck.py <cand> <twin>`: every twin (ab/r_*.c single-value ablations, pv_<var>.c per-variable splits,
pv_all.c) is `IDENTICAL statement lists` with the reuse body (38/38).

## (D)(4) measurements (scores.txt; engine sandbox --disable all; reuse body = 0)
| split | score | | single value | score |
|---|---|---|---|---|
| pv_all (every value fresh) | 524 | | dx seg/hit/off0/off1/dir/push | 14/4/180/180/393/12 |
| pv_dx | 266 | | dy seg/hit | 51/51 |
| pv_dy | 51 | | dz seg/hit/off0/off1/dir/push | 108/153/152/153/381/6 |
| pv_dz | 255 | | temp1 pt0/dsq0, temp2 pt1/dsq1 | 22/8, 16/9 |
| pv_temp1 / pv_temp2 | 22 / 16 | | temp3 hn/side | 192/192 |
| pv_temp3 | 192 | | idx seg/nearest | 131/131 |
| pv_idx | 131 | | alt blade/hitalt | 118/107 |
| pv_alt / pv_work | 118 / 9 | | work lensq/dist/ang/weight | 2/3/4/9 |
Structural respellings (scores_struct.txt): side tested inline 197; hitalt inline 123; point indices u8 36;
weight s16 20; nearest u8 129. Permuter campaign from pv_all (perm_setup.sh workspace, -j2, ~20 min, 370
iterations, permuter_harvest.json): base 14699, best find 12734 (a retype of a split value); the reuse body
scores 30 on the same scorer (relocation/sdata residue; 0 in the engine sandbox).

## (D)(1)/(2) dumps and mechanism
`dump.sh <tag> <body>` / `dumpall.sh`: splice over the INCLUDE_ASM in src/code6cac_b_tu2.c, build cpp,
`tools/decomp-permuter/strip_other_fns.py`, build cc1 `-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1
-mno-abicalls -fno-builtin -w -mel -msoft-float -dr -dl -dg`, then the instrumented `tools/gcc-2.7.2/cc1`
with `BB2_ALLOC_DEBUG=1`; the two compilers' asm is compared (IDENTITY OK for all 11). Banked:
`alloc_<tag>.txt` (ALLOCDBG), `regs_<tag>.txt` (.lreg Register lines); `table.txt` (table.py) maps each
value to its allocation. Pseudo map (reuse body): 85 alt, 86 temp1, 87 temp2, 90 idx, 91 dx, 92 dy,
93 dz, 95 temp3, 103 work; in a twin the fresh values are 106.. in `roles.py` order.
Mechanism (tools/gcc-2.7.2): flow.c counts refs / live length / calls crossed (flow.c:2081);
local-alloc.c:472-475 gives single-block, single-death pseudos to local allocation; global.c:575 sorts the
rest by allocno_compare; global.c find_reg :970-975 lets an allocno that crosses a call take only
call-saved registers; reload1.c alter_reg gives each unallocated pseudo its own frame slot.
- dx / dz (target $s0 / $s1 in all regions): reused, one allocno each crossing the knockback's ratan2
  calls -> call-saved $s0 / $s1. Split: seg/hit are local-alloc'd, off0/off1/push take $v1/$a1/$a0/$a3,
  only dir crosses calls ($s3).
- dy (target $a3 in both regions): reused, one global allocno -> first free $a3; split, both values are
  single-block and local-alloc'd to other registers.
- temp1 / temp2 (target frame slots 0x50 / 0x58, holding the indices in the pass loop and the squared
  distances in the knockback: `sw t2,0x50(sp)` / `sw a3,0x58(sp)` at 0x8002B79C / 0x8002B7AC): reused, one
  unallocated pseudo each -> one slot; split, the distances get $v1/$a0 and are never stored.
- temp3 (target $s4: 22-contact counter and side flag): reused, crosses the ratan2 calls -> $s4; split,
  the counter takes $a2, the flag $s6.
- idx (target $s5 for both loops' index): reused -> $s5; split, the nearest index is spilled to memory.
- alt (target $fp): reused -> $fp; split, blade is spilled to memory and hitalt takes $s0.
- work (target $a1 throughout): reused, one global allocno -> first free $a1; split, lensq is local-alloc'd,
  dist and ang take $v1.
