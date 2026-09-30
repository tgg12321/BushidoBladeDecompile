# func_8002F770 — Ruling 11 package for `work` (was `det`) and `temp` (was `sum`) (laneB, 2026-09-30)

Body: memory/grind/func_8002F770/candidate.c (verbatim inline_o.h islands, no do-while(0) wraps, `m00` inlined),
with the two multi-value locals renamed and annotated per (E)/(F). The rename is byte-neutral
(0 before and after, score_nostrip and the real sandbox). Tools: memory/grind/func_8002F770/r11/tools/.

## The two variables and their values
| variable | value 1 (writes -> reads) | value 2 (writes -> reads) |
|---|---|---|
| `work` | the 3x3 determinant `(d0 + m10*(c1>>12) + m20*(c2>>12)) >> 12` -> the six divisions (c0, c1, i2, r0, r1, r2) | the square root of c0*c0+c1*c1, written in both arms of `if ((u32)temp < 0x400)` (table byte >> 3, or the LZC-shifted table value) -> `ratan2(i2, work)` |
| `temp` | `c0*c0 + c1*c1` -> the `< 0x400` test, the small-value table index, the `>= 0` test, gte_ldlzc's input, `(u32)temp >> shift` | the square-root table byte `*(table + ((u32)temp >> shift))` -> `(u32)(temp << 16) >> ...` |

(A) both are fresh function-scope locals (the innermost scope enclosing all writes: value 1 is
written at function level), never `static`/`register`, address never taken. (B) every write is
read before the next write; no write re-stores a held value. (C)(1)-(2) the one-variable-per-value
spelling (`dist` for work's value 2, `tb` for temp's value 2) has the same statement list, only
declarations and identifiers differ (r11/variants/pv_both.c). (C)(3) all four values are real
computations whose instructions are in the target (arithmetic, loads).

## (D)(1)-(2) Dumps and the mechanism (r11/dumps_table.txt; command: r11/tools/dumps.sh = the
build's cc1 flags + `-dl -dg -df` on the sandbox-stripped TU)
```
== tmp/F770/d_reuse:func_8002F770
   det: pseudo 84, seat global 10 | Register 84 used 16 times across 21 insns; dies in 2 places; GR_REGS or none. | no preference line
   sum: pseudo 92, seat global 4 | Register 92 used 8 times across 22 insns; dies in 3 places; GR_REGS or none. | ;; 92 preferences: 4 16 17
== tmp/F770/d_pv:func_8002F770
   det: pseudo 84, seat global 2 | Register 84 used 13 times across 16 insns in block 0; GR_REGS or none. | no preference line
   sum: pseudo 92, seat global 16 | Register 92 used 6 times across 19 insns; dies in 2 places; GR_REGS or none. | ;; 92 preferences: 16 17
tmp/F770/d_reuse:func_8002F770: shift dest pseudo 239 (local seat 4), source pseudo 92
tmp/F770/d_pv:func_8002F770: shift dest pseudo 241 (local seat 4), source pseudo 237
```

- **`work` (value 1 = the determinant).** In the reuse spelling the determinant's pseudo is also
  written in the sqrt arms, so it is referenced in more than one basic block and dies twice
  (`dies in 2 places`). local-alloc.c:472 admits a pseudo to local allocation only when
  `reg_basic_block[i] >= 0 && reg_n_deaths[i] == 1`, so it is left to global.c, which seats it in
  $t2 (reg 10) — the target's `sra $t2,$v0,0xc; div $zero,$s1,$t2` register. In the per-value
  spelling the determinant's own pseudo lives only in block 0 with one death (`in block 0`), so
  local-alloc takes it and seats it in $v0 (reg 2): the split scores 35 (one instruction fewer,
  269/270).
- **`temp` (value 1 = c0*c0+c1*c1).** global.c:1484 mark_reg_store -> set_preference
  (global.c:1671-1760) takes the FIRST operand of a set's source (`(ashift (reg) 16)` ->
  the reg) and, when the destination was local-allocated to a hard register, gives the source's
  allocno that register as a preference. In the reuse spelling the source of `temp << 16` is
  temp's own pseudo and the shift's destination pseudo was local-allocated to $a0 (see the
  table), so temp's allocno gets preference {4} (`preferences: 4 16 17`) and find_reg's preferred pass
  (global.c:1133-1150) seats it in $a0 — the target's `addu $a0,$v1,$a2 ... move $t4,$a0 ...
  srlv $v0,$a0,$v1`. In the per-value spelling the shift's source is `tb`'s pseudo, temp keeps
  only {16,17} and is seated in $s0: the split scores 6.

## (D)(3) Necessity (mechanism + search, owner ruling Q31)
Both properties are properties of the variable, not of spelling details: (work) any spelling in
which the determinant has its own variable keeps every reference of that variable inside block 0
(the statement list is fixed by (C)(2)), so local-alloc.c:472 admits it; (temp) any spelling in
which the table byte has its own variable makes that variable, not temp's, the source of the
`<< 16`, so set_preference never gives temp the $a0 preference. Banked counting spellings, all
measured, none reaching the target (r11/measurements.md):
| spelling (all fresh-local, no FAKE constructs) | score |
|---|---|
| reuse body (candidate.c / candidate_r11.c, renamed) | **0** |
| per-value: `dist` for work's value 2 + `tb` for temp's value 2 (variants/pv_both.c) | 41 (one insn fewer) |
| per-value, `tb` declared at function scope (variants/pv_both_tbfn.c) | 41 |
| per-value, declaration order dist-before-sum (variants/pv_both_order.c) | 41 |
| only temp split (`tb`; variants/pv_sum.c) | 6 |
| only work split (`dist`; rejected/inline-o-h-fresh-dist-35.c) | 35 (one insn fewer) |
| table byte inlined, no second variable (rejected/inline-o-h-table-byte-inline-6.c) | 6 |

## (D)(4) Measured alternatives and permuter
- STILL OWED: the permuter campaign from variants/pv_both.c (r11/tools/mkperm.sh +
  camp.sh, as func_8002F2D0's campaign 1) — not run before the session closed.
  func_8002F2D0's identical-mechanism campaign (37,248 iterations, best 30, the gain
  re-creates the reuse) is corroboration only.

## Q30 set-aside
No spelling is set aside: no measured spelling carries a FAKE-annotated construct, and the
reuse body carries none.

## Status (2026-09-30, session close)
Work in progress, NOT submitted: candidate_r11.c is the renamed + annotated body (0); the landing still waits on the gte_rtv0 DMPSX-word owner question (docs/grind/borderline.md 2026-09-30) and on src/code6cac_b_tu2.c being freed.
