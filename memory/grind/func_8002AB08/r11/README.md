# func_8002AB08 â€” Ruling 11 package (laneC, 2026-10-01)

Ruling: `.claude/rules/reused-local-necessity.md` § Ruling 11. Body measured = the landing body,
`memory/grind/func_8002AB08/candidate.c`: records typed `PracticeMenuRec *` (members this landing adds:
s16 unk_8C, s16 unk_92, Vec4i32 unk_114[2]; `typed/hdr.py` builds the header overlay
tmp/func_8002AB08/typed/inc/include/code6cac.h on laneB's PracticeMenuRec), every per-player local declared in
the player-loop block (the innermost scope enclosing every write of each reused local, (A)). All scores below
use `typed/runh.py` (engine sandbox --disable all with the overlay ahead of include/); landing body = 0.
An earlier round on the untyped u8 * body gave identical numbers (scores.txt).

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
Value table (writes and the target instruction each one is; addresses in asm/funcs/func_8002AB08.s):
| var | value | writes | target |
|---|---|---|---|
| dx $s0 | seg | `= scr[3].x - scr[9].x` | subu 0x8002B0C8 |
| | hit | `= unkA8[i][n].x - ref.x` | subu 0x8002B4CC |
| | off0 / off1 | `= pt.x - other->unk_F4.x` (3 branches each) | subu 0x8002B6B4 / 0x8002B6EC (shared tail 0x8002B84C) |
| | dir | `= p1.x - p0.x` / `(..) / 4` / `= -dx` | subu 0x8002B84C, sra 0x8002B7B4, negu 0x8002B880 |
| | push | `= dx / 4 + (..)` | addu 0x8002B958 |
| dy $a3 | seg / hit | differences | subu 0x8002B0CC / 0x8002B4E8 |
| dz $s1 | seg / hit / off0 / off1 / dir / push | as dx | subu 0x8002B0E4 / 0x8002B508 / 0x8002B6D0 / 0x8002B840, subu 0x8002B858, sra 0x8002B7C0, negu 0x8002B884, addu 0x8002B980 |
| temp1 slot 0x50 | pt0 | `= 0` / `= 0` / `= 1` (Q20 per-branch constants) | sw 0x8002AEF0 (shared tail), 0x8002AF24 |
| | dsq0 | `= dx * dx + dz * dz` | sw 0x8002B79C / 0x8002B854 |
| temp2 slot 0x58 | pt1 | `= 1` / `= 1` / `= 2` (Q20) | sw 0x8002AEF4 (shared tail), 0x8002AF28 |
| | dsq1 | as dsq0 | sw 0x8002B7AC / 0x8002B864 |
| temp3 $s4 | hn | `= 0`, `++` | 0x8002B49C, addiu 0x8002B540 |
| | side | `= (mask_a & bit) != 0` | and / sltu 0x8002B684 / 0x8002B688 |
| idx $s5 | seg | `= 0`, `++` | 0x8002B1B8, addiu 0x8002B42C |
| | nearest | `idx = temp3;` (Q34 plain copy) | `addu $s5,$s4,$zero` 0x8002B538 |
| alt $fp | blade | `= 0` / `= 1` / `= 0` (Q20) | 0x8002AED0 / 0x8002AEE4 / 0x8002AF18 |
| | hitalt | `= (mask_a & (1 << idx)) != 0` | and / sltu 0x8002BB0C / 0x8002BB1C |
| work $a1 | lensq | `= dx*dx + dy*dy + dz*dz` | addu 0x8002B1A0 |
| | dist | `= .. - radius` | subu 0x8002B524 |
| | ang | `= (..) & 0xFFF`, `= 0x1000 - work` | andi 0x8002B8F0, subu 0x8002B904 |
| | weight | `= 0x400 - work`, `= 0` (clamp) | subu 0x8002B90C, 0x8002B918 |

**Copy-clause value (Q34, one in the function): idx `nearest`** = `idx = temp3;` (temp3, a named local,
is read again by the loop's `temp3++` / `temp3 < 22`; no cast; target `addu $s5,$s4,$zero` 0x8002B538). Every
other value of idx (`seg`) is a counter. Banked and measured: the fresh local (ab/r_nearest.c 131) and the
no-copy body (round2/nocopy_ptr.c: the nearest slot kept as a `LeafPos *` and the index recovered after the
loop, 223).

## (C)(2) statement lists
`stmtcheck.py <cand> <twin>`: every twin (single-value ablations `roles.py <cand> one`, per-variable splits
`roles.py <cand> var`, the full split `roles.py <cand> all`) is `IDENTICAL statement lists` with the landing
body (stmtcheck.txt, 38/38).

## (D)(4) measurements (scores_final.txt; landing body = 0)
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
Structural respellings (fst_*.c, scores_final.txt): side tested inline 197; hitalt inline 123; point indices
u8 36; weight s16 20; nearest u8 129. Permuter campaign from fpv_all of the landing body (perm_setup.sh workspace with the
header overlay, -j2, ~20 min, 451 iterations, permuter_harvest_final.json): base 14699, best find 12713. The
previous body's split (identical but for two casts at the func_8002A458 call): 433 iterations, best 11653, a
stale copy of the hit mask into dy_seg_ reused for mask_c (permuter_harvest_typed.json); the landing body scores 30 on the same scorer
(relocation/sdata residue; 0 in the engine sandbox). The untyped round: 370 iterations, best 12734
(permuter_harvest.json).

## (D)(1)/(2) dumps and mechanism
`dump.sh <tag> <body>` / `dumpall.sh`: splice over the INCLUDE_ASM in src/code6cac_b_tu2.c, build cpp,
`tools/decomp-permuter/strip_other_fns.py`, build cc1 `-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1
-mno-abicalls -fno-builtin -w -mel -msoft-float -dr -dl -dg`, then the instrumented `tools/gcc-2.7.2/cc1`
with `BB2_ALLOC_DEBUG=1`; the two compilers' asm is compared (IDENTITY OK for all 11). Banked:
`alloc_<tag>.txt` (ALLOCDBG), `regs_<tag>.txt` (.lreg Register lines); `table.txt` (table.py) maps each
value to its allocation. Pseudo map (landing body): 86 alt, 87 temp1, 88 temp2, 91 idx, 92 dx, 93 dy,
94 dz, 96 temp3, 104 work; in a twin the fresh values are 107.. in `roles.py` order.
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

## Other constructs (not Ruling 11)
- `vec` (pointer-alias-fake-exception, `/* FAKE */` at its declaration): direct `&D_800A37E8` at the three call
  sites 6 (fal_direct.c); alias declared inside the mode==1 block 6; alias assigned at the loop top 0 (same
  mechanism, s3 scores_alias.txt). Mechanism (dumps/cand): pseudo 74 carries REG_EQUIV (symbol_ref
  "D_800A37E8") in .lreg, ALLOCDBG `pseudo=74 hardreg=-1 pri=92` (lowest; live 1506 insns, 13 calls), so
  reload1.c's reg_equiv_constant rematerializes the address at each use. In-file precedent:
  func_80027AD8 `vec = &D_800A37E8;` (src/code6cac_b_tu2.c:395).
- `hit` / `deep` are `u32` bitmasks (the 22-slot test `hit & (1 << temp3)` keeps sllv/and only on an unsigned
  mask: fold-const.c:4437-4467 rewrites the signed form to `(hit >> n) & 1`; s32 locals score 3). The landing
  retypes func_8002A458's out-parameters to `u32 *hit, u32 *deep` instead of casting (its cc1 output is
  identical: typed/a458check.sh).
- `(Tbl8008E194 *)alt`: func_80027AD8's sixth parameter carries a record on its pass-1 calls
  (func_80031B24) and the 0/1 alternate-blade flag on this pass-0 call (the target loads `fp` into that
  argument slot); its `tbl == NULL ? 0xB : 0x19` mirrors func_80029454's `+0x8C != 0 ? 0x19 : 0xB`.
  Commented at the call.

## Round 2 (layer-2 round 1: rev-2AB08-dm PASS, rev-2AB08-r11 FAIL)
- (1) Q34 for idx `nearest`: above.
- (3) The knockback reads alt (`sll v0,fp,4` 0x8002B8C0) also on path (b) = npass == 0, hit != 0, guard not
  taken, where this iteration has no write to alt. Infeasible: hit is set only by func_8002A458 in the two
  pre-pass blocks, each entered only when other->unk_0E is 6/7 (first) or other->unk_0C is 0x1D/0xE (second),
  and mask_a |= hit after each, so the nearest bit is in mask_a; the guard is then taken unless +0x0C/+0x0E
  changed in between. Nothing writes them: func_8002A458's C body stores only to scr, *hit, *deep and
  D_800A37E8..EC and passes no record pointer on; the effect calls (func_80032854 0x32 / 0x2A with SPAD
  addresses; 0xB / 0xA with scr addresses from func_8002A458) reach func_800395B4 (D_80101BF0 effect slots),
  func_800325E0 and func_800611A4 / func_800619F0 -> func_80060A68, whose indirect calls dispatch through
  chractar_use_pset_combo_id_table (62 targets, asm/data/7D920.data.s:23341-23404). The transitive closure
  (round2/writeset_pathb_table.txt: 115 functions incl. the table targets; writeset_pathb.txt without them; writeset.txt for every func_80032854 arm, 133) references no symbol inside either
  record (0x80101EC8..0x80102713), receives no record pointer, and its displacement-0xC..0xF stores are into
  D_80101BF0 slots, func_80053614's workspace (D_800A33F4 = the caller's buffer), MATRIX out-parameters and
  effect / primitive buffers. No global in src/ is assigned a record address (the two record-derived
  assignments are a u8 result and a flag).
