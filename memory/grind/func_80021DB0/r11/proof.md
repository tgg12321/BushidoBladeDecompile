# func_80021DB0 — Ruling 11 submission for `temp` (was `j`); Q51 SOTN citation for `i` (laneG, 2026-09-30)

Answers the retro-audit CONCERN on 287c00642 (tmp/audit-2026-09-29/review/batch_01.md, owner Q49 fix-forward):
"The loop counter `j` is reused as the chosen stage-record index ... load-bearing and undisclosed: a fresh
`sel` scores 15/285 ... Neither [family] is claimed." Rulings: `.claude/rules/ordinary-c-judge-decidable.md`
§ Ruling 11 with its Q30 set-aside, Q31 mechanism + search standard and Q58 any-named-pass clause; for `i`,
owner Q51 (`.claude/rules/no-new-park-categories.md` § Owner ruling 2026-09-30 — SOTN precedent suffices).

What changed against the landed body (variants/landed_body.c), each sandbox 0:
- `j` -> `temp` (Ruling 11 (E)(i)), with the (F) declaration comment.
- `i` carries a SOTN citation comment (Q51). No statement changed.

Files: `final.c` (the submitted body, == ../candidate.c); `one-var-per-value-form.c` (the (C)(1) spelling,
== variants/split_all.c); `variants/` every measured spelling; `dumps.txt` the (D)(1) excerpts; `tools/` the
scripts (gen.py ablations, gen_struct.py structural, dump.sh / runall.sh / fr.sh dumps, r11table.py pseudo
naming, holders.py, collect.py -> dumps.txt, mkperm.sh permuter workspace, permfinds.py).

Scores: `sandbox func_80021DB0 --disable all` (target 285 insns), 2026-09-30, main with laneE's landing in.

## The variable `temp` and its values (all in $s1 in the target)
- V1 the ray-walk step: `for (temp = 1; temp < 41; temp++)`, read by `(dx * temp) / 40`, `(dz * temp) / 40`
  (target `addiu $s1,$zero,1` 0x80021F48, `addiu $s1,$s1,1` / `slti $v0,$s1,0x29` 0x80022000/04).
- V2 the floor-climb step: the second `for (temp = 1; temp < 41; temp++)`, read by `out->y + temp * 100`
  (0x80022080, 0x800220CC/D4).
- V3 the chosen stage start record: `temp = D_800A38E0;` (practice lesson) or `temp = 0` / `temp = i` in the
  nearest-of-four search, read by `stage += temp * 6 + 3` (`lbu $s1,%lo(D_800A38E0)($s1)` 0x80022108,
  `addu $s1,$zero,$zero` 0x80022128, `addu $s1,$s5,$zero` 0x8002219C, `sll $v0,$s1,1` 0x800221B0).
No write of one value reaches a read of another: V1 and V2 each begin with `temp = 1`, and both arms of the
`D_800A38DC == 3` test write V3 before its one read. Three values in Ruling 11's sense.

