# Hypothesis ledger — func_8007B844

## s1 (recon, 2026-07-21) — initial frontier

Baseline re-confirmed: candidate.c (Lever B) in src -> sandbox --disable all = 6,
build_insns 38 == target 38, verdict C. Function identified as PsyQ libgpu
**ClearOTagR** (see evidence.md s1 entries).

### F1 — struct-typed device-table dispatch (original-source shape)
- **Hypothesis:** the original TU declared `Gpu_dev` as a pointer to a struct
  with named function-pointer fields and dispatched `Gpu_dev->otc(ot, n)`.
  A struct-field call (declared fn-ptr member type) may stage the dispatch
  load / call boundary through different pseudo numbers than the
  `v0[11]` array-index cast, changing what sched.c sees at the post-call
  boundary (the discriminator the sibling evidence points at is the
  preceding-BODY shape, not the tail).
- **Mechanism:** RTL-gen pseudo allocation order at the call boundary feeds
  global RA's conflict graph and sched.c's dependence chains; a typed member
  access is a semantically-faithful respelling (NOT a cheat — it is the
  known original shape), unlike the measured-neutral direct-cast form
  (round 4), which kept the untyped word-array view.
- **Next probe:** declare a faithful `GpuDevice`-style struct (fields up to
  offset 0x2C named, `otc` as `void (*)(u32 *, s32)`), type
  g_gpu_dev_table's extern accordingly (header-level, use-site-consistent),
  dispatch `dev->otc(ot, n);` — sandbox, expect any score movement to be
  informative either way. Also try `GPU_printf`-shape debug call via the
  same struct-typing sweep.

### F2 — instrumented sched.c dump on the score-6 form (ledger avenue 1)
- **Hypothesis:** reading the return-staging pseudo's REG_DEP_TRUE
  predecessor list directly (BB2_SCHED_DEBUG instrumented cc1) will name the
  exact insn(s) whose dependence must be re-routed, narrowing the C search
  from "any restructure" to a specific dataflow target.
- **Mechanism:** sched.c INSN_PRIORITY = chain depth to jr; target needs
  return-staging depth >= 4, i.e. the store must consume the return pseudo
  (target asm: `sw $v1, 0($v0)`).
- **Next probe:** cc1 -da / BB2_SCHED_DEBUG run on the sandbox .i for the
  score-6 form; dump to tmp/grind/func_8007B844/sN/; enumerate which pure-C
  expression could make the store's base address a NON-copy-prop-foldable
  function of the return value (nothing found in 15 sessions; the dump may
  reveal an untried carrier).

### F3 — cross-project ClearOTagR reference (research modality)
- **Hypothesis:** other GCC 2.7.2 / ASPSX PSX decomps (psy-q library
  decompilations; projects that link-match libgpu sys.c) have byte-matched
  ClearOTagR; their exact C spelling + TU context (struct decls, GPU_printf
  macro shape) is direct evidence of the source shape that produces the
  return-staging-BEFORE-store schedule.
- **Mechanism:** same compiler family, same libgpu object — an existing
  matched decomp eliminates the search entirely.
- **Next probe:** web/GitHub research pass for ClearOTagR matches (SOTN psxsdk
  tree, psy-q lib decomp repos); transcribe candidate spellings and measure.

### Killed this session
- Const-first single-statement store (literal PsyQ spelling) = 7. KILLED.
- Lever B AND-operand swap = 6 (neutral; operand-order axis inside Lever B dead).
- Near-clone duplicate axis: zero pairs at threshold 0.75. DEAD.

## [s1] The literal leaked-PsyQ spelling `*ot = 0xFFFFFF & (u32)&g_gpu_ot_end;` (const-first single statement) closes or lowers the floor
- mechanism: Operand order in the commutative AND could change cc1's pseudo staging for the addr register even in the fused single-statement form
- probe: Applied to src/display.c over the candidate base; sandbox func_8007B844 --disable all
- result: score 7 (regression from 6); statement fusion loses the named-intermediate pseudo regardless of operand order
- verdict: KILLED

## [s1] Lever B with the second statement's AND operands swapped (`mask = mask & (u32)&g_gpu_ot_end;`) changes the residual
- mechanism: Commutative-operand order at RTL gen could reassign which pseudo carries the addr vs the mask
- probe: Applied swap; sandbox func_8007B844 --disable all
- result: score 6 (identical to candidate); cc1 canonicalizes the AND, operand order inside Lever B is score-invariant
- verdict: KILLED

