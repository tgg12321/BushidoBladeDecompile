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

## [s10] (2026-09-08, rederive) — floor 2/202, and the residual MOVED

Chassis re-measured at dispatch: HEAD ce1ed95f (-mel -msoft-float), the s5 candidate body
spliced into src/code6cac_b.c, `sandbox func_8002D780 --disable all` = **2/202**,
build_insns == target_insns == 202. The ledger's floor 2 stands.

### 1. Test 3 does not need ANY named locals — and without them the residual changes

The mandated rederive produced a body whose third triangle test is two plain inline
expressions:

    kc = (z2 - z0) * (cx - x0) - (x2 - x0) * (cz - z0);
    kp = (z2 - z0) * (px - x0) - (x2 - x0) * (pz - z0);

It scores **2/202** — identical to the s5 body with six named difference locals — but
`tools/pairdiff.py code6cac_b func_8002D780` shows a DIFFERENT residual pair:

  * s5 body (named locals): ours emits `subu $v0,$a2,$a3` (az) then `subu $v1,$t5,$t1`
    (dx); the target emits dx then az. Residual = the az/dx order.
  * s10 body (inline): ours emits `subu $a0,$t0,$a3` (dz) then `subu $v0,$t2,$t1` (ax);
    the target emits ax (in the `bltz` delay slot at 8002D910) then dz. Residual = the
    ax/dz order. **The dx/az pair, all four mults and every register are already right.**

Both bodies have all six values in the target's registers; only one adjacent pair of
subus is transposed, and it is a different pair in each. Naming only dz/dx (`w3`), only
the four $v0-hosted offsets (`w4`), or only dx and az (`x4`) all reproduce the inline
body's 2/202 and its ax/dz residual. This closes the s3-s9 open question about whether
the six test-3 difference locals are ordinary C or a named-intermediate family claim:
they are not needed at all, and the committed candidate no longer has them.

### 2. The dz/dx seat is a LIVE-RANGE-LENGTH question, read out of the allocator

Instrumented cc1 (tools/gcc-2.7.2/cc1, BB2_SUGG_DEBUG=1 BB2_QTY_DEBUG=1 via
tmp/grind/func_8002D780/s10/dumpvar.py) on the new body (`dw2`) and on the one-token
variant that flips the first product's operands (`dy1` = `(cx - x0) * (z2 - z0)`, 9/202).
The blk=7 SUGGDBG-QTY tables are IDENTICAL except for the ax and dz rows:

    body            qty  reg  birth death refs  span  pri = floor_log2(r)*r*size/span*10000
    s10 (2/202)     0    129    2    16    3     14   2142   <- dz
                    1    130    4     6    2      2  10000   <- ax
                    4    132    8    20    3     12   2500   <- dx      -> dx seated first, $v1  (TARGET)
    y1  (9/202)     0    129    2     6    2      4   5000   <- ax
                    1    130    4    16    3     12   2500   <- dz
                    4    132    8    20    3     12   2500   <- dx      -> TIE, qty-number fallback seats dz, $v1  (WRONG)

So the entire seat question is: does dz's live range outlast dx's? Evaluating `(z2 - z0)`
before `(cx - x0)` puts dz's subu at block-7 index 0 instead of index 1, which lengthens
dz's range from 12 to 14 and drops its priority under dx's 2500 (local-alloc.c:1725-1758).
The price is that ax is then emitted second, which is the one insn pair we are short.
This is the same tie s7/s9 measured, now shown to be one token wide in the source.

### 3. Everything measured this session (all at 202 insns unless noted)

Batch W (tmp/grind/func_8002D780/s10/variantsW, gen_w.py) — structural rewrites:
  w2 fully inline = 2 · w3 edge named only = 2 · w4 four offsets named = 2 ·
  w0b h1 named order = 9 · w7 pairwise named = 9 · w6 two reused $v0 temps = 26 ·
  w5/w5b one reused $v0 temp = 41/41 · w1/w1b in-place translation of cx/cz/px/pz = 44/44.
Batch X (variantsX, gen_x.py) — operand/eval-order permutations:
  x4 dx+az named = 2 · x3 edge+ax named = 9 · x5 fresh c3/p3 = 9 · x2 = 17 ·
  x1 = 18 · x7 named products = 18 · x9 = 19 · x6 kp first = 38 · x8 edge in-place = 40.
Batch Y (variantsY, gen_y.py) — single-product operand flips off the 2-floor body:
  y9 control = 2 · y6 = 3 · y7 = 3 · y3 both kc products flipped = 4 ·
  y2 kc second flipped = 5 · y1 kc first flipped = 9 · y5 = 9 · y8 = 9 · y4 kp first = 16.
  y3's diff is instructive: ax/dz ORDER correct, all seats correct, and the 4 residual
  insns are the two mult operand orders plus the az/dx transposition.
Batch Z (variantsZ, gen_z.py) — eval order decoupled from mult operand order:
  z1/z3/z4/z5/z8/z9 = 9 · z6 = 18 · z2/z7 = 22. A single mult operand flip is
  byte-neutral in the ax-first regime; flipping two costs 9-13.
Batch B (variantsB, gen_b.py) — hoisting one difference into the test-2 arm:
  b5 az = 20 (203 insns) · b2 dz = 21 · b1 ax = 26 · b7 = 26 · b8 bx = 28 (201) ·
  b4 bz = 32 (201) · b3 dx = 38 (203) · b6 edge pair = 39 (203).

### 4. Where the last two insns now stand

