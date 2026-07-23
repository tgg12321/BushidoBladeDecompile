# Hypothesis ledger — func_80061658

Gap = pure v0<->v1 swap: unpinned GCC puts load-temp `t`->v1 and mask const->v0;
target wants t->v0, mask->v1. Scheduling is byte-identical. Root cause: global.c
allocno priority — mask's short scheduler-fixed live range outranks t's one long
reused-variable live range, so mask grabs v0 first. Goal: make `t` win v0 (raise
t's priority / shorten its live range) WITHOUT touching the mask (mask is a
constant; its placement is inert, KILLED s1).

## Frontier (mechanism-grounded, for drill)
1. **Shorten t's live range so its allocno beats mask's for v0.** t is one reused
   variable => one long allocno. Splitting into t0/t1/t2 was measured 11 (worse)
   — but WHY 11 was never captured in RTL. Probe: split t, dump greg, check
   whether the 3 short webs reuse v0 (target) or scatter to v1/a0 (the likely
   cause of the +2). If they scatter because the scheduler makes two loads
   simultaneously live, try ordering that keeps each load dead before the next.
2. **Directed permuter on the tail with a CLEAN single-function target**
   (asm/funcs/func_80061658.s + prelude, offset 0 per difficult-is-not-impossible
   §3). Search RA-steering structures for the tail 3-load + const-store block.
   Vet any closing form against the cheat catalog before proposing.
3. **Verify the winning form against a sibling** (func_80061710/617C8/618B4/
   611A4/6133C share the pattern) — a real pure-C fix should generalize.

## KILLED
- mask-before-call (cross-call to force mask->v1): const rematerialized after
  call; floor 9. rejected/mask-before-call.c.
- [s2] Grouping-preserving reorders of the single-temp form (mask mid, decl-order
  swap) — all floor 9. Pure reorder/decl-order cannot flip the RA tiebreak.
- [s2] Outer/multi load splits (load1, loads1&3, 3-scoped, t0/t1/t2) — floor 11.
  Multiple simultaneously-live single-use temps scatter to v0/v1/a0/a1 and break the
  interleave. rejected/split-outer-loads-scatter.c.

## s2 CONFIRMED / PROGRESS
- [s2] Lever A (block-local split of the MIDDLE load) lowers floor 9 -> 7.
  mechanism: mask's RMW-chained sets make it a LOCAL quantity that grabs v0 before the
  DISJOINT-range global load-temp t; a block-local `u` for load2 is born before mask,
  takes v0, and pushes mask to v1 (target-correct) — but the shared t (loads 1&3) then
  scatters to a0 and the scheduler hoists load2, distorting the interleave. Net 7.
  verdict: CONFIRMED (floor improved; best 7-form saved to candidate.c).

