# func_80074E08 — Ruling 11 package for `work`

laneB, 2026-10-01, answering layer-2 rev-74E08's FAIL (the reopened body's `ot_idx` re-stored the value it
already held at the second select; rejected/r9-ot-idx-restore-fail-0.c). Landing body:
memory/grind/func_80074E08/candidate.c — `ot_idx` is now written only by the area select (once per path),
and the backdrop TILE's OT index lives in `work`, which later also carries the OT byte offset. `cells`
stays under Ruling 9 (r9/receipts.md, passed by rev-74E08). Every score is
`sandbox func_80074E08 --disable all --candidate <file>` on main with r9/prefix.h (r11/scores.txt).

## The variable and its values
| value | writes -> reads |
|---|---|
| 1: the TILE's OT index | `if (arg1 != 0) { work = 0xE; } else { work = 4; }` -> `AddPrim(g_gpu_ot_ptr + work * 4 + 0x24, prim)` |
| 2: the area/offset prims' OT byte offset | `work = ot_idx * 4;` -> the last three `AddPrim(g_gpu_ot_ptr + work (+ 0x24), ...)` |

- (A) fresh function-scope local (value 1 at function level before the loop, value 2 after it; the function
  body is the innermost scope enclosing both), not static/register, address never taken; no other
  declaration moved.
- (B)(1) each write is read: value 1 by the TILE AddPrim on both paths, value 2 by three AddPrims.
  (B)(2) no held-value re-store: value 2 is `ot_idx * 4` (an OT byte offset, never equal to the index
  value 1 holds on any path: 0x38 / 0x10 vs 0xE / 4); value 1's two writes are on exclusive arms.
- (C)(1)-(2) the one-variable-per-value spelling r11/variants/pv.c (`tile_ot` + `ot_ofs`) has the same
  statement list; only declarations and identifiers differ. (C)(3) value 2 is arithmetic in the target
  (`sll $s1,$s1,2`, asm/funcs/func_80074E08.s:191); value 1 is admitted under the Q20 per-branch-constants
  exception: two writes of different constants (0xE, 4) on the two arms of the runtime `arg1 != 0` test
  (target: `addiu $s1,$zero,0x4` :12 and `addiu $s1,$zero,0xE` :36 behind `beqz $s0` :34).

## (D)(1)-(2) Dumps and the deciding decision
r11/tools/dumps_all.sh (r9/tools/dumps.sh per variant: sandbox-stripped TU, `tools/gcc-2.7.2/build/cc1
-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float -dl -dg
-df`, then `BB2_FINDREG_DEBUG=<pseudo>` with the instrumented `tools/gcc-2.7.2/cc1`). Excerpts:
r11/dumps_table.txt. Pseudos: 75 prim, 76 ot_idx, 77 work / tile_ot.

**global.c find_reg pass 0's `regs_someone_prefers` exclusion (global.c:1000-1001), fed by set_preference
(global.c:1671-1760, from mark_reg_store :1484).** In the reuse body insn 562 `(set (reg/v:SI 77) (reg:SI
182))` copies value 2 from 182 = `ot_idx << 2`, which local-alloc seats in $s1 (`;; Register 182 in 17.`,
182 crosses a call inside block 10). set_preference records 17 on work: `;; 77 preferences: 17`. work's
pseudo spans value 1 too, so it conflicts with prim. global allocates in priority order
`116 79 75 72 78 80 73 77 ...`. When prim (75) is allocated, its find_reg trace shows
`someone_prefers: 16 17`: 16 comes from arg1, 17 from work. Pass 0 excludes both, so prim takes $s2 (global
18) and work later takes $s1 (17). That is the target's `lw $s2,0x14($s3)` / `addiu $s1,$zero,0x4`
(.s:11-12). In pv.c the TILE value's pseudo (77, `used 3 times across 23 insns`) has no preference and
does not even cross a call. prim's trace shows `someone_prefers: 16`, so prim takes $s1 (global 17) and the
TILE value lands in $v1 (global 3); score 22. sched1 also moves value 1's `= 4` above the SetTile call only
in the reuse body: compare insn 691's position in .flow with .lreg, where work is a call-crossing pseudo.

