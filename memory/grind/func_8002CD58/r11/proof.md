# func_8002CD58 — Ruling 11 package for `len` and `temp`

laneB, 2026-10-01. Landing body: memory/grind/func_8002CD58/candidate.c (the 2026-09-25 GTE islands
unchanged; `angle` gone as one name per write, yaw/pitch/nyaw/npitch, per the 2026-09-30 ff-b
measurements; sqrt-table reads spelled `g_sqrt_table_u8[i]`). It replaces the retro-audit FAIL body
(rejected/retro-audit-2026-09-30.c, `dist` with THREE values and no admitting ruling) and the Q51
attempt (rejected/ff-b-q51-citation-fail-0.c). Every score below is
`sandbox func_8002CD58 --disable all --candidate <file>` on main (tools/sandbox_sweep.ps1, 352/352
insns for every spelling); the per-spelling list is r11/scores.txt.

## The two variables and their values

| variable | value 1 (writes -> reads) | value 2 (writes -> reads) |
|---|---|---|
| `len` | sqrt(len_sq) = \|n\|, written in both arms of `if ((u32)len_sq < 0x400)` -> the `(u32)len < 0x4000` guard | sqrt(xz_sq) = \|a.xz\|, written in both arms of `if ((u32)xz_sq < 0x400)` -> `ratan2(a.y, len)` |
| `temp` | `n.x*n.x + n.z*n.z` of the scaled n -> the `< 0x400` test, the small-value table index, the `>= 0` test, gte_ldlzc's input, `(u32)temp >> shift` | the table byte `g_sqrt_table_u8[(u32)temp >> shift]` -> `(u32)(temp << 16) >> ...` |

The fallback's square root (\|n.xz\|) has its own variable `nxz_len`: sharing it with `len` is not
needed (r11/variants_extra/a3_abl_v2.c / a3_abl_v3.c: `len` shared with either later value scores
0), so the body carries the smaller reuse.

- (A) `len` is a fresh local declared once in the `n`-small block, the innermost scope enclosing all
  four of its writes; `temp` is a fresh function-scope local (its first write is at function level).
  Neither is static/register; neither address is taken; no other declaration moved (the per-value
  spellings below have the same declarations plus the new variable).
- (B)(1) every write is read: value 1 of `len` by the guard on every path, value 2 by ratan2; value 1
  of `temp` by the test that follows it, value 2 by the `<< 16` on the next line. (B)(2) no write
  re-stores a held value: each write is a new square root / sum / table load (Ruling 5 2(c)).
