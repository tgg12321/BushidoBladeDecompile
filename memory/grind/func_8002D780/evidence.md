# Evidence bank — func_8002D780

## s1 (2026-09-08, recon) — chassis: HEAD main @ 543ed812 (-mel -msoft-float), INCLUDE_ASM on main

OBJECT MODEL: the target touches ONE global, `D_8008D118` (census g_isqrt_lut, byte LUT).
The target addresses it as `lui $at,%hi; addu $at,$at,<idx>; lbu <r>,%lo(D_8008D118)($at)`
at both use sites (asm/funcs/func_8002D780.s L109-L111, L125-L127), i.e. a u8 array
indexed by a register. src/code6cac_b.c:279 declares it TU-local as `extern u8 D_8008D118;`
and every byte-matched user in this TU (func_800274BC:285, func_8002BC68:748,
func_8002D320:1070, func_8002D518:1237) spells the access as `(&D_8008D118)[idx]` /
`*(((u8 *)&D_8008D118) + idx)`. Verdict: **MATCHES** — the s1 baseline reproduces both LUT
sites byte-for-byte (pairdiff_baseline.txt has no lui/addu/lbu diff lines; the only
region-2 diffs are the `a0` copy of `dist` and the sp+16 slot). No header declaration is
needed; nothing to fix on the data-model axis.

### Baseline
- Retired body (retired-chassis-2026-08/body.c, 65 regfix rules on the old chassis) ported
  to the sibling func_8002D320's cop2 spelling (owner-authorized canonical form,
  `.claude/rules/cop2-addressing-preamble-cluster.md`): canonical gate = ASM-PARTIAL
  (8/202 insns GTE/cop2), sandbox `--disable all` = **48/202**, build_insns == target_insns
  (202) — every residual is order/register, not structure. Diff: tmp/grind/func_8002D780/s1/pairdiff_baseline.txt.
- The residual is three disjoint clusters (target insn indices):
  1. **48-111, first triangle side-test (block 5 of sched1)**, ~38 insns. Target multiply
     order: z0*cx (->t6), lw px, z0*px (->a0), lw pz, x0*pz (->v0), cz fixup (sra/subu),
     x0*cz (->s1), `subu v1,a0,v0` (cross_p), `mflo s1`, `subu v0,t6,s1` (cross_c), xor, bltz.
     Ours (baseline): z0*cx, cz fixup, x0*cz, lw px, z0*px, lw pz, x0*pz, cross_c, cross_p.
     Plus a register permutation that follows from it: target x0->t1, z0->a3, z2->t0, cx->t2,
     cz->a2, first mult sum in v0; ours x0->t0, z0->a2, z2->a3, cx->t1, cz->t2, sum in v1.
     Tests 2 and 3 (target 80-111) match ours in ORDER; only the register names differ.
  2. **116-140, sqrt path**, ~5 insns: target copies `dist` into a0 (`move a0,s1` in the
     beqz delay slot, stolen from the else-thread) and the LZCS input / `srlv v0,a0,v1`
     read a0 while the LUT index and the call args keep s1. Target LZCR slot is sp+16
     (`addiu v0,sp,16; move t4,v0`) vs ours sp+0 (`move t4,sp`); target mflo of y*y is t8
     vs ours t7.
  3. **tail**, 3 insns: target's 2nd post-call `bnez` carries `li v0,1` in its slot and
     the 3rd-call `sltu` falls into the epilogue; ours fills that slot with `move a0,s2`,
     keeps a standalone `.L303: li v0,1` block and a `j` over it.

### Pass attribution (cc1 -da dumps: tmp/grind/func_8002D780/dumps + s1/f.<pass>)
- Cluster 3 = **reorg.c** (dbr). Instrumented trace (BB2_DBR_DEBUG=1, s1/dbrdbg_all.txt
  L1961-1972): branch 345 (after call 1): thread=383 (`li v0,1`), own=0, setsopp=0 ->
  WINNER (steals li, redirects to the end label). Branch 381 (after call 2): thread=383,
  own=1, opposite=390 (call-3 block head), oppregs=202e0004 -> **bit 2 ($v0) reported
  live at the call-3 block entry**, so `li $2,1` sets an opposite-needed resource ->
  LOSE; the fall-through `move a0,s2` (insn 390) wins instead. For branch 345 the
  opposite (369, call-2 block head) has oppregs=20370000 (no $v0). Both branches carry
  identical REG_DEAD $v0 notes (f.jump2 L1058-1066, L1145-1151); the liveness difference
  comes from mark_target_live_regs (tools/gcc-2.7.2/reorg.c:2560-2760): both targets
  resolve to basic block 11 and the call-3 block has no CODE_LABEL after jump1 relocated
  the shared `v0=1` block behind the sltu block (f.jump: label 387 moved after jump 407,
  insn 385 deleted), so the pending-dead $v0 from branch 381 is never flushed. The
  target's `bnez; li v0,1` + fall-through sltu is exactly what a successful steal +
  own-thread delete + relax_delay_slots produces, so the target compile had $v0 NOT live
  there.