## [s1] A near-clone sibling elsewhere in the binary shares this body shape and its matched C reveals the lever
- mechanism: SOTN-style opcode-class Levenshtein duplicate scan surfaces cross-function analogs
- probe: tools/find_duplicates.py --threshold 0.75 over asm/funcs; grep pairs for 8007B844
- result: zero pairs involving func_8007B844; only informative sibling remains gpu_ClearOTag (already exploited in ledger)
- verdict: KILLED

## s2 (structural, 2026-07-21)

Baseline re-confirmed: candidate.c applied -> sandbox 6, build_insns 38.

## [s2] F1: struct-typed device-table dispatch (Gpu_dev->otc via faithful GpuDevice struct) changes the post-call pseudo landscape
- mechanism: RTL-gen pseudo allocation at the call boundary feeds RA conflicts and sched.c dependence chains; typed member access is the known original-source shape
- probe: GpuDevice struct (pad[11] + otc fn-ptr at 0x2C); measured typed-local form AND direct member-call form; sandbox each
- result: 6 and 6 — both identical to untyped v0[11] cast; no codegen change whatsoever
- verdict: KILLED

## [s2] Two-local const-first form with AND rebound into addr reproduces target's and-dest ($v1) and frees mask toward $a0
- mechanism: target's `and $v1,$v1,$a0` has dest = addr register; a 2-set addr pseudo receiving the AND could mirror it
- probe: `mask=0xFFFFFF; addr=(u32)&g_gpu_ot_end; addr=addr&mask; *ot=addr;` + decl-order swap variant; sandbox + objdump
- result: 7 both ways; stored pseudo always takes $v0, other local $v1; decl order RA-invariant
- verdict: KILLED

## [s2] Pseudo creation order/timing (block-local tail decls, hoisted dispatch decl) or mask signedness flips the RA/sched tiebreak
- mechanism: pseudo numbering feeds allocno ordering; block entry timing shifts numbering relative to the call boundary
- probe: tail wrapped in own block with mask decl inside (6); `u32 *dev;` at fn scope (6); `s32 mask` retype (6)
- result: all neutral at 6
- verdict: KILLED

## [s2] Duplicated-statement-into-arms ref-lift (dispatch+tail duplicated into debug arm, cross-jump re-merges byte-neutrally) lifts mask/ot ref counts and flips the RA cascade
- mechanism: doubled reg_n_refs before jump2 merge = the sanctioned byte-free priority lift (motion_SetMotion precedent)
- probe: full tail duplicated into the debug-if arm + shared copy; sandbox
- result: 22, build_insns 44 — cross-jump fails to re-merge (+6 insns), not byte-neutral, regression
- verdict: KILLED

## [s2] F1: struct-typed device-table dispatch (faithful GpuDevice struct, otc fn-ptr field at offset 0x2C) changes the post-call pseudo landscape vs the untyped v0[11] cast
- mechanism: RTL-gen pseudo allocation at the call boundary feeds RA conflicts and sched.c dependence chains; typed member access is the known original-source shape
- probe: Measured typed-local form (dev->otc(ot,n)) and direct member-call form (((GpuDevice*)g_gpu_dev_table)->otc(ot,n)); sandbox each
- result: 6 and 6 — both byte-identical to the untyped cast; dispatch surface type never reaches the cascade
- verdict: KILLED

## [s2] Two-local const-first form with the AND rebound into addr (mask=0xFFFFFF; addr=(u32)&g_gpu_ot_end; addr=addr&mask; *ot=addr) mirrors target's and-dest $v1 and frees mask toward $a0
- mechanism: Target's `and $v1,$v1,$a0` dest is the addr register; a 2-set addr pseudo receiving the AND could reproduce the dest-choice
- probe: Measured the form plus its decl-order-swapped variant; objdumped the sandbox .o to read register assignment
- result: 7 both ways; the STORED pseudo always seizes $v0 (first free in REG_ALLOC_ORDER across the tail), decl order is RA-invariant; mask-in-$a0 is unreachable without $v0 occupied across the tail (= the known copy-prop wall)
- verdict: KILLED

## [s2] Pseudo creation order/timing (block-local tail decl, fn-scope hoisted dispatch pointer decl) or mask signedness retype flips the RA/sched tiebreak
- mechanism: Pseudo numbering feeds allocno ordering; block-entry timing shifts numbering relative to the call boundary
- probe: Tail wrapped in own block with mask decl inside; u32 *dev hoisted to fn scope (gpu_SendPacket spelling); s32 mask retype; sandbox each
- result: 6 / 6 / 6 — all neutral; plateau insensitive to numbering, timing, and tree-type signedness
- verdict: KILLED

