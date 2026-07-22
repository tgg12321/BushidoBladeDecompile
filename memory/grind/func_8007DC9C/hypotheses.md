# Hypothesis ledger — func_8007DC9C

Floor = 9 (verdict C, target 91 / build 90 insns). Gap decomposes into two
independent axes: Axis A = 1 combine-fold insertion (BF68[0]); Axis B = 8-op
sched1 reorder cluster in the first debug_printf setup.

## Live frontier (mechanism-grounded, built s1 recon)

### H-A1 — offset-0 combine fold on D_8009BF68[0] (Axis A, 1 op)
- statement: target keeps a 3-insn materialized address for D_8009BF68[0]; our
  build combine-folds it to 2 insns. A declaration/access shape exists that
  suppresses the offset-0 fold in pure C.
- mechanism: combine.c folds `(mem (plus symbol_ref 0))` → `(mem symbol_ref)`.
  [[defeat-combine-symbol-fold]] excludes offset 0.
- next_probe: read tools/gcc-2.7.2/combine.c symbol+0 simplification path; test
  a declaration consistent with BF68's real function-pointer type (see H-A2).
- status: OPEN. Fold-defeat family already exhausted in the imported rejected
  bank (sized array / fn-ptr cast / intervening-call precompute all → 9). Do NOT
  re-run those; the un-tried lever is a combine.c-informed decl shape.

### H-A2 — D_8009BF68 real type is a function pointer, not an int array (Axis A)
- statement: the two in-tree block-scope externs disagree (line 872 fn-ptr vs
  line 965 array); func_8007D3F8 assigns `D_8009BF68 = arg0` (scalar fn-ptr) and
  fills a fn-ptr table. Declaring/accessing BF68 consistent with its true scalar
  fn-ptr type may emit the materialized (non-folded) load target wants.
- mechanism: a scalar (non-array) symbol read vs an array[0] read can differ in
  the address RTL combine sees.
- next_probe: after func_8007D3F8 matches (queue ~43, only other user), adopt its
  committed decl shape here; or probe the scalar-read shape directly now.
- status: OPEN, partly a sibling dependency (non-blocking; can probe scalar-read now).

### H-B1 — sched1 fmt-vs-BF78 load-order tie-breaker (Axis B, 8 ops)
- statement: target schedules the fmt address load BEFORE the BF78 load chain, so
  the early *stat_reg read allocates to $v0; our build loads BF78 first, pushing
  the early read to $a0 and fmt later. A C statement-order / liveness lever flips
  the tie-break.
- mechanism: sched.c insn priority / LUID tie-break in the first debug_printf
  argument setup. Closed by regfix `reorder 21,20,19,18` + `subst lw $4→$2`.
- next_probe: run instrumented cc1 (tmp/gccdbg/cc1, WSL) with BB2_SCHED_DEBUG=1
  BB2_PRIO_DEBUG=1 on the sandbox .i; read the priority values for the fmt-LUI vs
  BF78-LUI nodes; derive the C lever (arg-order / local-hoist / liveness). The
  imported block-scope/fmt-local/(void)-read levers did NOT move the schedule —
  the dump is the missing map.
- status: OPEN, highest-value un-run probe (8 of 9 ops).

## Rejected forms — see evidence.md (imported WIP bank). Do NOT re-propose.

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