- (2) BLOCKED: three writes re-store the value already held on every feasible path, and the target executes
  them. The pass-loop callees never write other->unk_8C (round2/writeset_8c.txt: func_8002CA8C stores only to
  scr; func_8002CD58 and their callees reference no record symbol, no displacement-0x8C store), so every pass
  >= 1 takes the same arm and pass 0 always writes alt = 0, temp1 = 0, temp2 = 1 first:
  `temp1 = 0; temp2 = 1;` (and `c = 1;`) in the unk_8C arm always re-store 0 / 1 / 1, and `alt = 0;` in the
  unk_0E 4/5 arm always re-stores 0. The target has them: the unk_8C arm (`li $fp,1` 0x8002AEE4) falls into
  the shared tail that pass 0 also reaches (`sw $zero,0x50` / `sw $t3,0x58` / `sw $t4,0x60` 0x8002AEF0-FC,
  pass 0 jumping in with `move $fp,$zero` in the delay slot 0x8002AED0); the 4/5 arm has `move $fp,$zero`
  0x8002AF18. Spellings without them (round2/rs_v*.c): no unk_8C-arm stores and no 4/5 alt store 13; no
  unk_8C-arm stores 12; no 4/5 alt store 1 (the missing `move $fp,$zero`); pass-0 / unk_8C merged with
  `alt = pass != 0` 4. Owner question docs/grind/borderline.md 2026-10-01 func_8002AB08: GRANTED as Q85
  (allow narrowly; each address cited in the variable's comment).