## [s2] Duplicated-statement-into-arms ref-lift (dispatch+tail duplicated into the debug arm, jump2 cross-jump re-merges byte-neutrally) lifts mask/ot reg_n_refs and flips the RA cascade
- mechanism: Doubled refs before post-RA cross-jump merge — the sanctioned byte-free priority lift (motion_SetMotion precedent)
- probe: Full dispatch+tail+return duplicated into the debug-if arm alongside the shared copy; sandbox
- result: 22, build_insns 44 vs target 38 — cross-jump fails to re-merge the duplicated suffix (+6 insns): not byte-neutral (fails the sanction prerequisite) and a large regression
- verdict: KILLED

## [s3] Splitting the debug-level load into a named u8 local (lvl = g_gpu_debug_level; if (lvl >= 2)) shifts pre-call pseudo numbering and perturbs the tail RA/sched cascade
- mechanism: Pre-call pseudo creation order feeds allocno ordering and sched.c dependence chains; the debug arm was the one body region s2 never touched
- probe: Applied to candidate base in src/display.c; sandbox func_8007B844 --disable all
- result: score 6 — byte-identical build; pre-call pseudo creation never reaches the tail cascade
- verdict: KILLED

## [s3] Re-associating the dispatch load as byte-offset arithmetic ((*(void(**)(u32*,s32))((u32)g_gpu_dev_table + 0x2C))(ot,n)) stages the call address through different pseudos than the v0[11] index form
- mechanism: Address-arithmetic surface shape could produce a different RTL chain at the call boundary feeding the post-call landscape
- probe: Applied to candidate base; sandbox
- result: score 6 — combine folds to the identical lw 0x2C($v0) dispatch; joins the s2 dispatch-surface-type kill
- verdict: KILLED

## [s3] goto-end tail (*ot = mask; goto end; end: return ot;) changes jump-opt/sched interaction at the function exit
- mechanism: shared-end-label family — label between store and return could affect scheduling regions
- probe: Applied to candidate base; sandbox
- result: score 6 — trivial goto folded before sched; single-exit function gains nothing from shared-end-label
- verdict: KILLED

## [s3] Two-local form with the AND folded into MASK (mask=0xFFFFFF; addr=(u32)&g_gpu_ot_end; mask=addr&mask; *ot=mask) — the one 2-local permutation s2 did not measure — moves the residual
- mechanism: s2's AND-into-addr variant scored 7; the AND-into-mask variant preserves Lever B's named-intermediate rebinding and could differ
- probe: Applied to candidate base; sandbox
- result: score 6 — addr copy-props into the lui/addiu chain; byte-identical to 3-statement Lever B. Refines the Lever B constraint: a named addr local is copy-prop-transparent iff the AND dest is mask
- verdict: KILLED

## [s3] Hoisting mask = 0xFFFFFF between the debug arm and the dispatch call (live across ONE call only, vs round-1's hoist across both = 19) reshapes the tail without full callee-save blowup
- mechanism: Constant materialization before the last call would change what sched.c sees at the post-call boundary
- probe: Applied to candidate base; sandbox
- result: score 20, build_insns 40 vs target 38 — mask forced into a callee-save reg across the dispatch call, prologue/epilogue +2 insns; can never match by construction
- verdict: KILLED

## s4 (permuter, 2026-07-21)

Baseline re-confirmed start+end: candidate.c applied -> sandbox 6, build_insns 38.

## [s4] A fresh-seed full-random basin on the Lever B chassis finds a sub-floor form the historical directed runs missed
- mechanism: random mutation pass (temp-for-expr, expr-expansion, statement moves) explores neighborhoods directed PERM runs never reach
- probe: campaign A (43k iters, ~28 min, -j4, --stack-diffs); wait windows + harvest
- result: only find = wrong-semantics store-to-global attractor at equal weighted score 135; no sub-135 form
- verdict: KILLED

## [s4] The two-local AND-into-mask chassis (different pseudo geometry) opens a random basin with a different attractor landscape
- mechanism: s3 showed the named addr local is copy-prop-transparent when the AND dest is mask; its pseudo landscape differs from Lever B and could randomize differently
- probe: campaign B (42k iters); novel find output-125-1 applied to src/display.c and sandbox-measured
- result: find = arg-staging + scalar tail rebind, permuter 135->125 but sandbox 6 NEUTRAL; both mutation families already known fold-transparent; no further finds in 41k subsequent iters
- verdict: KILLED

## [s4] Cross-products of individually-neutral levers (dispatch shape x AND operand order x exit shape) interact non-additively and move the residual
- mechanism: each lever alone is measured-neutral (s1-s3); PERM_GENERAL cross-product + PERM_RANDOMIZE tests all combinations, which hand-search never measured jointly
- probe: campaign C (36k iters over the 2x2x2 cross-product + random)
- result: no combination scored below 135; only output was the same wrong-semantics attractor as chassis A
- verdict: KILLED