- Cluster 1 = **sched1** (first scheduling pass, block 5 = insns 99-154). All four test-1
  multiplies have priority 13 and the two cross subu's 24 (f.sched L115-L137). The
  backward list scheduler breaks priority ties by dependence class then by INSN_LUID
  (tools/gcc-2.7.2/sched.c rank_for_schedule; RANKDBG hook via BB2_RANK_DEBUG=1). With
  RTL emission order z0*cx(141), x0*cz(143), z0*px(147), x0*pz(149) the tie-break yields
  our order; the target's order needs x0*cz emitted last (or on a shorter path). The
  register permutation in cluster 1 is downstream of the schedule (local-alloc qty order
  follows insn order), so it is NOT a separate RA hypothesis until the schedule matches.
- Cluster 2 = an un-coalesced second pseudo for `dist` (global.c conflict: `dist` stays
  live across the three calls in $s1, the copy dies before them -> $a0). This is the
  IDENTICAL residual the byte-matched sibling func_8002D518 closed in its s8 synthesis
  (src/code6cac_b.c:1244-1265: `ud = disc` copy kept alive by a FAKE dead-store re-store,
  32 rejected plain-C placements, 33,881 permuter iterations). A plain `u32 ud = dist;`
  copy folds here too (v3 measured 48 = baseline; cse.c make_regs_eqv deletes the copy).

### Measurements (tools/sweep_variants.py, s1/variants/, s1/pairdiff_<v>.txt)
| form | score | note |
|---|---|---|
| baseline (retired body, sibling cop2 spelling) | 48 | pairdiff_baseline.txt |
| v4 xor operands swapped `(cross_p ^ cross_c)` in ALL three tests | **47** | test-1 order becomes cx-mult, z0*px, x0*pz, cz fixup, x0*cz... (pairdiff_v4_xor_swap_all.txt); tests 2/3 still match |
| v1 same swap in test 1 only | 48 | 70 differing insns (vs 73) but same score |
| v2 px/pz read through `s32 *p` at use sites (no locals) | 48 | no change; cse/sched already place the loads |
| v3 plain `u32 ud = dist;` in the else arm | 48 | copy folded by cse (sibling func_8002D518 finding reproduced on this chassis) |
| v5 tail as nested `== 0` ifs with trailing `return 1` | 48 | identical RTL after jump1 (same relocation) -> same reorg result |

### Cross-knowledge
- func_8002D320 (same file, COMPLETED-C) is the sibling: identical prologue/GTE block and
  sqrt path; its tail diamond needed a FAKE dead-store. func_8002D518 (COMPLETED-C) is the
  callee and carries the `ud` copy FAKE. func_8002CA8C's notes describe the caller
  dispatch (`a1 != 0` -> this function).
- Args: (flag, obj, pos, threshold, r_sq); `r_sq` is the 5th arg read from 0x48($sp) into
  $a1 in the prologue (target L4), so `dist = r_sq - y*y` is `subu s1,a1,t8`.

- [s1] OBJECT MODEL: D_8008D118 (g_isqrt_lut) MATCHES — TU-local `extern u8 D_8008D118;` (src/code6cac_b.c:279) reproduces both lui/addu/lbu LUT sites byte-exact, same spelling as byte-matched siblings func_8002D320/func_8002D518.

- [s1] canonical gate: ASM-PARTIAL, 8/202 insns GTE/cop2 (mvmva + LZCS/LZCR idiom, owner-authorized canonical form per cop2-addressing-preamble-cluster).

- [s1] baseline honest floor 48/202 with build_insns == target_insns (202): residual is order/register only, in three disjoint clusters (target insns 48-111, 116-140, tail).

