# func_8002D780 — Ruling 11 package for `cross_center` / `cross_point` (was `kc` / `kp`) (laneA, 2026-09-30)

Body: memory/grind/func_8002D780/candidate.c (verbatim inline_o.h islands; the two FAKEs of the
2026-09-30 laneB entry unchanged), with `kc`/`kp` renamed to `cross_center`/`cross_point` and the (F)
annotation added at their declaration. The rename is byte-neutral: variants/reuse_renamed.c = 0/202
(the pre-rename body was 0/202 too). Scores are `sandbox func_8002D780 --disable all` against main
a739dbd20 via tools/sandbox_sweep.ps1 (full outputs in tmp/sandbox_sweep/func_8002D780/). Tools:
r11/tools/ (they read and write tmp/func_8002D780/).

## The two variables and their values
Coordinates are relative to the triangle's third vertex, so the triangle is (0,0), (x0,z0), (x2,z2);
(cx,cz) is its centroid and (px,pz) the query point. Each side test compares the sign of two 2-D cross
products of the same edge: one with the centroid's offset, one with the point's offset.

| variable | value 1 (test 1, edge (0,0)-(x0,z0)) | value 2 (test 2, edge (0,0)-(x2,z2)) | value 3 (test 3, edge (x0,z0)-(x2,z2)) |
|---|---|---|---|
| `cross_center` | `z0 * cx - x0 * cz` | `z2 * cx - x2 * cz` | `(flag * ax) - (dx * az)` |
| `cross_point` | `z0 * px - x0 * pz` | `z2 * px - x2 * pz` | `(flag * (px - x0)) - (dx * bz)` |

Each value is read exactly once, by that test's `if ((cross_center ^ cross_point) >= 0)`, before the
next write. No write can reach another test's read, so each variable holds three values in the sense
of Ruling 11.

- **(A)** Both are fresh locals of this function (not parameters, globals, `static` or `register`),
  declared once at the innermost scope enclosing all their writes (the block that opens with the
  x0..pz loads; value 1 is written there, values 2 and 3 in nested `if` arms). No address is taken,
  and no other declaration was moved or re-scoped by this change (the rename touched only these two
  identifiers; the annotation comment was added).
- **(B)** (1) Every write is read before the next write, by its test's XOR. (2) No write stores a
  value the variable already holds: the three values are cross products of three different edges.
- **(C)** (1) The one-variable-per-value spelling is variants/pv_all.c (`cross_center1..3`,
  `cross_point1..3`, each declared at the innermost scope enclosing its write: values 1 in the outer
  block, values 2 at the top of the test-1 arm, values 3 at the top of the test-2 arm). (2) Its
  statement list is the reuse body's: only declarations and identifiers differ (diff
  variants/reuse_renamed.c variants/pv_all.c). (3) Every value is an arithmetic computation (two
  multiplies and a subtract) whose instructions are in the target.

## (D)(1)-(2) Dumps and the mechanism (sched.c, first scheduling pass; owner Q58 any named pass)
Command (r11/tools/dump_sched.sh; the variant spliced into a copy of src/code6cac_b_tu2.c @ a739dbd20):
`mipsel-linux-gnu-cpp <build CPP flags> tu.c > tu.i;`
`BB2_SCHED_DEBUG=1 tools/gcc-2.7.2/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls
-fno-builtin -w -mel -msoft-float -df -dc -dl -dg -dS tu.i -o tu.s 2> sched.err` (the instrumented cc1;
.flow/.combine/.lreg/.greg/.sched dumps plus the sched1 SCHEDDBG trace). Excerpts: r11/dumps_table.txt
(r11/tools/r11table.py): for each spelling, every cross-product set (insn uid -> pseudo), its
.flow/.lreg lines, its .greg seat, and its sched1 `ADJPRI` / `PICK` trace lines.

The decision: sched.c `adjust_priority` (:2543-2595) runs when an insn becomes ready in the backward
list scheduler; for an insn with no REG_DEAD notes it calls `birthing_insn_p` (:2505-2536), which
returns `reg_n_sets[regno] == 1` for a SET of a live register, and if so raises the insn's priority to
`max_priority`. `rank_for_schedule` (:2408-2465) then picks the higher priority first, and on a tie
the higher INSN_LUID first. The scheduler works backwards, so the insn picked first in a block is
emitted LAST.

