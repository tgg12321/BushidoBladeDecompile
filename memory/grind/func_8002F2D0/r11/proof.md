# func_8002F2D0 — Ruling 11 package for `work` (was `det`) and `temp` (was `sum`)

laneB started this package (2026-09-30, ea39535f1); laneA re-measured and completed it on the landing body
the same day. Landing body: memory/grind/func_8002F2D0/candidate.c (verbatim inline_o.h / gtemac.h islands,
no do-while(0) wraps, the normalized cofactors in fresh `i0`/`i1` like `i2`, sqrt-table reads spelled
`(&g_sqrt_table_u8)[i]`), with the two multi-value locals named and annotated per (E)/(F). Every score is
`sandbox func_8002F2D0 --disable all` on main (tools/sandbox_sweep.ps1; outputs in
tmp/sandbox_sweep/func_8002F2D0/). Landing-chassis spellings: r11/variants_landing/ (generator
r11/tools/r11gen_landing.py). laneB's first-chassis spellings (in-place `c0 = c0 / det`) stay in
r11/variants/ and measure the same (41 / 41 / 41 / 6).

## The two variables and their values
| variable | value 1 (writes -> reads) | value 2 (writes -> reads) |
|---|---|---|
| `work` | the 3x3 determinant `(d0 + m10*(c1>>12) + m20*(c2>>12)) >> 12` -> the six divisions (i0, i1, i2, r0, r1, r2) | the square root of i0*i0+i1*i1, written in both arms of `if ((u32)temp < 0x400)` (table byte >> 3, or the LZC-shifted table value) -> `ratan2(i2, work)` |
| `temp` | `i0*i0 + i1*i1` -> the `< 0x400` test, the small-value table index, the `>= 0` test, gte_ldlzc's input, `(u32)temp >> shift` | the square-root table byte `(&g_sqrt_table_u8)[(u32)temp >> shift]` -> `(u32)(temp << 16) >> ...` |

(A) both are fresh function-scope locals (the innermost scope enclosing all their writes: value 1 is
written at function level), never `static`/`register`, address never taken. (B) every write is read
before the next write; no write re-stores a held value. (C)(1)-(2) the one-variable-per-value spelling
(`dist` for work's value 2, block-local `tb` for temp's value 2) has the same statement list; only
declarations and identifiers differ (variants_landing/reuse.c vs variants_landing/pv_both.c). (C)(3) all
four values are real computations whose instructions are in the target (arithmetic, a load).

## (D)(1)-(2) Dumps and the mechanism
Command: r11/tools/dumps.sh = the sandbox-stripped TU with the variant substituted, then
`tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w
-mel -msoft-float -dl -dg -df`; r11/tools/cut.py, r11table.py, shiftseat.py. Landing-chassis excerpts:
r11/dumps_table_landing.txt (laneB's first-chassis table, r11/dumps_table.txt, shows the same decisions
with sum = pseudo 89).

