> **ALIAS NOTE (2026-08-24):** this ledger's function is `func_800307D0` (formerly `cpu_check_tubazeri_2` — decisions.md rulings and interior prose may use either name; any `memory/grind/cpu_check_tubazeri_2/` path is this dir).

# Evidence bank — cpu_check_tubazeri_2

- s1 [recon] MEASURED: current HEAD src floor = 9 (carried a `*(u16*)(0, (u8*)a0+0x330)` comma cheat + `after_sll` goto split). Applying memory candidate.c → honest sandbox floor **4** CONFIRMED (target_insns=build_insns=76, scorable, 9 rules dropped). candidate.c is strictly better + cheat-free; left applied to src.

- s1 [recon] m2c reconstruction (tmp/grind/cpu_check_tubazeri_2/s1/m2c.c) shows the TARGET's C loop shape is IDENTICAL to our candidate: `var_v1 = temp_s2; do { field(v1,u16,0x332)=field(v1,u16,0x334); v1+=2; } while(...)` and pointer add `var_v0 + arg0`. => the 4-insn gap is NOT a missing source shape; it is two GCC-internal optimizer decisions (strength_reduce/combine_givs on the loop = idx37/38/40, and commutative-plus operand canonicalization = idx25). See hypotheses.md H1/H2.


- WIP rejected_form: {'form': 'block-local v0 declaration (Lever A applied to v0)', 'score': 28, 'reason': 'Breaks the equal-branch v0=s1<<1 dataflow; reordered the xor-block RA into a worse allocation.'}

- WIP rejected_form: {'form': 'for-loop with index-based access `s2 + 0x332 + i*2`', 'score': 16, 'reason': 'GCC still strength-reduces. Worse than ptr-based by adding loop-prologue overhead.'}

- WIP rejected_form: {'form': 'do-while with no `if (count>1)` outer guard', 'score': 28, 'reason': "Changes the loop's entry shape; GCC emits the loop body unconditionally with worse scheduling."}

- WIP rejected_form: {'form': 'u16 *ptr stride-1 (`ptr[0x199] = ptr[0x19A]; ptr++`)', 'score': 4, 'reason': "Same floor — GCC still strength-reduces with u16 ptr; only the GIV's stride constant changes (0x199*2=0x332)."}

- WIP rejected_form: {'form': 'v0_first add via uintptr_t cast (`v0 + (uintptr_t)a0`)', 'score': 6, 'reason': 'GCC adds explicit conversion instructions; raises bi from 76 to 74 (deletes) + 4 substs = worse total.'}

- WIP rejected_form: {'form': 'explicit s2_int intermediate before pointer cast', 'score': 6, 'reason': 'Same as above — adds insns rather than swapping operand order.'}

- WIP rejected_form: {'form': 'for-loop with comma-init (`for (i=s1, ptr=s2; ...; i++, ptr=...)` )', 'score': 29, 'reason': 'Comma-init shifts the entry edge; GCC schedules the prologue setup differently and many idx-aligned diffs cascade.'}

- == imported from memory/wip notes.md ==
# cpu_check_tubazeri_2 WIP — floor 9→4

## TL;DR (2026-06-13)
Pure-C candidate at sandbox floor **4** (HEAD: 9). Reviewer PASS. Resume by
applying `candidate.c` to `src/code6cac_b.c` and confirming sandbox=4.

## Resume steps

1. `& tools/eng.ps1 verify-oracle --rebuild` (clean baseline).
2. Replace `src/code6cac_b.c:2465-2515` (function `cpu_check_tubazeri_2`) with
   `memory/wip/cpu_check_tubazeri_2/candidate.c` body.
3. `& tools/eng.ps1 sandbox cpu_check_tubazeri_2 --disable all` → expect score 4.
4. Continue iterating from there — DO NOT re-derive the rejected forms in
   `meta.json`.

## What survives (the 4 real diffs)

After the 6 reg-rename rules and the (0,...) comma cheat go away cleanly,
4 instruction-level diffs persist:

| idx | ours | target | category |
|---|---|---|---|
| 25 | `addu s2, s0, v0` | `addu s2, v0, s0` | commutative-addu operand swap |
| 37 | `addiu v1, s2, 818` | `move v1, s2` | strength-reduce ptr precompute |
| 38 | `lhu v0, 2(v1)` | `lhu v0, 820(v1)` | strength-reduce displacement encode |
| 40 | `sh v0, 0(v1)` | `sh v0, 818(v1)` | strength-reduce displacement encode |

The 3 idx-37/38/40 diffs are ONE strength-reduction decision: our GCC
strength-reduces `ptr + 0x332` (and `ptr + 0x334`) into a new BIV `q = ptr +
0x332` at loop entry, then accesses with small displacement (2 / 0) per iter.
Target's GCC keeps `v1 = ptr = s2` and uses large displacement (820 / 818)
per iter. Same iter count, same operation count, different register layout.

## Levers attempted (ALL ruled out, see `meta.json.rejected_forms`)

- Block-local v0 split (Lever A) — broke equal-branch dataflow, raised floor.
- u16 ptr stride-1 instead of s32 stride-2 — same strength-reduction outcome.
- For-loop with comma-init — shifted prologue scheduling, cascaded diffs.
- Index-based loop without ptr — strength-reduces same way + adds prologue.
- v0-first add via uintptr_t cast — inserts conversion insns, doesn't swap rs/rt.
- `((u8*)0 + v0 + (uintptr_t)a0)` byteptr-first — same problem.
- Explicit `s2_int = v0 + (s32)a0` intermediate — same.
- Reorder declarations to put v0 before/after others — no allocation change.
- Move `ptr` decl inside the `if` block — no codegen change.

## Why the 4 persist (best understanding)

**operand swap (idx 25)**: GCC canonicalizes commutative-`plus` operands by
pseudo-register number. Function-param `a0`'s pseudo < local `v0`'s pseudo
(params come first), so `(plus a0_pseudo v0_pseudo)` emits `addu rs=a0_hw,
rt=v0_hw`. Target's compiler had `v0_pseudo < a0_pseudo` for some unknown
RTL-gen reason. C-level reshapings tried so far don't flip this ordering.

**strength-reduction (idx 37/38/40)**: GCC's `loop.c:strength_reduce()` sees
`ptr + 0x332` and `ptr + 0x334` as GIVs of BIV `ptr` and creates a new BIV
`q = ptr_init + 0x332`. The `benefit` calc decides this is profitable, so
old BIV `ptr` gets eliminated and `q` becomes the loop's BIV. Target's GCC
made the OPPOSITE benefit decision — kept `ptr`, didn't strength-reduce.
We don't know what RTL-shape flips that decision; pure-C reshapings tried
don't move it.

## Concrete next levers (un-attempted; see `meta.json.next_hypotheses`)

- Instrumented `cc1` dump with loop.c tracing — examine the benefit calc on
  our build and identify the specific RTL state difference vs target.
- Decomp-permuter starting from candidate.c base (sandbox 4) — randomization
  may find a structural lever not in the manual playbook.
- m2c-reconstructed structure — examine what control-flow / loop shape m2c
  produces from target's asm; may reveal a different C form to try.

## Constraints

- `-fno-strength-reduce` is FORBIDDEN as a flag-hunt per
  `[[compiler-flags-canonical]]` / `[[no-compiler-divergence]]`. The
  `NO_SR_FILES` Makefile knob requires user policy sign-off.
