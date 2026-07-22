# Hypothesis ledger — func_8007DC9C

Floor = 9 (verdict C, target 91 / build 90 insns). Gap decomposes into two
independent axes: Axis A = 1 combine-fold insertion (BF68[0]); Axis B = 8-op
sched1 reorder cluster in the first debug_printf setup.

## Live frontier — RE-MERGED by s19 synthesis (2026-07-22; supersedes s10)

s10 merged s1–s9. s19 re-merges ALL of s1–s18 into the strengthened dossier
tmp/grind/func_8007DC9C/s19/MERGED-ATTACK-s19.md (adds: permuter QUINTUPLE-
confirm ~118k iters s13/s14; forensics QUADRUPLE-run with combine.c:1458
SOURCE-verified + rule-completeness proof s15/s16; rederive control-flow leg
TRIPLY-confirmed dead s8/s17/s18). The original s1 frontier (H-A1, H-A2, H-B1)
is fully resolved — every one KILLED, do NOT re-propose. The frontier remains a
SINGLE escalation-ready item; s11–s18 empirically prove re-running any pure-C
modality only re-confirms the mechanism-pinned dead result (8 sessions, zero
movement). The ONLY unblock is the OWNER filing the escalation entry.

### RESOLVED (do NOT re-open) — s1 frontier, all KILLED
- **H-A1** (offset-0 combine fold decl/access lever) — KILLED s2/s3/s6/s9.
  Every decl/access shape folds; the 3-insn form is combine multi-use
  retention (combine.c:1458), needs a 2nd address use the function does not
  have. Cross-corpus prior-art (s9): a single pure offset-0 read ALWAYS folds.
- **H-A2** (BF68 real type = fn-ptr, adopt sibling decl) — KILLED s2 (scalar &
  fn-ptr reads both fold) + s7 (combine is per-function → sibling's use is
  mechanically invisible; the "wait for sibling" probe is impossible).
- **H-B1** (sched1 fmt-vs-deadread tie-break C lever) — KILLED s2/s4/s5, root
  CONFIRMED s6: priority is set by a volatile-MEM anti-dep (38→45), unflippable
  without changing observable volatile order (= cheat + semantically wrong).

### FRONTIER-1 (the only remaining item) — escalation-ready
- statement: func_8007DC9C is a fully-characterized ENDGAME-LOCK. Both residual
  axes are compiler-internal (axis A = combine multi-use retention; axis B =
  sched1 volatile-MEM anti-dep priority), orthogonal, and have NO legitimate
  pure-C lever single- OR cross-function across all five modalities.
- mechanism: see MERGED-ATTACK.md §Axis A / §Axis B — both pinned to exact
  compiler passes (combine.c:1458 added_sets_2 / sched.c INSN_PRIORITY via
  REG_DEP_ANTI 38→45). Matches [[endgame-lock-disposition-policy]] exactly:
  scan_hand_coded LOW (gate #1 refuse asm), no coercion precedent (gate #2
  refuse coercion) → keep 4 regfix rules, classify INCOMPLETE-owner-accepted.
- next_probe: OWNER files an OWNER-ESCALATION entry for func_8007DC9C in
  docs/grind/decisions.md (none exists as of s10). Once filed, a subsequent
  session emits `owner-gated` and the queue advances. No further pure-C
  modality remains to run — re-measuring any axis only re-confirms a
  mechanism-pinned dead result.

## Rejected forms — see evidence.md (imported WIP bank) + rejected/. Do NOT re-propose.

## [s1] Honest floor is 9 and the 9-op gap decomposes into 1 combine-fold insertion (D_8009BF68[0]) plus an 8-op sched1 reorder cluster in the first debug_printf setup.
- mechanism: canonical gate + cheat-invisible sandbox score, cross-read against asm/funcs/func_8007DC9C.s: target materializes BF68 address (3 insns) where our build folds (2), and target orders fmt-load before the BF78 chain where ours loads BF78 first.
- probe: canonical func_8007DC9C; sandbox func_8007DC9C --disable all; read asm/funcs/func_8007DC9C.s lines 23-53.
- result: canonical verdict C distance 9; sandbox score 9 (target_insns 91, build_insns 90, rules_dropped 4, cheat_asm_stripped 150). Axis A + Axis B confirmed as documented.
- verdict: CONFIRMED

## [s2] H-B1: a C statement-order/liveness lever flips the sched1 fmt-vs-deadread order so fmt schedules first (into $a0) and the dead *g_gpu_stat_reg read lands in $v0 (as target does).
- mechanism: .greg dump: dead read (insn 38) is volatile-ordered before the volatile D_8009BF7C read (insn 45), putting it on the critical path dead-read->BF7C->subu(arg1)->call. That gives it high sched1 PRIORITY (critical-path length), so it schedules early and grabs $a0; fmt (a lone `la`, chain length 1) schedules after and reloads $a0. Ordering is priority-driven, not LUID/source-order driven.
- probe: sandbox sweep of 8 structural forms: fmt-precompute-first, fmt-stmt-first, bare-(void)-read+fmt, arg1-temp, madr-temp, bf78-temp, new_var-declared-first, split-subtraction. Cross-checked against the greg dump insn allocation (insn 38 dead read -> reg/v 4 a0, REG_UNUSED; insn 60 fmt la later).
- result: All fmt/arg-precompute/decl-order forms scored 9 (no schedule change); split-subtraction 12; chcr-cache 19. The volatile ordering dead-read<BF7C matches target and is unswappable (observable), so the dead read stays high-priority regardless of source order.
- verdict: KILLED

## [s2] H-A1: a declaration/access shape suppresses the combine offset-0 symbol+0 fold so D_8009BF68[0] emits target's 3-insn materialized-address load (la sym; lw 0(reg)).
- mechanism: Our combine dump represents the access as (mem/s:SI (symbol_ref D_8009BF68)) -> folded to 2-insn lui;lw %lo. Target keeps the address in a register (la=lui+addiu) then lw 0(reg); combine only preserves (set reg sym)(mem reg) unmerged when the address pseudo has MULTIPLE uses (it will not propagate a multi-use reg into the mem). func_8007DC9C has exactly one use of &D_8009BF68, so combine always folds.
- probe: sandbox: array[0] (baseline), sized array[][1] 2D, scalar s32, scalar fn-ptr, s32* pointer-deref, local-base pointer precompute, one-array (BF6C/BF70 as bf68[1]/[2]).
- result: All fold to the 2-insn form (score 9) EXCEPT s32* pointer-deref, which gives build_insns=91 (target count!) but the middle insn is `lw` (load pointer value) where target has `addiu` (materialize address) -> score 12. So BF68 is not a pointer global, and no decl shape materializes the address for a single-use offset-0 access.
- verdict: KILLED

## [s2] H-A2: declaring/accessing D_8009BF68 consistent with its true scalar function-pointer type (per sibling func_8007D3F8, line 872) emits the materialized non-folded load.
- mechanism: A scalar (non-array) symbol read presents a different address RTL to combine than array[0].
- probe: sandbox: extern s32 (*D_8009BF68)(s32*,s32) with the value passed directly as the printf arg; also plain extern s32 D_8009BF68 scalar.
- result: Both scalar forms fold to lui;lw %lo (2 insns, score 9) — a scalar read never materializes the address. Same fold as the array form. H-A2 does not produce the 3-insn shape.
- verdict: KILLED

## [s3] Reading BF68/BF6C/BF70 through a single struct pointer at &D_8009BF68 gives the base a multi-use address pseudo, so combine keeps it materialized (la; lw 0(reg)) as target does for BF68 (combine.c:1458 added_sets_2 retention, same as func_8007EDBC).
- mechanism: combine keeps def + 2nd-use insn only for a multi-use address; a shared struct base gives the base pseudo >1 use.
- probe: sandbox --disable all with debug_printf(&D_80016044, ((struct{s32 a,b,c;}*)&D_8009BF68)->a, ->b, ->c).
- result: score 11 (worse than floor 9), build_insns 90. Shared base emits ONE materialized base + offset loads (lw 0/4/8(base)); target reads BF6C/BF70 as SEPARATE folded %lo(D_8009BF6C)/%lo(D_8009BF70) scalar symbols.
- verdict: KILLED