## [s4] A fresh-seed full-random basin on the Lever B chassis finds a sub-floor form the historical directed runs missed
- mechanism: Random mutation pass (temp-for-expr, expr expansion, statement moves) explores neighborhoods directed PERM runs never reach
- probe: Campaign A: tmp/grind/func_8007B844/s4/perm_a, 43k iters ~28 min -j4 --stack-diffs, in-turn wait windows, harvest --stop
- result: Only output was a wrong-semantics attractor (*new_var = mask with new_var = &g_gpu_ot_end — stores to the global, not *ot) at equal weighted score 135; no sub-135 find
- verdict: KILLED

## [s4] The two-local AND-into-mask chassis (different pseudo geometry, s3-neutral) opens a random basin with a different attractor landscape containing a sub-floor form
- mechanism: Different pseudo geometry at the tail could let random mutations reach a schedule the Lever B basin cannot
- probe: Campaign B: tmp/grind/func_8007B844/s4/perm_b, 42k iters; novel find output-125-1 transcribed into src/display.c and sandbox-measured
- result: Find = `addr = n;` arg-staging + tail scalar rebind `addr = mask; *ot = addr;` — permuter weighted 135->125 but sandbox NEUTRAL at 6 (build_insns 38); the permuter metric diverges from the masked honest distance on this residual; no further finds in 41k subsequent iters; banked memory/grind/func_8007B844/rejected/permuter_s4_addr_arg_staging_rebind.c
- verdict: KILLED

## [s4] Cross-products of individually-neutral levers (dispatch shape x AND operand order x return-vs-goto-end exit) interact non-additively and lower the floor
- mechanism: Each lever alone measured neutral in s1-s3; joint combinations were never measured — PERM_GENERAL cross-product + PERM_RANDOMIZE enumerates them
- probe: Campaign C: tmp/grind/func_8007B844/s4/perm_c, directed 2x2x2 cross-product + randomize, 36k iters
- result: No combination scored below base 135; only output was the same wrong-semantics store-to-global attractor as chassis A
- verdict: KILLED

## s5 (permuter, 2026-07-21)

Baseline re-confirmed start+end: candidate.c applied -> sandbox 6, build_insns 38.

## [s5] A fresh-seed basin on the debug-split + goto-end combined chassis (both levers s3-neutral, AST never randomized) reaches mutation neighborhoods the leverB/twolocal basins cannot
- mechanism: the permuter mutates the AST it is given; a structurally different (score-equal) seed changes the reachable mutation space even when the seed's codegen is identical
- probe: campaign D tmp/grind/func_8007B844/s5/perm_d, 60,137 iters, wait windows in-turn, harvest --stop
- result: 3 finds, all score-equal 135 — one novel spelling (named fn-ptr dispatch temp, sandbox-measured NEUTRAL 6, banked rejected/) and two seed re-derivations; no sub-135
- verdict: KILLED

## [s5] The struct-typed GpuDevice member-access dispatch chassis (s2-neutral, member-access AST) opens a random basin with a different attractor landscape
- mechanism: member-access expression nodes give the mutation pass different expression-level material (field refs vs array index + cast) than any prior basin
- probe: campaign E tmp/grind/func_8007B844/s5/perm_e, 61,543 iters
- result: 2 late finds (1107s, 1484s), both score-equal 135 — a known fold-transparent ot-rebind, and the SAME wrong-semantics store-to-global attractor as s4 chassis A/C (third geometry to converge on it); no sub-135
- verdict: KILLED

## [s5] The debug-arm-shape x tail-geometry directed cross-product (the one joint axis s4's campaign C omitted) contains a non-additive interaction that moves the residual
- mechanism: PERM_GENERAL enumerates the joint cells; s3 measured each lever alone, s4 crossed dispatch x AND-order x exit but never debug-arm x tail
- probe: campaign F tmp/grind/func_8007B844/s5/perm_f, 50,788 iters; the never-measured combo cell (debug-split + two-local) applied to src and sandbox-measured
- result: all finds score-equal 135; the combo cell = sandbox 6 NEUTRAL (banked rejected/permuter_s5_debugsplit_twolocal_combo.c); one dead-temp-chain form vetted out as cheat family (dead local written, never read); no sub-135
- verdict: KILLED