- (C)(1)-(2) the one-variable-per-value spelling (r11/variants/pv_both.c: `len` + `xz_len`, `nxz_sq`
  + block-local `tbl`) has the same statement list; only declarations and identifiers differ
  (diff it against r11/variants/reuse.c = candidate.c's body without the two comments).
  (C)(3) all four values are real computations whose instructions are in the target: the table
  loads / shifts (asm/funcs/func_8002CD58.s:104-108 and :128-133 for value 1 of `len`, :156-160 and
  :180-185 for value 2), the `addu` sum (:277) and the `lbu` (:305) for `temp`.

## (D)(1)-(2) Dumps and the deciding decisions

Commands: r11/tools/dumps_all.sh (= r11/tools/dumps.sh per variant: the sandbox-stripped TU with
the variant substituted, `tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000
-mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float -dl -dg -df`, then
`BB2_FINDREG_DEBUG=<pseudo>` with the instrumented `tools/gcc-2.7.2/cc1`, whose .s is identical),
r11/tools/cut.py, r11/tools/r11table.py. Excerpts: r11/dumps_table.txt.

- **`len`: global.c set_preference (global.c:1671-1760, called from mark_reg_store :1484) and
  find_reg's copy-preference pass (global.c:1096-1131).** In the reuse spelling `len` is pseudo
  126. Insn 316 `(set (reg:SI 5 a1) (reg/v:SI 126))` is the second ratan2 argument; set_preference
  sees a plain copy (copy = 1) into hard reg 5 and records 5 in 126's copy preferences. find_reg
  trace: `conflicts: 4 17 29; own_copy_prefs: 5; own_full_prefs: 2 5`; the copy-preference pass
  takes $a1, so value 1 is computed into $a1 (the target's `srl $a1,$v0,3` / `srlv $a1,$a0,$v0` /
  `sltiu $v0,$a1,0x4000`, asm/funcs/func_8002CD58.s:108, :133, :135). In pv_len / pv_both value 1
  is pseudo 126 alone (`used 3 times across 4 insns`): `conflicts: 17 29; own_copy_prefs: (none);
  own_full_prefs: 2 4`, and the preference pass (global.c:1133-1160) takes the lowest preferred
  free register, $v0 (global 2). Value 2 (pseudo 127) keeps copy preference 5 and $a1. Score 3 =
  the three value-1 instructions with $v0 for $a1.
- **`temp`: global.c set_preference via the `<< 16` and find_reg's preference pass.** In the reuse
  spelling `temp` is pseudo 78; insn 530 `(set (reg:SI 230) (ashift:SI (reg/v:SI 78) (const_int
  16)))` reads it, and 230 is local-allocated to $a0 (`;; Register 230 in 4.`). set_preference
  takes the first operand of the source (copy = 0) and records 4 on 78: trace `conflicts: 2 3 12
  17 29; own_full_prefs: 4 5`, the preference pass takes $a0 (global 4) = the target's
  `addu $a0,$a2,$t0` (:277) and the following $a0 uses. In pv_temp / pv_both the ashift reads the
  table byte's own pseudo (227 / 228), 78 is never the source of a $a0-seated set, and its only
  preference is 5 (expand_preferences, global.c:829-870, from the D0*D0 product pseudo 211 that
  dies in insn 465 `78 = 208 + 211`; 211's preference 5 comes from its operand 205, local-allocated
  to $a1): `own_full_prefs: 5`, seat $a1. Score 6 = the six fallback instructions with $a1 for $a0.

## (D)(3) Necessity (mechanism + search, owner Q31)

Both properties belong to the variable, not to spelling details.
- (`len`) Value 1's only insns are its two defining sets (from table-byte / shift pseudos) and the
  `ltu` guard, whose result is local-allocated; none has hard reg 5 on either side, so in any
  spelling where value 1 has its own variable set_preference never records 5 on it, and
  expand_preferences cannot pass it 5 (value 1 dies only in the `ltu`, whose destination is not an
  allocno). It reaches $a1 only by being the pseudo copied into $a1 for ratan2, i.e. by sharing the
  variable with a ratan2 argument (value 2 here, or the fallback's \|n.xz\|, a3_abl_v2.c).
- (`temp`) The only $a0-seated set in the fallback that can take the squared length as its first
  source operand is the `<< 16`; in any spelling where the table byte has its own variable, the
  `<< 16` reads that variable and the squared length keeps only preference 5.

Banked counting spellings (fresh locals, no FAKE construct), none reaching the target:

| spelling | score |
|---|---|
| reuse body = candidate.c (variants/reuse.c) | **0** |
| per-value both: `len`+`xz_len`, `nxz_sq`+`tbl` (pv_both.c) | 9 |
| per-value both, table byte read inline (pv_both_inline.c) | 9 |
| per-value both, guard as `goto fallback` (pv_both_goto.c) | 9 |
| `len` split only (pv_len.c) | 3 |
| `len` split, declarations reversed / xz_len in the guard block / at function scope (len_decl_rev / len_decl_inner / len_decl_func) | 3 / 3 / 3 |
| `len` split, both `u32` (len_u32.c) | 3 |
| `len` split, guard `<= 0x3FFF` (len_le.c) | 3 |
| `len` split, guard `>= 0x4000 goto fallback`, success path unnested (len_goto.c) | 3 |
| `len` split, success path as the else of an inverted guard (len_inverted.c) | 3 |
| value 1 renamed instead of value 2 (len_split_first.c) | 3 |
| `temp` split only, `s32 tbl` block-local (pv_temp.c) | 6 |
| `temp` split, `tbl` u32 / u8 / function-scope (temp_tbl_u32 / temp_tbl_u8 / temp_tbl_func) | 6 / 6 / 6 |
| `temp` split, byte read inline in the shift (temp_inline.c) | 6 |
| `temp` split, `tbl = byte << 16` (temp_shift_first.c) | 6 |
| 3-value chassis (`dist` names): full split / dist split / temp split (variants_extra/a3_pv_all, a3_pv_dist, a3_pv_temp) | 9 / 3 / 6 |
| 3-value chassis ablations: value 1 alone split (a3_abl_v1) | 3 |
| inline sqrt helper `static inline s32 isqrt(s32)`, no reused local (variants_extra/helper_H1, helper_H4) | 21 / 21 |
| helper writing its parameter for the byte (helper_H2, helper_H3) | 24 / 24 (354 insns) |
| ff-b 2026-09-30 split spellings (ff-b-2026-09-30/dist_split*.c, *_distsplit.c) | 3 each |
| 2026-09-25 session (hypotheses.md rows c-g, k0; rejected/fresh-tbl-site3-6.c, shared-sum-sites2-3-12.c, split-sums-mat-var-6.c) | 6-25 |

Reuse spellings that also reach 0 (not counting, recorded): a3_reuse.c (`dist` = all three square
roots), a3_abl_v2.c (`dist` = values 1 and 3), a3_abl_v3.c (`dist` = values 1 and 2, the landing
shape at function scope).

## (D)(4) Permuter

See the campaign record appended below (from r11/variants/pv_both.c's equivalent body, workspace
built by r11/tools/mkperm.sh, launched by r11/tools/camp.sh, -j2, --stack-diffs).

## Q30 set-aside
No spelling is set aside: no measured spelling carries a FAKE-annotated construct, and the body
carries none (the 2026-09-25 staged-value FAKE on `nxz_sq` is gone; that variable is now `temp`
under this ruling).

## (E) names, (F) annotation, (G), (H)
(E): `len` is a kind-name true of both values (each is the length of a vector: \|n\| and \|a.xz\|,
matching `len_sq`); `temp` is a generic scratch word. (F): the comment at each declaration names both
values and cites Ruling 11 and this file. (G): fresh layer-2 on the exact staged body. (H):
everything else judged on its merits.