## [s3] The original func_8007DC9C had a second visible &D_8009BF68 reference (address compare/store) that pure-C would emit, forcing the materialized load target has (s2/frontier H-A3 probe).
- mechanism: A genuine second in-function use of the address pseudo defeats the combine offset-0 fold.
- probe: Read all 96 lines of asm/funcs/func_8007DC9C.s and enumerate every D_8009BF68 reference in the emitted target.
- result: Target references &D_8009BF68 EXACTLY ONCE (lines 43-45). No second visible reference exists. The materialization is a combine-time multi-use retention of a use that a LATER pass DCE'd from output — not a visible second reference. Any single-function pure-C reproduction would be a dead second use of &D_8009BF68 = a forbidden coercion (no semantic purpose, dead in output, justified only by combine internals).
- verdict: KILLED

## [s4] H-B1 (permuter modality): a directed/random permuter over the first debug_printf arg-setup region finds a pure-C statement-order/liveness form that schedules fmt ahead of the dead *g_gpu_stat_reg read (target byte order), closing the 8-op axis-B cluster.
- mechanism: axis B is a sched1 critical-path tie — the dead read is volatile-ordered before the volatile D_8009BF7C read, giving it fixed high priority so it schedules early into $a0; target instead loads fmt first (dead read -> $v0). Permuter random reordering / arg-homing chassis explores the statement-order space the s2 structural enumeration may have missed.
- probe: two campaigns (standalone single-fn workspace, base_score 630). Chassis-1 random (20,157 iters). Chassis-2 arg-homing AST with named `diff` temp (7,137 iters). ~27.3k iters total.
- result: NO legitimate sub-baseline find. Only finds: `extern volatile [long|int] D_8009BF68[]` (volatile-coercion cheat, axis A only) and a pointer-alias/long-long-holder junk form (cheat-by-spelling). The axis-B cluster never reordered in any legitimate form. Corroborates s2: volatile ordering is observable and unswappable; no pure-C reorder reaches target's schedule.
- verdict: KILLED

## [s4] H-A1/H-A2 (permuter modality): permuter finds a pure-C declaration/access form for D_8009BF68[0] that emits target's 3-insn materialized address.
- mechanism: combine offset-0 fold; only a multi-use address pseudo survives (s2/s3).
- probe: same two campaigns.
- result: the ONLY axis-A closing form the permuter found is `extern volatile <T> D_8009BF68[]` — volatile coercion (forbidden, stripped by volatile_cheats). No non-volatile pure-C form materialized the address. Confirms s3: axis A has no legitimate single-function pure-C lever (any is a dead-second-use/volatile coercion); closes only via the cross-TU sibling func_8007D3F8.
- verdict: KILLED

## [s4] A directed/random permuter over the first debug_printf arg-setup region finds a pure-C statement-order/liveness form that schedules fmt ahead of the dead *g_gpu_stat_reg read (target byte order), closing the 8-op axis-B cluster.
- mechanism: Axis B is a sched1 critical-path tie: the dead *g_gpu_stat_reg read is volatile-ordered before the volatile D_8009BF7C read, giving it fixed high priority so it schedules early into $a0; target loads fmt first (dead read -> $v0). Permuter random reordering + an arg-homing chassis explore the statement-order space.
- probe: Two campaigns on a validated standalone single-function workspace (base_score 630): chassis-1 random (20,157 iters, ~12 min) + chassis-2 arg-homing AST with a named diff temp (7,137 iters, ~5 min). ~27.3k iters total; every campaign harvested + stopped in-turn.
- result: No legitimate sub-baseline find. Only finds: extern volatile [long|int] D_8009BF68[] (volatile-coercion cheat, axis A only, stripped by volatile_cheats) and a pointer-alias/long-long-holder junk form (cheat-by-spelling). The axis-B cluster never reordered in any legitimate form.
- verdict: KILLED

## [s4] The permuter finds a pure-C declaration/access form for D_8009BF68[0] that emits target's 3-insn materialized address (closing axis A).
- mechanism: combine offset-0 fold on (mem (plus symbol_ref 0)); only a multi-use address pseudo survives unfolded (combine.c:1458 added_sets_2).
- probe: Same two campaigns; inspected every sub-630 find.
- result: The only axis-A closing form found is extern volatile <T> D_8009BF68[] (volatile coercion, forbidden). No non-volatile pure-C form materialized the address. Confirms s3: axis A has no legitimate single-function pure-C lever.
- verdict: KILLED

## [s5] A structurally-distinct permuter chassis (whole-body named-temp topology; different register-pressure regime than s4's default-random / arg-homing) finds a legitimate pure-C form closing axis A or axis B.
- mechanism: Named temps for every non-volatile value (fmt1/fmt2, diff, arg_a/b/c) maximize randomizer reorder freedom and vary live-ranges feeding sched1 priority, WITHOUT touching the volatile-ordered reads. Base validated at 90 insns reproducing both ledger axes exactly.
- probe: permuter_campaign chassis-3 (s5-chassis3-wholebody-temps, -j8, base 630), 40,054 iters / ~27 min, three in-turn wait windows, harvested --stop in-turn.
- result: NO legitimate sub-baseline find. Only output = extern unsigned long long D_8009BF68[] at score 630 == floor 9 (NON-improving width-coercion cheat, same family as s4). Axis-B cluster never reordered; chassis-3 did not even hit s4's volatile cheat (different region, still nothing legitimate). Permuter TRIPLE-confirmed dead across 3 chassis / ~67k iters.
- verdict: KILLED

## [s5] A structurally-different permuter chassis (whole-body named-temp topology giving the randomizer maximal reorder freedom over the non-volatile surface under a different register-pressure regime than s4's default-random or single-diff arg-homing) finds a legitimate pure-C form that closes axis A (BF68 materialization) or axis B (8-op sched1 reorder).
- mechanism: Named temps for every non-volatile value (fmt1/fmt2 pointers, diff, arg_a/b/c second-printf reads) alter live-range/register pressure feeding sched1 priority; the two volatile *g_gpu_stat_reg reads and the volatile D_8009BF7C read stay inline+ordered (collapsing them is the banked score-19 cheat). base validated at 90 insns reproducing exactly both ledger axes.
- probe: permuter_campaign chassis-3 s5-chassis3-wholebody-temps (-j8, base_score 630), 40,054 iterations over ~27 min across three in-turn wait windows; harvested --stop in-turn.
- result: No legitimate sub-baseline find. The ONLY output (output-630-1) is at score 630 == baseline floor 9 (NO improvement) and is a width coercion: extern unsigned long long D_8009BF68[] to fake the axis-A materialization bytes - same coercion cheat family as s4's volatile-BF68 / long-long-holder junk. Axis-B reorder cluster never moved; chassis-3 did not even stumble into the volatile cheat s4 found (explored a genuinely different region). Floor held at 9 the entire campaign.
- verdict: KILLED

## [s6] The axis-B 8-op sched cluster order (dead *g_gpu_stat_reg read emitted before the fmt load) is fixed by sched1 INSN_PRIORITY, and that priority difference is derived from a volatile-MEM anti-dependency edge, not a LUID/source-order tie-break.
- mechanism: In display.i.sched BB2, dead-read insn 38 (mem/v *g_gpu_stat_reg) has priority=2, fmt insn 60 (la g_str_gpu_timeout, non-mem) has priority=1. GCC 2.7.2 sched.c priority = critical-path height to block end. insn 38 gains height 2 from the volatile anti-dep chain 38->45->...->call68 (REG_DEP_ANTI 38 present on volatile reads 45=D_8009BF7C and 64=2nd *stat, and on call 68) — an edge sched_analyze adds ONLY because both MEMs are mem/v. fmt (insn 60) feeds only the call so its height is 1. The backward list scheduler commits the higher-priority insn 38 earlier in the block (emitted order 36,54,38,56,60), and greg then assigns the dead read to $a0 (our wrong order). Target needs fmt before the dead read; raising fmt priority is structurally impossible (a la with a single downstream consumer), lowering the dead read's priority requires deleting the volatile anti-dep = making the read non-volatile/dropping it = a cheat.
- probe: Instrumented cc1 -da -dr over src/display.c; read exact sched1 priority values + REG_DEP_ANTI edges + the T-42..T-46 ready-list decisions for the fmt-LUI vs BF78/deadread nodes in BB2 (tmp/grind/func_8007DC9C/s6/display.i.sched lines 15356-15911; analysis in s6/FORENSICS.md).
- result: priority(38 dead read)=2, priority(60 fmt)=1; the +1 is exactly the volatile-MEM anti-dep 38->45. At T-42 ready list {60(1),38(2)} the scheduler prefers 38 (blocked 1 cyc for load hazard, so 60 fills that slot) then commits 38 earlier overall -> our dead-read-before-fmt order. No pure-C construct can flip it without altering observable volatile order.
- verdict: CONFIRMED