## s2 frontier (for next session)
1. Directed permuter over the tail RA (frontier #2, PERMUTER modality): base from the
   floor-9 pure-swap form (correct schedule, only RA differs) — cleaner base than the
   7-form. Build clean single-fn target from asm/funcs/func_80061658.s at offset 0.
   The specific unmet need: force the DISJOINT-range shared load-temp to win v0 over the
   RMW-chained LOCAL mask while keeping the single-temp interleave.
2. If permuter also plateaus, the residual is a local-vs-global allocation-class wall
   (register-rotation family); escalate per policy (NOT owner-gated yet — floor still
   dropping, still grindable).

## [s1] The honest floor-9 residual is a pure v0<->v1 register swap in the tail, not a scheduling/interleaving difference as the prior WIP claimed.
- mechanism: objdump of pin-stripped build vs asm/funcs shows identical instruction order/interleaving; only the register names differ: target puts load-temp t in v0 and mask 0x10FFFF in v1, unpinned GCC does the reverse. All 9 differing insns are the v0/v1 tail insns.
- probe: mipsel objdump of tmp/sandbox .o tail vs asm/funcs/func_80061658.s; RTL greg dump (np.i.greg) shows load-temp in reg3(v1), mask const in reg2(v0).
- result: 9 differing insns = the swap; scheduling identical.
- verdict: CONFIRMED

## [s1] Unpinned GCC swaps because global.c allocno priority ranks the mask const above the reused load-temp t for v0.
- mechanism: reg_alloc_order puts $2(v0) before $3(v1); priority=log2(n_refs)*freq/live_length. t is one reused C variable => single long allocno (6 refs, ~9-insn span) with LOWER priority than mask's short scheduler-fixed live range (~3 refs, ~5-insn span), so mask is allocated first and grabs v0.
- probe: greg conflict/disposition dump np.i.greg (5-6 regs to allocate; load-temp->reg3, mask->reg2).
- result: Priority ordering explains the observed swap.
- verdict: CONFIRMED

## [s1] Placing mask=0x10FFFF before the func_80060A68() call forces mask to cross the call (call clobbers v0), pushing mask off v0 into v1 and freeing v0 for t (= target allocation).
- mechanism: A value live across a call conflicts with the call-clobbered v0, so global.c must avoid v0 for it.
- probe: Edited src (pins removed, mask assign moved before call), sandbox --disable all; disassembled tail.
- result: Floor still 9; disasm shows GCC rematerializes the constant (lui+ori) AFTER the call, so mask never crosses it; tail still lw v1 (t->v1). Corollary: any source reorder of a constant mask is inert (scheduler pins its live range to just before its single use) -- explains prior WIP's 3 mask-move variants all being 9.
- verdict: KILLED

## [s2] The s1 v0<->v1 swap is caused by mask winning v0 in local_alloc (which runs before global_alloc) because its two sets (li+ori) are RMW-chained into one contiguous quantity, while the load-temp t's three disjoint load ranges force it to global_alloc where it gets leftover v1.
- mechanism: GCC 2.7.2 local-alloc combines only chained/contiguous sets into a quantity; local_alloc runs first and seizes v0. t's 3 independent loads are disjoint ranges -> not combinable -> deferred to global_alloc -> v1.
- probe: Clean cc1 -da dump from current src (tmp/grind/func_80061658/s2/dumps/cur.i.lreg + cur.i.greg): pseudo 76=mask (li insn105 + ior insn106, RMW), pseudo 75=t (loads insn76/81/92, disjoint); disposition + '6 regs to allocate' list show t global, mask local.
- result: mask local->v0, t global->v1; exactly the 9-insn pure swap.
- verdict: CONFIRMED

## [s2] Lever A (block-local split of the middle load) forces mask off v0 onto v1 (target-correct) and lowers the honest floor from 9 to 7.
- mechanism: A block-local `u` for load2 is a local pseudo born before mask; local_alloc gives u v0 and mask the next free reg (v1). But the shared t (loads 1&3) stays a disjoint-range global pseudo -> scatters to a0, and u (no anti-dep) lets the scheduler hoist load2 -> interleave distorted.
- probe: Edited src to `{ s32 u = arg0[1]; D_800F1144 = u; }`, sandbox --disable all = 7; emitted tail (cur.s): load1@a0, load2(u)@v0, mask@v1, load3@a0, load2 hoisted.
- result: Floor 9 -> 7; mask now correctly in v1; best 7-form saved to candidate.c.
- verdict: CONFIRMED

## [s2] Grouping-preserving reorders and declaration-order changes of the single-temp form cannot flip the RA tiebreak.
- mechanism: Pseudo birth follows statement/use order, not decl order; the constant mask's live range is scheduler-pinned, so reordering its store is inert; the local/global classification is unchanged.
- probe: Sandbox: mask between load1/load2 = 9; decl-order swap (mask decl first) = 9; [s1 banked] mask-early=9, mask-late=9, mask-before-call=9.
- result: All floor 9 - no movement.
- verdict: KILLED

## [s3] Chaining the 3 loads into ONE live range (frontier #2) makes the load-temp a single-death LOCAL quantity that wins v0.
- mechanism: local-alloc.c:472 gates local alloc on reg_n_deaths==1; a struct/block copy might fuse the 3 loads into one contiguous quantity.
- probe: `*(struct{s32 a,b,c;}*)&D_800F1140 = *(struct{...}*)arg0;` + separate mask store; sandbox --disable all.
- result: score 22 (build_insns 34 vs 46). GCC lowers the block copy to lw/sw pairs that STILL die 3x AND drops the mask interleave / folds addressing — wholly different shape. 3 distinct values = 3 deaths regardless of C spelling.
- verdict: KILLED. rejected/struct-block-copy.c.

## [s3] The v0<->v1 swap is a local-vs-global allocation-CLASS wall, GCC-source-confirmed — not a flippable priority tie.
- mechanism: local-alloc.c:472 `reg_n_deaths==1` gate forces the 3-death load-temp to global and keeps the 1-death mask local; MIPS has NO REG_ALLOC_ORDER so find_free_reg gives the lone local mask qty the lowest free reg = v0(2), leaving v1 for the global load-temp. Escapes (a) load-temp die-once [impossible], (b) mask die-twice [no byte-neutral C], (c) v0-blocking local split [= measured 7-11, scheduler hoist / v0 steal] all fail.
- probe: fresh greg/lreg dumps (s3/dumps/) + read of local-alloc.c:460-478,1533-1556 + grep REG_ALLOC_ORDER (0 matches) + struct-copy measurement.
- result: structural modality exhausted with proof; residual is class-wall, not tie.
- verdict: CONFIRMED (wall is real; NOT owner-gated — permuter modality still open).

## s3 frontier (for next session) — unchanged intent, structural axis now measured-dead
1. Directed PERMUTER over the tail RA (frontier #1, PERMUTER modality) from the floor-9
   pure-swap base (correct schedule, only RA differs). Specific unmet need: force the
   3-death GLOBAL load-temp to win v0 while mask stays local — which requires steering
   local-alloc's find_free_reg off v0 for mask (e.g. a copy-suggestion or a hard-v0
   conflict) that manual analysis can't construct byte-neutrally. If the permuter also
   plateaus, escalate per endgame-lock-disposition-policy (register-class wall,
   few insns short) — NOT owner-gated while any modality is unrun.

## [s2] Splitting loads other than the middle one reaches the match.
- mechanism: Multiple simultaneously-live single-use load temps have no anti-deps, so the scheduler hoists all loads and the temps scatter across v0/v1/a0/a1, breaking the target interleave.
- probe: Sandbox: split load1 = 11; split loads 1&3 = 11; 3 scoped block-locals = 11; 3 named temps t0/t1/t2 = 11; split load3 = 8; only split load2 = 7.
- result: All non-middle splits WORSE (8-11); only the middle-load split improves.
- verdict: KILLED

## [s3] Chaining the three tail loads into ONE live range (frontier #2) makes the load-temp a single-death LOCAL quantity that wins v0 over mask.
- mechanism: local-alloc.c:472 gates local alloc on reg_n_deaths==1; a struct/block copy of the 3 contiguous words (D_800F1140/1144/1148 <- arg0[0..2]) might fuse the 3 disjoint load ranges into one contiguous quantity so it becomes local and born-first.
- probe: Set src tail to `*(struct{s32 a,b,c;}*)&D_800F1140 = *(struct{...}*)arg0; mask=0x10FFFF; D_800A3464=mask;` and ran sandbox func_80061658 --disable all.
- result: score 22, build_insns 34 vs 46. GCC lowers the block copy to lw/sw pairs that STILL die 3x AND drops the mask-store interleave / folds addressing -> wholly different shape. Three distinct loaded values = three deaths regardless of C spelling.
- verdict: KILLED

## [s3] The honest v0<->v1 residual is a local-vs-global allocation-CLASS wall fixed by GCC's source, not a priority tie that grouping-preserving structural transforms can flip.
- mechanism: tools/gcc-2.7.2/local-alloc.c:472 defers any pseudo with reg_n_deaths!=1 to global_alloc: the load-temp loads 3 distinct values (dies 3x) -> always global; mask is one constant (dies once) -> always local. MIPS defines NO REG_ALLOC_ORDER (grep config/mips = 0), so find_free_reg assigns hard regs ascending; the lone local mask qty in block 5 deterministically takes v0(2), leaving the global load-temp v1(3). Target wants the reverse. The three escapes all fail: (a) load-temp die-once is impossible (3 distinct values); (b) mask die-twice has no byte-neutral pure-C form (its store is on the single post-call path; a 2nd use = an extra insn); (c) a v0-blocking single-death local across mask is a load SPLIT, and every split measured 7-11 (scheduler hoists the anti-dep-free split temp and/or steals v0 from the shared global load-temp -> scatter to a0).
- probe: Regenerated greg/lreg dumps from the floor-9 pure form (s1/s2 dumps were gitignored+gone); confirmed pseudo 75 (t) in global list -> v1, pseudo 76 (mask) absent from global list -> v0 via local alloc. Read local-alloc.c:460-478 and :1533-1556; grepped REG_ALLOC_ORDER; measured the struct-copy escape.
- result: Structural modality exhausted with a source-level proof. Best floor stays 7 (s2 middle-split; correct-schedule pure-swap base stays 9). Residual is class-wall, not tie.
- verdict: CONFIRMED

## [s4] Directed permuter finds a byte-neutral pure-C form reaching honest sandbox 0.
- statement: A permuter over the floor-9 pure-swap tail finds a byte-neutral structure that steers local-alloc off $v0 for the mask, letting the load-temp win $v0 while mask stays $v1.
- mechanism: Stage the mask constant through the already-used local `val` (`val = 0x10FFFF; mask = val;`). The `mask = val` copy gives mask a copy-preference and val's reuse (multi-death) changes the class disposition, so find_free_reg does not hand mask $v0. mask -> $v1, reused 3-death load-temp -> $v0 = target.
- probe: s4 permuter campaign (7 score-0 finds); output-0-2 applied to src -> engine sandbox --disable all = 0 (46/46, rules_dropped 0, zero pins/asm).
- result: FRONTIER #1 CONFIRMED — a pure-C sandbox-0 form exists and is bytes-proven. The residual was never a genuine wall.
- verdict: CONFIRMED (bytes), but layer-1 cheat-reviewer FAIL on classification -> ruling-request (not accepted; awaiting owner).

## s4 frontier (for the owner / next session)
1. OWNER RULING NEEDED: is `val = 0x10FFFF; mask = val;` (staging a constant through a
   reused live local to change local-alloc register CHOICE on straight-line code)
   sanctioned as COMPLETED-C (SOTN variable-reuse spirit), or does it need its own
   SOTN-master evidence pass + sign-off as a new technique-family application? All
   prereqs present: live code, FAKE annotation, lever-exhaustion (s1-s3), named GCC
   mechanism (local-alloc.c:472 copy-preference).
2. If SANCTIONED: apply candidate.c to src (revert the 2 pins to `s32 t; s32 mask;`,
   swap the mask store to the staged form), sandbox=0 is already proven, retire +
   queue done. The cluster siblings (func_80061710/617C8/618B4/611A4/6133C) share the
   t=$2/mask=$3 pattern — the SAME lever likely closes them (verify each).
3. If NOT sanctioned: the function is a genuine endgame lock; owner-gate per
   endgame-lock-disposition-policy (but a proven pure-C form exists, so it is NOT a
   register-class wall — the block is purely policy, not reachability).

## [s4b] The s4 `val`-staging copy is the UNIQUE score-0 basin; no sanctioned pure-C form reaches a match.
- statement: Every byte-neutral pure-C form that reaches honest sandbox 0 for func_80061658
  requires staging the mask constant 0x10FFFF through a copy source (the local-alloc
  copy-preference). No other mechanism moves mask off $v0. Denied a copy source, the permuter
  plateaus at the pure v0<->v1 swap (floor 50 weighted / 9 insns) and cannot reach 0.
- mechanism: s3 GCC-source proof — find_free_reg (no MIPS REG_ALLOC_ORDER) gives the lone
  single-death LOCAL mask the lowest free reg $v0; the only lever to steer it off $v0 is a
  copy-suggestion, and a bare constant has no natural copy source. So a match REQUIRES
  introducing an artificial copy (val-staging or a synthesized temp) = the coercion family.
- probe: fresh-seed campaign s4b-noval-freshseed on a chassis with `val` removed (switch
  constants inlined), base_score 510, 30,762 iters; best_new_score 50, zero score-0. Plus
  independent engine re-verification of the s4 val-staging form = sandbox 0 (46/46, 0 rules).
- result: CONFIRMED — the score-0 basin is uniquely the constant-staging coercion; the
  permuter modality yields no SANCTIONED closing form. Combined with the s3 structural
  closure, every grind-advanceable axis is measured dead. Residual is policy-only.
- verdict: CONFIRMED. Disposition: owner-gated (OWNER-ESCALATION filed 2026-07-23).

## [s4] The s4 `val = 0x10FFFF; mask = val;` copy is the UNIQUE score-0 basin; no sanctioned pure-C form reaches a match for func_80061658.
- mechanism: s3 GCC-source proof: find_free_reg (no MIPS REG_ALLOC_ORDER) gives the lone single-death LOCAL mask the lowest free reg $v0; the only lever to move mask off $v0 is a copy-suggestion, and a bare constant has no natural copy source. A match therefore REQUIRES introducing an artificial copy of the constant (val-staging or a synthesized dead temp) = the coercion family the Judge FAILed.
- probe: Fresh-seed campaign s4b-noval-freshseed on a structurally-different chassis with the reused local `val` REMOVED (switch constants inlined: `*v1 = 0x21000C;` / `0x21000D;`), base_score 510, -j8, 30,762 iterations, harvest+stop in-session. Plus independent engine re-verification of the s4 val-staging form.
- result: best_new_score = 50 (the pure v0<->v1 swap), NO score-0 produced. output-50-1 synthesized its own temp `new_var` but staged arg0/p through it (not the mask), so RA did not flip. The val-staging form re-verified this session at engine sandbox --disable all = 0 (46/46, rules_dropped 0, zero pins). Denied the natural copy source, the permuter cannot reach a match.
- verdict: CONFIRMED