- [s1] cluster 1 attributed to sched1 block 5: all four test-1 multiplies priority 13, subu's 24; target order z0*cx, z0*px, x0*pz, cz-fixup, x0*cz vs ours z0*cx, cz-fixup, x0*cz, z0*px, x0*pz; register permutation follows the schedule.

- [s1] cluster 3 attributed to reorg.c: instrumented trace shows branch 381 loses the `li v0,1` steal because $v0 is reported live at the call-3 block (oppregs=202e0004) while branch 345 (oppregs=20370000) wins; identical REG_DEAD notes on both branches; call-3 block has no label after jump1's relocation.

- [s1] cluster 2 is the sibling func_8002D518's closed residual (ud copy kept by a FAKE dead-store re-store, src/code6cac_b.c:1244-1265); a plain copy folds on this chassis too.

## s2 (2026-09-08, structural) — chassis: HEAD main @ 25c0188a (-mel -msoft-float), s1 candidate applied

FLOOR 47 -> 10 (build_insns == target_insns == 202 throughout). Every number below is a
`sandbox func_8002D780 --disable all` score measured this session via
`tools/sweep_variants.py` (8 batches, 40 variants, generators + variant bodies in
tmp/grind/func_8002D780/s2/).

### The three levers that moved it
1. **NAMED CROSS-PRODUCT LOCALS (47 -> 23).** Each triangle side test is a sign
   comparison between two 2-D cross products: the point-side one (`z0*px - x0*pz`) and
   the centroid-side one (`z0*cx - x0*cz`). s1 wrote both inline inside one nested
   expression. Computing them into two named locals (`kc` centroid, `kp` point) as
   separate statements before the `if` gives sched1 the target's block-5 multiply order
   (z0*cx, lw px, z0*px, lw pz, x0*pz, cz fixup, x0*cz) instead of s1's interleaving.
   Variant e_named_cross = 23.