- **`work` (value 1 = the determinant), local-alloc.c:472.** In the reuse spelling the determinant's pseudo
  (81) is also written in the sqrt arms, so it is referenced in more than one basic block and dies twice
  (`Register 81 used 16 times across 21 insns; dies in 2 places`). local_alloc admits a pseudo only when
  `reg_basic_block[i] >= 0 && reg_n_deaths[i] == 1`, so 81 is left to global.c, which seats it in $t2
  (`global 10`) — the target's `sra $t2,$v0,12; div $zero,$s1,$t2` (asm/funcs/func_8002F2D0.s:83-84). In
  the per-value spellings the determinant's pseudo lives only in block 0 with one death (`used 13 times
  across 16 insns in block 0`), local-alloc takes it and seats it in $v0 (pv_both .lreg `;; Register 81
  in 2.`, and 81 is absent from the .greg `;; 27 regs to allocate:` list, which in the reuse spelling
  starts `81 91 ...`; the table's "global 2" is r11table.py reading the final disposition): pv_both 41,
  work split alone 35 (both 269/270).
- **`temp` (value 1 = i0*i0+i1*i1), global.c set_preference.** global.c:1484 mark_reg_store ->
  set_preference (global.c:1671-1760) takes the first operand of a set's source (`(ashift (reg) 16)` -> the
  reg) and, when the destination was local-allocated to a hard register, gives that register to the
  source's allocno as a preference. In the reuse spelling the source of `temp << 16` is temp's own pseudo
  (91) and the shift's destination (227) is local-allocated to $a0, so temp gets preferences {4,16,17}
  (`;; 91 preferences: 4 16 17`) and find_reg's preferred pass seats it in $a0 (`global 4`) — the target's
  `addu $a0,$v1,$a2` (.s:207) ... `srlv $v0,$a0,$v1` (.s:232). In the per-value spellings the shift's source
  is `tb`'s pseudo (225 / 224), temp keeps only {16,17} and is seated in $s0 (`global 16`): temp split
  alone 6.

## (D)(3) Necessity (mechanism + search, owner Q31)
Both properties belong to the variable, not to spelling details. (work) In any spelling where the
determinant has its own variable, the statement list fixed by (C)(2) keeps every reference to that
variable inside block 0 with one death, so local-alloc.c:472 admits it and it cannot reach the global $t2
seat. (temp) In any spelling where the table byte has its own variable, that variable, not temp's, is the
source of the `<< 16`, so set_preference never gives temp the $a0 preference. Banked counting spellings,
all measured on the landing chassis, none reaching the target:
| spelling (fresh locals, no FAKE construct) | score |
|---|---|
| reuse body (= candidate.c) | **0** |
| per-value: `dist` + block-local `tb` (variants_landing/pv_both.c) | 41 (269 insns) |
| per-value, `tb` at function scope (pv_both_tbfn.c) | 41 |
| per-value, `dist` declared before the determinant's variable (pv_both_order.c) | 41 |
| per-value, determinant `det` in its own nested block around the six divisions (pv_both_detblock.c) | 41 |
| per-value `dist`, table byte read inline (pv_work_table_inline.c) | 41 |
| only work split (`dist`; pv_work_only.c) | 35 (269 insns) |
| only temp split (`tb`; pv_temp_only.c) | 6 |
| table byte read inline, temp single-valued (table_inline.c) | 6 |
| first chassis (laneB, in-place `c0 = c0 / det`): pv_both / pv_both_tbfn / pv_both_order / pv_sum (r11/variants/) | 41 / 41 / 41 / 6 |
| first chassis: compound `c0 /= det; c1 /= det;` on pv_both; det in a nested block; both (variants/pv_cdiv.c, pv_detblock.c, pv_detblock_cdiv.c) | 41 / 41 / 41 |

## (D)(4) Permuter
- Campaign 1 (laneB, first chassis; label f2d0-pv-both, from variants/pv_both.c, -j2, --stack-diffs,
  --stop-on-zero, launched 2026-09-30T17:42:55Z): 37,248 iterations, base 350, best 30 (output-30-1:
  `dist = det;` before the divisions — the determinant and the sqrt value sharing a variable again, the
  `work` property); next best 150 (a `new_var` alias plus `det = vec[2]`). No find reaches the target.
- Campaign 2 (laneA, first chassis, fresh workspace; label f2d0-pv-both-2): stopped after 1,122 iterations
  (best 220) when the body moved to fresh i0/i1; superseded by campaign 3.
- Campaign 3 (laneA, landing chassis, fresh workspace from variants_landing/pv_both.c; label
  f2d0-pv-both-i01, -j2, --stack-diffs, no --stop-on-zero, launched 2026-10-01T00:08Z): stopped after 1,233 s,
  12,583 iterations, base 350, best 30. Every find under 200 makes some variable hold the determinant
  and a second value again: 30 (`dist = <determinant>; work = dist;` — dist then also holds the sqrt,
  the `work` property), 150-2 (`temp = <determinant>; work = temp;`), 180 (`c0 = <determinant>;`), or
  splits the determinant expression in two writes of `work` (160, 170; Ruling 4 splits, still 160/170).
  No find reaches the target.

## Q30 set-aside
No spelling is set aside: no measured spelling carries a FAKE-annotated construct, and the body carries
none.

## (E) names, (F) annotation, (G), (H)
(E)(i): `work` and `temp` are generic scratch words. (F): the comment at each declaration names both
values and cites Ruling 11 and this file. (G): layer-2 on the exact staged body. (H): everything else is
judged on its own merits.