## [s6] Axis A (BF68[0] materialization) is a combine offset-0 symbol fold that only a multi-use address pseudo survives; single-use in-function means it always folds to the 2-insn form.
- mechanism: display.i.combine shows D_8009BF68[0] already reduced to (mem/s:SI (symbol_ref D_8009BF68)) with no (plus ... 0) — the const_int-0 displacement is simplified before combine's substitution, so it emits folded lui;lw %lo (2 insns). Target's 3-insn materialized lui;addiu;lw 0(reg) is combine.c:1458 added_sets_2 multi-use retention; s3's full-asm read proved exactly ONE &BF68 reference in-function, so the surviving 2nd use is cross-TU (sibling func_8007D3F8).
- probe: Read the BF68/BF6C access RTL in tmp/grind/func_8007DC9C/s6/display.i.combine within the func region.
- result: BF68 => (mem/s (symbol_ref D_8009BF68)), BF6C => (mem (symbol_ref D_8009BF6C)); both symbol-direct/folded. Confirms combine single-use fold. Closes only via the cross-TU sibling's genuine 2nd use.
- verdict: CONFIRMED

## [s7] Axis A's 3-insn BF68 materialization can be closed by matching the cross-TU sibling func_8007D3F8 (its genuine 2nd use of &D_8009BF68 supplies combine's multi-use retention).
- mechanism: claimed combine multi-use retention (combine.c:1458 added_sets_2) fed by a second &D_8009BF68 use somewhere in the TU.
- probe: (1) read the combine call site in tools/gcc-2.7.2/toplev.c + combine.c to establish combine's scope; (2) empirically add a 2nd intra-function use of &D_8009BF68 and objdump the sandbox .o.
- result: combine_instructions runs PER-FUNCTION (toplev.c:3004, inside rest_of_compilation; combine.c:453 walks only the passed chain) — a sibling's use is invisible to func_8007DC9C's combine. The 2nd use MUST be intra-function: a dead store `D_8009BF88=(s32)&D_8009BF68[0]` reproduced target's exact lui;addiu;lw 0(v0) form (objdump 1e84-1e90 == asm 43-45), sandbox 9->8/build 93, but is a coercion (dead store, persists undeleted, no semantic purpose).
- verdict: KILLED (sibling avenue mechanically impossible; only intra-function reproduction is a cheat).

## [s7] Axis A's 3-insn BF68[0] materialization closes only via the cross-TU sibling func_8007D3F8's genuine second use of &D_8009BF68 (combine multi-use retention).
- mechanism: Claimed combine.c:1458 added_sets_2 multi-use retention fed by a second &D_8009BF68 use elsewhere in the TU. Refuted: combine_instructions runs PER-FUNCTION (tools/gcc-2.7.2/toplev.c:3004 combine_instructions(insns, max_reg_num()) inside rest_of_compilation; def combine.c:453 walks only the passed insn chain) -> zero cross-function visibility. A use in a sibling function cannot appear in func_8007DC9C's RTL.
- probe: Read the combine call site (toplev.c) + combine.c scope; then empirically add a 2nd intra-function use `D_8009BF88=(s32)&D_8009BF68[0];` and objdump the sandbox .o.
- result: Per-function combine proven from source + s6 combine dump (sibling's `sw $19,D_8009BF68` sits in its own combine section, display.s:2693, separate from func_8007DC9C's folded read display.s:3170). Empirically, an intra-function 2nd use reproduces target's exact `lui v0,%hi; addiu v0,v0,%lo; lw a1,0(v0)` (objdump 1e84-1e90 == asm/funcs/func_8007DC9C.s:43-45), sandbox 9->8, build 90->93, but is a coercion (dead store, undeleted, no semantic purpose).
- verdict: KILLED

## [s8] REDERIVE: a fresh m2c decompile, a Kengo/sibling transplant, or a structurally-different control-flow shape (comma-expr dissolved into goto/nested-if) produces a C body that closes axis A or axis B.
- mechanism: A different overall structure could present different RTL to combine (axis A fold) or sched1 (axis B priority) than the forms structural/permuter explored within the fixed comma-expression body.
- probe: (1) fresh m2c tools/m2c/m2c.py -f func_8007DC9C; (2) grep Kengo corpus for a GPU-timeout/vsync reporter donor; (3) apply a goto-based early-return-guard restructure (short-circuit preserved) and sandbox it.
- result: (1) m2c reproduces HEAD's structure (scalar BF68 value-read that folds; dead stat-read DCE'd). (2) Kengo has only the callee motion_make_table + VSync, no reporter donor. (3) goto/nested-if restructure -> score 9, target 91 / build 90, rules_dropped 4, cheat_asm_stripped 150 = identical fingerprint. Axis A is per-expression (combine fold independent of control flow); axis B is volatile-order-locked (sched priority independent of block structure). Neither moved.
- verdict: KILLED

## [s8] A fresh m2c decompile, a Kengo/sibling transplant, or a structurally-different control-flow shape (comma-expr dissolved into goto/nested-if) produces a C body that closes axis A (BF68[0] combine fold) or axis B (sched1 volatile-MEM anti-dep priority).
- mechanism: A different overall structure could present different RTL to combine (axis A) or sched1 (axis B) than the forms structural/permuter explored inside the fixed comma-expression body. Refuted: axis A is per-expression (combine offset-0 fold is independent of surrounding control flow) and axis B priority is set by volatile ordering (independent of block structure).
- probe: (1) fresh m2c tools/m2c/m2c.py --valid-syntax --target mipsel-gcc-c -f func_8007DC9C; (2) grep Kengo corpus for a GPU-timeout/vsync reporter donor; (3) apply a goto-based early-return-guard restructure preserving the || short-circuit and sandbox --disable all.
- result: (1) m2c reproduces HEAD's structure (scalar BF68 value-read that folds; dead *g_gpu_stat_reg read DCE'd = the banked score-19 collapse). (2) Kengo corpus has only the callee motion_make_table + VSync/VSync2/SetVSyncFlag, NO reporter donor. (3) goto/nested-if restructure -> score 9, target 91 / build 90, rules_dropped 4, cheat_asm_stripped 150 = identical fingerprint to every prior form. Neither axis moved.
- verdict: KILLED

## [s9] REDERIVE (decomp.me-corpus leg): the decomp.me corpus (3754 GCC-2.7.2 PSX scratches) contains a matching pure-C scratch whose single pure offset-0 read of a word global materializes as target's 3-insn address (axis A), or a reporter donor whose C shape flips the axis-B schedule.
- mechanism: prior-art mining -- a known-good scratch might exhibit a declaration/access shape that materializes a single-use offset-0 read without a 2nd use (axis A), or a volatile-read + la-fmt ordering donor (axis B). This is the rederive leg s8 did not run (s8 = m2c + Kengo + control-flow).
- probe: cached corpus tmp/decomp_me_corpus/ (curl_cffi absent -> offline). Regex-validated STRICT search (addiu R,R,%lo(SYM) immediately -> offset-0 load 0(R), no intervening index math) = 541 hits / 25 in matching scratches; each classified by its C source. Keyword donor search for vsync/gpu/timeout/debug reporter.
- result: all 25 matching axis-A materializations are genuine C-source MULTI-USE (RMW load+store at [0], struct multi-field, struct-member-as-index, matrix copy, repeated reads); NONE is a single pure word-read at offset-0. func_8007DC9C's read is a single pure rvalue -> folds; materialization needs a 2nd use = coercion single-function (s7). Axis B: no corpus lever possible (frozen sched mechanism, s6; control-flow-insensitive, s8); no GPU-timeout reporter donor exists (consistent with s8 Kengo).
- verdict: KILLED