## [s5] A fresh-seed basin on the debug-split + goto-end combined chassis (s3-neutral levers, AST never randomized) reaches mutation neighborhoods the leverB/twolocal basins cannot
- mechanism: The permuter mutates the AST it is given; a structurally different score-equal seed changes the reachable mutation space even when the seed's codegen is identical
- probe: Campaign D (tmp/grind/func_8007B844/s5/perm_d), 60,137 iters, in-turn wait windows, harvest --stop; novel find sandbox-measured in src
- result: 3 finds, all score-equal 135: one novel spelling (named fn-ptr dispatch temp) sandbox-measured NEUTRAL at 6 (banked rejected/permuter_s5_fnptr_dispatch_temp.c) and two seed re-derivations; no sub-135
- verdict: KILLED

## [s5] The struct-typed GpuDevice member-access dispatch chassis (s2-neutral) opens a random basin with a different attractor landscape
- mechanism: Member-access expression nodes give the mutation pass different expression-level material (field refs vs array-index + cast) than any prior basin
- probe: Campaign E (tmp/grind/func_8007B844/s5/perm_e), 61,543 iters; single typed extern handle (no asm-alias — the alias-rename spelling was rejected at design time per inline-asm-injection sibling-cheat family)
- result: 2 late finds, both score-equal 135: a known fold-transparent ot-rebind, and the SAME wrong-semantics store-to-global attractor as s4 — now reproduced in a third chassis geometry (3 of 6 lifetime chassis converge on it); no sub-135
- verdict: KILLED

## [s5] The debug-arm-shape x tail-geometry directed cross-product (the one joint axis s4's directed campaign omitted) contains a non-additive interaction that moves the residual
- mechanism: PERM_GENERAL enumerates the joint cells; s3 measured each lever alone, s4 crossed dispatch x AND-order x exit but never debug-arm x tail
- probe: Campaign F (tmp/grind/func_8007B844/s5/perm_f), 50,788 iters; the never-measured combo cell (debug-split + two-local AND-into-mask) applied to src and sandbox-measured
- result: All finds score-equal 135; the combo cell = sandbox 6 NEUTRAL (banked rejected/permuter_s5_debugsplit_twolocal_combo.c); one chained dead-temp form (dead local written, never read) vetted out as cheat family without measurement; no sub-135
- verdict: KILLED

## s6 (forensics, 2026-07-21)

Baseline re-confirmed start+end: candidate.c applied -> sandbox 6, build_insns 38.

## [s6] The residual is caused by a sched.c INSN_PRIORITY differential (return-staging priority 1 vs chain 4 — the inherited ledger model)
- mechanism: priority = chain depth to jr; flipping requires depth >= 4
- probe: instrumented cc1 (-da + BB2_SCHED_DEBUG/BB2_PRIO_DEBUG) on the score-6 form; read sched1+sched2 priority tables and dependence lists (tmp/grind/func_8007B844/s6/display.i.sched{,2})
- result: ALL six tail insns have EQUAL priority 3 in both passes; the order is forced by REG_DEP_ANTI edges (staging writes $v0 which the and/sw hold as the mask register) — a dependence-topology problem created by RA, not a priority tie
- verdict: KILLED

## [s6] The copy-prop wall has a nameable single gate in cse.c that the sandbox-0 form threads
- mechanism: make_regs_eqv (cse.c:853-858) promotes a copy dest to canonical only if its live range extends beyond the current cse basic block; an arm-block set provides the beyond-block first-use
- probe: dual-form instrumented dump (run_forms.py): form X = banked conditional-dead-store, form Y = single-set rebind; diff .rtl/.cse/.flow/.greg per pass
- result: Y's copy folds at cse1 exactly at the gate (uses rewritten to ot, copy deleted); X's copy survives, flow deletes the dead arm store, global.c assigns the copy $v0 + target rotation (74 in 2, 82 in 3, 84 in 4), emitted asm == target
- verdict: CONFIRMED

## [s6] A LIVE arm use (assignment-in-arg `p = ot` in the debug call) satisfies the beyond-block gate without a dead store and reaches the bytes
- mechanism: same cse gate, but the arm set's value feeds the call arg (live), avoiding the dead-store carve-out
- probe: measured 4 cells: Z Lever-B tail (6/38), Z2 fused X-tail (8/39), Z3 two-local AND-into-addr (8/39), Z5 copy-mid Lever-B (6/38); dumped Z/Z3/Z5 asm
- result: topology holds in Z/Z5 (store base = surviving copy) but the residual becomes a register rotation (mask $v1 vs $a0, addr $v0 vs $v1); fused/two-local variants lose $v0 (+1 insn). The live arm segment deflates p's allocno priority; target RA requires the arm store DELETED pre-RA (flow), i.e. dead. No live spelling reaches the bytes
- verdict: KILLED