The target's block 7 is 14 RTL insns: ax(i0) dz(i1) mult1(i2) dx(i3) az(i4) mult2(i5)
bx(i6) mult3(i7) bz(i8) mult4(i9) kc-sub kp-sub xor branch. On that geometry dz spans
i1->i7 and dx spans i3->i9 — six insn slots each, refs 3 each, hence the tie. Making dx
win needs one of exactly three things, and s8/s9/s10 have now priced them all:
  (a) dz's range longer — only reachable by moving dz's def to i0, which is this body,
      and which costs the ax/dz order (this session's residual);
  (b) dx's refs = 4 (pri 6666) — a self-assign emits no RTL (s8), an in-block copy is
      folded by cse.c:1032 (s8, class), a cross-block carrier cannot merge quantities
      with a block-local pseudo (s8);
  (c) one extra insn between dz's def and dx's def that is deleted after local_alloc —
      it would give dz span 14 against dx's 12 without touching either ref count. The
      only late deleter is jump2's delete_noop_moves (toplev.c:3142), i.e. a reg-reg
      copy, and s8's cse class kill says such a copy cannot survive if both of its
      definitions sit inside block 7.

### 5. Block structure in the test region is free (batch C)

Five control-flow spellings of the three triangle tests — inverted `goto`-based early
exits, test 3 folded entirely into its own `if` condition, tests 2+3 chained with `&&`
and embedded assignments, an inverted innermost `if`, and fresh `c3`/`p3` locals — all
score 2/202 at 202 insns, byte-identical to the candidate (tmp/grind/func_8002D780/s10/
gen_c.py -> s10/variantsC). jump1 normalises them onto one CFG. Future probes can pick
whichever shape suits them; block structure is not the missing input to the dz/dx tie.

- [s10] Chassis re-measured at dispatch: HEAD ce1ed95f (-mel -msoft-float), s5 candidate body in src/code6cac_b.c, `sandbox func_8002D780 --disable all` = 2/202 with build_insns == target_insns == 202. The ledger's floor 2 stands; the brief's 'measurement unavailable' is resolved.

- [s10] 39 distinct bodies measured this session across five batches (W, X, Y, Z, B, C), every one recorded in hypotheses.md with its score and insn count.

- [s10] The new candidate (memory/grind/func_8002D780/candidate.c) writes test 3 as two plain inline expressions with NO locals: kc = (z2 - z0) * (cx - x0) - (x2 - x0) * (cz - z0); kp = (z2 - z0) * (px - x0) - (x2 - x0) * (pz - z0). Score 2/202. The s5 six-named-locals body is preserved verbatim as memory/grind/func_8002D780/candidate_alt_s5_named_locals.c and also still scores 2/202.

- [s10] The two bodies have DIFFERENT residual pairs: s5 body = the az/dx subu transposition; s10 body = the ax/dz subu transposition. In both, all six values already sit in the target's registers and all four multiplies match, so the residual is one adjacent-pair emission swap in each.

- [s10] Allocator trace (tmp/grind/func_8002D780/s10/dw2/qty.txt vs s10/dy1/qty.txt): the s10 body gives dz span 14 / pri 2142 against dx span 12 / pri 2500 so dx is seated first onto $v1 (target seats); the one-token ax-first flip gives dz and dx both span 12 / pri 2500, an exact tie that local-alloc.c:1757's quantity-number fallback resolves in dz's favour, inverting the seats.

- [s10] Family footprint of the candidate is now ONE annotated construct: the same-value re-store of the local `m` (dead-store family, .claude/rules/dead-store-fake-exception.md, in-TU byte-matched precedent at src/code6cac_b.c:1244-1265). The six test-3 difference locals that s3-s9 listed as an unresolved named-intermediate question are gone from the body.

- [s10] Block structure in the test region is a free degree of freedom: five distinct control-flow spellings (goto early exits, &&-chained tests with embedded assignments, an inverted inner if, fresh cross-product locals, test 3 folded into its own if condition) all produce byte-identical output at 2/202.

- [s10] Arithmetic of the remaining 2 insns, pinned: the target's block 7 is 14 RTL insns -- ax(i0) dz(i1) mult1(i2) dx(i3) az(i4) mult2(i5) bx(i6) mult3(i7) bz(i8) mult4(i9) kc-sub kp-sub xor branch -- on which dz spans i1->i7 and dx spans i3->i9, six slots and 3 refs each, hence the tie. Making dx win needs dz's range longer (only reachable by moving dz's def to i0, which costs the ax/dz order), or dx's refs = 4 (all three spellings class-killed in s8), or one extra insn between dz's def and dx's def that is deleted after local_alloc.

## [s11] (2026-09-08, rederive) — floor 2/202, and the residual is RE-ATTRIBUTED to reorg.c

Chassis re-measured at dispatch: HEAD 4e56c8b9 (-mel -msoft-float), the s10 candidate body
spliced into src/code6cac_b.c, `sandbox func_8002D780 --disable all` = **2/202**,
build_insns == target_insns == 202. The brief's "measurement unavailable" is resolved and
the ledger's floor 2 stands.

### 0. Kill re-audit (mandated) — the FAKE was never annotated, and it is still load-bearing

`tools/fake_ablate.py --func func_8002D780 --file code6cac_b --candidate
memory/grind/func_8002D780/candidate.c` returned **"no FAKE-annotated constructs found"**:
the s10 candidate's `m = dist;` same-value re-store shipped WITHOUT its `/* FAKE: ... */`
annotation even though the s10 header claimed it carried one. An un-annotated FAKE is an
automatic Judge FAIL, so this was a latent blocker on any future candidate-ready. Fixed in
this session's candidate.c. Hand ablation on the current chassis
(tmp/grind/func_8002D780/s11/variants/):
  * `a1_no_m_restore.c` (re-store deleted)     = **6/202** -> banked as
    memory/grind/func_8002D780/rejected/m-restore-ablated-still-load-bearing-6.c
  * `a2_annotated_m.c` (re-store + annotation) = **2/202** — the annotation is codegen-neutral.
So the s6-s10 kills measured "with the `m` carrier present" were measured against a construct
that is still worth 4 insns on this chassis; the carrier is not a stale artefact.

### 1. The two residual instructions are a REORG DELAY-SLOT FILL, not a local-alloc seat

`tools/pairdiff.py code6cac_b func_8002D780` on the current candidate:

    ours[91]  bltz v0,@          target[91] bltz v0,@
    ours[92]  subu a0,t0,a3 (dz) target[92] subu v0,t2,t1 (ax)   <- the delay slot
    ours[93]  subu v0,t2,t1 (ax) target[93] subu a0,t0,a3 (dz)
    ours[94]  mult a0,v0         target[94] mult a0,v0
    === 2 differing instructions ===

Every register, every multiply and every other insn in the function is already correct. The
transposed pair straddles a branch delay slot (target address 8002D90C `bltz $v0,.L8002D964`
with slot 8002D910), so the decision that produces it belongs to **reorg.c**, read directly
out of the instrumented cc1 (`BB2_DBR_DEBUG=1`, tmp/grind/func_8002D780/s11/dbr1/stderr_full.txt):

    DBRDBG thr insn=175 thread=179 opp=227 own=1 likely=0 tif=0 oppregs=20010020_00000000 oppmem=1
    DBRDBG thr insn=175 trial=179 refset=0 setset=0 setneed=0 setsopp=0 trap=0
    DBRDBG thr WINNER insn=175 trial=179 annul=0

`fill_slots_from_thread` (tools/gcc-2.7.2/reorg.c:3399-3480) walks the fall-through thread and
takes the FIRST insn that clears all five gates — references-set, sets-set, sets-needed,
sets-a-resource-needed-at-the-opposite-thread, and may_trap_p. Our block-7 head insn (uid 179 =
dz's subu) clears all five (`oppregs=20010020` decodes to $a1/$s0/$s1/$sp live at the branch
target — $a0 is dead there, so `setsopp=0`), so it wins the slot immediately. The target's slot
holds ax, therefore **the target's pre-reorg block 7 began with ax, not dz**.

### 2. The emission order and the seat are the SAME variable (INSN_LUID), and they disagree

Both scheduler passes decide the ax/dz order by LUID, i.e. by source order:

  * sched.c `rank_for_schedule` returns on priority first, then on dependence class, then
    `return INSN_LUID (tmp) - INSN_LUID (tmp2);` (tools/gcc-2.7.2/sched.c:2464).
    Instrumented run (`BB2_RANK_DEBUG=1`, tmp/grind/func_8002D780/s11/rank2/stderr_full.txt)
    prints `RANKDBG last=183 y=181 cls=3 x=179 cls2=3 val=0` — reaching the class print at all
    proves the priority comparison did NOT return, so uids 179 and 181 are priority-TIED, and
    both are class 3, so the LUID line decides. Both are inputs of the same mult (uid 183), so
    their longest-path priorities are structurally equal and no source spelling separates them.
  * The relative order is identical in .lreg (post-sched1), .greg, .jump2 and .sched2 for the
    candidate (uid windows measured: `[175, 179, 181, ...]` in all four), so sched2 does not
    decouple the post-reload emission order from the order local-alloc saw.

And the seat needs the opposite LUID order. Measured this session on the ax-first body
(tmp/grind/func_8002D780/s10/variantsY/y1_w2_kc_first_flip.c, cc1 asm in s11/y1/code6cac_b.s):

    cand (dz first) : bltz $2,.L296 / subu $4,$8,$7 (dz in $a0, IN THE SLOT) / subu $2,$10,$9 (ax)
    y1   (ax first) : bltz $2,.L296 / subu $2,$10,$9 (ax, IN THE SLOT — TARGET) / subu $3,$8,$7
                      (dz in $v1 — WRONG seat) / ... subu $4,$13,$9 (dx in $a0 — WRONG seat)

So: dz-first source order buys the target's SEATS and loses the SLOT; ax-first buys the SLOT
and loses the seats (dz and dx tie at refs 3 / span 12 and local-alloc.c:1684's quantity-number
fallback seats the lower-numbered dz first). One variable, two contradictory requirements.

### 3. The target's own block 7 forces the tie, so the fix must ADD an insn — and there is no free slot

Target block 7 (8002D910-8002D95C) is 20 machine instructions of which 2 are ASPSX hazard nops,
i.e. 18 RTL insns:

    i0 ax  i1 dz  i2 mult(dz,ax)  i3 mflo t0  i4 dx  i5 az  i6 mult(dx,az)  i7 mflo a2
    i8 bx  [nop]  i9 mult(dz,bx)  i10 mflo a0 i11 bz [nop] i12 mult(dx,bz)
    i13 kc-sub  i14 mflo v1  i15 kp-sub  i16 xor  i17 branch

dz spans i1->i9 and dx spans i4->i12: eight insn slots each, 3 refs each — the tie is a property
of the TARGET's own geometry, not of our spelling. Breaking it in dx's favour needs one extra
RTL insn strictly between i1 and i4 that costs zero machine instructions. The zero-cost sites in
this block are exactly the two ASPSX pads, and ASPSX only pads when fewer than two insns
separate an `mflo` from the next `mult` (target: mflo i3 -> mult i6 has two insns and no pad;
mflo i7 -> mult i9 has one and gets the nop at 8002D934; mflo i10 -> mult i12 has one and gets
the nop at 8002D944). Both pads sit AFTER dx's birth, and the arithmetic of spending them runs
the wrong way:
    free insn before i9  : dz span 9, dx span 9  -> tie, dz still seated first
    free insn before i12 : dz span 8, dx span 9  -> dz priority HIGHER, dz seated first
    both                 : dz span 9, dx span 10 -> dz priority HIGHER, dz seated first
There is no pad between i1 and i4, so any insn placed there costs +1 and leaves 203.

### 4. What this rules out and what it leaves

Ruled out this session (kill records in hypotheses.md [s11]): re-ordering the six test-3
difference expressions in any way (the ax/dz slot fill and the dz/dx seat are the same LUID and
pull opposite ways); making sched2 emit an order different from the one local-alloc saw
(priority- and class-tied, LUID-stable); making reorg pass over dz's subu (all five gates clear,
and `setsopp` would need $a0 live at .L8002D964, which the target's own sqrt block contradicts —
it writes $a0 at 8002D980 before any read); and paying for the missing insn out of ASPSX pad
slack (both pads are on the wrong side of dx's birth).

What is left is narrower than the s10 frontier stated: not "block 7 contained something else" in
general, but specifically **one RTL insn born strictly between dz's def and dx's def that either
(a) disappears between local_alloc and final, or (b) makes one of dz/dx a GLOBAL (cross-block)
quantity so that global.c, not local-alloc, seats it.** Route (b) is completely untried and is
the only door this session did not close: local-alloc only allocates quantities whose whole live
range sits inside one basic block, so a test-3 difference that is still live at the join would be
handed to global_alloc with an entirely different priority model, and global.c can hand out $a0
without ever consulting qty_compare_1.

### 5. Artifacts

  tmp/grind/func_8002D780/s11/cand/     — full -da dump set + qty tables for the candidate
  tmp/grind/func_8002D780/s11/y1/       — same for the ax-first body (cc1 .s shows the seats)
  tmp/grind/func_8002D780/s11/dbr1/stderr_full.txt  — BB2_DBR_DEBUG reorg trace
  tmp/grind/func_8002D780/s11/rank2/stderr_full.txt — BB2_RANK_DEBUG rank_for_schedule trace
  tmp/grind/func_8002D780/s11/sched2_fn_compact.txt — one-line-per-insn sched2 stream
  tmp/grind/func_8002D780/s11/dbg.py    — s11 dump driver (all four BB2_* hooks at once)
  tmp/grind/func_8002D780/s11/score.ps1 — variant scorer (apply.py via Windows python + wteng)

- [s11] Chassis re-measured at dispatch: HEAD 4e56c8b9 (-mel -msoft-float), s10 candidate body spliced into src/code6cac_b.c, `sandbox func_8002D780 --disable all` = 2/202 with build_insns == target_insns == 202. The brief's 'measurement unavailable' is resolved; the ledger floor 2 stands.

- [s11] The 2 residual instructions are ours[92] `subu a0,t0,a3` (dz) versus target[92] `subu v0,t2,t1` (ax), transposed with ours[93]/target[93]; every register, every multiply and every other instruction in the function already matches. The pair straddles the delay slot of `bltz $v0,.L8002D964` at 8002D90C.

- [s11] reorg.c owns the residual: BB2_DBR_DEBUG shows `DBRDBG thr insn=175 trial=179 refset=0 setset=0 setneed=0 setsopp=0 trap=0` then `WINNER insn=175 trial=179`. All five fill_slots_from_thread gates are clear for a register-subtract at the head of the thread, so reorg always takes the first insn of block 7 and the target's slot content is decided upstream, by which subu is emitted first.

- [s11] sched1 and sched2 both decide the ax/dz order by INSN_LUID (sched.c:2464): BB2_RANK_DEBUG prints `RANKDBG last=183 y=181 cls=3 x=179 cls2=3 val=0`, and reaching the class print proves the priority comparison did not return. The two subus feed the same mult, so their longest-path priorities are structurally equal and no source spelling separates them.

- [s11] The relative order of uids 179 and 181 is identical in the .lreg, .greg, .jump2 and .sched2 dumps of the candidate, so sched2 does not decouple the final emission order from the order local-alloc saw. The emission order and the seat are therefore the same variable.

- [s11] On the target's own block-7 geometry (18 RTL insns plus 2 ASPSX pad nops), dz spans i1->i9 and dx spans i4->i12 - eight slots and 3 refs each - so qty_compare_1 ties them regardless of spelling and local-alloc.c:1684's quantity-number fallback seats whichever was born first. The tie is a property of the target, not of our C.

- [s11] ASPSX pad accounting (new): the only zero-machine-cost insertion sites inside block 7 are the two mflo->mult hazard pads at 8002D934 and 8002D944, both after dx's birth. Inserting there gives dz/dx spans of 9/9, 8/9 or 9/10 - a tie or dz-favouring in every case. The interval that would work, strictly between dz's def and dx's def, carries no pad, so an insn there costs +1 and yields 203.

- [s11] The split-block route that would hand dz/dx to global_alloc is live but expensive as spelled: branching on `kc < 0` between the two cross products moves the seats (reg129 $a0->$v1, reg130 $v0->$a0, reg132 $v1->$t1) but jump2 does not tail-merge the duplicated kp arithmetic, costing 10 machine instructions (36/212 and 37/212).

- [s11] The s10 candidate shipped its `m = dist;` FAKE construct UN-ANNOTATED (fake_ablate.py: 'no FAKE-annotated constructs found'), which would have been an automatic Judge FAIL on any candidate-ready. Fixed this session; the annotation is codegen-neutral (2/202 with and without the comment) and the construct is still worth 4 insns (6/202 when deleted).

## [s12] (2026-09-08, structural) — floor still 2/202, but the RESIDUAL MOVED OFF THE DELAY SLOT

Chassis re-measured at dispatch: HEAD d9de5549 (-mel -msoft-float), the s11 candidate body
spliced into src/code6cac_b.c, `sandbox func_8002D780 --disable all` = **2/202**,
build_insns == target_insns == 202. The brief's "measurement unavailable" is resolved and
the ledger floor 2 stands. (The s11 kill re-audit of the `m` re-store is not repeated: s11
already ablated it on this same chassis lineage and the annotation is in candidate.c.)

### 0. The s11 contradiction is BROKEN, and the new candidate is the proof

s11 left the function on a two-horned fork: dz-first source order buys the target's register
SEATS and loses the delay SLOT; ax-first buys the slot and loses the seats. This session
found a body that takes BOTH horns at once. The new `memory/grind/func_8002D780/candidate.c`
(variant `d1_az_hoisted`) scores the same 2/202, but:

    ours[92] subu v0,t2,t1 (ax, IN THE SLOT)  target[92] subu v0,t2,t1   <- now MATCHES
    ours[93] subu a0,t0,a3 (dz, in $a0)       target[93] subu a0,t0,a3   <- now MATCHES
    ours[96] subu v0,a2,a3 (az)               target[96] subu v1,t5,t1 (dx)
    ours[97] subu v1,t5,t1 (dx)               target[97] subu v0,a2,a3 (az)
    === 2 differing instructions ===          (tmp/grind/func_8002D780/s12/pairdiff_d1.txt)

Every register in the whole function is correct, the delay-slot fill is correct, and the two
remaining instructions are an adjacent transposition of two independent subus that both feed
`mult $v1,$v0`. **No delay slot is involved any more**, so reorg.c is out of the picture and
the s11 "slot and seat are the same LUID" contradiction no longer binds. The old dz-first
body is preserved verbatim as `candidate_alt_s11_dzfirst.c`.

### 1. The three declarations, and exactly what each one buys

Test 3 is now spelled with three named difference locals:

    s32 ax = cx - x0;
    s32 dz = z2 - z0;
    s32 az = cz - z0;
    kc = dz * ax - (x2 - x0) * az;
    kp = dz * (px - x0) - (x2 - x0) * (pz - z0);

  * `ax` first makes ax's subu the head of block 7, so reorg's `fill_slots_from_thread`
    takes IT for the test-2 delay slot (the target's slot content) instead of dz's subu.
  * `dz` named keeps the first product's operand order, so mult1 stays `mult $a0,$v0`.
  * `az` is the tie-breaker. See section 2.

Measured on its own, `ax` alone (variant a1) = **9/202**, 202 insns
(rejected/axfirst-slot-right-seats-swapped-9.c): slot correct, dz/dx seats swapped. Adding
the `dz` name changes nothing (a4 = 9). Adding `az` is what closes the seats.

### 2. The quantity arithmetic, read out of the instrumented cc1 (not inferred)

local-alloc.c:1725-1758 ranks block-7 quantities by
`floor_log2(refs)*refs*size/(death-birth)*10000`, ties break on quantity number
(local-alloc.c:1684), and whichever of dz/dx is seated FIRST takes `$v1` while the other
takes `$a0`. The target needs dz=$a0 / dx=$v1, i.e. **dx must be seated first**.

BB2_QTY_DEBUG / BB2_SUGG_DEBUG tables (tmp/grind/func_8002D780/s12/<tag>/stderr_full.txt),
block 7, the two quantities that matter:

    variant a1 (ax first, az inline)   dz birth  4 death 16 refs 3 -> span 12, pri 2500
                                       dx birth  8 death 20 refs 3 -> span 12, pri 2500
                                       TIE -> qty number seats dz first -> dz $v1, dx $a0
                                       (allocation order printed: ord5 qty1 got=3,
                                        ord6 qty4 got=4)                       score 9
    variant c1 (kp's 2nd product first) dz birth 4 death 20 refs 3 -> span 16, pri 1875
                                       dx birth  8 death 16 refs 3 -> span  8, pri 3750
                                       dx seated first -> dx $v1 (got=3), dz $a0 (got=4)
                                       — the TARGET's seats, on an ax-first body    score 32
    candidate d1 (az hoisted)          dz span 14 / dx span 12 (dx 2500 > dz 2142)
                                       dx seated first -> the target's seats        score 2

c1 is the load-bearing measurement: it is the first body in twelve sessions to hold the
target's delay slot AND the target's dz/dx seats simultaneously, and it proves the span
model quantitatively. It scores 32 only because reordering kp's two products scrambles six
emitted instructions; d1 gets the same seats without touching the emission order.

The mechanism of `az` is purely positional: one extra RTL insn placed strictly between dz's
definition and dx's definition shifts dz's DEATH by +2 while shifting dx's BIRTH and DEATH
together (span unchanged), so dz's span grows to 14 and dx outranks it. `az` costs no
instruction because the subu it names is one the target emits anyway — it is simply
scheduled earlier by sched1 than it is emitted by sched2.

### 3. Hoisting the OTHER differences does not work, and why

  * `bz` hoisted (d2) = 9/202, `bx` hoisted (d3) = 9/202. sched1 sinks both back down to
    their consumers (mult4 / mult3), so no insn lands between dz and dx and the span tie is
    unchanged. Only `az` stays high, because its consumer mult2 is the earliest one after
    the first product.
  * `dx` named as the third declaration (d5) = 9/202: dx's birth moves EARLIER, which
    restores the tie from the other side.
  * `az` + `bz` together (d4) = 2/202 — same residual as d1; `bz` is inert, so the candidate
    keeps the smaller body.

### 4. The copy-carrier route for a free RTL insn is dead as spelled

s11's frontier 2 asked for "an RTL insn born between dz's def and dx's def that disappears
between local_alloc and final". Four multiply-defined copy carriers were built on the
ax-first chassis (`c = x2` / `c = z0` / `c = cz`, def-1 in the outer block or in the test-1
arm, def-2 at block-7 index 3, the block-7 uses rewritten to read the carrier). All four
scored 9/202 at 202 insns, and the BB2_QTY_DEBUG table for b1 is byte-identical to a1's:
the copy is gone before local-alloc. The `m = dist` precedent in the sqrt block does NOT
generalise — there the carrier's only use is an `__asm__` "r" operand, which cse will not
substitute through; a carrier whose uses are ordinary arithmetic is always foldable, so cse
propagates the source and DCE deletes the copy. This closes the copy spelling of that
frontier.

### 5. What is left, stated precisely for s13

One question, and it is now a pure scheduler question with no allocator or reorg coupling:
**make sched2 emit dx before az.** sched.c `rank_for_schedule` compares INSN_PRIORITY, then
dependence class, then `INSN_LUID (tmp) - INSN_LUID (tmp2)` (sched.c:2464). dx and az are
independent, both feed mult2, so their longest-path priorities are equal and both are in the
same class; az's LUID is lower because the declaration puts its subu earlier in sched1's
output, and that is exactly the property that buys the seats. Note the self-cancelling shape
of the naive fix: any source order that gives dx the lower LUID also gives dx the earlier
birth and restores the qty tie (measured, d5 = 9).

Two doors that are NOT self-cancelling:
  (a) an insn other than `az` that sched1 keeps between dz and dx but sched2 places AFTER dx
      — it needs either a lower INSN_PRIORITY than dx (a longer dependence chain to the
      block end) or a post-reload hard-register dependence that blocks it. bx and bz were
      measured and sink instead (section 3), so this needs the solver rather than guessing.
  (b) `tools/sched_solver` (order- and clock-exact for both passes) run on the d1 stream:
      ask directly which sched1 statement orders produce dz-span > dx-span AND a sched2
      output equal to the target's emission order. This is the instrument the residual has
      finally been reduced to a fit question for.

### 6. Artifacts

  tmp/grind/func_8002D780/s12/variants/   — 17 scored bodies (a*, b*, c*, d*)
  tmp/grind/func_8002D780/s12/pairdiff_d1.txt — the 2-instruction residual of the candidate
  tmp/grind/func_8002D780/s12/a1/         — full -da dump set + QTYDBG table, ax-first body
  tmp/grind/func_8002D780/s12/b1/         — same for the x2 copy carrier (identical table)
  tmp/grind/func_8002D780/s12/c1/         — same for the product-reordered body (target seats)
  tmp/grind/func_8002D780/s12/dbg.py      — dump driver (needs BB2_CC1=tools/gcc-2.7.2/cc1)
  tmp/grind/func_8002D780/s12/score.ps1   — variant scorer
  tmp/grind/func_8002D780/s12/gen_a.py, gen_b.py, gen_c.py, gen_d.py, bank.py

- [s12] Chassis re-measured at dispatch: HEAD d9de5549 (-mel -msoft-float), s11 candidate body in src/code6cac_b.c, sandbox --disable all = 2/202, build_insns == target_insns == 202.
- [s12] NEW CANDIDATE (d1_az_hoisted, 2/202): test 3 spelled with `s32 ax = cx - x0; s32 dz = z2 - z0; s32 az = cz - z0;` ahead of the two cross products. It holds the target's delay-slot fill AND every register in the function; the only 2 differing instructions are the adjacent az/dx transposition at indices 96/97. The old dz-first body is candidate_alt_s11_dzfirst.c.
- [s12] The s11 slot-vs-seat contradiction is broken. `ax` first gives reorg the target's slot insn; `az` hoisted puts one extra RTL insn between dz's def and dx's def, which lengthens dz's live range to 14 against dx's 12, so dx (pri 2500) outranks dz (pri 2142), is seated first and takes $v1, leaving dz $a0 — the target's seats.
- [s12] Quantitative proof of the span model, from BB2_QTY_DEBUG: a1 (ax first, az inline) has dz span 12 / dx span 12, ties, seats dz first, score 9; c1 (kp's second product evaluated first) has dz span 16 / dx span 8, seats dx first with got=3 for dx and got=4 for dz — the target's seats on an ax-first body — score 32 only because six emitted instructions are reordered.
- [s12] Hoisting bz or bx instead of az is inert (9/202 each): sched1 sinks them back to their later consumers so no insn lands between dz and dx. Naming dx as the third declaration (d5) is also 9/202 because dx's birth moves earlier and the tie returns.
- [s12] The multiply-defined COPY carrier route for a free RTL insn is closed: four spellings (c = x2 outer, c = x2 in the test-1 arm, c = z0, c = cz) all score 9/202 at 202 insns and b1's block-7 QTYDBG table is identical to a1's, i.e. the copy is deleted before local-alloc. The `m = dist` precedent does not generalise because m's only use is an __asm__ "r" operand, which cse will not substitute through.
- [s12] The residual is now a pure sched2 ready-list question: dx and az are independent, both feed mult2, priorities and class are equal, and sched.c:2464 falls to INSN_LUID, where az is lower precisely because that is what buys the seats. Any source order giving dx the lower LUID also gives dx the earlier birth and restores the qty tie (d5, measured).

- [s12] Chassis re-measured at dispatch: HEAD d9de5549 (-mel -msoft-float), the s11 candidate body spliced into src/code6cac_b.c, `sandbox func_8002D780 --disable all` = 2/202 with build_insns == target_insns == 202. The brief's 'measurement unavailable' is resolved and the ledger floor 2 stands.

- [s12] NEW CANDIDATE (variant d1_az_hoisted, 2/202, 202 insns, re-measured with it in place in src/): test 3 spelled as `s32 ax = cx - x0; s32 dz = z2 - z0; s32 az = cz - z0; kc = dz * ax - (x2 - x0) * az; kp = dz * (px - x0) - (x2 - x0) * (pz - z0);`. It holds the target's delay-slot fill and EVERY register in the function; the only two differing instructions are the adjacent az/dx transposition at indices 96/97.

- [s12] The s11 slot-versus-seat contradiction is broken. `ax` first gives reorg the target's slot insn; `az` hoisted puts one extra RTL insn between dz's def and dx's def, lengthening dz's live range to 14 against dx's 12, so dx (pri 2500) outranks dz (pri 2142), is seated first and takes $v1, leaving dz $a0 - the target's seats.

- [s12] Quantitative proof of the span model from BB2_QTY_DEBUG: a1 (ax first, az inline) has dz span 12 / dx span 12, ties, seats dz first, score 9; c1 (kp's second product evaluated first) has dz span 16 / dx span 8, seats dx first with got=3 for dx and got=4 for dz - the target's seats on an ax-first body - and scores 32 only because six emitted instructions are reordered.

- [s12] Hoisting bz or bx instead of az is inert (9/202 each): sched1 sinks them back to their later consumers so no insn lands between dz and dx. Naming dx as the third declaration is also 9/202 because dx's birth moves earlier and the tie returns.

- [s12] The multiply-defined COPY carrier route for a free RTL insn is closed: four spellings (c = x2 in the outer block, c = x2 in the test-1 arm, c = z0, c = cz) all score 9/202 at 202 insns and b1's block-7 QTYDBG table is identical to a1's, i.e. the copy is deleted before local-alloc. The `m = dist` precedent does not generalise because m's only use is an __asm__ "r" operand, which cse will not substitute through.

- [s12] The residual is now a pure sched2 ready-list question with no allocator or reorg coupling: dx and az are independent, both feed mult2, priorities and dependence class are equal, and sched.c:2464 falls to INSN_LUID where az is lower precisely because that is what buys the seats.

- [s12] Tooling note for future sessions: the instrumented cc1 with the BB2_* hooks is tools/gcc-2.7.2/cc1, but engine/buildconfig.py's CC1 points at tools/gcc-2.7.2/build/cc1, so the dump driver must be run with BB2_CC1=tools/gcc-2.7.2/cc1 or it silently produces dumps with no QTYDBG/DBRDBG lines.

- [s12] The new candidate reintroduces three named difference locals in test 3. `dz` is written once and read twice (an ordinary CSE variable); `ax` and `az` are written once and read once, which is the named-intermediate shape in .claude/rules/narrow-byte-args-packed-call.md plus the 2026-08-17 clarification in .claude/rules/no-new-park-categories.md. All three hold real values that appear in the target's bytes. A future candidate-ready session must settle whether the once-read pair needs the named-intermediate FAKE annotation or is ordinary C, and should emit a ruling-request rather than guess.

## s13 (2026-09-08, escalation) — chassis: HEAD 4a44c3ce (-mel -msoft-float), s12 candidate applied

Disposition session. Floor re-measured at dispatch and unchanged: `sandbox func_8002D780
--disable all` = **2/202**, build_insns == target_insns == 202, with
memory/grind/func_8002D780/candidate.c (d1_az_hoisted) spliced into src/code6cac_b.c.

### The un-tried lever this session spent: a permuter campaign on the 2-floor chassis
Frontier item 3 from s12. The s4 campaign ran on the 7-floor s3 chassis and was dominated
by residuals B and C; this is the first campaign on a chassis whose ONLY residual is the
adjacent az/dx transposition.

Workspace rebuild (tmp/grind/func_8002D780/s13/mkbase.py): the s4 workspace
(tmp/grind/func_8002D780/s4/nonmatchings/func_8002D780, four hurdles documented in [s4])
was re-based on the current candidate — comments stripped, the four `__asm__ volatile`
statements re-encoded into `#pragma _permuter b64literal`, spliced over the old body in
base.c. The s4 base.c is preserved at tmp/grind/func_8002D780/s13/base_s4_backup.c.

**The rebuild is self-validating: the permuter reports base_score 30 against s4's 300.**
That is the workspace independently confirming the chassis improvement the engine sandbox
measured, through a different scorer and a different compile path.

Campaign telemetry (label s13-d1-2floor-chassis, pid 1453781, -j 8, --stop-on-zero):
**52,368 iterations across three 9-minute fresh-seed windows, ZERO novel finds, zero
finds at or below base_score 30.** Every one of the 12 output dirs in the workspace is an
s4-era find (`"new": false` in the harvest report). Harvested with `--stop`; campaign
confirmed dead (`permuter_campaign.py status`: alive false, 52,603 iterations). This is a
strictly stronger negative than s4's: s4 at least produced 12 finds on its chassis, and
the permuter's randomized statement/expression mutations over 52k samples never once
produced a body that improves on the two-instruction residual.

### Four hand spellings of the sched2 lever, measured
| variant | score | note |
|---|---|---|
| e1 `kc = dz * ax - az * (x2 - x0)` (product operand order swapped) | **3** | regressed; the mult operand order is itself a matched byte |
| e2 `dx` named FOURTH (ax, dz, az, dx) | 2 | inert — naming dx after az does not move its birth (contrast s12 d5, dx named third, = 9) |
| e3 `s32 q = (x2 - x0) * az;` (the product named) | **36** | scrambles the block-7 multiply emission order wholesale |
| e4 `bz` named AFTER dx rather than before | 2 | inert, same residual as the candidate |

e2 and e4 confirm the s12 span model from the other side: an insn added AFTER dx's birth
changes nothing, only one added strictly BETWEEN dz's and dx's births does — and that is
exactly the insn that then loses the sched2 LUID tie.

### The endgame-lock gates
- Gate (a) canonical-asm: `python3 tools/scan_hand_coded.py --single func_8002D780` →
  **tier=LOW, score=1/8** (only S4 front-loads fires; S1 multu pacing, S2 empty branch,
  S3 no-spills, S5 cluster, S6 BIOS jumptable, S7 unsaved $sN, S8 redundant mask all
  negative). FAILS. Separately: this function IS a member of the cop2
  materialize-then-copy cluster (.claude/rules/cop2-addressing-preamble-cluster.md:75),
  but that grant's mechanical check condition 1 is `sandbox --disable all == 0` and its
  own load-bearing negative says membership "closes the TAIL ISLAND only" — the three
  cop2 islands in this body are already the authorized canonical form and are NOT where
  the 2 residual instructions are. The grant is unavailable and irrelevant at floor 2.
- Gate (b) SOTN-master precedent for a closing construct: **no construct is in hand to
  cite one for.** The residual is a sched2 ready-list tie between two independent subus
  with equal INSN_PRIORITY and equal dependence class, decided by INSN_LUID
  (tools/gcc-2.7.2/sched.c:2464), where the LUID ordering that would win the tie is the
  same property that buys the register seats (s12 d5, measured 9). Census of
  docs/reference/sotn-construct-index.md (1,365 entries): the only scheduling-related
  classes are `match_comment` (229 codegen-reason comments) and `new_var_temp` (20 named
  RA/sched temporaries) — and the named-temporary shape is what this candidate ALREADY
  uses for ax/dz/az. Zero entries are a construct that forces a transposition of two
  priority-tied independent insns. NEGATIVE census → gate FAILS.

- [s13] Chassis re-measured at dispatch: HEAD 4a44c3ce (-mel -msoft-float), s12 candidate (d1_az_hoisted) spliced into src/code6cac_b.c, `sandbox func_8002D780 --disable all` = 2/202 with build_insns == target_insns == 202.
- [s13] The s4 permuter workspace was re-based onto the 2-floor candidate (tmp/grind/func_8002D780/s13/mkbase.py; s4 base.c preserved as s13/base_s4_backup.c). The permuter's own scorer independently confirms the chassis improvement: base_score 30 against s4's 300.
- [s13] PERMUTER CAMPAIGN ON THE 2-FLOOR CHASSIS: 52,368 iterations, three 9-minute fresh-seed windows, ZERO novel finds and no find at or below base_score 30; harvested with --stop, campaign dead. Frontier item 3 from s12 is spent with a negative result.
- [s13] Four hand spellings of the sched2 ready-list lever measured: kc's second-product operand order swapped = 3 (regression - the mult operand order is a matched byte); dx named FOURTH = 2 (inert); the second product named as its own intermediate = 36 (scrambles the block-7 multiply order); bz named after dx = 2 (inert). e2/e4 confirm the s12 span model from the other side - an insn added after dx's birth is inert, only one strictly between dz's and dx's births moves the seats.
- [s13] Gate (a) FAILS: scan_hand_coded --single func_8002D780 = tier LOW, score 1/8 (only S4 front-loads). The cop2-cluster membership (.claude/rules/cop2-addressing-preamble-cluster.md:75) does not help: its mechanical check condition 1 is sandbox == 0 and its load-bearing negative scopes the grant to the TAIL ISLAND, which is already authorized canonical form here and is not where the 2 residual instructions sit.
- [s13] Gate (b) FAILS: negative census of docs/reference/sotn-construct-index.md - the only scheduling-related classes are match_comment (229) and new_var_temp (20 named RA/sched temporaries, the shape this candidate already uses for ax/dz/az); zero entries are a construct that forces a transposition of two priority-tied, class-tied independent insns.

## s14 — escalation (disposition re-file)

- [s14] The s13 disposition entry was DISCARDED by the driver purely on HEADING FORMAT: the
  owner-gated check in `tools/grinder/grind.ps1:1339` requires a SINGLE LINE in
  docs/grind/decisions.md that matches BOTH `OWNER-ESCALATION|CANONICAL-ASM GRANT PATH` AND the
  function name. s13's title (`## <date> — func_8002D780 — **RESOLVED BY STANDING RULING
  (2026-07-27): FORECLOSED**`) is the wording the brief prescribes but carries neither token, so
  the entry was invisible to the check. The re-filed entry at docs/grind/decisions.md:25758 uses
  `## 2026-09-08 — func_8002D780 — OWNER-ESCALATION — **RESOLVED BY STANDING RULING
  (2026-07-27): FORECLOSED**`, which satisfies the line check while still routing to the SILENT
  foreclosure branch (grind.ps1:1436 keys on `RESOLVED BY STANDING RULING` in escalation_ref).
  NOTE FOR FUTURE ESCALATION SESSIONS ON ANY FUNCTION: put `OWNER-ESCALATION` in the heading line
  alongside the standing-ruling phrase.
- [s14] Chassis re-measured at HEAD 551f3dae (-mel -msoft-float) with candidate.c spliced into
  src/code6cac_b.c: `sandbox func_8002D780 --disable all` = 2/202, build_insns == target_insns ==
  202, cheat_asm_stripped 24. Floor unchanged from s5-s13.
- [s14] Gate (a) re-run independently: `scan_hand_coded.py --single func_8002D780` = tier LOW,
  score 1/8 (only S4 front loads: 6 loads in an 8-insn window @ insn 163). 212 insns, 7 spills,
  27 distinct regs.
- [s14] KILL RE-AUDIT (mandated by the flat-floor rule) via `tools/fake_ablate.py`: the candidate
  carries exactly ONE FAKE unit (the same-value re-store of `m`, candidate.c L142). keep-all = 2,
  drop-1 = 6 — load-bearing, worth 4 insns, and it occupies the `m`/`lzcr` pseudo in the sqrt
  block, not the `dz`/`dx`/`az` pseudos of the residual. The s12/s13 sched-lever kills are
  therefore NOT contaminated by a FAKE carrier sitting on their target pseudo.
- [s14] New measurement: variant f1 swaps the operand order of the FIRST product only
  (`kc = ax * dz - (x2 - x0) * az;`) = 3/202, a regression. s13's e1 had swapped the SECOND
  product (also 3). Both mult operand orders are matched bytes. Banked as
  rejected/first-product-operand-swap-3.c.

- [s13] The s13 session was discarded on HEADING FORMAT alone, not on merit: grind.ps1:1339 requires a SINGLE line in docs/grind/decisions.md matching BOTH 'OWNER-ESCALATION|CANONICAL-ASM GRANT PATH' AND the function name, and the brief-prescribed title '## <date> - func_8002D780 - **RESOLVED BY STANDING RULING (2026-07-27): FORECLOSED**' carries neither token. The re-filed entry (docs/grind/decisions.md:25758) reads '## 2026-09-08 - func_8002D780 - OWNER-ESCALATION - **RESOLVED BY STANDING RULING (2026-07-27): FORECLOSED**', which passes the line check while still routing to the SILENT foreclosure branch (grind.ps1:1436 keys on 'RESOLVED BY STANDING RULING' in escalation_ref).

- [s13] Floor re-measured this session at HEAD 551f3dae (-mel -msoft-float) with candidate.c spliced into src/code6cac_b.c: sandbox func_8002D780 --disable all = {score 2, target_insns 202, build_insns 202, cheat_asm_stripped 24}. Floor 2 <= 5, so the 2026-07-27 standing ruling is the governing disposition.

- [s13] Gate (a) canonical-asm re-run independently this session: scan_hand_coded.py --single func_8002D780 = tier LOW, score 1/8 (212 insns). Only S4 fires (6 loads in an 8-insn window @ insn 163); S1 multu pacing (0 multu/mflo pairs), S2 empty branch, S3 no-spills (7 spills, 27 distinct regs), S5 cluster (jaccard < 0.5), S6 BIOS jumptable, S7 unsaved $sN, S8 redundant mask all negative. GATE FAILS.

- [s13] Gate (b) SOTN-master precedent FAILS: there is no closing construct to cite a precedent for. The residual is a sched2 ready-list tie between two independent subus with equal INSN_PRIORITY and equal dependence class, decided by INSN_LUID (tools/gcc-2.7.2/sched.c:2464), where the LUID ordering that would win the tie is the same source property that buys the register seats (s12 d5 = 9/202). Census of docs/reference/sotn-construct-index.md (1,365 entries) returns only match_comment (229) and new_var_temp (20 named RA/sched temporaries, the shape this candidate already uses for ax/dz/az); zero entries force a transposition of two priority-tied, class-tied independent insns. Any construct that did would be a scheduling barrier - a forbidden family by any spelling.

- [s13] The residual is exactly two instructions, an adjacent transposition inside block 7: ours[96] subu v0,a2,a3 (az) / ours[97] subu v1,t5,t1 (dx) against target[96] dx / target[97] az. Every register in the function and the reorg delay-slot fill are correct (tmp/grind/func_8002D780/s12/pairdiff_d1.txt).

- [s13] Nothing holds a byte-match: the function has never reached 0 and no cheat-asm holds one. It is committed as INCLUDE_ASM("asm/funcs", func_8002D780); on main; the candidate lives only in memory/grind/func_8002D780/ and carries one annotated FAKE (dead-store family, .claude/rules/dead-store-fake-exception.md) plus the three authorized cop2 islands.

- [s13] Exhaustion as re-tallied for the record: 14 sessions; modalities recon, structural x2, permuter, synthesis x2, solver, forensics x2, rederive x2, escalation x2; floor 56 -> 48 -> 47 -> 10 -> 7 -> 2, flat at 2 for seven consecutive sessions across five distinct modalities; 45 instance kills + 6 predicate-cited class kills; 67 rejected forms; 107,899 permuter iterations across two campaigns (55,531 on the 7-floor chassis, 52,368 on the 2-floor chassis with zero novel finds).

- [s13] CC1PSX SELF-DISPROOF (driver, ruling 2026-09-08): candidate c285cf7c83fd scores 2 under our cc1 and 46 under the original cc1psx — SOURCE-SIDE: the original compiler is no closer from this source, so the residual is a spelling not yet found (a pure-C preimage exists by construction).

- [s14] ENUMERATION (operator, 2026-09-08, tools/spelling_enum.py, out-of-tree scorer with the engine metric against build/src/code6cac_b.o at HEAD e1e1b977): test-3 region in fully-named form (ax, dz, dx, az, bx, bz; kc/kp assignments; the if as anchor) -> 3,914 distinct spellings across name/inline x declaration order x kc/kp order, operand order held as candidate.c. Histogram: 981 at floor 2, 488 at 4, 488 at 9, 523 at 29, 488 at 30, 458 at 35, 488 at 37. ZERO below 2. CLASS KILL for that axis set: no naming/ordering of the six differences reaches the target's dx-before-az order without losing the seats. The 62,624-spelling run that adds the commutative operand-swap axis (both products of kc and kp) is running; its histogram will be banked when it completes (partial: 13k scored, best still 2).

- [s14] ROTATION RULING CONTEXT (2026-09-08): this function's foreclosure record (decisions.md 25758) claimed only a class grant or a toolchain change could reopen it. The cc1psx self-disproof (engine cc1psx-check: ours 2/202, original cc1psx 46/202, same az-before-dx order) proves the residual is SOURCE-SIDE. Policy does not block this function; a pure-C preimage exists by construction. Item is now `rotated` and returns automatically (queue drain / toolchain change / sibling movement). Next instruments when it returns: enumerate with a WIDER region (fold the test-2 block and the cx/cz centroid computation into the marked region; try dz/dx declared at function scope vs block scope), and the declaration-scope axis the enumerator does not yet cover.


## s14 (rederive, 2026-09-08) - sibling transplant executed; residual re-attributed from sched2 to source order + a local-alloc span tie

**Directive executed.** The queue's auto-return directive (coupled sibling func_8002E6B0 ->
floor 0, 2026-09-08T18:29) was this session's first probe. func_8002E6B0 is COMPLETED-C on
main at src/code6cac_b.c:1332-1364 and is the SAME point-in-triangle predicate that
func_8002D780 inlines with one vertex at the origin: three cross-product sign tests against
a centroid and a query point. Its matched spelling names the two edge differences per test
(`s32 dz = argN[2] - argM[2]; s32 dx = argN[0] - argM[0];` inside a brace block) and leaves
the centroid/query differences inline. Transplanted onto this chassis it scores 2/202 -
same floor, DIFFERENT residual: the delay-slot pair ax/dz at ours[92:93] instead of the
block-7 pair az/dx at ours[96:97] (tmp/grind/func_8002D780/s14/pairdiff_a_sibling_dz_dx.txt).
Both spellings hold every register in the function; they differ only in which of the two
adjacent transpositions is left over. Banked as
rejected/s14-sibling-8002e6b0-transplant-slot-wrong-2.c.

**Chassis re-measured.** HEAD e3895bb7 (-mel -msoft-float), candidate.c spliced into
src/code6cac_b.c: `sandbox func_8002D780 --disable all` -> {"score": 2, "target_insns": 202,
"build_insns": 202}. The ledger's floor 2 is intact on the current chassis.

**PASS RE-ATTRIBUTION (the s12 header was wrong).** s12 recorded the residual as "a pure
sched2 ready-list order between two independent subus". The dumps say otherwise:

- sched1 ALREADY emits az before dx. tmp/grind/func_8002D780/s14/w_n1/code6cac_b.sched,
  func_8002D780 block 7: insn 179 (ax = cx - x0), 182 (dz = z2 - z0), 191 (mult1),
  185 (az = cz - z0), 188 (dx = x2 - x0), 193 (mult2), 198, 200, 202, 204.
- sched2 does not move them. RANKDBG in w_n1/stderr_full.txt:
  `RANKDBG last=193 y=188 cls=3 x=185 cls2=3 val=0` - equal INSN_PRIORITY, equal dependence
  class against the mult, so rank_for_schedule falls through to `INSN_LUID (tmp) -
  INSN_LUID (tmp2)` (tools/gcc-2.7.2/sched.c:2464), i.e. to SOURCE ORDER.
- Therefore the az/dx emission order IS the declaration order, in both scheduler passes.

**The coupling, in exact arithmetic.** local-alloc block-7 quantity tables (BB2_QTY_DEBUG),
func_8002D780's group identified by pseudo numbers 129-132. Positions are 2 units per RTL
insn: ax=2, dz=4, mult1=6, [3rd decl]=8, [4th decl]=10, mult2=12, (px-x0)=14, mult3=16,
(pz-z0)=18, mult4=20.

| body | order | dz (reg130) | dx | seats |
|---|---|---|---|---|
| n1 = candidate (ax,dz,az,dx) | az 3rd | birth 4 death 16 refs 3 -> pri 2500, got $a0 | reg132 birth 10 death 20 refs 3 -> pri 3000, got $v1 | CORRECT |
| n6 (ax,dz,dx,az) = target order | dx 3rd | birth 4 death 16 refs 3 -> pri 2500, got $v1 | reg131 birth 8 death 20 refs 3 -> pri 2500, got $a0 | REVERSED, 9/202 |

qty_compare_1 (tools/gcc-2.7.2/local-alloc.c:1708-1719) is
`floor_log2(refs)*refs*size / (death - birth) * 10000` with a suggestion pre-test and NO
tie-break of its own; on an exact tie qsort leaves the two quantities in quantity-number
order, and dz's quantity is created first because its definition insn is first. So in the
target's OWN emission order dz is seated first and takes $v1 while dx takes $a0 - the
reverse of the target's $a0/$v1. Writing the condition out: dx outranks dz iff
`pos(dx_def) - pos(dz_def) > pos(mult4) - pos(mult3)`. In the target's asm those are 2
positions each (asm/funcs/func_8002D780.s:103-117: ax 103, dz 104, mult 105, dx 107,
az 108, mult 109, px0 111, mult3 113, pz0 115, mult4 117), so the two sides are equal and
the tie is structural to the target's own instruction sequence.

**The obvious way out is dead: sched1 sinks every difference back to its consumer.** Six
bodies (tmp/grind/func_8002D780/s14/w2/) name or hoist the kp/kc difference subexpressions
in the target declaration order, trying to change the birth/death arithmetic: pz0 named,
px0+pz0 named, px0 named between dz and dx, pz0 named between dz and dx, pz0 in a nested
brace after kc, and both kp differences named. All six score 9/202, and the dumped block-7
quadruples are byte identical to the un-hoisted body in all of them (w_n6, w_w2b, w_w2c:
dz birth 4 death 16 refs 3; dx birth 8 death 20 refs 3). sched1 places each difference
immediately before its consuming mult regardless of where the C declares it, so the only
C-visible freedom left in block 7 is the order WITHIN the two pairs (ax,dz) and (dx,az).

**Candidate promoted.** memory/grind/func_8002D780/candidate.c is now the s12 chassis plus
the sibling's `dx` naming (all four block-7 differences are named once-written CSE locals,
no duplicated inline `(x2 - x0)`); 2/202, residual unchanged at ours[96:97]. The previous
body scores the same but duplicates a subexpression, so this one is closer to the sibling's
matched idiom and strictly easier to defend.

- [s14] Owner directive executed: the queue's auto-return directive (coupled sibling func_8002E6B0 -> floor 0, 2026-09-08T18:29) was this session's first probe, and the transplant is banked as a CONFIRMED hypothesis naming the sibling and its session number.

- [s14] Chassis re-measured on HEAD e3895bb7 (-mel -msoft-float): candidate.c spliced into src/code6cac_b.c gives {"score": 2, "target_insns": 202, "build_insns": 202}. The ledger floor is intact.

- [s14] The two 2/202 spellings are complementary, not redundant: the sibling spelling leaves the delay-slot ax/dz transposition and gets the block-7 pair right; the candidate leaves the block-7 az/dx transposition and gets the delay slot right. Both hold every register in the function.

- [s14] Position map for block 7 (2 units per RTL insn, from the QTYDBG tables): ax=2, dz=4, mult1=6, third decl=8, fourth decl=10, mult2=12, (px-x0)=14, mult3=16, (pz-z0)=18, mult4=20. dz is always born at 4 and dies at 16; dx dies at 20; only dx's birth moves.

- [s14] The coupling stated exactly: dx outranks dz in qty_compare_1 iff pos(dx_def) - pos(dz_def) > pos(mult4) - pos(mult3). In the target's own asm both sides are 2 positions (asm/funcs/func_8002D780.s:103-117: ax 103, dz 104, mult 105, dx 107, az 108, mult 109, px0 111, mult3 113, pz0 115, mult4 117), so on the target's emission order the two quantities tie exactly.

- [s14] qty_compare_1 (tools/gcc-2.7.2/local-alloc.c:1708-1719) has no tie-break clause of its own - unlike qty_sugg_compare_1, which ends with 'return *q1 - *q2' - so a tie is resolved by qsort's array order, i.e. by quantity number, i.e. by which definition insn comes first.

- [s14] Seven new rejected forms banked (81 total in memory/grind/func_8002D780/rejected/), and candidate.c is replaced by the sibling-aligned four-named-difference body at the same 2/202 with the same residual and no duplicated subexpression.


## s15 (2026-09-08) - enumerate modality: the DECLARATION-SCOPE axis is inert

Chassis: HEAD e3895bb7 (`-mel -msoft-float`), `memory/grind/func_8002D780/candidate.c`
spliced into `src/code6cac_b.c` via `tmp/grind/func_8002D780/s15/splice.py`. Baseline
re-measured THIS session: `sandbox func_8002D780 --disable all` -> `{"score": 2,
"target_insns": 202, "build_insns": 202}`. Floor unchanged at 2/202.

### Sweep 1 (enum1) - in-block spelling space, re-measured IN TREE

`tools/spelling_enum.py --candidate tmp/grind/func_8002D780/s15/enum_base.c --max 20000`
over the test-3 region (`ax`, `dz`, `az`, `dx` fully named, the two `kc`/`kp`
assignments, the closing `if` as anchor) with the commutative-operand-swap axis ON:
**2,080 distinct spellings**, all built and scored with `tools/sweep_variants.py` in eight
chunks (`tmp/grind/func_8002D780/s15/enum1_c[0-7].json`, histogram in
`hist_enum1.txt`). Result:

    score  2: 41 (incl. the baseline)     score 20:  65     score 33:  60
    score  3: 64                          score 21:  16     score 34:  11
    score  4: 49                          score 22: 138     score 37:  49
    score  5: 33                          score 29:  65     score 38:  59
    score  6: 16                          score 30: 179     score 39: 183
    score  9: 65                          score 31: 201     score 40:  81
    score 16: 66                          score 32: 136     score 41:  16
    score 17: 161                         score 18: 219     score 19: 115

**Nothing below 2.** This is the in-tree, current-chassis confirmation of the operator's
out-of-tree 2026-09-08 enumeration result (62,624 spellings at HEAD e1e1b977, 981 at 2,
nothing below - auto-memory `func-8002d780-enumeration-class-kill`). Our 2,080 is the
4-local subset of that space; it reproduces the same verdict on the tree the grinder
actually scores, so the class kill is not an artefact of the out-of-tree scorer.

### Sweep 2 (enum4) - the DECLARATION-SCOPE axis, the instrument the class kill named

The 2026-09-08 memory's "How to apply" names the concrete next instrument: *"add a
declaration-scope axis to the enumerator"* - `tools/spelling_enum.py` has no scope axis, so
the question of whether declaring the block-7 difference locals in the ENCLOSING block
(pseudos created earlier, lower REGNO/LUID, values still computed at the same point inside
block 7) flips the dx/az quantity tie had never been measured. `tools/` is outside a grind
session's writable surface, so the axis was generated by a session-local generator,
`tmp/grind/func_8002D780/s15/gen_scope.py`:

  * axis S - every subset of {ax, dz, az, dx} (2^4 = 16) hoisted to the enclosing block as
    an UNINITIALISED declaration (`s32 dx, az;` immediately before `s32 kc = ...`), with the
    value still assigned inside block 7 (`dx = x2 - x0;`);
  * axis O - every permutation of those outer declarations;
  * axis I - every permutation of the surviving initialised inner declarations followed by
    every permutation of the hoisted inner assignments (C89 forces decls before statements,
    which is itself part of what the axis tests);
  * kc/kp order and all four operand orders fixed at the banked 2/202 spelling, since
    sweep 1 and the 62,624-spelling run already exhaust those axes.

**816 distinct variants**, swept in four chunks
(`tmp/grind/func_8002D780/s15/enum4_c[0-3].json`, histogram `hist_enum4.txt`):

    score 2: 412     score 4: 204     score 9: 204     (nothing else, nothing below 2)

Broken out by how many locals are hoisted (`hoisted k {score: count}`):

    hoisted 0  {2:  12, 4:   6, 9:   6}
    hoisted 1  {2:  12, 4:   6, 9:   6}
    hoisted 2  {2:  24, 4:  12, 9:  12}
    hoisted 3  {2:  72, 4:  36, 9:  36}
    hoisted 4  {2: 288, 4: 144, 9: 144}

The distribution is the SAME 2:1:1 trichotomy at every hoist level. Moving a block-7
difference local's DECLARATION to the enclosing block changes the score by exactly nothing:
the score is a function of the inner STATEMENT order alone (which of `az`/`dx` is written
first, and whether `ax` sits between `dz` and `dx`), identically to the un-hoisted body.
That is the mechanism, stated plainly: an uninitialised outer declaration emits no RTL, so
it creates no pseudo activity at the point of declaration - the pseudo's first reference,
and therefore its local-alloc birth, its quantity-creation order and its sched LUID, are all
still set by the ASSIGNMENT insn inside block 7. Declaration scope is not an input to
`qty_compare_1` (`tools/gcc-2.7.2/local-alloc.c:1708`) at all.

A further collapse worth recording: the in-block axis produces 24 distinct scores over 2,080
spellings, the scope axis produces 3 over 816. Hoisting the declaration does not open a new
region of the search space - it MERGES spellings that the in-block axis distinguished.

### Artefacts
`tmp/grind/func_8002D780/s15/`: `enum_base.c`, `enum_base2.c`, `enum_base3.c` (marked
bodies), `gen_scope.py`, `chunk.py`, `hist.py`, `sweep.ps1`, `sweep_all.ps1`, `splice.py`,
`enum1/` + `enum1_c[0-7].json`, `enum4/` + `enum4_c[0-3].json`, `hist_enum1.txt`,
`hist_enum4.txt`. Banked forms: `rejected/s15-scope-hoist-all-four-outer-decl-inert-2.c`,
`rejected/s15-scope-hoist-inner-order-dx-before-az-9.c`.

- [s15] Baseline re-measured THIS session on the dispatch chassis: candidate.c spliced into src/code6cac_b.c, `sandbox func_8002D780 --disable all` -> {"score": 2, "target_insns": 202, "build_insns": 202}. The dispatch brief reported the HEAD floor as 'measurement unavailable'; it is 2/202.

- [s15] Sweep throughput measured at ~0.76 s per variant end-to-end (2,896 variants built and scored this session), which is the number a future enumerate session should budget with.

- [s15] The in-block spelling axis produces 24 distinct scores over 2,080 spellings; the declaration-scope axis produces 3 over 816. Hoisting a declaration to the enclosing block does not reach a new region of the search space, it merges spellings the in-block axis distinguished.

- [s15] The score under the scope axis is fully determined by the inner statement order: whether `az` or `dx` is written first, and whether `ax` sits between `dz`'s and `dx`'s definitions. Every combination that keeps `az` before `dx` with `ax` between them scores 2; the two other inner orders score 4 and 9, at every hoist level.

- [s15] The remaining region named by the 2026-09-08 class-kill memory is the OUTER centroid/test-1 declaration list (x0, x2, z0, z2, cx, cz, px, pz, kc, kp) feeding block 7. Its space was counted this session: 16,384 valid def-before-use orders with every name kept (all ten are read outside the region, so the inline axis is empty), and >900,000 with commutative swaps on the six products. A marked body is ready at tmp/grind/func_8002D780/s15/enum_base3.c.

- [s15] src/code6cac_b.c was restored to its committed INCLUDE_ASM state at the end of the session; the only tree changes are the two banked rejected forms, the ledger appends, and the candidate.c header note.

## s16 (structural, 2026-09-08) — the QUADRANT MODEL: the whole residual is two bits

**Directive executed.** The queue's auto-return directive (coupled sibling func_8002E6B0 →
floor 0) was already executed and measured in s14 (sibling transplant, 2/202 with the
residual moved to the ax/dz pair, banked as
`rejected/s14-sibling-8002e6b0-transplant-slot-wrong-2.c`). s16 did not re-run it; instead
it explains that result, because the quadrant model below subsumes it exactly.

**Chassis.** HEAD 82ab11bd (`-mel -msoft-float`), `memory/grind/func_8002D780/candidate.c`
spliced into `src/code6cac_b.c`: `sandbox func_8002D780 --disable all` → 2/202,
build_insns == target_insns == 202. Floor unchanged. 528 variants swept this session in four
batches, all through `tools/sweep_variants.py`.

### The model (new, and it closes the interpretation of every sweep in the ledger)

Block 7 emits four `subu`s. Two of them feed the FIRST product of each cross product
(`ax`, `dz` → `mult1`), two feed the SECOND (`dx`, `az` → `mult2`). sched1 schedules by
`INSN_PRIORITY`, which groups them by which mult they feed; WITHIN each pair the priorities
and dependence classes tie, so `rank_for_schedule` falls through to `INSN_LUID`
(`tools/gcc-2.7.2/sched.c:2464`) = source order. Therefore the emission order of the block
is decided by exactly TWO independent bits: whether `ax` is declared before `dz`, and
whether `dx` is declared before `az`. The score over all 24 declaration orders is a pure
function of those two bits, measured exhaustively (`tmp/grind/func_8002D780/s16/va.json`,
group `000`):

| ax before dz | dx before az | score | orders |
|---|---|---|---|
| no  | no  | 4 | CXZA CZAX CZXA ZACX ZCAX ZCXA |
| no  | yes | **2** | XCZA XZAC XZCA ZAXC ZXAC ZXCA |
| yes | no  | **2** | ACXZ ACZX AZCX CAXZ CAZX CXAZ |
| yes | yes | 9 | AXCZ AXZC AZXC XACZ XAZC XCAZ |

The target's own emission order is `ax` (in the test-2 branch delay slot), `dz`, `mult1`,
`dx`, `az`, `mult2` — i.e. the (yes, yes) quadrant. Pairdiffs of all twelve score-2 orders
(`tmp/grind/func_8002D780/s16/pairdiff_*.txt`) confirm the partition exactly and show that
the two score-2 classes are COMPLEMENTARY, each leaving one adjacent transposition with
every register in the function correct:

- (yes, no) — six orders, incl. the banked `AZCX` chassis: residual at ours[96:97],
  `subu v1,t5,t1` (`dx`) must move one slot earlier past `subu v0,a2,a3` (`az`).
- (no, yes) — six orders, incl. the s14 sibling transplant: residual at ours[92:93],
  `subu v0,t2,t1` (`ax`) must move one slot earlier past `subu a0,t0,a3` (`dz`), i.e. into
  the `bltz` delay slot that reorg fills.

Score 4 is the (no, no) quadrant: BOTH transpositions, seats still correct. Score 9 is the
(yes, yes) quadrant: both emission orders correct, but `dz` and `dx` then tie exactly in
`qty_compare_1` (`tools/gcc-2.7.2/local-alloc.c:1708`) — equal refs (3 each) and equal
spans (12 each, because `dz` is born 4 LUIDs before `dx` and also dies 4 LUIDs before it) —
and qsort leaves the tied pair in quantity-number order, which seats `dz` first and swaps
`$v1`/`$a0` through the rest of the block.

**So the function's whole remaining residual is one sentence:** reach the (yes, yes)
quadrant while making `dx` beat `dz` in `qty_compare_1`. Everything else is solved.

### The five structural axes measured this session (all inert or worse)

Every one of them was swept across all 24 declaration orders, and every inert one
reproduces the `{2:12, 4:6, 9:6}` trichotomy BYTE-FOR-BYTE — i.e. the axis does not reach
the tie at all.

1. **kc/kp variable identity** (`va.json`, 192 variants). Fresh `kc2/kp2` for test 2 alone:
   inert. Fresh `kc3/kp3` for test 3 alone: inert. Both: 36–43. Fresh `kc1/kp1` for test 1:
   45–66. Adding quantities to block 7 does not renumber the `dz`/`dx` pair.
2. **Hoisting the difference COMPUTATIONS** out of block 7 (`vb.json`, 192 variants — every
   non-empty subset, two destination blocks, all orders). Best 19, mode 41. This is the
   axis s15's enum4 could not reach (enum4 hoisted only uninitialised declarations, no RTL).
   A hoisted pseudo spans two blocks, leaves local_alloc for global_alloc, and the value is
   computed on the test-2-fails path too.
3. **Block SHAPE of the three tests** (`vc.json`, 168 variants) — the axis the s15 frontier
   named. `goto` early-out (tests as straight-line siblings with a label before the sphere
   block) and `inner` (test-3 differences in their own brace scope with the `if` outside)
   are EXACTLY inert. A `hit` flag tested after the nest closes costs 8–27. Block structure
   is not an input to the residual.
4. **Both-products sign flip** (`vc.json`, the `s1` half): a uniform **+22** in every one of
   the four shapes. `(kc ^ kp) >= 0` is invariant under negating both products, but the
   re-association swaps the pair grouping and costs 22 instructions.
5. **Single-product flip with a compensating `< 0` branch** (`ve.json`, 48 variants). The
   one construction found that lengthens `dz`'s live range past `dx`'s without touching the
   declaration order. Best 21, worst 39 — the inverted branch polarity does not stay local.
6. **PARTIAL inlining** (`vd.json`, 120 variants) — writing ONE of a difference's two uses
   as its expression, to drop `dz` from 3 references to 2 and win `qty_compare_1` on the
   `floor_log2(refs)*refs` numerator. `tools/spelling_enum.py` cannot express this (its
   name/inline axis is all-or-nothing). All five modes reproduce `{2:12, 4:6, 9:6}`
   EXACTLY: cse re-folds the duplicated subexpression onto the same pseudo and the
   reference count never drops.

### Mandated kill re-audit (flat floor ≥ 3 sessions)

The closest instance kill to the target is the (yes, yes) quadrant itself — `AZXC`
(`ax, dz, dx, az`, the target's own emission order), killed at 9 in s14. Re-measured this
session on the current chassis AND with the FAKE ablated
(`python3 tools/fake_ablate.py --func func_8002D780 --file code6cac_b --candidate
tmp/grind/func_8002D780/s16/va/a_000_AZXC.c`): keep-all **9**, drop-1 **13**. The baseline
is 2 / 6 under the same ablation, so the +7 penalty of the quadrant is identical with and
without the FAKE carrier. The `m` re-store does not sit on the `dz`/`dx` pseudos and is not
masking this lever; the s14 kill stands on the current chassis.

- [s16] Chassis re-measured this session: HEAD 82ab11bd (-mel -msoft-float), memory/grind/func_8002D780/candidate.c spliced into src/code6cac_b.c -> sandbox func_8002D780 --disable all = score 2, target_insns 202, build_insns 202. Floor unchanged; src/ restored byte-exact afterwards.

- [s16] 528 variants swept this session in five batches through tools/sweep_variants.py (va 192, vb 192, vc 168, vd 120, ve 48 - 720 counting overlap-free totals per batch file), plus 12 tools/pairdiff.py runs. All JSON histograms and pairdiffs are banked under tmp/grind/func_8002D780/s16/.

- [s16] THE QUADRANT MODEL: the four block-7 subus split into two sched1 priority groups by which mult they consume (ax,dz -> mult1; dx,az -> mult2); within each group INSN_PRIORITY and dependence class tie so rank_for_schedule falls through to INSN_LUID = source order (tools/gcc-2.7.2/sched.c:2464). Score is therefore a pure function of two bits - (ax<dz) and (dx<az) - verified over all 24 orders: (F,F)=4 {CXZA CZAX CZXA ZACX ZCAX ZCXA}, (F,T)=2 {XCZA XZAC XZCA ZAXC ZXAC ZXCA}, (T,F)=2 {ACXZ ACZX AZCX CAXZ CAZX CXAZ}, (T,T)=9 {AXCZ AXZC AZXC XACZ XAZC XCAZ}.

- [s16] Pairdiffs of all twelve score-2 orders show exactly two complementary residual classes, each one adjacent transposition with EVERY register in the function correct: (T,F) leaves ours[96:97] subu v1,t5,t1 (dx) needing to move one slot earlier past subu v0,a2,a3 (az); (F,T) leaves ours[92:93] subu v0,t2,t1 (ax) needing to move one slot earlier past subu a0,t0,a3 (dz), i.e. into the bltz delay slot reorg fills. The target's order is ax(slot), dz, mult1, dx, az, mult2.

- [s16] The (T,T) quadrant - the target's own emission order - costs 9 rather than 2 because dz and dx then tie EXACTLY in qty_compare_1 (tools/gcc-2.7.2/local-alloc.c:1708): 3 references each, span 12 each (dz born 4 LUIDs earlier and dying 4 LUIDs earlier), and qsort leaves the tied pair in quantity-number order, seating dz first. The whole remaining problem is one sentence: reach (T,T) while making dx beat dz in qty_compare_1.

- [s16] Every structural axis that is INERT reproduces the {2:12, 4:6, 9:6} trichotomy BYTE-FOR-BYTE, which is itself the proof that the axis never reaches the tie: fresh kc/kp in test 2 alone, fresh kc/kp in test 3 alone, goto early-out block shape, inner-brace scoping, and all five partial-inline modes.

- [s16] Axes measured WORSE, with their best score: hoisting the difference computations out of block 7 (19), the hit-flag tail shape (8), the both-products sign flip (+22 uniformly in all four block shapes), the single-product flip with a compensating < 0 branch (21), fresh kc/kp in both tests 2 and 3 (36), fresh kc1/kp1 in test 1 (45).

- [s16] Mandated kill re-audit: tools/fake_ablate.py on the (T,T) form a_000_AZXC.c gives keep-all 9 / drop-1 13 against the baseline's 2 / 6. The single FAKE unit (the m re-store) is worth 4 instructions in both bodies and does not sit on the dz/dx pseudos, so the s14 quadrant kill is not FAKE-masked.

- [s16] The queue's auto-return directive (coupled sibling func_8002E6B0 -> floor 0) was already executed and measured in s14; s16 did not re-run it because the quadrant model explains its result exactly - the sibling's spelling lands in the (F,T) class, which is why it scored 2 with the residual moved to the ax/dz delay-slot pair rather than improving.

- [s16] Ledger totals after s16: floor 2/202 flat for eight sessions; 53 instance kills and 8 predicate-cited class kills; 88 rejected forms banked.

## s17 (2026-09-08, synthesis) - floor 2/202, chassis unchanged

### What this session did
Re-read the whole ledger, re-derived the residual from the COMPILER SOURCE and the
TARGET ASM rather than from any banked narrative, ran the mandated kill re-audit, and
measured 52 new variants across the two axes the s16 frontier named. The floor did not
move; three axes closed, one of them a class kill that saves the next session a
16,384-variant / ~3.5 h sweep.

### The residual, fully derived (this supersedes every earlier partial account)
Block 7 at local-alloc time, in the target's own emission order (the (T,T) quadrant of
the s16 model), is exactly 14 insns.  Read off asm/funcs/func_8002D780.s:103-121 and
cross-checked against the w_n6 QTYDBG table (qty birth = 2 * insn index):

     1  subu ax   (cx - x0)      <- reorg later pulls this into the test-2 branch slot
     2  subu dz   (z2 - z0)          birth 4
     3  mult      dz * ax
     4  subu dx   (x2 - x0)          birth 8
     5  subu az   (cz - z0)
     6  mult      dx * az
     7  subu qx   (px - x0)
     8  mult      dz * qx            dz death 16
     9  subu qz   (pz - z0)
    10  mult      dx * qz            dx death 20
    11  subu kc   12  subu kp   13  xor   14  bgez

qty_compare_1's priority is floor_log2(refs) * refs * size / (death - birth) * 10000
(tools/gcc-2.7.2/local-alloc.c:1689-1712).  dz = 1*3*1/12 -> 2500; dx = 1*3*1/12 ->
2500.  An EXACT tie.  qty_compare_1 then falls through to "return *q1 - *q2;"
(local-alloc.c:1719), i.e. to the quantity NUMBER, and quantity numbers are handed out
by alloc_qty's next_qty++ (local-alloc.c:284) from block_alloc's FORWARD insn scan
(local-alloc.c:1169-1175).  Any body in the (T,T) quadrant emits dz's subu before dx's
by definition of the quadrant, so dz's quantity number is necessarily the lower one, dz
is always seated first, takes $v1, and dx takes $a0 - the exact mirror of the target
($a0 for dz, $v1 for dx).  That is the whole of the 9-point (T,T) penalty.

### The three levers the arithmetic allows, and their status
  L1  dx's qty_n_refs -> 4 (numerator 3 -> 8, priority 2500 -> 6666).  qty_n_refs is
      reg_n_refs of the pseudo PLUS the refs of any pseudo that combine_regs ties into
      the quantity (local-alloc.c:1929 "qty_n_refs[sqty] += reg_n_refs[sreg];").  OPEN -
      no ordinary-C realisation found yet; s16 proved the reference cannot be REMOVED
      from dz (cse refolds a partial inline), and s17 found no way to ADD one to dx.
  L2  dz's span > 12: one more insn between insn 2 and insn 4.  Only source-order
      levers reach there, and any of them re-orders the emitted pair (that is exactly
      what az declared third does - it is the banked 2/202 body).
  L3  dx's span < 12: dx's last use must be at insn <= 9, i.e. the qz subu must not sit
      between the two kp multiplies.  MEASURED DEAD this session (below).

### Measurements (52 variants, tmp/grind/func_8002D780/s17/vA.json + vE.json)
  - Group A, the s15/s16 frontier's "outer declaration list" item: 24 sampled
    def-before-use orders of x0,x2,z0,z2,cx,cz,px,pz on the (T,T) chassis.  5 stay at 9,
    the other 19 cost 17-31.  NOTHING below 9.  The mechanism above says why it can
    never help: quantity numbers come from the block scan, not from pseudo numbering,
    so no outer permutation can put dx's quantity before dz's.  This closes the item
    without the 16,384-variant sweep the s15/s16 frontier proposed.
  - Group B, the L3 axis: 8 further spellings of the two kp differences (both named
    before kp, named in reverse order, one named only, declared at the top of block 7,
    split-init accumulation).  Every naming/scoping spelling scores EXACTLY 9 - byte for
    byte the plain body - because sched1 re-sinks each difference to immediately before
    its consuming mult, so insn 9 stays between mult 8 and mult 10.  This reproduces and
    extends s14's six-spelling result on the current chassis.  The two split-init forms
    (kp = a; kp -= b;  and  kp = -(b); kp += a;) cost 36 and 32.
  - Group E, NEW axis - naming a PRODUCT rather than a difference.  A named product
    forces its multiply to expand before the other operand of the subtraction, which is
    the only ordinary-C construction that moves dx's last use earlier than dz's without
    inverting a branch (the s16-5 route).  Naming the FIRST product of either cross
    product is exactly inert (9, all four combinations).  Naming the SECOND product does
    reverse the multiply order - and costs 25 (both), 30 (kc only), 32 (kp only).  The
    seat gain is never worth the emission-order loss.
  - Mandated kill re-audit, current chassis (HEAD df6966b7, -mel -msoft-float):
    tools/fake_ablate.py on the (T,T) body tmp/grind/func_8002D780/s17/tt_chassis.c ->
    keep-all 9, drop-1 13; baseline 2 / 6.  The m re-store FAKE contributes the same
    4 instructions to both bodies, so the (T,T) kill is not a FAKE-masking artefact.
    Identical to the s16 re-audit at HEAD 82ab11bd, on a newer chassis.

### The standing contradiction, stated precisely for the next session
The target's OWN emission order produces an exact qty_compare_1 tie that GCC 2.7.2 must
break toward dz, yet the target bytes show dx seated first ($v1) and dz second ($a0).
Both cannot be true of the same quantity table.  Therefore the original source's block 7
did NOT present local-alloc with the quantity table we are producing: dx's quantity had
a higher priority than dz's, and by the arithmetic above that means dx had a fourth
reference (L1) or dz's span was longer (L2).  Every way of reaching those through the
declarations of ax/dz/az/dx (s15, 2,080 spellings), through their scope (s15, 816),
through the kc/kp variable identity, block structure, computation hoisting, partial
inlining, sign flips (s16, 528), through the outer declaration list (s17, 24) and
through the kp differences and the products (s17, 26) is now measured.  What has NEVER
been examined is the QUANTITY TABLE ITSELF: block 7 contains six difference pseudos,
four product pseudos, kc, kp and the xor result, but only SEVEN quantities appear in
w_n6's table.  Four product pseudos are unaccounted for - they are neither separate
quantities nor SUGG-pass quantities (there are no QTYDBG-SUGG lines for this block).
They are therefore already TIED into other quantities by combine_regs, and WHICH
quantity absorbs each product is the one input to the tie nobody has read.  If the kp
product can be made to tie into dx's quantity instead of wherever it goes now, dx's
refs become 3 + 2 = 5 and its death extends to the kp subu: priority
floor_log2(5)*5/16*10000 = 3125 against dz's 2500, and dx wins the seat with no change
to the emission order at all.  That is the single highest-value unread artefact in this
grind.

- [s17] Block 7 at local-alloc time in the target's own emission order is exactly 14 insns: 1 subu ax, 2 subu dz, 3 mult dz*ax, 4 subu dx, 5 subu az, 6 mult dx*az, 7 subu (px-x0), 8 mult dz*(px-x0), 9 subu (pz-z0), 10 mult dx*(pz-z0), 11 subu kc, 12 subu kp, 13 xor, 14 bgez. Read off asm/funcs/func_8002D780.s:103-121 and cross-checked against the w_n6 QTYDBG table (qty birth = 2 * insn index).

- [s17] qty_compare_1's priority is floor_log2(refs)*refs*size/(death-birth)*10000 (tools/gcc-2.7.2/local-alloc.c:1689-1712). dz = 1*3*1/12 -> 2500 and dx = 1*3*1/12 -> 2500: an exact tie, broken by `return *q1 - *q2;` at local-alloc.c:1719, i.e. by the quantity number.

- [s17] Quantity numbers are handed out by alloc_qty's next_qty++ (tools/gcc-2.7.2/local-alloc.c:284) during block_alloc's forward insn scan (local-alloc.c:1169-1175), which skips NOTEs (local-alloc.c:1174) and steps each real insn by 2 LUIDs. Pseudo numbering never enters any comparator in the file, which is why declaration-scope (s15) and outer-declaration-order (s17) sweeps are both inert.

- [s17] Only three levers can break the tie in dx's favour: L1 dx's qty_n_refs -> 4 (numerator 3 -> 8); L2 one more insn between insn 2 and insn 4 (lengthens dz's span alone); L3 dx's last use at insn <= 9 (shortens dx's span alone). L3 is measured dead this session; L2 is only reachable by source-order levers, all of which re-order the emitted pair (that is exactly the banked 2/202 body, where az declared third supplies the extra insn and costs the az/dx transposition).

- [s17] qty_n_refs is reg_n_refs of the pseudo PLUS the refs of any pseudo combine_regs ties into the quantity (tools/gcc-2.7.2/local-alloc.c:1929 `qty_n_refs[sqty] += reg_n_refs[sreg];`), and a tie also extends qty_death. So L1 has two realisations, an extra RTL reference or an absorbed pseudo.

- [s17] Block 7 contains six difference pseudos, four product pseudos, kc, kp and an xor result, but only SEVEN quantities appear in the w_n6 QTYDBG table (regs 129,130,131,132,135,137,139) plus eight HI/LO scratch entries, and grep finds no QTYDBG-SUGG line for this block. The four product pseudos are therefore already tied into other quantities by combine_regs, and which quantity absorbs each product has never been read.

- [s17] If the kp product were tied into dx's quantity, dx would have 3 + 2 = 5 references and a death extended to the kp subu (insn 12): priority floor_log2(5)*5/16*10000 = 3125 against dz's 2500, so dx would win the seat with the emission order untouched. That is the only construction found that satisfies L1 arithmetically.

- [s17] 52 new variants measured this session, all at build_insns == target_insns == 202: 24 outer-declaration orders (best 9), 10 kp-difference spellings (six exactly 9, four 32-36), 18 product-naming spellings (eight exactly 9, ten 25-32). Nothing below the banked floor of 2.

## s18 (solver, 2026-09-08) - the s17 frontier read out of the dumps, and two sweeps

Chassis re-measured at dispatch: `sandbox func_8002D780 --disable all` -> 2/202 with
memory/grind/func_8002D780/candidate.c spliced into src/code6cac_b.c (HEAD 220ab314,
-mel -msoft-float).  BK_base and TT_base in this session's sweeps reproduce 2 and 9
exactly, so both chassis are intact.

### The single highest-value unread artefact, read
s17 left the grind on one question: which quantity absorbs each of block 7's four
product pseudos, since the block's QTYDBG table shows only seven pseudo quantities for
eleven pseudos.  The answer is that they are absorbed by NOTHING - they are never
offered to local-alloc.

  * tmp/grind/func_8002D780/s14/w_n6/code6cac_b.lreg, block-7 pseudo census: registers
    129-139, exactly eleven.  129/130/131/132/135/137/139 are "GR_REGS or none"; the
    remaining four - 133, 134, 136, 138 - are "pref LO_REG, else GR_REGS".  Those four
    are the products of the four multiplies (each MIPS integer product is copied out of
    LO, so regclass costs LO_REG cheapest).
  * tools/gcc-2.7.2/local-alloc.c:469-477 is the eligibility gate:
        if (reg_basic_block[i] >= 0 && reg_n_deaths[i] == 1
            && (reg_alternate_class (i) == NO_REGS
                || ! CLASS_LIKELY_SPILLED_P (reg_preferred_class (i))))
          reg_qty[i] = -2;            /* local-alloc will see it */
        else
          reg_qty[i] = -1;            /* global_alloc's problem */
    CLASS_LIKELY_SPILLED_P defaults to `reg_class_size[CLASS] == 1` (local-alloc.c:77),
    and LO_REG is a one-register class, so the product pseudos take the else branch.
  * combine_regs (local-alloc.c:1784) bails on exactly that value:
    `if (reg_qty[sreg] >= -1 ... ) return 0;` (local-alloc.c:1904).  It also only ever
    ties the insn's SET DESTINATION into a dying INPUT operand's quantity
    (block_alloc's operand scan, local-alloc.c:1241-1296, calls
    combine_regs (r1 = input, r0 = operand 0)), never an input into a destination's.
  So s17's L1 construction - "tie the kp product into dx's quantity, refs 3 -> 5,
  priority 3125 vs 2500" - cannot happen for any spelling of the multiply.  The seven
  quantities in the block-7 table are the whole table, and dx's qty_n_refs is exactly
  reg_n_refs[dx].

### Sweep A - the block-7 hoist axis (s17 frontier item 2), 22 variants
gen_s18.py builds each of ax/dz/az/dx hoisted alone into the outer centroid block and
into the test-2 arm, on BOTH chassis, plus three pair hoists and an all-four hoist.
Rationale: the same gate at local-alloc.c:471 excludes a pseudo whose live range leaves
its block, so a hoist is the ordinary-C way to take a difference OUT of block 7's
quantity table.  Results (tmp/grind/func_8002D780/s18/v18.json): baseline 2, BK_base 2,
TT_base 9, then TT_outer_dz 19, BK_outer_dz 20, four forms at 21, TT_t2_ax 26,
TT_outer_ax 27, the dz+ax pairs 32, every az/dx hoist 35-42 (several at 203 insns),
TT_outer_all 41.  Nothing within 17 of the floor.

### Sweep B - the query differences and source-order interposition, 20 variants
gen_s18b.py.  Two axes.  (1) s17 frontier item 2's explicit remaining probe: hoist
qx = px - x0 and qz = pz - z0 out of block 7 - 21 (BK qx), 29-30 (qz), 35 (TT qx), 37
(both), all at 200-201 insns.  (2) L2 by interposition: write qx and/or qz as named
locals BETWEEN dz and dx inside block 7, so their subu sits between the two target
subus in source order and lengthens dz's local-alloc span alone.  Every interposed and
top-of-block form is byte-identical to its chassis baseline (BK 2, TT 9): sched1
re-sinks each query difference to immediately before its consuming multiply no matter
where it is written, exactly as s17 measured for the kp differences.

### The (T,T) chassis, characterised properly for the first time
tools/pairdiff.py on TT_base (tmp/grind/func_8002D780/s18/pairdiff_TT_base.txt):
14 differing instructions and NOT ONE of them is a transposition.  ours[93]
`subu v1,t0,a3` vs target `subu a0,t0,a3` (dz); ours[96] `subu a0,t5,t1` vs target
`subu v1,t5,t1` (dx); the swap then rides through both mflos, two multiplies and the
kp subtraction (`subu v1,v1,t1` vs `subu v1,a0,v1`).  So on the (T,T) chassis the
emission order is already the target's and the ONLY defect is that dz takes $v1 and dx
takes $a0 where the target has dz = $a0 and dx = $v1.  find_free_reg gives the first
quantity in qty_order the lowest free general register, so the target requires dx's
quantity to sort strictly ahead of dz's - one comparison, one direction.

### What the arithmetic now allows, after s18
dz and dx tie at 2500 in (T,T) and the quantity-number fallback (local-alloc.c:1719)
seats dz.  L1-by-product-tie is closed above; L1-by-extra-reference needs a genuine
third C-level use of `x2 - x0` inside block 7; L2-by-source-interposition and
L3-by-naming/scoping are measured dead.  ONE lever named by the source has never been
touched: qty_compare_1 is not the only comparator.  block_alloc allocates SUGGESTED
quantities FIRST, in a separate loop ordered by qty_sugg_compare
(local-alloc.c:1494-1527), before it ever sorts the rest by length of life.  Block 7
currently has NO suggested quantity at all (no QTYDBG-SUGG line for blk=7 in
w_n6/stderr_full.txt).  A suggestion is created by combine_regs when one side of a tie
is a HARD register (local-alloc.c:1855-1900) - a copy between the pseudo and an
argument, return-value or otherwise fixed register.  If dx's quantity acquired a
suggestion and dz's did not, dx would be seated in the suggestion pass and the
qty_compare_1 tie would never be reached.

- [s18] Chassis re-measured at dispatch: sandbox func_8002D780 --disable all -> {score 2, target_insns 202, build_insns 202} with memory/grind/func_8002D780/candidate.c spliced into src/code6cac_b.c at HEAD 220ab314 (-mel -msoft-float). Both sweeps' base controls (BK_base 2, TT_base 9) reproduce their chassis exactly.

- [s18] Block 7's complete pseudo census (tmp/grind/func_8002D780/s14/w_n6/code6cac_b.lreg) is eleven pseudos, registers 129-139. Seven are 'GR_REGS or none' (129 ax, 130 dz refs 3, 131 dx refs 3, 132 az, 135 px-x0, 137 pz-z0, 139 the xor/kc chain) and four - 133, 134, 136, 138 - are 'pref LO_REG, else GR_REGS'. Those four are the products of the four multiplies.

- [s18] local-alloc.c:471-477 excludes a pseudo from local-alloc when reg_alternate_class != NO_REGS and CLASS_LIKELY_SPILLED_P(reg_preferred_class) is true; CLASS_LIKELY_SPILLED_P defaults to reg_class_size == 1 (local-alloc.c:77) and LO_REG is a one-register class, so all four product pseudos get reg_qty = -1 and are allocated by global_alloc. That is why block 7's QTYDBG table holds only seven pseudo quantities plus eight HI/LO scratch quantities for eleven pseudos.

- [s18] combine_regs (local-alloc.c:1784) ties the SET DESTINATION into a dying INPUT operand's quantity, never the reverse (block_alloc's operand scan calls combine_regs(r1 = input, r0 = operand 0) at local-alloc.c:1293-1295), and it returns 0 immediately when reg_qty[sreg] >= -1 (local-alloc.c:1904). Both facts independently close the s17 L1 construction.

- [s18] 42 new variants measured this session, all bytes-scored against the oracle target. Sweep A (22 hoist forms, tmp/grind/func_8002D780/s18/v18.json): best non-baseline 19. Sweep B (20 query-difference forms, v18b.json): six interposition forms and two naming controls exactly inert at their chassis baselines, eight hoist forms 21-37. Nothing below the banked floor of 2.

- [s18] The (T,T) chassis pairdiff (tmp/grind/func_8002D780/s18/pairdiff_TT_base.txt) shows 14 differing instructions that are entirely a dz/dx hard-register exchange plus its cascade - the emission order is already the target's.

- [s18] block_alloc allocates SUGGESTED quantities in a separate loop BEFORE it sorts the rest by length of life (local-alloc.c:1494-1527, ordered by qty_sugg_compare at local-alloc.c:1694). Block 7 currently has no suggested quantity at all - there is no QTYDBG-SUGG line for blk=7 in w_n6/stderr_full.txt, and every block-7 SUGGDBG-QTY line reports ncopysugg=0 nsugg=0. This comparator has never been considered by this grind.

## s19 (forensics, 2026-09-08) - the seat is a whole-table allocation, not a two-quantity comparison

**Chassis re-measured at dispatch.** `sandbox func_8002D780 --disable all` -> {"score": 2,
"target_insns": 202, "build_insns": 202} with memory/grind/func_8002D780/candidate.c spliced
into src/code6cac_b.c (HEAD 2c87428f, -mel -msoft-float).  The floor is intact.

**FAKE re-audit (mandated by the brief).** `python3 tools/fake_ablate.py --func func_8002D780
--file code6cac_b --candidate memory/grind/func_8002D780/candidate.c` -> 1 FAKE unit, keep-all
2/202, drop-1 6/202.  The single annotated construct (the same-value re-store of `m`) lives in
the sqrt block and is worth 4 instructions THERE; the block-7 residual is present in both
ablation states, so every block-7 kill in this ledger is FAKE-independent.

### Frontier item 1 (the suggestion pass) is dead, and the matched sibling says why
Both chassis were re-dumped with the instrumented cc1 (tools/gcc-2.7.2/cc1, BB2_SUGG_DEBUG /
BB2_QTY_DEBUG / BB2_RANK_DEBUG / BB2_DBR_DEBUG) - tmp/grind/func_8002D780/s19/w_bk and
.../w_tt, 12,170 stderr lines each, 1,522 SUGGDBG-QTY lines covering every function in the TU.

  * func_8002D780 block 7 carries `ncopysugg=0 nsugg=0` on all seven pseudo quantities, on
    BOTH the banked chassis and the (T,T) chassis.
  * The MATCHED sibling func_8002E6B0 - the same point-in-triangle predicate, COMPLETED-C on
    main - carries `ncopysugg=0 nsugg=0` on EVERY quantity of its three predicate blocks
    (blk=1 qty0 reg117 and qty3 reg120; blk=2 qty0 reg138 and qty4 reg141).  Its only
    suggested quantities are blk=0 qty0-3, regs 72-75, with `copysugg=4,` `5,` `6,` `7,` -
    the four incoming parameters copied out of $a0-$a3 at function entry.  func_8002D320
    (also COMPLETED-C) has no suggested quantity anywhere.
  So the original source did NOT reach the target seats through a hard-register suggestion:
  the matched sibling reaches exactly the same dx-before-dz seat order with nsugg = 0.  In
  this whole TU a copy suggestion is created only where a pseudo is copied out of an incoming
  parameter register in block 0.

### How the matched sibling gets the seat, measured
func_8002E6B0 blk=1: dz = qty0 reg117 birth 2 death 10 refs 3; dx = qty3 reg120 birth 6
death 12 refs 3.  Births 4 apart, DEATHS ONLY 2 APART, so the spans are 8 and 6 and
qty_compare_1 gives dz 3750 against dx 5000 - dx sorts first with no tie to break.  The
deaths are close together because in the sibling the query differences (arg3[0] - arg0[0],
arg3[2] - arg0[2]) are common subexpressions across its three tests and cse hoists them into
block 0, so nothing is interleaved between the two multiplies that kill dz and dx.  In
func_8002D780 the query differences are unique to test 3, sched1 sinks each one to just
before its consuming multiply, and the deaths land 4 apart.

### The complete seat model for block 7 (new; supersedes the "dz vs dx" framing)
Block-7 quantity tables, read off the QTYDBG lines of both dumps (positions are 2 units per
RTL insn):

  banked chassis (ax, dz, az, dx):   ax q0 [2,6] r2 | dz q1 [4,16] r3 | az q4 [8,12] r2 |
                                     dx q5 [10,20] r3 | qx q8 [14,16] | qz q11 [18,20] | q14 [26,28]
    priorities: az/qx/qz/q14 = 10000, ax = 5000, dx = 3000, dz = 2500
    allocation order (ord=0..6): az, qx, qz, q14, ax -> ALL got $v0 (they are pairwise
    disjoint, so find_free_reg reuses reg 2 five times); then dx -> $v1 ($v0 blocked by az);
    then dz -> $a0 ($v0 blocked by ax, $v1 blocked by dx).  TARGET SEATS.

  (T,T) chassis (ax, dz, dx, az):    ax q0 [2,6] r2 | dz q1 [4,16] r3 | dx q4 [8,20] r3 |
                                     az q5 [10,12] r2 | qx q8 [14,16] | qz q11 [18,20] | q14 [26,28]
    priorities: az/qx/qz/q14 = 10000, ax = 5000, dz = dx = 2500 (EXACT TIE)
    allocation order: az, qx, qz, q14, ax -> all $v0; then dz (lower quantity number wins the
    tie at local-alloc.c:1719) -> $v1; then dx -> $a0.  SEATS REVERSED.

So the seat is not decided by a two-quantity comparison: it is decided by the whole table's
qty_order plus find_free_reg's $v0 reuse chain.  $v1 is free when the first of dz/dx is
allocated precisely because all five short quantities are pairwise disjoint and share $v0.
The model predicts the s16 quadrant trichotomy without a new measurement: (F,F) gives
dz [2,16] span 14 -> 2142 and dx [10,20] span 10 -> 3000, dx first, seats CORRECT (s16
recorded "all registers correct, score 4"); (T,F) is the banked chassis; (F,T) is the s14
sibling-transplant body.  Only (T,T) ties.

### The arithmetic condition, stated exactly
Correct seats require span(dx) < span(dz), i.e.
    birth(dx) - birth(dz)  >  death(dx) - death(dz).
In every layout measured in this grind the right-hand side is 4 (dz dies at the third
multiply, dx at the fourth, with one query-difference subu between them).  The target's OWN
final instruction order (asm/funcs/func_8002D780.s:103-121) puts only the first multiply
between the two subus, i.e. a left-hand side of 4 - an exact tie, which local-alloc.c:1719
resolves for dz.  Therefore the instruction order local-alloc saw when the original was
compiled was NOT the original's final instruction order: either a third insn sat between the
two subus (that is the banked chassis, and sched2 does not restore the target order - RANKDBG
val=0, sched.c:2464), or the two deaths were 2 apart rather than 4 because the second query
difference had not yet been sunk between the two multiplies.  The second alternative has
never been probed and is this session's headline frontier item.

- [s19] Chassis re-measured at dispatch: sandbox func_8002D780 --disable all -> {"score": 2, "target_insns": 202, "build_insns": 202}, candidate.c spliced into src/code6cac_b.c at HEAD 2c87428f (-mel -msoft-float).

- [s19] tools/fake_ablate.py on candidate.c: 1 FAKE unit, keep-all 2/202, drop-1 6/202. The FAKE (same-value re-store of `m`) is in the sqrt block; the block-7 az/dx residual is present with and without it, so the block-7 kill ledger is not FAKE-masked.

- [s19] func_8002D780 block 7 has NO suggested quantity on either chassis: all seven pseudo quantities report ncopysugg=0 nsugg=0 (tmp/grind/func_8002D780/s19/w_bk/stderr_full.txt and w_tt/stderr_full.txt, SUGGDBG-QTY lines for blk=7).

- [s19] The MATCHED sibling func_8002E6B0 also has NO suggestion on any predicate-block quantity (blk=1 qty0 reg117 / qty3 reg120, blk=2 qty0 reg138 / qty4 reg141, all ncopysugg=0 nsugg=0). Its only suggested quantities are blk=0 qty0-3 (regs 72-75, copysugg=4,5,6,7) - the four incoming parameters. func_8002D320 has no suggested quantity anywhere in the function. In this TU a copy suggestion exists only where a pseudo is copied out of an incoming parameter hard register in block 0.

- [s19] func_8002E6B0's blk=1 edge differences are dz reg117 birth 2 death 10 refs 3 and dx reg120 birth 6 death 12 refs 3: births 4 apart, deaths 2 apart, spans 8 and 6, so qty_compare_1 gives 3750 vs 5000 and dx sorts first with no tie. The sibling avoids the tie because its query differences are cross-test common subexpressions that cse hoists into block 0, leaving nothing interleaved between the two multiplies that kill dz and dx.

- [s19] Block-7 allocation order, banked chassis (QTYDBG blk=7, w_bk): ord0 az [8,12], ord1 qx [14,16], ord2 qz [18,20], ord3 q14 [26,28], ord4 ax [2,6] - all five got $v0 - then ord5 dx [10,20] got $v1, ord6 dz [4,16] got $a0. Target seats, and they come from find_free_reg's $v0 reuse chain, not from the dz/dx comparison alone.

- [s19] Block-7 allocation order, (T,T) chassis (QTYDBG blk=7, w_tt): ord0 az [10,12], ord1 qx, ord2 qz, ord3 q14, ord4 ax [2,6] - all five got $v0 - then ord5 dz [4,16] got $v1, ord6 dx [8,20] got $a0. dz and dx tie at 2500 and the quantity-number fallback (local-alloc.c:1719) seats dz first.

- [s19] Correct seats require birth(dx) - birth(dz) > death(dx) - death(dz). The right-hand side is 4 in every layout this grind has measured (one query-difference subu sits between the third and fourth multiplies). The target's own final order supplies a left-hand side of exactly 4, so the instruction order local-alloc saw when the original was compiled cannot have been the original's final order.

- [s19] qty_compare_1's tie-break was re-read directly (tools/gcc-2.7.2/local-alloc.c:1680-1685: `tem = pri2 - pri1; if (tem != 0) return tem; return *q1 - *q2;`) and qty_sugg_compare's suggestion-count ordering at local-alloc.c:1694-1719 - s17's reading of both is correct.

- [s19] Chassis re-measured at dispatch: sandbox func_8002D780 --disable all -> {score 2, target_insns 202, build_insns 202} with memory/grind/func_8002D780/candidate.c spliced into src/code6cac_b.c at HEAD 2c87428f (-mel -msoft-float).

- [s19] func_8002D780 block 7 has no suggested quantity on either chassis: all seven pseudo quantities report ncopysugg=0 nsugg=0 in the SUGGDBG-QTY lines for blk=7 (tmp/grind/func_8002D780/s19/w_bk/stderr_full.txt, w_tt/stderr_full.txt).

- [s19] The matched sibling func_8002E6B0 has no suggestion on any predicate-block quantity (blk=1 qty0 reg117 and qty3 reg120; blk=2 qty0 reg138 and qty4 reg141). Its only suggested quantities are blk=0 qty0-3, regs 72-75, copysugg=4,5,6,7 - the four incoming parameters. func_8002D320 has none anywhere.

- [s19] func_8002E6B0 blk=1: dz reg117 birth 2 death 10 refs 3, dx reg120 birth 6 death 12 refs 3 - births 4 apart, deaths only 2 apart, spans 8 and 6, priorities 3750 vs 5000, so dx sorts first with no tie. Its query differences are cross-test common subexpressions cse hoists into block 0, which is why nothing is interleaved between the two multiplies that kill dz and dx.

- [s19] Block-7 allocation order on the banked chassis (QTYDBG blk=7, w_bk): az, qx, qz, tail, ax all got=2 ($v0); dx [10,20] got=3 ($v1); dz [4,16] got=4 ($a0).

- [s19] Block-7 allocation order on the (T,T) chassis (QTYDBG blk=7, w_tt): the same five short quantities all got=2; dz [4,16] got=3; dx [8,20] got=4. dz and dx tie at 2500 and local-alloc.c:1719 seats the lower quantity number.

- [s19] qty_compare_1's tie-break re-read directly at tools/gcc-2.7.2/local-alloc.c:1680-1685 ('tem = pri2 - pri1; if (tem != 0) return tem; return *q1 - *q2;'), and the suggestion pass's separate qty_sugg_compare ordering at local-alloc.c:1494-1527 - s17's and s18's readings of both are correct.

- [s19] tools/fake_ablate.py on candidate.c: 1 FAKE unit, keep-all 2/202, drop-1 6/202; the ablation cost is inside the sqrt block and the block-7 residual is unaffected.

- [s19] src/code6cac_b.c was restored to its HEAD state after the measurements; the working tree carries no source edits from this session.

## s20 (forensics, 2026-09-08) - sched1/sched2 are BACKWARD list schedulers; the first form that reaches BOTH the target's block-7 order and the target's block-7 seats

**Chassis re-measured at dispatch.** `sandbox func_8002D780 --disable all` -> {"score": 2,
"target_insns": 202, "build_insns": 202} with memory/grind/func_8002D780/candidate.c spliced
into src/code6cac_b.c (HEAD 01bd10a2, -mel -msoft-float).  Floor intact at 2.

**Owner directive executed.** The queue's auto-return directive (coupled sibling func_8002E6B0
moved to floor 0) is discharged here: the sibling's block-0 CSE geometry was re-read from the
s19/s20 whole-TU dumps and is the reference the seat model below is calibrated against; the
sibling's own spelling of block 7 was already transplanted in s14
(rejected/s14-sibling-8002e6b0-transplant-slot-wrong-2.c) and is not re-run.

### The instrument s19 left on the table
s19's dbg.py enabled BB2_SUGG_DEBUG / BB2_QTY_DEBUG / BB2_RANK_DEBUG / BB2_DBR_DEBUG but NOT
BB2_SCHED_DEBUG / BB2_PRIO_DEBUG, which are the two hooks that print sched.c's ready list, its
pick order, its function-unit blockage decisions and schedule_select's tie-break.  s20 turned
both on (tmp/grind/func_8002D780/s20/dbg.py; 103,800 stderr lines per chassis).  NOTE for
future sessions: the dump script must run with BB2_CC1=tools/gcc-2.7.2/cc1 - engine
buildconfig's CC1 (tools/gcc-2.7.2/build/cc1) is NOT the instrumented binary and silently
produces the -da dumps with an empty debug stream (13 stderr lines, all front-end diagnostics).

### What the trace says (first trace-level attribution in this grind)
GCC 2.7.2's sched.c schedules each basic block BACKWARDS: block 7's jump (insn 210) is picked
at clock 1 and the final emission order is the REVERSE of the pick order.  The ready list is
sorted by rank_for_schedule (tools/gcc-2.7.2/sched.c:2408-2464) with exactly three terms -
INSN_PRIORITY, then dependence class against last_scheduled_insn, then INSN_LUID - and
schedule_select (sched.c:2660-2745) then QUEUES every ready insn that is blocked on a function
unit and picks the first survivor (potential_hazard, sched.c:1327-1360, returns 0 for every
unit=-1 insn, so best_insn is simply the first non-queued index).

Block 7 pass 1, banked chassis: 14 insns, luid 0..13 =
ax(179) dz(182) az(185) dx(188) M1=dz*ax(191) M2=dx*az(193) kc(195) qx=px-x0(198)
M3=dz*qx(200) qz=pz-z0(202) M4=dx*qz(204) kp(206) xor(208) jump(210).
Pick order 210,208,206,195,204,202,200,198,193,188,185,191,182,179 ->
EMISSION ax,dz,M1,az,dx,M2,qx,M3,qz,M4,kc,kp,xor - exactly the QTYDBG positions
ax[2,6] dz[4,16] az[8,12] dx[10,20] qx[14,16] qz[18,20].

Two decisions in that trace are the whole residual:

  (1) *Why a query difference always sits between the last two multiplies.*  After M4 is
  picked at clock 15 the multiply unit is blocked for 11 cycles (SCHEDDBG SELBLOCK clock=16
  insn=200 unit=1 cost=11), and the ONLY unblocked ready insn is qz(202) - which became ready
  the moment M4, its single successor, was scheduled.  So qz is emitted between M3 and M4.
  The same happens for qx between M2 and M3.  A single-use query difference is therefore
  always emitted immediately before its consuming multiply no matter where it is written;
  this is the mechanism behind s17's eight inert kp-difference spellings and s18's inert
  interposition sweep, now read off the scheduler's own log rather than inferred.

  (2) *Why az is emitted before dx.*  At pass-1 clock 40 (and again at pass-2 clock 47) the
  ready list is [M1(blocked), dx(188), az(185)]: both subus have INSN_PRIORITY 1 and both are
  dependence class 3 against the last-scheduled multiply (RANKDBG last=193 y=188 cls=3 x=185
  cls2=3 val=0), so rank_for_schedule falls through to INSN_LUID and the HIGHER luid sorts
  first.  schedule_select queues the blocked multiply and takes the first survivor - dx - so
  dx is PICKED first and therefore EMITTED last.  The az/dx transposition is thus decided by
  source order and by nothing else, in both scheduling passes, and sched2 cannot repair it
  because it re-derives the same three-term comparison on sched1's output.  Both subus sit at
  dependence depth 1 from the top of the block, so no in-block rewrite can separate them on
  the priority term, and the class term cannot separate two operands of the same multiply.

### The seat/order dilemma, stated exactly
With the target's emission order (ax,dz,M1,dx,az,M2,qx,M3,qz,M4) the pre-split positions are
dz[4,16] and dx[8,20]: both span 12, qty_compare_1 ties at 2500 and local-alloc.c:1719 seats
the lower quantity number (dz) first - the (T,T) chassis, score 9.  With the banked order the
positions are dz[4,16], dx[10,20]: dx spans 10, is seated first, and gets the target's $v1 -
but az is emitted before dx, score 2.  Because M3 must precede M4 and each query difference is
pinned immediately before its own multiply (finding 1 above), no in-block spelling that keeps
the target's multiply order can both put dx at position 8 and keep death(dx) - death(dz)
below 4.

### s20's new result: staging the dx kp product breaks the dilemma at the seat end
`q = (dx * (pz - z0)); kp = (dz * (px - x0)) - q;` on the (T,T) chassis makes the dx multiply
expand FIRST, so it becomes M3 and dz's multiply becomes M4.  Measured QTYDBG blk=7
(tmp/grind/func_8002D780/s20/w_qdx/stderr_full.txt): dx = qty4 reg131 birth 8 death 16 refs 3
ord=5 got=3 ($v1); dz = qty1 reg130 birth 4 death 20 refs 3 ord=6 got=4 ($a0).  Those are the
TARGET'S SEATS, reached with the TARGET'S dx-before-az emission order - the first form in 20
sessions to hold both at once, and it needs no tie-break at all (spans 8 vs 16).  The banked
chassis reaches the seats with the wrong order; the (T,T) chassis reaches the order with the
wrong seats; TT_qdx reaches both.

It scores 32/202 anyway, for two reasons that are now the frontier
(tmp/grind/func_8002D780/s20/pairdiff_TT_qdx.txt, 20 hunks):
  * the two kp multiplies come out in the opposite order to the target (ours qz, M(dx*qz),
    qx, M(dz*qx) against the target's qx, M(dz*qx), qz, M(dx*qz)) - about 6 instructions;
  * the extra `q` pseudo shifts GLOBAL allocation in the centroid block (blocks 5/6): every
    difference in hunks ours[48..86] is a pure rename ($t1<->$t2, $t4<->$t5, $a2<->$a3) of the
    x0/z0/x2/z2/px/pz holders - about 26 instructions.  Block 7's first eight emitted
    instructions already match the target modulo that rename.

### The sweep
12 product-order and staging spellings plus 2 base controls, all at build_insns 202 except one
(tmp/grind/func_8002D780/s20/gen_s20.py, v20.json):
BK_base 2 | TT_base 9 | TT_qdz 9 (staging the DZ product instead - exactly inert, the control
the model predicts) | TT_kpopswap 22 | TT_negboth 24 | BK_qdx 25 | TT_qkc 30 | BK_negboth 32 |
TT_kpsplit_dxfirst 32 | TT_qdx 32 | TT_qdx_decl 32 | TT_kpsplit_dzfirst 36 | TT_qdx_qc 36 (201
insns) | TT_qdx_kcinline 42.  Nothing below the banked floor of 2.

- [s20] Chassis re-measured at dispatch: sandbox func_8002D780 --disable all -> {"score": 2, "target_insns": 202, "build_insns": 202}, candidate.c spliced into src/code6cac_b.c at HEAD 01bd10a2 (-mel -msoft-float).

- [s20] The instrumented cc1 must be selected explicitly (BB2_CC1=tools/gcc-2.7.2/cc1). engine/buildconfig.py's CC1 is tools/gcc-2.7.2/build/cc1, which is NOT instrumented: running s19's dbg.py without BB2_CC1 produced complete -da dumps and a 13-line stderr with no BB2 debug records at all.

- [s20] GCC 2.7.2's sched.c schedules each basic block BACKWARDS (block 7's jump insn 210 is picked at clock 1); the final emission order is the reverse of the pick order. Verified against the QTYDBG positions for both chassis.

- [s20] rank_for_schedule (tools/gcc-2.7.2/sched.c:2408-2464) has exactly three terms: INSN_PRIORITY, dependence class against last_scheduled_insn, INSN_LUID. schedule_select (sched.c:2660-2745) then queues every function-unit-blocked ready insn and picks the first survivor; potential_hazard returns 0 for every unit=-1 insn (sched.c:1327-1360), so best_insn is simply the first non-queued index.

- [s20] Block 7 pass 1 luid map, banked chassis: 0 ax(179) 1 dz(182) 2 az(185) 3 dx(188) 4 M1=dz*ax(191) 5 M2=dx*az(193) 6 kc(195) 7 qx(198) 8 M3=dz*qx(200) 9 qz(202) 10 M4=dx*qz(204) 11 kp(206) 12 xor(208) 13 jump(210). Pick order 210,208,206,195,204,202,200,198,193,188,185,191,182,179.

- [s20] A single-use query difference is emitted immediately before its consuming multiply no matter where it is written: it becomes ready only when that multiply is scheduled, and the multiply unit is blocked for 11 cycles afterwards (SCHEDDBG SELBLOCK clock=16 insn=200 unit=1 cost=11), so it is the only unblocked filler. This is the measured mechanism behind s17's and s18's inert difference-placement sweeps.

- [s20] The az/dx transposition is decided by INSN_LUID in BOTH scheduling passes: at pass-1 clock 40 and pass-2 clock 47 the ready list is [multiply (blocked), dx, az] with both subus at INSN_PRIORITY 1 and dependence class 3 (RANKDBG last=193 y=188 cls=3 x=185 cls2=3 val=0), so the higher luid (dx) sorts first, is picked first and is emitted last.

- [s20] TT_qdx (`q = dx * (pz - z0); kp = dz * (px - x0) - q;` on the (T,T) declaration order) is the first form in this grind to hold the target's block-7 emission order AND the target's block-7 seats simultaneously: QTYDBG blk=7 in tmp/grind/func_8002D780/s20/w_qdx/stderr_full.txt gives dx qty4 birth 8 death 16 ord=5 got=3 ($v1) and dz qty1 birth 4 death 20 ord=6 got=4 ($a0), with no tie to break (spans 8 vs 16).

- [s20] TT_qdx nevertheless scores 32/202: about 6 instructions because the two kp multiplies are emitted in the opposite order to the target, and about 26 because the extra `q` pseudo renames the centroid block's global allocation ($t1<->$t2, $t4<->$t5, $a2<->$a3 on the x0/z0/x2/z2/px/pz holders). See tmp/grind/func_8002D780/s20/pairdiff_TT_qdx.txt.

- [s20] Staging the DZ product instead (`q = dz * (px - x0); kp = q - dx * (pz - z0);`) is exactly inert at the (T,T) baseline of 9, which is the control the seat model predicts: it does not change which multiply expands last.

- [s20] 14 forms measured (tmp/grind/func_8002D780/s20/v20.json): BK_base 2, TT_base 9, TT_qdz 9, TT_kpopswap 22, TT_negboth 24, BK_qdx 25, TT_qkc 30, BK_negboth 32, TT_kpsplit_dxfirst 32, TT_qdx 32, TT_qdx_decl 32, TT_kpsplit_dzfirst 36, TT_qdx_qc 36 (201 insns), TT_qdx_kcinline 42.

- [s20] src/code6cac_b.c was restored to its HEAD state after the measurements; the working tree carries no source edits from this session.

- [s20] Chassis re-measured at dispatch: sandbox func_8002D780 --disable all -> {score 2, target_insns 202, build_insns 202} with candidate.c spliced into src/code6cac_b.c at HEAD 01bd10a2 (-mel -msoft-float).

- [s20] TOOLING FACT for every future dump session: the instrumented cc1 must be selected explicitly with BB2_CC1=tools/gcc-2.7.2/cc1. engine/buildconfig.py's CC1 is tools/gcc-2.7.2/build/cc1, which is NOT instrumented - s19's dbg.py run without BB2_CC1 produced complete -da dumps and a 13-line stderr containing only front-end diagnostics and no BB2 debug records at all.

- [s20] s19 left BB2_SCHED_DEBUG and BB2_PRIO_DEBUG off; they are the hooks that print sched.c's ready list, pick order, function-unit blockage and schedule_select's tie-break, and turning them on is what produced this session's attribution.

- [s20] GCC 2.7.2's sched.c schedules each basic block BACKWARDS: block 7's jump (insn 210) is picked at clock 1 and the emitted order is the reverse of the pick order.

- [s20] Block 7 pass-1 luid map (banked chassis): 0 ax(179), 1 dz(182), 2 az(185), 3 dx(188), 4 M1=dz*ax(191), 5 M2=dx*az(193), 6 kc(195), 7 qx=px-x0(198), 8 M3=dz*qx(200), 9 qz=pz-z0(202), 10 M4=dx*qz(204), 11 kp(206), 12 xor(208), 13 jump(210). Pick order 210,208,206,195,204,202,200,198,193,188,185,191,182,179.

- [s20] rank_for_schedule (tools/gcc-2.7.2/sched.c:2408-2464) has exactly three terms - INSN_PRIORITY, dependence class against last_scheduled_insn, INSN_LUID - and schedule_select (sched.c:2660-2745) queues function-unit-blocked ready insns and picks the first survivor, because potential_hazard (sched.c:1327-1360) returns 0 for every unit=-1 insn.

- [s20] The az/dx transposition is settled by INSN_LUID at pass-1 clock 40 and again at pass-2 clock 47 (ready = [multiply (blocked), dx, az]; both subus INSN_PRIORITY 1; RANKDBG cls=3 cls2=3 val=0; SELBEST insn=188 pos=1).

- [s20] A single-use query difference is always emitted immediately before its consuming multiply: it becomes ready only when that multiply is scheduled, and the multiply unit is blocked for 11 cycles afterwards (SCHEDDBG SELBLOCK clock=16 insn=200 unit=1 cost=11), so it is the only unblocked filler.

- [s20] TT_qdx QTYDBG blk=7 (tmp/grind/func_8002D780/s20/w_qdx/stderr_full.txt): dx qty4 reg131 birth 8 death 16 refs 3 ord=5 got=3 ($v1); dz qty1 reg130 birth 4 death 20 refs 3 ord=6 got=4 ($a0) - the target's seats with the target's dx-before-az emission order, spans 8 vs 16 and no tie.

- [s20] TT_qdx pairdiff (tmp/grind/func_8002D780/s20/pairdiff_TT_qdx.txt, 20 hunks): hunks ours[48..86] are a pure global-alloc rename in the centroid blocks ($t1<->$t2, $t4<->$t5, $a2<->$a3 on the x0/z0/x2/z2/px/pz holders), and hunks ours[92..110] show block 7's first eight instructions matching the target modulo that rename, with the two kp multiplies emitted in the opposite order.

- [s20] Sweep results (tmp/grind/func_8002D780/s20/v20.json, all 202 insns except TT_qdx_qc at 201): BK_base 2, TT_base 9, TT_qdz 9, TT_kpopswap 22, TT_negboth 24, BK_qdx 25, TT_qkc 30, BK_negboth 32, TT_kpsplit_dxfirst 32, TT_qdx 32, TT_qdx_decl 32, TT_kpsplit_dzfirst 36, TT_qdx_qc 36, TT_qdx_kcinline 42.

- [s20] The queue's owner directive (auto-return after coupled sibling func_8002E6B0 reached floor 0) is discharged: the sibling's block-0 CSE geometry is the calibration reference for the seat model recorded here, and its block-7 spelling was already transplanted in s14 (rejected/s14-sibling-8002e6b0-transplant-slot-wrong-2.c).

- [s20] src/code6cac_b.c was restored to its HEAD state after the measurements; the working tree carries no source edits from this session.

## s21 (2026-09-08, forensics) — the centroid rename is a global_alloc allocno-ORDER shift driven by allocno_live_length, and it is the mirror image of the block-7 seat fix

Chassis re-measured this session: `BK_base` (memory/grind/func_8002D780/candidate.c, unchanged
body) = **2/202**, `TT_base` = 9/202, `FF_base` = 4/202 — the s16 quadrant table reproduces
byte-for-byte, so every conclusion below is on the same chassis the ledger has been using.

OWNER DIRECTIVE (auto-return, coupled sibling func_8002E6B0 -> floor 0) — ACKNOWLEDGED AND
ALREADY SPENT. The sibling's matched body is src/code6cac_b.c:1332-1364; its test-3 spelling
(`s32 dz = ...; s32 dx = ...;` with all four differences inline) was transplanted onto this
chassis in s14 and scores 2/202 with the residual moved to the reorg delay-slot pair
(tmp/grind/func_8002D780/s14/pairdiff_a_sibling_dz_dx.txt); its allocation data was read again
in s19 (no suggestions anywhere). Nothing in the sibling's ledger has moved since. The directive
is recorded here so the audit stops flagging it.

### 1. The no-new-pseudo carrier sweep (13 bodies, tmp/grind/func_8002D780/s21/v21.json)

s20's frontier said the ~26 instructions of TT_qdx's 32 were caused by the EXTRA PSEUDO `q`
shifting global_alloc's allocno order, and proposed reusing an existing block-7 local as the
product carrier to remove it. Measured:

| body | score | insns | what it is |
|---|---|---|---|
| BK_base | 2 | 202 | banked chassis (control) |
| FF_base | 4 | 202 | (F,F) quadrant (control) |
| TT_base | 9 | 202 | (T,T) quadrant (control) |
| TT_axcarrier | 32 | 202 | `ax = dx * (pz - z0); kp = (dz * (px - x0)) - ax;` |
| BK_kpself / FF_kpself / TT_kpself | 36 | 202 | `kp = dx * (pz - z0); kp = (dz * (px - x0)) - kp;` |
| TT_kpself_dz | 36 | 202 | same split, dz product staged (control) |
| BK_azcarrier / TT_azcarrier | 37 | 202 | `az` borrowed as the carrier |
| TT_kpself_opswap | 37 | 202 | operands of the staged product swapped |
| TT_kcself | 38 | 202 | the same split applied to kc |
| TT_kcself_kpself | 42 | 202 | both |

`kp = dx * (pz - z0); kp = (dz * (px - x0)) - kp;` is the same-variable split-init accumulation
that s20's `TT_kpsplit_dxfirst` did NOT measure (that body carried an extra negation). It is
measured here for the first time and is 36 on all three quadrants — the split erases the
quadrant distinction entirely.

`TT_axcarrier` is the decisive body. Instrumented dump tmp/grind/func_8002D780/s21/w_ax
(BB2_CC1=tools/gcc-2.7.2/cc1, all six BB2 debug hooks, 103,791 stderr lines):
`QTYDBG blk=7 ord=4 qty=3 reg1=131 birth=8 death=16 refs=3 got=3` (dx -> $v1) and
`ord=5 qty=0 reg1=130 birth=4 death=20 refs=3 got=4` (dz -> $a0) — **exactly TT_qdx's
block-7 quantity table and exactly the target's block-7 seats, reached with no fresh `q`
pseudo**. And its pairdiff (tmp/grind/func_8002D780/s21/pairdiff_TT_axcarrier.txt) still shows
the full centroid rename: ours `lw t3,168(s0) / lw t4,184(s0)` vs target `lw t1,168(s0) /
lw t5,184(s0)`, and 15 further single-register replacements through blocks 5 and 6.

**So s20's attribution was wrong: the rename is not caused by the extra allocno.**

### 2. What actually moves — measured, not inferred

`;; N regs to allocate:` in the .greg dump is global_alloc's post-qsort allocation order
(global.c:575 -> dump_conflicts). Comparing the three dumps:

    w_bk  : ... 125 126 110 142 103 122 124 136 104 101 105 134 115 116 102 107 121 ...
    w_tt  : ... 125 126 142 110 103 122 124 136 104 101 105 134 115 116 102 107 121 ...
    w_qdx : ... 125 126 103 142 110 122 124 133 104 105 101 116 135 102 107 115 121 ...
    w_ax  : ... 125 126 134 103 141 122 124 110 104 105 116 101 102 107 115 121 ...

The centroid group (101 102 103 104 105 110 115 116) permutes, and 115/116 change places.

The input that moved is `allocno_live_length` and nothing else. From the `.lreg` dumps'
func_8002D780 section (`Register N used R times across L insns`), n_refs identical everywhere:

| pseudo | refs | w_bk | w_tt | w_qdx | w_ax |
|---|---|---|---|---|---|
| 101 | 7 | 38 | 38 | 40 | 40 |
| 102 | 5 | 35 | 34 | 34 | 34 |
| 103 | 7 | 35 | 35 | 33 | 33 |
| 110 | 4 | 18 | 19 | 19 | 21 |
| 115 | 4 | **26** | **26** | 28 | 28 |
| 116 | 4 | **26** | **26** | 24 | 22 |

`allocno_compare` (tools/gcc-2.7.2/global.c:635-655) is
`floor_log2(n_refs) * n_refs / live_length * 10000 * size`, with `return *v1 - *v2` (the
ALLOCNO NUMBER) as its only tie-break. Registers 115 and 116 are the two query holders; they
are **exactly TIED at 26/26 on both the banked chassis and the (T,T) chassis**, so the tie-break
seats them in allocno-number order and the centroid allocation comes out as the target's. Every
spelling that fixes the block-7 seat moves the two kp multiplies, 115 and 116 die at those two
multiplies, and the tie breaks 28-vs-24 (fresh carrier) or 28-vs-22 (borrowed carrier) — in the
direction that swaps them.

This is the first time the 26-instruction half of the TT_qdx residual has been attributed to a
named pass and a named comparator input.

### 3. The birth-side compensation is inert (104 further bodies)

`live_length = death - birth`, so if the deaths cannot move back, the births can in principle
move forward by the same amount. Two exhaustive sweeps say they cannot:

* **48 bodies** — all 24 orders of the four coordinate declarations (`x0 x2 z0 z2`) on BOTH
  carriers (tmp/grind/func_8002D780/s21/v21b.json). QDX: 12 at 32, 12 at 33. AXC: 12 at 32,
  12 at 34. The only bit that moves anything at all is z0-before-z2 vs z2-before-z0, worth 1-2
  instructions, and nothing goes below 32.
* **56 bodies** — every placement of the `px` and `pz` declarations in the outer declaration
  list, independently, in both relative orders, on the TT_qdx chassis
  (tmp/grind/func_8002D780/s21/v21c.json). **All 56 score exactly 32**, i.e. the axis is
  BYTE-inert: the loads' births are set by where sched1 places them, not by where the
  declaration is written, which is the same result s15 measured for declaration SCOPE.

### 4. Where that leaves the residual

Block 7's fourth-multiply position is now known to be read by TWO independent comparators that
want opposite things, and it is the only remaining degree of freedom:

* `qty_compare_1` (local-alloc.c:1680-1685) needs `span(dx) < span(dz)`, which needs dx's kp
  product expanded FIRST — i.e. the two kp multiplies emitted in the reverse of the target's
  order;
* `allocno_compare` (global.c:635-655) needs 115 and 116 to stay tied at live_length 26, which
  needs the two kp multiplies in the TARGET's order, because those two multiplies are where the
  two query holders die.

Artifacts: tmp/grind/func_8002D780/s21/{gen_s21.py, gen_s21b.py, gen_s21c.py, lenspan.py,
v21.json, v21b.json, v21c.json, pairdiff_TT_axcarrier.txt, w_ax/}.

- [s21] Chassis re-measured this session: BK_base (the banked candidate body, unchanged) 2/202, FF_base 4/202, TT_base 9/202 - the s16 quadrant table reproduces byte-for-byte on HEAD 722aa906.

- [s21] The owner directive on this queue item (auto-return after coupled sibling func_8002E6B0 reached floor 0) is acknowledged and already spent: the sibling's matched test-3 spelling was transplanted in s14 (2/202, residual moved to the reorg delay-slot pair) and its allocation data re-read in s19; nothing in its ledger has moved since. Recorded in evidence.md so the audit stops flagging it.

- [s21] priority() in tools/gcc-2.7.2/sched.c:1433-1520 sets max_priority = 1 for an insn with no in-block LOG_LINKS, and MIPS ADJUST_COST (tools/gcc-2.7.2/config/mips/mips.h:2946-2948) zeroes the cost of every anti/output dependence - so the az/dx INSN_PRIORITY tie recorded in s20 cannot be broken by an in-block rewrite that leaves both subus reading block-live-in registers.

- [s21] rank_for_schedule's class term (sched.c:2427-2444) classifies both az and dx as class 3 against the multiply they both feed, because insn_cost of a subu->mult data dependence is 1; a class separation would require one of the two values to be produced by a load, which the target's own bytes forbid.

- [s21] TT_axcarrier is the first no-new-pseudo body to reach the target's block-7 seats: QTYDBG blk=7 dx reg131 [8,16] refs 3 got=3, dz reg130 [4,20] refs 3 got=4, identical to the s20 TT_qdx landmark.

- [s21] global_alloc's post-qsort allocno order for func_8002D780 differs between chassis: w_bk '... 125 126 110 142 103 122 124 136 104 101 105 134 115 116 102 107 121 ...' vs w_qdx '... 125 126 103 142 110 122 124 133 104 105 101 116 135 102 107 115 121 ...'.

- [s21] Per-pseudo n_refs is identical on every chassis; only live_length moves (101 38->40, 102 35->34, 103 35->33, 110 18->19->21, 115 26->28, 116 26->24->22), and allocno_compare divides by exactly that quantity.

- [s21] All 56 px/pz declaration placements on the TT_qdx chassis score exactly 32, confirming at the score level that a load's birth is set by sched1's placement and not by the C declaration position.
