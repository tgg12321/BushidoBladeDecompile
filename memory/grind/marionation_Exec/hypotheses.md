# Hypothesis ledger — marionation_Exec

## [s1] The ledger's masked-4 floor (vT40 candidate) reproduces on main after the work/marion merge
- mechanism: candidate.c spliced into src/system.c; cheat-invisible sandbox with all 42 rules disabled is the honest metric
- probe: splice.py applied candidate.c; ran `canonical marionation_Exec` and `sandbox marionation_Exec --disable all`
- result: canonical verdict C (distance 4, total 179); sandbox score 4, build 178 vs target 179 insns, 42 rules dropped — exactly the documented floor. src restored to HEAD after measuring.
- verdict: CONFIRMED

## [s1] The handoff's one active lever — the rich-pass permuter campaign (watcher, triage.log, vT40 base) — survived the merge and can be resumed directly
- mechanism: campaign lived in worktree bb2-work-marion's tmp/, which is gitignored; worktree was removed at merge
- probe: globbed tmp/perm_mar*/ for triage.log and campaign state; checked campaign.pid; checked for vP160.c, ordersweep.log, sched1dump.py, probe.py, qtydbg/dbrdbg/knobs tooling on main
- result: KILLED: no triage.log anywhere; campaign.pid empty; tmp/perm_mar outputs are the STALE July-3 whole-file-noise campaign (scores ~237k); perm_mar6 best=200 (July 3-4, pre-vT40); all session-10 tooling and the vP160 masked-3 signpost source are gone. Mechanisms/measurements survive in the ledger only.
- verdict: KILLED

## [s1] The twin cpu_side_move_dir_4 (same coupled pair-swap per ledger lore) has since closed and offers a transferable lever
- mechanism: identical sched2-LUID + qty_compare-tie coupling documented for its g3 pair
- probe: read engine/queue.json entry for cpu_side_move_dir_4
- result: KILLED: parked, distance 7, 5 rules — unsolved; no lever to transfer.
- verdict: KILLED

## [s2] A true-semantics second arg5 ref via self-staged address (arg5 = v0 + tbl; arg5 = *(s32*)arg5;) lifts weighted refs to 6 and wins the qty_compare 5.33v5.33 tie with no new pseudo
- mechanism: self-staging adds set+use to arg5's web (pri 8.0 > t0's 5.33) while eliminating the compiler addr-temp instead of birthing one
- probe: v01 (t0-first) and v02 (vT32 position) swept via sweep_variants (tmp/grind/marionation_Exec/s2/sweep1.json)
- result: masked 11 both — absorbing the addr-temp into arg5's web re-times the head exactly like the fresh-temp launches (vT33/vT34 class); the addr-temp 102 seat (v0) is load-bearing
- verdict: KILLED

## [s2] Reducing t0-web to refs 2 (index staged through dead v0) flips the qty tie the other way
- mechanism: qty_compare tie 5.33v5.33 breaks on birth order; t0 as single-set address (set+use) changes its pri/birth
- probe: v05 (v0 stages t0 index), v22 (t0 via v0 after arg5), v10 (combo with arg5 self-addr) — sweep1/sweep3
- result: v05=11 with 180 insns (+2, extra moves), v22=17, v10=18 — every t0-web reduction births/retimes worse than it fixes
- verdict: KILLED

## [s2] Threading arg5 or the arg4 deref through the dead v0 at the call changes the window without new pseudos
- mechanism: v0 is dead after arg5's load; a real copy into an existing pseudo is not a launchable fresh temp
- probe: v03 (v0 = arg5 passed as 5th arg), v04 (v0 = *(s32*)t0 as 4th arg) — sweep1
- result: v03=14, v04=15 — the extra move enters the window and scrambles sched
- verdict: KILLED

## [s2] Shift-merged-into-load geometry (arg5 = *(s32*)((v0<<2)+tbl) / v0 = idx[1]<<2) explores sched-tie space the 140-statement-ordering sweep could not reach
- mechanism: merging changes RTL LUIDs and temp structure without adding user pseudos or notes
- probe: v08/v09 (sweep1), v13-v21 (sweep2), windows adiffed (adiff_v08/v09/v20/v21.txt)
- result: CONFIRMED as mechanism: v08/v20 emit a THIRD window order (sll a0; sll v0; addu) at masked 4 with seats intact — the tie IS geometry-movable without seat trade. KILLED as closer: target's (sll v0; addu; sll a0) needs LUID(arg5-addu) < LUID(t0-sll) = arg5-first source, and ALL arg5-first geometries (v13/v14/v15) = 8 with seats traded — the coupled fixed point holds across the whole geometry axis
- verdict: CONFIRMED