## [s9] The decomp.me corpus (3754 GCC-2.7.2 PSX scratches, the un-run rederive leg s8 skipped) contains a matching pure-C scratch whose SINGLE pure offset-0 read of a word global materializes as target's 3-insn lui;addiu R,R,%lo(SYM);lw X,0(R) (axis A), or a GPU-timeout reporter donor whose C shape flips the axis-B schedule.
- mechanism: Prior-art mining: a known-good scratch might exhibit a declaration/access shape that materializes a single-use offset-0 read without a 2nd use (axis A combine fold), or a volatile-read + la-fmt ordering donor (axis B sched priority). Cached corpus at tmp/decomp_me_corpus/ carries both source_code and target_assembly per scratch.
- probe: Offline (curl_cffi absent). Regex-validated STRICT search for 'addiu R,R,%lo(SYM)' immediately followed by an offset-0 load 0x0(R) with no intervening index math = 541 hits / 25 in MATCHING scratches; classified all 25 by their C source. Keyword donor search for vsync/gpu/timeout/debug reporter analogues.
- result: All 25 matching axis-A materializations are genuine C-source MULTI-USE: RMW at [0] (D_800AF9D8[0]|=0x8000 / &=0x7FFF / D_80097C40[0]++ = load+store, 2 addr uses), struct multi-field (BtlDrawFlag.fade/.chr, g_Pad.m_Down), struct-member-as-index (D_800B2384.bankIndex x7), matrix copy (cameraMat=D_800AFA64), or repeated reads (D_800AF93A[0] x2). NONE is a single pure word-read at offset-0. func_8007DC9C reads D_8009BF68[0] as a single pure rvalue (s3: one target reference) so it folds to 2-insn; the 3-insn form only ever comes from a genuine 2nd use = coercion single-function (s7). Axis B: no corpus lever possible (frozen sched mechanism s6, control-flow-insensitive s8); no GPU-timeout reporter donor exists (consistent with s8 Kengo).
- verdict: KILLED

## [s10] The 9-op gap is two INDEPENDENT compiler-internal axes with no possible joint pure-C lever (a single construct cannot close both).
- mechanism: Axis A is a per-EXPRESSION combine offset-0 fold (control-flow-insensitive, s8); axis B is a whole-block sched1 priority tie set by volatile ordering (expression/decl-shape-insensitive, s2/s6). Each proven insensitive to the other's lever surface, so orthogonal.
- probe: s10 synthesis cross-read of s1-s9 evidence + hypotheses; no re-run needed (mechanism-pinned).
- result: Orthogonality confirmed; the two axes must be closed independently and each is independently dead. Merged into tmp/grind/func_8007DC9C/s10/MERGED-ATTACK.md.
- verdict: CONFIRMED

## [s10] Axis A (3-insn D_8009BF68[0] materialization) has no legitimate pure-C lever single- OR cross-function.
- mechanism: combine.c:1458 added_sets_2 multi-use retention keeps the 3-insn form only when the address pseudo has >=2 uses; combine_instructions runs per-function (toplev.c:3004) so a sibling use is invisible. Function has exactly one &D_8009BF68 ref (s3); any single-function 2nd use is dead-in-output = coercion (s7). Cross-corpus prior-art (s9): every offset-0 materialization traces to a genuine C multi-use, never a single pure read.
- probe: Consolidated s2 (decl/access shapes fold) + s3 (full-asm one-ref, struct-triple KILLED) + s6 (combine dump) + s7 (per-function proof + empirical dead-store) + s9 (3754-scratch corpus classification).
- result: All decl/access shapes fold to 2-insn; only reproduction is a forbidden dead 2nd use; sibling avenue mechanically impossible.
- verdict: KILLED

## [s10] Axis B (8-op fmt-vs-deadread sched cluster) has no pure-C lever.
- mechanism: sched.c INSN_PRIORITY: dead-read insn 38 (mem/v) priority=2 vs fmt insn 60 priority=1; the +1 is the volatile-MEM anti-dep REG_DEP_ANTI 38->45 (added only because both are volatile). Flipping it requires deleting the anti-dep = non-volatile/dropped read = cheat + semantically wrong (target keeps two volatile reads).
- probe: Consolidated s2 (8 order/liveness variants all 9), s4/s5 (permuter x3 ~67k iters never reorder), s6 (sched dump root), s8 (control-flow-insensitive), s9 (no corpus donor).
- result: Priority is volatile-order-derived and control-flow/expression-insensitive; unflippable in legitimate pure C.
- verdict: KILLED

## [s11] Some structural lever not yet in the rejected bank (block-local var split, declaration order, type narrowing, or statement re-association) moves axis A (BF68[0] 3-insn materialization) or axis B (fmt-vs-deadread 8-op sched cluster).
- mechanism: Axis A = combine.c:1458 added_sets_2 multi-use retention; the offset-0 access folds to 2-insn because it is a single pure rvalue (s3: exactly one target &BF68 ref) and the const-0 displacement is simplified pre-substitution. Per-EXPRESSION fold => control-flow/decl-order insensitive (s8). Axis B = sched.c INSN_PRIORITY: dead-read insn 38 (mem/v) priority=2 > fmt insn 60 priority=1, the +1 being the volatile-MEM anti-dep REG_DEP_ANTI 38->45. Whole-block priority tie => expression/decl-shape insensitive (s2/s6).
- probe: Enumerated the full structural lever surface against the imported rejected bank + rejected/ forms + the s6/s7 mechanism proofs; re-ran sandbox --disable all baseline; confirmed src/display.c clean at HEAD (candidate byte-equivalent). Did NOT re-run banked rejected forms (forbidden).
- result: Every structural lever is already banked or mechanism-excluded: block-local var splits (s2 block-scoped new_var), declaration order (s2 8-form sweep, all 9), type narrowing (s2 scalar/fn-ptr/pointer fold; s4/s5 volatile/long-long/u16 = stripped coercions), statement re-association (s2 order sweep; s8 goto/nested-if byte-identical), struct/array grouping (s3 struct-triple -> 11). Baseline this session = score 9, target 91 / build 90, rules_dropped 4, cheat_asm_stripped 150 (identical fingerprint). No un-banked structural form exists; both axes provably insensitive to structural shape.
- verdict: KILLED

## [s12] Some structural lever (block-local var split, declaration order, type narrowing, statement re-association, or struct/array grouping) not already banked moves axis A (BF68[0] 3-insn materialization) or axis B (fmt-vs-deadread 8-op sched cluster).
- mechanism: Axis A = combine.c:1458 added_sets_2 multi-use retention; the offset-0 read is (mem/s (symbol_ref D_8009BF68)) with the const-0 displacement simplified pre-substitution, so a single pure rvalue read folds to 2-insn. Per-EXPRESSION => control-flow/decl-order/type-shape insensitive (s2/s3/s6/s8). Axis B = sched.c INSN_PRIORITY: dead-read insn 38 (mem/v) priority=2 > fmt insn 60 priority=1 via volatile-MEM anti-dep REG_DEP_ANTI 38->45 (s6); whole-block priority tie => expression/decl-shape/control-flow insensitive.
- probe: Re-ran sandbox --disable all baseline (score 9); enumerated the full structural lever surface against the imported rejected bank + rejected/ forms + the s6/s7 compiler-source mechanism proofs; confirmed src/display.c clean at HEAD (candidate byte-equivalent). Did NOT re-run banked rejected forms.
- result: Baseline score 9, target_insns 91, build_insns 90, rules_dropped 4, cheat_asm_stripped 150 (identical fingerprint to s1-s11). Every structural category already banked or mechanism-excluded: block-local splits (s2), decl order (s2 8-form sweep), type narrowing (s2 scalar/fn-ptr/pointer fold; s4/s5 volatile/ll/u16 stripped coercions), statement re-association (s2 order sweep; s8 goto/nested-if byte-identical), struct/array grouping (s3 struct-triple -> 11). No un-banked structural form exists; both axes provably insensitive to structural shape.
- verdict: KILLED