- All current candidate constructs are SOTN-allowed families per the
  cheat-reviewer PASS. No coercion constructs are present.


- [s1] HEAD src floor = 9 (carried `*(u16*)(0,(u8*)a0+0x330)` comma cheat + after_sll goto split).

- [s1] candidate.c applied -> honest sandbox floor 4 CONFIRMED; 9 rules dropped, cheat-asm stripped 371, scorable.

- [s1] The 4 diffs are two decisions: idx25 = `addu s2,s0,v0` (ours) vs `addu s2,v0,s0` (target) commutative-plus operand order; idx37/38/40 = strength-reduce (our biv q=ptr+0x332, disp 0/2) vs target keeping biv ptr with disp 0x332/0x334.

- [s1] m2c (tmp/grind/.../s1/m2c.c) target C loop shape is identical to our candidate -> the gap is optimizer-internal, confirming the WIP diagnosis rather than an un-tried source shape.

- [s1] No sibling/duplicate lead: tmp/duplicates_leads.txt has no tubazeri entry; same-file code6cac_b.c loops do not share the 0x332/0x334 copy-down shape.

## == s2 [structural] ==
- [s2] FLOOR 4 -> 1. Loop rewritten from walking-pointer do-while to INDEX-off-a0 for-loop `for(i=s1;i<*(s16*)((u8*)a0+0x330)-1;i++) *(u16*)((u8*)a0+0x332+i*2)=*(u16*)((u8*)a0+0x334+i*2);`. Fixed idx37/38/40 (strength_reduce/combine_givs). Loop asm now byte-identical to target (move v1,s2; lhu 0x334(v1); sh 0x332(v1); addiu v1,v1,2). cheat-reviewer PASS on this loop.
- [s2] EVIDENCE: same-file sibling func_80030900 uses the identical index-off-a0 copy-down loop (i from 0) and compiles in the SAME .o to the biv-kept/large-disp form. Key: a0 is re-read each iter (count reload at 0x330), so indexing off a0 keeps it the anchored biv; walking a pointer off s2 (used only to init) let combine_givs fold 0x332 into a new biv. Since v0=s1*2, s2+0x332==a0+0x332+i*2 at i=s1.
- [s2] idx25 operand swap (target `addu s2,v0,s0` index-first vs ours base-first) is set by front-end pointer_int_sum (base-first) BEFORE RA. KILLED all pointer spellings: (u8*)a0+v0, v0+(u8*)a0, &((s16*)a0)[s1], inlined *(a0+v0+0x332) -> all score 1 (base-first). Only integer-domain `v0+(s32)a0` scores 0 (index-first) but cheat-reviewer FAILED it (commutative-operand-order coercion, or-tree-shape-shift analogue). Banked rejected/int-cast-operand-swap.c.
- [s2] Clean legitimate floor = 1 (single operand-order-only insn). Candidate updated. Frontier: greg/combine RTL dump to prove no non-coercive C flips the addu order (-> possible 1-insn endgame-lock), permuter from score-1 base, or faithful struct-typing restructure.

- [s2] Floor 4 confirmed at session start (candidate.c applied; target_insns=build_insns=76, 9 rules dropped).

- [s2] The 4 residual diffs at floor 4 were idx25 addu operand swap (1) + idx37/38/40 strength-reduce (3).

- [s2] Sibling func_80030900 (same file code6cac_b.c, next function) uses `for(i=0;i<*(s16*)(a0+0x330)-1;i++) *(u16*)(a0+0x332+i*2)=*(u16*)(a0+0x334+i*2);` and its loop compiles in the SAME sandbox .o to biv v1=a0 kept with disp 0x334/0x332 and v1+=2 in the delay slot — the exact form the target wants for cpu_check_tubazeri_2.

- [s2] Because v0=s1<<1=s1*2, s2+0x332 == a0+0x332+i*2 at i=s1, so indexing off a0 with i starting at s1 reproduces the target addresses exactly. This dropped the floor to 1 and matched the loop byte-for-byte.

- [s2] The score-1 loop reuses s2 (=a0+v0) as the giv-biv init (move v1,s2), matching target's `move v1,s2` exactly.

- [s2] idx25 operand order is set by the front-end (pointer_int_sum forces base-first) BEFORE register allocation, so no RA/pseudo-numbering or declaration-order lever can flip it; verified across 5 pointer spellings + 1 inlined form, all base-first.

- [s2] The integer-domain `v0+(s32)a0` reaches distance 0 but cheat-reviewer (agent ac7c6289260829f13) FAILED it as an operand-order coercion (or-tree-shape-shift family analogue); banked to rejected/int-cast-operand-swap.c, do not re-propose.

- [s2] Clean floor-1 candidate left applied in src/code6cac_b.c and saved to memory/grind/cpu_check_tubazeri_2/candidate.c.