## [s2] Block-local declaration order or type narrowing (u32) perturbs qty birth order / the seat tie
- mechanism: qty birth order breaks the 5.33v5.33 tie; decl order might reorder pseudo allocation
- probe: v24-v29: all 6 decl permutations + u32 t0 + u32 arg5, baseline and v08 geometries — sweep3
- result: all 6 score 4 with identical emission — pseudo birth follows RTL first-use order, not declarations; u32 is codegen-identical here
- verdict: KILLED

## [s2] Deferring t0's table-add entirely into the call deref (*(s32*)((u8*)tbl+t0)) re-times the window since t0's addu already emits late (B61)
- mechanism: moves the addu's LUID into the call-arg region, changing t0's web life
- probe: v30 — sweep4
- result: masked 16
- verdict: KILLED

## [s3] Decoupling arg5's address-add from its load (addu as a 3rd statement in the existing v0 staging web, deref deferred) breaks the coupled fixed point - the one decomposition the 140-ordering sweep structurally could not reach (addu was always fused to the load statement)
- mechanism: addu LUID placed before t0-sll for the sched2 tie while the load (arg5val birth) stays late for tight lives; v0 is an existing staged var so no launchable fresh pseudo, no notes
- probe: 12 variants (w01-w12): all addu positions x deref positions x pp positions x merged spellings x operand swap, swept via sweep_variants (tmp/grind/marionation_Exec/s3/sweep1.json)
- result: Every 3-statement v0 web (split load/shift/add) scores 17 UNIFORMLY regardless of any position - growing the staging web to 3 sets re-times the head like a launch, position-invariant. 2-statement merged spellings (w04 shift+add merged, w05 load+shift merged) score 8: window order is EXACTLY target (sll v0; addu; sll) but seats trade (t0-web->v1, arg5val->a0; adiff_w04.txt)
- verdict: KILLED

## [s3] The w04 seat trade is caused by the decoupled deref shortening arg5val's life (pri 8.0 beats the 5.33 tie, allocates first, steals a0) - so placing the deref EARLY (long arg5val life, back to the tie, t0 births first) fixes seats while the early addu keeps the order
- mechanism: qty_compare pri = floor_log2(refs)*refs*size/life; lengthening arg5val's life to ~6 restores the 5.33v5.33 tie which vT40 wins by t0's birth order
- probe: 6 variants (x01-x06): addr-in-v0 as 1-stmt (fully merged) and 2-stmt spellings x deref immediately/mid-t0/late (tmp/grind/marionation_Exec/s3/sweep2.json), x01 adiffed
- result: ALL 6 score 8 with the identical signature (target order, seats traded; adiff_x01.txt byte-identical window to w04). The trade is INVARIANT to arg5val's life and deref position: whenever the addu's LUID precedes t0's sll, seats trade - in fused (s2), decoupled, and merged spellings alike
- verdict: KILLED

## [s3] Array-indexing (tbl_125c[v0]) and tbl-first operand spellings build the RTL plus with opposite operand order / mult-by-4 instead of ashift, reaching tree shapes and LUID orders no swept form used
- mechanism: expand-level canonicalization might not normalize (plus tbl (mult v0 4)) to the same insn order as (plus (ashift v0 2) tbl)
- probe: 4 variants (y01-y04) swept (tmp/grind/marionation_Exec/s3/sweep3.json); y01 adiffed
- result: Canonicalization is total: y03 fused operand swap = 4 emission-identical to baseline; y01 array-index t0-first = 4 but emits only the OTHER known addu-last order (sll a0; sll v0; addu, seats correct - adiff_y01.txt), confirming the reachable-order set from t0-first is exactly {addu-last x2}; y02 array-index arg5-first = 8 (trade again); y04 = 5
- verdict: KILLED