- **Test 1 (block 5), `cross_point` value 1 (insn 164).** Reuse: `ADJPRI insn=164 ... birth=0 pri=13`
  (pseudo 120 is set three times), and the centroid subtract insn 157 (pri 24) is picked first
  (`PICK clock=3 picked=157`, `clock=4 picked=164`): the point subtract is emitted BEFORE the centroid
  subtract, the target's order (asm/funcs/func_8002D780.s: `subu $v1,$a0,$v0` at 0x8002D8B4, then
  `mflo $s1; subu $v0,$t6,$s1`). Any spelling in which value 1 has its own variable
  (variants/pv_all.c, variants/abl_p1.c = s_0_1, s_0_123): `ADJPRI insn=164 ... birth=1`, the priority
  becomes max_priority, and insn 164 is picked first (`PICK clock=3 picked=164 (pri=2130706433
  luid=21)`), so it is emitted AFTER the centroid subtract: the order flips and the register
  assignment downstream of it follows (pv_all 36, s_0_1 41, s_0_123 44).
- **Tests 2 and 3 (blocks 6, 7), `cross_center` values 2 and 3 (insns 176/177, 207/208).** Reuse: the
  centroid and point subtracts tie at pri 12 with birth=0, and the tie goes by luid: the point
  subtract is picked first and emitted last (`picked=183` then `picked=176`; `picked=219` then
  `picked=207`), the target's order. When the centroid value has its own variable and the point value
  does not (variants/var_c_all.c = s_123_0, s_2_0, s_3_0): `ADJPRI insn=177 ... birth=1` / `insn=208
  ... birth=1`, max priority, the centroid subtract is picked first and emitted last: the order flips
  (s_123_0 38, s_2_0 13, s_3_0 28).

## (D)(3) Necessity (mechanism + search, owner Q31)
The property the target's order depends on is a property of the variable: the set of the test-1 point
cross product must NOT be a birthing insn, i.e. its register must be set more than once. In every
one-variable-per-value spelling the value-1 point variable is set exactly once (the statement list is
fixed by (C)(2)), so birthing_insn_p returns 1 whatever its declaration order, scope, type or name, and
adjust_priority raises it above the centroid subtract (pri 24). The centroid subtract could then only
come first by also being a birthing insn AND winning the luid tie, i.e. by the centroid statement
following the point statement in the source; that spelling is banked below (swap_1: 35). For
`cross_center`: with `cross_point` kept as in the reuse body, a centroid variable holding only one of
values 2/3 is a birthing insn while the shared point is not, which flips that test (s_2_0 13, s_3_0 28,
s_123_0 38). The full 64-subset grid (every combination of the six values split out or shared; r11/
tools/r11all.py, scores r11/grid64_scores.txt) has exactly four zeros: the reuse body (s_0_0), s_1_0
(centroid value 1 split out), s_2_2 (both test-2 values split out) and s_3_3 (both test-3 values split
out). In every one of them BOTH variables still hold two or more values. None is a
one-variable-per-value spelling.

Banked counting spellings (fresh locals only; none carries a FAKE construct the reuse body lacks):
| spelling | score |
|---|---|
| reuse body, renamed (variants/reuse_renamed.c) | **0** |
| one-variable-per-value, values 2 declared then assigned (variants/pv_all.c) | 36 |
| one-variable-per-value, values 2 as initialized declarations (variants/pv_all_init.c) | 36 |
| pv_all, test-1 values declared then assigned (variants/decl_assign.c) | 36 |
| pv_all, point statement before centroid statement in test 1 / 2 / 3 / 1+2 / 1+3 / 2+3 / all (variants/swap_*.c) | 35 / 41 / 41 / 42 / 42 / 46 / 46 |
| pv_all with Ruling 4 compound-assignment splits (`x = a*b; x -= c*d;`) in every non-empty subset of the six values (63 spellings, r11/tools/r11pv.py; e.g. variants/si_111111.c 60, si_000100.c 50) | 37 .. 65 (r11/pv_respell_scores.txt) |
| pv_all, test-1 centroid product written inline in the `if`, no variable (variants/pinl.c) | 35 |
| pv_all, test-1 point product inline (variants/pinl_p.c) | 36 |
| pv_all, both test-1 products inline (variants/pinl_both.c) | 36 |