2. **CENTROID TERM FIRST + REUSE (23 -> 14).** Writing the test as `(kc ^ kp) >= 0`
   (centroid cross product as the XOR's first operand, matching the operand order the
   target's three tests share) and REUSING the same two locals for tests 2 and 3
   measured 14 (variant e2_reuse_cfirst_xor). Controls: `(kp ^ kc)` = 23; fresh locals
   per test = 52; p-assigned-before-c = 47; named intermediates in test 1 only = 48.
   The reuse is not a codegen trick in search of a purpose — it is how the three tests
   read as one repeated computation — but it IS load-bearing (fresh locals cost 38).
3. **SIBLING LZCS CLOBBER FOOTPRINT (14 -> 10).** The LZCS island's clobber list
   `"$12","$13","$14","$15"` (the byte-matched sibling spelling on main for the SAME
   island in this TU: func_8002BC68 src/code6cac_b.c:762-767, func_8002BEA0 :825-830,
   carrying the 2026-07-28 judge ruling that RTL mention of $13-$15 is the only route
   by which reload1.c picks $24) turns our `mflo $t7` ($15) into the target's
   `mflo $t8` ($24) and takes two further permutation insns with it. Variant
   g1_clobbers = 10 at 202 insns. THIS IS AN INHERITED SIBLING CONSTRUCT, not a new
   one: any candidate-ready session must cite the in-line ruling record at
   src/code6cac_b.c:751-758 (the identical comment block ships on main).

### Byte-neutral hygiene adopted
The redundant `(u8 *)` casts on `&D_8008D118` are dropped (`extern u8 D_8008D118;` at
src/code6cac_b.c:279 already makes `&D_8008D118` a `u8 *`). Measured identical (10,
variant p1_no_pun_cast) and it removes the two declaration puns the s2 brief flagged.
The sibling func_8002BC68's staged-u32 sqrt chain (`m = (u32)-2; m &= lzcr; sh = 0x16 - m;
idx = dist >> sh; ...`) was transplanted per the sibling mandate and measured 10 —
byte-neutral here, so the simpler s1/func_8002D320 spelling is kept.

### The remaining 10 insns (tmp/grind/func_8002D780/s2/pairdiff_g1.txt)
- **A (2 insns), edge-test delay slot.** Target fills the test-2 `bltz` slot with
  `subu v0,t2,t1` (cx - x0) and emits `subu a0,t0,a3` (dz = z2 - z0) after it; we emit
  dz first, so reorg steals dz instead. Everything else in test 3 matches insn-for-insn.
- **B (5 insns), cluster 2.** Unchanged from s1: the target keeps a second pseudo
  holding `dist` in $a0 (`move a0,s1` in the `beqz` delay slot) feeding the LZCS input
  and the `srlv`, while $s1 keeps the compare, the small-path LUT index and the call
  arguments; and its LZCR frame slot is addressed `addiu v0,sp,16; move t4,v0` where we
  emit `move t4,sp`. NOTE (new this session): our `lw` of the slot already reads
  `16($sp)` — only the asm's address computation differs, and passing `"r"(&sp_var)` as
  a third operand DOES produce the target's two-insn form, but costs one insn overall
  (203) because our tail still carries the extra `j` from cluster C. A and C must land
  together with it for the count to stay at 202.
- **C (3 insns), tail.** Unchanged from s1: reorg.c will not steal `li v0,1` into the
  second post-call branch's slot. Six further tail shapes measured this session
  (goto-yes label, nested `== 0`, truthy `if`, result local, ternary, `!!`) — all 10.

- [s2] FLOOR 47 -> 10 (202/202 insns) via three ordinary-C/inherited-sibling levers: named cross-product locals per side test, centroid term first in the XOR + locals reused across the three tests, and the sibling LZCS clobber footprint $12-$15.
- [s2] Named cross-product locals are worth 24 insns; the XOR operand order worth 9; the sibling clobber list worth 4. Fresh locals per test instead of reuse costs 38 (52 vs 14).
- [s2] The LZCR frame slot is at 16($sp) in BOTH builds — only the asm's address computation differs; `"r"(&sp_var)` reproduces the target's `addiu v0,sp,16; move t4,v0` but pushes build_insns to 203 until the tail (cluster C) also lands.
- [s2] Cluster 2's `dist` copy still folds under four more natural spellings (u32/s32 copy in the else arm, copy declared in the outer block, copy used only as the srlv index) — cse.c make_regs_eqv, same as s1's v3.

- [s2] s2 floor: 47 -> 10 of 202, with build_insns == target_insns == 202 at every step; verified this session with the edits in src/code6cac_b.c (sandbox func_8002D780 --disable all -> score 10) before src was restored to its committed INCLUDE_ASM state.

- [s2] The single biggest lever was structural and ordinary: each triangle side test's two 2-D cross products computed into named locals as separate statements (47 -> 23), with the centroid-side product as the XOR's first operand and the two locals reused across the three tests (23 -> 14).

- [s2] Reuse of the cross-product locals across the three tests is worth 38 insns against fresh locals per test (14 vs 52) — the next session must not 'clean it up' into per-test locals.

- [s2] The LZCS island's clobber list is an INHERITED sibling construct, not a new one: $12-$15 is what the byte-matched func_8002BC68 (src/code6cac_b.c:762-767) and func_8002BEA0 (:825-830) ship on main for the identical island, with the 2026-07-28 judge ruling recorded in the in-line comment at src/code6cac_b.c:751-758. It is worth 4 insns here (mflo $t7 -> $t8 plus two permutation insns).

- [s2] The LZCR frame slot is at 16($sp) in BOTH builds — only the asm's address computation differs. Passing "r"(&sp_var) reproduces the target's addiu v0,sp,16 / move t4,v0 exactly, but pushes build_insns to 203 while the tail still carries its extra j: residuals B and C are coupled by the insn count and must land together.

- [s2] Nine natural spellings of the cluster-2 dist copy are now measured dead across s1 and s2 (plain u32/s32 copies, outer-block declaration, index-only use, pointer reads); cse.c make_regs_eqv folds every one.

- [s2] Eleven tail spellings are now measured dead across s1 and s2 (nested == 0, goto-to-shared-label, truthy if, result local, ternary, !!, result carrier, p10C hoist, ...); the mechanism is jump1's relocation of the shared li v0,1 block, not the tail's C shape, so the next probe should attack the cross-jump rather than the spelling.

## s3 (2026-09-08, structural) — chassis: HEAD main @ 6d8020d9 (-mel -msoft-float), s2 candidate applied

FLOOR 10 -> 7, with build_insns == target_insns == 202. Every number below is a
`sandbox func_8002D780 --disable all` score measured this session (28 variants in 6
batches; generators + variant bodies + pairdiffs in tmp/grind/func_8002D780/s3/).

### THE TAIL RESIDUAL (cluster C, 3 insns) IS CLOSED — and it unlocked residual B
s1 and s2 measured ELEVEN tail SPELLINGS at no change and concluded the mechanism was
jump1's relocation of the shared `li v0,1` block, not the tail's C shape. That was
right, and the fix is a BLOCK-STRUCTURE change, not a respelling: write the third
`func_8002D518` call as a test with an explicit fall-out

    if (func_8002D518(sqrt_val, dist, p10C, p124) != 0) return 1;
    return 0;

instead of `return func_8002D518(...) != 0;`. All three post-call exits then have the
same shape, jump.c cross-jumps them differently, and reorg.c fill_simple_delay_slots
steals `li v0,1` into the SECOND post-call branch's delay slot exactly as the target
does. The standalone `.L303: li v0,1` block and the `j` over it are both gone.
Measured 7 at 201 insns (variant c1_third_call_if_form).

201 insns is ONE SHORT of the target's 202 — which is precisely the coupling s2
predicted. Adding the s2-banked LZCR slot-address operand (`"r"(&sp_var)` as a third
asm operand of the LZCS island, which reproduces the target's `addiu v0,sp,16;
move t4,v0` and which s2 measured at 203 insns and could not adopt) takes the count
back to exactly 202 at the same score 7 (variant d1_c1_plus_slotaddr). Residuals B and
C were coupled by the insn count in BOTH directions: C had to land before the slot
operand could be paid for, and the slot operand is what restores the count.

Note the corollary for the next session: our `nop` in the beqz delay slot and the
target's `move a0,s1` are now the ONLY thing separating the two sqrt blocks; the LZCR
slot addressing, the `mflo t8`, the `lw 16($sp)` and the whole tail all match.

### Residual A is now an ALLOCATION question, not a scheduling one
The test-3 block is a 2-D cross product of two vertex-0-relative vectors. Writing it
that way — six named difference locals, `ax`/`az` = centroid relative to vertex 0,
`bx`/`bz` = the point, `dx`/`dz` = the 0->2 edge — is byte-neutral on score (7) but
FIXES the first of the block's two subu-pair order diffs: the target's
`subu v0,t2,t1` (cx - x0) now precedes `subu a0,t0,a3` (z2 - z0) insn-for-insn
(variant e2_full_relative_vectors, pairdiff_e2.txt). The residual is the SECOND pair:
the target emits `subu v1,t5,t1` (dx = x2 - x0) before `subu v0,a2,a3` (az = cz - z0);
we emit az first.

Declaring dx ahead of az DOES produce the target's order — variants
f1_ax_dz_dx_az_bx_bz and f3_ax_dx_dz_az_bx_bz both emit dx then az, matching the
target through the whole test-3 block (pairdiff_f1.txt / pairdiff_f3.txt) — but they
permute dz and dx between $a0 and $v1 (ours dz->$v1, dx->$a0; target dz->$a0,
dx->$v1), which costs 7 and takes the mflo/mult chain with it. So the two-insn
residual A is now precisely: get the target's emission order (dx second) AND the
target's allocation (dz->$a0, dx->$v1). This is a local-alloc/global.c question and
the designated tool is tools/ra_solver (inverse_compose.py classify on the dz/dx
seats), not more source spellings.

Pass attribution for the order itself (cc1 -da, tmp/grind/func_8002D780/dumps/): both
subus sit in sched1 basic block 7 with priority 1 and identical function units, so
rank_for_schedule (tools/gcc-2.7.2/sched.c) falls through the priority and dependence-
class tests to the INSN_LUID tie-break, and schedule_select's "largest potential
hazard" rule (sched.c:2708-2723, the `;; insn N has a greater potential hazard` line in
code6cac_b.sched block 7) decides the pair. Empirically the LOWER-LUID insn of the pair
lands FIRST in the block — which is why declaration order controls the order and why
it also controls the allocation.

### Residual B: five more copy spellings dead (fourteen total across s1-s3)
Re-measured on the 7-floor chassis per the chassis-relative-kill rule: a `mag = dist`
copy at the head of the else arm covering both else-arm uses; the same copy declared in
the outer block right after `dist`; the copy used only as the asm input; the copy used
only as the srlv index; and a `u32 mag = (u32)dist` covering both. ALL measured 7 =
control. cse.c make_regs_eqv folds every one, reproducing s1's H-s1-c and s2's H-s2-j
on the new chassis.

- [s3] FLOOR 10 -> 7 of 202 (build_insns == target_insns == 202), measured this session with the edits in src/code6cac_b.c before src was restored to its committed INCLUDE_ASM state.
- [s3] The tail residual (cluster C, 3 insns) is CLOSED by writing the third func_8002D518 call as `if (... != 0) return 1; return 0;` instead of `return ... != 0;` — a block-structure change, not one of the eleven tail spellings s1/s2 killed. reorg.c now steals `li v0,1` into the second post-call branch's slot as the target does.
- [s3] That change alone lands at 201 insns, which is what finally pays for the s2-banked `"r"(&sp_var)` LZCR slot-address operand (s2 measured it at 203 and could not adopt it): together they are 7 at exactly 202. Residuals B and C were coupled by the insn count in both directions.
- [s3] Writing test 3 as the cross product of two vertex-0-relative vectors (six named difference locals ax/az/bx/bz/dx/dz) is score-neutral but fixes the FIRST of residual A's two subu-pair order diffs: `subu v0,t2,t1` now precedes `subu a0,t0,a3` insn-for-insn.
- [s3] Declaration order of those six locals is load-bearing: ax,az,bx,bz,dz,dx = 7 and ax,az,dz,dx,bx,bz = 7; every other order measured, and every form that inlines az and/or bz, measured 14.
- [s3] Residual A is no longer a scheduling question: f1/f3 (dx declared before az) DO produce the target's emission order for the whole test-3 block, but permute dz and dx between $a0 and $v1 (target dz->$a0, dx->$v1), costing 7. The next probe is tools/ra_solver on those two seats, not more source spellings.
- [s3] Five more spellings of the residual-B `dist` copy (mag copy in the else arm, in the outer block, asm-input-only, srlv-index-only, u32) all measured 7 = control — fourteen natural copy spellings are now dead across s1-s3; cse.c make_regs_eqv folds every one.

- [s3] [s3] FLOOR 10 -> 7 of 202 with build_insns == target_insns == 202, measured this session with the edits in src/code6cac_b.c (sandbox func_8002D780 --disable all -> score 7) before src was restored to its committed INCLUDE_ASM state.

- [s3] [s3] The tail residual (cluster C, 3 insns, flat since s1) is CLOSED: writing the third func_8002D518 call as `if (... != 0) return 1; return 0;` instead of `return ... != 0;` makes reorg.c steal `li v0,1` into the second post-call branch's slot as the target does. Eleven tail SPELLINGS were dead; the fix was a block-structure change, not a respelling.

- [s3] [s3] That change alone lands at 201 insns, which is what finally pays for the s2-banked "r"(&sp_var) LZCR slot-address operand (s2 measured it at 203 and could not adopt it). Together they are 7 at exactly 202: residuals B and C were coupled by the insn count in both directions.

- [s3] [s3] Writing test 3 as the cross product of two vertex-0-relative vectors (six named difference locals) is score-neutral but fixes the first of residual A's two subu-pair order diffs: subu v0,t2,t1 now precedes subu a0,t0,a3 insn-for-insn.

- [s3] [s3] Declaration order of those six locals is load-bearing: ax,az,bx,bz,dz,dx = 7 and ax,az,dz,dx,bx,bz = 7; every other order measured, and every form inlining az and/or bz, measured 14.

- [s3] [s3] Residual A is no longer a scheduling question: f1/f3 (dx declared before az) DO produce the target's emission order for the whole test-3 block but permute dz and dx between $a0 and $v1 (target dz->$a0, dx->$v1), costing 7. The next probe is tools/ra_solver on those two seats, not more source spellings.

- [s3] [s3] Five more spellings of the residual-B dist copy all measured 7 = control - fourteen natural copy spellings are now dead across s1-s3; cse.c make_regs_eqv folds every one.

- [s3] [s3] Everything else in the sqrt region now matches the target (LZCR slot addressing addiu v0,sp,16 / move t4,v0, mflo t8, lw 16($sp), the whole tail): the only diff left there is our nop in the beqz delay slot vs the target's move a0,s1.

- [s3] [s3] Pass attribution read from cc1 -da (tmp/grind/func_8002D780/dumps/code6cac_b.sched, basic block 7): both test-3 subus carry priority 1 and the same function unit, so rank_for_schedule falls through to the INSN_LUID tie-break and schedule_select's largest-potential-hazard rule (tools/gcc-2.7.2/sched.c:2708-2723) decides the pair; empirically the lower-LUID insn of a pair lands first in the block.