## [s4] The rich-pass offset-0 permuter campaign can be rebuilt on main from the perm_mar6 skeleton with the vT40 base
- mechanism: single-function offset-0 target.o (asm/funcs/marionation_Exec.s via prelude, reused from perm_mar6) + full-context splice compile.sh (cc1|prologue_fix|maspsx|multu_pad, fn extracted and assembled standalone); permuter scores base 220 = 2 reorderings(60) + 1 deletion(100) = exactly the masked-4 residuals
- probe: build_ws.py built tmp/grind/marionation_Exec/s4/perm; permuter --debug scored base; campaign -j24 --better-only --stop-on-zero + watcher auto-triage (pair window 50-63, region-3 window 144-157) into triage.log
- result: base score 220 confirmed; ~10k iterations on the vT40 base produced 15 finds (best 145); campaign infrastructure fully reproducible from ledger notes alone
- verdict: CONFIRMED

## [s4] The sampler can reach an honest-semantics 179-insn form that kills the region-3 delay-slot steal
- mechanism: output-145-1 mutation: stage idx_1494 through the EXISTING dead-at-top local dst2 (dst2 = idx_1494; reads via dst2[0]/dst2[1]/*dst2), i.e. staged-value-reused-variable family (owner-sanctioned 2026-07-03). The merged web spans fn-top to the copy blocks, so dst2 allocates CALLEE-SAVED (s1); check2's 'dst2 = a1' compiles to move s1,s4 whose dest is live at the backedge target -> reorg fill_simple REJECTS the steal -> beqz;nop;sb;move exactly as target, at 179/179 insns with true semantics (dst2 re-set before every later use, no uninit reads)
- probe: fndiff.py extraction + s2 adiff.py sandbox splice + adiff2.py normalized LCS on the find
- result: region-3 aligned insn-for-insn (first honest steal-kill; all previously-known routes paid a visible byte) but masked 17: idx/saved trade s1/s2, dst2 seat s1 vs target a1, loop-reg cascade (a1/a3)
- verdict: CONFIRMED

## [s4] Seeding the sampler on find145 recovers the scrambled seats
- mechanism: perm_b/perm_c campaigns based on the find; output-105-1 adds a split-init on saved (saved = *D_800A147C_2; saved = saved & 3; - the SANCTIONED split-init-accumulation family) which flips idx back to s2 (target seat, pair window lbu a0,0(s2) restored); output-95-1 (u8->u16 saved) shaves permuter reg points only
- probe: adiff2.py on find105/find95: masked 10 both; residue = pair displacement 56/57 (2), lbu dest s1-vs-v0 at 86 + andi operand at 88 (split cost), move s2-vs-a1 at 151 + beqz/li cascade (154/156)
- result: masked 10 is the family floor: permuter progress 105->95 moved ZERO masked points - the permuter metric (regs x5) diverges from the engine masked metric once reg-name diffs dominate
- verdict: CONFIRMED

## [s4] The alias-merge (liveness) steal-kill family can close region-3 exactly
- mechanism: for the fill to be rejected by liveness, the move's DEST register must be live at the backedge target; everything live there crosses the loop's calls, so the dest must be CALLEE-SAVED. Target's move is 'move a1,s4' - a1 is caller-saved, and target's prologue already uses all 8 callee-saved regs. The original source therefore did NOT kill the steal via liveness; its region-3 nop comes from another route (consistent with s10's byte-proven BB2_ALLLIVE_LABEL all-live/young-label diagnostic)
- probe: structural argument grounded in the measured finds: every liveness-kill find (145/105/95) carries the callee-saved move seat diff; every non-liveness steal-kill find (160/165/170/175 class) pays a visible +1 insn (180 total, some also semantically divergent sb s0 forms - vP160 class re-confirmed by the sampler independently)
- result: family KILLED as closer (floor of the family measured at masked 10); banked as rejected/alias-merge-dst2-kills-steal-callee-saved-seat-17.c and rejected/alias-merge-plus-saved-split-idx-s2-restored-10.c
- verdict: KILLED

## [s4] The pair-swap addu-early <=> seats-trade coupling might break on the find105 chassis (different callee-saved landscape than vT40, where s2/s3 measured it)
- mechanism: the dst2-merge relandscapes the qty allocation (idx web merged, saved split) - the 5.33v5.33 qty tie inputs could differ, potentially decoupling window order from seat assignment
- probe: 4 do_timeout geometry variants of find105 swept via sweep_variants.py (tmp/grind/marionation_Exec/s4/f105vars/): arg5-first order, v08 shift-merged t0-first, v08 arg5-first, both-merged arg5-first
- result: coupling HOLDS chassis-invariantly: t0-first merge = 10 (neutral, same as find105), ALL arg5-first forms = 14/15 (+4, the identical seat-trade penalty vT32 paid on vT40: 8 vs 4). The pair-swap fixed point survives callee-saved relandscaping
- verdict: KILLED

## [s5] Deeper permuter progress on the find105 (alias-merge) chassis (perm_c output-95-1 masked 10 -> output-85-1 permuter score 85) breaks past the ledger's masked-10 family floor
- mechanism: the alias-merge family's floor is set by the callee-saved-seat trade (dst2 = a1-target vs s1-callee-saved) — permuter's reg-diff optimization can only shuffle seats, not change the family constraint; the metric divergence (perm score vs masked) means lower perm doesn't imply lower masked
- probe: extracted marionation_Exec from perm_c/output-85-1 and output-95-2, spliced into src/system.c, ran `sandbox marionation_Exec --disable all`; restored src (tmp/grind/marionation_Exec/s5/extract_and_score.py)
- result: output-85-1 = masked 14; output-95-2 = masked 15 — both WORSE than the ledger's family floor of 10 (output-95-1). Permuter is trading seats further from target as reg-diff points drop.
- verdict: KILLED

## [s5] The running perm campaign (vT40 base, ~8h beyond s4 snapshot) has produced a masked-below-10 find outside the alias-merge family
- mechanism: the vT40 base samples statement geometries and label-attracting shapes without the callee-saved-seat penalty of find105/find145
- probe: listed all output-* under tmp/grind/marionation_Exec/s4/perm/; sub-145 scored; per-triage new outputs since s4 = output-145-2, output-160-3, output-165-3
- result: No new sub-145 finds. output-145-2 = masked 17 (find145-class); output-165-3 unchecked but same +1 class; output-160-3 = masked 3 at 180 insns (vP160 signpost class — same +1 rejection). No new lever.
- verdict: KILLED

## [s5] perm/output-160-3 (a NEW variant with 0xFF routed through `new_var` locals + `while (status = 0)` / `while (status)` in the idx_1496 clear blocks) is a semantically distinct route to the masked-3 signpost that could yield an honest-semantics closer
- mechanism: `while (status = 0)` is a valid single-iter loop (assigns 0, condition false, exits) that carries a label after the clear store — potentially serving as a young-label / find_basic_block(-1) trigger equivalent to vP160's `while(status)`
- probe: extracted the function, spliced, sandbox = masked 3 @ 180 insns. Reading the body: `saved = (*D_800A147C_2) & 3;` (single-stmt, dropped split-init), `new_var = 0xFF; new_var3 = 0xFF;` intermediates used as AND-mask, `while (status = 0);` after `*idx_1496 = 0` in check1 and `while (status);` in check2. Bank: memory/grind/marionation_Exec/rejected/vP160-3-newvar-0xff-while-status-eq-0-sentinel.c
- result: Still 180 insns (+1). The label-alive route always emits a visible byte — this is a THIRD variant confirming the sampler cannot find a semantically-valid ZERO-cost label route from the vT40 base. Also reads status across blocks (`while (status)` in check2 uses status from `func_80080828()` an unbounded number of blocks earlier) — not semantically safe.
- verdict: KILLED

## [s6] The pair-swap @ mar_system_s6.s lines 1155-1157 is decided at sched2 BB #3 T-14 between insn 106 (sll a0<<=2, pri=2) and insn 117 (sll v0<<=2, pri=2), with 106 winning the priority tie via lower insn number
- mechanism: sched2 ready-list reorder at T-14 = [106(pri2), 141(pri1), 117(pri2)] -> [106, 117, 141]. Neither 106 nor 117 is flagged 'greater potential hazard'. The tie between two equal-priority ashlsi3s is broken by insn UID (106 < 117 → 106 emitted at chronological pos 7 (T-14); 117 emitted at pos 6 (T-15)). Target requires pos 7 = insn 120 (addu v0,v0,s5) or a lower-numbered 117. Insn 120 first becomes ready at T-13 because 120's forward user insn 122 (lw v1,0(v0)) schedules at T-12 (backward walk), one T-index too late to reach T-14. Chain-shortening blocked by dep DAG (122<-137 sw stack-arg push at T-8).
- probe: Fresh cc1 dumps this session: tmp/grind/marionation_Exec/s6/dumps/mar_system_s6.i.sched2 + .greg; extracted BB #3 schedule trace + insn definitions; correlated insn numbers to mar_system_s6.s emission at 1155-1160
- result: CONFIRMED at the insn level: sched2 tie decision is exactly as named; both ligature insn-number and dep-chain path measured. Reconfirms session-10's sched1dump.py finding from the removed worktree using dumps produced on main.
- verdict: CONFIRMED

## [s6] The region-3 slot steal at mar_system_s6.s line 1276-1277 is decided at reorg pass #1 fill_simple_delay_slots on the check2 branch_zero, taking `move $5,$20` (dst2=a1) from the fall-through into the slot; the fill is not rejected because neither (a) the fall-through target label is 'young' (post-flow-analysis) nor (b) the move's destination $5=a1 is present in the target label's live-pseudo set
- mechanism: dbr pass summary in mar.dbr: '3 insns needing delay slots, 2 got 0 delays, 1 got 1 delays'. The 1 filled slot at pass #1 is check2's beqz. In target asm/funcs/marionation_Exec.s @ 71A84 the check2 slot is nop and the move a1,s4 emits post-branch at 71A8C. All pseudos live at the fall-through target cross the outer loop's calls -> callee-saved seats, so the caller-saved a1 destination cannot be in the live-set. find_basic_block on the fall-through target label returns a valid BB (label is old, pre-flow) → no all-live rejection. Session-10's BB2_ALLLIVE_LABEL env-knob forces all-live and produces target's nop (byte-proven).
- probe: Fresh cc1 dumps this session: tmp/grind/marionation_Exec/s6/fn/mar.dbr (delay-branch reorg RTL), tmp/grind/marionation_Exec/s6/dumps/mar_system_s6.s (emitted asm) versus asm/funcs/marionation_Exec.s (target)
- result: CONFIRMED at the insn level: the slot fill is done in dbr pass #1 (fill_simple_delay_slots) with the fall-through move; the reject predicates (own_fallthrough=0, all-live) do not fire. Reconfirms session-10's DBRDBG mechanism from the removed worktree using dumps produced on main.
- verdict: CONFIRMED

## [s6] The alias-merge / callee-saved lever explored in s4 was structurally KILLED as a byte closer because insn 120 (addu v0,v0,s5) — the target-order insn we need at T-14 — has its ready-time set by the arg5 stack-store insn 137 backward, not by any user-visible C construct we haven't tried
- mechanism: From the greg dump: insn 137 (sw v1,16(sp) = arg5 stack push) depends on insn 122 (lw v1,0(v0) = arg5 load). 137 must emit at pos 12 or earlier (before the jal at 152). Backward walk schedules 137 at T-8 → 122 at T-12 → 120 at T-13. To bring 120 to T-14, 122 would need T-11 or earlier, requiring 137 to move earlier — but 137's dependent is the debug_printf jal (insn 152, pri=4) which schedules at T-3. All backward-flow ready-times are tightly packed.
- probe: Cross-read sched2 trace T-6..T-18 against greg insn dep-lists (insn 137, 143, 145 all show 'insn_list 137 (...)') — confirms 137 anchors the debug_printf arg-flow chain
- result: The dep-DAG mechanism explains why the s2/s3 exhaustive 140-ordering sweep found the coupled fixed point: statement order in C alters expand-time insn numbering (which breaks the T-14 UID tie one way or the other) but any dep-chain shortening would require semantic changes to the debug_printf call site, which are impossible (its args are load-bearing).
- verdict: CONFIRMED

## [s7] No GCC 2.7.2 pass creates a new CODE_LABEL after flow.c that a semantically-true C construct could route through — the young-label mechanism is unreachable in this compiler+C combination (s6 frontier item #1).
- mechanism: s6 concluded: dbr creates labels too late; loop.c splits labels pre-flow; cse2/combine don't create; sched moves but doesn't create; no computed goto / macro-inlined switch / __builtin_expect available in K&R.
- probe: Enumerated `code_label N` entries in every RTL dump pass (jump/cse/loop/cse2/flow/combine/sched/lreg/greg/jump2/sched2/dbr) on the vT40 candidate; diffed the label-id set across transitions (tmp/grind/marionation_Exec/s7/labels_*.txt + jump2_label_delta.txt).
- result: FALSIFIED — jump2 (jump.c: jump_optimize, called between greg and sched2) synthesizes 3 new code_labels (591, 301, 85) not present in ANY prior pass. Label 591 is used by TWO branches (label_ref at insn 397 = check1 branch_equality, and insn 454 = an in-body branch inside SEQUENCE insn 610), proving jump2 folded two originally-distinct jump-around sequences under a single new label. Label 493 (the ORIGINAL check2 copy-skip label) was DELETED by that same fold. So the mechanism DOES exist, but jump2 places the young label at check1's position because check1 emits first and its skip-target absorbs check2's during fold. To reposition onto check2's fall-through walk (between insn 424 = beqz(a2==0) and insn 445 = move a1,s4), we'd need either (a) check2 to emit first (semantically impossible — check1 is the prior-state precheck), or (b) check2's copy-skip position to have an extra branch reference check1 lacks (no C construct provides this without a visible-byte jump). No C-level lever.
- verdict: KILLED

## [s7] The vT40 candidate reproduces masked-4 on main at s7 start; baseline unchanged from s1-s6.
- mechanism: candidate.c spliced via s6/splice_apply.py; sandbox --disable all is the honest cheat-invisible score.
- probe: python3 tmp/grind/marionation_Exec/s6/splice_apply.py memory/grind/marionation_Exec/candidate.c; & tools/wteng.ps1 main sandbox marionation_Exec --disable all; splice_apply.py --restore; git checkout -- src/system.c.
- result: score=4, build_insns=178, target_insns=179, rules_dropped=42, cheat_asm_stripped=20. src restored to HEAD (git status clean; CRLF fix via git checkout).
- verdict: CONFIRMED

## [s8] The twin cpu_side_move_dir_4's clean do_timeout shape (no arg4/arg5 locals, no pp alias, no v0 staging web — simply `debug_printf(&fmt, D_800F19C0, tbl_ptr[idx[?]], tbl_125c[idx_1494[0]], tbl_125c[idx_1494[1]])`) transplants to marionation_Exec at or below the vT40 floor.
- mechanism: twin uses value-form array indexing directly at the call site; if canonicalization at expand-time is total (as s3 y03 fused-operand-swap suggested for CANONICAL sub-expressions), the resulting expand insns should match vT40's non-pp branch.
- probe: wrote v01 (tmp/grind/marionation_Exec/s8/v01_inline_debug_args.c), spliced via s6/splice_apply.py, `& tools/wteng.ps1 main sandbox marionation_Exec --disable all` = masked 16 / 178 insns.
- result: masked 16 (regression +12 vs vT40); rejected/twin-inline-noweb-nopp-masked16.c.
- verdict: KILLED

## [s8] A hybrid — keeping the pp pointer-alias staging + arg5 named local, but expressing BOTH tbl accesses as clean s32-array indexing `tbl_125c[idx_1494[i]]` inside the call (no explicit t0 temp, no (u8*) byte-cast, no v0 staging) — reaches floor <=4 by isolating the pp alias as the sufficient scheduling lever.
- mechanism: Only untested combination in the s2/s3 y-family sweep: y01 used named `s32 arg4` locals; y03 tested operand swap on the fused form. NEW: pp-alias-present + inline-tblidx-in-call has no direct measurement in the ledger.
- probe: wrote v02 (v02_hybrid_array_index_pp.c), spliced, sandbox = masked 16.
- result: masked 16 (regression +12); pp alias alone doesn't compensate for the lost t0-web decomposition. rejected/hybrid-tblidx-pp-noweb-masked16.c.
- verdict: KILLED

## [s8] Typing the t0 web pointer as `s32 *t0` (pointer-typed, using `t0 = tbl_125c + idx_1494[0]` natural s32-index add and `*t0` deref) instead of vT40's address-value `s32 t0` with (u8*)+byte-offset casts reaches floor <=4 — pointer-type expand is not identical to address-value expand at the RTL level.
- mechanism: vT40 uses `t0 *= 4; t0 = (u8*)tbl_125c + t0` to force byte-offset addressing (a s32*4 explicit); typing as s32* lets GCC use ptr+index which lowers to (index<<2)+ptr canonically — LUIDs of the resulting insn sequence may differ.
- probe: wrote v03 (v03_t0_typed_s32ptr.c), spliced, sandbox = masked 11 / 178 insns.
- result: masked 11 (regression +7). Type axis for the t0 pointer is a NEW measured negative — confirms the address-value s32 spelling is deliberately load-bearing. rejected/t0-typed-s32ptr-masked11.c.
- verdict: KILLED
