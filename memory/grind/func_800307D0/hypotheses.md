# Hypothesis ledger — cpu_check_tubazeri_2

## Honest floor: 4 (candidate.c applied; HEAD src was 9 with a `(0,...)` comma cheat + after_sll split)

The 4 residual diffs are TWO independent GCC-internal decisions:

### H1 — strength-reduce / combine_givs on the copy loop (idx 37/38/40, 3 insns)
- **Mechanism:** loop.c `strength_reduce` + `combine_givs` replaces the biv
  `ptr` (`ptr += 2`) with a new giv-biv `q = ptr + 0x332`, then expresses the
  0x334 access as `2(q)` and the 0x332 access as `0(q)`. Target's build keeps
  `ptr` as the biv and expresses BOTH accesses as replaceable displacement givs
  (`0x334(v1)` / `0x332(v1)`), so no new register is allocated.
- **Evidence it's an optimizer divergence, not a shape gap:** m2c reconstruction
  (tmp/grind/.../s1/m2c.c) shows the target's C loop is byte-for-byte our
  candidate's shape (`var_v1 = temp_s2; ... field(v1,0x332)=field(v1,0x334); v1+=2`).
  Same source shape → our fork combines the givs, target does not.
- **Next probe:** instrumented cc1 loop.c dump (BB2 debug knobs / `-da` RTL after
  loop) on the candidate's `.i` to read the `benefit` calc that makes combine_givs
  fire here; derive the C shape that leaves both accesses as replaceable
  displacement givs (cost-0, no new reg). WIP already killed the obvious loop
  reshapes (u16 stride-1, for-index, comma-init, no-guard do-while).

### H2 — commutative-plus operand order in the pointer add (idx 25, 1 insn)
- **Mechanism:** `s2 = (u8*)a0 + v0` compiles to `(plus a0_pseudo v0_pseudo)`;
  GCC canonicalizes commutative operands by pseudo regno. Param `a0` gets a lower
  pseudo than the local shift-result `v0`, so a0 lands as rs → `addu s2,s0,v0`.
  Target had the shift-result pseudo lower → `addu s2,v0,s0`.
- **Next probe:** greg/RTL dump to read the two pseudos + their conflicts; find C
  that makes the shift-result pseudo sort before the param pseudo (e.g. copy a0
  into a fresh local consumed only by the add, AFTER the shift is materialized).
  WIP killed the source-level operand-order rewrites (uintptr cast, byteptr-first,
  explicit s2_int) — all add conversion insns; the flip is a pseudo-numbering
  property, not a source-order property.
</content>
</invoke>

## [s1] Applying memory candidate.c to src lowers the honest floor from 9 to 4.
- mechanism: HEAD src carried a `(0,...)` comma cheat + after_sll goto split (floor 9); the candidate's clean v0/i-split + xor-intermediate + ptr-based do-while form scores 4 with 9 rules dropped.
- probe: sandbox cpu_check_tubazeri_2 --disable all before/after applying candidate.c
- result: HEAD=9, candidate=4 (target_insns=build_insns=76, scorable).
- verdict: CONFIRMED

## [s1] The 4 residual diffs are GCC-internal optimizer divergence, not a source-shape gap.
- mechanism: m2c reconstruction of the target asm produces a loop and pointer-add byte-for-byte identical to our candidate's C shape (var_v1=temp_s2; field(v1,0x332)=field(v1,0x334); v1+=2; and var_v0+arg0). Same source shape yields our fork's strength-reduce+operand-swap while target does neither.
- probe: python3 tools/m2c/m2c.py --valid-syntax asm/funcs/cpu_check_tubazeri_2.s -> tmp/grind/.../s1/m2c.c
- result: m2c loop == candidate loop; residual = strength_reduce/combine_givs (idx37/38/40) + commutative-plus operand order (idx25).
- verdict: CONFIRMED