## (A) Fresh local, not a borrow
A local of func_80021DB0, declared once at function scope (V3's writes are at function level), not a
parameter/global/static/register, never `&temp`. No other declaration is moved or re-scoped against the
one-var form except the per-value declarations themselves.

## (B) Every write is live; no re-store
(1) V1/V2 writes are read by the loop test and the loop body; V3's writes by `stage += temp * 6 + 3`
(`temp = 0` by it when no `d < best`, else overwritten by `temp = i` which is read).
(2) Writes that may store a held value, with a feasible path where the held value differs:
- V1 `temp = 1`: on the first outer iteration there is no earlier write; on a later one it holds V1's or
  V2's exit value, e.g. 41 after a full walk (every func_8005344C call returning 0).
- V2 `temp = 1`: holds V1's exit value; path: V1 ran all 40 steps -> 41.
- V3 `temp = D_800A38E0`: holds V1/V2's last value or nothing; path: D_800A38E0 = 0 after a full walk (41).
- V3 `temp = 0`: path: an iteration ran V1 to 41 and then `continue`d, D_800A38DC != 3 -> holds 41.
- V3 `temp = i`: at i = 1 with d < best it stores 1 while holding 0 (or the i = 0 value 0).
- `temp++` always changes the value.

## (C) Same statements; real computations
(1) one-var-per-value-form.c: V1 -> `step`, V2 -> `rise`, both declared at the top of the outer-loop body
(the innermost scope enclosing each loop's writes); V3 -> `sel` at function scope (its writes are in both
arms of a function-level if). (2) `diff final.c one-var-per-value-form.c`: declarations, identifiers and the
(F)/(Q51) comments only. (3) V1/V2: `temp++` (`addiu $s1,$s1,1`); V3: the `lbu` load of D_800A38E0.

## (D)(1) Dumps — dumps.txt
`tools/dump.sh` substitutes the body into a copy of src/code6cac_tu2.c (engine.inlineasm.substitute_body),
preprocesses with engine/buildconfig.py CPP_FLAGS + CPP_DEFS, and compiles with the INSTRUMENTED
`tools/gcc-2.7.2/cc1`, the build's CC_FLAGS (`-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls
-fno-builtin -w -mel -msoft-float`) plus `-dr -ds -dt -df -dc -dl -dg`, BB2_ALLOC_DEBUG=1, and
BB2_FINDREG_DEBUG=<pseudo> reruns. The build cc1 is run on the same .i: every .s is identical
("instcheck: identical" per variant). Dumped: final, split_all (the one-var form), split_loop1, split_loop2,
split_sel. Pseudo naming by tools/r11table.py (reg/v pseudos zipped with declaration order; count checked).
final: temp = 79, i = 78, best = 83. split_all: sel = 79, step = 91, rise = 92.

## (D)(2) Mechanisms (pass and source location)
**M1 — global.c find_reg's call-clobbered exclusion (global.c:970-975) and first-free scan.** A pseudo that
crosses a call may only take call-saved registers (`used1 = call_used_reg_set`); one that crosses none
starts from `fixed_reg_set`. final: temp (79) crosses calls (V1/V2's ranges contain the func_80053614 /
func_8005344C calls); FINDREG 79 pass1_used = every call-used register plus its conflict $s0 -> $s1 for all
three values, the target's register for V3. split_all: sel (79) lives only in the call-free tail (from its
writes to `stage += sel * 6 + 3`); FINDREG 79 pass1_used = {0-4, s3-s5, 26-31}, no call-used block -> first free $a1, and `best`
moves from $a1 to $a3 (the target: index $s1, best $a1). split_sel = 15 is this decision alone.
**M2 — global.c allocno priority order (allocno_compare, global.c:635-655; allocation loop global.c:575-598)
against loop.c's strength-reduced giv.** loop.c rewrites `dz * step` as a giv incremented by dz each
iteration (final pseudo 272: `(set (reg 272) (reg/v 82))`, `(set (reg 272) (plus (reg 272) (reg/v 82)))`,
.lreg). Priority = floor_log2(refs) * refs / live_length. final: temp 28 refs / 83 insns -> 13493, above the
giv's 13125 (14/32) -> temp is allocated first (ord 8) and takes $s1 through both loops; the giv gets $s2
(ord 10). split_all: step 11/35 -> 9428 < 13125 -> the giv (274, ord 10) takes $s1 in the walk loop and step
gets $s2 (FINDREG 91 conflicts include 17; holders.py: 274). split_loop1 (7): the same for step;
split_loop2 (14): the walk counter shared only with V3 (17/62 -> 10967) still sorts below the giv and gets
$s2. V1 reaches the target's $s1 only when its allocno also carries V2's references.

## (D)(3) Necessity (Q31 mechanism + search)
(a) M1, M2 from banked dumps of both spellings. The properties, and why each per-value spelling lacks them
BECAUSE the value has its own variable: M1 — the V3 variable crosses calls only through V1/V2's live ranges;
a per-value V3 variable is referenced only by the selection statements, which (C)(2) fixes and which contain
no call, so allocno_calls_crossed is 0 and the scan reaches a free call-clobbered register before $s1.
M2 — the walk counter's priority is its own refs over its own live length; with its own variable those are
fixed by the loop's statements (11 refs over 35 insns, 9428), below the giv's 13125, so the giv is seated
first. (b) Every per-value spelling proposed is banked in variants/ and measured (table). (c) None reaches the
target.

## (D)(4) Measured alternatives (sandbox --disable all; target 285)
| spelling (variants/) | score | insns |
|---|---|---|
| final.c | **0** | 285 |
| landed body (`j`) | 0 | 285 |
| full one-var-per-value (split_all = one-var-per-value-form.c) | 22 | 285 |
| ablation: V1 alone split (split_loop1) | 7 | 285 |
| ablation: V2 alone split (split_loop2) | 14 | 285 |
| ablation: V3 alone split (split_sel) | 15 | 285 |
| V3 split, `sel` declared first / before `i` / after `j` / after `d` (sel_*.c) | 15 / 15 / 15 / 15 | 285 |
| structural: every per-value local at function scope (s_S1_funcscope) | 22 | 285 |
| structural: declaration order reversed, `sel` first (s_S2_declorder) | 22 | 285 |
| structural: each step loop in its own `{ s32 step; ... }` block (s_S3_loopblocks) | 22 | 285 |
| structural: the record pointer advanced in each arm (s_S5_perarm) | 27 | 290 |
| permuter from the one-var form | see § Permuter | |
No FAKE-construct spelling was measured (none was needed to argue (D)(3); Q30 would set them aside).

### Permuter
Workspace tools/mkperm.sh (reduced TU: the body's own declarations + the body; compile.sh = the build's cpp |
build cc1 (CC_FLAGS) | prologue_fix | maspsx (MASPSX_FLAGS) | multu_pad | as; the landed body built there differs
from target.o in 0 instructions). Campaign from one-var-per-value-form.c (tools/permuter_campaign.py, 2 jobs,
--stack-diffs, 2026-09-30 19:33-19:58): 22,370 iterations in 1,472 s, 11 finds, permuter score 130 -> best 95
(found at ~110 s; no better find in the remaining ~1,350 s; stopped). Nothing reached the target (score 0).
What the finds reuse (permuter.txt, the 6 best as changed lines): each one stages an unrelated value through
`sel` or `best` (the tail-only locals) inside the call-crossing loops -- `sel = base.x + dx; probe.x = sel;`
(95), `sel = out->y + rise * 100` (105), `best = func_8005344C(...) != 0` (105), `best` as angle / hit.y /
stage[2] holders (115). That is a second value put into a per-value variable, the reuse this submission
declares, rediscovered by search; none is a one-variable-per-value spelling.

## (E) Name
`temp`: Ruling 11 (E)(i) generic scratch word. (The three values are two step counters and a record index,
not one kind of quantity, so (E)(ii) is not claimed.)

## (F) Annotation
final.c's declaration comment names the three values and cites Ruling 11 and this file.

## `i` — owner Q51 SOTN citation (not Ruling 11)
`i` is the counter of the eight-direction probe loop and, after it, of the nearest-of-four start-record search:
two consecutive `for (i = 0; ...)` loops, each initializing it. SOTN reuses one counter the same way across
consecutive loops in CheckIfAllButtonsAreAssigned, `src/dra/menu.c:132-166` at aa53500 (`s32 i;` :134, loops
:138, :142, :146, :155, :161, each `for (i = 0; i < N; i++)`); menu.c is `[0x5483C, c, menu]` in
config/splat.us.dra.yaml:226 (the US PS1 build) and has no INCLUDE_ASM, and the function is outside any
version guard (the only `#if` inside it, :148-150, guards one statement). Q53 paperwork: the citation comment
on the declaration; simpler spelling measured: a separate counter for the search loop (s_S4_selcounter)
scores 80 (285). SOTN does not mark the construct, so no FAKE marking is carried (Q52 applies only to
SOTN-marked constructs); whether Q53's "FAKE where match-motivated" applies to plain consecutive-loop counter
reuse is left to the reviewer.

## Not in scope, disclosed
The body reads the sine table as `(&Judge)[...]` because code6cac_tu2.c declares `extern s16 Judge;` (a
scalar) TU-wide (code6cac_tu2.c:65; other consumers in the same TU index it the same way). text1b.c declares
`extern s16 Judge[];` (17e01239b). Respelling the TU declaration touches every consumer in the file; it is
not part of this concern and is left for a separate cleanup.
