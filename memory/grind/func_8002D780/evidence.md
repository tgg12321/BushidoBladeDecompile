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

- [s4] A FAITHFUL decomp-permuter workspace for func_8002D780 now exists and is validated: tmp/grind/func_8002D780/s4/nonmatchings/func_8002D780 (base.c / compile.sh / target.o / settings.toml), built by tools/decomp-permuter/import.py --no-prune plus four hand fixes. Rebuild recipe and the four hurdles are in the s4 scratch dir (asmpragma.py, mktarget.sh, compile.sh, joinstr.py).
- [s4] Hurdle 1: src/code6cac_b.c writes its cop2 macro bodies as multi-line string literals (a raw newline inside the quotes, 17 of them at lines ~1729-1774). cpp warns and pycparser refuses. Joining each pair into one `"...\n"` literal is codegen-NEUTRAL (measured: score 7, 202 insns before and after).
- [s4] Hurdle 2: import.py's stub macro is `-D__asm__(...)=_permuter_ignore_line __asm__(__VA_ARGS__)`, which only fires on `__asm__(` - this project writes `__asm__ volatile(`, so no asm statement gets b64-encoded and base.c will not parse. tmp/grind/func_8002D780/s4/asmpragma.py rewrites all 40 asm statements in base.c into the `#pragma _permuter b64literal <b64>` form import.py would have produced.
- [s4] Hurdle 3: import.py assembles target.s with `mips-linux-gnu-as -march=vr4300`, which rejects `mvmva 1, 0, 0, 3, 0`. Build target.o instead from decomp-permuter's prelude.inc (with `.set gp=64` stripped) + asm/funcs/<func>.s with the mvmva line sed-replaced by `.word 0x4A486012`, assembled with mipsel-linux-gnu-as -march=r3000.
- [s4] Hurdle 4 (bites EVERY permuter workspace in this repo): the repo path contains spaces, and maspsx's linemarker handler does `_, num, filename = line.split()` (tools/maspsx/maspsx/__init__.py:1062), so any absolute input path makes maspsx die with "too many values to unpack (expected 3)". compile.sh must copy its input to a space-free path (/tmp) before cpp sees it.
- [s4] decomp-permuter's ast_util.extract_fn REPLACES every other function definition in base.c with a bare declaration, so the permuter's compile context is not the full TU. Measured here: for func_8002D780 that stripping is codegen-NEUTRAL - the stripped and full-TU objects differ only in branch-target text and jal relocation names (tmp/grind/func_8002D780/s4/validate3.sh). The workspace is therefore faithful and the campaign result below is not a context artifact.
- [s4] CAMPAIGN RESULT: 55,531 iterations, 12 finds, best permuter weighted score 110 against a base of 300 - and ZERO engine-metric improvement. Re-measured on the real chassis: output-110-1 = 8, output-115-1 = 8, output-115-2 = 8, output-115-3 = 9, output-125-1 = 9, output-210-1 = 7 at 201 insns; control = 7 at 202. The permuter's weighted scorer (reorderings x60) and the engine's raw differing-instruction count are ANTI-CORRELATED on this chassis: the permuter buys reordering credit with an extra raw diff.
- [s4] The one genuinely new lever the campaign surfaced is a copy spelling no prior session tried: an embedded assignment inside a condition, `if ((u32)(m = dist) < 0x400)`, rather than a plain `s32 c = dist;` declaration. Nine arrangements of it were measured on the real chassis (m carrying asm input, srlv index, small-path LUT index, call args, and combinations) - all 7 = control at 202 insns, i.e. cse.c make_regs_eqv folds it too and no `move` survives. Twenty-three dist-copy spellings are now dead across s1-s4.

- [s4] The s3 7-floor chassis reproduces exactly on HEAD 8cd6da6b: candidate.c spliced into src/code6cac_b.c gives sandbox score 7 with build_insns == target_insns == 202.

- [s4] A faithful decomp-permuter workspace for func_8002D780 now exists for the first time: tmp/grind/func_8002D780/s4/nonmatchings/func_8002D780 (base.c / compile.sh / target.o / settings.toml), with the rebuild scripts in tmp/grind/func_8002D780/s4/.

- [s4] Workspace hurdle 1: src/code6cac_b.c writes its cop2 macro bodies as multi-line string literals (17 pairs at lines ~1729-1774); pycparser refuses them. Joining each pair into one "...\n" literal is codegen-neutral (score 7, 202 insns before and after).

- [s4] Workspace hurdle 2: import.py's stub macro is -D__asm__(...)=_permuter_ignore_line __asm__(__VA_ARGS__), which only fires on __asm__( — this project writes __asm__ volatile(, so no asm statement gets b64-encoded and base.c will not parse. tmp/grind/func_8002D780/s4/asmpragma.py rewrites all 40 asm statements into the #pragma _permuter b64literal form import.py would have produced.

- [s4] Workspace hurdle 3: import.py assembles target.s with mips-linux-gnu-as -march=vr4300, which rejects `mvmva 1, 0, 0, 3, 0`. Build target.o from decomp-permuter's prelude.inc (with .set gp=64 stripped) + asm/funcs/<func>.s with the mvmva line replaced by .word 0x4A486012, assembled with mipsel-linux-gnu-as -march=r3000.

- [s4] Workspace hurdle 4 (bites EVERY permuter workspace in this repo): the repo path contains spaces and maspsx's linemarker handler does `_, num, filename = line.split()` (tools/maspsx/maspsx/__init__.py:1062), so an absolute input path makes maspsx die with 'too many values to unpack (expected 3)'. compile.sh must copy its input to a space-free path before cpp sees it.

- [s4] Campaign telemetry: 55,531 iterations, 12 finds, scores 300/300/260/240/240/215/210/125/115/115/115/110 against a base of 300; first find at 29 s, last novel find at 1804 s; harvested with --stop, campaign confirmed dead (permuter_campaign.py status: alive false).

- [s4] Re-measured on the real engine sandbox, the permuter's best finds are all WORSE than the control: 110-find = 8, 115-finds = 8/8/9, 125-find = 9, control = 7. The two scorers are anti-correlated here because the permuter charges 60 per reordering and 100 per insertion/deletion while the engine counts raw differing instructions.

- [s4] The only genuinely new lever the campaign surfaced is the embedded-assignment copy spelling `if ((u32)(m = dist) < 0x400)`; nine arrangements of it were measured on the real chassis and all well-defined ones are 7 = control at 202 insns, so cse folds it like the other twenty-two dist-copy spellings.

## s5 (2026-09-08, synthesis) - chassis: HEAD main @ 994d8b2d (-mel -msoft-float), s3/s4 candidate applied

FLOOR 7 -> 2, with build_insns == target_insns == 202 throughout. Every number below is
a `sandbox func_8002D780 --disable all` score measured this session (49 variants in 8
batches; generators + variant bodies in tmp/grind/func_8002D780/s5/, run through
tmp/grind/func_8002D780/s3/run.ps1). Control re-measured on this chassis first: 7/202.

