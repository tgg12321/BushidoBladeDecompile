# Evidence bank — func_800325E0

## [s1 2026-09-02, recon] Chassis + floor

- Function: `src/code6cac_b.c` (was `INCLUDE_ASM`), 149 target insns, one call
  (`func_8005C650(arg0, L, R)`). 3D positional sound pan/volume: `dx/dy/dz` = listener
  position (`D_800A36B4 + 0x20/0x24/0x28`) minus `arg1[0..2]`; range-check on dx/dz
  (`(u32)(d + 0x9C40) > 0x13880`) -> `dist_volume = 0x9C40`; else `dist_sq = dx²+dy²+dz²`,
  `< 0x400` -> byte table `D_8008D118[dist_sq] >> 3`, else GTE LZCS/LZCR island ->
  log-table shift arithmetic (verbatim the func_80032314 tail). Then
  `distance_scale = ((0x9C40 - dist_volume) << 11) / 32000` clamped >= 0; pan projection
  through the `Judge` sin/cos table at `listener_angle + 0x400` and `listener_angle`
  (`D_800A36B4 + 0x12`, s16); `pan_sign = ~pan >> 31`; abs; L = `((0x7530 - pan) << 12) / 30000`,
  R = `((0x2710 - pan) << 12) / 10000`, clamp >= 0, swap when `pan_sign`, scale by
  `distance_scale >> 16`, clamp to 0x7F, call.
- `canonical func_800325E0`: 2/149 insns canonical-asm (mtc2, swc2) — the LZCS island only;
  the rest is compiled C. Target `.s` lines 50-56: `addu $t4,$a0,$zero` / `mtc2 $t4,$30` /
  `nop` / `nop` / `addiu $v0,$sp,0x10` / `addu $t4,$v0,$zero` / `swc2 $31,0($t4)` — the
  sp_tmp slot is at 0x10($sp), NOT 0($sp) as in func_800274BC / func_80032314, so the
  authorized template needs the `addiu $v0,$sp,0x10` + `addu $t4,$v0,$zero` store-address
  preamble spelling — exactly the island the Judge passed for func_8002E838 on 2026-09-02
  (src/code6cac_b.c:1185-1201, decisions.md:20264, inline_asm_canonical.txt:373). Unlike
  func_80032314 / func_8002E838 the target has NO `bltz` guard before the island (`beqz
  $v0,.L80032694` falls straight into the `addu $t4`), so no `if ((s32)dist_sq >= 0)` arm.
- Cluster membership: enumerated by name in `tools/grinder/owner_cluster_grants.txt:24` and
  `.claude/rules/cop2-addressing-preamble-cluster.md` (LZCS/LZCR sub-family). Honest bucket
  is COMPLETED-INLINE-ASM-CANONICAL (allowlist line required).

## [s1] Floor trajectory (all `sandbox func_800325E0 --disable all`, rules_dropped 0)

| form | score | insns | note |
|---|---|---|---|
| retired chassis body (pins + `.word` island + `pan_sign asm("$7")` + do-while) | 37 (migration pin) | — | never rebuilt this session; banked in `retired-chassis-2026-08/` |
| cand1: retired body, pins dropped, island with `"r"(&sp_tmp)` %2 operand for the store address | 33 | 148/149 | the `%2` operand costs an extra insn and misplaces the preamble; banked `rejected/island-percent-operand-addressing-score33.c` |
| cand2: island respelled to the func_8002E838 form (`addiu $v0,$sp,0x10` / `addu $t4,$v0,$zero`, `"$2","$12"` clobbers) | 29 | 149/149 | ALL 29 diffs are register renames, instruction order identical (pairdiff) |
| + H1: `pan_L = (distance_scale * pan_L) >> 16;` (drop `L_scaled`/`R_scaled`) | 18 | 149/149 | pan_L/pan_R seat in the call-arg regs $a1/$a2 as target |
| + H2: `pan_sign = ~(u32)projected_pan; pan_sign >>= 31;` | **0** | 149/149 | final; re-measured 0 after the header comment |

## [s1] Register map at score 29 (ours -> target), from `tools/pairdiff.py code6cac_b func_800325E0`