## (D)(4) Measured alternatives
- Full one-variable-per-value: 36 (above).
- Ablation, each value split out alone with the rest shared: centroid value 1 / 2 / 3 = 0 / 13 / 28
  (variants/abl_c1..3.c); point value 1 / 2 / 3 = 41 / 11 / 13 (variants/abl_p1..3.c). The single zero
  (abl_c1 = s_1_0) still has both variables multi-valued; see "Other byte-exact forms" below.
- Per-test pairs split: test 1 / 2 / 3 = 36 / 0 / 0 (variants/pair_1..3.c); each variable split
  completely with the other shared: centroid 38 (var_c_all.c), point 44 (var_p_all.c).
- Structural respellings: the swap, compound-split, inline and declared-then-assigned rows above.
- Permuter, campaign 1 (tools/permuter_campaign.py, label d780-pv-all, workspace built by
  r11/tools/mkperm.sh from variants/pv_all.c, -j2, --stack-diffs, --stop-on-zero; launched
  2026-09-30T22:18:16Z): base 910, 1,305 iterations, stopped on a zero at 150 s. The zero
  (variants/pinl_dead.c, re-measured in the sandbox: 0/202) writes the test-1 centroid product inline in
  the `if` AND keeps `s32 cross_center1 = z0 * cx - x0 * cz;` declared but never read. Without that
  unread local it scores 35 (variants/pinl.c). It is set aside under Q30 (below). The other finds
  re-create a reuse: 40 (`cross_point1 = (cross_center3 ^ cross_point3) >= 0;` — cross_point1 written a
  second time), 815 (`x2 = cross_center1;` borrow), or add non-C devices (284 `volatile`, 240/465/600
  `inline_fn`).
- Permuter, campaign 2 (fresh workspace = fresh seed, same body, no --stop-on-zero; label
  d780-pv-all-2, launched 2026-09-30T22:22:37Z, harvested and stopped after 1,356 s): 12,252
  iterations. Its zeros (output-0-1, output-0-2) are the same unread-`cross_center1` find as campaign 1
  (set aside, below; 0-2 also adds a `new_var = 1` return). Every other low find writes `cross_point1`
  a second time, i.e. re-creates the point variable's reuse — exactly the property named in (D)(3):
  20 (`cross_point1 = z2 * cx;`), 45 (`cross_point1 = x2 * cz;`), 70 (`dx = (cross_point1 = x2 - x0);`),
  155 (`cross_point1 = px - x0;`), 225 (`cross_point1 = z2 - z0;`). No find reaches the target without
  either a second write of a value-1 variable or an unread local.

## Q30 set-aside
variants/pinl_dead.c needs a local whose stored value is never read (`cross_center1`, initialized and
never used) — [[dead-store-fake-exception]] ("dead store to a local: `dest = val1;` where the STORED
VALUE is never read"), whose prerequisite 3 reads: "**Mandatory annotation:** `/* FAKE: <one-line
reason> */` or `// FAKE: <reason>` ON the statement. Un-annotated instances remain forbidden". It
carries, unchanged, the body's two FAKE constructs (the `flag` staging and the `m = dist` re-store),
and it needs one the reuse body does not carry, so it is set aside and does not defeat the reuse.

## Other byte-exact forms (not one-variable-per-value)
s_1_0 (centroid value 1 in its own `cross_center1`, `cross_center` holding values 2 and 3), s_2_2 and
s_3_3 also reach 0/202. Each still has two variables holding two or more values, so each needs this
same Ruling 11 package for both variables; none has fewer no-purpose constructs than the body. The body
keeps the uniform spelling — one pair of variables for the three repetitions of the same test — which
is also how the matched sibling in this file, func_8002E6B0 (src/code6cac_b_tu2.c, `cross_center` /
`cross_point` rewritten for each of its three edges), spells the same triangle test.

## (E) Names, (F) annotation
(E)(ii): every write of `cross_center` is a cross product of a triangle edge with the centroid's offset,
and every write of `cross_point` one with the point's offset, so each name is true of every value (the
names the sibling func_8002E6B0 uses). (F): the comment at the declaration names the three values and
cites Ruling 11 and this file.

## (G), (H)
Layer-2 on the exact staged body. Everything else in the body is judged on its own: the `flag`
staged-value FAKE and the `m = dist` same-value re-store FAKE keep their own annotations (neither
variable claims this ruling; `flag` is read by the value-3 writes, not a consumer of either variable).
