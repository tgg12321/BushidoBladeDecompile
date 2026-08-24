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