### Residual B (the `dist` copy, 5 insns, flat since s1) is CLOSED
Twenty-three copy spellings were dead across s1-s4 and EVERY ONE OF THEM WAS A
SINGLE-DEF COPY -- which is precisely the shape cse.c make_regs_eqv folds. The
byte-matched sibling func_8002D518 closes the identical residual by making the carrier
MULTIPLY-DEFINED across the join (src/code6cac_b.c:1244-1265, a FAKE-annotated
same-value re-store under .claude/rules/dead-store-fake-exception.md). Transplanting
that shape verbatim measured 7 = control (variant d1) -- and the reason took two batches
to find:

  * THE CARRIER MUST BE DECLARED BEFORE `lzcr`. `s32 lzcr = 0; s32 m = dist;` folds
    (d1 = 7); `s32 m = dist; s32 lzcr = 0;` does not (f4 = 5 at 202 insns). Declaration
    order sets the RTL emission order of the two stores, and reorg.c steals the FIRST
    schedulable insn into the `beqz` delay slot: with the carrier first we get the
    target's `move a0,s1` there and `move v1,zero` in the `bltz` slot; with lzcr first
    the copy folds away entirely.
  * THE SECOND DEFINITION MUST SIT IN THE SAME ARM. Putting it in an `else` arm
    (`if (dist >= 0) {...} else { m = dist; }`) also defeats cse and the copy survives
    -- variant e1, tmp/grind/func_8002D780/s5/pairdiff_e1.txt shows `move a0,s1`
    materialising for the first time in five sessions -- but the arm costs an extra `j`
    plus a duplicated `li v0,-2` (204 insns, score 9). A carrier defined once in EACH
    arm survives too but emits BOTH copies (205/206 insns, score 10; d2/d3/d5/d7/e4).
  * The carrier must feed BOTH the LZCS island input and the `srlv`. Asm-input-only
    (d4) folds back to 7.

With f4 the sqrt block's `move a0,s1`, `srlv v0,a0,v1` and the LUT reload all match the
target insn-for-insn (tmp/grind/func_8002D780/s5/pairdiff_f4.txt, 6 differing insns).

### The LZCR slot-address ORDER (2 insns) is CLOSED by splitting the asm island
On the f4 chassis the remaining sqrt diff was ORDER, not content: the target emits
`addiu $v0,$sp,0x10` BETWEEN the `mtc2 $t4,$30` + nop/nop group and the
`addu $t4,$v0,$zero` + `swc2 $31,0($t4)` group (asm/funcs/func_8002D780.s L141-147).
A single asm statement cannot produce that -- an "r" operand's setup insn is always
emitted before the whole asm insn, which is why s2/s3's one-statement island with
`"r"(&sp_var)` hoisted the addiu above the mtc2. Writing the island as TWO asm
statements (the mtc2 island, then the swc2 island whose "r" operand is `&sp_var`) puts
the addiu exactly where the target has it: score 5 -> 2 at 202 insns.
Clobber placement across the two statements is not load-bearing between orderings
(g1 `"$12"` then `$12-$15` = 2; g2 reversed = 2; g3 both = 2) but SOME $13-$15 mention
is (g4, `"$12"` on both, = 6) -- the same reload1.c bad_spill_regs effect the s2 sibling
clobber footprint buys.

### Residual A (2 insns) survives twenty-two more forms
The test-3 subu pair is still the only diff: the target emits `subu v1,t5,t1` (dx) then
`subu v0,a2,a3` (az); we emit az then dx. Registers/seats are CORRECT in the 2-floor
body. Measured this session:
  * Batch A (8 variants): DECLARATION order and STATEMENT order can be decoupled
    (`s32 az;` declared with the others, assigned after dx), and it does NOT help --
    every form whose az STATEMENT follows dx's scores 14 (7-chassis) / 9 (2-chassis),
    and every form whose az statement precedes dx's holds the floor regardless of where
    az is DECLARED (a2/a6/a7 = 7). So the seat permutation follows the STATEMENT order,
    not expand_decl's pseudo numbering -- which kills the "declare early, assign late"
    lever outright.
  * Batch B/C (19 variants): the fully-inlined edge-first form
    `kc = (z2-z0)*(cx-x0) - (x2-x0)*(cz-z0)` (b3/h5) ALSO scores at the floor, but with
    the OTHER pair mis-ordered (it emits dz before ax where the target has ax then dz;
    tmp/grind/func_8002D780/s5/pairdiff_b3_inlined_edge_first_term_first.txt). The
    fully-inlined ORIGIN-first form (b1) gets BOTH pairs in the target's emission order
    and is the only form that does -- and it pays for it with the dz/dx seat flip
    (dz->$v1, dx->$a0) plus an upstream permutation in tests 1-2, scoring 23.
  * Batch H/I/J (20 variants on the 2-floor chassis): partial-local forms (ax/bx local
    with the edges inlined = 9; az/bz local = 4), fresh test-3 cross locals (2 = control),
    four further declaration orders (2), kp-assigned-first (35), az written as a negated
    reverse difference (11). Nothing beats 2.
So residual A is confirmed as the s3 diagnosis: the target's emission order and the
target's dz/dx seats are, on this chassis, mutually exclusive across 34 measured forms.

- [s5] FLOOR 7 -> 2 of 202 (build_insns == target_insns == 202), measured this session with the edits in src/code6cac_b.c (sandbox func_8002D780 --disable all -> score 2) before src was restored to its committed INCLUDE_ASM state.
- [s5] Residual B is CLOSED. All 23 previously-dead copy spellings were SINGLE-DEF copies (what cse.c make_regs_eqv folds); the sibling func_8002D518's multiply-defined carrier shape works here too, but ONLY when the carrier is declared BEFORE `lzcr` and its second definition sits inside the same `if (dist >= 0)` arm.
- [s5] Declaration order of the carrier vs `lzcr` decides WHICH insn reorg.c steals into the `beqz` delay slot: carrier first gives the target's `move a0,s1` there (score 5); `lzcr` first folds the copy away entirely (score 7).
- [s5] A carrier whose second definition sits in an `else` arm DOES survive cse (the first time `move a0,s1` has ever materialised for this function) but costs an extra `j` + a duplicated `li v0,-2` (204 insns, score 9); a carrier defined once in EACH arm emits both copies (205/206 insns, score 10).
- [s5] The LZCR slot-address residual was an asm-STATEMENT-COUNT question, not a C-spelling question: the target's `addiu $v0,$sp,0x10` sits between the mtc2 group and the swc2 group, which one asm statement cannot emit (operand setup always precedes the whole asm insn). Two asm statements reproduce it exactly: 5 -> 2.
- [s5] Splitting the LZCS island still needs a $13-$15 clobber mention somewhere (both statements clobbering only "$12" scores 6); which statement carries it does not matter (g1/g2/g3 all 2).
- [s5] Residual A: DECLARATION order and STATEMENT order of the six test-3 difference locals can be decoupled, and the seat permutation follows the STATEMENT order alone (a2/a6/a7 = control at 7 with declarations reshuffled; every az-statement-after-dx form = 14/9). The "declare early, assign late" lever is dead.
- [s5] Residual A: the fully-inlined edge-first test-3 form scores at the floor with the OTHER subu pair mis-ordered; the fully-inlined origin-first form is the only measured form with BOTH pairs in the target's emission order, and it carries the dz/dx seat flip plus an upstream permutation (23). 34 forms across s3+s5 now show the target's order and the target's seats to be mutually exclusive on this chassis.

- [s5] s5 floor: 7 -> 2 of 202 with build_insns == target_insns == 202, measured this session with the edits in src/code6cac_b.c (sandbox func_8002D780 --disable all -> score 2) before src was restored to its committed INCLUDE_ASM state. Final diff: tmp/grind/func_8002D780/s5/pairdiff_candidate_s5.txt, 2 differing instructions.

- [s5] Residual B (the `dist` copy, flat since s1, 23 dead spellings) is CLOSED. Every previously-dead spelling was a SINGLE-DEF copy, which is exactly the shape cse.c make_regs_eqv folds; a multiply-defined carrier survives.