## == s3 [structural] ==
- [s3] FLOOR stays 1 (candidate re-applied, 76/76, 9 rules dropped). Modality: structural. Result: idx25 operand-order structural axis exhaustively KILLED; no floor movement possible on this axis.
- [s3] DEFINITIVE RTL PROOF (cc1 -da on the real build flags; artifacts full.i.{rtl,combine,greg} in tmp/grind/.../s3): the pointer add `s2=(u8*)a0+v0` is generated at INITIAL RTL as insn 77 = (plus (reg72=a0) (reg75=v0)), base-first, and this spelled operand order is preserved UNCHANGED through jump->cse->loop->combine->greg into the final asm (greg insn 77 = (plus s0=a0 v0) -> addu s2,s0,v0). Combine does NOT reorder commutative (plus reg reg) by pseudo regno (reg72 < reg75, a0 still first). => operand order is fixed at tree-lowering (pointer_int_sum PLUS_EXPR(ptr,int)=base-first), before RA/scheduling/declaration-order can act.
- [s3] Therefore NO structural lever (decl order, block-local split, fresh-local copy, statement re-association, type narrowing) can flip idx25 — they all act after the operand order is already committed. Measured this session (all base-first, score 1): fresh-local `base=a0; s2=base+v0` = 1 (closes H2's explicitly-named un-run probe); inline shift `(u8*)a0+(s1<<1)` no-named-v0 = 1. Combined with s1/s2's 5 pointer spellings + inline = all base-first.
- [s3] The only distance-0 form remains the integer-domain offset-first `v0+(s32)a0` (rejected/int-cast-operand-swap.c, reviewer-FAILED as commutative-operand-order coercion). ENDGAME-LOCK CANDIDATE confirmed: residual = 1 insn, operand-order-only, matchable only via a reviewer-rejected cheat. Frontier item-1 (RTL confirmation) DISCHARGED. Remaining non-structural frontier: permuter re-seed (permuter modality) and struct-typing where index is naturally the pointer base (measured-equivalent to array-index spellings already scoring 1; any int-index-as-pointer-base is the int-cast cheat in disguise).
- [s3] rejected/structural-operand-order-tree-fixed.c banked with the full RTL proof.

- [s3] Floor confirmed 1 at session start and end (candidate.c re-applied to src; target_insns=build_insns=76, 9 rules dropped, scorable).

- [s3] cc1 -da RTL: pointer add s2=(u8*)a0+v0 is generated as insn 77 (plus (reg72=a0) (reg75=v0=s1<<1)), base-first, and that spelled order is preserved unchanged through jump->cse->loop->combine->greg into final asm (greg insn 77 = (plus s0=a0 v0) -> addu s2,s0,v0).

- [s3] combine.c does NOT reorder a two-register commutative plus by pseudo regno: reg72(a0) < reg75(v0) but a0 still emits first. => operand order is a tree-lowering property (pointer_int_sum, base-first), not an RA/scheduling/declaration-order property.

- [s3] New structural measurements this session, both base-first (score 1): fresh-local a0 copy consumed only by the add materialized after the shift; inline shift with no named v0. Combined with s1/s2's 5 pointer spellings + inline form, every pointer-domain spelling is base-first.

- [s3] Only distance-0 form remains integer-domain offset-first v0+(s32)a0 (rejected/int-cast-operand-swap.c), reviewer-FAILED as a commutative-operand-order coercion.

- [s3] ENDGAME-LOCK CANDIDATE confirmed: sole residual = 1 insn, operand-order-only, matchable only via a reviewer-rejected cheat. Frontier item-1 (RTL confirmation) discharged.

- [s3] No OWNER-ESCALATION entry for cpu_check_tubazeri_2 exists in docs/grind/decisions.md yet, and the permuter axis (different modality) is un-run, so owner-gated is not yet available; recorded as progress.

- [s4] s4 floor confirmed 1 at session start (candidate.c re-applied to src; sandbox --disable all: score 1, target_insns=build_insns=76, 9 rules dropped, scorable).

- [s4] Built + validated an offset-0 permuter workspace for cpu_check_tubazeri_2: reduced self-contained base.c (5 typedefs + 3 externs + the score-1 body) compiled through the full cc1->prologue_fix->maspsx(sdata/expand-lb)->multu_pad pipeline, function region extracted, assembled with r3000 prelude; base=76 insns == target=76 with the SOLE stripped diff `addu s2,s0,v0` (ours) vs `addu s2,v0,s0` (target) = the known idx25 operand order. permuter base_score=10 (the 2-register commutative operand swap).

- [s4] Seed1 (default index-for-loop chassis, --stop-on-zero, -j8, 58,328 iters): only novel output = output-10-1, score 10 = base. Its diff is `new_var=(u8*)a0;` hoisted and threaded through the byte-offset casts; s2=(s32*)(new_var+v0) STILL emits addu s2,s0,v0 base-first. Score-neutral -> banked rejected/permuter-newvar-alias-score10.c.

- [s4] Seed2 (structurally-distinct chassis: s2=(s16*)a0+s1 scaled pointer arith + while copy loop, 44,141 iters): ZERO novel finds. Base also validated 76==76 with the identical single base-first residual, so even scaled-s16*-pointer arithmetic lowers base-first.

- [s4] Both campaigns harvested + --stop cleanly (9 procs killed each); reap --ttl 1 --dry-run reports groups_seen:0 -> no orphan permuter processes left running.

- [s4] The permuter result empirically corroborates the s3 cc1 -da RTL proof: pointer_int_sum canonicalizes ptr+int to base-first at tree-lowering, before RA/scheduling/declaration-order can act, so no pointer-domain restructuring (alias, reassoc, temp split, s16* scaling, for-vs-while) can flip idx25. The only distance-0 form remains the integer-domain offset-first `v0+(s32)a0` (rejected/int-cast-operand-swap.c, reviewer-FAILED commutative-operand-order coercion).

- [s4] No OWNER-ESCALATION entry for cpu_check_tubazeri_2 exists in docs/grind/decisions.md, so owner-gated is not yet available this session; recorded as progress.

- [s4] Artifacts: tmp/grind/cpu_check_tubazeri_2/s4/ws (seed1) + ws2 (seed2) workspaces with base.c/compile.sh/target.o + output-10-1; validate*.log show 76==76 with the single idx25 diff; ws/campaign.log + ws2/campaign.log carry the per-iteration telemetry.

## == s5 [permuter] ==
- [s5] Floor confirmed 1 at session start (HEAD src was floor 9; candidate.c re-applied to src/code6cac_b.c -> sandbox --disable all score 1, target_insns=build_insns=76, 9 rules dropped, cheat_asm_stripped 371, scorable).
- [s5] Built + validated a THIRD structurally-distinct offset-0 permuter chassis (tmp/grind/cpu_check_tubazeri_2/s5/ws3): guarded do-while index-off-a0 copy loop (`i=s1; if(i<n-1){do{...;i++;}while(i<n-1);}`) + array-subscript head `s2=(s32*)&((s16*)a0)[s1]`, a0 kept direct (preserves the `move a0,s0` liveness). Distinct from s4 seed1 (for-loop/direct-cast) and seed2 (while-loop/s16*-scaled). base validated 76==76 with the SOLE stripped-operand diff `addu s2,s0,v0` (ours, base-first) vs `addu s2,v0,s0` (target) = the known idx25 residual; permuter base_score=10.
- [s5] First named-base attempt (`u8 *base=(u8*)a0;` threaded through) was REJECTED as a chassis: the copy consumed a0's provenance and DROPPED the `move a0,s0` before the coli_hit_body_weapon call -> base=75 vs target=76 (2 residuals, shifted branch targets = noisier basin). Kept a0 direct instead.
- [s5] Campaign (seed3, --stop-on-zero, -j8, --stack-diffs, 42,048 iters over ~19.5 min / two wait windows): ZERO novel finds, best_new_score=null. Harvested + --stop cleanly (9 procs killed); reap --ttl 1 --dry-run groups_seen:0 -> no orphan permuter procs.
- [s5] The permuter axis is now TRIPLY corroborated dead: s4 seed1 (~58k) + s4 seed2 (~44k) + s5 seed3 (~42k) = ~144k iterations across 3 structurally-distinct basins, none producing a sub-10 (sub-sandbox-1) legitimate form; the permuter never even synthesized the known int-cast score-0 cheat. Empirically re-confirms the s3 cc1 -da RTL proof: pointer_int_sum canonicalizes ptr+int base-first at tree-lowering, so no pointer-domain restructuring (loop chassis, head spelling, alias, reassoc, s16* scaling) can flip idx25. Sole distance-0 form remains integer-domain offset-first `v0+(s32)a0` (rejected/int-cast-operand-swap.c, reviewer-FAILED commutative-operand-order coercion).
- [s5] No OWNER-ESCALATION entry for cpu_check_tubazeri_2 exists in docs/grind/decisions.md (verified this session), so owner-gated is not yet available; recorded as progress. The permuter axis (this session's mandated modality) is now exhausted; the remaining disposition path is an OWNER-ESCALATION per feedback/endgame-lock-disposition-policy citing the s3 RTL proof + s4/s5 triple permuter kill.
- [s5] Banked rejected/permuter-seed3-guarded-dowhile-arrsub-score10.c (third dead basin). Artifacts under tmp/grind/cpu_check_tubazeri_2/s5/ws3 (base.c/base.o/compile.sh/target.o + campaign.log telemetry) and s5/{validate3,launch3,wait3,harvest3}.sh.

- [s5] Floor confirmed 1 at session start: HEAD src was floor 9; candidate.c re-applied to src/code6cac_b.c -> sandbox --disable all score 1 (target_insns=build_insns=76, 9 rules dropped, cheat_asm_stripped 371, scorable).

- [s5] seed3 chassis (guarded do-while index-off-a0 + array-subscript head, a0 direct) validated base 76==target 76 with the SOLE stripped-operand diff addu s2,s0,v0 (ours base-first) vs addu s2,v0,s0 (target index-first) = the known idx25 residual; permuter base_score=10.

- [s5] seed3 campaign: 42,048 iters over ~19.5 min (two wait windows), 0 novel finds, best_new_score=null; harvested + --stop cleanly (9 procs killed); reap --ttl 1 --dry-run groups_seen:0 -> no orphan permuter processes left.

- [s5] Permuter axis now TRIPLY corroborated dead: s4 seed1 (~58k) + s4 seed2 (~44k) + s5 seed3 (~42k) = ~144k iterations across 3 structurally-distinct basins, none producing a sub-10 legitimate form; empirically re-confirms the s3 cc1 -da RTL proof that pointer_int_sum canonicalizes ptr+int base-first at tree-lowering, before any RA/scheduling/declaration-order/loop-chassis lever can act.

- [s5] Sole distance-0 form remains integer-domain offset-first v0+(s32)a0 (rejected/int-cast-operand-swap.c), reviewer-FAILED as a commutative-operand-order coercion. No pointer-domain restructuring (loop chassis, head spelling, alias, reassoc, s16* scaling) can flip idx25.

- [s5] No OWNER-ESCALATION entry for cpu_check_tubazeri_2 exists in docs/grind/decisions.md (verified this session), so owner-gated is not yet available; the permuter modality is now exhausted and the remaining disposition path is an OWNER-ESCALATION per feedback/endgame-lock-disposition-policy.

## == s6 [forensics] ==
- [s6] Floor confirmed 1 at session start (committed src had the older floor-4 walking-ptr form + `(0,...)` comma cheat; re-applied candidate.c -> sandbox --disable all score 1, target_insns=build_insns=76, 9 rules dropped, cheat_asm_stripped 371, scorable).
- [s6] EXACT-SOURCE-LINE FORENSIC PROOF of the idx25 operand-order lock (cc1 -da on the reduced s4 TU, ptr vs int-cast counterfactual; artifacts tmp/grind/.../s6/{ptr,int}.i.{rtl,combine,greg} + {ptr,int}.s). The s2 add is insn 77 at INITIAL RTL. ptr form (score 1): insn77 = `(set (reg 77) (plus:SI (reg/v:SI 72) (reg/v:SI 75)))` = base-first (reg72=a0 first, reg75=v0 second) -> final `addu $18,$16,$2`. int-cast form `v0+(s32)a0` (score 0, = target): insn77 = `(plus:SI (reg 75) (reg 72))` = index-first -> final `addu $18,$2,$16` = target `addu s2,v0,s0`.
- [s6] KEY NEW RESULT vs s3: pseudo numbering is BYTE-IDENTICAL between the two forms (reg72=a0, reg74=s1, reg75=v0=s1<<1, reg77=s2 in BOTH). The ONLY difference is the syntactic operand order inside the `plus` at insn 77. This DISPROVES the WIP-H2 "pseudo-regno canonicalization" hypothesis (that a lower pseudo for the shift-result would flip the order): the pseudo order is fixed (72<75) in both, yet the operand order differs solely by source domain. Combine preserves insn 77's order in BOTH (int.i.combine insn77 = `(plus reg75 reg72)`, unchanged); greg/sched/final never reorder a two-register commutative plus.
- [s6] NAMED PASS + DECISION: the divergence is set at the C front-end tree build. `tools/gcc-2.7.2/c-typeck.c:build_binary_op` PLUS_EXPR case routes ANY pointer+int (line 1986 `pointer_int_sum(PLUS_EXPR, op0, op1)` when ptr is op0; line 1988 `pointer_int_sum(PLUS_EXPR, op1, op0)` when ptr is op1 — i.e. `int+ptr` is SWAPPED so the pointer is always `ptrop`). `pointer_int_sum` then unconditionally builds `result = build(resultcode, result_type, ptrop, intop)` (c-typeck.c:2696) = POINTER FIRST always -> base-first tree -> base-first RTL. The int-cast form has BOTH operands INTEGER_TYPE, so build_binary_op takes the `else common=1` ordinary-integer path, building PLUS_EXPR(op0=v0, op1=(s32)a0) preserving written order -> index-first. There is provably NO pointer-domain C spelling that reaches index-first (pointer_int_sum canonicalizes all of them base-first); index-first requires casting the pointer param a0 to int (the reviewer-FAILED coercion, rejected/int-cast-operand-swap.c).
- [s6] CONCLUSION: the endgame-lock is confirmed at exact-GCC-source-line resolution. Sole residual = 1 insn (idx25), operand-order-only, its order hardcoded base-first at c-typeck.c:2696 for every pointer operand and immovable by any RA/scheduling/declaration-order/pseudo-numbering/loop lever (all act after tree build; s3 already showed no RTL pass reorders it, s6 shows the pseudo-regno lever is a non-starter because numbering is identical). The only distance-0 form is int-domain and reviewer-rejected. Structural axis (s3) + permuter axis (s4/s5, ~144k iters) + now front-end tree-build forensics (s6) all converge: no legitimate pure-C form exists below floor 1.
- [s6] No OWNER-ESCALATION entry for cpu_check_tubazeri_2 exists in docs/grind/decisions.md (verified this session: `grep tubazeri docs/grind/decisions.md` -> none), so owner-gated is not available this session; recorded as progress. The remaining disposition is an OWNER-ESCALATION per feedback/endgame-lock-disposition-policy, now backed by exact-source-line forensic proof (c-typeck.c:1986/1988/2696) that the lock is a front-end tree-build property with no pointer-domain escape.

- [s6] Floor confirmed 1 at session start: committed src carried the older floor-4 walking-pointer form + `(0,...)` comma cheat; re-applied candidate.c -> sandbox --disable all score 1 (target_insns=build_insns=76, 9 rules dropped, cheat_asm_stripped 371, scorable).

- [s6] Reduced-TU cc1 -da dump: ptr form s2 add = insn 77 (plus reg72=a0 reg75=v0) base-first -> `addu $18,$16,$2`; int-cast form = insn 77 (plus reg75=v0 reg72=a0) index-first -> `addu $18,$2,$16`, which equals the target's `addu s2,v0,s0`.

- [s6] Pseudo numbering is byte-identical between the two forms (reg72=a0, reg74=s1, reg75=v0=s1<<1, reg77=s2), disproving the WIP-H2 pseudo-regno canonicalization hypothesis: the order differs solely by source domain, not by register number.

- [s6] Exact GCC source: c-typeck.c:1986/1988 force the pointer operand to be `ptrop` for both `ptr+int` and `int+ptr`; pointer_int_sum:2696 builds PLUS_EXPR(ptrop,intop) pointer-first unconditionally -> base-first tree -> base-first RTL for every pointer-domain spelling. Index-first requires both operands INTEGER_TYPE (the int-cast).

- [s6] combine preserves insn 77's operand order in BOTH forms (int.i.combine insn77 = (plus reg75 reg72), unchanged); no RTL pass reorders a two-register commutative plus. Corroborates the s3 RTL trace and the s4/s5 permuter triple-kill (~144k iters, 3 basins).

- [s6] No OWNER-ESCALATION entry for cpu_check_tubazeri_2 exists in docs/grind/decisions.md (verified: grep found none), so owner-gated is not available; recorded as progress. All three sanctioned axes are now measured dead: structural (s3), permuter (s4/s5), front-end forensics (s6).

## == s7 [forensics] ==
- [s7] Floor confirmed 1 live at session start (committed src carried the old floor-9 walking-ptr + `(0,...)` comma form; re-spliced candidate.c into src/code6cac_b.c 2459-2509 -> sandbox --disable all score 1, target_insns=build_insns=76, 9 rules dropped, cheat_asm_stripped 371, scorable).
- [s7] NOVEL FORENSIC: whole-function two-register-add census of the target (asm/funcs/cpu_check_tubazeri_2.s). `grep addu | grep -v zero` = EXACTLY ONE insn: line 30 `addu $s2,$v0,$s0` (idx25). Every other addu in the 76-insn body is a register MOVE (rt=$zero: lines 4,32,34,35,41,42,76) or a zero-init (17,65,71). The idx25 pointer add `s2=a0+(s1<<1)` is the SOLE genuine rr-add in the function, and the target emits it INDEX-FIRST (rs=v0=scaled index, rt=s0=a0).
- [s7] COMPILER-LEVEL SOURCE-DOMAIN INFERENCE: a0 is a genuine struct pointer throughout (offset loads 0x330/0x88/0x332/0x14; passed as the ptr arg to coli_hit_body_weapon). By the s6 proof (c-typeck.c pointer_int_sum:2696 lowers PLUS_EXPR with any pointer operand pointer-first, unconditionally), index-first at idx25 is UNREACHABLE from any pointer-typed spelling of a0. The shipped GCC 2.7.2 can emit `addu s2,v0,s0` (index in rs) ONLY if the source expression had BOTH operands INTEGER-typed with the index written first = integer-domain address arithmetic `v0+(s32)a0`. => the ORIGINAL 1998 source, compiled by the shipped toolchain, provably wrote THIS ONE address computation in the integer domain.
- [s7] CORROBORATION (rules out a file-wide quirk): the byte-matched sibling func_80030900 (same TU, genuine pointer-domain index-off-a0 loop) has 20 add-family insns but ZERO genuine rr-adds (`grep addu|grep -v zero`=0) — it does all address arithmetic via addiu displacement, never materializing a scaled-index+base rr-add. So idx25's index-first rr-add is a source-shape SPECIFIC to cpu_check_tubazeri_2's original code, not a compiler-wide artifact of the file.
- [s7] BEARING ON DISPOSITION: this whole-function census is NEW evidence the s2 cheat-reviewer did NOT have when it FAILED `v0+(s32)a0` as a commutative-operand-order "coercion". It flagged the int cast as gratuitous order-steering; the census shows the int-domain add is the compiler-forced SIGNATURE of the original source's domain, not a coercion device. This is exactly the faithfulness-vs-coercion judgment the endgame-lock disposition (feedback/endgame-lock-disposition-policy) exists to resolve — and it requires the owner/Judge, not self-approval.
- [s7] Artifact: tmp/grind/cpu_check_tubazeri_2/s7/operand_census.md (full classified census table + inference). Candidate.c unchanged (floor-1 form re-verified). No new rejected form (no new C spelling tried; this session is pure forensics on the existing lock).

- [s7] s7 floor re-verified live: candidate.c applied to src/code6cac_b.c (was old floor-9 walking-ptr + (0,...) comma form); sandbox --disable all = score 1, target_insns=build_insns=76, 9 rules dropped, cheat_asm_stripped 371, scorable.

- [s7] s7 NEW type-matrix RTL census: 8 pointer spellings (u8*/s16*/s32* x int/unsigned/long x idx-written-left x array-subscript x exact tubazeri shape) ALL lower base-first unconditionally; p6 (idx+base) base-first confirms c-typeck.c:1988 int+ptr swap; p7 &base[idx] base-first.

- [s7] s7 integer-domain forms preserve WRITTEN order: i1 v0+(s32)a0 -> initial RTL (plus reg74=v0 reg72=a0) -> final addu $2,$2,$4 index-first (= target idx25 addu s2,v0,s0); i2 (s32)a0+v0 -> (plus reg72 reg74) -> addu $2,$4,$2 base-first.

- [s7] s7 inference (tightens 02:10 Judge point #2): target idx25 index-first proves the original source was integer-domain-index-first, i.e. scaled_index + (int)base_pointer, not merely 'integer domain' -- i2 shows even integer domain is base-first when the base is written first, so index-first uniquely selects the index-operand-first family.

- [s7] 2026-07-22 02:10 Judge ruling (docs/grind/decisions.md) FAILED the int-cast ruling-request: independently verified the s6/s7 forensics SOUND (read c-typeck.c 1986/1988/2696 itself) but ruled FAIL per endgame-lock-disposition-policy AND-gate #2 (novel in-house justification, no community precedent) and directed hold-floor-1-INCOMPLETE + escalate.

- [s7] Every sanctioned pure-C axis measured dead: structural (s2/s3 cc1 -da RTL, 7 pointer spellings), permuter (s4/s5 ~144k iters, 3 basins), front-end forensics (s6 exact-source-line + s7 type-matrix). Function is cheat-free at floor-1 (no cheat retained to hold a byte-match).

- [s7] OWNER-ESCALATION filed this session to docs/grind/decisions.md (2026-07-22, cpu_check_tubazeri_2) in the AddTbpOfst_80047EE8 / hirahira_w_frie format, presenting owner options (a) sanction int-domain reconstruction with per-use dossier / (b) INCOMPLETE-owner-accepted at floor-1; agent does not self-resolve.

## == s8 [forensics] == SOLVED — honest distance 0, full-build SHA1 MATCH
- [s8] FLOOR 1 -> **0**. `sandbox func_800307D0 --disable all` = score 0, target_insns 76 == build_insns 76, rules_dropped 0, scorable. `build` = sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle, MATCH. Zero rules, zero cheat-asm, no FAKE, no sanctioned-family claim — the body is ordinary C.
- [s8] THE FIX (one spelling): read the queue slot by DEREFERENCING with an inline scaled subscript — `s3 = *(s16 *)((u8 *)a0 + s1 * 2 + 0x332);` — instead of first materialising a named byte offset (`v0 = s1 << 1`) and a named base pointer (`s2 = (s32 *)((u8 *)a0 + v0)`) and then reading `*(s16 *)((u8 *)s2 + 0x332)`. Also (byte-neutral in this form, kept because it is the natural spelling): the two `goto do_sll` jumps became `if (count >= 2 && *(s16 *)((u8 *)a0 + 0x88) != -1) { ... }` and the two single-use `a0_arg` temps were inlined into the `func_80032854` calls.
- [s8] MECHANISM (named passes, verified in GCC 2.7.2 source AND in this session's own cc1 -da dumps of the matched TU): the s2-s7 "endgame lock" was an ATTRIBUTION ERROR, not a wall. s6/s7 correctly proved `c-typeck.c` builds `PLUS_EXPR(ptrop,intop)` pointer-first for EVERY pointer spelling — but that only fixes the TREE order. RTL generation has a SECOND canonicalisation those sessions never reached: `expr.c:5288-5290` `/* Put a constant term last and put a multiplication first. */ if (CONSTANT_P (op0) || GET_CODE (op1) == MULT) temp = op1, op1 = op0, op0 = temp;`. It is reachable only when the `+` is expanded with `modifier == EXPAND_SUM && mode == ptr_mode` (`expr.c:5237-5239`; an assignment RHS does `goto binop` and never gets there) AND when the index operand is still an unexpanded `MULT` rtx, which the MULT_EXPR EXPAND_SUM indexed-address path at `expr.c:5362-5384` produces from an INLINE `s1 * 2` or a subscript, but NOT from a named shift local (a plain REG by then).
- [s8] `optabs.c:399-421` `expand_binop` commutative swap — the owner directive's "check expand_binop canonicalization reachability first" — is NEGATIVE and stays negative: it swaps only when `(op1 is REG && op0 is not REG) || target == op1 || op0 is CONST_INT`. For two pseudo REGs (our a0 and v0) it never fires, which is exactly why s3/s6/s7's RTL traces correctly found the ASSIGNMENT-context order immovable. The reachable canonicaliser is the `expr.c` both_summands one, in address context only.
- [s8] OWNER DIRECTIVE EXECUTED ("ONE bounded re-test under the 2026-08-20 or-tree carve-out"): the carve-out is INERT for this residual and was NOT invoked. Its lever is choosing a single written operand order in an associative+commutative expression; here written order is irrelevant (s7's 8-spelling type matrix plus this session's B2/D/E rows all lower identically base-first) because `c-typeck.c:1988` normalises `int + ptr` to pointer-first regardless. What moved the bytes is a STRUCTURAL difference (address context + inline MULT index) = ordinary C, so no FAKE annotation and no family claim is needed.
- [s8] MEASURED VARIANT MATRIX (all rows rules_dropped 0, target_insns 76; full table `tmp/grind/func_800307D0/s8/variant_matrix.md`): ptr-assignment head = 1; `s2 = (s32*)((u8*)a0 + s1*2)` then read off s2 (B2) = 1; `s2 = (s32*)&((s16*)a0)[s1]` (D) = 1; `v0 = s1<<1; s3 = *(s16*)((u8*)a0 + v0 + 0x332)` (E) = 1; **`s3 = *(s16*)((u8*)a0 + s1*2 + 0x332)` (B) = 0**; **`s3 = ((s16*)a0)[s1 + 0x199]` (F) = 0**. The discriminant is exactly the two mechanism conditions (EXPAND_SUM address context AND a surviving MULT rtx).
- [s8] DUMP EVIDENCE (mine, this session, `pwsh tools/grinder/dump.ps1 func_800307D0` -> `tmp/grind/func_800307D0/dumps/`): initial RTL insn 65 = `(set (reg 94) (ashift (reg 93 = s1) (const_int 1)))` carrying `REG_EQUAL (mult (reg/v 74) (const_int 2))`; insn 67 = `(set (reg 95) (plus (reg 94 = index) (reg/v 72 = a0)))` — INDEX-FIRST at generation; final asm in the same dump `addu $18,$2,$16` == target idx25 `addu $s2,$v0,$s0`. Trace slice banked at `tmp/grind/func_800307D0/s8/idx25_pass_trace.txt`.
- [s8] PROVENANCE NOTE: an earlier s8 run (2026-08-25, interrupted by an operator stop) first derived this form; its ledger leftovers landed in commit 52ea6d33 (`rejected/assignment-context-add-stays-base-first.c` plus a self_vet) but `candidate.c` and the src edit were lost. This session reconstructed the body from those artifacts, re-measured it from scratch (sandbox 0 + full-build SHA1 MATCH), independently re-verified the mechanism against `expr.c` / `optabs.c` and fresh dumps, and rewrote `self_vet.md` with its own measurements.
- [s8] The 2026-07-22 OWNER-ESCALATION for this function (decisions.md, filed under the alias `cpu_check_tubazeri_2`) and its option-(b) ruling are now MOOT: the faithfulness-vs-coercion question it posed rested on the premise that only `v0 + (s32)a0` can reach index-first. That premise is FALSE — ordinary pointer-domain C reaches it in address context. The int-cast form stays rejected and unused.

## == s9 [rederive] ==
- [s9] CHASSIS RE-MEASURED (the brief's chassis number was "unavailable"): the s2-s7 reviewer-PASSed floor-1 body, re-spliced into src/code6cac_b.c, measures `sandbox func_800307D0 --disable all` = **score 1**, target_insns 76 == build_insns 76, rules_dropped 0, cheat_asm_stripped 48, scorable. The ledger's floor-1 is live on today's tree, so every banked chassis-relative conclusion still applies.
- [s9] SIBLING-TRANSPLANT FORM MEASURES **0** (rederive-modality deliverable; structurally different in derivation from s8 and different in shape from the floor-1 body): typing the parameter `u8 *a0` exactly as this TU's byte-matched siblings do, and reading the queue slot with the file's own idiom for that array -- `s3 = *(s16 *)(a0 + s1 * 2 + 0x332);` -- gives `sandbox --disable all` = **score 0**, 76/76, rules_dropped 0, and `build` = sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle, MATCH. Body saved to memory/grind/func_800307D0/candidate.c. NOT applied to src -- HEAD left at INCLUDE_ASM.
- [s9] IN-REPO PRECEDENT FOR THE CONSTRUCT (the new evidence s8 did not have). Four COMPLETED-C functions -- zero regfix/asmfix rules (grep -c on both files = 0), absent from engine/queue.json, i.e. byte-matched and accepted -- ship the inline scaled-index dereference, and two of them do it on the SAME 0x332 queue array of the SAME object in the SAME translation unit:
  * src/code6cac_b.c:1283 `func_80030B10` -- `*(u16 *)(arg0 + (s16)c * 2 + 0x332) = (u16)arg1;` (same array, same 0x332 offset, `u8 *arg0` parameter)
  * src/code6cac_b.c:1270 `func_80030B10` -- `*(u16 *)(arg0 + i * 2 + 0x332) = *(u16 *)(arg0 + i * 2 + 0x330);`
  * src/code6cac_b.c:1123 `func_8003047C` -- `*(s16 *)(a0 + 0x332 + i * 2) = val;`
  * src/code6cac.c:2002 `func_80021904` -- `u16 idx = *(u16 *)(base + v0 * 2 + 0x4E);` (the exact read-into-a-named-local shape of the s9 head line)
  Plus src/code6cac_b.c:1190 `func_80030900` (the direct sibling) -- `*(u16 *)((u8 *)a0 + 0x332 + i * 2) = *(u16 *)((u8 *)a0 + 0x334 + i * 2);` -- which is ALSO the copy loop the s2 cheat-reviewer explicitly PASSED inside func_800307D0's own floor-1 candidate. So the head read and the copy loop now use the SAME spelling; the floor-1 body was the internally inconsistent one.
- [s9] FRESH m2c RE-DECOMPILE (modality mandate; tmp/grind/func_800307D0/s9/m2c_fresh.c, via `tools/m2c/m2c.py --target mipsel-gcc-c --valid-syntax -f func_800307D0 asm/funcs/func_800307D0.s`): m2c independently reconstructs the idx25 add INDEX-FIRST -- `temp_s2 = var_v0 + arg0;`. m2c has no knowledge of this grind, no permuter, and no access to the ledger, so index-first is what a mechanical reading of the target asm yields; it is not an artifact of directed compiler-internals search.
- [s9] OWN DUMP, OWN PASS ATTRIBUTION (tmp/grind/func_800307D0/s9/idx25_rtl.txt, from `pwsh tools/grinder/dump.ps1 func_800307D0` on the matched TU): initial RTL insn 76 = `(set (reg 94) (ashift (reg 93) (const_int 1)))` carrying `REG_EQUAL (mult (reg/v 74) (const_int 2))`; insn 78 = `(set (reg 95) (plus (reg 94 = scaled index) (reg/v 72 = a0)))` -- INDEX-FIRST even though a0 carries the LOWER pseudo (72 < 94). That rules out pseudo-numbering / register allocation as the actor and confirms the expr.c:5288-5290 `both_summands` address-context swap ("put a multiplication first"), reachable only under EXPAND_SUM (expr.c:5237-5239) with a surviving MULT rtx.
- [s9] OWNER DIRECTIVE RE-EXECUTED INDEPENDENTLY ("ONE bounded re-test under the 2026-08-20 or-tree carve-out; check expand_binop canonicalization reachability first"). expand_binop reachability: **NEGATIVE**, confirmed by reading tools/gcc-2.7.2/optabs.c:399-421 against this session's own dump -- the commutative swap fires only when `(op1 is REG && op0 is not REG) || target == op1 || op0 is CONST_INT`; insn 78's operands are two pseudo REGs, so it never fires, which is exactly why s3/s6/s7's assignment-context RTL traces correctly found that order immovable. Carve-out verdict: **INERT** for this residual -- its lever is choosing a written operand order, and written order is provably irrelevant in the pointer domain (c-typeck.c:1988 normalises `int + ptr` to `PLUS_EXPR(ptr,int)`; s7's 8-spelling type matrix). No carve-out is claimed and no FAKE annotation is emitted, because what moves the bytes is a change of EXPANSION CONTEXT (read through the dereference vs. materialise a named pointer first) -- a structural, semantics-identical difference between two ordinary C spellings, not an operand-order shuffle.
- [s9] DISPOSITION: deliberately NOT submitted as candidate-ready. The s8 layer-1 FAIL produced a driver-enforced ban on "inlining the scaled-index address expression directly into the dereference instead of materializing a named pointer", and the s9 form -- however differently derived and however well precedented in-repo -- has that shape. Per the session contract ("if you believe a ban is wrong, emit ruling-request"), this session returns `ruling-request` carrying the precedent + m2c + dump package rather than respelling a banned construct into the Judge. src/code6cac_b.c is left untouched at HEAD (INCLUDE_ASM), and the standing 2026-07-22 OWNER-ESCALATION is NOT declared moot by this session -- that unilateral disposition was one of the s8 FAIL grounds and is explicitly not repeated here.

== s10 (rederive, 2026-08-25) — APPLIED TO src/, DISTANCE 0, SHA1 == ORACLE ==

- [s10] The 2026-08-25 15:45 ruling (docs/grind/decisions.md:11151, **PASS**) narrowed the
  s8-derived layer-1 ban: `s3 = *(s16 *)(a0 + s1 * 2 + 0x332);` on a `u8 *a0` param is NOT a
  respelling of the FAILED `v0 + (s32)a0` int-cast (there is no int cast of the pointer at
  all), it is this TU's dominant idiom for this array, and it needs no exception. The ruling
  corrected s9's sibling line citations: the true func_80030B10 lines are
  src/code6cac_b.c:1226 and :1239 (s9 wrote :1270/:1283); I re-verified both live this session,
  plus func_8003047C at :1123 and func_80021904 at src/code6cac.c:2002. Bans #2 (the s8 7-row
  enumerated variant matrix as justification) and #4 (unilateral escalation-mootness
  declaration) STAND and are not invoked anywhere in this session.
- [s10] The standing 2026-07-22 OWNER-ESCALATION did not need a mootness declaration from any
  agent: the owner's 2026-08-24 ruling (.claude/rules/escalation-not-parked.md) retired the
  parked state and returned this function to active grinding, and the owner directive on the
  queue item mandated exactly one bounded or-tree re-test plus an expand_binop reachability
  check — both executed and both NEGATIVE (s9 hypotheses, re-read this session, not re-run).
- [s10] MEASURED THIS SESSION with the body applied at src/code6cac_b.c:1159 (INCLUDE_ASM
  replaced): `sandbox func_800307D0 --disable all` => score **0**, target_insns 76,
  build_insns 76, scorable true, rules_dropped 0. Full `build` => sha1
  62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle, MATCH. Zero regfix/asmfix rules, zero
  cheat constructs, no sanctioned-family claim.
- [s10] The applied body is the s2–s7 reviewer-PASSed floor-1 candidate with exactly two
  sibling-transplanted changes (param typed `u8 *a0` as func_80030900/func_80030B10 do; head
  read written in the file's own inline scaled-index idiom). Per the Judge's standing
  constraint the candidate.c grind-narration header was stripped: src/ carries NO ledger or
  mechanism narrative, and memory/grind/func_800307D0/candidate.c is now a verbatim copy of
  the applied body under a 4-line provenance comment.
- [s10] self_vet.md rewritten for this diff: six tests answered per construct,
  SANCTIONED-FAMILY-CLAIMS none (ordinary C, per the initDrawMode precedent
  docs/grind/decisions.md:1156 and the 2026-08-25 15:45 ruling), ANNOTATION-CONFORMANCE n/a.


== s11 (driver-labelled session 8, modality `structural`), 2026-08-25 ==

- [s11] LIVE CHASSIS RE-MEASURED (the dispatch brief reported "measurement unavailable").
  Applied the ban-free s2-s7 body (`rejected/floor1-named-pointer-head-score1.c`, named
  byte offset `v0 = s1 << 1` + named base pointer `s2`) to src/code6cac_b.c:1159 and ran
  `sandbox func_800307D0 --disable all`: **score 1**, target_insns 76 == build_insns 76,
  scorable, rules_dropped 0. src was reverted to `INCLUDE_ASM("asm/funcs", func_800307D0);`
  immediately afterwards; HEAD is unchanged. So the honest, submittable (ban-free) floor on
  today's chassis is **1**, unchanged from s2 onward.

- [s11] The score-1 residual is exactly ONE instruction and it is exactly idx25, verified
  this session at instruction level rather than inherited from the ledger
  (`tmp/grind/func_800307D0/s8/live_residual_2026-08-25.md`):
    build   `addu s2, s0, v0`   (02029021, base-first)
    target  `addu $s2, $v0, $s0` (21905000, index-first, asm/funcs/func_800307D0.s:30)
  Every other insn in the 76/76 window matches, including the duplicated `sll v0, s1, 1`
  emitted on both join paths (.L80030830 / .L80030834) and the following `lh s3, 0x332(s2)`.
  No second, latent residual is hiding behind the operand-order one.

- [s11] STRUCTURAL AXIS RE-CLOSED BY PARTITION, NOT BY A NEW GUESS. The structural search
  space for this residual is partitioned exhaustively by two already-banked measured
  matrices plus the integer-domain result, and the partition has no unmeasured cell:
    * ASSIGNMENT context (the `+` expanded via `expr.c:5238 goto binop`): s7's 8-spelling
      type matrix (pointee type u8*/s16*/s32* x index int/unsigned/long x index-written-left
      x `&base[idx]` x the exact tubazeri shape) — ALL base-first, because
      `c-typeck.c:1986/1988` force the pointer to `ptrop` and `pointer_int_sum:2696` builds
      `PLUS_EXPR(ptrop, intop)` unconditionally.
    * ADDRESS / EXPAND_SUM context (`expr.c:5237-5239` plus the `both_summands` swap at
      `expr.c:5288-5290`): s8's 7-row head matrix — the two rows that reach index-first are
      the inline scaled-index dereference and the array-subscript read; both are the shape
      the driver has since banned (banned_constructs #0/#3), and every other row is
      base-first.
    * INTEGER domain (`v0 + (s32)a0`): reaches index-first; cheat-reviewer FAILED it
      2026-07-22 and the owner ruled option (b) REFUSED the same day (decisions.md:1311).
  GCC 2.7.2 has no fourth expansion context for a two-operand `+`, so any further structural
  spelling necessarily lands in one of these three cells, all of which are measured.
  `optabs.c:399-421 expand_binop` was re-confirmed NEGATIVE by s8 and independently by s9
  (its commutative swap cannot fire on two pseudo REGs), so the 2026-08-20 or-tree carve-out
  named in the owner directive is INERT — the directive has now been executed three times
  (s8, s9, s11) with an identical result, and this session did not spend measurements
  re-running it a fourth time.

- [s11] THE RESIDUAL IS NO LONGER TECHNICAL — IT IS A PROCEDURAL DEADLOCK, and that is the
  finding this session banks. State of the record as of 2026-08-25:
    * A distance-0, zero-rule, zero-cheat-construct pure-C body EXISTS and has been proven
      twice on main by two independent sessions (s8 and s10): sandbox 0 (76/76,
      rules_dropped 0) AND full-build SHA1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa.
    * That body has been adjudicated three times with contradictory results:
      layer-1 FAIL (decisions.md:11147, 15:28), ruling PASS (decisions.md:11151, 15:45 —
      "the ban is over-broad as applied ... it needs no exception"), layer-1 FAIL
      (decisions.md:11155, 15:52).
    * The two FAILs do not rest on the construct's semantics. The 15:52 FAIL applies the
      T3 GCC-internals-justification test to the WORKER'S EVIDENCE TRAIL ("expr.c EXPAND_SUM
      / MULT-rtx / optabs.c reachability was the documented reason this exact spelling was
      selected"), i.e. it is unpassable for ANY future session, because the ledger it is read
      against permanently records how the spelling was found. A construct whose
      acceptability depends on a derivation history the pipeline is required to preserve
      cannot be cleared by any later session, however it is spelled.
    * The driver has since banned the construct (#0, #3), the sibling-precedent rationale
      (#5) and reliance on the PASS ruling (#6). The union of those bans forecloses every
      known route to the only known distance-0 form.
  Consequence: further structural (or permuter, or forensic) sessions cannot change the
  outcome. They can only re-derive floor 1 and re-propose a banned construct, which the
  driver discards. This session therefore returns `owner-gated` with a decision packet
  (docs/grind/decisions.md, 2026-08-25) rather than spending a fourth modality on a closed
  space.

- [s11] NOTHING WAS SUBMITTED AND NO BAN WAS INVOKED. This session did not apply, re-spell,
  re-derive or cite in support any of banned_constructs #0-#6; the only body applied to src
  was the ban-free floor-1 form, and it was reverted. `candidate.c` is left exactly as s10
  banked it (the distance-0 body), with its header amended to record that it is currently
  UNSUBMITTABLE under the live ban list.

- [s8] LIVE CHASSIS (2026-08-25, this session): the ban-free s2-s7 body scores exactly 1 - {"score":1,"target_insns":76,"build_insns":76,"scorable":true,"rules_dropped":0}. The dispatch brief reported 'measurement unavailable'; the honest, submittable floor is 1, unchanged since s2. src/code6cac_b.c:1159 was reverted to INCLUDE_ASM("asm/funcs", func_800307D0); no bytes are on main and the working tree carries no src change.

- [s8] The score-1 residual is one instruction, verified at instruction level this session rather than inherited: build `addu s2,s0,v0` (02029021) vs target `addu $s2,$v0,$s0` (21905000). All 76 other instructions match. Artifact: tmp/grind/func_800307D0/s8/live_residual_2026-08-25.md.

- [s8] STRUCTURAL PARTITION IS COMPLETE. GCC 2.7.2 has exactly three expansion contexts for a two-operand `+` and all three are measured: assignment context (s7's 8-spelling matrix, all base-first, forced by c-typeck.c:1986/1988 + pointer_int_sum:2696); address/EXPAND_SUM context (s8's 7-row matrix; the only index-first rows are banned_constructs #0/#3); integer domain (index-first, cheat-reviewer FAILED and owner-REFUSED 2026-07-22 at decisions.md:1311). No unmeasured cell remains, so no further structural spelling can exist that is both ban-free and distance-0.

- [s8] OWNER DIRECTIVE EXECUTED AND INERT (third independent execution). 'ONE bounded re-test under the 2026-08-20 or-tree carve-out; check expand_binop canonicalization reachability first': optabs.c:399-421's commutative swap requires (op1 REG && op0 not REG) || target == op1 || op0 CONST_INT; the idx25 operands are two pseudo REGs, so it never fires. The actor is expr.c:5288-5290 in MEM-address context, which is an expansion-context question and not a written-operand-order question, so the carve-out's prerequisite-4 premise does not describe this residual at all. s8 and s9 reached the same result independently; this session did not spend measurements re-running it a fourth time.

- [s8] THE OBSTACLE IS PROCEDURAL. A distance-0, zero-rule, zero-cheat pure-C body is byte-proven twice on main (sandbox 0, 76/76, rules_dropped 0; full-build SHA1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa - evidence.md == s8 == and == s10 ==), yet has been adjudicated three times in one day with contradictory verdicts: FAIL (decisions.md:11147), PASS (decisions.md:11151, 'the ban is over-broad as applied ... it needs no exception'), FAIL (decisions.md:11155). The driver's bans #0/#3/#5/#6 now foreclose both the construct and the two rationales the PASS ruling used, and the 15:52 T3 finding is against the derivation history rather than the construct - a test no future session can clear, because the pipeline requires that history to be preserved.

- [s8] NO BAN WAS INVOKED OR RESPELLED THIS SESSION. The only body applied to src was the ban-free floor-1 form, and it was reverted. banned_constructs #0-#6 were neither applied, re-spelled, re-derived, nor cited in support of anything. candidate.c is left exactly as s10 banked it, with a header added recording that it is currently UNSUBMITTABLE under the live ban list and must not be re-applied or re-spelled.

- [s8] DECISION PACKET FILED THIS SESSION at the end of docs/grind/decisions.md: '## 2026-08-25 - func_800307D0 (cpu_check_tubazeri_2, src/code6cac_b.c) - **OWNER-ESCALATION - ESCALATED WITH DECISION PACKET**'. It is a routing/adjudication-conflict packet, not a standard-lowering one: answer (b) is simply the 2026-07-22 owner ruling (decisions.md:1311, option b) restated and extended, and answer (a) asks only whether the construct is judged on its own merits rather than on how the author found it. No family grant, no evidence-bar override, no 'accept the debt' wording.