dx `$t0 -> $t1`; arg0 copy `$t1 -> $t2`; distance_scale `$a3 -> $t0`; pan_sign
`$t0 -> $a3` (target computes it in place: `nor $a3,$zero,$a1; srl $a3,$a3,31`; ours
`nor $v0,...; srl $t0,$v0,31`); pan_L `$a0 -> $a1`; pan_R `$v1 -> $a2`; the two
divide-by-constant `mfhi` scratch regs follow (`$t2 -> $t3`, `$t3 -> $t1`).

## [s1] Mechanism (global.c allocno priority) — dump-confirmed

- `global.c:635-648 allocno_compare`: priority = floor_log2(n_refs) * n_refs / live_length
  * size. Allocnos are seated in descending priority, each taking the lowest-numbered free
  non-conflicting register.
- At score 18 ours seated distance_scale first (5 refs / ~63 insns -> 10/63) -> `$a3`, then
  dx (4 refs) -> `$t0`, then pan_sign (2 refs: srl def + beqz use -> 2/29) -> `$t0` (dx is
  dead by then), then arg0 (2 refs / whole function) -> `$t1`.
- Target needs pan_sign seated FIRST (-> `$a3`, the first register free of the a0-a2
  call-arg conflicts), pushing distance_scale to `$t0`, dx to `$t1`, arg0 to `$t2`. Writing
  the flag in place (`pan_sign = ~x; pan_sign >>= 31;`) makes the nor write the variable's
  own pseudo, so its refs become nor-def + srl-use + srl-def + beqz-use = 4 ->
  floor_log2(4)*4 = 8/29 > distance_scale's 10/63. The `.greg` dump of the final form
  (`tmp/grind/func_800325E0/dumps/code6cac_b.greg`, insn 208) shows
  `(set (reg/v:SI 7 a3) (not:SI (reg/v:SI 5 a1)))` — the user variable itself in `$a3`.
- Note `x >= 0` is NOT the source of nor/srl on this compiler: `mips.c gen_int_relational`
  (cmp_info table, GE row) emits `slt` + `xori 1` for GE-vs-constant. `~x >> 31` is the
  only spelling that reaches the target's `nor`/`srl` pair.
- H1 mechanism: with `L_scaled`/`R_scaled` as separate locals, pan_L/pan_R had no copy
  preference toward the outgoing arg registers and seated in `$a0`/`$v1`; reusing the
  variables makes the pseudo that reaches `func_8005C650`'s `$a1`/`$a2` the same pseudo
  that holds the unit pan, so it takes the arg register throughout (target `$a1`/`$a2`).

## [s1] Artifacts

- `tmp/grind/func_800325E0/s1/cand1.c` (score-33 form), `fix_island.py`, `fix2.py`,
  `h1.py`, `h2.py`, `hdr.py` (the edit scripts, in order).
- `tmp/grind/func_800325E0/dumps/code6cac_b.{greg,lreg,combine,sched,...}` — cc1 -da dumps
  of the FINAL (score-0) form (dump.ps1 prints two pre-existing TU-level "conflicting types"
  warnings for func_80032854 / D_800A391D; unrelated to this function).

## [s1-retry 2026-09-02, recon] Re-application after driver discard

- The first s1 run reached sandbox 0 but was DISCARDED by the validator: its self_vet.md SCOPE
  quotes spanned multiple lines and `tools/grinder/grindlib.py` `_SCOPE_LINE` requires
  `SCOPE: "..."` on ONE line (regex anchored `^...$`). No code defect. Fix: single-line verbatim
  quotes (`.claude/rules/cop2-addressing-preamble-cluster.md:104`,
  `.claude/rules/ordinary-c-judge-decidable.md:157`).
- Re-applied `candidate.c` over `INCLUDE_ASM` (`tmp/grind/func_800325E0/s2/apply.py`);
  `sandbox func_800325E0 --disable all` = **0**, 149/149, rules_dropped 0 (re-measured twice).
- Island text (11 lines, `src/code6cac_b.c:2432-2442`) diffed against the func_8002E838 island
  (`:1189-1199`): identical; only the consuming assignment after the block differs (`clz =` vs
  `lzcr =`).

