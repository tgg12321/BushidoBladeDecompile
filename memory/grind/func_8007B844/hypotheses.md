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