- [s5] Declaration order of the carrier vs `lzcr` decides which insn reorg.c steals into the `beqz` delay slot: carrier declared first gives the target's `move a0,s1` (score 5); `lzcr` first folds the copy away entirely (score 7). The sibling func_8002D518's FAKE shape transplanted verbatim, with lzcr declared first, measured 7 = control.

- [s5] A carrier whose second definition sits in an `else` arm survives cse but costs an extra `j` plus a duplicated `li v0,-2` (204 insns, score 9); a carrier defined once in EACH arm emits both copies (205/206 insns, score 10).

- [s5] The LZCR slot-address residual was an asm-STATEMENT-COUNT question, not a C-spelling question: the target emits `addiu $v0,$sp,0x10` between the mtc2 group and the swc2 group (asm/funcs/func_8002D780.s L141-147), which one asm statement cannot do because operand setup always precedes the whole asm insn. Two asm statements reproduce it: 5 -> 2.

- [s5] Splitting the LZCS island still needs a $13-$15 clobber mention on one of the two statements (both clobbering only "$12" scores 6); which statement carries it does not matter (g1/g2/g3 all 2).

- [s5] Residual A: declaration order and statement order of the six test-3 difference locals CAN be decoupled, and the dz/dx seat permutation follows the STATEMENT order alone -- so the 'declare early, assign late' lever is dead and the s3 diagnosis stands.

- [s5] Residual A: 34 forms across s3 and s5 now agree that the target's test-3 emission order (dx before az) and the target's dz/dx seats (dz->$a0, dx->$v1) are mutually exclusive under source rearrangement on this chassis; the 2-floor body has the seats right and the order wrong, and h2/h5 give the complementary shapes.

- [s5] candidate.c now carries ONE FAKE-annotated construct (the same-value re-store of the local `m`, dead-store family, .claude/rules/dead-store-fake-exception.md, byte-matched in-TU precedent at src/code6cac_b.c:1244-1265) with a 23-spelling lever-exhaustion ledger cited in the annotation.

## s6 (2026-09-08, synthesis) - chassis: HEAD main @ 06451cd2 (-mel -msoft-float), s5 candidate applied

Control re-measured first: the s5 candidate.c body scores **2/202** on this chassis
(build_insns == target_insns == 202), so the floor is unchanged and every s5 conclusion
about residuals B and C still holds. 37 further variants measured in four batches
(K/L/N/O, generators + bodies in tmp/grind/func_8002D780/s6/), none below 2.

### RESIDUAL A IS NOW MECHANISM-COMPLETE: two named passes, two dumps, one tie
This is the first session that READ the pass dumps for residual A instead of inferring
it (tmp/grind/func_8002D780/s6/dumpsA = the 2-floor candidate, dumpsH1 = the h1 body,
sliced out of tmp/grind/func_8002D780/dumps by s6/slice.py).

**The scheduler half (sched1, tools/gcc-2.7.2/sched.c).** In the test-3 block (block 7)
the six difference subus and the four mults ALL carry INSN_PRIORITY 1 (dump
s6/dumpsA/sched.txt L248-258): `priority()` (sched.c:1434) is a forward longest-path
over LOG_LINKS *within the block*, and every operand of the six subus (x0,x2,z0,z2,cx,
cz,px,pz) is computed in an EARLIER block, so all six are dependence-graph leaves with
priority 1. rank_for_schedule (sched.c:2408-2464) therefore falls through the priority
test, then through the dependence-class test, and decides on INSN_LUID -- i.e. on the
statement order. That is why 34 source rearrangements across s3/s5 could only trade the
subu order against something else.

**The allocator half (local_alloc, tools/gcc-2.7.2/local-alloc.c).** dump_flow_info at
the head of the .lreg dump gives the two inputs of qty_compare_1 (local-alloc.c:1660,
priority = floor_log2(n_refs) * n_refs * size / (qty_death - qty_birth)) directly:
  * 2-floor candidate body (s6/dumpsA/lreg.txt L111-113): `Register 133 [dz] used 3
    times across 7 insns in block 7` / `Register 134 [dx] used 3 times across 6 insns`.
    dx's live range is one insn SHORTER (it is born after the first mult instead of
    before it), so dx's qty priority is strictly higher, dx is allocated first and takes
    $v1 (default REG_ALLOC_ORDER, $v0 already conflicted by ax/az/bx/bz), dz takes $a0 --
    the TARGET's seats. The price is that az's statement precedes dx's, so sched1's LUID
    tie-break emits `subu v0,a2,a3` before `subu v1,t5,t1`: the 2-insn residual.
  * h1 body, dx's statement moved ahead of az's (s6/dumpsH1/lreg.txt L105-107):
    `Register 130 [dz] used 3 times across 7 insns` / `Register 131 [dx] used 3 times
    across 7 insns` -- an EXACT TIE. qty_compare_1 falls through to its `*q1 - *q2`
    quantity-number tie-break, which favours the quantity born first (dz), so dz takes
    $v1 and dx $a0: the seats invert and the body scores 9 (14 differing insns,
    s6/pairdiff_h1.txt).
So residual A is one tie in local-alloc, not a scheduling wall: in the TARGET's emission
order dz and dx have identical n_refs (3) and identical live length (7), and the target
nevertheless allocates dx first. Any winning form must raise dx's qty priority strictly
above dz's WITHOUT perturbing the emitted stream.

### CORRECTION to the s5 frontier
The s5 frontier described `tmp/grind/func_8002D780/s5/variantsH/h2_ax_dx_dz_az.c` as a
second 2-floor body ("order right, seats wrong"). It is NOT: h1 and h2 both measure
**9** (202 insns) on this chassis. There is only one 2-floor family, the candidate's.
Do not plan an s7 probe around a complementary 2-floor body; it does not exist.

### The sched-class route is FORECLOSED BY THE MIPS BACK END (class kill)
rank_for_schedule's second test classifies each ready insn by its dependence on
`last_scheduled_insn`: class 1 (data dep, cost > 1), class 2 (anti/output dep, cost > 1),
class 3 (independent, or cost 1), highest class wins. Making az class-2 by giving it an
anti-dependence on the preceding mult (i.e. having az write a register the mult reads --
the variable-reuse spelling) CANNOT work on this target: `ADJUST_COST`
(tools/gcc-2.7.2/config/mips/mips.h:2946) sets COST = 0 for every anti- or
output-dependence, insn_cost (sched.c:1372-1425) then sets LINK_COST_FREE and returns 1,
and rank_for_schedule's `insn_cost (...) == 1` test puts the insn back in class 3. So no
C form whose only effect is a WAR/WAW dependence can move this tie-break, on any block of
any function built with this compiler.