## [s13] A 4th structurally-distinct permuter chassis (tail-temp topology: tail-store values AND second-printf args all hoisted to top-computed named temps, a higher-register-pressure regime with base_score 695 vs prior chassis 630) finds a legitimate pure-C form closing axis A (BF68[0] 3-insn materialization) or axis B (8-op fmt-vs-deadread sched1 cluster).
- mechanism: Maximal reorder freedom over the whole tail under a live-range/register-pressure regime distinct from s4 chassis-1 (default-random), s4 chassis-2 (arg-homing single temp), s5 chassis-3 (whole-body named-temp). Volatile reads kept inline+ordered so the axis-B volatile-MEM anti-dep (REG_DEP_ANTI 38->45, s6) is preserved. base compiles to 90 insns / target 91, reproducing both ledger axes.
- probe: permuter_campaign chassis-4 (s13-chassis4-tail-temps, -j8, base_score 695), 30,817 iterations across in-turn wait windows; inspected every novel output; harvested --stop in-turn.
- result: No legitimate sub-floor find. Legitimate reordering finds ceiling at 655-680, ALL ABOVE the 630 floor (chassis-4 never even recovers the true floor legitimately). Only sub-floor find (590) = `extern volatile int D_8009BF68[]` = the banked volatile-BF68 coercion (axis A only), stripped by engine.volatile_cheats -> scores 9 under sandbox. 655 = `(long long)D_8009BF70` width coercion (banked family, non-improving); 635 = `new_var2=-1` dead constant-holder (cheat, non-improving). Axis B never legitimately reordered. Permuter now QUADRUPLE-confirmed dead across 4 chassis (~98k iters s4+s5+s13); only permuter-reachable sub-floor form is the banked volatile coercion (axis A); axis B is volatile-order-locked.
- verdict: KILLED

## [s13] A 4th structurally-distinct permuter chassis (tail-store values AND second-printf args all hoisted to top-computed named temps, a higher-register-pressure regime with base_score 695 vs prior chassis 630) finds a legitimate pure-C form closing axis A (BF68[0] 3-insn materialization) or axis B (8-op fmt-vs-deadread sched1 cluster).
- mechanism: Maximal reorder freedom over the entire tail under a different live-range/register-pressure regime than s4 chassis-1 (default-random), s4 chassis-2 (arg-homing single temp), s5 chassis-3 (whole-body named-temp). Volatile reads kept inline+ordered so the axis-B volatile-MEM anti-dep is preserved. base compiles to 90 insns / target 91, reproducing both ledger axes.
- probe: permuter_campaign launch --func func_8007DC9C --dir tmp/perm_dc9c_s13 --label s13-chassis4-tail-temps -j8; 30,817 iterations across in-turn wait windows; inspected every novel output; harvest --stop in-turn.
- result: No legitimate sub-floor find. Legitimate reordering/re-inlining finds ceiling at 655-680, ALL ABOVE the 630 floor (chassis-4 never even recovers the true floor via legitimate reordering). The only sub-floor find (590) is `extern volatile int D_8009BF68[]` = the banked volatile-BF68 coercion (axis A only), stripped by engine.volatile_cheats -> scores 9 under sandbox --disable all. Other sub-basin finds are coercions too: 655 = `(long long)D_8009BF70` width coercion, 635 = `new_var2=-1` constant-holder. Axis B (fmt-vs-deadread sched cluster) never legitimately reordered.
- verdict: KILLED