## [s3 2026-09-02, recon] Layer-1 FAIL (13:26) resolved: island comment now names the SDK macro

- The 2026-09-02 13:26 layer-1 FAIL was comment-conformance only: owner Ruling A (2026-09-02,
  `.claude/rules/cop2-addressing-preamble-cluster.md:163-176`) requires every island comment to
  name the Sony PsyQ GTE macro and header line it reproduces; the s1/s2 comment named only the
  sibling func_800274BC. Instructions unchanged.
- Macro identification (verified against the PsyQ 4.5 headers vendored at
  `tmp/grind/motion_SetMotion/s7/repos/rood-reverse/include/psx/`, `$PSLibId: Run-time Library
  Release 4.5$`): the 7-insn island is **`gte_Lzc(r1, r2)`** (`gtemac.h:174-178`) =
  `gte_ldlzc(r0)` = `mtc2 %0,$30` (`inline_c.h:228-231`) + `gte_nop()` x2 = `nop`
  (`inline_c.h:1346-1347`) + `gte_stlzc(r0)` = `swc2 $31,0(%0)` (`inline_c.h:1318-1322`).
  The `addu $t4,%1,$zero` and `addiu $v0,$sp,0x10` / `addu $t4,$v0,$zero` GPR insns are the
  operand materialisations (the cop2-addressing-preamble idiom the cluster grant covers), not
  macro text; nothing else is in the island. Note: there is no `gte_lzc` (lowercase) macro —
  the load is `gte_ldlzc`; the composite is `gte_Lzc` in gtemac.h.
- Re-applied candidate.c over `INCLUDE_ASM` with the new comment
  (`tmp/grind/func_800325E0/s3/apply.py`, `fixindent.py`): `sandbox func_800325E0 --disable all`
  = **0**, 149/149, rules_dropped 0; `canonical` = 2/149 (mtc2, swc2 only).
- Driver ban tripwire checked mechanically (`tmp/grind/func_800325E0/s3/check_vet.py` calling
  `grindlib.check_banned_constructs`): ok — the vet's CONSTRUCTS block declares the island by
  macro name and does not echo the banned comment text. All six PRECEDENT file:line cites exist;
  both SCOPE quotes are single-line.
- Ban entry semantics: the banned construct is the island *with the old comment* (the FAIL's own
  "Next action" says fix the comment and identify the macro). The island body is admitted by the
  same grant that passed func_8002E838 (`inline_asm_canonical.txt:373`, decisions.md:20264).

## [s4 2026-09-02, recon] Second driver discard resolved: phantom third FAMILY line in self_vet.md

- The s3 session was DISCARDED by the validator: "claims 3 sanctioned family/families but quotes
  only 2 verbatim SCOPE sentence(s)". Root cause (measured with
  `tmp/grind/func_800325E0/s4/famcount.py`, which runs grindlib's own `_FAMILY_BLOCK` regex
  `(?im)^\s*FAMILY\s*:\s*(.+)$`): the T5 prose "It matches no forbidden\nfamily: no pin, ..."
  wrapped so that `family:` began a line, and the case-insensitive regex counted it as a third
  FAMILY block. No code defect and no family-claim defect. Fix: reworded that line to
  "construct class: ..."; famcount now reports FAMILY@71, FAMILY@86, SCOPE@72, SCOPE@87 (2/2).
- Lesson for every future vet on any function: never let the bare word `family:` (any case)
  start a line outside the SANCTIONED-FAMILY-CLAIMS blocks — the validator counts it.
- Re-applied `candidate.c` over `INCLUDE_ASM` (`tmp/grind/func_800325E0/s4/apply.py`):
  `sandbox func_800325E0 --disable all` = **0**, 149/149, rules_dropped 0;
  `canonical func_800325E0` = 2/149 canonical-asm (mtc2, swc2 — the gte_Lzc island only).
  `grindlib.check_banned_constructs` ok; all six PRECEDENT file:line cites exist
  (s3/check_vet.py). Candidate text unchanged from s3.