### What was measured (37 variants, none below 2)
  * Batch K (11): product OPERAND order (`dx * az` -> `az * dx`, both terms swapped) and
    the sign-flipped cross-product pair (`kc = dx*az - dz*ax; kp = dx*bz - dz*bx`, which
    preserves `(kc ^ kp) >= 0`), plus product locals. k1/k3 = 17, k2 = 19, k4 = 32,
    k5 = 43, k6 = 24, k8 = 33, k9 = 23. TWO byte-neutral alternates at the floor:
    k7 (four named product locals p1/p2/q1/q2) = 2 and k10 (dx inlined as `(x2 - x0)` in
    both of its products, no dx local at all) = 2.
  * Batch L (11): reordering the four mults so dx DIES BEFORE dz (shortening dx's live
    range, which is the qty_compare_1 input) via named product locals and via a single
    `q2 = dx * bz` local. 9, 25, 25, 32, 32, 32, 32, 33, 36, 36. Reordering the products
    necessarily reorders the bx/bz subus too, so the 2 insns it could win are paid for
    several times over.
  * Batch N (7): raising dx's qty n_refs from 3 to 4 (floor_log2 steps at 4, which would
    more than double dx's priority) by carrying the point-side cross product kp in the
    dx variable -- motivated by the target sharing $v1 between dx, dx*bz and kp
    (asm/funcs/func_8002D780.s L107/L119/L120). n1/n2/n3 = 16 regardless of statement
    order, n4 = 41, n5 = 46, n6 = 10, n7 = 10. The extra def makes kp share dx's quantity
    and the join with the other two tests' kp costs more than the seat is worth.
  * Batch O (8): carrying dz or dx in x2/z2 (locals already live in the test-2 block), so
    that the carrier becomes multi-block, is skipped by block_alloc and is assigned by
    global.c instead -- dissolving the tie by removing one of its two operands from
    local-alloc entirely. o1/o2 = 23, o3/o4 = 38, o5/o6 = 40, and the score is IDENTICAL
    for the candidate and h1 statement orders, i.e. once a carrier is multi-block the
    emission order stops mattering at all. Controls o7 (h1) = 9 and o8 (candidate) = 2
    also establish a new byte-neutral degree of freedom: declaring the six difference
    locals UNINITIALISED and assigning them in the same order as separate statements is
    free (2 == control), so a future probe may split declaration from assignment without
    paying for it.

- [s6] Control: the s5 candidate body still measures 2/202 (202 == 202 insns) on HEAD 06451cd2. Floor unchanged this session.
- [s6] Residual A is a local_alloc qty_compare_1 TIE, read from the dumps, not a scheduling wall: in the candidate's emission order dz is 3 refs / 7 insns and dx is 3 refs / 6 insns (dx wins, target seats); in the target's emission order both are 3 refs / 7 insns and the qty-number tie-break hands $v1 to dz (seats invert, 14 differing insns).
- [s6] All six test-3 difference subus carry INSN_PRIORITY 1 because every operand is computed in an earlier block, so sched1's rank_for_schedule decides them purely on INSN_LUID (statement order) -- the reason 34 source rearrangements could only trade order against seats.
- [s6] CLASS KILL: no C form whose only effect is an anti- or output-dependence can move rank_for_schedule's class tie-break on MIPS -- ADJUST_COST (config/mips/mips.h:2946) zeroes the cost of anti/output deps, insn_cost returns 1, and the insn is classified 3 exactly as if independent.
- [s6] CORRECTION: the s5 frontier's "second 2-floor body" (variantsH/h2, 'order right, seats wrong') does not exist -- h1 and h2 both score 9 on this chassis. Only the candidate-order family reaches 2.
- [s6] Two byte-neutral alternates at the floor were found and are available as chassis for future probes: k7 (four named product locals) = 2, k10 (dx inlined in both of its products, no dx local) = 2, and o8 (split declaration/assignment of the six difference locals) = 2.
- [s6] Making the dz or dx carrier MULTI-BLOCK (carried in x2/z2) removes it from local-alloc and makes the statement order irrelevant -- the candidate and h1 orders then score identically (23 / 38 / 40) -- which independently confirms that local-alloc, not sched1, owns the seat half of residual A.

- [s6] Control: the s5 candidate body still measures 2/202 with build_insns == target_insns == 202 on HEAD 06451cd2, so the floor is unchanged and the s5 conclusions about residuals B and C stand.

- [s6] Residual A is a local_alloc qty_compare_1 tie, read from the dumps rather than inferred: candidate order dz = 3 refs / 7 insns, dx = 3 refs / 6 insns (dx allocated first, takes $v1 = target seats); target emission order dz = 3/7, dx = 3/7 (tie, quantity-number fallback gives $v1 to dz, 14 differing insns).

- [s6] All six test-3 difference subus carry INSN_PRIORITY 1 because every operand is computed in an earlier basic block, so sched1's rank_for_schedule decides them purely on INSN_LUID (statement order) -- the structural reason 34 source rearrangements could only trade emission order against seats.

- [s6] CLASS RESULT: rank_for_schedule's dependence-class tie-break is unreachable from C on this back end -- ADJUST_COST (tools/gcc-2.7.2/config/mips/mips.h:2946) zeroes the cost of every anti- and output-dependence, insn_cost returns 1, and the insn is classified 3 exactly as if independent.

- [s6] CORRECTION to the inherited frontier: the s5 'second 2-floor body' (variantsH/h2) does not exist -- h1 and h2 both score 9. Only the candidate-order family reaches 2.

- [s6] Three byte-neutral alternates at the floor are banked as chassis for future probes: k7 (four named product locals) = 2, k10 (dx inlined in both of its products, no dx local) = 2, o8 (split declaration/assignment of the six difference locals) = 2.

- [s6] Making the dz or dx carrier multi-block (carried in x2 / z2) makes the statement order irrelevant -- the candidate and h1 orders then score identically (23 / 38 / 40) -- an independent confirmation that local-alloc, not sched1, owns the seat half of residual A.

- [s6] 37 variants measured this session across four batches (K product operand order and sign-flipped pairs, L product emission order, N extra dx reference, O multi-block carrier); none scored below 2.

## s7 (2026-09-08, solver) - chassis: HEAD main @ b3a8a19a (-mel -msoft-float), s5 candidate applied

Control re-measured first: `sandbox func_8002D780 --disable all` on the s5 candidate.c body
= **2/202** (build_insns == target_insns == 202). Floor unchanged; every s5/s6 conclusion
still stands on this chassis.

This session ran the SOLVER SUITE end to end on the last residual, and it converted s6's
"one local-alloc tie" narrative into a typed, exhaustive result on BOTH halves.

### 1. Triage (mandated first step)
`inverse_compose.py classify` refuses text mode for a zero-rule function; the object route
`goal_from_tgt.py classify code6cac_b func_8002D780` returns:

    ours 202 insns, target 202 insns   [object-level: replace_with_asmfile-safe]
    FIRST DIVERGENCE: SCHED -- texts match as a multiset, order differs

i.e. the residual is PURE EMISSION ORDER: our 202 instruction TEXTS are a permutation of
the target's.