## [s14] A 5th structurally-distinct permuter chassis (low-register-pressure block-scoped topology: first-printf diverging pieces — fmt/dead-read/BF78-subtraction — as tightly block-scoped temps, 2nd-printf args left INLINE; the OPPOSITE pressure regime to s5 chassis-3 / s13 chassis-4 which hoisted the 2nd-printf args) finds a legitimate pure-C form closing axis A (BF68[0] 3-insn materialization) or axis B (8-op fmt-vs-deadread sched1 cluster).
- mechanism: A distinct live-range/register-pressure regime feeding sched1 priority (axis B) and combine (axis A) than all 4 prior chassis. Volatile reads kept inline+ordered so the axis-B volatile-MEM anti-dep (REG_DEP_ANTI 38->45, s6) is preserved. base_score 695, compiles to 90 insns / target 91, reproduces both ledger axes.
- probe: permuter_campaign chassis-5 (s14-chassis5-lowpressure-blockscope, -j8, base_score 695), 20,344 iterations across in-turn wait windows; inspected every novel output incl. both sub-floor finds; harvested --stop in-turn (alive=False, active=False, 0 live campaigns, 0 orphan permuter procs). Also confirmed tools/permuter_annotate.py is INAPPLICABLE (its hint catalog — register-asm-pins / shared-end-label / loop-rotation-two-shift / loop-counter-fills-load-delay — covers none of func_8007DC9C's diverging region; axis A is a combine offset-0 fold, axis B a volatile-MEM sched-priority tie).
- result: No legitimate sub-floor find. Legitimate reordering ceiling = 635-695, ALL ABOVE the 630 floor (chassis-5 never even recovers the true floor legitimately; output-630-1 ties the floor via a null 0x800-constant-hoist reshuffle, non-improving). The ONLY two strictly-sub-floor finds are BOTH cheats: output-500-1 (500) = corrupted fmt pointer (passes g_gpu_dma_madr where target passes &g_str_gpu_timeout — masked-Levenshtein false-match) + char width-coercion + dead pointer-alias self-assign (banked permuter-alias-longlong-junk family; rejected/permuter-s14-fmt-corrupt-char-narrow-junk.c); output-580-1 (580) = extern volatile int D_8009BF68[] = the banked volatile-BF68 coercion (axis A only; rejected/axisA-permuter-volatile-bf68.c; stripped by engine.volatile_cheats). output-635-1 = `unsigned short new_var=-1; return new_var` u16 constant-holder cheat (banked family, non-improving). Axis A never legitimately materialized (BF68[0] stays inline combine-folded in every legitimate find); axis B 8-op sched cluster never legitimately reordered. Permuter now QUINTUPLE-confirmed dead across 5 structurally-distinct chassis (~118k total iters: s4 27k + s5 40k + s13 31k + s14 20k).
- verdict: KILLED

## [s14] A 5th structurally-distinct permuter chassis (low-register-pressure block-scoped topology: first-printf diverging pieces fmt/dead-read/BF78-subtraction as tightly block-scoped temps, 2nd-printf args left INLINE — the OPPOSITE pressure regime to s5 chassis-3 / s13 chassis-4) finds a legitimate pure-C form closing axis A (BF68[0] 3-insn materialization) or axis B (8-op fmt-vs-deadread sched1 cluster).
- mechanism: A distinct live-range/register-pressure regime feeding sched1 INSN_PRIORITY (axis B) and combine (axis A) than all 4 prior chassis; volatile reads kept inline+ordered so the axis-B volatile-MEM anti-dep REG_DEP_ANTI 38->45 (s6 root) is preserved. base_score 695, 90 insns / target 91, reproduces both ledger axes.
- probe: permuter_campaign chassis-5 (s14-chassis5-lowpressure-blockscope, -j8, base_score 695), 20,344 iters across in-turn wait windows; inspected every novel output incl. both sub-floor finds; harvested --stop in-turn (alive=False, active=False, 0 live campaigns, 0 orphan permuter procs). Also confirmed tools/permuter_annotate.py is INAPPLICABLE (hint catalog covers none of the diverging region).
- result: No legitimate sub-floor find. Legitimate reordering ceiling 635-695, ALL ABOVE the 630 floor (chassis-5 never even legitimately recovers the true floor; output-630-1 ties it via a null 0x800-constant-hoist reshuffle, non-improving). Only two strictly-sub-floor finds, BOTH cheats: output-500-1 (500) = corrupted fmt pointer (g_gpu_dma_madr substituted for &g_str_gpu_timeout, masked-Levenshtein false-match) + char width-coercion + dead alias self-assign (banked permuter-alias-longlong-junk family); output-580-1 (580) = extern volatile int D_8009BF68[] = banked volatile-BF68 coercion (axis A only, stripped by volatile_cheats). output-635-1 = u16 constant-holder cheat (return -1). Axis A never legitimately materialized (BF68[0] stays inline combine-folded); axis B 8-op sched cluster never legitimately reordered.
- verdict: KILLED

## [s15] Independent forensic re-verification (fresh dumps + GCC source) confirms or refutes the s6/s7 opus-authored mechanism pins for axis A (combine.c:1458 added_sets_2) and axis B (sched1 volatile-MEM anti-dep REG_DEP_ANTI dead-read->BF7C).
- mechanism: Axis A = combine keeps the 3-insn address materialization only when the address pseudo (I2DEST) is live past I3 (multi-use); a single pure offset-0 rvalue read folds. Axis B = sched1 INSN_PRIORITY = critical-path height; the dead volatile *g_gpu_stat_reg read gains height 2 from a volatile-MEM ANTI edge to the volatile D_8009BF7C read, out-ranking the fmt `la` (height 1).
- probe: Fresh cc1 -da -dr dumps over committed src/display.c (tmp/grind/func_8007DC9C/s15/); read combine.c:1458 in tools/gcc-2.7.2; grep sched priority list + post-sched RTL for the dead-read/fmt/BF7C insns and the REG_DEP_ANTI edge; re-run sandbox --disable all.
- result: combine.c:1458 verbatim `added_sets_2 = ! dead_or_set_p (i3, i2dest);` (source-verified, s6/s7 citation exact). s15 sched: insn[41] dead read priority=2 ref_count=5 = (mem/v (reg 82)); insn[63] fmt priority=1 ref_count=1; insn[48] = (mem/v D_8009BF7C) carries (insn_list:REG_DEP_ANTI 41) — both mem/v, so the ANTI edge is purely volatile-ordering. Decision T-42 {63(1),41(2)} emits dead read before fmt (our order); target wants the reverse. UIDs shifted uniformly +3 vs s6 (HEAD body vs candidate) with otherwise-identical priorities/topology/decision => axis B is body-shape-invariant. Baseline held at score 9 (identical fingerprint). Both mechanisms CONFIRMED; no new pure-C lever. s6/s7 hold up under independent scrutiny.
- verdict: CONFIRMED (both axes mechanism-pinned dead; escalation-ready, no owner entry filed)

## [s15] The axis-A 3-insn D_8009BF68[0] materialization is combine.c:1458 added_sets_2 multi-use retention, so a single pure offset-0 rvalue read (which this function is) always folds to the 2-insn form.
- mechanism: tools/gcc-2.7.2/combine.c:1458 reads verbatim `added_sets_2 = ! dead_or_set_p (i3, i2dest);` — combine keeps the address-materialization SET (I2) only when I2DEST (the address pseudo) is live past I3 (a 2nd use). For a single-use offset-0 read I2DEST dies in I3, so the address folds into the mem.
- probe: Read combine.c:1440-1475 in tools/gcc-2.7.2; grep the exact line number; inspect the BF68 access in the fresh s15 combine dump.
- result: Line 1458 is exactly `added_sets_2 = ! dead_or_set_p (i3, i2dest);` (preceding comment confirms 'kept around ... needed past I3'). s15 combine dump shows `(mem/s:SI (symbol_ref "D_8009BF68"))` symbol-direct with no (plus ... 0) — folded to 2-insn. The s6/s7 citation is exact; a single-function 3-insn reproduction requires a dead 2nd &D_8009BF68 use = coercion (s7 banked).
- verdict: CONFIRMED

## [s15] The axis-B dead-read-before-fmt order is fixed by sched1 INSN_PRIORITY, the dead volatile *g_gpu_stat_reg read out-ranking the fmt la by exactly 1 via a volatile-MEM anti-dependency edge, not a source-order tie-break.
- mechanism: sched.c INSN_PRIORITY = critical-path height to block end. sched_analyze adds a REG_DEP_ANTI edge between the two volatile MEM reads (dead *g_gpu_stat_reg read -> volatile D_8009BF7C read) ONLY because both are mem/v; that edge gives the dead read height 2, while fmt (a lone la feeding only the call) has height 1.
- probe: Fresh cc1 -da -dr dumps over committed src/display.c; grep sched priority list, post-sched RTL insn identities, the REG_DEP_ANTI edge, and the BB2 ready-list decision.
- result: s15 sched: insn[41] priority=2 ref_count=5 = (set (reg/v 75) (mem/v (reg 82))) = dead *g_gpu_stat_reg read; insn[63] priority=1 ref_count=1 = fmt la into a0; insn[48] = (mem/v D_8009BF7C) carries (insn_list:REG_DEP_ANTI 41) — both mem/v => edge is purely volatile-ordering. Decision T-42 {63(1),41(2)}: blocks 41 one cycle (63 fills it) then launches 41 before 63 => dead read emitted before fmt (our order); target wants fmt first. Flipping requires deleting the volatile anti-dep = non-volatile/dropped read = cheat + semantically wrong.
- verdict: CONFIRMED

## [s15] Axis B is invariant to the exact C body shape (candidate vs committed HEAD), so no structural/body-shape lever can move it.
- mechanism: The two byte-equivalent bodies (floor 9) differ only in a no-op volatile-ptr dance, shifting BB2 insn UIDs uniformly by +3 (dead-read 38->41, fmt 60->63, BF7C 45->48) while leaving the dependency topology and priorities untouched.
- probe: Compare s15 fresh dump (HEAD body) against s6 dump (candidate body): priority values, ref_counts, REG_DEP_ANTI edges, and the ready-list decision.
- result: Priorities (2 vs 1), ref_counts (5 vs 1), the volatile ANTI edge, and the T-42 launch decision are IDENTICAL across the two bodies modulo the +3 UID shift. Confirms the sched1 tie is body-shape-invariant (independently corroborates s8/s11/s12 structure-insensitivity).
- verdict: CONFIRMED

## [s16] The 4 regfix rules for func_8007DC9C map completely onto the two pinned axes (no hidden third divergence hides inside the 9-op masked-Levenshtein gap).
- mechanism: axis A = combine.c:1458 added_sets_2 multi-use retention (rules 2830 subst la + 2831 insert offset-0 load); axis B = sched1 INSN_PRIORITY via volatile-MEM REG_DEP_ANTI (rules 2833 dead-read reg subst + 2835 4-insn reorder). If a third divergence existed it would require a 5th rule (or a larger honest gap than 9).
- probe: enumerated regfix.txt:2830-2835 first-hand; cross-mapped each rule to target asm lines 43-45 (axis A) / 23-42 (axis B); re-confirmed axis-A fold in fresh s16 combine dump (line 10051 symbol-direct) and the axis-B priority block (s16 sched insns 335-351); re-ran sandbox baseline (score 9, 4 rules, 91/90).
- result: 4 rules partition exactly 2+2 into axis A / axis B; honest gap 9 = 1 (axis A insertion) + 8 (axis B cluster); no 5th rule, no unaccounted insn. Rule-set is COMPLETE. Both axes mechanism-pinned dead (combine per-function fold + sched volatile-order priority), unchanged across all five modalities. func_8007DC9C is a fully-characterized ENDGAME-LOCK; owner-gated not claimable (no docs/grind/decisions.md entry).
- verdict: CONFIRMED (rule-set complete; no hidden lever; escalation-ready)

## [s16] The 4 regfix rules for func_8007DC9C (regfix.txt:2830-2835) map completely onto the two pinned axes, so no hidden third divergence hides inside the 9-op masked-Levenshtein gap.
- mechanism: AXIS A = combine.c:1458 added_sets_2 multi-use retention -> rules 2830 (subst folded lw -> la $2,BF68) + 2831 (insert_after lw $5,0($2)); AXIS B = sched1 INSN_PRIORITY via volatile-MEM REG_DEP_ANTI (dead *g_gpu_stat_reg read out-ranks fmt la by +1) -> rules 2833 (dead-read reg subst $a0->$v0) + 2835 (reorder 21,20,19,18). A third divergence would require a 5th rule or a larger honest gap than 9.
- probe: Enumerated regfix.txt:2830-2835 first-hand; cross-mapped each rule to target asm/funcs/func_8007DC9C.s lines 43-45 (axis A) and 23-42 (axis B); re-confirmed axis-A fold in fresh s16 combine dump (line 10051 (mem/s (symbol_ref D_8009BF68)) symbol-direct); located axis-B priority block in fresh s16 sched dump (insns 335-351, critical-path priorities 1-5); re-ran sandbox --disable all baseline.
- result: Baseline score 9, target 91 / build 90, rules_dropped 4, cheat_asm_stripped 150 (identical fingerprint s1-s15). 4 rules partition exactly 2+2 into axis A / axis B; honest gap 9 = 1 (axis A insertion) + 8 (axis B cluster); no 5th rule, no unaccounted insn. Rule-set is COMPLETE. Sibling func_8007D3F8's BF68 store sits in its own combine section (dump line 8422), corroborating s7 per-function combine.
- verdict: CONFIRMED

## [s17] REDERIVE: a fresh m2c decompile or a not-yet-banked control-flow rederivation (inverted early-return guard, divergent block hoisted to straight-line function scope) produces a C body that closes axis A (BF68[0] combine fold) or axis B (sched1 volatile-MEM anti-dep priority).
- mechanism: An inverted guard + function-scope-hoisted body presents a different overall skeleton to combine (axis A) / sched1 (axis B) than HEAD's comma-expression form and s8's goto/nested-if form. Refuted in advance by s8/s15 (axis A per-expression, axis B whole-block volatile-order; both control-flow-insensitive) -- tested empirically to add a fresh measured negative in the rederive modality.
- probe: (1) fresh m2c tools/m2c/m2c.py --valid-syntax --target mipsel-gcc-c -f func_8007DC9C; (2) apply the inverted early-return guard (`if (vcount >= sys_VSync(-1)) { drawcnt++; if (temp <= 0xF0000) return 0; }` + straight-line report block, short-circuit preserved) and sandbox --disable all.
- result: (1) m2c reproduces HEAD (scalar BF68 value-read folds; dead stat-read DCE'd). (2) inverted-return form -> score 12, build_insns 88 (two fewer than floor 90; target 91) -- WORSE than floor; GCC merges the two return paths. Neither axis moved. Only HEAD's comma-expression guard reaches the 90-insn floor; s8 goto/nested-if ties it; s17 inverted-return worsens it; none reaches target's 91.
- verdict: KILLED

## [s17] A fresh m2c decompile or a not-yet-banked control-flow rederivation (inverted early-return guard with the divergent debug-report block hoisted to straight-line function scope) produces a C body that closes axis A (BF68[0] combine offset-0 fold) or axis B (sched1 volatile-MEM anti-dep priority).
- mechanism: An inverted guard + fn-scope-hoisted body presents a different overall skeleton to combine (axis A) / sched1 (axis B) than HEAD's comma-expression form and s8's goto/nested-if form; refuted in advance by s8/s15 (axis A per-expression, axis B whole-block volatile-order, both control-flow-insensitive) but tested empirically for a fresh measured negative.
- probe: Fresh m2c (tools/m2c/m2c.py --valid-syntax --target mipsel-gcc-c -f func_8007DC9C); then applied the inverted early-return guard (if (g_gpu_vcount >= sys_VSync(-1)) { temp=g_gpu_draw_count; g_gpu_draw_count=temp+1; if (temp<=0xF0000) return 0; } + straight-line report block, || short-circuit preserved) and ran sandbox --disable all.
- result: m2c reproduces HEAD (scalar BF68 value-read folds; dead *g_gpu_stat_reg read DCE'd = banked score-19 collapse). Inverted-return form -> score 12, build_insns 88 (two FEWER than the 90-insn floor; target 91) = WORSE; GCC merges the two return paths. Neither axis moved. Only HEAD's comma-expression guard reaches the 90-insn floor; s8 goto/nested-if ties it (90/score 9); s17 inverted-return worsens it (88/score 12); none reaches target's 91.
- verdict: KILLED

## [s18] A not-yet-banked rederive skeleton — de-comma'ing HEAD's short-circuit comma-expression guard into an explicit boolean accumulator `do_report` computed across two separate `if` statements — produces a C body that closes axis A (BF68[0] combine offset-0 fold) or axis B (sched1 volatile-MEM anti-dep priority).
- mechanism: The flag-accumulator form presents a different overall control-flow skeleton to combine (axis A) / sched1 (axis B) than HEAD's comma-expression form (90/floor 9), s8's goto/nested-if (90/floor 9), and s17's inverted early-return guard (88/score 12). Refuted in advance by s8/s15/s17 (axis A per-expression combine.c:1458 offset-0 fold; axis B whole-block volatile-MEM REG_DEP_ANTI priority; both proven control-flow-insensitive), tested empirically for a fresh measured negative in the rederive modality.
- probe: Applied the two-if flag skeleton (do_report = (vcount < sys_VSync(-1)); if(!do_report){ temp=g_gpu_draw_count; g_gpu_draw_count=temp+1; do_report=(temp>0xF0000);} if(do_report){<report>return -1;} return 0;) — short-circuit semantics preserved (draw_count++ only on the A-false path) — and ran sandbox func_8007DC9C --disable all; then git checkout restored src to clean HEAD.
- result: score 12, build_insns 88 (two FEWER than the 90-insn floor build; target 91), rules_dropped 4, cheat_asm_stripped 150. WORSE than floor: splitting the guard into a flag + two ifs lets GCC MERGE the two exit paths (post-first-if fall-through and the final return 0), collapsing to 88 insns and moving away from target's 91. Lands at the SAME 88/12 fingerprint as s17's inverted-return: any control-flow shape that coalesces the two exit paths drops to 88. Neither pinned axis moved. Rejected: rejected/rederive-computed-flag-two-if.c.
- verdict: KILLED

## [s19] SYNTHESIS: re-merging all 18 prior sessions surfaces some un-banked pure-C lever OR strengthens the escalation dossier to the point the disposition is unambiguous.
- mechanism: A second synthesis pass (s10 was first, over s1-s9 only) cross-reads the post-s10 accumulation — permuter s13/s14, forensics s15/s16, rederive s17/s18 — for any lever that survives the individual KILLs but only becomes visible when merged. Axis A = combine.c:1458 added_sets_2 (source-verified s15); axis B = sched1 REG_DEP_ANTI volatile-MEM priority (s6/s15); rule-set proven complete 2+2 (s16).
- probe: No sandbox re-run (mechanism-pinned; frontier + s11-s18 empirically show re-measurement only re-confirms). Full cross-read of evidence.md (460 lines) + hypotheses.md + all 12 rejected/ forms + docs/grind/decisions.md; wrote consolidated dossier tmp/grind/func_8007DC9C/s19/MERGED-ATTACK-s19.md.
- result: NO un-banked lever exists. Both axes remain orthogonal and mechanism-pinned dead across all five modalities (structural s2/s3/s11/s12, permuter x5 ~118k iters s4/s5/s13/s14, forensics x4 s6/s7/s15/s16, rederive x4 s8/s9/s17/s18, synthesis s10/s19). The 4 regfix rules are a proven 1:1 cover (s16, no hidden 3rd axis). endgame-lock-disposition-policy: gate #1 REFUSE (scan_hand_coded LOW), gate #2 REFUSE (no coercion precedent) -> keep 4 rules, INCOMPLETE-owner-accepted. STILL no OWNER-ESCALATION entry in docs/grind/decisions.md (grep 8007DC9C = No matches this session) -> owner-gated NOT claimable; the only unblock is the OWNER filing the entry.
- verdict: KILLED (no lever; escalation dossier strengthened and re-merged)

## [s19] Re-merging all 18 prior sessions surfaces some un-banked pure-C lever, OR strengthens the escalation dossier to the point the endgame-lock disposition is unambiguous.
- mechanism: A 2nd synthesis pass (s10 covered only s1-s9) cross-reads the post-s10 accumulation for a lever that survives the individual KILLs but only becomes visible when merged. Axis A = combine.c:1458 added_sets_2 multi-use retention (single pure offset-0 rvalue read folds to 2-insn; SOURCE-verified s15). Axis B = sched1 INSN_PRIORITY via volatile-MEM REG_DEP_ANTI dead-read->BF7C (s6/s15). Rule-set proven complete 2+2 (s16).
- probe: No sandbox re-run (mechanism-pinned; s11-s18 empirically show re-measurement only re-confirms). Full cross-read of evidence.md (460 lines) + hypotheses.md + all 12 rejected/ forms + docs/grind/decisions.md; wrote consolidated dossier tmp/grind/func_8007DC9C/s19/MERGED-ATTACK-s19.md.
- result: No un-banked lever exists. Both axes remain orthogonal and mechanism-pinned dead across all five modalities (structural s2/s3/s11/s12, permuter x5 ~118k iters s4/s5/s13/s14, forensics x4 s6/s7/s15/s16, rederive x4 s8/s9/s17/s18, synthesis s10/s19). endgame-lock-disposition-policy: gate #1 REFUSE (scan_hand_coded LOW), gate #2 REFUSE (no coercion precedent) -> keep 4 rules, INCOMPLETE-owner-accepted. Still no OWNER-ESCALATION entry (grep = No matches) -> owner-gated NOT claimable.
- verdict: KILLED

## [s20] A combination of two s2 structural levers (fmt-pointer precompute declared FIRST + first-printf masked subtraction hoisted to a named temp diff0) — a form s2 only measured piecewise — moves axis A (BF68[0] combine offset-0 fold) or axis B (fmt-vs-deadread 8-op sched1 cluster).
- mechanism: The combination changes LUID/declaration topology feeding sched1 and combine simultaneously, a joint perturbation not covered by any single s2 form. Refuted in advance by s15 (axis B priority is body-shape-invariant: dead-read priority 2 > fmt priority 1 via the volatile-MEM anti-dep REG_DEP_ANTI, identical modulo a uniform UID shift across every body shape) and by axis A being a per-expression combine.c:1458 added_sets_2 offset-0 fold (BF68[0] is a single pure rvalue with no 2nd address use).
- probe: Applied const s32 *fmt0 = &g_str_gpu_timeout (declared first) + s32 diff0 = (D_8009BF78 - D_8009BF7C) & 0x3F, then debug_printf(fmt0, diff0, ...) in src/display.c; ran sandbox func_8007DC9C --disable all; restored src to clean HEAD.
- result: score 9, target_insns 91, build_insns 90, rules_dropped 4, cheat_asm_stripped 150 — IDENTICAL fingerprint to s1-s19. Neither axis moved. Rejected: rejected/structural-combined-fmtptr-difftemp.c.
- verdict: KILLED

## [s21] A novel un-banked structural three-lever combination (dead stat-read staged FIRST + fmt-pointer precompute + masked-subtraction diff-temp, as a UNIT) moves axis A (BF68[0] combine offset-0 fold) or axis B (fmt-vs-deadread 8-op sched1 cluster).
- mechanism: The combination reshapes declaration/LUID/statement topology of the axis-B cluster in a way no single s2 form (fmt-precompute-first, new_var-declared-first, bf78/diff-temp — all 9) nor s20 (fmt-ptr-FIRST + diff-temp — fmt before the dead read, 9) covered. Refuted in advance by s15 (axis B priority body-shape-invariant: dead-read priority 2 > fmt priority 1 via volatile-MEM REG_DEP_ANTI, identical modulo uniform UID shift) and by axis A being a per-expression combine.c:1458 added_sets_2 offset-0 fold on a single pure rvalue.
- probe: Applied const s32 *fmt0=&g_str_gpu_timeout + s32 diff0=(D_8009BF78-D_8009BF7C)&0x3F with `new_var=*g_gpu_stat_reg` staged as the FIRST if-body statement, then debug_printf(fmt0, diff0, ...); ran sandbox func_8007DC9C --disable all; restored src to clean HEAD.
- result: score 9, target_insns 91, build_insns 90, rules_dropped 4, cheat_asm_stripped 150 — IDENTICAL fingerprint to s1-s20. Neither axis moved. Rejected: rejected/structural-deadread-first-fmtptr-difftemp.c.
- verdict: KILLED

## [s21] A novel un-banked structural three-lever combination — dead stat-read `new_var` staged FIRST + fmt-pointer precompute (fmt0) + masked-subtraction home (diff0), as a UNIT — moves axis A (BF68[0] combine offset-0 fold) or axis B (fmt-vs-deadread 8-op sched1 cluster).
- mechanism: The combo reshapes declaration/LUID/statement topology of the axis-B cluster in a way no single s2 form (fmt-precompute-first, new_var-declared-first, bf78/diff-temp) nor s20 (fmt-ptr-FIRST + diff-temp) covered. Refuted in advance by s15 (axis B priority body-shape-invariant: dead-read priority 2 > fmt priority 1 via volatile-MEM REG_DEP_ANTI, identical modulo uniform UID shift) and by axis A being a per-expression combine.c:1458 added_sets_2 offset-0 fold on a single pure rvalue.
- probe: Applied const s32 *fmt0=&g_str_gpu_timeout + s32 diff0=(D_8009BF78-D_8009BF7C)&0x3F with new_var=*g_gpu_stat_reg staged as the FIRST if-body statement, then debug_printf(fmt0, diff0, ...); sandbox func_8007DC9C --disable all; restored src to clean HEAD.
- result: score 9, target_insns 91, build_insns 90, rules_dropped 4, cheat_asm_stripped 150 — identical fingerprint to s1-s20; neither axis moved. Rejected: rejected/structural-deadread-first-fmtptr-difftemp.c.
- verdict: KILLED

## [s22] A 6th structurally-distinct permuter chassis (fully-inlined MINIMAL-register-pressure topology: no named temps, all first/second-printf pieces inline — opposite regime to temp-heavy chassis-3/4/5, distinct from chassis-1's HEAD-block form) finds a legitimate pure-C form closing axis A (BF68[0] 3-insn materialization) or axis B (8-op fmt-vs-deadread sched1 cluster).
- mechanism: minimal-live-range regime feeding sched1 INSN_PRIORITY (axis B) and combine (axis A) distinct from all 5 prior chassis; volatile reads kept inline+ordered so the axis-B volatile-MEM anti-dep REG_DEP_ANTI (s6/s15) is preserved. base validated 90 insns / target 91; base-vs-target objdump diff reproduces EXACTLY both ledger axes.
- probe: permuter_campaign chassis-6 (s22-chassis6-fullyinline-minpressure, -j8, base_score 630), 35,277 iters ~21 min across 3 in-turn wait windows; inspected the single novel find; harvest --stop in-turn (stopped=true, procs_killed=9, 0 orphan s22 permuter procs).
- result: No legitimate sub-floor find. The ONLY novel find (output-515-1, 515, t+666s) = `extern volatile unsigned long D_8009BF68[]` = banked volatile+width coercion (axisA-permuter-volatile-bf68.c / axisA-permuter-longlong-width-coercion.c family), stripped by volatile_cheats; closes axis A only via coercion. Axis-B 8-op sched cluster never legitimately reordered. Floor held at 9. Permuter now SEXTUPLE-confirmed dead across 6 chassis (~153k total iters: s4 27k + s5 40k + s13 31k + s14 20k + s22 35k).
- verdict: KILLED

## [s22] A 6th structurally-distinct permuter chassis (fully-inlined MINIMAL-register-pressure topology: no named temps, all first/second-printf pieces inline — the opposite regime to the temp-heavy chassis-3/4/5 and distinct from chassis-1's HEAD-block form) finds a legitimate pure-C form closing axis A (BF68[0] 3-insn materialization) or axis B (8-op fmt-vs-deadread sched1 cluster).
- mechanism: A minimal-live-range regime feeding sched1 INSN_PRIORITY (axis B) and combine (axis A) distinct from all 5 prior chassis; volatile reads kept inline+ordered so the axis-B volatile-MEM anti-dep REG_DEP_ANTI (s6/s15) is preserved. base validated at 90 insns / target 91, base-vs-target objdump diff reproduces EXACTLY both ledger axes (axis-B fmt/BF78/deadread reorder cluster + axis-A target 3-insn lui;addiu;lw 0(v0) vs base folded 2-insn lui;lw).
- probe: permuter_campaign launch chassis-6 (s22-chassis6-fullyinline-minpressure, -j8, base_score 630); 35,277 iterations over ~21 min across 3 in-turn wait windows; inspected the single novel find; harvested --stop in-turn (stopped=true, procs_killed=9, 0 orphan s22 permuter procs).
- result: No legitimate sub-floor find. The ONLY novel find (output-515-1, score 515, surfaced at t+666s) is `extern volatile unsigned long D_8009BF68[]` = the banked volatile+width coercion of the game-state global (axisA-permuter-volatile-bf68.c / axisA-permuter-longlong-width-coercion.c family), stripped by engine.volatile_cheats and forbidden by inline-asm-policy + legitimate-volatile-interrupt-touched criterion #2 (single printf-arg read, not spin-wait/double-read/IRQ-loop-bound). Closes axis A only via coercion; the axis-B 8-op sched cluster never legitimately reordered. Floor held at 9 the entire campaign.
- verdict: KILLED