## [s2] Rewriting the copy-down loop to index off base param a0 (for i=s1; ... *(u16*)((u8*)a0+0x332+i*2)=*(...+0x334+i*2)) instead of walking a pointer off s2 keeps the biv anchored, eliminating the combine_givs strength-reduction (idx37/38/40, 3 insns).
- mechanism: loop.c strength_reduce/combine_givs folds the 0x332 displacement into a new giv-biv q=ptr+0x332 when the walking pointer's base s2 is used only to init ptr. Indexing off a0 (which is ALSO re-read each iter for the count reload at 0x330) keeps a0/s2 as the anchored biv, both accesses expressed as displacement givs 0x332/0x334 off it. Directly evidenced: same-file sibling func_80030900 uses the identical index-off-a0 loop and compiles (same TU) to the exact biv-kept form.
- probe: Applied the index-based for-loop; `sandbox cpu_check_tubazeri_2 --disable all`; objdump vs target.
- result: score 4 -> 1. Loop asm now byte-identical to target (move v1,s2; lhu 0x334(v1); addiu; sh 0x332(v1); slt; bnez; addiu v1,v1,2). cheat-reviewer PASS (normal C loop, real semantics, sibling-mirrored).
- verdict: CONFIRMED

## [s2] The residual idx25 operand swap (target `addu s2,v0,s0` scaled-index-first vs ours `addu s2,s0,v0` base-first) is NOT reachable by any pure-pointer-C spelling; the only distance-0 form is an integer-domain offset-first add, which the cheat-reviewer FAILED.
- mechanism: Front-end pointer_int_sum canonicalizes ptr+int to base-first, so every pointer spelling emits base-first. Only integer-domain `v0 + (s32)a0` (offset written first) preserves source order and emits index-first -> score 0, but its sole purpose is flipping the commutative addu operand order (or-tree-shape-shift analogue); reviewer FAIL on tests 1/2/3/5.
- probe: 5 spellings measured at the fixed-loop base: (u8*)a0+v0=1, v0+(u8*)a0=1, &((s16*)a0)[s1]=1 (with and without explicit v0=s1<<1), inlined *(a0+v0+0x332) with no named s2=1 (CSE re-forms base-first addu), (s32)a0+v0=1; v0+(s32)a0=0. cheat-reviewer invoked on the score-0 form.
- result: All pointer/base-first forms score 1. Integer-offset-first scores 0 but is a reviewer-rejected commutative-operand-order coercion. Clean legitimate floor = 1 (single operand-order-only insn).
- verdict: KILLED

## [s3] The idx25 addu operand order is FIXED at tree-lowering (base-first) and cannot be moved by any structural lever.
- mechanism: c-typeck pointer_int_sum lowers `ptr + int` to PLUS_EXPR(ptr,int) = base-first. Expand emits (plus a0 v0) at initial RTL (insn 77). combine.c does NOT canonicalize a two-register commutative plus by pseudo regno, so the generation-time spelled order survives to final asm. RA / declaration order / block-local splits / statement re-association all act AFTER this order is committed, so none can flip it.
- probe: cc1 -da RTL dump (real build flags) on the score-1 src; traced insn 77 across full.i.rtl (initial), full.i.combine, full.i.greg (final). Plus measured 2 new structural forms (fresh-local a0 copy after shift; inline shift no-named-v0) via sandbox.
- result: insn 77 = (plus a0 v0) base-first at initial RTL, UNCHANGED through combine and greg (reg72=a0 < reg75=v0, a0 still first — no regno swap). Both new structural forms score 1 (base-first). Only int-domain `v0+(s32)a0` reaches 0 (reviewer-rejected cheat).
- verdict: KILLED (structural axis for idx25 is dead; 1-insn operand-order-only endgame-lock candidate confirmed).