### 2. The scheduler half is EXHAUSTIVELY SOLVED and it is NOT the wall
Model: `tools/sched_solver/extract.py code6cac_b` -> tmp/grind/func_8002D780/s7/model.json
(parity=True, 134 funcs, 1586 blocks). func_8002D780 pass-1 block 7 is the test-3 block, 14
insns, and the simulator reproduces our schedule exactly (`model self-check: sim == dump
(err=None)`).

  UID map (LUID order = the candidate's statement order):
    179 ax   182 az   185 bx   188 bz   191 dz   194 dx
    197 dz*ax  199 dx*az  201 kc  204 dz*bx  206 dx*bz  208 kp  210 xor  212 branch
  ours   emitted: 179 191 197 [182 194] 199 185 204 188 206 201 208 210 212
  TARGET emitted: 179 191 197 [194 182] 199 185 204 188 206 201 208 210 212

`inverse_sched.py --goal-order <target>` returns NEGATIVE at depth 2, and
`perturb.py --atoms luid,luid_move --depth 3` also returns negative -- but that is a DEPTH
statement, not a foreclosure. tmp/grind/func_8002D780/s7/sim_luid.py drives the validated
simulator directly over ALL 720 permutations of the six difference-local statements:
**180 of the 720 reach the target's emission order exactly**, and h1
(ax,dz,dx,az,bx,bz - the s5 body) is one of them. So the scheduler half was already solved
in s5 and no further sched search is warranted.

### 3. The allocator half: dz and dx tie on EVERY qty_compare_1 input (BB2_SUGG_DEBUG)
On the h1 chassis -- the body whose sched1 output IS the target's stream -- the complete
local-alloc input table (`local_extract.py code6cac_b --func func_8002D780 --suggest`,
tmp/ra_solver_work/code6cac_b.sugg.json, block 7) reads:

    qty 1 [dz, pseudo 130]: birth 4  death 16  refs 3  size 1  minclass 1  nsugg 0  ncopysugg 0
    qty 4 [dx, pseudo 131]: birth 8  death 20  refs 3  size 1  minclass 1  nsugg 0  ncopysugg 0

Every input of qty_compare_1 (local-alloc.c:1659) is identical: floor_log2(3)*3*1/12 for
both. The comparator therefore falls through to its documented last resort
`return *q1 - *q2;` (tools/gcc-2.7.2/local-alloc.c:1684), which orders by QTY NUMBER, and
qty numbers are handed out in birth order by block_alloc's forward scan -- so dz (born
first, because the target's own stream emits `subu dz` before `subu dx`) is allocated first
and takes $v1 under the default allocation order, and dx takes $a0. The seats invert
relative to the target, which is exactly the 14-instruction h1 diff s6 measured.
Because all 180 order-reaching permutations produce the SAME scheduled stream, they all
present local-alloc with this same tied table: measured h1 = 9, p1 (h1 + fresh test-3
cross-product locals) = 9, p7 (h1 + outer kc/kp) = 9.

**The structural consequence, and the most important thing this session produced:** on the
target's own emission order, six independent block-local difference pseudos cannot produce
the target's seats, because refs (3) and span (12) are pinned by that emission order and
the suggestion columns are zero. So the original compile did not present local-alloc with
this table -- something in the original's block 7 carried an extra reference, an extra
quantity, or a copy that no longer exists in the final 202 instructions (reload/flow2
`delete_noop_moves` and reorg all remove insns AFTER local-alloc runs). That is the shape
of the missing lever, and it is where s8 should look.

### 4. The inverse RA solver's vectors, spelled and measured
`inverse.py local tmp/ra_solver_work/code6cac_b.local.json --func func_8002D780 --block 7
--swap 1,4 --depth 2` returns **REACHABLE, minimal solution size 1 atom, 35 distinct
vectors** (full report: s7/inverse_local_swap_top40.txt). The spellable families and what
they measured (all at 202 insns, batches P and Q, generators s7/gen_p.py and s7/gen_q.py):

  * `[refs_up] qty 4 (dx): refs 3->4` -- carry the test-3 kp in dx. This is the repair of
    the s6 batch-N failure: batch N used the OUTER kp (defined in all three tests), which
    merged a cross-block quantity into dx; here the test-3 cross products get FRESH
    block-local names so dx stays block-local. Measured 28 (h1 order, p2) and 28 (candidate
    order, p3); with the outer kc kept, 16 (p4/p8, reproducing batch N). Re-extracting the
    local table from p2 shows why: dx's qty is gone (merged with kp, birth 24 death 28) and
    dz's death moved 16 -> 10, i.e. the reuse also moved the SCHEDULE. The atom is not
    spellable by variable reuse without paying for it in the stream.
  * `[live_extend] qty 1 (dz): dies later` -- carry the test-3 kp in dz instead: 30 (p5).
  * `[refs_up]` via local-alloc's combine_regs -- a `dx2 = dx` copy merged into dx's qty
    would lift n_refs to 5 and the copy would end as a same-register no-op that flow2
    deletes. Measured in five placements (batch Q, q1/q2/q3/q5/q7): **byte-neutral in every
    one** -- 9 on the h1 order, 2 on the candidate order. cse.c `make_regs_eqv` folds every
    SINGLE-DEF copy long before local-alloc sees it. This is the identical mechanism s5
    documented for residual B, and it says the combine_regs route needs a MULTIPLY-DEFINED
    carrier, which this straight-line block currently offers no arm for.
  * `[live_extend] qty 1: born earlier` / `[live_shrink] qty 4` -- not spellable: dz's and
    dx's births/deaths in the target's stream are its own instruction positions.
  * `[refs_up] qty 2 / qty 3` -- the HI/LO hard-register quantities; not C-addressable.

### 5. Free byte-neutral chassis moves banked this session
Giving test 3 its own cross-product locals instead of reusing the outer kc/kp is
byte-neutral on both statement orders (p1 = 9 = p7 on the h1 order; p6 = 2 = candidate on
the candidate order), so an s8 probe may use fresh test-3 names at no cost. A single-def
copy of dz or dx is likewise free (batch Q), which makes it a zero-cost carrier slot for
any probe that can also make the carrier multiply-defined.

- [s7] Control: the s5 candidate.c body scores 2/202 on HEAD b3a8a19a (build_insns == target_insns == 202); the floor is unchanged and every s5/s6 conclusion still holds on this chassis.

- [s7] inverse_compose.py classify refuses text mode for a zero-rule function; the object route goal_from_tgt.py classify code6cac_b func_8002D780 reports 'FIRST DIVERGENCE: SCHED -- texts match as a multiset, order differs', i.e. our 202 instruction texts are a permutation of the target's.

- [s7] sched_solver model for code6cac_b extracted with parity=True (134 funcs, 1586 blocks, 8134 picks); func_8002D780 pass-1 block 7 is the test-3 block (14 insns) and the simulator self-checks against the dump exactly.

- [s7] Block-7 UID map: 179 ax, 182 az, 185 bx, 188 bz, 191 dz, 194 dx, 197 dz*ax, 199 dx*az, 201 kc, 204 dz*bx, 206 dx*bz, 208 kp, 210 xor, 212 branch. Ours emits ... 197 [182 194] 199 ...; the target emits ... 197 [194 182] 199 ... -- the whole residual.

- [s7] 180 of the 720 permutations of the six difference-local statements reach the target's emission order exactly (s7/sim_luid.py, driving the validated simulator); the s5 h1 body is one of them, so the scheduler half was already solved in s5.

- [s7] BB2_SUGG_DEBUG local-alloc input table on the h1 body, block 7: qty 1 (dz, pseudo 130) birth 4 death 16 refs 3 size 1 nsugg 0 ncopysugg 0; qty 4 (dx, pseudo 131) birth 8 death 20 refs 3 size 1 nsugg 0 ncopysugg 0 -- an exact tie on every qty_compare_1 input.

- [s7] inverse.py local --swap 1,4 returns REACHABLE with minimal solution size 1 atom and 35 distinct vectors; the C-addressable families are refs_up on dx, refs_down / live_extend on dz, live_shrink on dx, and one extra_qty vector.

- [s7] Spelling the refs_up-on-dx vector by variable reuse also moves the schedule: the local-alloc table re-extracted from p2 shows dx's quantity merged into kp (birth 24) and dz's death moved 16 -> 10, which is why it scores 28 rather than 0.

- [s7] Every single-def copy carrier (dx2 = dx, dz2 = dz, five placements) is byte-neutral because cse.c make_regs_eqv folds it before local-alloc runs -- the same mechanism s5 documented for residual B, and the reason the combine_regs route needs a multiply-defined carrier.

- [s7] Free chassis moves banked: fresh test-3 cross-product locals are byte-neutral on both statement orders (p1 = 9 = p7, p6 = 2 = the candidate), and so is a single-def copy of dz or dx.

## [s8] forensics -- the block-7 decision points, read off the instrumented cc1 dumps

Chassis re-measured at dispatch: the s5 candidate body spliced into src/code6cac_b.c scores
**2 / 202** on HEAD a91a7f44 (`sandbox func_8002D780 --disable all`, build_insns 202 ==
target_insns 202). The ledger floor of 2 is intact and every s8 measurement below is relative
to it.

### The exact two instructions, and the exact two decisions behind them

Target block 7 (test 3) is asm/funcs/func_8002D780.s L103-L123, with the operand map recovered
from the four subus: `$t1` = x0, `$a3` = z0, `$t2` = cx, `$a2` = cz, `$t3` = px, `$t4` = pz,
`$t5` = x2, `$t0` = z2. The block emits

    103 subu $v0,$t2,$t1   ax = cx - x0   (in the test-2 bltz delay slot, stolen by reorg)
    104 subu $a0,$t0,$a3   dz = z2 - z0
    105 mult $a0,$v0       dz * ax
    107 subu $v1,$t5,$t1   dx = x2 - x0
    108 subu $v0,$a2,$a3   az = cz - z0
    109 mult $v1,$v0       dx * az
    111 subu $v0,$t3,$t1   bx = px - x0
    113 mult $a0,$v0       dz * bx
    115 subu $v0,$t4,$a3   bz = pz - z0
    117 mult $v1,$v0       dx * bz

Our 2-floor body emits the identical multiset with insn 182 (`az`) and insn 194 (`dx`) swapped.
The RTL for our block is in tmp/grind/func_8002D780/s8/sched_fn.txt: pseudos 129=ax, 130=az,
131=bx, 132=bz, 133=dz, 134=dx, scheduled 179,191,197,182,194,199,185,204,188,206,201,208,210,212.

Two independent decisions produce the swap, and s7 + s8 have now typed both:

**(1) sched1 ready-list pick.** The scheduler is BACKWARD, so the insn rank_for_schedule
*prefers* is emitted LATER. At the decision that matters, last_scheduled_insn is the `dx*az`
mult (our insn 199) and the ready pair is {182 az, 194 dx}. Both have INSN_PRIORITY 1 -- every
one of the six difference subus reads both operands from an earlier basic block, so LOG_LINKS
is `(nil)` for all six and priority() (sched.c:1434, a DEPTH function over predecessors) floors
at 1. Both are data predecessors of 199 with insn_cost 1, so both classify 3
(sched.c:2429). The comparator therefore reaches `INSN_LUID (tmp) - INSN_LUID (tmp2)` and the
higher-LUID insn wins the pick. Our statement order puts dx last, so dx is preferred and lands
after az; the target needs az preferred, i.e. dx's STATEMENT before az's -- the h1 order.

**(2) local_alloc qty_compare_1.** On the candidate order the resulting stream gives dz 3 refs
over 7 insns and dx 3 refs over 6, so dx wins the priority sort, is allocated first and takes
`$v1` -- the target's seats, which is why the candidate scores 2 and its only defect is the
order. On the h1 order the stream IS the target's, and there both quantities are born 2 insns
apart with identical 3-ref / 6-span profiles, so the comparator ties and local-alloc.c:1684's
qty-number fallback hands `$v1` to dz (s7's class kill; h1 scores 9).

Residual A is therefore a genuine two-sided constraint: statement order is the ONLY C-side
input to (1), and the order that wins (1) is exactly the order that loses (2).

### What s8 eliminated

* **s6's frontier item 1 had the direction inverted.** It proposed raising *dx's* INSN_PRIORITY.
  Because the list scheduler is backward, a higher-priority insn is emitted LATER, so raising
  dx would move dx further from the target. It is *az* whose priority would have to rise, and
  az is precisely the one with no in-block operand: raising it needs an in-block producer with
  result_ready_cost 2 (a load), and every one of the six operands is also read by test 1 or
  test 2, so its load is hoisted out of block 7 by construction. Both the priority axis and the
  class axis of rank_for_schedule are therefore closed on this block shape (class-killed,
  sched.c:2408; the anti/output half was already closed in s6 by mips.h:2946).

* **Self-assign is not a lever in this compiler.** `dx = dx;` produces NO RTL insn at all --
  copyscan.py finds zero `(set (reg N) (reg N))` patterns in `.rtl`, let alone in `.cse` or
  `.lreg`. All ten batch-S variants are byte-neutral (9 on h1, 2 on the candidate order, 202
  insns each). This is the sharp distinction from s5's `m = dist;`, which is a copy between two
  DIFFERENT variables and does emit an insn.

* **An in-block double-def carrier does not survive cse.** Batch T built a block-local `c`
  initialised from one difference local and re-assigned from another, in nine placements; every
  one is byte-neutral at 202. The t1 dump is the proof: its two copies, `(insn 197 (set (reg
  135) (reg 132)))` and `(insn 200 (set (reg 135) (reg 131)))`, are present in `.rtl` and
  `.jump` and gone from `.cse` onward, while in the SAME dump the s5 `m` copies (insns 264/274,
  `(set (reg 155) (reg 143))`) survive into `.lreg`. The discriminating condition is not the
  number of definitions but whether they share a basic block: cse's per-block value table
  (insert_regs -> make_regs_eqv, cse.c:1032) deletes the first copy as a dead store and folds
  the second as a single-def copy. Block 7 has no internal control flow, so no carrier confined
  to it can reach local_alloc.

* **Borrowing test 2's `kp` to manufacture the cross-block second definition costs more than
  the residual.** Batch R measured 16 on h1 and 13 on the candidate order (202 insns each),
  with the single-def control r5 = 9 isolating the entire cost to the change in test 2's own
  codegen. The copies themselves DO vanish by final output, which confirms the second half of
  the surviving-copy route works; only the manufacture of the second definition is unaffordable
  in this spelling.

### The paradox this leaves, stated precisely for s9

A carrier that survives cse must have a definition in another basic block. A carrier that can
lend dx a reference (combine_regs) must be block-7-local, because block_alloc skips any pseudo
referenced in more than one block. Those two requirements are mutually exclusive, so the
reference-count route into qty_compare_1 is closed. The SPAN route is not: a cross-block
carrier's copy insn is still a real insn inside block 7 at local_alloc time, and one placed
between dz's definition and dx's definition lengthens dz's live range to 7 while leaving dx's
at 6, breaking the tie in dx's favour without touching either reference count. That is the
first frontier item, and it must be verified in `.lreg` (copy present, correctly placed, dz
span 7 vs dx span 6) before it is scored.

### Incidental trap for future generators

Splicing a statement between two declarations violates C89 declaration order and silently
produces a wrong-sized body rather than a hard error at the sandbox level (batch S variant s2 =
182 insns, batch R variant r7 = 184 insns). Every generated variant for this function must keep
all declarations at the head of their block.

### Artifacts

    tmp/grind/func_8002D780/s8/sched_fn.txt              block-7 RTL slice of the 2-floor body
    tmp/grind/func_8002D780/s8/copyscan.py               pass-by-pass reg-reg-copy survival scan
    tmp/grind/func_8002D780/s8/t1_rtl_slice.txt          t1 .rtl slice  (copies 197/200 present)
    tmp/grind/func_8002D780/s8/t1_jump_slice.txt         t1 .jump slice (copies still present)
    tmp/grind/func_8002D780/s8/t1_cse_slice.txt          t1 .cse slice  (copies gone)
    tmp/grind/func_8002D780/s8/t1_lreg_slice.txt         t1 .lreg slice (only the `m` copies left)
    tmp/grind/func_8002D780/s8/gen_r.py + variantsR/     batch R (8 variants)
    tmp/grind/func_8002D780/s8/gen_s.py + variantsS/     batch S (10 variants)
    tmp/grind/func_8002D780/s8/gen_t.py + variantsT/     batch T (9 variants)

- [s8] Chassis check: the s5 candidate body spliced into src/code6cac_b.c scores 2/202 on HEAD a91a7f44 (build_insns == target_insns == 202); the ledger floor of 2 is intact and every s8 number is relative to it.

- [s8] Target block 7 operand map, recovered from the four subus at asm/funcs/func_8002D780.s L103-L123: $t1 = x0, $a3 = z0, $t2 = cx, $a2 = cz, $t3 = px, $t4 = pz, $t5 = x2, $t0 = z2; dz lands in $a0 and dx in $v1, and the two differing instructions are the target's L107 (subu $v1,$t5,$t1 = dx) and L108 (subu $v0,$a2,$a3 = az), which our body emits in the opposite order.

- [s8] Our block-7 RTL (tmp/grind/func_8002D780/s8/sched_fn.txt): pseudos 129=ax, 130=az, 131=bx, 132=bz, 133=dz, 134=dx; scheduled 179,191,197,182,194,199,185,204,188,206,201,208,210,212; every one of the six difference subus carries `(nil)` LOG_LINKS, which is the datum behind the uniform INSN_PRIORITY of 1.

- [s8] GCC 2.7.2's sched.c list scheduler is BACKWARD, so the insn rank_for_schedule prefers is emitted LATER -- this inverts the direction of the s6 frontier's proposed priority lever, which named dx when the insn that would have to gain priority is az.

- [s8] jump_optimize is passed noop_moves == 1 at exactly one call site, toplev.c:3142 (jump2, after reload); no-op moves are therefore NOT removed before local_alloc, which keeps the surviving-copy route mechanically viable and is confirmed by every batch R/S/T carrier variant staying at 202 insns.

- [s8] `x = x` on a plain register local produces no RTL insn whatsoever in GCC 2.7.2 -- copyscan.py finds zero `(set (reg N) (reg N))` patterns in the .rtl dump of a body that contains one, so a self-assign cannot be a reference-count lever regardless of where it is placed.

- [s8] cse deletes a reg-reg copy whose definitions all sit in the same basic block (t1's insns 197 and 200 are present in .rtl and .jump, gone from .cse onward) while leaving a copy whose second definition is in a conditional arm (this body's own `m` copies 264/274 survive into .lreg in that same dump) -- the s5 `m` mechanism depends on the cross-block join, not merely on the definition count.

- [s8] The two requirements for a local-alloc-time artifact that lends dx a reference are mutually exclusive: surviving cse needs a definition in another basic block, and being eligible for combine_regs needs the pseudo to be referenced in only one block. The reference-count route into qty_compare_1 is therefore closed; the live-span route (an extra insn between dz's and dx's definitions, lengthening dz's range to 7 while dx's stays 6) is not.

- [s8] Splicing a statement between two declarations is a C89 declaration-order violation that silently yields a wrong-sized body rather than a hard failure at the sandbox level (batch S s2 = 182 insns, batch R r7 = 184 insns); every generated variant for this function must keep all declarations at the head of their block.

- [s8] 27 variants measured this session across three batches; not one moved the floor, and the best form remains the unchanged s5 candidate at 2/202.

## s9 (2026-09-08, forensics) - chassis: HEAD main @ c7aa37e7 (-mel -msoft-float), s5 candidate applied

Control re-measured first: the s5 candidate.c body scores **2/202** on this chassis
(build_insns == target_insns == 202). Floor unchanged. Mandated kill re-audit run:
`tools/fake_ablate.py --func func_8002D780 --file code6cac_b --candidate
memory/grind/func_8002D780/candidate.c` gives keep-all = 2/202, drop-1 = **6/202**, so the
single FAKE construct (the same-value re-store of the local `m`) is worth 4 insns on this
chassis (it was 5 when s5 measured it) and residual A's 2 insns are present WITH and
WITHOUT it -- residual A is not masked by the FAKE carrier, and every s6/s7/s8 kill taken
"with one FAKE construct present" is unaffected by that carrier.

### The whole of local-alloc's block-7 decision is now READ, not inferred
This session used the instrumented cc1 (tools/gcc-2.7.2/cc1, NOT build/cc1) with
BB2_SUGG_DEBUG=1 BB2_QTY_DEBUG=1, which prints local-alloc's complete per-quantity input
table (SUGGDBG-QTY, local-alloc.c:1447) and every find_free_reg answer in allocation order
(QTYDBG, local-alloc.c:1585). Driver: tmp/grind/func_8002D780/s9/dumpvar.py; captures in
tmp/grind/func_8002D780/s9/{cand,h1,u1_fresh_kc_kp_locals,u3_probe_early_extra_qty}/qty.txt.

Block 7 (test 3) is 14 RTL insns in the h1 = TARGET emission order (sliced from
s9/h1/code6cac_b.lreg):

     1 (179) reg129 ax = cx - x0        2 (182) reg130 dz = z2 - z0
     3 (197) reg135 = dz * ax           4 (185) reg131 dx = x2 - x0
     5 (188) reg132 az = cz - z0        6 (199) reg136 = dx * az
     7 (191) reg133 bx = px - x0        8 (204) reg137 = dz * bx
     9 (194) reg134 bz = pz - z0       10 (206) reg138 = dx * bz
    11 (201) reg117 kc = 135 - 136     12 (208) reg120 kp = 137 - 138
    13 (210) reg139 = kc ^ kp          14 (212) branch

Only SEVEN of those pseudos are local-alloc quantities: ax, dz, dx, az, bx, bz and the xor
result. reg117/reg120 are the OUTER `kc`/`kp` variables, hence multi-block. reg135-138 (the
four multiply results) are excluded by **local-alloc.c:472**: `dump_flow_info` prints them
as `pref LO_REG, else GR_REGS`, and the -2 (allocatable-locally) marking requires
`reg_alternate_class (i) == NO_REGS || ! CLASS_LIKELY_SPILLED_P (reg_preferred_class (i))`
-- a multiply result fails both halves, so no multiply result in this compiler is ever a
block_alloc quantity.

The two orders, from the dumps:

  * CANDIDATE order (ax,az,bx,bz,dz,dx): dz = qty1 birth 4 death 16 refs 3 (pri 2500),
    dx = qty5 birth 10 death 20 refs 3 (pri 3000). dx outranks dz, is allocated at ord=5 and
    takes $v1; dz takes $a0 at ord=6. **Target seats, wrong subu order.**
  * H1 = TARGET order (ax,dz,dx,az,bx,bz): dz = qty1 birth 4 death 16, dx = qty4 birth 8
    death 20 -- both refs 3, both span 12, both pri 2500. qty_sugg_compare_1's last line
    (`return *q1 - *q2`, local-alloc.c:1757) therefore decides, dz has the lower quantity
    number because it is born first, dz is allocated at ord=5 and takes $v1, dx takes $a0.
    **Target subu order, inverted seats -- score 9.**
  * In BOTH orders every 2-ref quantity (az, bx, bz, xor) has pri 10000 and is allocated
    first onto $v0, ax has pri 5000 and also lands on $v0, and the tied pair is decided last.

### The s7 `[extra_qty]` vector is REAL, and the dump shows exactly why it cannot be paid for
Probe u3 (tmp/grind/func_8002D780/s9/variantsU/u3_probe_early_extra_qty.c -- a deliberately
semantics-altered MECHANISM probe, never a candidate) adds one short-lived value `e` born
immediately after dz and consumed by the first multiply. Its dump
(s9/u3_probe_early_extra_qty/qty.txt) shows the predicted chain end to end:

    qty2 (e) birth 6 death 8, refs 2, pri 10000 -> allocated FIRST, takes $v0
    -> ax (birth 2 death 8) now overlaps e, cannot have $v0, takes $v1
    -> dz (birth 4 death 18) overlaps ax's $v1, takes $a0            <- target seat
    -> dx (birth 10 death 22) finds $v1 free after ax dies, takes $v1 <- target seat

So an extra block-7 quantity born between dz's and dx's births DOES break the tie and DOES
produce the target's dz/dx seats on the target's own emission order. The price is that the
new quantity outranks ax (pri 10000 vs 5000) and steals ax's $v0: the target has ax, az, bx
and bz ALL on $v0 (asm/funcs/func_8002D780.s L110/L124/L130/L140). u3 measures 13/203.

The admissible window is now closed arithmetically. A quantity B that fixes the seats without
disturbing ax must simultaneously (i) rank below ax (pri < 5000, or == 5000 with a higher
quantity number) so ax keeps $v0, (ii) rank above dz (pri > 2500) so it is seated first,
(iii) conflict with dz (overlap [4,16]) so dz is pushed off $v1, and (iv) not conflict with
dx (death <= 8) so dx can still take $v1. With qty priority = floor_log2(refs)*refs*size/
(death-birth) * 10000 and births at 2*insn-index, the only (refs, birth, death) triple that
satisfies all four is refs 2 / birth 4 / death 8 -- i.e. a pseudo born by block-7 insn 2,
which is the insn that defines dz itself. One insn sets one pseudo, so on this 14-insn block
geometry the slot is occupied; anything born at insn 1 outranks ax (u3's outcome) and
anything born at insn 3 or later conflicts with dx.

### Byte-neutral degrees of freedom found (both at the h1 baseline of 9, 202 insns)
  * u1: giving test 3 its OWN cross-product locals (`s32 c3, p3;` instead of reusing the
    outer `kc`/`kp`) makes them block-7 quantities (qty14 reg141 birth 22 death 28 refs 4,
    qty15 reg136 birth 24 death 26 refs 2) -- but both are born after dx's death window, dz
    and dx stay tied at 3 refs / span 12, and the score is unchanged at 9.
  * u2: naming the first product (`s32 e = dz * ax; kc = e - dx * az;`) is byte-neutral at 9
    and, as local-alloc.c:472 predicts, adds no quantity at all.

- [s9] Control + kill re-audit: the s5 candidate body still measures 2/202 on HEAD c7aa37e7; fake_ablate says the one FAKE construct is worth 4 insns (2 -> 6) and residual A's 2 insns are present in both, so no s6-s8 kill was measured behind a FAKE carrier occupying the contested pseudo.
- [s9] Block 7 has exactly seven local-alloc quantities. The four multiply results are excluded by local-alloc.c:472 (pref LO_REG = CLASS_LIKELY_SPILLED_P with alternate GR_REGS), and the outer kc/kp are excluded as multi-block, so no existing insn in block 7 can be turned into the extra quantity the seat flip needs.
- [s9] Read from the instrumented dump: on the h1 = target emission order dz and dx are qty1 (birth 4 death 16 refs 3) and qty4 (birth 8 death 20 refs 3) -- identical priority 2500 -- and local-alloc.c:1757's `*q1 - *q2` quantity-number fallback seats dz on $v1 first. On the candidate order dx is birth 10 death 20 (pri 3000) and wins outright, which is why the candidate holds the target's seats.
- [s9] MECHANISM CONFIRMED: an extra short-lived block-7 quantity born between dz's and dx's births flips dz to $a0 and dx to $v1 on the target's emission order (probe u3's QTYDBG trace), so the s7 inverse-solver [extra_qty] vector is a genuine lever and not a modelling artifact.
- [s9] MECHANISM PRICED: that same quantity has priority 10000 and is therefore allocated before ax (5000), taking $v0 and pushing ax onto $v1, which the target does not do (ax/az/bx/bz are all $v0). u3 scores 13 at 203 insns.
- [s9] The admissible (refs, birth, death) triple for a quantity that outranks dz, ranks below ax, conflicts with dz and not with dx is uniquely refs 2 / birth 4 / death 8 -- the birth slot of dz's own defining insn on the 14-insn block-7 geometry.
- [s9] Two byte-neutral degrees of freedom at the h1 baseline: fresh test-3 cross-product locals (u1) and a named first product (u2), both 9/202, both available free to future probes.

- [s9] Control + kill re-audit: the s5 candidate body measures 2/202 on HEAD c7aa37e7; tools/fake_ablate.py gives keep-all 2/202 and drop-1 6/202, and residual A's 2 insns are present in both, so no s6-s8 kill was taken behind a FAKE carrier occupying the contested pseudo.

- [s9] Block 7 (test 3) is 14 RTL insns in the h1 = TARGET emission order: ax, dz, mult(dz*ax), dx, az, mult(dx*az), bx, mult(dz*bx), bz, mult(dx*bz), kc, kp, xor, branch (sliced from tmp/grind/func_8002D780/s9/h1/code6cac_b.lreg).

- [s9] Only seven of block 7's pseudos are local-alloc quantities: ax, dz, dx, az, bx, bz and the xor result. The outer kc/kp are multi-block; the four multiply results are excluded by local-alloc.c:472 because dump_flow_info gives them `pref LO_REG, else GR_REGS` and LO_REG is CLASS_LIKELY_SPILLED_P.

- [s9] Candidate order (ax,az,bx,bz,dz,dx): dz = qty1 birth 4 death 16 refs 3 (pri 2500), dx = qty5 birth 10 death 20 refs 3 (pri 3000); QTYDBG shows dx allocated at ord=5 with got=3 ($v1) and dz at ord=6 with got=4 ($a0) -- the target's seats.

- [s9] h1 = target order (ax,dz,dx,az,bx,bz): dz = qty1 (4,16) and dx = qty4 (8,20), identical refs 3 and identical span 12, so qty_sugg_compare_1's `return *q1 - *q2` line (local-alloc.c:1757) seats dz first on $v1 and dx on $a0; score 9/202.

- [s9] In both orders every 2-ref block-7 quantity (az, bx, bz, the xor result) has pri 10000 and is seated on $v0 first, ax has pri 5000 and also lands on $v0, and the tied dz/dx pair is decided last.

- [s9] MECHANISM CONFIRMED: probe u3 adds one short-lived value born between dz and dx; its QTYDBG trace shows the new quantity taking $v0, ax displaced to $v1, dz to $a0 and dx to $v1 -- the target's dz/dx seats on the target's emission order. The s7 inverse-solver [extra_qty] vector is real.

- [s9] MECHANISM PRICED: that quantity's pri is 10000 against ax's 5000, so it outranks ax and steals ax's $v0, which the target does not do (ax/az/bx/bz are all $v0 at asm/funcs/func_8002D780.s L110/L124/L130/L140). u3 scores 13 at 203 insns.

- [s9] The admissible (refs, birth, death) triple for a quantity that outranks dz, ranks below ax, conflicts with dz and not with dx is uniquely refs 2 / birth 4 / death 8 -- the birth slot of dz's own defining insn on this 14-insn block geometry.

- [s9] Two byte-neutral degrees of freedom at the h1 baseline of 9/202: fresh test-3 cross-product locals (u1) and a named first product (u2); both are available free to future probes.

- [s9] Tooling note for the next session: the BB2_SUGG_DEBUG / BB2_QTY_DEBUG hooks only exist in tools/gcc-2.7.2/cc1, not in engine/buildconfig.py's CC1 (tools/gcc-2.7.2/build/cc1); tmp/grind/func_8002D780/s9/dumpvar.py takes the binary from $BB2_CC1 and writes both the qty trace and the full -da dump set per variant.