## (D)(3) Necessity (mechanism + search, Q31)
In any spelling where the TILE OT value has its own variable, that variable's insns are its two constant
sets and the `<< 2` feeding the TILE AddPrim. The `<< 2`'s result is local-allocated to $v0 or $v1. None of
these insns has a 17-seated register on the other side, so the variable never gets preference 17. Nothing
else that conflicts with prim prefers 17: prim's conflicts are the TILE value, arg0, arg1 and arg1 * 0xF0.
So find_reg's pass 0 seats prim in $s1 and the target's $s2 is unreachable. Value 2 gives work the
preference only because it is a copy of the $s1-seated `ot_idx << 2`. A fresh `ot_ofs` has the
preference but does not conflict with prim (z4 / pv).

Banked counting spellings (fresh locals, no FAKE construct), none reaching the target:
| spelling | score |
|---|---|
| reuse body = candidate.c (variants/reuse.c) | **0** |
| one variable per value: tile_ot + ot_ofs (pv.c; reversed declarations pv_rev_decl.c; tile_ot in its own block pv_v1_block.c) | 22 / 22 / 22 |
| no second value, every AddPrim computes `ot_idx * 4` (pv_no_ofs.c) | 22 |
| `ot_ofs` after the area select, used by all four (pv_ofs_after_select_all4.c) | 26 (280) |
| value 1 as default + override / declaration initializer / ternary / inline in the call (pv_v1_default, pv_v1_declinit, pv_v1_ternary, pv_v1_inarg) | 22 / 18 / 22 / 38 (283) |
| variants_extra w1-w9, x1-x5, y1-y4 (TILE value split, at function entry, prim retyped, decl order, store forms) | 18-38 |
| variants_extra z3 / z4 (fresh `ot_ofs` at the other placements) | 22 (280) / 18 |
| reviewer rev-74E08's split_ot.c / split_ot2.c (rejected/rev74e08/) | 22 |
| reopened body without the re-store (rejected/rev74e08/noreset_ot.c) | 26 (279) |

Reuse spellings that also reach 0 (not counting): variants_extra z2 / z7 / z8 / z11 (work's second value
at the second area AddPrim with different use sets), the reopened held-value re-store (refused).

## (D)(4) Permuter
Campaign `e08-split-ot` from the split body (variants_extra/w9_decl_init.c, r9/tools/mkperm.sh /
camp.sh, -j2, --stack-diffs, --stop-on-zero): 816 iterations, 604 s, base 335. It stopped at its only 0:
`tile_ot = ot_idx * 4;` before the second area AddPrim, used by the last one. That is this reuse itself (the
origin of this body), not a counting spelling. Second campaign `e08-split-ot-nostop` from
variants_extra/w5_split_ifelse_first.c (= pv.c without `ot_ofs`, base 957, no --stop-on-zero, launched
2026-10-01T09:28Z): stopped after 1,204 s, 13,362 iterations, best 545. Best finds: 545 (output-545-1) a
`(float)` cast on SetTile's argument (not C we could land); 545 (output-545-2) `tile_ot` also holding the
TILE colour 0xD0 (another reuse of the same variable); 605 a statement reorder. No find reaches the
target. Kept: r11/perm_finds/ (stop_output-0-1.c is the first campaign's 0).

## Q30 set-aside
None: no measured spelling carries a FAKE construct and the body carries none.

## (E) name, (F) annotation, (G), (H)
(E) `work`, a generic scratch word (value 1 is an OT index, value 2 an OT byte offset; no one kind-name fits
both). (F) the declaration comment names both values and cites Ruling 11 (Q20 for value 1) and this file.
(G) fresh layer-2 on the exact staged body. (H) everything else judged on its merits.