## [s3] The idx25 addu operand order (target index-first `addu s2,v0,s0` vs ours base-first `addu s2,s0,v0`) is fixed at tree-lowering and cannot be moved by any structural lever (declaration order, block-local split, fresh-local copy, statement re-association, type narrowing).
- mechanism: c-typeck pointer_int_sum lowers `ptr + int` to PLUS_EXPR(ptr,int) = base-first. Expand emits (plus a0 v0) at initial RTL (insn 77). combine.c does NOT canonicalize a two-register commutative plus by pseudo regno, so the generation-time spelled order survives to final asm. RA / declaration order / block-local splits / statement re-association all act AFTER the operand order is already committed, so none can flip it.
- probe: cc1 -da RTL dump with the real build flags on the score-1 src; traced insn 77 across full.i.rtl (initial), full.i.combine, full.i.greg (final). Also measured 2 new structural forms via sandbox: fresh-local `base=a0; s2=base+v0` (H2's explicitly-named un-run probe) and inline shift `(u8*)a0+(s1<<1)` with no named v0.
- result: insn 77 = (plus reg72=a0 reg75=v0) base-first at initial RTL, UNCHANGED through combine and greg (reg72<reg75 yet a0 still emits first -> no regno swap) -> addu s2,s0,v0. Both new structural forms score 1 (base-first). Only integer-domain `v0+(s32)a0` reaches distance 0, and that is the reviewer-FAILED int-cast cheat.
- verdict: KILLED

## [s4] A decomp-permuter re-seed from the score-1 base finds a non-manual structural lever that flips the idx25 addu operand order (base-first addu s2,s0,v0 -> target index-first addu s2,v0,s0) without the reviewer-rejected integer-cast cheat.
- mechanism: Randomized/directed structural search from the closer base explores C forms (aliases, reassociation, temp splits, loop/pointer-type variants) outside the hand-derived pointer-spelling axis; any lower-scoring form is a proposal to vet against the cheat catalog.
- probe: Built an offset-0 single-function permuter workspace (base.c reduced TU + full maspsx pipeline compile.sh + target.o from asm/funcs, validated base=76==target=76 with the exact single idx25 residual). Ran two campaigns under fresh-seed discipline via tools/permuter_campaign.py, --stop-on-zero, -j8, --stack-diffs: seed1 default index-for-loop chassis (58,328 iters), seed2 structurally-distinct s16*-scaled-arith + while-loop chassis (44,141 iters). Waited in-turn (permuter_campaign.py wait windows), harvested + --stop each.
- result: seed1: 1 novel output at score 10 (== base 10; the score-neutral new_var=(u8*)a0 alias variant, s2 still lowers addu s2,s0,v0 base-first). seed2: ZERO novel finds. Combined ~102k iterations, no sub-10 (sub-sandbox-1) legitimate form; permuter never synthesized even the known int-cast score-0 cheat.
- verdict: KILLED

## [s5] A THIRD structurally-distinct decomp-permuter basin (guarded do-while index-off-a0 + array-subscript head) flips the idx25 addu operand order (base-first -> target index-first) without the reviewer-rejected integer-cast cheat.
- mechanism: A fresh chassis maximally different from s4's two seeds seeds a different random-walk basin; any lower-scoring mutation is a proposal to vet against the cheat catalog. Targets the sole residual = the idx25 `addu s2,s0,v0` (base-first) commutative operand order.
- probe: Built + validated an offset-0 single-function workspace (tmp/grind/.../s5/ws3): guarded do-while `i=s1; if(i<n-1){do{copy;i++;}while(i<n-1);}` + head `s2=(s32*)&((s16*)a0)[s1]`, a0 kept direct; base 76==target 76 with the exact single idx25 residual, permuter base_score=10. Ran one campaign under fresh-seed discipline (--stop-on-zero, -j8, --stack-diffs), waited in-turn across two ~9-min windows (42,048 iters, ~19.5 min), harvested + --stop.
- result: ZERO novel finds (best_new_score=null). Combined with s4's two chassis: ~144k iters over 3 independent basins, no sub-10 legitimate form and not even the known score-0 int-cast cheat. (A named-base `u8 *base=(u8*)a0;` chassis variant was rejected pre-launch: it drops the `move a0,s0` liveness -> base 75 vs 76, a noisier 2-residual basin.)
- verdict: KILLED (permuter axis triply-dead; corroborates the s3 RTL tree-lowering proof — idx25 operand order is un-flippable by any pointer-domain restructuring, and the only distance-0 form is the reviewer-rejected int-cast operand swap).

## [s5] A third structurally-distinct decomp-permuter basin (guarded do-while index-off-a0 copy loop + array-subscript head s2=(s32*)&((s16*)a0)[s1], a0 kept direct) flips the idx25 addu operand order (base-first addu s2,s0,v0 -> target index-first addu s2,v0,s0) without the reviewer-rejected integer-cast cheat.
- mechanism: A fresh chassis maximally different from s4's two seeds (for-loop/direct-cast; while-loop/s16*-scaled) seeds a different random-walk basin; any lower-scoring mutation is a proposal to vet against the cheat catalog. The sole residual it must move is the idx25 addu commutative operand order, which the s3 cc1 -da RTL proof showed is fixed base-first at tree-lowering by pointer_int_sum.
- probe: Built + validated an offset-0 single-function permuter workspace (tmp/grind/cpu_check_tubazeri_2/s5/ws3) through the full cc1->prologue_fix->maspsx->multu_pad pipeline: base 76==target 76 with the exact single idx25 residual, permuter base_score=10. Ran one campaign (tools/permuter_campaign.py, --stop-on-zero, -j8, --stack-diffs), waited in-turn across two ~9-min windows (42,048 iters, ~19.5 min), harvested + --stop.
- result: ZERO novel finds (best_new_score=null). Combined with s4's two chassis: ~144k iterations over 3 independent structurally-distinct basins, no sub-10 (sub-sandbox-1) legitimate form and not even the known score-0 int-cast cheat. A named-base (u8 *base=(u8*)a0;) chassis variant was rejected pre-launch: it drops the move a0,s0 liveness -> base 75 vs 76, a noisier 2-residual basin.
- verdict: KILLED

## [s6] The idx25 addu operand order could be flipped to target's index-first by a pseudo-regno / RA / declaration-order lever that makes the shift-result pseudo sort before the param pseudo (WIP-H2's un-run mechanism).
- mechanism: WIP-H2 posited GCC canonicalizes commutative plus by pseudo regno, so making v0's pseudo < a0's pseudo would emit index-first without an integer cast.
- probe: cc1 -da on reduced TU (s4 base.c), two variants differing ONLY in the s2 add line: ptr `(u8*)a0+v0` (score 1) vs int `v0+(s32)a0` (score 0). Compared insn 77 (the s2 add) across .rtl (initial), .combine, and final .s; compared pseudo numbering; read c-typeck.c build_binary_op + pointer_int_sum.
- result: DISPROVEN. Pseudo numbering identical in both (reg72=a0 < reg75=v0=s1<<1, reg77=s2). insn 77 differs ONLY in operand order: ptr=(plus 72 75) base-first -> addu $18,$16,$2; int=(plus 75 72) index-first -> addu $18,$2,$16 (=target). Order is set at C front-end tree build: pointer_int_sum builds PLUS_EXPR(ptrop,intop) pointer-first unconditionally (c-typeck.c:2696), and build_binary_op forces the pointer to ptrop for both `ptr+int` (line 1986) and `int+ptr` (line 1988). No RTL pass (cse/loop/combine/greg/sched) reorders a two-register commutative plus by regno. Index-first requires both operands INTEGER_TYPE (the reviewer-rejected int-cast).
- verdict: KILLED (front-end tree-build lock; pseudo-regno/RA/decl-order axis definitively dead at exact-source-line resolution; corroborates s3 RTL trace + s4/s5 permuter triple-kill).

## [s6] The idx25 addu operand order could be flipped to the target's index-first form by a pseudo-regno / RA / declaration-order lever making the shift-result pseudo (v0) sort before the param pseudo (a0), without the reviewer-rejected integer cast (WIP-H2's explicitly un-run mechanism).
- mechanism: cc1 -da on the reduced s4 TU, two variants differing ONLY in the s2 add line: ptr `(u8*)a0+v0` (sandbox 1) vs int-cast `v0+(s32)a0` (sandbox 0). The s2 add is insn 77 at INITIAL RTL. ptr: insn77=(set(reg77)(plus:SI(reg72=a0)(reg75=v0))) base-first -> final `addu $18,$16,$2`. int: insn77=(plus:SI(reg75=v0)(reg72=a0)) index-first -> final `addu $18,$2,$16` == target `addu s2,v0,s0`. Pseudo numbering is BYTE-IDENTICAL between the two (reg72=a0 < reg75=v0=s1<<1, reg77=s2), so the operand order is NOT a regno-canonicalization property. It is set at the C front-end tree build: c-typeck.c build_binary_op PLUS_EXPR routes any pointer+int to pointer_int_sum with the pointer forced as ptrop (line 1986 for ptr+int, line 1988 SWAPS int+ptr so pointer is still ptrop), and pointer_int_sum builds `result=build(resultcode,result_type,ptrop,intop)` (line 2696) = pointer-first unconditionally. The int-cast form has both operands INTEGER_TYPE -> ordinary integer PLUS_EXPR path preserving written order (v0 first) -> index-first. combine/greg/sched never reorder a two-register commutative plus (int form's insn77 stays (plus 75 72) through combine).
- probe: tmp/grind/cpu_check_tubazeri_2/s6/forensic_dump.sh: cpp+cc1 -da on ptr.c and int.c (only the s2 line differs); compared insn 77 in {ptr,int}.i.rtl / .combine and the final {ptr,int}.s; read tools/gcc-2.7.2/c-typeck.c build_binary_op (1983-1990) + pointer_int_sum (2621-2700).
- result: Pseudo numbering identical; insn 77 base-first (ptr) vs index-first (int) at initial RTL, unchanged through combine/greg into final asm; the ONLY index-first form requires casting the pointer param to int (the reviewer-rejected coercion). No pointer-domain C and no RA/scheduling/decl-order/pseudo-numbering lever can reach index-first.
- verdict: KILLED

## [s7] The target's idx25 index-first order is compiler-level evidence that the ORIGINAL source wrote this address computation in the integer domain (making v0+(s32)a0 a faithful reconstruction, not a coercion).
- mechanism: Whole-function two-register-add census: idx25 (`addu s2,v0,s0`) is the SOLE genuine rr-add in the 76-insn target (all other addu are moves/zero-inits). a0 is a genuine struct pointer throughout. Per s6 (pointer_int_sum:2696 lowers any pointer operand pointer-first, unconditionally), the shipped GCC 2.7.2 CANNOT produce index-first from a pointer-typed a0; index-first requires both operands integer-typed with the index first = `v0+(s32)a0`. Therefore the original source, compiled by the shipped toolchain, provably used integer-domain address arithmetic at exactly this add. Sibling func_80030900 (same TU) has ZERO rr-adds (all addiu-displacement), ruling out a file-wide compiler quirk.
- probe: grep census of asm/funcs/cpu_check_tubazeri_2.s (`addu` \ `zero`) and asm/funcs/func_80030900.s; cross-ref s6 c-typeck.c pointer_int_sum proof. Live sandbox re-confirm floor 1.
- result: idx25 is the sole rr-add and is index-first; unreachable from pointer domain under this compiler; sibling has no rr-add. The int-domain form the reviewer FAILED is the compiler-forced signature of the original source domain.
- verdict: CONFIRMED (forensic) — reframes the sole residual from "reviewer-rejected coercion" to "faithfulness-vs-coercion owner ruling", now backed by whole-function compiler-level evidence. Not self-classifiable per the prime directive -> ruling-request.

## [s7] Some pointer-domain spelling (a different pointee type, index signedness/width, index-written-on-the-left, or array-subscript form) flips idx25 from base-first to the target's index-first WITHOUT the reviewer-FAILED integer cast.
- mechanism: If c-typeck.c's pointer-first canonicalization (pointer_int_sum:2696) were conditional on pointee type, index type, or written operand order, a legitimate pointer spelling could reach index-first. Tested exhaustively via cc1 -da initial-RTL + final-asm operand-order read on a reduced type-matrix TU.
- probe: tmp/grind/cpu_check_tubazeri_2/s7/typematrix.c + typematrix_dump.sh (real build cc1 flags): 8 pointer probes (u8*/s16*/s32* x int/unsigned/long x idx-left `idx+base` x `&base[idx]` x exact tubazeri shape) + 2 integer probes (idx-first v0+(s32)a0, base-first (s32)a0+v0); read (plus ...) in typematrix.i.rtl and addu rs,rt in typematrix.s.
- result: ALL 8 pointer spellings lower base-first (initial RTL (plus BASE INDEX); final `addu rd,$4(base),INDEX`), unconditionally -- including p6 idx-written-left (confirms c-typeck.c:1988 int+ptr swap) and p7 array-subscript. Integer domain preserves WRITTEN order: i1 `v0+(s32)a0` -> index-first `addu $2,$2,$4` (= target idx25); i2 `(s32)a0+v0` -> base-first. So index-first is reachable ONLY from integer-domain-with-index-written-first.
- verdict: KILLED

## [s7] cpu_check_tubazeri_2 is a confirmed 1-insn commutative-operand-order endgame-lock whose sole distance-0 C is the reviewer-rejected int-cast; structural, permuter, and front-end forensic axes are all measured dead and the residual is not autonomously approvable.
- mechanism: idx25 operand order is fixed at the C front-end tree build (c-typeck.c build_binary_op forces pointer->ptrop at 1986/1988; pointer_int_sum builds PLUS_EXPR(ptrop,intop) pointer-first at 2696), immovable by any RA/scheduling/decl-order/pseudo-numbering/loop/combine lever. Only integer-domain-index-first source yields index-first = the reviewer-FAILED v0+(s32)a0. Per endgame-lock-disposition-policy AND-gate #2 the int-cast lacks SOTN/VS/ESA community precedent, so it is not autonomously approvable.
- probe: Live sandbox re-verify (score 1, 76/76, 9 rules dropped) + s7 type-matrix RTL census (this session) + s6 exact-source-line dump + s3 RTL trace + s4/s5 ~144k-iter permuter triple-kill + 2026-07-22 02:10 Judge FAIL (independently verified the forensics SOUND, ruled FAIL, directed escalation).
- result: Floor 1 confirmed live. No new closable pure-C form exists below floor 1. OWNER-ESCALATION filed to docs/grind/decisions.md (2026-07-22, cpu_check_tubazeri_2) with options (a) sanction int-domain reconstruction / (b) accept INCOMPLETE-owner-accepted at floor-1.
- verdict: CONFIRMED

## [s8] The idx25 index-first operand order is reachable from ordinary pointer-domain C after all, via a second RTL-generation canonicalisation (`expr.c` both_summands, "put a multiplication first") that s2-s7 never reached because every probe expanded the `+` in ASSIGNMENT context rather than in MEM-ADDRESS context.
- mechanism: `expr.c:5237-5239` routes a `+` to the address path only under `modifier == EXPAND_SUM && mode == ptr_mode`; an assignment RHS does `goto binop` and keeps the front end's pointer-first tree order (`c-typeck.c:2696` — the s6/s7 result, which is true but only about the TREE). In address context, `expr.c:5288-5290` swaps the summands when `GET_CODE (op1) == MULT`, and `expr.c:5362-5384` (MULT_EXPR under EXPAND_SUM with a constant multiplier) is what leaves the index as a literal `MULT` rtx — but only if the scale is written inline or comes from a subscript, never through a named shift local. `optabs.c:399-421 expand_binop` never swaps two pseudo REGs, so the assignment-context order genuinely is immovable: s3/s6/s7 were right about their axis and wrong only in generalising it to "no pointer-domain C can reach index-first".
- probe: 7-row head-spelling matrix measured with `sandbox func_800307D0 --disable all` (`tmp/grind/func_800307D0/s8/variant_matrix.md`), plus `pwsh tools/grinder/dump.ps1 func_800307D0` cc1 -da dumps of the winning TU, plus a source read of `expr.c` 5237/5288/5362 and `optabs.c` 399-421.
- result: `s3 = *(s16 *)((u8 *)a0 + s1 * 2 + 0x332)` (address context + inline MULT) => **score 0**; `((s16 *)a0)[s1 + 0x199]` => score 0; every assignment-context head (named pointer; `s2 = (s32*)((u8*)a0 + s1*2)`; `&((s16*)a0)[s1]`) and the named-shift-local address form => score 1. Initial RTL insn 67 = `(plus (reg 94 index) (reg 72 a0))`, final `addu $18,$2,$16` == target. Full build sha1 == oracle.
- verdict: CONFIRMED — function MATCHED at distance 0 with ordinary C, zero rules, zero cheat constructs, no family claim.

## [s8] The 2026-08-20 or-tree-shape-shift carve-out is the lever that closes this residual (owner directive: ONE bounded re-test).
- mechanism: the carve-out sanctions a single justified target-matching operand order in an associative+commutative expression (`|`, `&`, `^`, `+`). If idx25 were an operand-order choice, the carve-out would cover it.
- probe: re-read `.claude/rules/or-tree-shape-shift.md` § "Owner ruling 2026-08-20" prerequisites 1-5, then re-measured written-order and grouping variants of the head at the live chassis (the B2 / D / E rows of the s8 matrix).
- result: INERT. Written operand order has no effect in the pointer domain (`c-typeck.c:1988` normalises `int + ptr` to `PLUS_EXPR(ptr,int)` regardless), so there is no order to "choose" and prerequisite 4's premise (a commutative shuffle with identical semantics) does not describe the residual at all. The match came instead from a structural, semantics-preserving change of EXPANSION CONTEXT (dereference instead of assignment), which is ordinary C needing no carve-out, no FAKE annotation and no family claim.
- verdict: KILLED (as a lever) — and moot: the function matched without it.
