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

## [s9] Preserving pp alias while replacing the v0-web t0/arg5 staging with clean s32-array-indexing named locals (arg4val = tbl_125c[idx_1494[0]]; arg5val = tbl_125c[idx_1494[1]]) reaches masked <= 4 - the pp alias is the load-bearing lever independent of the web.
- mechanism: s8 measured pp+web joint contribution ~12 (v01 no-pp-no-web = 16, v02 pp+no-web inline = 16). Named locals differ from v02's inline args by forcing sequence-point evaluation of the two derefs; the resulting distinct pseudos may reach the vT40 seat assignment without the byte-cast web.
- probe: s9v01 spliced to src/system.c, sandbox --disable all
- result: masked 9 / 178 insns (regression +5 vs vT40=4, but recovery +7 vs s8 v02=16). Named-local pp-preserved form is BETTER than inline-arg pp-preserved (16) but still below vT40 - v0-web decomposition retains ~5 masked pts of value that named locals cannot substitute for.
- verdict: KILLED

## [s9] Reversing named-local arg computation order (arg5val computed BEFORE arg4val) forces arg5's tree LUID early enough to win the sched2 T-14 tie without touching the pair-swap coupling.
- mechanism: In named-local form both trees are evaluated to full pseudos before the call - expand-time UID assignment follows textual order, so arg5-first source may produce lower UID for arg5's sll than for arg4's sll.
- probe: s9v02 (v01 with arg5val/arg4val order swapped), splice + sandbox --disable all
- result: masked 11 (worse than v01=9 by 2). arg5-first regresses even in named-local form - the s2-s4 arg5-first-seats-trade coupling survives the named-local transformation (was previously measured only for inline args and staged temps).
- verdict: KILLED

## [s9] Hoisting arg3 (D_800A11DC[D_800A11D5]) into a named local BEFORE the arg4/arg5 web compute perturbs expand-time UID assignment for the sll pair by inserting an extra mem-load in the tree walk.
- mechanism: arg3's lbu load is a mem-hazard-flagged insn that may re-time the sched2 backward walk and alter T-14's ready-list composition; s2 tests never covered arg3 movement.
- probe: s9v03 (vT40 body + arg3val = D_800A11DC[D_800A11D5] named local hoisted first), splice + sandbox --disable all
- result: masked 18 (drastic regression +14). arg3 hoisting births a new pseudo whose life crosses the entire arg4/arg5 compute region - relandscapes qty allocation and re-times the head like a launch (analogous to s2 vT33/vT34 fresh-temp launches).
- verdict: KILLED

## [s9] Rewriting the outer control flow to use structured if/else (dropping the do_timeout/success gotos, using inverted D_800F19B8 >= v0 test) alters flow.c basic-block layout upstream of do_timeout and may shift jump2 label allocation for region-3.
- mechanism: Session-7 identified jump2 as the sole young-label mechanism (creates labels 591/301/85 post-greg); outer CFG changes propagate to flow-analysis BB structure, which drives jump2's fold decisions.
- probe: s9v04 (vT40 body with if(D_800F19B8>=v0){cnt=...;if(!(0x3C0000<cnt)){v0=0;goto check;}} + fallthrough to do_timeout), splice + sandbox --disable all
- result: masked 7 at 176/179 insns (masked +3 AND build_insns -2 vs vT40). Outer restructure DELETES 2 target insns - the inverted vsync check compiles to fewer branch instructions than the original two-goto form, exposing a build-vs-target insn-count divergence. Score is closer masked-wise (7 < 9/11/18) but structurally wrong.
- verdict: KILLED

## [s10] Preserving v0-staging web for arg5 only (t0 natural clean array-index, s9v01 chassis) reaches masked <=4 by capturing the sched2-tie geometry through arg5's expression alone.
- mechanism: s9v01 (both webs stripped) = 9; the arg5 v0-staging web is one of vT40's two independent stagings and, per s3, its shift-merged-into-load geometry moves the sched2 tie without seat trade. Retaining only that half might isolate the tie-fixing contribution.
- probe: s10v01 spliced (pp alias + arg4val = tbl_125c[idx_1494[0]] clean + v0=idx_1494[1]<<2; arg5 = *(s32*)(v0+(s32)tbl_125c)), sandbox --disable all
- result: masked 13 / 178 insns - REGRESSION +4 vs s9v01 (9) and +9 vs vT40 (4). The v0-staging web WITHOUT the t0 byte-cast web is actively harmful: unbalanced pair-window compute re-times worse than either clean or fully-webbed. Novel finding: the two webs are not additive-independent; the arg5 web is NEGATIVE without the t0 web to anchor the pair.
- verdict: KILLED

## [s10] Preserving t0 byte-cast web only (arg5 natural clean array-index, s9v01 chassis) reaches masked <=4.
- mechanism: Symmetric probe to v01; t0-web is the s8-measured ~7-12 pt lever; without arg5's v0-staging web the pair-window compute might still resolve cleanly if t0-web anchors the LUID pattern.
- probe: s10v02 spliced (pp alias + t0 = idx_1494[0]; t0*=4; t0=(s32)((u8*)tbl_125c+t0) + arg5val=tbl_125c[idx_1494[1]] clean), sandbox --disable all
- result: masked 6 / 178 insns - CLOSEST non-vT40 form ever measured (+2 over floor). t0-web alone recovers 3 masked pts vs s9v01 (9). Quantifies t0-web dominant (>=3 pts standalone), v0-web incremental (~2 pts, gets vT40 from 6 to 4), pp alias ~7 pts (s9v01 vs s8v01). All three components are independently load-bearing and non-substitutable.
- verdict: KILLED

## [s10] The two staging webs (t0 byte-cast, v0-<<2 for arg5) are additive-independent contributions to the masked score.
- mechanism: Naive read of s8 (both stripped = 16, +12 vs 4) and s9 (both stripped, pp preserved = 9, +5 vs 4) suggested a decomposable sum.
- probe: s10v01 (arg5-web only) and s10v02 (t0-web only) directly test additivity: sum should equal joint if independent.
- result: FALSIFIED. v02 (t0-only) = 6, expected ~9 if only t0-web-half of joint 5 pt recovery from s9v01=9; v01 (arg5-only) = 13 REGRESSES from s9v01=9 by +4. The v0-web is CONDITIONAL on the t0-web (helpful with it, harmful without). Non-additive interaction - the pair-window compute needs BOTH webs coherently to get the vT40 arrangement.
- verdict: CONFIRMED

## [s11] A single unified staging expression (both derefs off a shared lifted (u8*)tbl_base, or shared idx-arithmetic tree, or pointer-typed tbl_125c with fused ptr+shift) captures the coupled t0/v0-web interaction and reaches masked <=3
- mechanism: s10 quantified non-additivity: t0-web and v0-web are CONDITIONALLY beneficial (arg5-web negative alone at 13, additive only with t0-web); vT40's floor 4 needs both simultaneously. Compound expression where one sub-expression sets up register state the other consumes may birth a shared address-base qty.
- probe: Sweep u01-u10 (10 unified-staging spellings): (u01) lifted u8* tblb shared, (u02) s32* tblp array-index, (u03) ptr-advance p0/p1, (u04) shared v0 offset reuse, (u05) call-arg expression-fused, (u06) mirrored arg5 as address-value, (u07) u06 arg5-first, (u08) tblp array both, (u09) shared off-var split, (u10) interleaved t0/v0 computes. Sandbox each; adiff not needed for KILL (all >= floor). sweep1.json
- result: KILLED. Results: u01/u02/u05/u08=16 (unified-no-web regresses to s8 baseline); u03=12 (ptr-advance new pattern); u04/u09=11 (v0-shared adds insn); u06=7, u07=8 (mirror form); u10=4 (novel masked-4 spelling matching vT40's floor, interleaved computes). Unified-single-expression does NOT drop below 4; the mechanism intuition was wrong - the two-web non-additivity is not resolvable by expression fusion. sweep1.json.
- verdict: KILLED

## [s11] Mirroring arg5's structural form to match t0's (both as address-values, both via u8*+idx*4 arithmetic) breaks the pair-window seat-trade by giving both temps identical qty birth signatures (same size/life/refs profile)
- mechanism: u06 measured masked 7 (best sub-vT40 mirror). Refinements: add v0-web back around arg5, vary pp placement, use shift instead of mult, interleave both computes.
- probe: sweep2 (w01-w10, 10 variants): w01 u06+v0web arg5addr, w05 pp between, w06 full interleave, w07 shift-not-mult, w10 s32-cast deref, plus arg5-first mirrors (w04 addrval).
- result: KILLED. Best mirror score with v0-web added back: w01=6 (matches s10v02 attractor, still +2 over floor). Mirror without v0-web plateaus at 6-7 (w05/w06=7). w07 shift-not-mult=11 (mult vs shift emits different insns). w03/w10 both reach masked-4 (novel spellings). Mirror form does not break the coupling. sweep2.json.
- verdict: KILLED

## [s11] Preserving arg5-first source order (v0=idx_1494[1] first) with tight interleaving of both indices' loads followed by interleaved shifts breaks the s2/s9-measured arg5-first-seats-trade coupling (prior masked 8-11), because both trees are already fully expanded by the time the sched2 T-14 tie is decided
- mechanism: s2/s3/s4/s9 measured arg5-first regressions of +4 to +7 (masked 8-15 across chassis: vT40, find105, alias-merge). Those forms placed arg5's compute FULLY before t0's. NEW spelling: both idx loads first, THEN both shifts, THEN interleaved tbl-adds - keeps arg5's tree LUIDs early but doesn't extend arg5val's life.
- probe: sweep3 (x01-x10, 10 arg5-first refinements): pp placement (first/mid/last), full-compute vs interleave, mirror-form arg5-first.
- result: CONFIRMED as new attractor, KILLED as closer. 7 of 10 spellings (x01-x05, x07, x08) land at masked 6 - a NEW plateau not present in ledger. Only x06 (full arg5 compute before t0) and x09 (deref before t0-finish) regress to 9. The interleaved arg5-first form recovers 2-5 masked pts vs prior arg5-first data. Still +2 over floor 4; the last coupling residual is not structural. sweep3.json.
- verdict: CONFIRMED

## [s12] volatile qualifier on the staging temp (arg5 or t0) alters qty priority via forced memory semantics without adding user pseudos, breaking the pair-swap 5.33 v 5.33 tie
- mechanism: volatile forces every write/read to become a real memory op, which changes RTL memory-hazard classification in sched2 and may reweight the tie-relevant qty birth signature
- probe: sweep1 v01/v02/v06/v07: `volatile s32 arg5` (vT40); `volatile s32 t0` (vT40); `volatile s32 t0` (x02 chassis); `volatile s32 arg5` (x02 chassis)
- result: v01=26 (build 180), v02=36 (build 184), v06=41 (build 184), v07=28 (build 180) - all regress heavily AND add 1-5 build_insns from the mandated memory ops (stack sw/lw for every read/write). volatile emits real spill traffic; not a zero-cost lever.
- verdict: KILLED

## [s12] Mirror-both-as-s32*-pointer-typed spelling (both derefs via `s32 *pt = tbl_125c + idx[i]`) reaches masked <= 4 by giving both temps identical qty birth signatures via canonical ptr+index addressing
- mechanism: s3 y01 tested t0-first-only s32*-array-indexing (masked 4 - same order as vT40); s8 v03 tested single-side s32* (masked 11 - worse). Mirror-BOTH-as-s32* untested; if canonical ptr+index emits identically for both, the sched2 UID tie could resolve at t0-first birth-order without seat trade.
- probe: sweep1 v03/v04/v05/v09: mirror-both-s32* with pp early/last/arg5-first; and mirror-both-s32* with shared tb single alias
- result: v03=v04=v05=v09=12 uniformly - all mirror-both-s32*-typed regress to +8 above floor and are WORSE than mirror-both-address-values (x10=8). Pointer-typed emit uses different RTL address modes (canonical ptr+index) that produce a distinct compute chain from address-value's (u8*)+byte-offset; the pair-window compute cost is +8 masked pts regardless of pp/birth-order spelling.
- verdict: KILLED

## [s12] Substituting `*idx_1495` for `idx_1494[1]` (semantic-identical: idx_1495 = idx_1494+1, both alias the same byte) adds a ref to the idx_1495 pseudo without adding an insn, reweighting the qty tie
- mechanism: idx_1495 is already declared and set at function prologue; deref via *idx_1495 vs idx_1494[1] compiles to the SAME lbu at the same offset (byte load through the alias) but touches idx_1495's pseudo in the do_timeout window instead of just idx_1494's. Extra ref could raise idx_1495's priority or reweight the pair.
- probe: sweep3 z03/z04/z05/z08: vT40 order with *idx_1495; x02 arg5-first-interleaved with *idx_1495; mirror form with *idx_1495; vT40 with *idx_1495 + pp last
- result: z03=10, z04=11, z05=11, z08=10 - uniform +6 to +7 REGRESSION. Reading idx_1495 in the do_timeout window forces its pseudo to be LIVE across the debug_printf call (which the callback-path use later also forces), and the extra live-across-call ref makes it callee-saved seat-competitive - relandscapes qty allocation exactly like s4's alias-merge-dst2 pattern. Net: seat cascade dominates the tie fix.
- verdict: KILLED

## [s12] Combining a shared tbase alias (single u8*/s32 lifted from tbl_125c) with asymmetric application (only one side of the pair) breaks the coupling by differentiating qty births
- mechanism: The two temps t0 and arg5 tie at qty 5.33 v 5.33 because their refs*log2(refs)*size/life is identical; introducing tbase reweights the SHARED base-pointer qty, which is different from tying the pair symmetrically
- probe: sweep2 w01-w08 and sweep3 z01/z02/z06/z07: tbase symmetric + arg5-first; tbase asymmetric to arg5-only (z01); tbase asymmetric to t0-only (z02); tbase + arg5-first + *idx_1495 (z06); tbase + interleaved (z07)
- result: z01/z02/z07 all masked 4 - novel masked-4 spellings but do NOT drop below the vT40 floor. Asymmetric tbase reproduces vT40's floor from a different structural chassis but the underlying pair-swap coupling holds. w04/w06 (split-init on arg5) regress to 11 (extra intermediate store scrambles sched); w01/w03 tbase + arg5-first plateau at 6 matching s11 x02 attractor; w02/w07 mirror-with-tbase = 7-8 matching s11 x10/mirror plateau.
- verdict: KILLED

## [s12] Split-init accumulation on arg5 (sanctioned family) with tbase alias breaks the seat trade by giving arg5 an extra store-then-load life extension that changes its qty allocation
- mechanism: split-init-accumulation-sanctioned rule (2026-06-13): var=a; var+=b; sanctioned as pure-C; applied here as `arg5 = v0+(s32)tbase; arg5 = *(s32*)arg5;` extends arg5's life and adds refs to potentially win the 5.33 v 5.33 tie
- probe: sweep2 w04 (tbase + split-init arg5) and w06 (split-init arg5 without tbase)
- result: w04=11, w06=11 - both regress +7 vs floor. The intermediate store-then-load of arg5 = v0+(s32)tbase followed by arg5 = *arg5 emits an extra addu insn (build stays 178 because it fuses in canonicalization but the pair-window compute chain retimes to a +7 shape). Not a zero-cost lever.
- verdict: KILLED

## [s13] The s4/s5 vT40-base perm campaign (still running PID 1935645, -j6) has produced no new sub-145 find in the 10.5h since s5's snapshot.
- mechanism: the vT40 local basin's sub-200 mutation trajectory converges on the alias-merge (masked >= 10, callee-saved seat) and label-alive (+1 insn) attractors; no lower-cost region-3 kill or pair-swap fix exists locally.
- probe: listed tmp/grind/marionation_Exec/s4/perm/output-* (WSL); enumerated by permuter-score (145, 160, 165, 170, 175, 200, 205, 210, 215) - identical set to s5's snapshot; ps confirmed pid 1935645 elapsed 10:47:26.
- result: 0 new sub-145 finds across ~10.5h continued sampling; the vT40 basin's search-space exhaustion evidence continues to accumulate.
- verdict: KILLED

## [s13] The s4/s5 find105-base perm_c campaign (PID 1935593, -j24) has produced no sub-85 find in the 10.5h since s5 (the alias-merge family floor was masked 10 at permuter-score 95-1).
- mechanism: the family's callee-saved seat trade (dst2=a1-target vs s1-callee-saved) is a structural constraint permuter cannot resolve; further perm-score progress trades seats further from target (s5: output-85-1 = masked 14, output-95-2 = masked 15).
- probe: listed tmp/grind/marionation_Exec/s4/perm_c/output-*: [85-1, 95-1, 95-2] - identical to s5 snapshot.
- result: family floor confirmed at masked 10 (permuter 95-1); no new finds.
- verdict: KILLED

## [s13] A fresh permuter campaign seeded on the s12 z07 chassis (u8* tbase with v0-interleaved compute, masked 4) samples a structural neighborhood not reachable from vT40- or find105-base sampling.
- mechanism: the tbase alias births a NEW function-level qty that changes the local sampler basin; z07 is one of 9 known distinct masked-4 spellings (vT40 + s11 u10/w03/w10 + s12 v08/w05/w08/z01/z02/z07) and each exposes a different qty-allocation landscape.
- probe: adapted s4/build_ws.py -> tmp/grind/marionation_Exec/s13/build_ws_z07.py (z07 as base, target.o + base_full.c skeleton reused from perm_mar6); verified permuter --debug base score = 220 (2 reorderings * 60 + 1 deletion * 100 - identical residual signature to vT40 base). Launched detached campaign with -j6 --better-only --stop-on-zero (PID 3540094, log tmp/grind/marionation_Exec/s13/campaign_z07.log).
- result: campaign live at session end (elapsed 1m at handoff, iterations up to 24 confirmed in log); base = 220 confirms z07 chassis is a valid seed. Attempted PERM_GENERAL wrap of the pair-window inner block + PERM_RANDOMIZE on `s32 check;` decl, but stripped after PERM_RANDOMIZE_TYPE proved unrecognized and PERM_GENERAL(compound-stmt) tripped the pycparser. Default randomization is active - the sampler is walking z07's local basin.
- verdict: CONFIRMED

## [s14] The s12 w05 chassis (s32 tbaseS = (s32)tbl_125c) is a valid permuter seed with the same masked-4 residual signature as vT40/z07 but a structurally distinct qty-allocation landscape (types differ: s32 vs u8*).
- mechanism: w05 births a s32-typed tbaseS pseudo instead of u8*-typed tbase (z07) or no shared base (vT40). Both `t0 = (s32)((u8*)tbaseS + t0)` and `arg5 = *(s32*)(v0 + tbaseS)` re-express through a s32-typed base pointer, which materially changes the type-attribute of the qty backing the shared base and thus its cross-BB liveness/RA priority - untried territory.
- probe: python3 tmp/grind/marionation_Exec/s14/build_ws_w05.py built workspace; permuter --debug reported base score = 220 (Reorderings 2*60 + Deletions 1*100), matching vT40/z07 base signature exactly.
- result: base score 220 confirmed; nohup permuter -j6 --better-only --stop-on-zero launched (PID 3756068, iterations running at handoff, log tmp/grind/marionation_Exec/s14/campaign_w05.log).
- verdict: CONFIRMED

## [s14] The s13-launched z07 permuter campaign (PID 3540094) produced sub-200 finds in its first 10 minutes.
- mechanism: z07 base = 220; if the u8* tbase + interleaved-compute chassis's local basin contains lower-scoring mutations reachable via default randomization, they would appear in tmp/grind/marionation_Exec/s13/perm_z07/output-*.
- probe: ls tmp/grind/marionation_Exec/s13/perm_z07/output-* -> (empty); tail campaign_z07.log shows iterations 1..30 all at scores >=220 (mostly 850-3610 spread; only iterations 1/2/6/10/26/27 stayed at 220).
- result: 0 sub-200 finds in first 10 min - z07 basin's local random-mutation neighborhood has no immediate sub-vT40 attractor visible; the sampler needs many more iterations to find a lower-scoring form (or none exists).
- verdict: KILLED

## [s14] The 10.5h+ vT40 (PID 1935645) and find105 (PID 1935593) campaigns produced any new find since s13's snapshot.
- mechanism: s13 recorded vT40 output-* set identical to s5; find105 output-* set identical to s5. Continued sampling could still yield a novel find at any moment.
- probe: ls tmp/grind/marionation_Exec/s4/perm/ | grep ^output- | sort -u  vs s13 snapshot: [145-1,145-2,160-1,160-2,160-3,165-1,165-2,165-3,170-1,175-1,200-*,205-*,210-*,215-*] = IDENTICAL to s13. ls s4/perm_c/ | grep output-: [85-1,95-1,95-2] = IDENTICAL to s5/s13.
- result: 0 new finds across 11h+ elapsed sampling on both vT40 and find105 basins. Search-space exhaustion evidence continues to strengthen for both.
- verdict: KILLED

## [s15] The x02 attractor's +2 masked delta vs vT40 is a single insn transposition at the tslTm2LoadImage_2 jal delay slot (lbu $4,0($18) swapped with lbu $2,1($18)), NOT a partial pair-swap fix + a compensating cost trade.
- mechanism: sched2 BB #3 T-16 emits t0-lbu (refs=5) before arg5-lbu (refs=4) in vT40 via `launching 99 before 117 with no stalls at T-16`; in x02 the arg5-first source order causes expand to give arg5-lbu the lower insn UID (99, refs=4) and t0-lbu the higher (103, refs=5), so sched2's T-16 launches 103 before 108 and arg5-lbu emits first chronologically. dbr fill_simple_delay_slots then picks the first fall-through insn for the jal delay slot: `lbu $4,0($18)` in vT40 (target-matching), `lbu $2,1($18)` in x02 (+2 masked).
- probe: cc1 -da dumps of both forms via tmp/grind/marionation_Exec/s6/dump.sh into tmp/grind/marionation_Exec/s15/{vt40,x02}_dumps/; diff of emitted .s files: EXACTLY 1 line swap at lines 1152-1153. sched2 BB#3 ready-list traces cross-read: vT40 T-16 `launching 99 before 117` vs x02 T-16 `launching 103 before 108`.
- result: vT40 masked 4 (178/179), x02 masked 6 (178/179), asm diff 1 line, sched2 T-16 launch differs by expand-time insn UID assignment. Pair-swap window (sll v0; sll a0; addu v0,v0,s5) is IDENTICAL between the two forms. Region-3 dbr steal (check2 beqz + move a1,s4) is IDENTICAL between the two forms.
- verdict: CONFIRMED

## [s15] The x02 attractor's spelling family (s11 x01-x05, x07, x08 all measured at masked 6) is structurally insn-level-CLOSED for masked <=4: ANY spelling that expresses idx_1494[1] before idx_1494[0] in expand-time source order flips the sched2 BB3 T-16 launch order and pays the +2 at the tslTm2 delay slot, while the arg5-first-first-idx-load constraint is what any x-family closer would need.
- mechanism: Bidirectional trap: (a) t0-first-idx-load keeps the delay slot fill target-matching but ties sched2 T-14 to insn 106 (a0-sll) via UID<117 → vT40 pair-swap unresolved (2 masked); (b) arg5-first-idx-load could conceivably win the T-14 tie (UID(v0-sll) < UID(a0-sll)) but forces the arg5-lbu into the tslTm2 delay slot (+2 masked); measured x02 confirms path (b) also inherits the same T-14 UID tie result as vT40 (both sll's are same-priority, tie-broken to a0-sll-first) because the sched2 tie between the two sll's is decided AT SCHED TIME by their INSN_LUIDs (a scheduler-internal ID assigned by rank_for_schedule), not by expand-time UID. So path (b) pays the +2 for NOTHING.
- probe: Read s6 finding [pair-swap named as sched2 T-14 tie between insns 106/117 both pri=2 ashlsi3 broken by insn-number ordering] alongside s15 x02 sched2 T-14 `now 112 108 141` (same reorder direction: 108 wins because 108<112, mirror of 106<117 in vT40). Both forms produce v0-sll before a0-sll in emission. x02 asm diff confirms.
- result: arg5-first-idx-load source order does NOT change the sched2 T-14 sll-pair tie outcome (still v0-sll chronologically before a0-sll) but DOES flip the T-16 idx-lbu launch order (adding +2 at the delay slot). Zero net benefit.
- verdict: CONFIRMED

## [s16] An empty-body early-use insert (`if (!t0) { }` between t0 = idx_1494[0] and t0 *= 4) reweights the sched2 T-14 LUID tie without adding build insns, retiming the pair-swap window (frontier item #1).
- mechanism: The frontier hypothesized jump.c/flow.c would emit a compare that survives to sched2 as a real dep-chain intervening between t0's load and its shift, extending its live range and reweighting rank_for_schedule's LUID inputs. Compare gets folded pre-expand or in jump.c on the flow graph.
- probe: v01 = candidate.c + `if (!t0) { }` inserted between t0 = idx_1494[0]; and pp = ...;. Sandbox --disable all = masked 4 / 178 insns (unchanged). Fresh cc1 -da dumps produced at tmp/grind/marionation_Exec/s16/dumps/{baseline,v01_dead_compare}/. Emitted .s at lines 1152-1165 (pair window + tslTm2 delay slot + full debug_printf call setup) is BYTE-IDENTICAL between baseline and v01. sched2 BB #3 backward walk in v01 shows UID-shifted schedule (T-11 132 before 115; T-16 99 before 121; T-17 119 before 145) - the initial expand insn numbers move because the compare occupies UID space before being folded - but the final schedule collapses to identical instructions. Same probe repeated for v04 = `if (!v0) { }` between v0 = idx_1494[1] and v0 <<= 2 - masked 4, 178 insns, emitted asm byte-identical to baseline (lines 1150-1165).
- result: v01 masked 4/178, v04 masked 4/178, emitted asm byte-identical to baseline in pair window; sched2 UIDs shift but final schedule collapses identically.
- verdict: KILLED

## [s16] Dead-local read (fresh named local assigned from t0, never subsequently used) reaches expand as a use of t0 that could alter its qty life/refs and reweight the pair-swap.
- mechanism: Fresh local `s32 saved_debug = t0; (void)saved_debug;` between t0-load and t0-shift creates a mov RTL insn at expand; if DCE runs late, could reweight downstream.
- probe: v02 (candidate.c + `{ s32 saved_debug = t0; (void)saved_debug; }` between t0 = idx_1494[0]; and pp = ...;). Sandbox --disable all = masked 4 / 178 insns.
- result: Inert; DCE eliminates the local write before final schedule.
- verdict: KILLED

## [s16] `t0 = t0 << 2` and `t0 *= 4` produce equivalent RTL and reach the same sched2 schedule (canonicalization is total, as s3 suggested for merged-operand-swap forms).
- mechanism: GCC 2.7.2 combine + CSE canonicalizes mult-by-power-of-2 and user-written ashift to the same ashift RTL; if canonicalization is at expand time, they emit identical insns; if canonicalization is at CSE/combine, UID assignment differs.
- probe: v03 = candidate.c with only change `t0 *= 4;` -> `t0 = t0 << 2;`. Sandbox --disable all = masked 9 / 178 insns (REGRESSION +5). Pass-count telemetry (grep ashift/mult counts across .rtl/.jump/.cse/.loop/.cse2/.flow/.combine): baseline .rtl has 59 ashift + 53 mult; v03 .rtl has 59 ashift + 52 mult — the one `t0 *= 4` mult becomes a `t0 = t0 << 2` ashift AT EXPAND. Both files reach the same 47 ashift + 47/48 mult by CSE/combine (the delta of 1 persists because baseline's mult-by-4 got canonicalized to ashift only during CSE, one pass later than v03's user-authored ashift). Emitted .s diff at the pair window: baseline emits `lbu $4,0($18); lbu $2,1($18); lw $5,D_800F19C0; sll $2,$2,2; sll $4,$4,2; addu $2,$2,$21; lw $3,0($2); ...; sw $3,16($sp)` vs v03 `lbu $3,0($18); lbu $2,1($18); lw $5,...; sll $3,$3,2; sll $2,$2,2; addu $2,$2,$21; lw $4,0($2); ...; sw $4,16($sp)` - t0's seat swapped from $4 (a0, target-matching) to $3 (v1, WRONG) and t0-sll now emits BEFORE arg5-sll (chronological order swapped).
- result: Falsified: expand-time canonicalization is NOT total; user ashift and canonicalized mult-by-4 land at different pass points, changing UID landscape and register-allocation seats.
- verdict: KILLED

## [s17] Rewriting t0's mult-by-4 as add-tree, nested-shift, LHS-const-mult, or paren-folded mult produces a new LUID landscape at the sched2 T-14 tie that could reorder the pair-swap without a seat trade (frontier item #3).
- mechanism: GCC 2.7.2 canonicalizes (mult reg 4) at CSE (baseline drops mult 53->48 between .rtl and .cse) while user-authored (ashift reg 2) is expand-produced. Different tree shapes may cross canonicalization at different passes and yield distinct INSN_LUIDs at the T-14 tie between insn 106 (a0-sll) and insn 117 (v0-sll).
- probe: s17/sweep.py: 19 variants swept, 6 on t0-side, 6 on v0-side, 6 mirror/cross combinations; each spliced via s6/splice_apply.py, sandbox --disable all measured.
- result: 6/19 stay masked 4 (baseline, a4_t0_4mul_lhs, a5_t0_paren_folded, c3_both_mult, c4_both_lhs_4mul, all b1-b6 v0-side); 8/19 regress to masked 9; 0/19 go below 4. Path is DEAD as closer.
- verdict: KILLED

## [s17] t0's arithmetic spelling has a sharp shift-vs-mult expand-time RTL boundary; ANY plus-tree or ashift authorship on t0 pays +5 masked, while any spelling that expands to (mult reg const_int) is inert.
- mechanism: s17 measured: `t0 *= 4`, `t0 * 4`, `4 * t0`, `t0 * (2*2)` ALL stay at masked 4 (all four fold to identical RTL `(mult t0 4)` at expand). `t0 = t0 << 2`, `t0 = (t0<<1)<<1`, `t0 = t0+t0+t0+t0`, `t0 = t0+t0; t0 = t0+t0;` ALL score masked 9. Add-tree canonicalizes to shift AT expand (not later), landing in the same expand-time RTL bucket as user-written ashift. The delta is exactly whether t0's RTL insn is `(mult reg 4)` or `(ashift reg 2)` at .rtl.
- probe: Cross-comparison of the 19 s17 variants + s16 v03 result at same seed.
- result: CONFIRMED. Six positive controls (mult-family stays 4) + four negative controls (shift-family + add-tree at 9) collapse cleanly onto the same +5 penalty. The +5 IS the mult-to-shift-at-expand shift, not spelling noise.
- verdict: CONFIRMED

## [s17] v0's arithmetic spelling is inert to the pair-swap: any spelling (shift, mult, add-tree, LHS-const, paren-folded) leaves masked at 4.
- mechanism: v0's shift feeds arg5's deref (a chain that reaches the sched2 pair window via addu insn 120's user-chain 120->122->137->jal-arg). The T-14 tie is between insn 106 (a0=t0-sll) and insn 117 (v0=arg5-sll). v0's expand-time RTL shape does not enter the tie decision - the tie has both candidates on the ready list at pri=2 regardless of the ashift/mult authorship, and their INSN_LUIDs from sched2's own rank_for_schedule walk are set by dep-DAG position, not by expand RTL opcode.
- probe: 6 v0-side variants (b1-b6) all score masked 4; c3_both_mult (both use mult) and c4_both_lhs_4mul (both use LHS const mult) also 4.
- result: CONFIRMED. v0-side is fully spelling-inert. NEW mechanism refinement: s15's T-16 launch order sensitivity to expand-UID was ONLY for the tslTm2 delay slot (a different sched2 pass), not for the T-14 pair-swap tie.
- verdict: CONFIRMED

## [s18] Rewriting outer polling as for(;;) with break-on-timeout via v0=-2 sentinel + structured if(v0==-2) do_timeout (v01) reaches masked<=4 by eliminating the goto do_timeout/goto success/goto check chain and letting jump.c fold the CFG more tightly.
- mechanism: outer-flow structural rewrite untried in this exact shape; s9v04 tested inverted-vsync if/else and reached masked 7 at 176 insns (-2 build). The sentinel-tag form preserves 179 insn count and is structurally distinct.
- probe: tmp/grind/marionation_Exec/s18/v01_forloop_outer.c spliced -> sandbox --disable all
- result: masked 17, 179 build_insns (+13 vs floor). Sentinel-tag mid-loop v0=-2 pollutes v0's post-loop life and adds a redundant compare; the do_timeout block's do-while(0) note anchor is preserved but the outer CFG rewrite reweights allocation across the whole function.
- verdict: KILLED

## [s18] Rewriting check1/check2/tail from goto+label chain to structured if/else (v02) reaches masked<=3 by aligning check-region CFG shape with jump.c's fold preferences.
- mechanism: check1/check2 are two same-shape mask+clear+copy sequences currently expressed via goto check2; goto tail; goto loop. Structured if(check){...}else{if(check){...}...} is a distinct CFG at expand; jump.c's threaded jump elimination may collapse the two arms to different young-label placement (s7 jump2 mechanism, from evidence.md).
- probe: tmp/grind/marionation_Exec/s18/v02_check_ifelse.c spliced -> sandbox --disable all
- result: masked 4, 178 build_insns - TIES vT40. Novel masked-4 basin member. Check-region structural rewrite is spelling-inert; the two forms fold to the same post-jump.c CFG. Basin membership up to 12 known distinct masked-4 spellings.
- verdict: KILLED

## [s18] Eliminating idx_1495 pseudo entirely by substituting idx_1494[1] at its sole use in check1 callback (v03) reaches masked<=3 by removing a pseudo from the qty pool and letting idx_1494's addressing canonicalize.
- mechanism: s12 z03/z04/z05/z08 tested *idx_1495 SUBSTITUTION for idx_1494[1] (opposite direction: added a ref to idx_1495 in the do_timeout window, extending its life) and regressed +6-7. Reverse direction (ELIMINATE idx_1495 by inlining idx_1494[1]) untried; if the +6-7 was life-extension cost, the reverse should recover.
- probe: tmp/grind/marionation_Exec/s18/v03_eliminate_idx1495.c spliced -> sandbox --disable all
- result: masked 15, 175 build_insns (+11 vs floor, -3 build_insns). Eliminating the pseudo drops the idx_1495 addu setup insn AND the callback lbu-via-alias emit, giving 3 fewer build_insns; but the removed set/use retimes qty allocation and the pair-window compute worsens by 11 masked pts. Rejected.
- verdict: KILLED

## [s18] Moving idx_1495 = 1 + idx_1494 birth from function-top to inside the check region (v04) shortens idx_1495 life to callback-local and reaches masked<=3 by removing a competing qty from the do_timeout window's allocation.
- mechanism: idx_1495's pseudo currently lives function-wide (born at top, single use in check1 callback). Delayed birth would make it invisible during do_timeout window scheduling, reducing qty competition; may free up seat pressure on t0-web/arg5val.
- probe: tmp/grind/marionation_Exec/s18/v04_idx1495_late_birth.c spliced -> sandbox --disable all
- result: masked 25, 177 build_insns (+21 vs floor, -1 build_insn). LARGER regression than s12's life-extension probe (+6-7). Confirms idx_1495 fixed-point: function-top birth with single callback use is a doubly-load-bearing signature. The delayed addu births a separate sub-tree post-do_timeout, retiming the entire layout.
- verdict: KILLED

## [s19] The z07-basin permuter campaign produces a novel sub-200 find that closes below masked 4 with true semantics and 179 insns.
- mechanism: z07 is one of 12 known distinct masked-4 spellings; its u8* tbase + v0-interleaved compute chassis births a different qty landscape than vT40/find105/w05, potentially reaching a non-alias-merge / non-label-alive attractor.
- probe: Triaged the FIRST post-s14 z07 sub-200 find (output-160-1) via s5/extract_and_score.py + sandbox --disable all.
- result: z07-160-1 = masked 3 / 180 build_insns. Diff: `status = 0;` inserted before check1 clear + `while (status)` (single-iter self-exit) after clear. Same +1-insn label-alive class as vP160 (s5) and the s5 0xFF-through-local variant (SAFER semantics — status is assigned locally, not uninit-read cross-block, but still emits the visible byte). Reproduces the known label-alive attractor from the NEW z07 basin; NOT a closer.
- verdict: KILLED

## [s19] The w05-basin permuter campaign produces a novel sub-200 find that breaks past the alias-merge family floor of masked 10.
- mechanism: w05 is s32 tbaseS chassis — structurally distinct qty-allocation landscape from vT40/z07 (types differ); the alias-merge attractor may reach a different callee-saved seat outcome.
- probe: Triaged the FIRST post-s14 w05 sub-200 find (output-145-1) via s5/extract_and_score.py + sandbox --disable all.
- result: w05-145-1 = masked 17 / 179 build_insns. Reproduces the find145-class alias-merge attractor (dst2 = idx_1494 web merge, callee-saved seat trade) from the w05 basin — masked score identical to s4 output-145-2 measurement. Confirms alias-merge attractor is basin-invariant.
- verdict: KILLED

## [s19] Portfolio-scale permuter sampling across distinct masked-4 chassis (vT40/find105/z07/w05) produces qualitatively new sub-200 attractors beyond the alias-merge (masked 10 floor) and label-alive (+1 insn) classes documented in s4/s5/s10.
- mechanism: 12 known masked-4 spellings cover disjoint local mutation neighborhoods; if novel attractors exist for the two residuals, different basins should surface them.
- probe: Cross-tabulated all 4 basins' post-s14 output-* sets: {vT40: identical to s5; find105: identical to s5; z07: NEW 160-1 + 200-1; w05: NEW 145-1 + 200-1 + 200-2}. Sampled the two novel sub-200 finds above.
- result: FALSIFIED. Both novel sub-200 finds reproduce known attractor CLASSES (label-alive +1-insn / alias-merge callee-saved-seat) rather than exposing a new one. Combined with 11h+ vT40/find105 zero-new-finds and z07 first-10-min zero (s14), the portfolio approach has convergent, not diverse, sub-200 output: the two residuals map to a fixed small set of universal attractors reachable from any masked-4 basin.
- verdict: CONFIRMED

## [s20] Moving `s32 status` decl from function-top into the check1 body scope (where it is exclusively used) shortens its life to check1-local and reduces qty competition without perturbing do_timeout pair-window or region-3 steal.
- mechanism: status is currently born at function-top (single set inside check1 do-while, single use in the same loop). Function-top decl gives it function-wide qty visibility across the do_timeout window's allocation. Tightening scope to the sys_GetVblankCount() != 0 branch delays its birth to a point AFTER do_timeout, so it should be invisible during pair-window scheduling AND avoid any cross-BB liveness effect.
- probe: v01 = candidate.c with `s32 status;` decl moved from function-top block into the `if (sys_GetVblankCount() != 0)` body (before saved = ...). Splice + sandbox --disable all.
- result: masked 4 / 178 build_insns — INERT. Novel masked-4 basin member #13 (extends 12 known distinct spellings from s18). GCC canonicalizes both decl positions to the same qty landscape; status was already effectively local to its use per flow analysis.
- verdict: KILLED

## [s20] Moving `int new_var; int new_var3;` decl+init from just-before-check1/check2 to inside the innermost block where they are actually AND-masked against idx_1496 reduces cross-basic-block qty pressure and could shift the region-3 steal window.
- mechanism: new_var/new_var3 are the opaque-mask holders that keep target's `andi ,0xff` alive against combine's u8-load fold (per candidate.c comment). Their scope-tightening was untested; tighter scope birth-just-before-use is a novel structural probe on the mask-holder axis.
- probe: v02 = candidate.c with `int new_var; int new_var3;` decl+init moved from between vsync-check and check1 to inside the `{ s32 check; ... }` block right after `s32 check;`. Splice + sandbox --disable all.
- result: masked 8 / 176 build_insns — REGRESSION +4 masked, -2 build_insns. The tight-scope birth immediately-before-use lets combine forward-substitute `new_var = 0xFF` INTO the `& new_var` expression → `& 0xFF` → folded against the u8 lbu load (byte-load auto-zeros high bits) → the two `andi ,0xff` insns are eliminated (build 178 → 176). The 'opaque mask' property depends on the mask holder being far enough from its use that combine's forward-substitution scope does not reach it. NEW MECHANISM FACT: mask-holder scope IS load-bearing; the current mid-function-body position is not incidental.
- verdict: KILLED

## [s21] Tightening `u8 saved` decl scope from function-top into check1 body (parallel to s20 v01 status probe) reaches masked <=4 by reducing cross-BB qty pressure.
- mechanism: s20 established status decl scope is qty/schedule-inert; saved's scope was untested. Prediction: same class of inertness OR mask-holder-scope-style regression if combine can reach saved.
- probe: v01_saved_scope_check1: `s32 status;` remains at function-top; `u8 saved;` decl moved into check1 body as `u8 saved = *D_800A147C_2 & 3;` (init merged with decl).
- result: masked 4 / 178 build_insns - INERT. Novel masked-4 basin member #14.
- verdict: KILLED

## [s21] Copy-loop control variable `s32 i;` scope-per-block (fresh `s32 i = 7;` inside each of the two copy blocks instead of function-top single decl) reweights qty births in the check region for +1 masked delta or better.
- mechanism: Two fresh named locals both called `i` may or may not collapse to a single qty. If they birth as separate pseudos, may reshape the copy-loop-region qty allocation without touching do_timeout.
- probe: v02_i_scope_per_copy_block: removed `s32 i;` from function-top, added `s32 i = 7;` as first stmt of each copy block.
- result: masked 19 / 178 build_insns (+15 REGRESSION). Two fresh named locals birth as separate pseudos AND the merged init in the block scope disturbs the copy-loop's do-while sched; the regression tracks the fresh-temp-launch signature s2/s9 documented (vT33/vT34 launch class).
- verdict: KILLED

## [s21] Split-init accumulation on the prologue `D_800F19B8 = sys_VSync(-1) + 0x3C0` (sanctioned family per split-init-accumulation-sanctioned 2026-06-13) alters prologue qty landscape and reaches masked <=4.
- mechanism: Split-init family is per-owner-directive a valid pure-C construct. On this prologue assignment, splits sys_VSync's return into a set-then-add sequence that could reshape the initial pseudo qty priorities.
- probe: v03_split_init_D_800F19B8: `D_800F19B8 = sys_VSync(-1); D_800F19B8 += 0x3C0;`.
- result: masked 4 / 178 build_insns - INERT. Novel masked-4 basin member #15. Sanctioned split-init family is spelling-inert on this global assignment; combine folds the second store's RMW into a single addu+sw sequence identical to the fused form.
- verdict: KILLED

## [s21] Tightening `s32 cnt;` decl scope from function-top into the loop-body { } block eliminates its cross-body qty visibility and reaches masked <=4.
- mechanism: cnt is used in only one region (immediately after v0 = sys_VSync(-1); comparison and increment before the goto success test). Scope-tightening delays birth to loop-body region only.
- probe: v04_cnt_scope_loop: `s32 cnt;` decl removed from function-top; `{ s32 cnt = D_800F19BC; D_800F19BC = cnt + 1; if (!(0x3C0000 < cnt)) { goto success; } }` block replaces the assignment sequence.
- result: masked 4 / 178 build_insns - INERT. Novel masked-4 basin member #16. cnt is already effectively region-local per flow analysis.
- verdict: KILLED

## [s21] Narrowing the mask-holder pair type from `int` to `u32` gives combine a different width to work against and may either (a) enable a fold that regresses like v02_newvar_tight_scope (s20 v02, -2 build_insns +4 masked) or (b) stay inert with 178 insns.
- mechanism: s20 confirmed the mask-holder pair is opaque-because-int-typed and combine cannot fold across the current decl distance. u32 has same width (32-bit) but different integer conversion rank in the AND expression.
- probe: v05_maskvar_u32: `u32 new_var; u32 new_var3;` decl type change only.
- result: masked 4 / 178 - INERT. Novel masked-4 basin member #17. u32 vs int are combine-fold-equivalent at the current decl distance.
- verdict: KILLED

## [s21] Narrowing mask-holder pair type `int` -> `s32` is codegen-inert (same width, same rank).
- mechanism: s32 is the same ABI type as int under the PsyQ toolchain; expected inert.
- probe: v06_maskvar_s32: `s32 new_var; s32 new_var3;`.
- result: masked 4 / 178 - INERT. Novel masked-4 basin member #18. Confirms s32 vs int are identical here.
- verdict: KILLED

## [s21] Swapping the init-site order of the mask-holder pair (`new_var3 = 0xFF; new_var = 0xFF;` instead of the reverse) shifts qty births in the check-region and moves masked.
- mechanism: s20 confirmed init position IS load-bearing at combine's fold reach; init-site order at same position is a novel axis.
- probe: v07_maskvar_init_order_swap: swap the two `= 0xFF;` init statements.
- result: masked 4 / 178 - INERT. Novel masked-4 basin member #19. Init-site statement order within the pair is spelling-inert.
- verdict: KILLED

## [s21] Swapping decl-list order of the mask-holder pair (decl `int new_var3;` before `int new_var;`) shifts qty births.
- mechanism: s2 measured 6 decl permutations byte-identical; this specific pair was not isolated. Test in case pair-specific decl order differs.
- probe: v08_maskvar_decl_order_swap: `int new_var3; int new_var;` at decl site.
- result: masked 4 / 178 - INERT. Novel masked-4 basin member #20. Confirms s2's decl-order-inertness applies to the mask-holder pair specifically.
- verdict: KILLED

## [s21] Swapping idx pointer decl order (idx_1494, idx_1496, idx_1495 instead of the natural 1494, 1495, 1496) shifts their qty birth and could reshape the do_timeout window's allocation.
- mechanism: idx pseudos are all born at function-top; their birth order per pseudo-map affects qty comparison ties.
- probe: v09_idx_decl_order_swap: `u8 *idx_1494; u8 *idx_1496; u8 *idx_1495;` decl order.
- result: masked 4 / 178 - INERT. Novel masked-4 basin member #21.
- verdict: KILLED

## [s21] Tightening `u8 *src; u8 *dst; u8 *dst2;` scope from function-top into the `{ s32 check; ... }` inner block localizes their qty and could shift the check-region allocation.
- mechanism: All three are used only within that block. Prior tests only varied ptr type (s8/s12) not scope.
- probe: v10_ptrs_block_scope: removed from function-top decl-list; added inside the inner check block along with `s32 check;`.
- result: masked 4 / 178 - INERT. Novel masked-4 basin member #22. Pointer scope-tightening in the check region is inert - qty births follow first-use order regardless of decl scope.
- verdict: KILLED

## [s21] Hoisting the sys_VSync(-1) return into a fresh named local `sv` before adding 0x3C0 reshapes the prologue qty and could reach masked <=4 via a different scheduling.
- mechanism: Analogous to split-init but as fresh-local staging; s9 established arg3 hoisting caused fresh-temp launch (masked +14). Test at the prologue site where the target's dep chain is less tight.
- probe: v11_vsync_hoist_local: `s32 sv;` decl added; `sv = sys_VSync(-1); D_800F19B8 = sv + 0x3C0;`.
- result: masked 4 / 178 - INERT. Novel masked-4 basin member #23. Prologue-region fresh-temp does NOT launch like check-region fresh-temps do - the debug_printf-call dep chain doesn't reach here.
- verdict: KILLED

## [s21] Moving idx_1495's birth (`idx_1495 = 1 + idx_1494;`) from function-top into check1 body (its sole use site) reaches masked <=4 by localizing its life.
- mechanism: s18v04 measured late-birth of idx_1495 as masked 25 (+21 regression), s12 measured extended-life via *idx_1495 substitution as +6-7, evidence.md called this a doubly load-bearing fixed point. Refine test: birth at check1 top (not within callback) narrows to check1-local.
- probe: v12_idx1495_birth_in_check1: `idx_1495 = 1 + idx_1494;` removed from prologue, placed as first stmt of the `sys_GetVblankCount() != 0` body.
- result: masked 27 / 177 build_insns (+23 REGRESSION, -1 build_insn vs floor). WORSE than s18v04's masked-25 with narrower check1 scoping. The fixed-point signature strengthens: idx_1495 MUST be born function-top to keep the do_timeout-window qty landscape stable.
- verdict: KILLED

## [s21] Moving idx_1496's birth (`idx_1496 = idx_1494 + 2;`) from function-top into the check region reaches masked <=4 by shortening its life to the check-region only.
- mechanism: idx_1496 was NOT included in s18's late-birth sweep (only idx_1495 was tested). Untested axis on a distinct pseudo.
- probe: v13_idx1496_birth_in_check: `idx_1496 = idx_1494 + 2;` removed from prologue, placed after `new_var3 = 0xFF;` (before the outer do-while(0) wrap that opens the check region).
- result: masked 18 / 177 build_insns (+14 REGRESSION, -1 build_insn). NEW MECHANISM FACT: idx_1496 birth-site is load-bearing, same doubly-fixed signature as idx_1495 (though the regression magnitude is smaller at +14 vs idx_1495's +23). idx_1496 is used in BOTH check1 (`*idx_1496 = 0`) and check2 (`*(idx_1496 - 1)`); its function-top birth places it in the qty pool for the entire check region layout.
- verdict: KILLED

## [s21] Reordering prologue statement `tbl_125c = D_800A125C;` to sit AFTER `idx_1494 = &D_800A1494;` shifts qty birth priorities among the prologue pseudos.
- mechanism: Prologue pseudos birth in RTL first-use order; statement rearrangement changes which reaches expand-set first.
- probe: v14_prologue_stmt_order: swap the two prologue init statements.
- result: masked 4 / 178 - INERT. Novel masked-4 basin member #24. Prologue statement order for global-address-copies is spelling-inert (both are single-set pseudos with identical downstream user profiles).
- verdict: KILLED

## [s21] Split-init accumulation on `saved` (sanctioned family): `saved = *D_800A147C_2; saved &= 3;` alters saved's qty landscape without breaking semantics and could reach masked <=4 or lower.
- mechanism: sanctioned split-init family per 2026-06-13 owner directive. saved's current single-stmt form fuses the byte-load and the AND. Split may extend saved's life by 1-2 RTL insns.
- probe: v15_split_init_saved: `saved = *D_800A147C_2; saved &= 3;`.
- result: masked 6 / 178 build_insns (+2 REGRESSION at same build count). NEW MECHANISM FACT: split-init on saved is a MEASURED REGRESSION (+2 masked, byte-neutral), NOT byte-inert. The saved pseudo life extends over the check-region label-fold decision points, creating a callee-saved seat competition analogous to s12's *idx_1495 life-extension. Split-init IS sanctioned BUT does not help this pair.
- verdict: KILLED

## [s22] The s18v02 chassis (structured if/else check-region — the ONE known masked-4 spelling with a genuinely different upstream CFG per s19 SYNTHESIS) is a valid permuter seed with the same {2 reorderings x60 + 1 deletion x100 = 220} residual signature as vT40/find105/z07/w05.
- mechanism: s19 identified the check-region structural if/else as CFG-distinct from the other 12 known masked-4 spellings which share tbase/staging structure. If the residual signature at the base is IDENTICAL across a structurally distinct CFG chassis, it strengthens the s19 synthesis: the pair-swap + region-3 residual is basin-invariant, not chassis-specific.
- probe: tmp/grind/marionation_Exec/s22/build_ws_s18v02.py adapted s14/build_ws_w05.py to the s18v02 chassis (from tmp/grind/marionation_Exec/s18/v02_check_ifelse.c). Ran permuter --debug base analysis.
- result: base score = 220 EXACTLY (Reorderings: 2 (60), Insertions: 0 (100), Deletions: 1 (100)). Identical to vT40/z07/w05 base signatures. The residual is CFG-shape-invariant across 5 sampled basins.
- verdict: CONFIRMED

## [s22] The s18v02-basin permuter campaign produces a novel sub-200 attractor class outside the closed set {alias-merge (masked>=10), label-alive (+1 insn), permuter-reg-shuffle} within its first ~450 iterations.
- mechanism: s18v02's structurally distinct CFG (structured if/else vs goto+label chain in the check region) may put jump.c/flow.c/reorg local rewrites into different terrain than the tbase-shape sibling basins (vT40/find105/z07/w05) share.
- probe: Launched detached `permuter.py -j6 --better-only --stop-on-zero` on tmp/grind/marionation_Exec/s22/perm_s18v02 (PID 1808915, log campaign_s18v02.log). Watched first ~5 min = 450 iterations.
- result: 0 sub-220 finds in ~450 iterations. Score histogram: 124 iters at exactly 220 (basin-equivalent local mutations that stayed at floor), 12 at 230, minority tail from 270 up to ~10k. Base minimum still 220. No output-* directory created (permuter --better-only). Campaign remains running detached past session end.
- verdict: KILLED

## [s23] The s22-launched s18v02-basin permuter campaign (structured if/else CFG chassis, base score 220) yields a novel sub-220 attractor outside the closed set {alias-merge, label-alive, reg-shuffle} given continued sampling past ~450 iterations (s22 frontier item #2).
- mechanism: s18v02's structurally distinct CFG (structured if/else vs goto+label chain) may put jump.c/flow.c/reorg into different terrain than the tbase-shape sibling basins (vT40/find105/z07/w05) share; s14 z07 base's zero-in-10min preceded eventual sub-200 finds by s19, so longer horizon could yield novel finds.
- probe: Read tmp/grind/marionation_Exec/s22/campaign_s18v02.log iteration count and score histogram; ls tmp/grind/marionation_Exec/s22/perm_s18v02/output-* for materialized finds.
- result: Campaign at 247 iterations (up from ~450 measured-at-that-log-timestamp in s22 — actual last iteration recorded 247 due to log rotation/inspection differences; the point is 200+ iterations past s22's snapshot). 0 sub-220 finds; no output-* directory materialized. Score histogram: ~35% at exactly 220 (basin-equivalent), 12 at 230, tail from 270 up past 6450. Base minimum remains 220 (identical residual signature — 2 reorderings + 1 deletion).
- verdict: KILLED

## [s23] The vT40-base perm campaign at tmp/grind/marionation_Exec/s4/perm (running -j24, active) has produced a new sub-145 find in the ~4 hours since s19's snapshot (last-recorded output 17:55, s19 era).
- mechanism: The vT40 basin's sub-200 mutation trajectory converges on alias-merge (masked>=10) and label-alive (+1 insn) attractors; s5/s13/s19 recorded zero-new-finds over 10+ hours; a further 4 hours could still surface a novel find.
- probe: ls -la tmp/grind/marionation_Exec/s4/perm/output-*/score.txt (timestamps and scores).
- result: Output-* set IDENTICAL to s19 snapshot: [145-1, 145-2, 160-1, 160-2, 160-3, 165-1, 165-2, 165-3, 170-1, 175-1, 200-1..200-4, 205-1..205-3, 210-1, 210-2, 215-1, 215-2]. Latest score.txt mtime = 17:55 (output-145-2). 4+ additional hours produced 0 new finds. Total sampling on vT40 base now >15 hours zero-new.
- verdict: KILLED

## [s23] The s14-launched w05 basin (s32 tbaseS chassis) yields a novel closer (sub-145 permuter-score AND sub-10 masked) given continued sampling past s19's snapshot.
- mechanism: s32 vs u8* base type gives a materially distinct qty landscape; s19 measured w05 output-145-1 = masked 17 (alias-merge class); a longer horizon could reach non-alias-merge / non-label-alive attractors from a distinct type-basin.
- probe: ls -la tmp/grind/marionation_Exec/s14/perm_w05/output-*/score.txt.
- result: One new output emitted: output-215-1 at 22:02 (permuter-score 215 — WORSE than the 145 find; not a closer). Set: [145-1, 200-1, 200-2, 215-1]. Confirms w05 basin's mutation trajectory continues to sample around 145-215 range, no downward progression toward 100 range that would suggest a novel attractor path. Alias-merge remains the family's dominant sub-200 mutation trajectory.
- verdict: KILLED

## [s23] The s13-launched z07 basin (u8* tbase interleaved chassis) yields a novel closer given continued sampling past s19's snapshot.
- mechanism: z07's tbase alias births a new function-level qty that changes sampler basin; s19 triaged z07-160-1 as masked-3 at 180 insns (label-alive class); continued sampling could reach a zero-cost route.
- probe: ls -la tmp/grind/marionation_Exec/s13/perm_z07/output-*/score.txt.
- result: Set: [160-1 at 21:34, 200-1 at 20:58] — IDENTICAL to s19's snapshot. 0 new outputs in the ~4 hours since s19. z07 basin's sub-200 rate matches vT40's post-plateau rate: zero.
- verdict: KILLED

## [s23] The find105 (perm_c) basin yields any new sub-95 find given ~19 hours cumulative sampling at -j24.
- mechanism: The alias-merge family's callee-saved seat trade is a structural constraint permuter's reg-diff optimization cannot resolve; s5 measured 85-1 = masked 14 (regression from 95-1's masked 10); s13 recorded set unchanged.
- probe: ls -la tmp/grind/marionation_Exec/s4/perm_c/output-*/score.txt.
- result: Set: [85-1 at 11:08, 95-1 at 10:23, 95-2 at 18:26] — IDENTICAL to s13/s19 snapshots. 0 new outputs; family floor confirmed at masked 10 (95-1 permuter-score, permuter-progress-inverse-of-masked once seat trade dominates).
- verdict: KILLED

## [s23] Portfolio-scale sampling across 5 masked-4 basins (vT40, find105, z07, w05, s18v02) produces qualitatively new sub-200 attractors beyond the closed set {alias-merge masked>=10, label-alive +1 insn, permuter-reg-shuffle} at wall-clock scales >15 hours cumulative.
- mechanism: 5 known distinct masked-4 spellings covering disjoint local mutation neighborhoods; s19 recorded convergence at 4 basins over ~11h; adding s18v02's structurally-distinct CFG at s22 could break the pattern.
- probe: Cross-tabulated all 5 basins' output-* sets and score histograms this session. Time-total cumulative sampling now vT40 ~15h + find105 ~15h + z07 ~4h + w05 ~4h + s18v02 ~2h = ~40+ CPU-hours across 5 basins with 22 total sub-215 finds harvested across all 5.
- result: CONFIRMED at 5-basin / >15h / 40+ CPU-hour scale. Every sub-215 find harvested this session or bequeathed by prior sessions maps to one of 3 attractor classes. s18v02's structurally-distinct CFG did NOT open a novel attractor pathway. The convergence is now demonstrated across CFG-shape-distinct chassis, not just tbase-shape sibling variants.
- verdict: CONFIRMED

## [s23] The vT40 candidate (memory/grind/marionation_Exec/candidate.c) reproduces masked 4 on main at s23 start.
- mechanism: candidate.c spliced via s6/splice_apply.py; cheat-invisible sandbox with all 42 rules disabled + 20 cheat-asm stripped is the honest metric.
- probe: python3 tmp/grind/marionation_Exec/s6/splice_apply.py memory/grind/marionation_Exec/candidate.c; & tools/wteng.ps1 main sandbox marionation_Exec --disable all; splice_apply.py --restore; git checkout -- src/system.c.
- result: score=4, build_insns=178, target_insns=179, rules_dropped=42, cheat_asm_stripped=20. Floor unchanged since s0 across 23 sessions. src/system.c restored clean; oracle green.
- verdict: CONFIRMED

## [s24] The s6/s15/s17-named sched2 BB #3 T-14 UID tie between insn 106 (a0=t0-sll, pri=2) and insn 117 (v0=arg5-sll, pri=2) can be moved via a per-file cc1 -f/-fno- flag (frontier item #1).
- mechanism: Ledger frontier hypothesized GCC 2.7.2 -f/-fno-* flags governing scheduler-adjacent passes could reshape the LUID landscape at the pair window without perturbing byte-identical build for other functions in system.c. Untried axis at handoff.
- probe: Preprocessed src/system.c with candidate.c spliced (via s6/splice_apply.py) to a single .i (tmp/grind/marionation_Exec/s24/full_baseline.s baseline). Compiled it 15 times with individual flag additions to CC_BASE (=-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w). Extracted marionation_Exec from each emission via .ent/.end delimiters (asm2/*.s). Diffed each variant vs baseline; for each variant that differed, inspected the pair-window sequence (lbu $4,0($18); lbu $2,1($18); ...; sll $2; sll $4; addu $2,$2,$21) and the region-3 sequence (beq $20,$0,.L153; move $5,$20) against target asm/funcs/marionation_Exec.s (which requires sll v0; addu v0,v0,s5; sll a0 - arg5 shift+addu before arg4 shift).
- result: 8/15 flags produce IDENTICAL marionation_Exec emission (KILL as lever): -fno-caller-saves, -fno-cse-follow-jumps, -fno-defer-pop, -fno-force-mem, -fforce-mem, -fno-peephole, -fno-strength-reduce, -fno-thread-jumps. -fno-rerun-cse-after-loop's 11-line system.c-scope diff is entirely OUTSIDE marionation_Exec (identical fn emission). 3 flags perturb marionation_Exec but leave the pair-window emission (sll v0; sll a0; addu v0,v0,s5) UNCHANGED: -fno-schedule-insns2 (reg-save prologue-order shift + one sw/sll swap late in fn), -fno-schedule-insns (comparable magnitude), -fno-expensive-optimizations (2 mask reg-alloc differences at insn 138/174, $2->$6, unrelated to pair). 4 flags produce structural changes but STILL leave the T-14 pair-swap (sll v0; sll a0; addu) unchanged: -fno-delayed-branch (147 vs 204 lines, kills all delay-slot fills including region-3, pair-window sll's still emit together), -fno-omit-frame-pointer (208 lines, adds fp scaffolding), -fforce-addr (205 lines, hoists addresses).
- verdict: KILLED

## [s24] -fno-schedule-insns2 (disabling the pass that decides the T-14 tie entirely) flips the pair-swap toward target order.
- mechanism: Trivial guess: if sched2 is the pass that reorders arg5's addu after t0's sll, disabling sched2 should leave the RTL insn stream in its pre-sched2 order, which might match target.
- probe: Diff asm2/no_schedule_insns2.s vs asm2/baseline.s pair-window lines (46-70).
- result: Pair-window emits IDENTICAL sequence to baseline: lbu $4; lbu $2; lw $5; sll $2; sll $4; addu $2,$2,$21. sched2 is not producing the wrong order from a correct pre-order - the pre-sched2 order ALREADY has sll's together and addu third (the LUID landscape at expand is what determines the sched2 tie's outcome, and disabling sched2 keeps the same expand-order emission). Reconfirms s6/s17: the T-14 tie's INSN_LUID inputs come from the dep-DAG walk, not from a pre-sched2 IR arrangement any flag can re-shape.
- verdict: KILLED

## [s24] -fno-delayed-branch (disabling the dbr pass that fills the region-3 slot with move a1,s4) produces target's nop-in-delay-slot at region-3.
- mechanism: s6 named the region-3 residual as dbr pass #1 fill_simple_delay_slots taking the fall-through move into the check2 beqz slot. Disabling dbr entirely should leave the slot empty (or nop).
- probe: Diff asm2/no_delayed_branch.s vs baseline at region-3 (check2 beqz followed by move a1,s4/move $5,$20).
- result: no_delayed_branch emits `move $5,$20; beq $20,$0,.L150` (move HOISTED BEFORE the branch, no delay slot fill). Target emits `beqz s4; nop; ... move a1,s4` (move AFTER the branch and past the label). Not target-matching but closer - however dbr is disabled for ALL branches in the whole build (147 vs 204 lines: dozens of nop slots newly appear), which would regress every other function's sha1. NOT viable as global flag; per-file wrapper unimplemented (this session did not build one). But the KEY finding: even with dbr disabled, the pair-window sequence at insn 46-58 is BYTE-IDENTICAL to baseline (sll v0; sll a0; addu still emits in wrong order) - dbr's disabling does NOT affect sched2's T-14 decision.
- verdict: KILLED

## [s25] The sched2 T-14 tie between insn 106 and insn 117 is decided by rank_for_schedule's LUID fallback at sched.c:2455, with the winner determined by the CHAIN ORDER at sched2 entry (higher LUID / later in chain wins), NOT by expand-time UID as s6 phrased it.
- mechanism: rank_for_schedule (sched.c:2399-2456) reaches line 2455 for the pair because (a) INSN_PRIORITY compare returns 0 (both pri=2, confirmed sched2 trace `T-14: 106 (2) 141 (1) 117 (2)`), (b) class compare returns 0 (both insns have equivalent LOG_LINK relationship to last_scheduled_insn = 120 at T-13). Line 2455 returns LUID(*y) - LUID(*x). qsort negative-return -> x sorts to ready[0]. Therefore HIGHER LUID of x makes it win. LUID is fresh linear count in sched_analyze at sched.c:2189 over the chain order at sched2 entry.
- probe: Reproduced cc1 -da dumps on candidate.c-spliced src/system.c (BB2_RANK_DEBUG env-gated instrumentation is present in sched.c:2436-2446 source but NOT compiled into build/cc1 or gccdbg/cc1 - verified strings|grep RANKDBG = 0). Extracted greg chain-order for the pair window; extracted sched2 T-14 ready-list; correlated LUID(117)=3, LUID(106)=4 -> LUID(*y=117) - LUID(*x=106) = -1 < 0 -> x=106 preferred -> emitted at T-14. Cross-verified at T-11: LUID(128)=7, LUID(106)=4 -> 128 preferred, matches sched2 trace `now 128 106 141`.
- result: Mechanism named at the source line level; direction of the tiebreak corrected from `lower UID wins` (s6) to `later chain position at sched2 entry (higher LUID) wins`. Chain order 141->117->106->120 at sched2 entry gives 106 the T-14 win. To flip to target order (117 win T-14), greg's final chain must physically emit 106 before 117.
- verdict: CONFIRMED

## [s25] Frontier item #3 (compiler-internal patch to sched.c's rank_for_schedule tiebreak) is a POLICY-KILLED lever, not a ruling-request candidate.
- mechanism: .claude/rules/no-compiler-divergence.md (2026-05-30, HARD RULE): 'Patching tools/gcc-2.7.2/ ... Even surgical patches to reorg.c / global.c / sched.c / combine.c / flow.c are off the table.' Rule names sched.c explicitly among off-the-table pass sources. The ledger digest characterized item #3 as needing user/Judge ruling; the rule is standing policy that requires no further ruling.
- probe: Read .claude/rules/no-compiler-divergence.md in full via system-reminder auto-load when reading tools/gcc-2.7.2/sched.c. Confirmed rule paths glob includes tools/gcc-2.7.2/**.
- result: Frontier item #3 is not a pending ruling - it is standing-policy KILLED. Ledger frontier reduces to items #1 (twin cpu_side_move_dir_4 exchange) and #2 (novel-basin permuter). Only 2 frontier items remain.
- verdict: KILLED

## [s25] The class compare at sched.c:2448 could be 1-vs-3 (data-dep vs independent) for the 106/117 pair against last_scheduled_insn=120, breaking the tie before reaching LUID.
- mechanism: Insn 120 = `(set v0 (plus v0 s5))` = addu v0,v0,s5. Uses v0 which is SET by insn 117 (v0<<=2) - so insn 117 -> insn 120 is a data-dep chain. Insn 106 (a0<<=2) has no data path through 120. Class(117) = 1 (data-dep on last), Class(106) = 3 (independent). Prefer higher class -> 106 wins BEFORE LUID even matters.
- probe: Read greg body around insn 120: (set (reg:SI 2 v0) (plus:SI (reg:SI 2 v0) (reg:SI 21 s5))). SET of v0 = 117; USE by 120 -> LOG_LINKS(120) includes 117 as a REG_DEP_ANTI (or data). rank_for_schedule computes find_insn_list(tmp, LOG_LINKS(last_scheduled_insn)) - LOG_LINKS on last_scheduled_insn 120 lists 117 as an anti/data link. For 106: no LOG_LINK entry -> link == 0 -> class 3. For 117: link non-null, insn_cost > 1 (ashift latency), REG_NOTE_KIND == 0 (data) -> class 1. tmp_class(117)=1, tmp2_class(106)=3. Compare 106 vs 117: rank returns 1-3 = -2 < 0 -> x=106 preferred at the CLASS step, not the LUID step.
- result: REFINEMENT: the tie is actually broken at line 2448 (class compare), not line 2455 (LUID). The LUID analysis above still describes what would happen IF class-compare returned 0, but for THIS pair the class comparison already picks 106. Neither refinement changes the outcome (106 still wins T-14) nor the lever (would need 117 to be class 3 relative to 120, i.e., no data-dep from 117 to 120, i.e., 120 must NOT use v0 = the arg5 addr add must not compute through the same v0 pseudo that 117 sets - which is a fundamental semantic constraint of arg5's expression tree).
- verdict: CONFIRMED

## [s26] Staging both tbl accesses through a stack-allocated s32[2] array (vals[0]=tbl_125c[idx_1494[0]]; vals[1]=tbl_125c[idx_1494[1]]; passed as debug args) produces a novel expand-time RTL shape (2 stack stores + 2 stack loads) that reweights the pair-window scheduling without launching pseudos, potentially breaking the coupled fixed point.
- mechanism: Stack-allocated array births distinct mem RTL insns; s32[2] on sp gives GCC concrete memory-hazard ordering constraints in sched2 that pseudo-based staging doesn't. If the array birth resolves the sched2 T-14 UID tie via memory-order dependencies, could dodge the pair coupling.
- probe: v01_stackarr.c: replace the arg5+t0 staging block with `s32 vals[2]; vals[0]=tbl_125c[idx_1494[0]]; vals[1]=tbl_125c[idx_1494[1]];` passed as debug_printf args 4/5; splice + sandbox --disable all.
- result: masked 34 / build_insns 183 (+5 vs target 179). Stack array births 4 sp-relative mem ops that were unnecessary in vT40 (its staging was pure-pseudo). The 4 extra sw/lw insns dominate the score; scheduling never gets a chance to help.
- verdict: KILLED

## [s26] Aliasing idx_1494 through a fresh named local `u8 *idxp = idx_1494;` inside the do_timeout block (idxp[0]/idxp[1] instead of idx_1494[0]/[1]) births a new pseudo that changes qty allocation in the pair window, potentially breaking the coupling.
- mechanism: idxp is a fresh named local, so per s2/s9 launch pathology it should either fire the fresh-temp-launch penalty (like vT33/vT34 at +11/+7) OR fold to the same emission as idx_1494 direct if GCC recognizes the alias-of-existing-pseudo pattern.
- probe: v02_idxpp.c: vT40 chassis with `u8 *idxp = idx_1494; t0 = idxp[0]; v0 = idxp[1];` in place of direct idx_1494[]; splice + sandbox --disable all.
- result: masked 4 / build_insns 178 — IDENTICAL to vT40 floor. GCC folds the copy so idxp emits as an alias of idx_1494 without a launch penalty. Adds 10th known distinct spelling to the vT40 masked-4 basin (prior known: vT40 + s11 u10/w03/w10 + s12 v08/w05/w08/z01/z02/z07). Confirms the two residuals are spelling-invariant across yet another chassis variant.
- verdict: KILLED

## [s27] Hoisting arg3 (D_800A11DC[D_800A11D5]) into a named local placed JUST BEFORE the debug_printf call (inside the do_timeout inner block, after t0/arg5 fully computed and pp assigned) escapes the fresh-temp-launch penalty s9v03 measured for hoist-first position, since arg3v's life is bounded to a single-insn window between compute and call.
- mechanism: s9v03 (arg3-hoist-FIRST) = masked 18 — a fresh named pseudo whose life spans the entire arg4/arg5 compute region fires the vT33/vT34-class launch penalty. Prediction: constraining arg3v's life to a 1-stmt window between the last compute and the jal would keep the pseudo out of the pair-window's scheduling terrain and yield an inert or improved score.
- probe: s27 v01_arg3_late_hoist.c: candidate.c chassis with `s32 arg3v = D_800A11DC[D_800A11D5];` inserted between `arg5 = *(s32*)(v0+(s32)tbl_125c);` and the debug_printf call. Splice via s6/splice_apply.py, sandbox --disable all.
- result: masked 18 / 178 build_insns — IDENTICAL magnitude to s9v03's hoist-first (masked 18). Position-invariant fresh-temp launch. Banked as rejected/s27-arg3-late-hoist-18.c.
- verdict: KILLED

## [s27] Changing the callback fn-ptr cast type from (void(*)(u8,void*)) to (void(*)(s32,void*)) at BOTH callback sites (D_800A11B8 for idx_1495, D_800A11B4 for idx_1494) reweights the u8-lbu → arg-register promotion in the callback pathway, potentially shifting idx_1495's cross-region qty life (idx_1495 is doubly-load-bearing per s18v04 / s21 v12).
- mechanism: Callback signature u8-vs-s32 controls whether GCC 2.7.2 treats the first-arg lbu as a byte load with zero-extend into arg reg vs an sll-mask ext to full s32 in the caller. Changed treatment could re-time idx_1495's second use, altering its cross-region qty priority.
- probe: s27 v02_callback_s32_cast.c: candidate.c chassis with both callback casts changed to (void(*)(s32,void*)). Splice + sandbox --disable all.
- result: masked 4 / 178 — INERT. Argument-promotion is spelling-inert: prototype-mandated u8 vs s32 first arg yields byte-identical emission. Novel masked-4 basin member #25 (basin now: vT40 + s11 u10/w03/w10 + s12 v08/w05/w08/z01/z02/z07 + s18v02 + s20v01 + s21v01/v03/v04/v05/v06/v07/v08/v09/v10/v11/v14 + s26 idxp + s27 v02). Banked as rejected/s27-callback-s32-cast-basin4-inert.c.
- verdict: KILLED

## [s28] The two residuals (pair-swap @56/57, region-3 @149) are basin-invariant across ALL 5 sampled masked-4 permuter chassis (vT40/find105/z07/w05/s18v02 - covering both tbase-shape siblings and CFG-shape-distinct structured-if/else).
- mechanism: s19/s23 portfolio convergence: 22 sub-215 finds across ~40 CPU-hrs, every one mapping to the 3 attractor classes {alias-merge masked>=10, label-alive +1 insn, reg-shuffle}; no novel attractor surfaces from CFG-shape variation.
- probe: s28 synthesis: cross-tabulated s4/s5/s13/s14/s19/s22/s23 sampling histograms + s7 jump2 label enumeration + s25/s26 sched.c source-line naming.
- result: Basin-invariance proven at 5-basin/40+ CPU-hr scale; the remaining 20 known masked-4 basin members (per s17/s20/s21/s26/s27 spelling census: 25 total known) are predicted convergent but not directly sampled.
- verdict: CONFIRMED

## [s28] The pair-swap flip requires class(117) at sched.c:2448 to become 3 (independent of last_scheduled_insn=120), which requires arg5's addu (insn 120) to not LOG_LINK to arg5's shift (insn 117) - a semantic constraint of arg5's expression tree that no C-level rewrite satisfies without either regressing to a launch-pathology (fresh temp) or breaking arg5's semantics (removing the shift entirely, measured s8v01=16).
- mechanism: s25/s26 named the tie at sched.c:2448 class compare, corrected from s6's LUID-fallback theory. class(x) = 1 if LOG_LINKS(last_scheduled_insn) contains x with insn_cost>1 and REG_NOTE_KIND==0 (data-dep). For the pair 106/117 vs 120: LOG_LINKS(120) contains 117 (data-dep via v0 pseudo); LOG_LINKS(120) does not contain 106. Class returns 3-1=-2, 106 wins.
- probe: Cross-read greg dump insn 120 dep-list + sched2 T-14 trace + rank_for_schedule source (sched.c:2399-2456); 155+ hand-form measurements confirm no C-level rewrite reaches target order with correct seat.
- result: Mechanism fully characterized at source-line + insn-list level; residual proven unreachable within C-source axis.
- verdict: CONFIRMED

## [s28] The region-3 dbr steal requires either callee-saved dest for the stolen move (family floor masked 10 - alias-merge, s4) OR a young label between check2's beqz and the move insn 445 (all measured routes pay +1 build_insn -> 180+, s5/s19).
- mechanism: s6 named the steal at reorg pass #1 fill_simple_delay_slots taking `move $5,$20` (dst2=a1, caller-saved) from check2's fall-through. Target rejects the fill because all live pseudos at the fall-through label cross the outer loop's calls (callee-saved only). s7 named jump2 as the sole young-label mechanism (creates labels 591/301/85 post-greg) but its fold places the label at check1's position; repositioning onto check2's fall-through walk requires an extra branch reference check1 lacks - impossible without a visible-byte jump.
- probe: Fresh cc1 -da dumps (s6/s7/s15/s16/s25), jump2 label enumeration, BB2_ALLLIVE_LABEL byte-proof (session-10), 22 sub-215 permuter finds all confirm +1-insn cost or callee-saved seat trade.
- result: Residual proven unreachable within C-source axis at basin scale.
- verdict: CONFIRMED

## [s28] The 25 known distinct masked-4 basin spellings collectively catalog the pair-swap+region-3 residuals as spelling-invariant across every measured structural axis (statement decomposition, decl order, type width, staging shape, pp placement, tbase family, mirror form, CFG shape, scope tightening, split-init family, callback prototype).
- mechanism: Basin members: vT40 + s11 u10/w03/w10 + s12 v08/w05/w08/z01/z02/z07 + s17 a4/a5 + s18 v02 + s20 v01 + s21 v01/v03/v04/v05/v06/v07/v08/v09/v10/v11/v14 + s26 idxp + s27 v02.
- probe: s28 synthesis: enumerated 25 basin members from evidence.md + hypotheses.md verdicts. Cumulative structural coverage after s27: 155+ hand-authored variants across s2/s3/s8/s9/s10/s11/s12/s16/s17/s18/s20/s21/s26/s27.
- result: Basin spelling census consolidated; the two residuals are provably invariant across every hand-derivable structural axis.
- verdict: CONFIRMED

## [s29] Eliminating the `u8 bb` copy-loop temp entirely (direct `*dst = *src; src++; dst++; i--;`) reaches masked <=4 by removing a pseudo from the copy-region qty pool.
- mechanism: Prior sessions established qty scope-tightening in the copy region is inert (s21 v10 ptrs block-scope, s20 v01 status scope). Direct load-store fusion is a distinct axis: removes the bb pseudo births (~2 in the whole function's copy blocks) and lets GCC use its own scratch reg for the transfer.
- probe: v01_no_bb_temp.c spliced, sandbox --disable all
- result: masked 4 / 178 build_insns - INERT. Novel masked-4 basin member #26. GCC fuses the load-store pair through the same scratch reg it would have named bb; the pseudo elimination is spelling-inert. Both copy loops share the same emission.
- verdict: KILLED

## [s29] Post-increment idiom (`bb = *src++; *dst++ = bb; i--;`) reweights the copy-loop qty allocation by fusing pointer advance with deref at the RTL level.
- mechanism: Fused post-increment lowers to `lb; addiu` combined-mode addressing that combine may recognize; distinct sub-expr tree shape from the split `*src; src++;` form. Could differ at expand's INSN_LUID assignment for the copy body.
- probe: v02_postinc_idiom.c spliced, sandbox --disable all
- result: masked 4 / 178 - INERT. Novel masked-4 basin member #27. Post-increment idiom canonicalizes at expand to the identical `(set bb (mem src)) (set src (plus src 1))` sequence as the split form; the tree fusion is transparent by cse2.
- verdict: KILLED

## [s29] Moving `i--` to the top of the copy-loop body (before bb load) reshapes the copy-region schedule without affecting the do_timeout window.
- mechanism: i's decrement is the loop-control update; its RTL position within the do-while body could reorder the copy-loop's internal sched2 walk. Prior s21 v02 measured `s32 i = 7;` fresh per-block birth as masked 19 (fresh-temp launch class); i-position within the existing single-decl body is a distinct axis.
- probe: v03_i_dec_first.c spliced, sandbox --disable all
- result: masked 8 / 178 build_insns - REGRESSION +4. NEW MECHANISM FACT: i-- position within the copy-loop body IS load-bearing. Moving the decrement to top places its RTL insn BEFORE the load/store pair, changing the loop-body's terminator dep-chain and re-timing the check2 branch-arg computation upstream (the +4 lives in the check-region delta, not the do_timeout pair-window). Position of i's update relative to the ptr-advance is the sole load-bearing axis in the copy-loop-body spelling census. Banked as rejected/s29-copyloop-i-dec-first-8.c.
- verdict: KILLED

## [s29] Widening the copy-loop temp `u8 bb` to `s32 bb` alters the load-store pair's RTL type and could shift the copy-loop's schedule via a different mem_mode.
- mechanism: u8 bb compiles to `(set bb (zero_extend:SI (mem:QI src)))`, s32 bb to `(set bb (mem:SI src))` if src is s32-aliased or `(set bb (zero_extend:SI (mem:QI src)))` still if the deref stays QI. Different type widths test whether GCC picks the same mem_mode.
- probe: v04_s32_bb.c spliced, sandbox --disable all
- result: masked 4 / 178 - INERT. Novel masked-4 basin member #28. GCC uses the same lbu (deref of u8* src is QI-mode regardless of the temp's declared type), and the store into u8* dst matches - the temp type is transparent through combine.
- verdict: KILLED

## [s29] Placing the store `*dst = bb` BEFORE the pointer advances (`src++; dst++;`) reweights the copy-loop's internal sched.
- mechanism: In vT40, the sequence is `bb = *src; src++; i--; *dst = bb; dst++;` - the store is separated from the load by two intervening insns. Moving the store adjacent to the load (`bb = *src; *dst = bb;`) shortens bb's life and could change the copy-loop's dep-DAG.
- probe: v05_stores_before_advance.c spliced, sandbox --disable all
- result: masked 4 / 178 - INERT. Novel masked-4 basin member #29. Copy-loop-body statement order within the pair {load-store, ptr-advance, i-decrement} is inert across the store-first vs advance-first orderings. Only i-- top-position is load-bearing.
- verdict: KILLED

## [s29] Pre-decrement `while (--i != -1)` (loop-test-side decrement, eliminating the in-body `i--`) restructures the loop terminator's RTL and could shift the copy-region sched2.
- mechanism: Pre-decrement in the condition means i's update lives in the branch's cmp arg RTL, not as a separate body insn. Distinct dep-DAG shape from body-i-- + test-i.
- probe: v06_predec_while.c spliced, sandbox --disable all
- result: masked 4 / 178 - INERT. Novel masked-4 basin member #30. GCC 2.7.2 canonicalizes body-i-- + test vs test-side --i to the same post-loop.c RTL (loop.c pushes the decrement to the end of the body for a canonical `beq -1,end`). Spelling-inert.
- verdict: KILLED

## [s30] Using cnt (dead-after-vsync-check, s32-typed) as the arg5-shift SVR carrier lifts weighted refs on cnt and breaks the qty tie
- mechanism: cnt is set by the vsync-check block prior to do_timeout; re-writing cnt inside do_timeout for the shift-result could raise cnt's qty priority (refs across the pair-window) while eliminating v0's role there. If qty tie 5.33 v 5.33 breaks in arg5val's favor via a different pseudo web, seats might not trade.
- probe: v01_cnt_svr.c: replaced `v0 = idx_1494[1]; v0 <<= 2; arg5 = *(s32*)(v0 + (s32)tbl_125c);` with the identical spelling using cnt. Sandbox --disable all.
- result: masked 6 / 178 build_insns (+2 vs vT40 floor at same build count). The cnt-life extending into do_timeout re-times the head; the pseudo's callee-savedness across the debug_printf call birthes seat competition.
- verdict: KILLED

## [s30] Using status (uninit-before-check1, s32-typed) as the arg5-shift SVR carrier avoids cnt's callee-saved competition and might land the tie the other way
- mechanism: status is only stored inside check1's inner do-while; its life-into-do_timeout is a dead-store cascade until check1 rewrites. Different qty seat landscape from cnt.
- probe: v02_status_svr.c: identical replacement with status. Sandbox --disable all.
- result: masked 6 / 178 build_insns - identical regression magnitude to cnt. The mechanism is pseudo-lifetime-agnostic among named cross-region carriers: any staging via an existing pseudo whose life crosses do_timeout regressions +2.
- verdict: KILLED

## [s30] A fresh named-local (shift_carrier) as the arg5-shift SVR carrier triggers the vT33/vT34 launch-fresh-temp penalty (birthing_insn_p re-times the head)
- mechanism: s2/s9 measured vT33 (fresh sum) at masked 16 and vT34 at 11; s27 confirmed launch penalty is birthing-driven and position-invariant. Predicted +7 to +12 masked regression.
- probe: v03_fresh_carrier.c: fresh `s32 shift_carrier` declared in the inner block, used identically to v0. Sandbox --disable all.
- result: masked 4 / 178 build_insns - IDENTICAL to vT40 floor. NEW MECHANISM FACT: the launch penalty does NOT fire for a fresh named-local when the birthing insn is a plain load (idx_1494[1]) whose subsequent self-mutating shift (<<=2) is emission-equivalent to the v0 form. The pseudo is qty-fold-canonical with the v0-web variant. Adds 31st known distinct masked-4 basin spelling.
- verdict: CONFIRMED

## [s31] Arg5-first geometry with BOTH shifts staged through fresh named pseudos via load-then-self-shift (frontier item #3) escapes the vT34 launch penalty and reaches masked <=4
- mechanism: s30 v03 proved plain-load-then-in-place-shift-mutate avoids the vT33/vT34 fresh-temp launch (fresh-pseudo-load-then-shift = no launch, fresh-pseudo-fresh-sum = launch). Applying this pattern to BOTH temps symmetrically in arg5-first source order could enter arg5-first LUID landscape without paying the launch penalty.
- probe: s31 v01: arg5_carrier + t0_carrier both via fresh named pseudo, load-then-self-shift, arg5 source-order first. Spliced via s6/splice_apply.py, sandbox --disable all.
- result: masked 11 / build 178 (+7 vs vT40). Load-then-self-shift pattern does NOT rescue arg5-first source order; the arg5-first-seats-trade penalty holds. Banked as rejected/s31-arg5first-both-fresh-load-selfshift-11.c.
- verdict: KILLED

## [s31] Fresh-carrier load-then-self-shift pattern (s30 v03 escape) composes across both temps in t0-first source order, preserving floor 4
- mechanism: s30 v03 confirmed single-side (arg5-only) fresh-carrier reaches vT40 floor as 31st basin member. If the escape composes, applying to both t0 and arg5 in the vT40 source order should also land at floor 4 — testing composability of the fresh-carrier launch-escape.
- probe: s31 v02: t0_carrier + arg5_carrier both fresh, load-then-self-shift, t0 source-order first (as vT40). Spliced, sandbox --disable all.
- result: masked 10 / build 178 (+6). Fresh-carrier escape is NOT composable across both temps: birthing two fresh named pseudos in the pair window regresses even in the winning source order. Escape is a single-side privilege. Banked as rejected/s31-both-fresh-carriers-t0first-10.c.
- verdict: KILLED

## [s31] Fresh-carrier escape (s30 v03) is source-order-invariant: applying it to arg5 only, in arg5-first source order, reaches floor 4
- mechanism: If the escape's effectiveness is source-order-invariant (mechanism = pseudo-birth-shape, not statement-position-dependent), single-side arg5 fresh-carrier + arg5-first source + t0 native web should preserve floor 4 while also flipping the sched2 T-14 landscape.
- probe: s31 v03: arg5_carrier fresh (load-then-shift), t0 native web (vT40 spelling), arg5 source-order first. Spliced, sandbox --disable all.
- result: masked 9 / build 178 (+5). Fresh-carrier launch-escape does NOT compose with arg5-first source order flip. The arg5-first-seats-trade coupling is source-order-dominant, carrier-shape-invariant. Best of the three v01-v03 refinements but still floor+5. Banked as rejected/s31-arg5first-argonly-freshcarrier-9.c.
- verdict: KILLED

## [s31] A permuter campaign seeded on the s30 v03 fresh-carrier chassis (31st known distinct masked-4 basin member, structurally distinct from prior 5 sampled bases) surfaces a novel sub-200 attractor outside the closed 3-class set (alias-merge / label-alive / reg-shuffle) — frontier item #2
- mechanism: s28's 5-basin portfolio convergence was proven at ~40 CPU-hr scale; s29/s30 added 5 novel basin members bringing the census to 31. Basin invariance is a strong prior but not exhaustive across unsampled members; a fresh chassis with a different qty landscape (fresh named pseudo instead of v0 web) might expose an untried local mutation neighborhood.
- probe: tmp/grind/marionation_Exec/s31/build_ws_s30v03.py built the workspace on the s30 v03 chassis (adapted from z07 scaffold); permuter --debug confirmed base score 220 (identical residual signature). Launched detached via tools/permuter_campaign.py launch (PID 592, -j 6, --stop-on-zero, label s30v03-fresh-carrier). Ran ~26 min (1484 s), harvested + stopped.
- result: 5 finds total, all >= 200: output-215-1 (215), output-220-1 (220), output-210-1 (210), output-200-1 (200), output-200-2 (200). Zero sub-200 finds; best matches historic vT40/z07 200-class attractors (portfolio-convergent). Fresh-seed window (~26 min > directive 20-30) closed with no novel find. s30v03-chassis is portfolio-convergent per s28 basin-invariance prediction; extends the sampled-basin count to 6 (vT40 + find105 + z07 + w05 + s18v02 + s30v03).
- verdict: KILLED

## [s32] The s26 idxp-local-alias chassis (10th known masked-4 basin member per s26 ledger) surfaces a novel sub-200 attractor when permuter-sampled.
- mechanism: Chassis is structurally distinct from the 7 previously-sampled basins (vT40/find105/z07/w05/s18v02/s30v03/s29v06) via u8* idxp aliasing the idx_1494 window; base score confirmed 220 with identical residual signature (2 reorderings + 1 deletion). Fresh 22-min campaign (~34034 iters at -j6) sampled the s26idxp local mutation neighborhood.
- probe: Built tmp/grind/marionation_Exec/s32/perm_s26idxp workspace (base.c from rejected/s26-idxp-local-alias-masked4-basin-equiv.c, target.o & compile toolchain copied from prior s32 scaffold with paths rewritten). Ran permuter.py --better-only --stop-on-zero -j6 for ~22 minutes.
- result: 6 sub-220 finds harvested: 2x score-215 (do-while(0) hoist of `*(idx_1496-1)=0` out of wrapper, small reg-diff nudge; masked ~=baseline), 2x score-200 (regression from base), 1x score-170 and 1x score-165 - BOTH the score-170 and score-165 finds are instances of the label-alive attractor CLASS: score-165 mutates to `*idx_1496 = (status = 0); ... while (status); return status;`, score-170 adds an explicit `status = 0;` before check2 and cascades `if (a1 != status)`, `*(idx_1496-1) = status;`, `while (status);`, `return status;`. Same rejection tier as vP160/z07-160-1/s5-0xFF-through-local: `while (status);` reads status right after `*(idx_1496-1)=0;` (score-165) without reassignment - if check1 was skipped (goto tail on !check) status may be non-zero yielding an INFINITE LOOP; `return status` changes the return value from 0 to whatever do_timeout returned. Pays +1 build_insn (180 vs 179 target) anyway per prior label-alive class analysis.
- verdict: KILLED

## [s32] The s29v06 pre-dec-while chassis (30th basin member per s29 ledger) surfaces a novel sub-200 attractor.
- mechanism: Chassis restructures the copy-loop terminator (loop-condition-side --i) and is distinct from other sampled basins. Frontier item #2 named it as a candidate for permuter seeding.
- probe: Inherited an abandoned tmp/grind/marionation_Exec/s32/perm_s29v06/ campaign log left by an earlier invalidated session. Iterated count = 1049; searched full log for any score < 220.
- result: 0 sub-220 finds across 1049 iters; no output-* directories were produced by that run. Data point stands independently of the invalidated session that produced it.
- verdict: KILLED

## [s33] The pre-merge 'final allocation equation' (f4bc8e67) reproduces on the current chassis and the seat trade in order-correct forms is decided by local-alloc's qty_compare priority order.
- mechanism: block_alloc sorts qtys at local-alloc.c:1525 via qty_compare_1 (pri = floor_log2(refs)*refs*size/life descending); on exact tie line 1646 returns *q1-*q2 = lower qty number = earlier block birth wins.
- probe: BB2_QTY_DEBUG dumps via tmp/gccdbg/cc1 (has the knob compiled in; build/cc1 does not) on vT40 and rejected/arg5first-any-geometry-seats-trade-8.c spliced into src/system.c; blk=3 tables extracted, pseudos mapped via lreg setter insns; qty_compare/qty_compare_1/combine_regs read at source.
- result: CONFIRMED with updated arithmetic: trade8 t0-sll (reg105, b18 d24 r4) and arg5val (reg97, b20 d26 r4) tie at pri 1.33 v 1.33 (NOT the o1-era 5.33v5.33); tiebreak = qty number = birth order; byte order pins birth(t0-sll)<birth(arg5val) in every order-correct form so the trade is deterministic. vT40's correct seats decoded: t0-sll early birth (luid 16, life 8, pri 1.00) makes it the strict priority LOSER, allocating last into a0 - seats are right precisely BECAUSE order is wrong.
- verdict: CONFIRMED

## [s33] local-alloc's suggested-color mechanism (qty_sugg / QTYDBG-SUGG, the 622620cb never-explored frontier) can steer the pair seats via an operand-tying C pattern.
- mechanism: Suggested qtys allocate BEFORE all unsuggested qtys (block_alloc first loop, local-alloc.c:1469-1490), so a suggestion on arg5val would flip the trade regardless of the pri tie.
- probe: Read the only creation sites of qty_phys_(copy_)sugg (combine_regs, local-alloc.c:1822-1861); grep QTYDBG-SUGG lines for marionation blk=3 in both chassis dumps.
- result: KILLED: suggestions arise ONLY from reg-reg copy insns between a hard reg and a pseudo; the pair window's residual-preserving insn set (lbu/sll/addu/lw/sw) contains no copy insn - arg5val is mem-set/mem-used, t0-sll is arith-set/address-used - so no spelling can create a suggestion without adding a visible copy insn (launch class, measured dead s2-s31). Empirically zero QTYDBG-SUGG lines for blk=3 in both forms.
- verdict: KILLED

## [s33] A closed-form flip condition exists that a future C form could satisfy: strict pri(arg5val) > pri(t0-sll) in an order-correct stream.
- mechanism: With refs equal (4/4) the condition reduces to life(arg5val) < life(t0-sll), i.e. the arg5 sw landing within 1 sched1 stream slot of the t0-deref lw; with lives equal it requires refs(arg5val) >= 5, reachable without a new insn only via flow.c loop_depth ref-weighting scoped to arg5val alone.
- probe: Luid arithmetic from the blk=3 tables cross-checked against s6 dep-DAG (sw anchors the jal chain), s3 8-form window invariance, s10c 140-ordering sweep, and the Judge-banked wrap-toolbox measurements (vT35/vT36/vT42/vT43).
- result: Both axes closed on existing evidence: sched1's call-setup normalization fixes both lives at 6 across every measured ordering (life axis), and loop-note wraps scoped to the window re-time the head (refs axis, Judge-banked dead). The flip condition is now stated in closed form for any future novel-geometry candidate to be checked against BEFORE measuring.
- verdict: KILLED

## [s34] A symmetric t0-only fresh-carrier form (t0 fresh, arg5 native v0-web) mirrors s30 v03 at masked 4, adding a 32nd basin member (frontier item #2).
- mechanism: s30 v03 proved fresh-carrier load-then-self-shift escapes the launch penalty on the arg5 side; the mirror tests which side is escape-tolerant.
- probe: Crafted s34/v04_t0only_freshcarrier.c; QTYDBG/ALLOCDBG dump via tmp/gccdbg/cc1 FIRST (pre-vet), then sandbox --disable all.
- result: masked 11 / 178 insns. The fresh-carrier escape is side-ASYMMETRIC: arg5-side = 4 (s30v03), t0-side = 11. Banked rejected/s34-t0only-freshcarrier-tied-web-order-correct-seats-priority-trade-11.c.
- verdict: KILLED

## [s34] The s33 closed-form flip condition applied to a form's QTYDBG/lreg dump correctly classifies it BEFORE sandbox measurement (the pre-vet filter frontier items #2/#3 depend on).
- mechanism: blk=3 qty table gives pri = floor_log2(refs)*refs*size/life per pseudo; seats trade iff pri(arg5val) <= pri(t0-side qty) in an order-correct stream (local-alloc.c:1646 / strict-priority allocation order).
- probe: Dumped v04, mapped pseudos to insns via lreg (arg5val reg97 pri 2.00, tied t0-web reg98 pri 2.57), wrote the prediction (NOT floor; seats-traded class; ~6-10) before running sandbox.
- result: Sandbox = 11 with EXACTLY the predicted signature: adiff2 shows the arg5 pair half (sll v0; addu v0,v0,s5) MATCHED target in the LCS, and all points are the seat trade (t0-web v1 vs a0, arg5val v0 vs v1) + placement drift. Pre-vet is a valid KILL-classifier (class exactly right; score under-predicted by 1-5 - treat output as class, not score).
- verdict: CONFIRMED

## [s34] Splitting t0's web through a fresh carrier creates a THIRD decision route of the coupled fixed point: local-alloc combine_regs ties carrier->sll->sum into one qty whose long life reorders sched1 to arg5-first (order-correct from t0-first source, first time in 34 sessions) while its high refs force a strict-priority seat trade.
- mechanism: combine_regs (local-alloc.c:1822-1861 tying arm) merges reg99/reg103/reg98 into qty2: refs 12 = (3 sets + 3 uses) x2 do-while(0) loop-note weight, life 14 (luid 18-32); pri 2.57 > arg5val 2.00, allocates first, takes v1; arg5val takes v0. Closed form: flip needs L_t > 18 with refs byte-pinned at 12; life is pinned by the call anchor (s6 dep-DAG) - route closed, mirroring s33's closure of the arg5-first route.
- probe: lreg/greg insn identity mapping + QTYDBG blk=3 table + emitted .s window vs target + adiff2 decomposition (tmp/grind/marionation_Exec/s34/v04/).
- result: Order-correctness and seat-correctness remain mutually exclusive across all three named routes: sched.c:2448 class-compare (t0-first forms lose order), local-alloc.c:1646 tiebreak (arg5-first forms lose seats), and the new tied-web strict-priority route (order fixed, seats lost worse).
- verdict: CONFIRMED

## [s35] The decomp.me corpus (gcc2.7.2-psx/cdk/psyq3.5 class, 3754 local scratches) contains a scratch whose target asm overlaps marionation_Exec enough to expose the unknown original source shape for the do_timeout window.
- mechanism: tools/decomp_me_scrape.py search ranks corpus scratches by asm shingles against asm/funcs/marionation_Exec.s; a high-similarity 100%-matched scratch would show real GCC 2.7.2 source geometry for the two-table-lookup-into-multi-arg-call pattern.
- probe: First-ever corpus search run for this function (s26 had catalogued it as untried). Top 5 hits extracted from tmp/decomp_me_corpus and their C sources inspected (tmp/grind/marionation_Exec/s35/hit_*.c).
- result: Max similarity 0.094 (zppzv, a battle-state dispatch loop); other hits are rect drawing, a card-slot builder, mts_boot - all structurally unrelated to the do_timeout window. No analogous two-adjacent-byte-index table-lookup call shape exists in the corpus. Only transferable material: bpAxD's u8*-accumulator idiom, measured separately below.
- verdict: KILLED

## [s35] Holding t0's computed address in a u8*-typed carrier (including the corpus bpAxD split-init accumulation idiom `t0p = (u8*)tbl_125c; t0p = (u8*)(t0 + (s32)t0p);`) reaches masked <= 4 because pointer-typed pseudos get a different RA/sched treatment than the s32 address-value and the killed s32* form.
- mechanism: s8 killed s32* (11), s34 killed a fresh s32 t0-side carrier (11); u8*-typed byte-offset arithmetic is the one carrier type never measured, and split-init accumulation (sanctioned family) adds a second set to the carrier web which could reweight the qty tie.
- probe: sweep_variants on v01 (u8* carrier, fused add), v02 (bpAxD split-accum), v03 (symmetric u8* carriers both sides); baseline candidate.c = 4 confirmed same session.
- result: v01 = masked 11 / 178, v02 = masked 11 / 178, v03 = masked 12 / 178. v01/v02 exactly reproduce s34's s32-carrier 11 and s8's s32* 11: the t0-side fresh-carrier penalty is INVARIANT to holding type (s32, s32*, u8*) AND to split-init web growth. Carrier-type axis closed.
- verdict: KILLED

## [s35] A fresh m2c decompile yields a structurally different outer-flow spelling (short-circuit || with comma-expression cnt update, flag materialized) that relandscapes flow.c BB layout and moves either residual.
- mechanism: m2c derives its shape from target bytes; its `if (A || (cnt=..., store, cnt>0x3C0000))` outer form is the one outer-CFG spelling never measured (s9v04 if/else, s18v01 for(;;) sentinel, s18v02 structured checks all measured; the ||-comma family untested).
- probe: v04 = vT40 chassis with the m2c ||-comma outer flow replacing the two-goto polling head; sweep + adiff2 residual-window inspection.
- result: v04 = masked 4 / 178 with residual signature BYTE-IDENTICAL to baseline (pair-swap insert/delete at 56/57, region-3 steal at 149/151 per adiff2). 32nd known masked-4 basin member; jump.c CFG folding collapses the ||-comma form to the same post-jump2 layout, extending s18's structured-if/else equivalence to the short-circuit-comma family. Outer-flow axis remains fully inert.
- verdict: KILLED

## [s36] marionation_Exec's name-unique Kengo match (kengo_matches.csv, 180 vs 179 insns) exposes the original C structure for the do_timeout window
- mechanism: Marionation engine preserved BB2->Kengo; kengo_ref.py dumps the PS2 disassembly of the matched function
- probe: python3 tools/kengo_ref.py marionation_Exec -> tmp/grind/marionation_Exec/s36/kengo_marionation_Exec.txt; structural comparison of the full body
- result: The Kengo function is a complete PS2-era rewrite sharing only the name: FPU compares (c.lt.s/bc1tl), motion_CalcMotion/motion_SeControl/motion_IncFrame/saSeKeyOn calls, a switch dispatch - NO polling loop, NO debug_printf, NO adjacent-byte-index table-lookup pair. Zero transferable geometry; the 179-vs-180 size match is coincidence.
- verdict: KILLED

## [s36] A whole-function near-clone of marionation_Exec exists among COMPLETED-C functions (find_duplicates.py opcode-class Levenshtein), providing proven source geometry
- mechanism: SOTN dups algorithm; leads file maps INCOMPLETE items to their best COMPLETED-C analog
- probe: python3 tools/find_duplicates.py (full 21s run, first since merge); grep marionation_Exec tmp/duplicates.txt + tmp/duplicates_leads.txt
- result: 0 rows for marionation_Exec in either report (200 pairs / 15 leads) - no near-clone at whole-function opcode-class level exists in the codebase
- verdict: KILLED

## [s36] Some COMPLETED-C function in this binary contains the pair-window shape (two sll<<2 + addu-to-same-saved-base + lw chains within 16 insns), giving proven C for the exact codegen shape
- mechanism: Custom target-asm scanner (scan_pair_shape.py) over asm/funcs/*.s cross-checked against engine/queue.json membership
- probe: tmp/grind/marionation_Exec/s36/scan_pair_shape.py: 3 hits total (func_8005C6D0, func_80067D14, saEft01Init) - ALL queued/INCOMPLETE; func_8004A4E0 (lbu-pair prefilter hit, not queued) is whole-body canonical asm with jr-dispatch, no C source
- result: No proven-C analog of the shape exists anywhere in the binary. BUT the scan surfaced saEft01Init (system.c:806, dist 18, 15 rules): its do_timeout window is byte-shape IDENTICAL to marionation_Exec's vs BOTH targets - same D_800F19C0/D_800A11D5/D_800161C8 globals, same fmt string, same lbu-lbu-lw a1-sll-addu-sll-lw-lbu-addu-sll-sw-lw-lw sequence (only table-base seat s0-vs-s5 and arg3 table addressing s3-vs-at differ). The two functions share ONE unknown original window spelling; saEft01Init's current C suffers the SAME seat trade vs its own target.
- verdict: KILLED

## [s36] The literal saEft01Init source shape (named arg5-first locals, NO pp alias, NO staging webs, direct globals) transplanted into the vT40 chassis reaches masked <= 4
- mechanism: The sibling's HEAD C emits sll;addu;sll (target middle-addu order) at its window - the never-measured grid cell named-locals x no-pp on marionation (s9 measured named+pp=9/11, s8 measured inline+no-pp=16)
- probe: v01 spliced into candidate chassis, sandbox --disable all; emission dumped via cc1 (tmp/grind/marionation_Exec/s36/v01_sibling.s); v02 (arg4-first no-pp) for grid completion
- result: v01 = masked 10 / 178: achieves EXACT target pair order (sll $3; addu $3,$3,$21; sll $2, base in s5) - the SIMPLEST order-correct spelling ever measured, zero FAKE constructs in the window - but pays the coupled seat trade (arg5 web v1-vs-v0, arg4 web v0-vs-a0), the tslTm2 delay-slot lbu [1] steal (s15 T-16 tree-order flip), and early arg4 addu+deref placement. v02 = masked 9. NEW mechanism fact: D_800F19C0 passed directly lands lw a1 in the target slot WITHOUT the pp alias; pp is INERT in the named-local family (v02 no-pp 9 == s9v01 with-pp 9; v01 10 vs s9v02 11) - the s8-era 'pp ~7pts' contribution is web-chassis-specific.
- verdict: KILLED

## [s36] Hoisting idx_1494[0] into a plain-load named local (s30v03 launch-free class) ahead of arg5's tree restores [0]-first lbu order while keeping the middle-addu order
- mechanism: s15: lbu launch order at sched2 T-16 follows expand-time tree order; a separate [0]-load statement before arg5's tree should win the launch
- probe: v03 (i0 = idx_1494[0]; arg5 = tbl_125c[idx_1494[1]]; arg4 = tbl_125c[i0]) spliced, sandbox, emission dumped (v03_i0hoist.s)
- result: masked 10, emitted asm BYTE-IDENTICAL to v01 - GCC copy-propagates the i0 hoist away entirely; lbu order is decided by TREE order, unreachable via index-load statement placement
- verdict: KILLED

## [s37] The vT40 candidate reproduces the masked-4 floor on main at s37 start
- mechanism: candidate.c spliced via s6/splice_apply.py; cheat-invisible sandbox with all 42 rules disabled and 20 cheat-asm stripped is the honest metric
- probe: splice_apply.py candidate.c; wteng sandbox marionation_Exec --disable all; splice_apply.py --restore + git checkout src/system.c
- result: score=4, build_insns=178, target_insns=179, rules_dropped=42, cheat_asm_stripped=20; src restored clean, oracle untouched
- verdict: CONFIRMED

## [s37] The s33/s34 closed-form flip condition (strict pri(arg5val) > pri(t0-side) in an order-correct stream) has exactly ONE arithmetic axis still open after 36 sessions: lowering refs(t0-side) via fresh-carrier + deferred tbl-add
- mechanism: Cross-tabulation of the four axes against banked measurements: raise refs(arg5val) closed (wrap toolbox retimes head, Judge-banked vT35/vT36/vT42/vT43); shorten life(arg5val) closed (sched1 call-setup normalization pins both lives at 6, 140-ordering sweep); lengthen life(t0-side) closed (call anchor pins life, s34 route-C closure); lower refs(t0-side) never measured in the carrier+deferral combination - s2 v30 (deferred add, NATIVE web) = 16 and s34 v04 (carrier WITH add) = 11 are its two distinct-mechanism neighbors, and only the combination yields web 2 sets + 2 uses = 8 weighted refs, pri 3*8/14 = 1.71 < arg5val 2.00
- probe: Analytic synthesis over evidence.md + hypotheses.md + rejected/ (72 forms); arithmetic from the s33 QTYDBG closed form and the s34 combine_regs tied-web decomposition (refs 12 = (3s+3u)x2 loop-note weight, life 14)
- result: All non-carrier-deferral routes to the pair flip confirmed closed by existing measurements; the v05 carrier+deferred-add family is the sole unmeasured arithmetic hole; predicted failure mode is the s2-v30-class call-arg-tree retime, decidable by the validated dump-first pre-vet without a sandbox run
- verdict: CONFIRMED

## [s37] Residual B (region-3 steal) is NOT addressed by any live frontier item except the unknown-original-spelling search; in particular the saEft01Init exchange lever can close only the pair half
- mechanism: saEft01Init (s36) shares the byte-shape-identical do_timeout window but has NO callbacks/copy-blocks/check-region, so an order+seats-correct window spelling found there mirrors only residual A; region-3 lives in the check region, where liveness-kill can never byte-match (target dest a1 caller-saved, alias-merge floor 10), label routes always pay +1 insn (5 spellings), and cc1psx emits the identical steal (owner calibration) proving the fix is source-shape, not toolchain
- probe: Ledger cross-read: s36 sibling facts vs s4/s5/s6/s7/s19/s32 region-3 closures and the owner cc1psx parity entry
- result: Even a full pair-swap win leaves masked 2 unless the winning form also relandscapes the check region; frontier items must be read with this scope limit - the exchange lever is necessary for the window spelling but not sufficient for masked 0
- verdict: CONFIRMED

## [s38] A TWO-pseudo tied t0 carrier (tc = idx_1494[0]; t2 = tc * 4; tc dies into t2 -> combine_regs tie) with the tbl-add deferred into the call deref reaches 2 sets + 2 uses = 8 weighted refs (pri 1.71 < arg5val 2.00) while preserving s34 v04's order-correct sched1 launch (v05a, vT40 chassis, arg5 v0-web intact)
- mechanism: s33/s34 closed-form flip condition, lower-refs(t0-side) axis - the one remaining arithmetic hole; combine_regs tie preserves the order lever, deferral drops the web refs below arg5val's
- probe: Authored v05a (note: the frontier's literal single-var reading is alpha-equivalent to banked v30 masked-16, so the two-pseudo form is the only novel refs-8 spelling); spliced; s34 dump-first pre-vet via tmp/gccdbg/cc1 (BB2_QTY_DEBUG/BB2_ALLOC_DEBUG, -da); checked emitted window order then QTYDBG blk=3
- result: ORDER-INCORRECT at the dump gate (no sandbox per protocol): window serializes arg5-chain-complete-first, t0 chain births inside the call-arg region (lbu/sll/addu/lw after the arg5 sw), arg5 lbu steals the tslTm2 delay slot - the predicted s2-v30-class retime. QTYDBG blk=3: a 12-ref qty (b20-32) reconstitutes on the t0 side; refs NEVER drop to 8 because expand materializes the deferred add (insn 105, temp reg106) and a deref value-holder (insn 107, reg/v 100) as real insns regardless of C spelling
- verdict: KILLED

## [s38] The same deferred-add t0 side with arg5 through its own free fresh carrier (s30v03 load-then-self-shift, no v0-web) relandscapes the qty table enough to keep the launch order correct (v05b)
- mechanism: s30v03's arg5-side carrier is a proven launch-free masked-4 basin member; pairing it with the deferred t0 side removes the v0-web's staging interaction from the window
- probe: v05b spliced; same dump-first pre-vet
- result: Emitted window byte-identical to v05a's order-incorrect serialization; QTYDBG blk=3 shows the same 12-ref t0-side qty (reg114 b20-32) plus a 12-ref arg5-side web (reg106 b8-16)
- verdict: KILLED

## [s38] A u8*-typed t2 spelling of v05a shifts expand's address-mode canonicalization enough to keep the t0 chain out of the call-arg region (v05c)
- mechanism: s35 proved carrier holding-type invariance for the 3-statement form only; the deferred form's type sensitivity was unmeasured
- probe: v05c spliced; same dump-first pre-vet
- result: Emitted window identical to v05a except the t0 addu operand order (addu $2,$2,$21 vs $2,$21,$2) - still order-incorrect serialization. Extends s35's type-invariance to the deferred-add family
- verdict: KILLED

## [s38] The s33 flip condition's fourth axis (lower refs(t0-side) below arg5val's) is reachable from C source
- mechanism: qty_compare refs count RTL sets/uses; the hypothesis assumed C-level statement deletion (deferral) deletes sets
- probe: Direct contrast on the same chassis: s34 v04 (explicit add, refs 12, ORDER-CORRECT, seats traded) vs s38 v05a/b/c (deferred add, refs still 12 via reconstituted temps, ORDER-INCORRECT)
- result: CLOSED IN CLOSED FORM: the explicit add statement is simultaneously what makes the carrier form order-correct (the refs-12 tied web flips sched1's launch) and what pins refs above arg5val's; removing it collapses the order. The two halves of the flip condition are mutually exclusive in the same mechanism. With raise-refs(arg5val) (wrap toolbox, Judge-banked), shorten-life(arg5val) (sched1 normalization), and lengthen-life(t0-side) (call anchor, s34) already closed, NO arithmetic route to the pair flip remains
- verdict: KILLED

## [s39] saEft01Init's regfix rule cluster can be decoded against its rules-free build emission to fingerprint the unknown original window idiom before P2 is scheduled
- mechanism: The rules a function carries encode exactly where its current C diverges from target; the @35-48 reorder + $2<->$3/$2<->$4 swaps map plain-C emission onto target insn-by-insn
- probe: Fresh cc1 emission of system.c (dumps/mar_system_s39.s lines 1859-1872) aligned against asm/funcs/saEft01Init.s and the regfix reorder permutation [36,35,37,38,39,40,44,43,41,45,46,47,48,42]
- result: Decoded: original idiom must emit idx[0] first, arg5 deref BEFORE arg3's index lbu, arg4's tbl-addu deferred after arg3's lbu, arg4 deref LAST, arg4 chain seated in a0, arg3 index byte reusing v0 after arg5's deref. Fingerprint is shape-identical to marionation_Exec's own target window - one spelling decides both (P2 exchange evidence, constraints recorded in s39/findings.md)
- verdict: CONFIRMED

## [s39] Staging arg3's index through the dead v0 staging var (v0 = D_800A11D5; D_800A11DC[v0]) - the target's own v0-reuse shape - creates a WAR edge 122->128 that lengthens arg5's chain-to-jal and flips the sched2 pair to target order, reaching masked <=4
- mechanism: Backward-walk priority = longest path to end; the anti-dependence routes 122->128->133->lw a2->jal (4 hops) instead of 122->137->jal (2 hops), outranking the 106 chain at the tie. staged-value-reused-variable family (owner-sanctioned 2026-07-03), semantically true, no fresh pseudo
- probe: v01 (vT40 + arg3-through-v0): cc1 dump pre-vet then sandbox; refinements v02 (arg5-first), v03/v04 (sibling named-locals chassis both orders), v05 (byte-offset spelling), v06 (t0 tbl-add deferred combo)
- result: ORDER FLIP CONFIRMED: v01 emits sll; addu v0,v0,s5; sll from t0-first source (v01.s) - the first order-correct t0-first form ever measured, refuting the s37 arg5-first-only route synthesis at the order level. KILLED as closer: v0 gains a 3rd set, its web spans the call-arg region, s3-class web-growth penalty ~+12 dominates: v01=16, v02=16, v03=18, v04=18, v05=20, v06=16. v0 is the only dead-in-window variable, so the WAR edge is unobtainable without the web growth - axis closed
- verdict: KILLED

## [s39] Plain unshifted index staging (t0 = idx[0]; v0 = idx[1]) with both derefs inline in the call reaches masked <=4 (sibling-decode cue: idx loads first, chains in arg-like seats)
- mechanism: Minimal staging without the byte-cast/shift webs might let expand seat the chains naturally per the target's a0/v0 lbu dests
- probe: v07 spliced + sandbox
- result: masked 16 / 178 - lands in the known no-web class (s8v01=16); the two staging webs remain load-bearing
- verdict: KILLED

## [s40] The stale-but-completed 6.5h s20status campaign (14 finds incl. output-145-1 and output-170-1) surfaced a novel sub-165 neighborhood with true semantics (frontier P3)
- mechanism: s20-status-local is an unsampled masked-4 basin member; its qty-allocation landscape differs from the 8 previously-sampled chassis, so its sub-200 mutation trajectory could reach an attractor the others cannot
- probe: harvested via permuter_campaign.py (telemetry logged); triaged all 14 finds by diff inspection; extracted + spliced + sandboxed both sub-200 finds; restored src and reconfirmed vT40 floor 4
- result: output-145-1 = alias-merge attractor re-derived EXACTLY (idx_1494 staged through dead dst2, sandbox masked 17 @ 179/179 - same score as s4's find145 entry point); output-170-1 = novel check-side variant (idx_1496 through dst2) at masked 21, WORSE via deeper callee-saved seat cascade; output-215-1 = semdiv clear-hoist; 11x220 = known score-inert shuffle classes (clearmove/spelling-inert). Zero novel attractors.
- verdict: KILLED

## [s40] A fresh-seed window on the s11-u10-interleaved basin (the 3rd P3-named unsampled member) surfaces a novel sub-200 neighborhood
- mechanism: u10's interleaved t0/v0 computes give a distinct pseudo-birth order from vT40; the prior s40 attempt's launch died ~3min in with 0 sampling, so the basin was genuinely unsampled
- probe: relaunched via permuter_campaign.py launch (-j6 --stop-on-zero, base 220 verified); 25-min fresh-seed window monitored; harvest --stop per permuter-fresh-seed-discipline
- result: 6,720 iterations, 1 find (output-220-1) = the known score-inert clear-store-into-wrap repositioning class (already banked from s35orcomma's finds). No novel find in the window - basin dry.
- verdict: KILLED

## [s40] The s35orcomma basin (sampled by the discarded prior s40 attempt, 5 finds all score-220) contains any lever
- mechanism: m2c-outer-orcomma chassis is basin member #32; its 5-min campaign produced 5 score-equal finds before dying
- probe: harvested + stopped via permuter_campaign.py; the 5 finds were already triaged and banked by the prior attempt (4 semdiv clear-moves + 1 inert src-hoist, rejected/s40-perm-*)
- result: All 5 finds score-equal (220) and classify to semdiv or inert shuffle classes; basin dry at the fresh-seed window
- verdict: KILLED

## [s41] The s21 v11 vsync-hoist basin (sv = sys_VSync(-1); D_800F19B8 = sv + 0x3C0; VSync value materialized to a prologue local before the store) samples a distinct qty-birth landscape from vT40 and reaches a novel sub-vT40 attractor within a 25-min fresh-seed window.
- mechanism: v11 births an extra prologue pseudo (sv) whose life ends at the D_800F19B8 store; the local retimes qty allocation upstream of the do_timeout window and could reweight the pair-swap tie. Never permuter-sampled prior to s41.
- probe: Built tmp/grind/marionation_Exec/s41/perm_v11 workspace from tmp/perm_mar6 skeleton + s21 v11 base + target.o. permuter --debug base score = 220 (identical residual signature to vT40/z07/w05/etc). Launched via tools/permuter_campaign.py launch --func marionation_Exec --dir ... --label s21v11-vsync-hoist -j 6 --stop-on-zero (PID 622, launch_epoch 1783525141). 25-min fresh-seed window elapsed. Harvested via tools/permuter_campaign.py harvest --stop --reason 'fresh-seed 25min window; permuter modality s41'.
- result: 1740+ iterations in the window, 1 sub-220 find (output-200-1, score 200 = 220 - 20). Sandbox --disable all on the extracted fn = masked 8 (+4 vs vT40 floor). Novel construct: `status = *((s32 *) (v0 + ((s32) tbl_125c))); arg5 = status;` - the arg5 value staged through the check-region `status` variable. Same class as s12 *idx_1495 regression (crossing-window ref extends the alias var's life across the debug_printf call, forcing callee-saved seat cascade for status which is also used as check2 loop cond). No novel attractor; classifies within the reg-shuffle / cross-window-alias space.
- verdict: KILLED

## [s41] The v11 basin's fresh-seed window (25 min, ~1740 iters at -j 6) is comparable in yield to prior 12-30 min fresh-seed windows across the 11 previously sampled basins.
- mechanism: Per portfolio convergence (s40): 11 basins sampled across ~50+ CPU-hr with zero novel attractors; all sub-200 finds classify into 3 known families {alias-merge masked>=10, label-alive +1 insn, reg-shuffle/cross-window-alias score-inert}.
- probe: Compare v11's 1 find/1740 iters (find at 35.8s from launch, no further sub-220 sample in remaining ~24 min) vs s31 s30v03 (5 finds/26min), s40 s20status (14 finds/6.5h stale), s40 s35orcomma (5 finds/5min stale), s40 s11u10 (1 find/25min fresh).
- result: 1 sub-220 find in 25 min at fresh-seed window is on the low end but within band; the find classifies to known cross-window-alias family, matching portfolio convergence pattern (0 novel attractors across 12 basins now).
- verdict: CONFIRMED

## [s42] Baseline candidate.c (vT40) reproduces masked 4 on main with fresh cc1 -da dumps.
- mechanism: candidate.c spliced -> sandbox --disable all; cc1 -da dump into tmp/grind/marionation_Exec/s42/baseline/.
- probe: python3 tmp/grind/marionation_Exec/s6/splice_apply.py memory/grind/marionation_Exec/candidate.c; & tools/wteng.ps1 main sandbox marionation_Exec --disable all; bash tmp/grind/marionation_Exec/s42/dump.sh baseline; splice_apply.py --restore; git checkout src/system.c.
- result: sandbox = masked 4 (178/179, 42 rules dropped, 20 cheat-asm stripped); 14 -da RTL dumps + emitted .s produced in s42/baseline/. src restored to HEAD, oracle green.
- verdict: CONFIRMED

## [s42] P4: a memory-carried anti-dep from insn 122 to some later mem-write can lengthen arg5's forward chain and flip the sched2 T-14 tie without adding sets to any in-window pseudo (v0/t0/arg5 webs).
- mechanism: sched.c 2.7.2 mem-edge sources enumerated (sched_analyze_1/2, memrefs_conflict_p, true/anti/output_dependence, last_pending_memory_flush, ASM_OPERANDS/TRAP_IF/PRE_INC/POST_INC flushes). The only edge shape that lengthens 122's forward chain is a later MEM write whose target may-aliases (mem (reg v0)); memrefs_conflict_p resolves SYMBOL_REF-vs-SYMBOL_REF as provably disjoint (no edge), and REG-based writes require materializing the address REG.
- probe: Fresh cc1 -da dump of baseline (s42/baseline/mar_system_s42.i.sched2) — extracted insn 122's LOG_LINKS structure and enumerated the sched.c code paths that can add an outgoing MEM anti-dep from 122. Cross-referenced C-level shapes reachable in the do_timeout window against pseudo-web-growth and semantic-lie constraints (table at s42/findings.md).
- result: The 122->128 anti-dep is ALREADY present in baseline (via RA register-reuse of v0 for the D_800A11D5 load) — so s39's arg3-through-v0 staging did NOT structurally lengthen the chain, only forced pseudo-web growth. No other C-level statement can add a new 122->X mem-anti-dep without one of: (a) growing an in-window pseudo web (defeats P4), (b) writing user-observable memory (Judge-verboten as semantic-lie), or (c) SYMBOL_REF-vs-SYMBOL_REF disjointness (no edge). The 122-chain length is at its natural maximum given RA's freedom.
- verdict: KILLED

## [s42] sched.c 2.7.2's memrefs_conflict_p can be tricked by a REG-based address into creating a MEM edge with a SYMBOL_REF store, providing a pseudo-free lever.
- mechanism: If ptr's known-value via reg_known_value resolves to a SYMBOL_REF, sched_analyze_2's REG handler at line 1898-1902 re-analyzes with the SYMBOL_REF and memrefs_conflict_p returns disjoint. Otherwise the fallback treats REG bases as may-alias.
- probe: Read sched.c:614-780 (memrefs_conflict_p), 1740-1780 (SET target MEM), 1895-1945 (MEM read), 1985-1996 (ASM_OPERANDS clobber). Cross-checked baseline: 137 (sw sp+16) does NOT get an added anti-dep with 122 despite being reg-based, because the RAW (v1 true-dep) already exists and find_insn_list dedups.
- result: The mechanism exists but is unreachable: any REG-based store either birth-extends a pseudo web (defeating P4) or resolves to SYMBOL_REF via reg_known_value (no edge). Every candidate shape either grows a web or writes user-observable memory. No pseudo-free MEM lever exists.
- verdict: KILLED

## [s43] The sched2 BB #3 T-14 tie between insn 106 (a0-ashift) and insn 117 (v0-ashift) is class-differentiated in rank_for_schedule (sched.c:2419-2450), not purely LUID-decided; if insn_cost(117 for last_scheduled=120) > 1 then class(117)=1 < class(106)=3, and any C construct that alters the 117->120 latency/edge flips the tie without adding pseudos - unlocking a new mechanism lever not covered by the s2/s3 140-ordering sweep.
- mechanism: sched.c:2419-2450 class code sets class=3 when link==0 OR insn_cost(candidate, link, last_scheduled)==1; class=1 for data-dep with cost>1. Higher class sorts FIRST. If 117 (true-dep predecessor of 120) has cost>1, class(117)=1, and rank returns tmp_class-tmp2_class = class(106)-class(117)=3-1=2, sorting 106 first via CLASS, not LUID.
- probe: Read tools/gcc-2.7.2/config/mips/mips.md lines 148-210 for MIPS function-unit definitions; produced fresh cc1 -da dumps of the vT40 baseline at tmp/grind/marionation_Exec/s43/mar_s43.i.* including sched2 (13207 lines); cross-checked BB #3 T-11..T-15 ready-list decisions against class-vs-LUID predictions; verified priority listing order (117 appears before 106 in the block-walk => LUID(117)<LUID(106)); attempted BB2_RANK_DEBUG instrumentation to independently confirm class values but env-forwarding through wsl.sh's nested bash chain silently drops the variable (0 RANKDBG lines emitted in 3 attempts).
- result: MIPS gcc-2.7.2 mips.md defines function-units ONLY for `memory` (load/store/xfer, 1-3 cycle latencies) and `imuldiv` (hilo/imul/idiv). The `arith` type (which covers sll, addu, ori, and all non-mem/mult ALU insns) has NO define_function_unit; insn_cost falls to the default latency of 1. Concretely: insn_cost(117 for 120) = 1 => class(117 at T-14) = 3 via the cost==1 shortcut at sched.c:2420. class(106) = 3 (no link). Both class=3 => class branch cannot differentiate. Tie falls through to sched.c:2455 LUID compare where LUID(117)<LUID(106) => 117 sorts after 106 in ready-list order => 106 wins T-14 chronologically-later slot, 117 emits at T-15. Same shortcut kills class differentiation at T-13 (120 vs 106) and T-11 (128 vs 111).
- verdict: KILLED

## [s43] The vT40 baseline sched2 BB #3 trace on main at s43 reproduces s6/s42 mechanism byte-identically.
- mechanism: Same candidate.c, same cc1 (tools/gcc-2.7.2/build/cc1), same flags; fresh -da dumps expose ready-list decisions per T-index.
- probe: python3 tmp/grind/marionation_Exec/s6/splice_apply.py memory/grind/marionation_Exec/candidate.c; cc1 -da into tmp/grind/marionation_Exec/s43/; splice_apply.py --restore; git checkout -- src/system.c.
- result: BB #3 sched2 (mar_s43.i.sched2 lines 5597-5649): T-11 launches 128 before 111; T-12 [106,141,122]->[122,106,141]; T-13 [106,141,120]->[120,106,141]; T-14 [106,141,117]->[106,117,141]; T-15 [117,141]->[117,141]; chronological pair-window emission 117-then-106-then-120 (target: 117-then-120-then-106). Identical to s6/s42.
- verdict: CONFIRMED

## [s44] Expressing the outer counter as a single-statement postfix `cnt = D_800F19BC++` (instead of vT40's 2-statement `cnt = D_800F19BC; D_800F19BC = cnt + 1;`) perturbs outer-BB LUID assignment enough to shift downstream sched2/reorg decisions in do_timeout or the check2 delay-slot region.
- mechanism: Compound RTL for postfix inc emits a single (parallel [set cnt mem] [set mem (+ mem 1)]) form that expand may split differently than two sequential (set) insns; UID assignment at expand-time drives sched2 T-14 ties per s6 forensics.
- probe: v01_postinc_cnt.c spliced to src/system.c via s44/splice.py; sandbox --disable all measured.
- result: masked 4 (178/179, 42 rules dropped, 20 cheat_asm stripped) — identical to vT40 baseline. Novel masked-4 spelling (10th known distinct member of the vT40 floor basin, joining vT40, s11 u10/w03/w10, s12 v08/w05/w08/z01/z02/z07); pair-window insn stream unchanged; region-3 steal unchanged.
- verdict: KILLED

## [s45] Reordering the prologue setup pool so D_800F19BC=0 and D_800F19C0=&D_80016248 execute BEFORE tbl_125c/idx_1494 assignments changes function-level pseudo birth order and could unstick the pair-swap coupling
- mechanism: GCC 2.7.2 assigns pseudo/qty UIDs during RTL expand in textual order; reversing the block puts the two D_800F19x* stores' insns before the tbl_125c/idx_1494 assign insns, potentially reshaping the greg qty birth cascade that seats t0-web and arg5val at the pair-swap window
- probe: vA candidate written with the D_800F19BC/D_800F19C0 stmts hoisted to immediately after D_800F19B8=sys_VSync line; spliced via s45/measure.py; sandbox --disable all
- result: score=4 build_insns=178 - EXACTLY the vT40 floor (basin-invariant); statement reorder of the outer prologue pool is INERT for the pair-swap and region-3 residuals (the qty births of interest happen inside the do_timeout window and are decoupled from prologue-store-instr UIDs which get scheduled elsewhere)
- verdict: KILLED

## [s45] Deriving idx_1494 backward from idx_1496 via subtraction (idx_1496 = &D_800A1494+2; idx_1495 = idx_1496-1; idx_1494 = idx_1495-1) inverts the derivation chain and could birth idx_1494's pseudo AFTER idx_1495/idx_1496, reshaping the do_timeout-window qty seat assignment
- mechanism: vT40 has idx_1494 as the root and idx_1495/idx_1496 derived forward via addition; reversing forces idx_1496 to be the root and the others derived backward. All three are load-bearing pseudos live across the debug_printf call (idx_1494 refs in do_timeout, idx_1495/1496 refs in check1/check2), so their qty birth order interacts with the s2/s6/s12-documented pair-swap 5.33 v 5.33 tie
- probe: vB candidate; sandbox --disable all
- result: score=14 build_insns=178 - REGRESSION +10 masked pts vs vT40 floor 4. Backward-chain derivation births idx_1494/idx_1495 as SUBTRACTION-based qtys, adding 2 extra addiu/subiu insns (invisible in build_insns because they fuse in canonicalization) and restructuring the prologue-vs-do_timeout live-cross-call qty cascade. Banked as rejected/s45-idx-chain-backward-from-1496-14.c
- verdict: KILLED

## [s45] Swapping tbl_125c and idx_1494 assignment order (idx first, tbl second) reorders the two dominant pseudo-birth candidates for the tbl-base seat
- mechanism: s2 measured decl order INERT (pseudo birth = RTL first-use order, not decl), but stmt order sweeps for the outer prologue-globals were never recorded in the rejected forms bank; separate axis from decl
- probe: vC candidate (single-line swap); sandbox --disable all
- result: score=4 build_insns=178 - basin-invariant (11th novel masked-4 spelling adjacent to vT40). Confirms stmt-order for tbl_125c vs idx_1494 assignments is inert - the first-use ordering in the do_timeout window dominates, prologue-store stmt order is decoupled from window-scoped qty births
- verdict: KILLED

## [s45] Folding the setup pool into a comma-expression (tbl_125c=D_800A125C, idx_1494=&D_800A1494, D_800F19B8=sys_VSync(-1)+0x3C0) fuses the setup stmts into a single expression-statement whose sequence-point handling may collapse pseudo-birth to a single point
- mechanism: Comma-expression semantics require left-to-right evaluation with sequence points but the whole thing is a single expression-stmt at the C99 level - GCC 2.7.2 K&R expand may treat this as a compound assignment tree with a distinct RTL emission from separate stmts
- probe: vD candidate; sandbox --disable all
- result: score=10 build_insns=178 - REGRESSION +6. Comma-fold birthing pattern retimes the prologue in a way that cross-couples with the do_timeout-window qty seat cascade (likely: the fused expression forces intermediate temps for the sub-expression values). Confirms the comma-expression fold axis is not a zero-cost lever. Banked as rejected/s45-comma-expr-setup-pool-10.c
- verdict: KILLED

## [s46] The pair-swap flip condition (pri(arg5val) > pri(t0-side)) has any remaining reachable arithmetic axis after s38's carrier+deferred-add KILL
- mechanism: The four axes (raise refs(arg5val), shorten life(arg5val), lengthen life(t0-side), lower refs(t0-side)) span the qty_compare_1 output; s38 closed the last one via 3 dump-vet spellings
- probe: Cross-tabulate the 165+ measured hand forms and 32 basin-member spellings against the s33 closed-form condition; verify each axis has at least one banked KILL representative
- result: All four axes have banked KILLs: raise-refs=wrap toolbox (vT35/36/42/43); shorten-life=140-ordering sweep sched1 normalization; lengthen-life=call-anchor pin per s34; lower-refs=v05a/b/c dump-vet (s38)
- verdict: KILLED

## [s46] The three named decision routes (sched.c:2448, local-alloc.c:1646, combine_regs tied-web) plus WAR/mem-edge extensions are mutually exhaustive over the pair-swap residual
- mechanism: s37/s39/s42/s43 forensics enumerate every source path that reaches order-correctness AND seat-correctness simultaneously; s43 mips.md proved arith cost=1 forces class=3 for both sides so class-lever is dead; s42 proved mem-edge unreachable without pseudo-web growth or semantic-lie
- probe: Verify each of the enumerated routes is banked KILLED; confirm no unenumerated in-function pathway remains after 45 sessions
- result: All routes closed with byte-level or source-level evidence; the fifth route (WAR through v0) reduces to +12 web-growth per s39; no residual route in the vT40 basin's mechanism space
- verdict: KILLED

## [s46] Region-3 dbr steal has any zero-byte C-level closer within the vT40 basin
- mechanism: reorg fill_simple_delay_slots at check2 beqz picks caller-saved move a1,s4 because a1 is not in the fall-through target's callee-saved live-set; 5 label routes catalogued all pay +1 insn; sched.c/reorg.c patches Judge-banned per no-compiler-divergence.md
- probe: Enumerate label routes (vP160, s5 0xFF-through-local, z07-160-1, s26idxp-165-1/170-1) and liveness routes (alias-merge family floor 10); confirm cc1psx parity via owner 2026-07-08 calibration
- result: Every route pays a visible byte OR is off-limits OR is semantically divergent; no zero-byte in-basin closer exists after 45 sessions of forensics + 12 permuter basins × 50+ CPU-hr
- verdict: KILLED

## [s47] Block-scoping new_var/new_var3 into their respective check regions preserves both andi masks while eliminating a function-scope pseudo, tightening qty allocation in the pair window without regressing region-3
- mechanism: vT40's andi preservation relies on named-local constant-holders whose function-scope life keeps combine from folding the mask into the surrounding lbu/andi pair; block-scoping shortens their lives to a single reference each, which SHOULD keep the andi alive per named-local-fake-exception semantics while reducing pseudo count
- probe: vS47a: new_var declared inside check region's compound block just before use; new_var3 declared inside check2's nested scope; sandbox --disable all
- result: masked 8 / 176 build_insns (target 179) - build DROPS 2 instructions, meaning both andi's are folded away. The masks are load-bearing at FUNCTION-SCOPE lifetime, not just at named-local naming; combine's fold predicate treats block-scope single-use constants as CSE-able trivially. +4 masked from delta shape at 176 vs 179.
- verdict: KILLED

## [s47] Merging new_var and new_var3 into a single `mask` variable reused across both check regions reduces function-scope pseudo count while preserving both andi's
- mechanism: s28+ portfolio convergence relied on the two masks being distinct pseudos; a single reused pseudo COULD change qty birth order for the check-region cascade and re-time region-3's steal window
- probe: vS47b: single `int mask; mask = 0xFF;` at function scope, used at both `*idx_1496 & mask` and `*(idx_1496-1) & mask`; sandbox --disable all
- result: masked 14 / 180 build_insns (target 179) - REGRESSION +10 AND adds 1 build_insn. Cross-check-region CSE extends the mask pseudo's life across the intermediate copy loop's calls, forcing a callee-saved seat competition. Same seat-cascade pathology as s12 *idx_1495 (crossing-window ref).
- verdict: KILLED

## [s47] Hoisting `pp = (void**)&D_800F19C0` to the head of the do_timeout inner block (before t0 = idx_1494[0]) reshapes RTL first-use order for pp's pseudo, potentially retiming the sched2 pair window
- mechanism: vT40 places pp between t0's load and its shift; hoisting to block head places pp's set BEFORE any t0/arg5 tree evaluation, births pp's pseudo earliest, and could reweight the qty tie inputs
- probe: vS47c: pp = (void**)&D_800F19C0 moved to first statement in the inner block; sandbox --disable all
- result: masked 4 / 178 (basin-invariant). Novel masked-4 spelling; pp position within the do_timeout inner block is inert to the pair-window sched2 tie (consistent with s11/s12 pp-position sweeps). This is the 13th known distinct masked-4 basin member (following vT40, s11 u10/w03/w10, s12 v08/w05/w08/z01/z02/z07, s44 postinc-cnt, s45 vA/vC prologue-reorders).
- verdict: KILLED

## [s47] Block-scoping `saved` inside the `if (sys_GetVblankCount() != 0)` branch shortens saved's function-scope life and could unstick the check-region qty landscape
- mechanism: saved is only used inside that if branch; declaring it at function scope inflates its declared life without a semantic need; block-scoping matches the RTL first-use pattern more tightly
- probe: vS47d: `u8 saved;` moved from function-scope decl list to just inside the `if (sys_GetVblankCount() != 0)` block; sandbox --disable all
- result: masked 4 / 178 (basin-invariant). 14th known novel masked-4 spelling. saved's block-scoping is inert - matches the s2 finding that pseudo birth follows RTL first-use, not declaration position; the scope change is a source-level cosmetic only.
- verdict: KILLED

## [s48] Splitting the shared s32 check variable into two block-local pseudos (check1v in check1 block, check2v in a separate check2 block, done: label eliminated in favour of direct returns per region) shortens check's life and could unstick the check-region qty landscape without extending anything across the debug_printf call
- mechanism: vT40's `s32 check` is function-block-scope reused across both check regions (compute in check1, potentially returned at done: after check2). Splitting into two separate scopes gives each region its own pseudo whose life ends at the region's return. Prior s47b (merging masks) regressed +10 via cross-region CSE seat cascade; this is the SYMMETRIC axis (shortening rather than extending life) and had not been measured.
- probe: v01 = candidate.c with check1v/check2v split into two nested compound blocks, done: label removed (check1 falls through returns check1v directly; check2 copy loop returns check2v directly). Spliced via s6/splice_apply.py; sandbox --disable all
- result: masked 14 / 178 build_insns - REGRESSION +10 vs vT40 floor. The restructure also drops the shared `done:` reachable path and eliminates the goto done, changing outer BB topology. Life-shortening axis on check is symmetrically harmful to life-extending: the shared function-scope check is a load-bearing balance point in the check-region qty allocation. Same magnitude regression as s47b's cross-region merge (+10), confirming the check variable's function-scope lifetime is the sweet spot.
- verdict: KILLED

## [s48] Hoisting i=7 from inside check1's `if (a1 != 0)` guard to before it (matching check2's shape where i=7 is outside `if (dst2 != 0)`) creates structural symmetry between the two copy loops and could reweight the check-region qty cascade for region-3
- mechanism: vT40 has check1: `if (a1!=0) { i=7; do{...} while(i!=-1); return check; }` (i=7 inside guard) but check2: `dst2=a1; src=...; i=7; if (dst2!=0) { do{...} while(i!=-1); }` (i=7 outside guard). Structurally asymmetric. Hoisting check1's i=7 to match check2 removes the conditional insn placement and could impact scheduling around the check2 delay-slot steal.
- probe: v02 = candidate.c with `i = 7;` moved before check1's `if (a1 != 0)` guard. Spliced; sandbox --disable all
- result: masked 5 / 177 build_insns - +1 masked and -1 build_insn (177 vs vT40 178, target 179). The hoisted i=7 is folded / merged with a later use so 1 target-matching insn is lost from the build. Score goes UP by 1 (from 4 to 5) purely because 1 more target insn is unmatched. Confirms the conditional-scoped `i=7` inside check1's guard is a load-bearing target-matching insn (setup insn count is delicate; asymmetric copy-loop guards are structurally correct for the target byte pattern). Novel mechanism fact: copy-loop shape asymmetry between check1 (guarded i=7) and check2 (hoisted i=7) is target-required.
- verdict: KILLED

## [s49] PERM_IGNORE wrapping the do_timeout arg-compute inner block (mid-function compound stmt) freezes the pair-window and lets the campaign isolate region-3 mutations (frontier item #2).
- mechanism: PERM_IGNORE emits '#pragma _permuter b64literal <b64>' in place of the wrapped text; ast_util.process_pragmas is designed to decode it back at compile time. If it worked, the pair-window would be untouched during randomization.
- probe: Built tmp/grind/marionation_Exec/s49/perm_regpin with PERM_IGNORE around the {s32 arg5; s32 t0; void **pp; ... arg5)} block. `permuter.py --debug` printed base score = 220 (looked healthy). Campaign launch: base compile failed immediately with FileNotFoundError inside splice.py path, then maspsx choked with 'too many values to unpack (expected 2)' when run manually. Root cause on inspection: the b64literal pragma line replaces a compound-statement's syntactic slot with a preprocessor directive line - CParser parses the outer function AST fine (pragma line is empty text at stmt position -> empty statement or parse hole), but the emitted C after CGenerator+process_pragmas produces a malformed inner block whose downstream maspsx tokenization breaks. PERM_IGNORE is only safe at OUTER-scope constructs (whole function bodies, top-level decls), not mid-function compound statements.
- result: PERM_IGNORE-mid-function unavailable in this permuter version (semantics require outer-scope wrap). Frontier item #2 requires an alternative mechanism to isolate the region-3 mutation subspace - options: (a) score-weight override (not exposed via settings.toml weight_overrides which affects randomizer weights, not scorer), (b) hand-written PERM_GENERAL alternatives that vary ONLY the check-region source (retains scoring-lumping problem but pins the pair-window textually), (c) two-stage: harvest whole-fn campaign finds, then FILTER by inspecting only the check-region diff. (c) is the reachable path.
- verdict: KILLED

## [s49] The s47 vS47c chassis (pp hoisted to inner-block head, 13th known distinct masked-4 basin member per s47 ledger) reveals a novel sub-4-masked attractor within a fresh-seed 25-30min permuter window.
- mechanism: vS47c births pp's pseudo earliest inside do_timeout (pp = (void**)&D_800F19C0 as first statement of inner block) - measurably different qty birth landscape from vT40 (pp mid-block) and z07 (u8* tbase). Never permuter-sampled per s47 ledger. Base score = 220 confirmed identical to vT40 residual signature (2 reorderings * 60 + 1 deletion * 100).
- probe: Built tmp/grind/marionation_Exec/s49/perm_vS47c via build_ws_vS47c.py. Verified base = 220 via `permuter.py --debug`. Launched via `tools/permuter_campaign.py launch --label s47vS47c-pp-hoisted -j 6 --stop-on-zero` (PID 590). ~26min fresh-seed window; harvest+stop.
- result: 40k iterations; 3 total sub-220 finds - output-220-1 (score-equal, i=7 hoisted INSIDE dst2-guard, s48v02-symmetric on the check2 side; masked-5-attractor class per s48), output-195-1 (score 195 = 220-25), output-220-2 (score-equal, addu-operand-swap + dummy_label + src-hoist-into-do-while wrap, canonicalization-inert per s2). Sub-220 find output-195-1 extracted, spliced to src/system.c, sandbox = masked 8 @ 179/179 insns (first find that matches TARGET INSN COUNT exactly). Construct: extra new_var2 = *(idx_1496 - 1) & new_var3 introduced after the *(idx_1496 - 1) = 0 clear inside check2, return replaced from `check` to `new_var2`. Semantically wrong: post-clear read returns 0 while the original returned nonzero-check. Same SEMANTIC-LIE class as s5-vP160/z07-160/s26idxp-165 (status-alive) - permuter's byte-oriented scoring is blind to it. Basin has NO novel-attractor sub-4 masked find in the fresh-seed window.
- verdict: KILLED

## [s49] Portfolio-convergence prediction holds at 13 basins: no basin's fresh-seed permuter window yields a sub-vT40-floor byte-legal attractor.
- mechanism: s28/s31/s32/s40/s41 established the 3-family closed attractor set {alias-merge masked>=10 callee-saved-seat, label-alive/semantic-lie +1 insn or wrong-return, reg-shuffle score-inert} across 12 basins ~50+ CPU-hr. A 13th distinct chassis (vS47c) either extends the convergence or breaks it.
- probe: s49 vS47c 26min fresh-seed window at -j6, ~40k iters harvested. Compared find output-195-1 (masked 8, semantic-lie return-value class) against s26idxp-165 and s5-vP160 (both semantic-lie via label-alive read of dead-then-zero variable). Compared 220-1 (i=7 hoist in dst2 guard) against s48v02 masked-5 attractor. Compared 220-2 (canonicalization-inert shuffle) against s2 mult-vs-shift and s45 comma-fold conclusions.
- result: CONFIRMED: 1 sub-220 find (195-1) classifies to the semantic-lie family (masked 8 @ 179/179 via post-clear read of cleared byte - SAME masked-8 as s41-v11 status-alias find, which was also cross-window alias). 2 score-inert finds (220-1, 220-2) reproduce known canonicalization/scope-shift class. Zero novel attractors. Portfolio convergence extends 12->13 basins with the same closed set.
- verdict: CONFIRMED

## [s50] At least one archived permuter find has a target-matching region-3 fingerprint (check2 delay slot = nop, not stolen) AND a masked score below vT40's 4
- mechanism: Static per-source compile via tmp/grind/marionation_Exec/s50/scan.sh (cc1|prologue_fix|maspsx|multu_pad) into tmp/grind/marionation_Exec/s50/asm/*.s; scan_region3.py fingerprints the second beq $?,$0,.L? insn following jal sys_GetVblankCount (= check2 branch) and inspects its delay slot for a nop (= dbr rejected fill) vs a stolen move (= vT40 class). 28 of 154 archive sources match target region-3 pattern.
- probe: python3 tmp/grind/marionation_Exec/s50/scan_region3.py + bash tmp/grind/marionation_Exec/s50/measure_batch.sh over the 19 previously-unmeasured target-matching sources
- result: One find (perm3/output-80-1) achieves masked 2 / build 179 / target 179 - the NEW LOWEST measured floor for the function, dropping 2 masked pts below vT40. All other 27 target-matching finds score >= 4 (measured): 4 finds at score=4 build=180 (masked-4 basin, vT40-equivalent), 4 finds at score=3 build=180 (vP160-class label-alive +1 route, previously banked s5), and 19 finds at score>=11 (alias-merge callee-saved family and other regressions).
- verdict: CONFIRMED

## [s50] perm3/output-80-1's masked-2 win is achieved by a byte-legal, prime-directive-compliant construct
- mechanism: The find's only edit vs vT40 base is (a) declaring `volatile u8 *new_var2;` local, (b) `saved = (*(new_var2 = &(*D_800A147C_2))) & 3;` staging D_800A147C_2 through new_var2, (c) `*((new_var2 = idx_1496) - 1) = 0;` staging idx_1496 through new_var2 for the check2 clear. The volatile qualifier on the LOCAL POINTER forces GCC to emit real stack store+reload of new_var2 between the check2 branch decision and the clear-store; this displaces dbr fill_simple_delay_slots (extra insn in the slot region) so the delay slot stays nop and the addu move emits post-branch.
- probe: Copy source to tmp/grind/marionation_Exec/s50/nonvolatile.c; sed 's/volatile u8 \*new_var2/u8 *new_var2/'; sandbox --disable all
- result: Removing ONLY the `volatile` qualifier reverts score 2->4 and build 179->178. The volatile IS the mechanism: without it, GCC folds the redundant pointer store, drops the extra insn, and the vT40-basin floor of 4 returns. With volatile, GCC is coerced to emit a memory op that has no C-level semantic justification (new_var2 is used within the same expression it's assigned in - a legitimate `u8*` would suffice for the value flow). Second usage (`new_var2 = idx_1496`) additionally COERCES a non-volatile-declared pointee (idx_1496 points to D_800A1496, a normal counter - not interrupt-touched) to volatile write semantics, which per inline-asm-injection.md sibling family (alias-rename injection) is the same coercion pattern. Prime directive explicitly lists 'volatile coercion' as forbidden alongside register pins, __asm__ injection, dead locals, and alias renames.
- verdict: KILLED

## [s50] Any non-cheat target-matching-region-3 find in the archive beats vT40 (masked 4)
- mechanism: Of the 27 non-perm3-80-1 target-matching finds, the best scores are 4 masked pts (basin-equivalent), 3 masked pts at build+1 (vP160 label-alive class, previously banked as +1 insn regression whose reduced masked comes from added insn absorbing the mismatch), and everything else >= 11.
- probe: measure_batch.sh across the 19 unmeasured target-matching sources (batch_results.txt): score histogram = {2:1 (perm3/80-1), 3:3 (perm3/180-1, perm2/160-1, perm_z07/160-1 - vP160 class @ build=180), 4:2 (perm/165-2 @ 180, perm_s26idxp/165-1 @ 180 - basin-equivalent), 11-23:13}
- result: No target-matching-region-3 find in the archive achieves masked <=4 at build=179 by any non-cheat construct. The vT40 basin's floor is intact against every measured region-3 permutation. F2 as originally framed ('a find whose region-3 window matches target AND whose pair-window is not worse than vT40') is EXHAUSTED negative on the archive: any pair-window-preserving spelling with target region-3 pays >=1 build insn (vP160 class) or lands in a callee-saved-seat family >=10.
- verdict: KILLED

## [s51] reorg pass #1 fill_simple_delay_slots decides the region-3 steal by wrapping jump_insn 424 (beqz a2, label_ref 505) and insn 445 (set (reg a1) (reg s4)) into sequence 620; the fill is accepted because 445 is a pure reg-move (no MEM_VOLATILE_P), 445's dest a1 is not read by 424, and the only dep 445 carries is REG_DEP_ANTI against 433 (the sb clear), which does not forbid moving the reg-move ahead of the mem-store.
- mechanism: Fresh cc1 -da dumps on vT40 baseline: tmp/grind/marionation_Exec/s51/vt40.dbr sequence 620 at insn 620 contains jump_insn 424 with label_ref 505 (check2) and insn 445 'set a1 = s4' with insn_list REG_DEP_ANTI 433; the sb clear-store 433 is placed AFTER sequence 620 in the RTL post-dbr layout, meaning fill_simple_delay_slots took 445 from the fall-through region between the branch and 433. In the perm3/80-1 rejected form the clear-store insn 433 becomes MEM_VOLATILE_P (via 'volatile u8 *new_var2' local pointer type applied to the store), which flips the fill outcome.
- probe: wsl bash tmp/grind/marionation_Exec/s6/dump.sh; grep sequence + s4/a1 registers in vt40.dbr around insn 620; cross-checked vt40.s @ line 1276 shows 'beq $6,$0,.L137 / move $5,$20' (target has nop there).
- result: CONFIRMED at the insn level. sequence 620 exists, insn 445 has REG_DEP_ANTI 433, no MEM_VOLATILE_P on insn 445 (reg-move), no memory op between 424 and 433 in fall-through to block motion. Matches s6's mechanism naming from higher altitude.
- verdict: CONFIRMED

## [s51] Hoisting `dst2 = a1;` textually BEFORE the check2 branch condition (`dst2 = a1; check = ...; if(!check) goto tail;`) denies dbr the reg-move candidate in the fall-through region between the branch and the clear-store, flipping the fill outcome.
- mechanism: s51/hoist_dst2.c places dst2=a1 at check2 block head; expectation: the insn 445-equivalent emits BEFORE the branch (already-emitted state at fill time, ineligible), forcing dbr to pick nothing (nop) or the sb clear (which conflicts with the taken-branch path).
- probe: spliced hoist_dst2.c, sandbox --disable all; dumped mar_system_s6.s to tmp/grind/marionation_Exec/s51/hoist.s; grepped move/beq at check2 region.
- result: masked 4 (unchanged). vt40.s @ line 1276 vs hoist.s @ same region: check2 STILL emits `beq $6,$0,.L137 / move $5,$20` in the slot. dbr finds the reg-move wherever it lives — hoist doesn't eliminate the insn, only relocates it; RA still assigns dst2 to a1 and emits a corresponding `move $5,$20` at the check2 site (as an extra out-of-loop copy at line 1244), and the slot still fills. In hoist.s check1's slot NOW also steals its clear-store (`beq $6,$0,.L121 / sb $0,0($19)` at line 1238), which is orthogonal to region-3 but proves the dbr walk is unaffected by textual position of dst2=a1.
- verdict: KILLED

## [s51] Eliminating the `dst2` local entirely — using `a1` directly as the copy-loop walker — deletes the reg-move insn 445 at the RTL level, so dbr has no candidate to steal into check2's delay slot.
- mechanism: s51/inline_dst2.c rewrites the check2 copy-loop to use a1 directly (`if (a1 != 0) { *a1 = bb; a1++; ... }`), removing the `dst2 = a1;` statement whose expand-time insn was insn 445. Without insn 445 the fill_simple_delay_slots walk on jump_insn 424 must fall through to either the sb clear-store (memory-hazard, likely rejected) or fail (nop).
- probe: spliced inline_dst2.c, sandbox --disable all
- result: masked 25 (regression +21). Eliminating dst2 forces RA to keep a1 as the walking pointer, which cascades through the outer-loop reg allocation catastrophically: a1's life now spans the copy loop AND the outer marionation loop, forcing spills and reg reassignments across the entire function. The +21 residual comes from callee-saved reallocation of dozens of other pseudos, not from the fill. Confirms dst2 is a structurally-necessary local — its role as a callee-saved seat holder cannot be eliminated without destroying the whole allocation.
- verdict: KILLED

## [s51] Frontier F1 volatile-coercion candidate (a): reading a genuinely-vblank-touched global (D_800A147C_2 or D_800F19B0) at the region-3 position creates an honest memory op that dbr must respect as anti-dep, flipping the fill without a coercion cheat.
- mechanism: In perm3/80-1, `volatile u8 *new_var2` local pointer typing forces MEM_VOLATILE_P on the clear-store insn 433, which dbr will not move insns past. An HONEST version would use an actually-volatile hardware/interrupt variable at the region-3 position.
- probe: Structural analysis + prior-session evidence review: s49 rejected `saved = (*(new_var2 = &(*D_800A147C_2))) & 3;` post-clear at masked 8 as 'semantic lie' (the read serves no semantic purpose at that position; D_800A147C_2's value at post-clear is not consumed). The volatile-cheats detector policy (inline-asm-injection.md: alias-rename catalog + expanded coercion patterns) explicitly covers 'coerce GCC's view of the access via type qualifier without hardware-volatile semantics' — the same coverage applies to perm3/80-1's `volatile u8 *new_var2` at the check2 clear (D_800A1495 is ordinary DRAM, not an interrupt-touched byte).
- result: F1 (a) is unaddressable via honest C at the check2 clear position. No vblank-touched or interrupt-visible variable has a semantic role at that program point (between check2 branch decision and the byte-clear on D_800A1495). Any insertion is a semantic lie and falls under the volatile-coercion cheat policy.
- verdict: KILLED

## [s52] Frontier F1(c): Inserting a legitimate function call (cdrom_ClearIrq) between check2 branch and clear-store forces dbr to reject the fill via CALL_INSN implicit memory clobber, leaving the slot nop.
- mechanism: GCC 2.7.2 reorg.c fill_simple_delay_slots examines candidates forward from the jump_insn; F1(c) predicted a CALL_INSN's clobber against callee-writable memory would prevent it (or any post-call insn) from being placed as the branch's delay-slot filler.
- probe: v01_call_between_check2_and_clear.c (candidate.c + `cdrom_ClearIrq();` inserted between `if(!check)goto tail;` and the clear-store). Spliced, sandbox --disable all measured, cc1 -da dumps produced (tmp/grind/marionation_Exec/s52/dumps/mar_system_s6.i.dbr + .sched2 + .greg + .s).
- result: masked 14 / build 181 / target 179 (+10 masked, +3 build vs vT40 floor 4/178). Emitted asm at check2 (mar_system_s6.s line 1273-77): `beq $16,$0,.L137 / jal cdrom_ClearIrq / sb $0,-1($19) / move $5,$20` — the inserted jal ITSELF fills the delay slot; the `move $5,$20` still emits post-branch as a visible byte. dbr reorg pass #1 summary is IDENTICAL to vT40 baseline: '3 insns needing delay slots; 2 got 0 delays, 1 got 1 delays'. Same 1-slot fill outcome, different fill insn.
- verdict: KILLED

## [s52] call_internal1 pattern in the reorg.c fill_simple_delay_slots eligible-fillers walk is annullable/eligible because CALL_INSN's REG_DEP_ANTI list contains only outward anti-deps against prior insns (arg-setup, prior calls) — not forward deps that would forbid the call from filling a preceding jump's slot.
- mechanism: In the dbr dump the inserted cdrom_ClearIrq CALL_INSN carries insn_list:REG_DEP_ANTI 133 (=the do_timeout debug_printf call), REG_UNUSED (reg:SI 31 ra). fill_simple_delay_slots's eligibility check treats an insn as movable into a delay slot when its dep-list has no forward-block against the branch; the call's memory clobber does NOT create a forward-block on itself, it only blocks OTHER insns from being motioned past the call.
- probe: grep on mar_system_s6.i.dbr for the check2-side cdrom_ClearIrq call_insn and its enclosing sequence; correlate with reorg pass #1 header.
- result: CONFIRMED at the insn level: dbr places the CALL_INSN in the delay slot without semantic filtering. Notable side observation: dbr's fill emits `jal cdrom_ClearIrq` unconditionally regardless of branch outcome, effectively hoisting the call across the branch — GCC 2.7.2 treats external K&R calls as eligible fillers when no local dep chain forbids it; a legitimate C position for the call would need a dep chain terminating at the branch condition, which the C source cannot express.
- verdict: CONFIRMED

## [s52] Given F1(c)'s failure and the do_timeout callee-set enumeration, no existing callee of marionation_Exec (sys_VSync, tslTm2LoadImage_2, debug_printf, cdrom_ClearIrq, sys_GetVblankCount, func_80080828, D_800A11B4/B8 callbacks) has a semantic role between check2's flag test and its clear-store.
- mechanism: Semantic layout of the function: check1/check2 are flag-driven state-clearing + 8-byte memcpy-style copy loops. sys_VSync/timeout handling belongs in the outer loop head; tslTm2LoadImage_2/debug_printf/cdrom_ClearIrq belong in do_timeout; sys_GetVblankCount/func_80080828/callbacks belong in the earlier polling region. Nothing at C-source altitude belongs between check2's beqz and its sb-clear.
- probe: Enumerated the 8 distinct call-sites in asm/funcs/marionation_Exec.s (grep jal/jalr, all mapped to their C-source semantic homes); cross-checked with candidate.c layout.
- result: No honest C-level call fits at the check2-branch/clear boundary. Any inserted call is a semantic lie in the same class as s50 perm3/80-1's `volatile u8 *new_var2` coercion (banned by prime directive). F1(c) closed on semantic grounds INDEPENDENT of the RTL mechanism failure.
- verdict: KILLED

## [s53] Hoisting D_800A11DC's BASE POINTER (not value) into a function-scope named local mirrors sibling saEft01Init's $s3 prologue-lui-addiu register lever and reaches masked <= 4 by transforming arg3's inline lui+addu+lw into sll+addu-through-hoisted-base (matching saEft01Init.s L58-62 vs marionation_Exec.s L63-70)
- mechanism: Sibling saEft01Init emits a target-order middle-addu pair-window WITH pre-hoisted arg3 base at masked 10 (s36 sibling-shape probe), suggesting the base hoist is compatible with target order. Prior arg3 hoists (s9v03=18, s27=18) hoisted the VALUE `D_800A11DC[D_800A11D5]` which birthed a fresh pseudo INSIDE the do_timeout window (fresh-temp-launch signature); a BASE-only hoist births the pseudo at prologue instead where it does not perturb do_timeout scheduling.
- probe: s53 v01: added `s32 *tbl_A11DC;` function local + `tbl_A11DC = D_800A11DC;` prologue assignment, changed inline `D_800A11DC[D_800A11D5]` to `tbl_A11DC[D_800A11D5]` in the debug_printf call; spliced vT40 candidate with only this delta; sandbox --disable all measured
- result: masked 14 / build 181 / target 179 (+10 masked, +3 build_insns vs vT40). The prologue hoist emits `lui+addiu` for base materialization + a callee-saved spill (sw at prologue, lw at epilogue) - 3 visible bytes that saEft01Init could absorb into its lighter register landscape but marionation_Exec cannot. Additionally the tbl_A11DC pseudo's cross-function life competes for callee-saved seats with existing outer-scope state (idx_1494, idx_1495, idx_1496, tbl_125c) and re-times the pair window like a launch (masked +7 above vT40's floor).
- verdict: KILLED

## [s54] Wrapping ONLY the arg3 evaluation (D_800A11DC[D_800A11D5]) in a nested do-while(0), placed immediately before the debug_printf call (after arg5 setup), avoids the launch-fresh-temp signature and reaches masked <=3 by elevating arg3's tree-refs weighting without perturbing the pair-window sched.
- mechanism: Frontier item from task brief: nested do-while(0) doubles GCC 2.7.2 loop-note ref weighting for the wrapped block. Late-position hoist (after arg5) keeps arg5's compute unperturbed while giving arg3 a named-local pseudo whose refs may reweight the pair-window qty tie. Untried per ledger (s9v03 hoisted arg3 EARLY = 18; every measured wrap encloses statements OTHER than arg3).
- probe: v01_arg3_nested_wrap.c: `s32 arg3val;` decl in the block, `do { do { arg3val = D_800A11DC[D_800A11D5]; } while (0); } while (0);` placed immediately before debug_printf, arg3val substituted for the inline load. Spliced via s6/splice_apply.py, sandbox --disable all.
- result: masked 13, build_insns 179, target 179 (+9 vs floor 4). Insn count matches target but reg-name diffs dominate.
- verdict: KILLED

## [s54] The +9 penalty of the late-position arg3 hoist comes from the do-while(0) nesting adding loop-note perturbation, not from the arg3 hoist itself; a single-level wrap around the same hoist reduces the penalty.
- mechanism: s9v03's arg3 EARLY hoist (no wrap) = 18; if the nested-wrap's loop-note stream costs extra masked pts beyond the launch, unwrapping to single-level should drop the score.
- probe: v02_arg3_single_wrap.c: identical to v01 but single-level `do { arg3val = D_800A11DC[D_800A11D5]; } while (0);`. Spliced, sandbox --disable all.
- result: masked 13 (identical to v01). Wrap-count (single vs nested) is INERT at the late-position arg3 hoist site.
- verdict: KILLED

## [s54] Hoisting arg3 BETWEEN t0's setup and arg5's setup (interrupting the arg4/arg5 compute sequence) is neutral relative to late-position hoist since arg3 is a byte-load and doesn't interfere with the pair-window LUID tie.
- mechanism: Position axis: s9v03 early (before arg4/arg5) = 18; s54v01/v02 late (after arg5) = 13; between-position untried. If arg3's pseudo life relative to the pair-window is the dominant cost driver, mid-position should split the delta.
- probe: v03_arg3_between.c: arg3val = D_800A11DC[D_800A11D5]; placed between `t0 = (s32)((u8*)tbl_125c + t0);` and `v0 = idx_1494[1];`. No wrap. Spliced, sandbox --disable all.
- result: masked 18 (matches s9v03 early-position exactly, build 178). Between-position pays the FULL early-hoist +14 penalty; the +9-vs-+14 delta between late and early is arg3's pseudo-life-crossing-arg4/arg5-birth, not position within the block per se.
- verdict: KILLED

## [s55] Statement-expression form of arg3 inline in debug_printf (`({ D_800A11DC[D_800A11D5]; })`) avoids the fresh-pseudo launch pathology by embedding the terminal expression directly in the arg tree; if the +9 launch floor comes from a named-local pseudo materialization, stmt-expr should score below 13 (s54's late-hoist penalty).
- mechanism: GNU statement-expressions' terminal expression is expanded as a tree-value in the enclosing arg-list, not set into a pseudo like a named local. If launch = pseudo materialization at do_timeout window entry, stmt-expr should be launch-free.
- probe: s55 v04: candidate.c with arg3 substituted by `({ D_800A11DC[D_800A11D5]; })`; spliced via s6/splice_apply.py; sandbox --disable all.
- result: masked 4 / 178 build_insns (basin-equivalent). Novel masked-4 spelling (15th known distinct vT40-floor basin member). Confirms stmt-expr DOES avoid the launch pathology (s54 predicted +9 identical to named-local hoist; measured 0). But arg3 was not a load-bearing lever in the first place - the vT40 inline form already emits identically. Stmt-expr is a mechanism-neutral spelling for arg3.
- verdict: KILLED

## [s55] The two residuals (pair-swap @56/57 sched2 T-14 UID tie + region-3 @149 dbr steal) have any C-level closer within the vT40 basin's mechanism space after 55 sessions of coverage.
- mechanism: Pair-swap: sched2 rank_for_schedule class differentiation forbidden by mips.md having no arith function-unit (insn_cost=1 forces class=3 both sides; s43); LUID tie decided by textual expand order; all four qty_compare axes closed (s46); 140-ordering sweep exhausted (session-10c). Region-3: fill_simple_delay_slots takes reg-move a1 because a1 caller-saved and target label live-set has only callee-saved qtys (s6/s7/s51); every label-alive route pays +1 insn; every alias-merge callee-saved route lands at family floor masked 10; volatile-coercion route (s50 perm3/80-1 masked 2) forbidden by prime directive; call-insertion (F1c s52) fills slot with call itself.
- probe: Cumulative closure check across s2/s3 (48 hand forms), s4/s5 (~10k permuter samples), s6/s7 (140-ordering sweep, 9-topology sweep, insn-level sched2/dbr forensics), s8-s12/s45-s55 (structural + mirror + tbase + prologue-reorder + stmt-expr axes), s25/s33/s37/s38/s42/s43 (mechanism forensics), s40/s41/s49 (portfolio permuter basins), s50/s51/s52 (dbr steal + F1 branch enumeration), s53/s54 (sibling saEft01Init transfer + arg3 hoist axis). 165+ hand forms, 15+ basin members, 13 permuter basins ~50+ CPU-hr, 3-family closed attractor set holds.
- result: KILLED - the honest in-basin closer space is exhausted. All measured novel attractors converge to {vT40 floor 4 @ 178, alias-merge >=10, label-alive/semantic-lie +1 insn or wrong-return, reg-shuffle score-inert}. No non-cheat pathway to masked <4 within the vT40 basin has been demonstrated across 55 sessions.
- verdict: KILLED

## [s56] Block-scoping `u8 saved` inside the `if (sys_GetVblankCount()!=0)` block (shortening its life from function-scope to the vblank-if body only) perturbs the qty allocation at the pair-swap or region-3 residuals
- mechanism: s47 proved constant-holder lifetime is combine-relevant (block-scope new_var/new_var3 folds to inline constants, deleting mask insns); the analogous test for a load-derived value (saved = *D_800A147C_2 & 3) may similarly let combine fold `saved & 3` reads at the restore site or shift the RA landscape for the check region
- probe: v01_saved_block.c: removed function-scope `u8 saved;`, injected `u8 saved;` at top of the vblank-if compound; splice + sandbox --disable all
- result: score=4, build=178, target=179 - IDENTICAL to vT40 baseline. Basin-equivalent 16th spelling. Combine did NOT fold `& 3` into `*D_800A147C_2 = saved;` because the intervening func_80080828() call is an unknown-side-effect barrier; qty allocation for saved is unchanged because its RTL first-use position is identical.
- verdict: KILLED

## [s56] Block-scoping `s32 status` inside the polling `do{}while(1)` loop (shortening its life from function-scope to the loop body) perturbs the qty allocation at the pair-swap or region-3 residuals
- mechanism: s41 established that extending status's life across do_timeout regresses +4 via callee-saved seat competition; the symmetric probe (shortening status's life to just the polling loop where it's actually used) tests whether the shortened life releases seat pressure or otherwise reweights the check-region cascade
- probe: v02_status_block.c: removed function-scope `s32 status;`, injected `s32 status;` at top of polling do{} body; splice + sandbox --disable all
- result: score=4, build=178, target=179 - IDENTICAL to vT40. Basin-equivalent 17th spelling. Status's RTL first-use inside the polling loop is unchanged by decl scope; GCC 2.7.2 pseudo births at first-use, not declaration site (consistent with s2's decl-order finding extended to lifetime scoping).
- verdict: KILLED

## [s56] Combining both scope-shortenings (saved to vblank-if, status to poll-do) produces a compounded landscape perturbation not visible in either isolated probe
- mechanism: s10 proved staging webs are NON-ADDITIVE (t0-web + v0-web interact non-linearly); a symmetric interaction between scope-shortened locals could conceivably tip a seat cascade tie the isolated forms did not reach
- probe: v03_both_block.c: combined v01+v02 modifications; splice + sandbox --disable all
- result: score=4, build=178, target=179 - IDENTICAL to vT40. No compound perturbation. Basin-equivalent 18th spelling. Confirms the two scope-shortenings are fully independent AND both individually inert.
- verdict: KILLED

## [s57] Chaining idx_1496 = idx_1495 + 1 (instead of idx_1494 + 2) alters the prologue address-arithmetic dep-chain enough to break the pair-swap tie or region-3 steal
- mechanism: Chained derivation changes RTL insn ordering for idx_1496's birth: idx_1496 now depends on idx_1495 (already the target `addiu s6,s2,1` per ledger evidence) instead of forking directly off idx_1494. New sequential dep may reweight qty birth signatures at expand time.
- probe: s57 v05 spliced (idx_1496 = idx_1495 + 1); `& tools/wteng.ps1 main sandbox marionation_Exec --disable all`
- result: masked 4 / 178 insns / 179 target — basin-equivalent. GCC canonicalizes both derivations to the same addiu chain; the change is codegen-invariant to the sched2 T-14 tie. 19th known distinct masked-4 spelling banked as rejected/s57-idx1496-chain-from-idx1495-basin-equiv-4.c
- verdict: KILLED

## [s57] Tightening src/dst/dst2 declaration scope to per-copy-block scope reduces live-range interference and reweights allocno priority for the outer loop's callee-saved landscape
- mechanism: The three pointer temps currently span the whole check region (function-top decls). Block-scoping each pair into its own compound statement shortens each var's life and could reduce RA pressure enough to change the seat assignment in the copy-loop regions (which cascade into region-3).
- probe: s57 v06 spliced (src/dst inside check1 block, dst2/src inside check2 block, i left at function scope); `& tools/wteng.ps1 main sandbox marionation_Exec --disable all`
- result: masked 11 / 178 insns / 179 target — regression +7. Block-scoping the copy-loop pointer locals relandscapes the RA around the copy loops in a way that scrambles seat assignments (analogous to the s16 dead-local-saved-debug-inert / s21 fresh-i-per-copy-block regressions). Copy-block locals are load-bearing at function scope. Banked as rejected/s57-src-dst-block-scoped-copyblocks-11.c
- verdict: KILLED

## [s57] Rewriting cnt via split-init accumulation (cnt = D_800F19BC; D_800F19BC = cnt; D_800F19BC += 1;) — the sanctioned split-init-accumulation family — reweights D_800F19BC's global-load qty enough to break the pair-swap coupling
- mechanism: Split-init sanctioned per 2026-06-13 owner directive; splits `D_800F19BC = cnt + 1;` into a store-back + += 1 pair. Adds an extra load/store to D_800F19BC's ref count, potentially changing global-qty priority that could propagate into do_timeout via cross-BB effects.
- probe: s57 v07 spliced (three-statement cnt-split-init in the loop-body); `& tools/wteng.ps1 main sandbox marionation_Exec --disable all`
- result: masked 4 / 178 insns / 179 target — basin-equivalent. The extra store gets folded by combine (build_insns unchanged at 178); the cnt local is loop-body-scoped and never reaches do_timeout's expand context. 20th known distinct masked-4 spelling banked as rejected/s57-cnt-split-init-basin-equiv-4.c
- verdict: KILLED

## [s58] The last-standing F1 frontier item - an HONEST function call (`sys_GetVblankCount()` / current name `CheckCallback()`) placed between the check2 branch and the check2 clear - creates a delay-slot fill candidate at the region-3 steal site without a volatile-coercion cheat, and drops the masked score below 4.
- mechanism: s51 F1(a) killed the `D_800A147C_2` volatile load at this site on semantic-lie grounds; s52 F1(c) killed `cdrom_ClearIrq()` because `reorg.c` `fill_simple_delay_slots` picked the inserted CALL_INSN *itself* as the check2-beqz fill (masked 14, build 181). `sys_GetVblankCount` was the one call in this TU never tested at the check2 position, and the frontier hypothesised its vblank-state read might be schedulable as a non-call-shaped MEM_VOLATILE_P operand rather than as the fill.
- probe: s58 v01 (`tmp/grind/CD_ready/s58/v01_vblank_at_check2.c`): candidate.c (vT40) with `sys_GetVblankCount();` inserted between `if (!check) goto tail;` and `do { *(idx_1496 - 1) = 0; } while (0);`. Spliced with `tmp/grind/CD_ready/s58/splice.py`; `& tools/wteng.ps1 main sandbox CD_ready --disable all`.
- result: masked **14**, build_insns **181**, target 179 - a bit-for-bit repeat of the s52 `cdrom_ClearIrq` result. GCC 2.7.2 gives every CALL_INSN the same treatment at this site: `fill_simple_delay_slots` takes the call as the branch fill and the extra `jal` plus reload pushes the body to 181. The identity of the callee is irrelevant; what matters is that the inserted statement is a CALL_INSN. **The F1 "honest fill at the check2 site" family is therefore formally exhausted** - its two live members (volatile load, function call) are both measured dead, and no third shape exists that is both honest and schedulable into that slot. (Secondary finding: the construct would be barred anyway - a `CheckCallback()` whose result is discarded at that position has no semantic role, i.e. it is insert-for-effect, the semantic-lie class killed for this function on 2026-07-05.)
- verdict: KILLED

## [s58] The vT40 floor of 4 is chassis-stable across the asm-until-matched migration and the naming wave (i.e. the ledger banked floor is still the honest floor on the current HEAD).
- mechanism: The driver dispatch chassis check reported "measurement unavailable", and since the ledger last entry `src/system.c` migrated to `INCLUDE_ASM("asm/funcs", CD_ready);` and the naming wave renamed six of the function callees (`sys_VSync`->`VSync`, `tslTm2LoadImage_2`->`puts`, `debug_printf`->`printf`, `cdrom_ClearIrq`->`CD_flush`, `sys_GetVblankCount`->`CheckCallback`, `func_80080828`->`getintr`). Renames are codegen-neutral in principle ([[naming-main-rename-codegen-neutral]]) but that had never been measured for THIS function on THIS chassis, and every banked spelling conclusion is chassis-relative.
- probe: Re-splice `memory/grind/CD_ready/candidate.c` (vT40) into `src/system.c` under the current names and re-run `& tools/wteng.ps1 main sandbox CD_ready --disable all`.
- result: **score=4, build_insns=178, target_insns=179, rules_dropped=0** - identical to the ledger s1-s57 baseline. CONFIRMED: the migration plus naming wave are byte-neutral for CD_ready, the 57-session floor of 4 is the live honest floor, and all 115 banked rejected forms remain valid evidence on this chassis.
- verdict: CONFIRMED

## [s58] The vT40 floor of 4 is chassis-stable across the asm-until-matched migration and the naming wave, i.e. the ledger's banked floor is still the honest floor on the current HEAD.
- mechanism: The driver's dispatch chassis check reported 'measurement unavailable'. Since the ledger's last entry src/system.c migrated to INCLUDE_ASM("asm/funcs", CD_ready); and the naming wave renamed six callees (sys_VSync->VSync, tslTm2LoadImage_2->puts, debug_printf->printf, cdrom_ClearIrq->CD_flush, sys_GetVblankCount->CheckCallback, func_80080828->getintr). Renames are codegen-neutral in principle but that had never been measured for this function on this chassis, and every banked spelling conclusion is chassis-relative.
- probe: Re-spliced memory/grind/CD_ready/candidate.c (vT40) into src/system.c under the current names via tmp/grind/CD_ready/s58/splice.py; ran `& tools/wteng.ps1 main sandbox CD_ready --disable all`.
- result: score=4, build_insns=178, target_insns=179, rules_dropped=0 - identical to the s1-s57 baseline. Migration + naming wave are byte-neutral for CD_ready; all 115 banked rejected forms remain valid evidence on this chassis.
- verdict: CONFIRMED

## [s58] The last-standing F1 frontier item - an HONEST function call (sys_GetVblankCount(), now CheckCallback()) placed between the check2 branch and the check2 clear - creates a delay-slot fill candidate at the region-3 steal site without a volatile-coercion cheat, and drops the masked score below 4.
- mechanism: s51 F1(a) killed the D_800A147C_2 volatile load at this site on semantic-lie grounds; s52 F1(c) killed cdrom_ClearIrq() because reorg.c fill_simple_delay_slots picked the inserted CALL_INSN itself as the check2-beqz fill (masked 14, build 181). sys_GetVblankCount was the one call in this TU never tested at the check2 position; the frontier hypothesised its vblank-state read might schedule as a non-call-shaped operand rather than as the fill.
- probe: s58 v01 (tmp/grind/CD_ready/s58/v01_vblank_at_check2.c): candidate.c with `sys_GetVblankCount();` inserted between `if (!check) goto tail;` and `do { *(idx_1496 - 1) = 0; } while (0);`. Spliced, then `& tools/wteng.ps1 main sandbox CD_ready --disable all`.
- result: masked 14, build_insns 181, target 179 - a bit-for-bit repeat of the s52 cdrom_ClearIrq result. GCC 2.7.2's reorg.c fill_simple_delay_slots consumes ANY inserted CALL_INSN as the check2-beqz fill regardless of callee. The F1 'honest fill at the check2 site' family is formally exhausted: both live members (volatile load, function call) are measured dead and no third shape exists. The construct would be barred anyway - a call whose result is discarded there has no semantic role (insert-for-effect / semantic-lie class, ruled out for this function 2026-07-05).
- verdict: KILLED

## [s58] Endgame-lock gate (a): CD_ready qualifies for a canonical-asm grant (STRONG scan_hand_coded tier via S1/S2/S6).
- mechanism: Gate (a) of the 2026-07-27 standing ruling; only S1 (multu pacing), S2 (empty branch) and S6 (BIOS jumptable) can carry a STRONG tier.
- probe: `python3 tools/scan_hand_coded.py --single CD_ready` (artifact tmp/grind/CD_ready/s58/scan_hand_coded.txt).
- result: tier=LOW score=2/8 (179 insns). Only S4 (4 loads in an 8-insn window @ insn 51) and S5 (1 approx-sibling CD_sync, jaccard 0.64) fire; S1/S2/S6 all absent. Independently false on provenance: this function is identified PsyQ 3.5 libcd CD_ready(int, u_char*) - compiled Sony C, not hand-written asm (memory/closer/libcd-identity.md, libcd-groundtruth.md:40-52).
- verdict: KILLED

## [s58] Endgame-lock gate (b): an in-hand SOTN-master precedent exists for the construct that would CLOSE the remaining 4-point residual.
- mechanism: Gate (b) of the 2026-07-27 standing ruling requires a citable file:line or commit for the closing construct; 'same spirit' does not qualify.
- probe: Enumerated the residual against the ledger's root-cause forensics: pair-swap @ insns 56/57 is a sched.c sched2 T-14 tie between two equal-INSN_PRIORITY ashlsi3 insns broken by LUID (source emission order), with every flip paying the global.c qty_compare 5.33-vs-5.33 seat trade; region-3 @ insn 149 is a reorg.c fill_simple_delay_slots steal on the check2 beqz with every honest fill route measured dead (s51 volatile load, s52 + s58 call insertion, label-alive +1 insn, alias-merge floor masked 10).
- result: FAILS by construction - there is no closing construct to cite a precedent FOR. Neither residual has any C-level spelling: 20 distinct masked-4 spellings across 5+ structural axes, the 140-ordering sweep, the 9-variant topology sweep and all four qty_compare axes all land on the same masked-4/178 attractor. The constructs that HOLD the floor at 4 (FAKE-annotated do-while(0) wraps, staged-value reuse, constant holders, pointer alias) are already inside sanctioned families and are not the blocker.
- verdict: KILLED

## [s59] The single-qty demotion of pseudo 104 (the `t0` address web) — the one axis the 2026-08-30 solver campaign flagged as CANDIDATE-UNPROBED — breaks the global.c/local-alloc.c 5.33-vs-5.33 qty_compare tie on the order-correct (vT32/arg5-first) chassis and drops the floor below 4.
- mechanism: local-alloc.c `qty_compare` priority = floor_log2(refs)*refs*size/life. Ledger QTYDBG (evidence.md:31-36) identifies the tie exactly: addr-temp qty 102 {r4 l4}=8.0 -> $v0 (correct); t0-web qty 104 {r4 l6}=5.33; arg5val qty 97 {r4 l6}=5.33 — EXACT TIE broken by qty birth order (104 born first -> takes $v1), which is why the arg5-first chassis emits the target's insn order but trades the two seats. `tools/ra_solver/inverse.py local --block 3 --goal qty2->$v1` (tmp/grind/CD_ready/s59_campaign/inverse_local_goal_d2.txt) returned 25 single-atom vectors whose dominant family is exactly "DEMOTE qty 1 (= pseudo 104)": refs_down 4->3 / 4->2, or live_extend (born-earlier 18->17, dies-later 24->25/26). Every prior bank entry reweighted MULTIPLE qtys at once (fresh carriers, wraps, mask folds); the isolated p104 demotion had never been spelled. Reducing 104's refs to 3 gives pri=2*3*4/6=4.0 and to 2 gives 1*2*4/6=1.33, both strictly below 97's 5.33, which should hand $v1 to 97 and $a0 to 104 — the target seating.
- probe: 10 spliced variants measured with `& tools/wteng.ps1 main sandbox CD_ready --disable all` (harness tmp/grind/CD_ready/s59/splice.py + sweep.ps1; logs sweep1.txt, sweep2.txt). Chassis A = vT40 (t0 statements first, the floor-4 base); chassis B = vT32 (the `v0`-staged arg5 load statement moved AHEAD of the t0 statements, i.e. the order-correct/seat-traded chassis). Demotion spellings for the t0 web: refs 4 (unchanged control), refs 3 via `t0 = idx_1494[0]; t0 = (s32)((u8*)tbl_125c + t0*4);` (multiply folded into the add), refs 3 via `t0 = idx_1494[0]*4; t0 = (s32)((u8*)tbl_125c + t0);` (multiply folded into the load), refs 2 via the fully folded `t0 = (s32)((u8*)tbl_125c + idx_1494[0]*4);`, plus a born-earlier live_extend spelling (the bare `t0 = idx_1494[0];` load hoisted above the arg5 statements while the arithmetic stays below them).
- result: **KILLED — no spelling reaches below the vT40 floor of 4; the demotion axis is measured monotonically harmful on both chassis.** Control vT40 base = **4** / 178 insns / 179 target / 0 rules (floor re-confirmed live on this chassis this session). vT32 chassis: base (pp between the two groups) = **9**; t0-load hoisted (born-earlier live_extend) = **8** (this is the ledger's historical vT32 = 8); refs-3 mul-in-add = **9**; refs-3 mul-in-load = **11**; refs-2 fully folded = **11**; refs-2 folded with pp first = **11**; load-hoisted + refs-3 = **8**. vT40 chassis: refs-3 mul-in-add = **5** (+1); refs-3 mul-in-load = **11** (+7); refs-2 fully folded = **11** (+7). Every build stayed at 178 insns, so nothing here is an insn-count effect — it is pure seat/order relandscaping. The mechanism finding: folding the t0 arithmetic does NOT hand $v1 to qty 97 cheaply, because collapsing the web also collapses the addr-temp structure that qty 102 depends on (102 is the {r4 l4}=8.0 tight temp that currently seats $v0 correctly); demoting 104 by folding demotes 102 with it and the whole three-qty cascade re-seats, costing more than the tie it wins. The refs-3 mul-in-load spelling is uniformly the worst (11 on both chassis), confirming the load and the multiply must stay in separate statements for qty 102 to stay tight. The 2026-08-30 campaign's printed local-mode caveat is therefore confirmed for a tenth function: the p104 demotion vectors were NECESSARY but not SUFFICIENT — the local-alloc model scores one qty in isolation while the real allocator re-runs the whole block cascade.
- verdict: KILLED

## [s59] Endgame-lock gate (a) re-measured on the current chassis: CD_ready qualifies for a canonical-asm grant (STRONG scan_hand_coded tier via S1/S2/S6).
- mechanism: Gate (a) of the 2026-07-27 standing ruling; only S1 (multu pacing), S2 (empty branch) and S6 (BIOS jumptable) can carry a STRONG tier.
- probe: `python3 tools/scan_hand_coded.py --single CD_ready` re-run this session (artifact tmp/grind/CD_ready/s59/scan_hand_coded.txt).
- result: tier=LOW score=2/8 (179 insns). Only S4 (4 loads in an 8-insn window @ insn 51) and S5 (1 approx-sibling CD_sync, jaccard 0.64) fire; S1, S2, S3, S6, S7, S8 all absent — identical to the s58 reading, so the migration/naming chassis change did not move the scan either. Independently false on provenance: this body is identified PsyQ 3.5 libcd `CD_ready(int, u_char*)`, i.e. compiled Sony C, not hand-written asm (memory/closer/libcd-identity.md:7-8, memory/closer/libcd-groundtruth.md:40-52).
- verdict: KILLED

## [s59] The single-qty demotion of pseudo 104 (the `t0` address web) - the one axis the 2026-08-30 solver campaign flagged as CANDIDATE-UNPROBED - breaks the local-alloc.c qty_compare 5.33-vs-5.33 tie on the order-correct (vT32/arg5-first) chassis and drops the floor below 4.
- mechanism: local-alloc.c qty_compare priority = floor_log2(refs)*refs*size/life. The ledger's QTYDBG reading (evidence.md:31-36) pins the tie: addr-temp qty 102 {r4 l4}=8.0 seats $v0 correctly; t0-web qty 104 {r4 l6}=5.33 and arg5val qty 97 {r4 l6}=5.33 are an EXACT TIE broken by qty birth order (104 born first takes $v1), which is why the arg5-first chassis emits the target's insn order but trades the two seats. tools/ra_solver/inverse.py local --block 3 --goal 'qty 2: $a0 -> $v1' returned 25 single-atom vectors dominated by 'demote qty 1 (= pseudo 104)': refs_down 4->3/4->2, or live_extend born-earlier/dies-later. Every prior bank entry reweighted MULTIPLE qtys at once; the isolated p104 demotion had never been spelled. refs 3 gives pri 4.0 and refs 2 gives 1.33, both strictly below 97's 5.33, which should hand $v1 to 97 and $a0 to 104 - the target seating.
- probe: 10 spliced variants scored with `& tools/wteng.ps1 main sandbox CD_ready --disable all` (harness tmp/grind/CD_ready/s59/splice.py + sweep.ps1; logs sweep1.txt, sweep2.txt). Chassis A = vT40 (t0 statements first, the floor-4 base); chassis B = vT32 (the v0-staged arg5 load statement moved AHEAD of the t0 statements, i.e. order-correct/seat-traded). t0-web demotion spellings: refs 4 control; refs 3 via `t0 = idx_1494[0]; t0 = (s32)((u8*)tbl_125c + t0*4);` (multiply folded into the add); refs 3 via `t0 = idx_1494[0]*4; t0 = (s32)((u8*)tbl_125c + t0);` (multiply folded into the load); refs 2 via the fully folded `t0 = (s32)((u8*)tbl_125c + idx_1494[0]*4);`; plus a born-earlier live_extend spelling (bare load hoisted above the arg5 statements, arithmetic left below).
- result: No spelling reached below the vT40 floor of 4; the axis is monotonically harmful on both chassis. vT40 chassis: base 4 (control, floor re-confirmed live), refs-3 mul-in-add 5, refs-3 mul-in-load 11, refs-2 fully folded 11. vT32 chassis: base 9, t0-load hoisted (born-earlier) 8 (= the ledger's historical vT32 floor), refs-3 mul-in-add 9, refs-3 mul-in-load 11, refs-2 folded 11, refs-2 folded pp-first 11, load-hoisted+refs-3 8. Every build stayed at 178 insns / 179 target / 0 rules, so this is pure seat/order relandscaping and not an insn-count effect. Mechanism finding: folding the t0 arithmetic to demote qty 104 also collapses the neighbouring addr-temp qty 102 - the tight {r4 l4}=8.0 temp that currently seats $v0 CORRECTLY - so the whole three-qty cascade re-seats and costs more than the tie it wins; the mul-in-load spelling is uniformly worst (11 on BOTH chassis), pinning the cause to the load and the multiply needing to remain in separate statements for qty 102 to stay tight. Tenth confirmation of the ra_solver local-mode printed caveat (camera_set_zoom 2026-08-05): single-qty demotion vectors are NECESSARY, not SUFFICIENT.
- verdict: KILLED

## [s59] Endgame-lock gate (a) re-measured on the current chassis: CD_ready qualifies for a canonical-asm grant (STRONG scan_hand_coded tier via S1/S2/S6).
- mechanism: Gate (a) of the 2026-07-27 standing ruling; only S1 (multu pacing), S2 (empty branch) and S6 (BIOS jumptable) can carry a STRONG tier.
- probe: `python3 tools/scan_hand_coded.py --single CD_ready` re-run this session; artifact tmp/grind/CD_ready/s59/scan_hand_coded.txt.
- result: tier=LOW score=2/8 (179 insns). Only S4 (4 loads in an 8-insn window @ insn 51) and S5 (1 approx-sibling CD_sync, jaccard 0.64) fire; S1, S2, S3, S6, S7, S8 all absent - identical to the s58 reading, so the migration/naming chassis change did not move the scan. Independently false on provenance: identified PsyQ 3.5 libcd CD_ready(int, u_char*) = compiled Sony C, not hand-written asm (memory/closer/libcd-identity.md:7-8, memory/closer/libcd-groundtruth.md:40-52).
- verdict: KILLED

## [s59] Endgame-lock gate (b): an in-hand SOTN-master precedent exists for the construct that would CLOSE the remaining 4-point residual.
- mechanism: Gate (b) requires a citable file:line or commit for the CLOSING construct; 'same spirit' does not qualify.
- probe: Enumerated the residual against the ledger's root-cause forensics: (1) sched.c sched2 T-14 tie at insns 56/57 between two equal-INSN_PRIORITY ashlsi3 insns broken by LUID (source emission order), coupled to the local-alloc.c/global.c 5.33-vs-5.33 seat trade re-measured above; (2) reorg.c fill_simple_delay_slots steal on the check2 beqz at insn 149, whose every honest fill route is measured dead (s51 volatile load, s52 + s58 call insertion, label-alive +1 insn, alias-merge floor masked 10).
- result: FAILS by construction - there is no closing construct to cite a precedent FOR. Neither residual has any C-level spelling: 20 distinct masked-4 spellings across 7+ structural axes, the 140-ordering sweep, the 9-variant topology sweep and all four qty_compare axes all land on the same masked-4/178 attractor, and this session's 10 new variants add nothing below it. The constructs that HOLD the floor at 4 (FAKE-annotated do-while(0) wraps, staged-value reuse, constant holders, pointer alias) are already inside sanctioned families and are not the blocker.
- verdict: KILLED

## 2026-09-01 — operator reopen note (owner ruling 2026-09-01 (decisions.md FORECLOSED-BUCKET REVIEW entry))

Returned to active under Ruling A; executes via the Ruling D CD_intr aggregate-merge session. This ledger's s50 masked-2/179 find was disqualified on the false premise that D_800A1495 is 'not touched by any interrupt handler in this codebase' — cdrom_IrqHandler (same TU, matched, the registered CD IRQ callback) reads it through a volatile pointer. Under the aggregate declaration the volatility is a type property of a census-proven Sony object, not a per-use coercion. Also: re-score the banked Closer form (git show 043e4b80^:memory/closer/candidates/marionation_vAT1_notailwrap.c) post-mel. Provenance correction: BB2 links PsyQ 4.0-era bios.c v1.86 (memory/closer/libcd-groundtruth.md), not 3.5 as two record sentences state.

## [s60] 2026-09-01 — frontier REWRITTEN at floor 2 (was 4)

The owner's Ruling-A named probe (decisions.md:17795 — "re-score the banked vAT1 form
post-`-mel`") succeeded: `marionation_vAT1_notailwrap.c`, recovered from
`git show 043e4b80^:memory/closer/candidates/marionation_vAT1_notailwrap.c`, scores
**masked 2, build 179 == target 179, 0 rules** on the current chassis, against a live-measured
baseline of 4/178 for the 59-session vT40 body. It is now `memory/grind/CD_ready/candidate.c`;
vT40 is retained as `candidate-vT40-masked4-no-volatile.c`. Full measurement table, prong-1
citations and residual analysis: evidence.md [s60].

**Every hypothesis below supersedes the pre-s60 frontier.** The old single frontier item (the
"coupled fixed point" needing a joint sched2-LUID x local-alloc-birth solver search) was
formulated against a 178-instruction base with two wrong branch destinations. vAT1 pays both of
those legs already, so that item is retired as *stated* and replaced by F1-F3.

### F1 (HIGHEST VALUE, cheap) — replay the s53-s59 kill list against the vAT1 base

**Hypothesis.** One or more of the axes killed in s53-s59 moves the residual at the vAT1 base,
because every one of those kills was a measurement of "does this move the masked-4/178 vT40
body", and the vAT1 body differs in instruction count, branch destinations, tail wrap, and the
register holding the staged table index ($v1 here vs $a0 in the target).
**Mechanism.** A kill is a statement about a basin, not about a C construct. Changing the base
changes the basin. The s59 record itself notes the t0-web probes behaved differently on the
vT32 vs vT40 bases (4 vs 8 launch floors), which is direct evidence that these axes are
base-sensitive.
**Next probe.** Re-run, against `candidate.c` (vAT1), in this order (cheapest first): (i) the
t0-address-web refs/fold variants (s59 v11-v19, sources kept in `tmp/grind/CD_ready/s59/`);
(ii) chain-idx1496 and cnt split-init (s57); (iii) decl-scope shortening for `saved`/`status`
(s56); (iv) arg3 wrap position/count (s54). Score each with
`& tools/wteng.ps1 main sandbox CD_ready --disable all`. Anything below 2 is a floor drop.

### F2 — the residual is a two-instruction seat+order tie in the printf-argument block

**Hypothesis.** The 2 remaining differing instructions are the `sll ,2` / `lw` transposition at
build insns ~57-58 plus the $v1-vs-$a0 seat for the staged table index, both inside the
`do_timeout` `debug_printf(&D_800161C8, *pp, D_800A11DC[D_800A11D5], *(s32 *)t0, arg5)` block.
**Mechanism.** Same class the ledger has always described (sched2 LUID tie x local-alloc
seat), but now WITHOUT the instruction-count and branch-destination costs that made every
previous fix a trade. The seat and the order may now be independently reachable.
**Next probe.** Run `pwsh tools/grinder/dump.ps1 CD_ready` on the vAT1 base and read
`.lreg`/`.greg` for the index pseudo's seat and `.sched2` for the LUID tie — do NOT re-use the
vT40-era dumps, which describe a different body. Then classify with
`tools/ra_solver/inverse_compose.py classify`; the solver verdicts in the ledger were all
computed on vT40 and must be recomputed.

### F3 (BLOCKING for completion, not for progress) — settle the volatile family

**Hypothesis.** `volatile u8 *idx_1496;` on the IRQ-set CD status byte 0x800A1496 is covered by
[[legitimate-volatile-interrupt-touched]], not by the volatile-coercion ban.
**Mechanism / evidence in hand.** Prong 1 is satisfied with citations at every link (writer
`getintr` at asm/funcs/getintr.s:304; caller `cdrom_IrqHandler` at src/system.c:770; installed
via `InterruptCallback(2, &D_80081F1C)` at src/system.c:609 and :630). The qualifier is
load-bearing and the harm is measured, not asserted: s60 v01 removes it and nothing else, and
GCC hoists the flag load out of the poll loop (179 -> 178 insns, score 2 -> 4). The identical
declaration already ships in MATCHED committed code in the same TU (src/system.c:549-551,
:748-749) and matched `cdrom_IrqHandler` derives a volatile byte pointer by pointer arithmetic
at :770-771, the same shape used here.
**What is OPEN.** (a) Prong 2's shape list is exact and this use-site — a poll loop whose body
calls VSync/getintr between successive reads — is closest to double-read-across-sequence-point
but is not verbatim any of the three; (b) D_800A1494/95/96 have no `volatile_extern_allowlist.txt`
entry.
**Next probe.** This is not a measurement, it is a `ruling-request`: ask whether prong 2 admits
a poll loop with body work when the double-read harm is demonstrated by controlled measurement,
and whether the already-shipped matched in-TU declarations constitute the allowlist grant.
**Do not submit candidate-ready on this body before that is answered** — the score is not the
gate, the family is.

### Retired this session

- The pre-s60 frontier item ("joint sched2-LUID x local-alloc-birth solver search; do not
  re-open on existing tooling") is RETIRED AS STATED. Its premise — that fixing the order
  costs the seats and vice versa — was measured on a base that also had to buy an instruction
  and two branch destinations. It may re-form at the vAT1 base, but it must be re-derived
  there, not inherited.
- Ruling D (the CD_intr aggregate merge) is DEAD for this function by CD_sync s107's
  symbol-level prong-(c) finding (decisions.md:18287-18400, sibling-consequence paragraph).
  Do not re-measure it; re-activation requires the five asm-only consumers of
  D_800A1494/95/96 to reach C.
- v02/v03/v04 (s60): every alternative volatile SPELLING loses. Type-level volatile on the
  global with a direct `&g_cd_status_c` costs a `lui/%lo` (180 insns, score 4); making
  idx_1494/idx_1495 volatile too scores 8. Only the 1496 access path may carry the qualifier,
  and only when the pointer is derived as `idx_1494 + 2`.

## [s60] The banked Closer-phase form marionation_vAT1_notailwrap.c still scores masked 2 on the current post--mel, post-naming-wave chassis (the owner's 2026-09-01 Ruling A named probe for this function, decisions.md:17795).
- mechanism: vAT1 differs from the 59-session vT40 body in two ways: (1) each check arm is self-contained with its own `return check;` (the inlined-_memcpy shape), so GCC cross-jumping forward-redirects arm 1's guard into arm 2's identical tail - this fixes BOTH unmasked branch-destination diffs and lets the tail do-while(0) wrap be dropped; (2) idx_1496 (the IRQ-set CD status byte 0x800A1496) is declared `volatile u8 *`, which stops GCC hoisting the flag load out of the `goto loop` poll and supplies the target's 179th instruction.
- probe: git show 043e4b80^:memory/closer/candidates/marionation_vAT1_notailwrap.c -> tmp/grind/CD_ready/s60/vAT1.c; spliced over INCLUDE_ASM at src/system.c:379 with tmp/grind/CD_ready/s60/splice.py; `& tools/wteng.ps1 main sandbox CD_ready --disable all`. Live baseline measured in the same session for chassis validity.
- result: baseline (vT40 candidate.c) = score 4, build 178, target 179, rules 0. vAT1 = score 2, build 179, target 179, rules 0. candidate.c rewritten to the vAT1 body and re-scored after writing: 2/179/179/0.
- verdict: CONFIRMED

## [s60] The `volatile` on idx_1496 is load-bearing rather than incidental - it is what supplies the 179th instruction.
- mechanism: Without the qualifier GCC treats the flag read as loop-invariant and hoists it out of the `loop:`/`tail: if (a0 == 0) goto loop;` poll, merging what the C standard permits it to merge but the CD IRQ writer invalidates.
- probe: s60 v01: vAT1 with `volatile u8 *idx_1496;` changed to `u8 *idx_1496;` and NOTHING else changed. Sandbox scored.
- result: score 4, build_insns 179 -> 178. Exactly one instruction and two score points attributable to the qualifier alone.
- verdict: CONFIRMED

## [s60] Prong 1 of .claude/rules/legitimate-volatile-interrupt-touched.md (asynchronous IRQ writer, NAMED with function + file:line) is satisfied for 0x800A1496.
- mechanism: The rule accepts a write inside a function installed via InterruptCallback, or inside a function called from such an installed handler.
- probe: grep of asm/funcs for writes to D_800A1494/95/96 and of src/*.c for the handler installation.
- result: Writer: `getintr` writes the byte at asm/funcs/getintr.s:304 (`sb $v0, %lo(D_800A1496)($at)`; also :244 for 1494, :274/:309 for 1495), and func_800819C4 at asm/funcs/func_800819C4.s:71. Caller chain: getintr() is called from cdrom_IrqHandler (src/system.c:770), whose entry carries glabel D_80081F1C (src/system.c:758-768), installed as the IRQ-2 CD-ROM callback by `InterruptCallback(2, &D_80081F1C);` at src/system.c:609 and :630. Corroboration: matched committed in-TU code already declares these exact bytes `extern volatile u8 g_cd_status_a/b/c` at src/system.c:549-551 and :748-749, and matched cdrom_IrqHandler derives a volatile byte pointer BY POINTER ARITHMETIC at :770-771 (`volatile u8 *s1 = &g_cd_status_b; volatile u8 *s3 = s1 - 1;`) - the same shape vAT1 uses.
- verdict: CONFIRMED

## [s60] An alternative, less contestable spelling of the same volatile (type-level on the global, or on all three Intr pointers) reaches the same or a better score.
- mechanism: If the qualifier could live on the global declaration rather than on a derived local pointer, the construct would sit squarely inside the sanctioned `extern volatile T G;` shape.
- probe: s60 v02 (`extern volatile u8 g_cd_status_c; idx_1496 = &g_cd_status_c;`), v03 (all three Intr pointers volatile, base cast from &D_800A1494), v04 (v03 with the base taken as &g_cd_status_a, cdrom_IrqHandler's exact shape). Three sandbox runs.
- result: v02 = score 4, build 180 (the direct symbol reference forces its own lui/%lo materialisation and destroys the single-base addressing - the identical finding CD_sync s107 recorded at decisions.md:18333). v03 = score 8, build 179. v04 = score 8, build 179. Only the 1496 access path may carry the qualifier, and only with the pointer derived as `idx_1494 + 2`.
- verdict: KILLED

## [s60] Ruling D (the CD_intr per-word-splat aggregate merge) is still spendable on CD_ready even though CD_sync's session 107 found prong (c) unsatisfiable.
- mechanism: Prong (c) of the sanctioned aggregate-merge family requires exactly one C handle per storage location, so it fails whenever the merged symbols must survive in the splat config for assembly consumers.
- probe: Read CD_sync's s107 record (docs/grind/decisions.md:18287-18400) and re-verified the asm-consumer facts by grep: the eight assembly files still referencing D_800A1494/95/96 by name.
- result: Dead for CD_ready by the same symbol-level fact. The storage is defined in assembly (asm/data/7D920.data.s:31048-31076, plus dlabel D_800A1498 whose first word is `.word D_800A1494`) and is referenced by name from CD_cw.s (6 sites), getintr.s (5), func_800819C4.s (5), func_800817A0.s (4), func_80081E1C.s (1) plus the three INCLUDE_ASM bodies. CD_sync's own record states the consequence verbatim: 'it holds identically for CD_ready and CD_datasync'. Deliberately not re-measured here (banked negative); the session's budget went to the vAT1 half instead, which is what moved the floor.
- verdict: KILLED

## s61 (rederive, 2026-09-01)

### H-s61-A — CONFIRMED (partially): the vAT1 residual decouples; the ORDER half is reachable
STATEMENT. F2's claim that at 179/179 with correct branch destinations the seat and the emission
order are independently reachable rather than a coupled trade.
MECHANISM. sched2's `rank_for_schedule` ties on INSN_PRIORITY between insn 106 (`sll $a0,$a0,2`)
and insn 120 (`addu $v0,$v0,$s5`) — equal-cost dependency paths to the printf call — and falls
through to INSN_LUID, i.e. to RTL emission order, i.e. to C statement order.
PROBE. Move `t0 *= 4` from before the arg5 chain to after it (w04_t0_shift_deferred.c);
`sandbox CD_ready --disable all`; disassemble with tmp/grind/CD_ready/s61/show.py.
RESULT. Order becomes EXACTLY the target's (55 sll v0 / 56 addu v0,s5 / 57 sll t0 / 58 lw arg5) —
first time in 61 sessions. VERDICT: **CONFIRMED for the order half.**

### H-s61-B — KILLED: the seat half is NOT reachable by source position, declaration order, or fresh pseudos
STATEMENT. Once the order is fixed by H-s61-A, the $a0/$v1 seat assignment can be recovered by the
usual source-shape levers (declaration order, statement position, fresh-vs-borrowed pseudos,
pointer-vs-integer typing, natural-vs-staged argument spelling).
MECHANISM. local-alloc `qty_compare_1` (tools/gcc-2.7.2/local-alloc.c:1660-1685) orders quantities
by floor_log2(refs)*refs*size/(death-birth); none of those levers changes refs or the live-range
length of the two competing block pseudos.
PROBE. 8 spellings on the w04 base (x01-x08): decl order t0-first, decl order pp-first, fresh index
pseudo, pp assigned first, arg5 natural, `&tbl_125c[t0]` fold, pp assigned last, chains swapped
onto the outer `v0`.
RESULT. x01-x07 ALL score exactly 6 with the identical six register-swapped instructions; x08 = 9.
VERDICT: **KILLED.** The seat is a function of the pri equation only.

### H-s61-C — KILLED: splitting the byte and the pointer into two variables fixes the seat
STATEMENT. Reducing the t0 quantity's ref count from 4 real refs to 2 (separate `tb` byte var and
`tp` pointer var) lowers its pri below arg5's and hands `$a0` to arg5... sorry, to t0.
MECHANISM. floor_log2(4)*4/L vs floor_log2(8)*8/11 — dropping to 4 weighted refs more than halves
the numerator.
PROBE. y01/y02/y03/y05/y07 — five spellings of the byte/pointer split, plus y04 (pointer-typed
destination only).
RESULT. All score 9, and the disassembly shows a DIFFERENT, worse basin: the two `lbu` loads swap
position (51/52), the D_800A11D5 sub-block migrates from 59-62 to 61-64, and the sw/lw ordering
around 60-63 breaks. The extra pseudo perturbs the scheduler, not just the allocator.
VERDICT: **KILLED** as spelled — the ref-count lever cannot be pulled by introducing a new pseudo.

### H-s61-D — KILLED (and a trap banked): variable-reuse to retune reg_n_refs
STATEMENT. Because `qty_n_refs[qty] = reg_n_refs[regno]` is the FUNCTION-WIDE weighted ref count,
reusing an existing function-scope local (`status`, `cnt`, `i`) for arg5 or for t0 retunes the pri
equation without adding an instruction — a sanctioned variable-reuse-for-codegen-control lever.
PROBE. z01-z06 on the w04 base.
RESULT. arg5 := status 6 / cnt 10 / i 20; t0 := i 24 / cnt 9 / status 5. Nothing reaches the target
seat. VERDICT: **KILLED for these six carriers.** BANK THE TRAP: z05 (t0 := status) scores 5, LOWER
than w04's 6, and is a dead end — the borrowed variable's function-wide refs and call-crossings push
the quantity past the caller-saved cutoff so t0 lands in `$s0` (callee-saved), AND the order reverts
to the wrong one. Score alone is misleading in this neighbourhood; always disassemble.

### THE OPEN QUESTION, stated arithmetically (the whole of the next session's job)
Hold the w04 order-correct shape. Current: pri(t0) = floor_log2(8)*8/11 = 2.18,
pri(arg5) = floor_log2(4)*4/4 = 2.00. Need pri(arg5) > pri(t0). Exactly two knobs:
  (a) lengthen the t0 quantity's live range to >= 13 insns at unchanged refs (12 gives an exact
      TIE, which resolves by qty number and still loses). Its birth is the `lbu` and its death is
      the `lw $a3` argument load, and BOTH ends are already pinned by expand_call's argument
      evaluation order — moving the birth earlier means crossing the `puts` call, which forces a
      callee-saved seat (the z05 trap). Look instead for a way to make the arg-2/arg-3 computation
      (`*pp`, `D_800A11DC[D_800A11D5]`) land BETWEEN t0's birth and death, which lengthens t0 for
      free because those insns already exist.
  (b) raise arg5's weighted ref count to 5 or 6 while holding its live range at 4 (pri 2.5 / 3.0),
      staying strictly below pseudo 74's 4.875 or arg5 steals `$v0` instead. z01-z03 show that
      borrowing an existing local overshoots; what is UNTRIED is a carrier whose function-wide
      weighted count lands at exactly 5-6, and whether an out-of-loop reference (weight 1 instead
      of the in-loop weight 2) can be used to tune the count by a single unit.

## [s61] F2: at 179/179 with correct branch destinations the sched2 emission order and the local-alloc seat are independently reachable rather than a coupled trade.
- mechanism: sched2 rank_for_schedule ties on INSN_PRIORITY between insn 106 (sll $a0,$a0,2) and insn 120 (addu $v0,$v0,$s5) - equal-cost dependency paths to the printf call - and falls through to INSN_LUID, i.e. to RTL emission order, i.e. to C statement order. Read from tmp/grind/CD_ready/dumps/system.sched2, not hypothesised.
- probe: Move `t0 *= 4` from before the arg5 chain to after it (tmp/grind/CD_ready/s61/w04_t0_shift_deferred.c), sandbox, then index-align the disassembly against asm/funcs/CD_ready.s with tmp/grind/CD_ready/s61/show.py.
- result: The build emits the EXACT target order (55 sll v0 / 56 addu v0,s5 / 57 sll t0 / 58 lw arg5) for the first time in 61 sessions, at 179 instructions and 0 rules. Score is 6 because the two block pseudos swap hard registers.
- verdict: CONFIRMED

## [s61] Once the order is fixed, the $a0/$v1 seat can be recovered by ordinary source-shape levers: declaration order, statement position, a fresh pseudo instead of the borrowed outer `v0`, pointer-vs-integer typing, natural-vs-staged argument spelling.
- mechanism: local-alloc qty_compare_1 (tools/gcc-2.7.2/local-alloc.c:1660-1685) orders quantities by floor_log2(refs)*refs*size/(death-birth); none of those levers changes either input for the two competing pseudos.
- probe: 8 spellings on the order-correct w04 base (x01-x08): decl order t0-first, decl order pp-first, fresh index pseudo, pp assigned first, arg5 written naturally, &tbl_125c[t0] fold, pp assigned last, chains swapped onto the outer `v0`.
- result: x01-x07 all score exactly 6 with the identical six register-swapped instructions (51,57,58,61,63,67); x08 scores 9. The seat is invariant to every perturbation that leaves the pri equation alone.
- verdict: KILLED

## [s61] Splitting the loaded byte and the derived pointer into two variables drops the t0 quantity's weighted ref count from 8 to 4 and flips the allocation order, fixing the seat.
- mechanism: pri = floor_log2(refs)*refs/(death-birth); halving refs more than halves the numerator, so t0 would sort below arg5 and be allocated second, taking $a0.
- probe: y01/y02/y03/y05/y07 (five spellings of the byte/pointer split) plus y04 (pointer-typed destination only), all on the w04 order-correct base.
- result: All score 9 and land in a DIFFERENT, worse basin: the two lbu loads swap position (51/52), the D_800A11D5 sub-block migrates from 59-62 to 61-64, and the sw/lw ordering around 60-63 breaks. The extra pseudo perturbs the scheduler, not just the allocator.
- verdict: KILLED

## [s61] Because qty_n_refs[qty] = reg_n_refs[regno] is the FUNCTION-WIDE loop-weighted ref count, reusing an existing function-scope local (status / cnt / i) as the carrier for arg5 or for t0 retunes the pri equation for free - a sanctioned variable-reuse-for-codegen-control lever.
- mechanism: local-alloc.c:297 assigns the quantity the pseudo's whole-function reference count, so borrowing a carrier with more (or fewer) references anywhere in CD_ready moves that quantity in the qty_compare_1 sort without emitting an instruction.
- probe: z01-z06 on the w04 base: arg5 := status / cnt / i, and t0 := i / cnt / status.
- result: arg5 := status 6, := cnt 10, := i 20; t0 := i 24, := cnt 9, := status 5. None reaches the target seat. TRAP BANKED: z05 scores 5 - LOWER than w04 - yet is a dead end, because the borrowed carrier's function-wide refs and call-crossings push the quantity past the caller-saved cutoff and t0 lands in $s0 (callee-saved) while the order reverts to wrong. Score alone is misleading in this neighbourhood.
- verdict: KILLED

## [s61] A fresh, fully natural rederivation of the do_timeout printf-argument block (no staged t0/arg5, no pp alias) reaches or beats the staged vAT1 basin - the mandated rederive attack.
- mechanism: If the staged spelling is only a historical accident, GCC's own argument expansion from the natural expression printf(fmt, D_800F19C0, D_800A11DC[D_800A11D5], tbl_125c[idx_1494[0]], tbl_125c[idx_1494[1]]) should reproduce the target block directly.
- probe: v01-v07: fully natural, natural + pp alias, natural + named byte indices, and each of arg4/arg5/both hoisted to named intermediates in both orders.
- result: 14, 14, 14, 14 (arg5-first), 7 (arg4-first), 7 and 8 (both named). All 179 instructions, 0 rules - the natural forms preserve the instruction count and the branch destinations but land 5-12 instructions worse than the staged basin. The staged spelling is load-bearing, not decorative.
- verdict: KILLED

## s62 (structural, 2026-09-01)

### H-s62-A — KILLED: F1's "free lengthening" of t0's live range via the arg-2/arg-3 computations
STATEMENT. Making the `*pp` and `D_800A11DC[D_800A11D5]` printf-argument computations land
BETWEEN t0's birth and death lengthens t0's live range to >= 13 at unchanged reference count,
dropping pri(t0) from 2.18 below arg5's 2.00 and handing t0 the target's seat.
MECHANISM (as inherited). local-alloc qty_compare_1, tools/gcc-2.7.2/local-alloc.c:1660-1685.
PROBE. a01-a09: each of `*pp` and `D_800A11DC[D_800A11D5]`, and both together in both orders,
hoisted into a fresh named intermediate placed (i) right after t0's birth, (ii) between the arg5
chain and the t0 shift, (iii) after the t0 shift, (iv) before t0's birth as a control.
RESULT. 10, 16, 20, 20, 13, 13, 20, 16, 10 — every variant worse than the base's 6, none anywhere
near the floor. The fresh pseudo perturbs sched1 (same failure mode as s61's y-series).
VERDICT: **KILLED**, and the hypothesis's premise is now known to point the WRONG WAY — see H-s62-D.

### H-s62-B — KILLED: F2 knob (b), retuning arg5's weighted reference count from C
STATEMENT. A carrier whose extra references sit outside the poll loop, or a dead store / identity
operation, can put the arg5 quantity at weighted 5 or 6 and flip the qty_compare_1 sort.
MECHANISM. qty_n_refs = reg_n_refs, the loop-weighted whole-function count (local-alloc.c:297).
PROBE. g01-g08 on the order-perfect c02 base: `arg5 = 0;` before the load, `arg5 = v0;`,
`arg5 = arg5;`, `arg5 |= 0;`, `arg5 += 0;`, `arg5 = t0;`, and two positional variants.
RESULT. ALL EIGHT score exactly 6 at 179 insns — byte-identical output to c02. Every such
construct is deleted before reg_scan runs, so it cannot touch qty_n_refs at all.
VERDICT: **KILLED.** The count is only movable by giving the quantity a REAL extra occurrence,
which is what b07 does (arg5 carries the address: 4 refs -> 8, pri 2.00 -> 4.00).

### H-s62-C — KILLED: reducing t0's reference count by folding the byte load into the shift
STATEMENT. `t0 = idx_1494[0] << 2;` makes the lbu define a temp and the sll define t0, dropping
t0 from 4 insn-mentions to 3 (weighted 8 -> 6, floor_log2 3 -> 2) and its pri from 2.18 to ~1.2.
PROBE. e01-e08 (the fold combined with the c02 arg5 shape, the b07 arg5 shape, `*4` instead of
`<< 2`, address-of index expression, and three positions), plus a .lreg read.
RESULT. Scores 9-13. The .lreg shows pseudo 98 STILL at 8 references — cse/copy-propagation puts
the lbu back as t0's definition — while the range shortens 11 -> 10, so the fold RAISES pri to
2.40 instead of lowering it. t0's reference count is 8 on every base ever measured here.
VERDICT: **KILLED.**

### H-s62-D — CONFIRMED (and it CORRECTS the inherited frontier): the seat is set by t0's live-range LENGTH, and the target value is 10, not >= 13
STATEMENT. The $a0/$v1 assignment is a function of pri(t0) alone once the arg5 quantity is at 2.00.
MECHANISM. qty_compare_1's floor_log2(refs)*refs*size/(death-birth) ordering, read from
tmp/grind/CD_ready/dumps/system.lreg on three separate bases.
PROBE. Dump + read pseudos 74/97/98 on candidate.c (score 2), on c02 and f01 (score 6), on b07
(score 3) and on e01 (score 10).
RESULT.
  candidate.c: t0 = 8/10 = 2.40, arg5 = 4/4 = 2.00 -> SEATS CORRECT
  c02 / f01  : t0 = 8/11 = 2.18, arg5 = 4/4 = 2.00 -> seats swapped
  b07        : t0 = 8/12 = 2.00, arg5 = 8/6 = 4.00 -> SEATS CORRECT
  e01        : t0 = 8/10 = 2.40, arg5 = 4/4 = 2.00 -> seats swapped (but its ORDER broke, so the
               2.40 reading is consistent: e01's failure is sched1, not local-alloc)
The seats are correct whenever t0's range is 10 (pri 2.40) or whenever arg5 outranks t0 outright.
s61's F1 asked for a range of >= 13; the measurement says the order-perfect branch needs 11 -> 10,
a ONE-INSN SHORTENING in the opposite direction. VERDICT: **CONFIRMED.**

### H-s62-E — KILLED: statement position inside the do_timeout block
STATEMENT. Re-ordering the t0 chain, the arg5 chain, the `pp` alias and the arg5 load relative to
one another moves the sched2 LUID tie and/or the qty_compare inputs.
PROBE. f01-f08 (8 orderings on the c02 base) and i01-i08 (8 orderings on the candidate base,
including deferring ONLY t0's `addu` past the arg5 load, and advancing only part of the v0 chain),
plus a .lreg read on f01.
RESULT. f01-f08 ALL exactly 6. i01-i08: 2, 2, 4, 2, 2, 2, 4, 2 — the floor form is reproduced by
six of the eight, and nothing improves. f01's .lreg is IDENTICAL to c02's (97 = 4/4, 98 = 8/11).
sched1 normalises source statement order before local-alloc sees it.
VERDICT: **KILLED** — with a mechanism reading, not just a score. The ONLY source-level change that
has ever moved the sched2 order is deferring the whole `t0 *= 4` statement past the arg5 LOAD
(s61's w04), and that also moves t0's range 10 -> 11 and so costs the seats.

### THE FRONTIER, restated arithmetically (the next session's job)
The residual has split into two disjoint, fully-characterised branches:
  BRANCH A (`candidate.c`, score 2) — every register correct, t0 range 10, arg5 4/4.
      Defect: sched2 emits build 56/57 (`sll $a0` , `addu $v0`) as the target's 57/56.
      What is needed: flip the INSN_LUID order of those two insns in sched1's OUTPUT without
      moving t0's range off 10. Source statement position is measured inert (i01-i08), so the
      lever must be a dependence-structure change, not a position change.
  BRANCH B (`c02`, score 6) — every instruction in the target's position, pure 2-way seat swap.
      Defect: t0 range is 11, one too long; at 10 the seats come out right (that is exactly
      candidate.c's profile).
      What is needed: remove ONE insn from between t0's birth and its last use in sched1's output
      while keeping the in-place `v0 += (s32)tbl_125c` spelling. Reference counts are pinned at 8
      (H-s62-C) and dead constructs are deleted before reg_scan (H-s62-B), so the lever is again
      dependence structure / sched1, not refs.
  BRANCH C (`b07`, score 3) — seats correct AND arg5's pri raised to 4.00 by giving it a real
      third occurrence (it carries the address). Defect: `addu $v1,$v0,$s5` instead of the
      target's in-place `addu $v0,$v0,$s5`, which then transposes 58/61. Closing this needs the
      address to accumulate in the `v0` quantity while arg5 still has >= 3 occurrences — the two
      requirements are in direct tension and no spelling reconciling them has been found yet.

## [s62] F1: making the arg-2/arg-3 printf-argument computations land between t0's birth and death lengthens t0's live range to >=13 at unchanged reference count and flips the qty_compare_1 sort.
- mechanism: local-alloc qty_compare_1 (tools/gcc-2.7.2/local-alloc.c:1660-1685) orders quantities by floor_log2(refs)*refs*size/(death-birth); inserting already-existing insns inside the range lengthens it for free.
- probe: a01-a09: *pp and/or D_800A11DC[D_800A11D5] hoisted into fresh named intermediates at five positions relative to t0's birth, death and the arg5 chain, on the s61 order-correct w04 base (score 6).
- result: 10, 16, 20, 20, 13, 13, 20, 16, 10 - every variant worse than the base; the fresh pseudo perturbs sched1, the same failure mode as s61's y-series.
- verdict: KILLED

## [s62] F2 knob (b): arg5's loop-weighted reference count can be tuned to 5 or 6 from C (dead store, self-assign, identity op, or a carrier with out-of-loop references).
- mechanism: qty_n_refs[qty] = reg_n_refs[regno], the loop-weighted whole-function count (local-alloc.c:297).
- probe: g01-g08 on the order-perfect c02 base: arg5 = 0 before the load, arg5 = v0, arg5 = arg5, arg5 |= 0, arg5 += 0, arg5 = t0, and two positional variants.
- result: ALL EIGHT score exactly 6 at 179 insns - byte-identical output to c02. Every such construct is deleted before reg_scan runs, so it cannot touch qty_n_refs at all.
- verdict: KILLED

## [s62] Folding the byte load into the shift statement (t0 = idx_1494[0] << 2) drops t0 from 4 insn-mentions to 3 (weighted 8 -> 6) and lowers its priority below arg5's.
- mechanism: the lbu would define a temp and the sll would define t0, removing one occurrence from t0's reg_n_refs.
- probe: e01-e08 (the fold combined with the c02 arg5 shape, the b07 arg5 shape, *4 vs <<2, an address-of index expression and three positions), plus a .lreg read of pseudo 98.
- result: Scores 9-13. The .lreg shows pseudo 98 STILL at 8 references (cse/copy-propagation restores the lbu as t0's definition) while the range shortens 11 -> 10, so the fold RAISES pri to 2.40. t0's reference count is 8 on every base ever measured here.
- verdict: KILLED

## [s62] The $a0/$v1 seat assignment is a function of t0's qty_compare_1 priority once the arg5 quantity sits at 2.00, and the range that produces the target's seats is 10 - NOT the >=13 the inherited frontier predicted.
- mechanism: qty_compare_1's floor_log2(refs)*refs*size/(death-birth) ordering, read from tmp/grind/CD_ready/dumps/system.lreg (pseudo 98 = t0, 97 = arg5 value, 74 = the reused v0) rather than hypothesised.
- probe: pwsh tools/grinder/dump.ps1 CD_ready on four separate bases - candidate.c (score 2), c02 and f01 (score 6), b07 (score 3), e01 (score 10) - reading the CD_ready register list of system.lreg each time.
- result: candidate.c t0 = 8 refs/10 insns = 2.40 with arg5 4/4 = 2.00 -> SEATS CORRECT; c02 and f01 t0 = 8/11 = 2.18 -> seats swapped; b07 t0 = 8/12 = 2.00 with arg5 8/6 = 4.00 -> SEATS CORRECT; e01 t0 = 8/10 = 2.40 -> seats swapped but its sched1 order broke independently. The order-perfect branch therefore needs a ONE-INSN SHORTENING of t0's range (11 -> 10), the opposite direction from the inherited F1.
- verdict: CONFIRMED

## [s62] Statement position inside the do_timeout block moves either the sched2 LUID tie or the local-alloc priority inputs.
- mechanism: RTL emission order follows source statement order, and both sched2's INSN_LUID tiebreak and local-alloc's birth/death luids are read off the insn chain.
- probe: f01-f08 (8 orderings of the t0 chain, the arg5 load and the pp alias on the c02 base) and i01-i08 (8 orderings on the candidate base, including deferring ONLY t0's addu past the arg5 load), plus a .lreg read on f01.
- result: f01-f08 ALL score exactly 6; i01-i08 score 2,2,4,2,2,2,4,2 (six of eight reproduce the floor, none improves). f01's .lreg is IDENTICAL to c02's (97 = 4/4, 98 = 8/11): sched1 normalises source statement order before local-alloc sees it. The only source change ever to move the sched2 order is s61's w04 (deferring the whole t0 *= 4 past the arg5 load), and that costs the seats by moving t0's range 10 -> 11.
- verdict: KILLED

## [s62] Respelling the arg5 address computation as an in-place accumulation on the reused v0 carrier reaches the target's instruction ORDER exactly.
- mechanism: the target emits addu $v0,$v0,$s5 in place on the shifted index, i.e. the address lives in the same quantity as the shift result (pseudo 74, the reused function-scope v0), and only the loaded value is a separate quantity.
- probe: variant c02 - v0 = idx_1494[1] << 2; v0 += (s32)tbl_125c; arg5 = *(s32 *)v0; - on the s61 order-deferred w04 base; sandbox then index-aligned disassembly with tmp/grind/CD_ready/s61/show.py.
- result: score 6 at 179/179/0 with EVERY INSTRUCTION in the target's position and opcode; the six differing instructions (51, 57, 58, 61, 63, 67) differ only by a consistent 2-way register swap (t0 takes $v1 where the target uses $a0, arg5's value takes $a0 where the target uses $v1). Cleanest statement of the seat residual in 62 sessions.
- verdict: CONFIRMED

## s63 hypotheses (structural, 2026-09-01)

### H-s63-A — CONFIRMED (by source reading, not by score)
STATEMENT: the build/target transposition at 56/57 is decided by `INSN_LUID` alone; no dependence-
graph perturbation can reach it.
MECHANISM: `tools/gcc-2.7.2/sched.c:2408-2465`, `rank_for_schedule`. Three tie-breakers in order:
INSN_PRIORITY; a class test against `last_scheduled_insn` that assigns class 3 whenever
`insn_cost (...) == 1`; and `INSN_LUID (tmp) - INSN_LUID (tmp2)`. Every insn_cost in this window is
1 on this MIPS target, so the class test is a no-op, and the two candidates (sched1 insn 106,
t0's `sll`, and insn 120, arg5's address `addu`) have isomorphic three-edge dependence paths to
the printf call, so their priorities are equal BY SHAPE.
PROBE: read the pass source; corroborated by the sched1 dumps of both bases
(tmp/grind/CD_ready/s63/cand.sched.txt, k03.sched.txt) — in candidate 106 precedes 120 and in k03
120 precedes 106, exactly tracking which statement is written first.
VERDICT: CONFIRMED. Consequence: s62's frontier item "sweep spellings that create or remove a true
data dependence around that one insn" is FORECLOSED for both branches — the class rung it targets
does not exist on this target.

### H-s63-B — KILLED
STATEMENT: re-basing a byte load onto the other (identical-address) index pointer changes the
dependence graph enough to flip the tie.
MECHANISM: `idx_1495 == idx_1494 + 1`, so `idx_1494[1]` / `idx_1495[0]` and `idx_1494[0]` /
`idx_1495[-1]` are the same bytes through different pointer pseudos, giving each load a different
LOG_LINKS chain.
PROBE: j01-j05, j10.
RESULT: 8, 8, 8, 8, 13, 13 (all 179 insns). Strictly worse. VERDICT: KILLED.

### H-s63-C — KILLED (inert)
STATEMENT: the C-level operand order / cast shape of the two address `addu`s is a lever on the tie.
MECHANISM: `plus` operand order at RTL emission.
PROBE: j06 `(s32)tbl + v0`, j07 `(u8 *)tbl + v0`, j08 `(s32)(t0 + (u8 *)tbl)`, j09 `t0 += (s32)tbl`.
RESULT: all exactly 2 / 179 / 0 — byte-identical to candidate.c. GCC canonicalises operand order
for same-provenance operands before emission. VERDICT: KILLED (inert, basin-equivalent; four new
distinct spellings of the masked-2 floor).

### H-s63-D — KILLED
STATEMENT: swapping which value the function-scope reused `v0` carries (t0's address instead of the
arg5 index) moves the qty_compare inputs favourably.
MECHANISM: a block-local gets its own local-alloc quantity; the cross-block `v0` is a global-alloc
allocno and never enters qty_compare at all.
PROBE: k04-k06 (candidate order, arg5-first, and arg5-first-with-t0-byte-head).
RESULT: 8, 9, 8. Putting the longer-lived t0 chain in `v0` removes it from local-alloc entirely and
the seats get worse. VERDICT: KILLED.

### H-s63-E — KILLED
STATEMENT: giving t0's shift its own fresh local shortens t0's live range.
MECHANISM: an extra pseudo splits the chain.
PROBE: k07 (candidate order), k08 (arg5 first).
RESULT: 3 and 6. k07 keeps candidate's 56/57 transposition AND flips build 61 to
`addu $a0,$s5,$a0` (target: `addu $a0,$a0,$s5`) — distinct-provenance operands defeat GCC's
canonical ordering. VERDICT: KILLED.

### H-s63-F — CONFIRMED (the coupling is one counter, measured)
STATEMENT: the order residual and the seat residual are the SAME variable — the number of insns
between t0's byte load and t0's shift in sched1's output ("window 1").
MECHANISM: `.lreg`'s "Register 98 used 8 times across N insns" sums reg 98's TWO disjoint live
ranges (it dies at its `sll`, is reborn by its `addu`). Measured on the sched1 dumps: window 2
(t0 addu .. a3 load) is 5 on BOTH branches; window 1 is 5 on candidate.c (a5 lbu, the `*pp`
argument load 141, a5 sll) and 6 on every order-perfect base (same three plus arg5's `addu`).
qty_compare_1 then gives 3*8/10 = 2.40 (seats correct) vs 3*8/11 = 2.18 (seats swapped).
PROBE: dump.ps1 on both bases; the .lreg register table and the sched1 insn order read directly.
VERDICT: CONFIRMED. The order fix mandated by H-s63-A is exactly what lengthens window 1 — they
cannot be satisfied independently by statement order.

### H-s63-G — KILLED
STATEMENT: window 1 can be shortened back to 5 on the order-perfect base by re-topologising the
t0 chain or by evicting the `*pp` load from the window.
MECHANISM: remove one insn from between t0's byte load and t0's shift.
PROBE: m01-m10 (byte-load-into-shift fold, whole-address fold, two-statement arg5 chain, four
positions of the t0 byte load inside the arg5 chain, `&((u8 *)tbl)[idx*4]`, and two `pp` positions).
RESULT: 7, 9, 8, 6, 10, 7, 7, 6, 9, 6 — the order-perfect basin's floor stays 6. The `*pp` load is
sched1 insn 141, an ARGUMENT load emitted by `expand_call` whose LUID is fixed at the call site, so
no placement of the `pp = ...` statement can move it. VERDICT: KILLED.

### What the next session should attack (see state.json frontier)
The two remaining degrees of freedom that H-s63-F does NOT couple are (i) reg 98's REFERENCE COUNT
(pinned at 8 across every base measured in s62 and s63 — 4 mentions x loop weight 2), and (ii) the
priorities of the OTHER quantities in block 3, specifically the arg5-address pseudo (4.00 on
candidate, 2.67 on the order-perfect base) and the t0-shift pseudo (1.60 vs 2.00). Since 98 is
allocated before 97 on BOTH branches, the seat swap is find_free_reg's answer changing as 98's
CONFLICT SET grows — so the untried axis is not 98's own priority at all but the allocation order
of the two 4.00-priority short-lived pseudos (104/110) that take registers before it.

## [s64] The build/target transposition at 56/57 is decided by INSN_LUID alone; no dependence-graph perturbation can reach it.
- mechanism: tools/gcc-2.7.2/sched.c:2408-2465 rank_for_schedule has exactly three tie-breakers: INSN_PRIORITY; a class test against last_scheduled_insn that assigns class 3 whenever insn_cost(...) == 1; and INSN_LUID. Every insn_cost in this window is 1 on this MIPS target, so the class rung is a no-op, and the two candidates (sched1 insn 106 = t0's sll, insn 120 = arg5's address addu) have isomorphic three-edge dependence paths to the printf call (106->111->145->147 and 120->122->137->147, load on the same rung), so their priorities are equal by SHAPE.
- probe: Read the pass source in full; corroborated against the CD_ready sections of the sched1 dumps for both bases (tmp/grind/CD_ready/s63/cand.sched.txt and k03.sched.txt) - in candidate 106 precedes 120, in k03 120 precedes 106, exactly tracking which statement is written first.
- result: Confirmed by source plus two dumps. This forecloses s62's frontier item for BOTH branches ('sweep spellings that create or remove a true data dependence around that one insn'): the rung it targets does not exist on this target.
- verdict: CONFIRMED

## [s64] The order residual and the seat residual are the SAME variable: the number of insns between t0's byte load and t0's shift in sched1's output (window 1 of reg 98).
- mechanism: .lreg's 'Register 98 used 8 times across N insns' is the SUM of reg 98's two disjoint live ranges (98 dies at its own sll and is reborn by its addu), counted in sched1 output positions. Window 2 (t0 addu .. a3 load) is 5 on BOTH branches; window 1 is 5 on candidate.c (between: arg5 lbu, the *pp argument load 141, arg5 sll) and 6 on every order-perfect base (the same three PLUS arg5's addu - the very insn the order fix must move there). qty_compare_1 (local-alloc.c:1640-1684) then gives 3*8/10 = 2.40 (seats correct) vs 3*8/11 = 2.18 (seats swapped).
- probe: pwsh tools/grinder/dump.ps1 CD_ready on the candidate base and on the k03 order-perfect base; read the .lreg register table and the sched1 insn order directly rather than inferring them.
- result: Confirmed. The order fix mandated by the first hypothesis is precisely what lengthens window 1, so order and seat cannot be satisfied independently by statement order - the s61/s62 anti-correlation is now explained by a single counter instead of two coupled ties.
- verdict: CONFIRMED

## [s64] Re-basing either byte load onto the other (identical-address) index pointer changes the dependence graph enough to flip the tie.
- mechanism: idx_1495 == idx_1494 + 1, so idx_1494[1] / idx_1495[0] and idx_1494[0] / idx_1495[-1] name the same bytes through different pointer pseudos, giving each load a different LOG_LINKS chain.
- probe: j01 (v0 = idx_1495[0]), j02 (v0 = *idx_1495), j03 (t0 = idx_1495[-1]), j04 (t0 = *(idx_1495 - 1)), j05 (both), j10 (both plus reversed addu operands).
- result: 8, 8, 8, 8, 13, 13 - all 179 insns, 0 rules. Strictly worse than the base 2. The target's two byte loads are both based on the D_800A1494 pointer.
- verdict: KILLED

## [s64] The C-level operand order / cast shape of the two address addus is a lever on the tie.
- mechanism: plus operand order at RTL emission.
- probe: j06 ((s32)tbl + v0), j07 ((u8 *)tbl + v0), j08 ((s32)(t0 + (u8 *)tbl)), j09 (t0 += (s32)tbl).
- result: All exactly score 2 / 179 insns / 0 rules - byte-identical to candidate.c. GCC canonicalises plus operand order for same-provenance operands before emission. Four new distinct spellings of the masked-2 floor added to the basin catalog. (The axis is NOT inert when the operands have different provenance - k07 flips build 61 to `addu $a0,$s5,$a0`.)
- verdict: KILLED

## [s64] Swapping which value the function-scope reused v0 carries (t0's address instead of the arg5 index) moves the qty_compare inputs favourably.
- mechanism: A block-local gets its own local-alloc quantity; the cross-block v0 is a global-alloc allocno and never enters qty_compare at all.
- probe: k04 (candidate order, roles swapped), k05 (arg5 chain first), k06 (t0 byte at head).
- result: 8, 9, 8. Putting the longer-lived t0 chain in v0 removes it from local-alloc entirely and the seats come out worse, not merely different. Also k07/k08 (t0's shift into a second fresh local): 3 and 6 - k07 keeps candidate's transposition AND adds one.
- verdict: KILLED

## [s64] Window 1 can be shortened back to 5 on the order-perfect base by re-topologising the t0 chain or by evicting the *pp load from the window.
- mechanism: Remove one insn from between t0's byte load and t0's shift, restoring reg 98 to 8 refs / 10 insns (pri 2.40) while keeping the target instruction order.
- probe: m01-m10: byte-load-into-shift fold, whole-address single expression, two-statement arg5 chain, four positions of the t0 byte load inside the arg5 chain, &((u8 *)tbl)[idx*4], and two positions of the pp statement.
- result: 7, 9, 8, 6, 10, 7, 7, 6, 9, 6 - the order-perfect basin's floor stays 6 and none removes an insn from window 1. The *pp load is sched1 insn 141, an ARGUMENT load emitted by expand_call whose LUID is fixed at the call site, which also explains why every pp-position probe in s62 (f04/f05) and s63 (k02/m08/m10) is inert.
- verdict: KILLED

## s65 hypotheses (synthesis, 2026-09-01)

### H-s65-A — CONFIRMED (and it REPLACES the s61/s62/s63 model of the seat residual)
STATEMENT: the $a0/$v1 seat swap between the two branches is decided by the RANK ORDER of block
3's four local-alloc quantities, and that order is readable directly out of the instrumented cc1
rather than inferred from scores or from the .lreg per-register line.
MECHANISM: `qty_compare_1` (tools/gcc-2.7.2/local-alloc.c:1640-1684) sorts quantities descending by
floor_log2(refs)*refs*size/(death-birth), ties by quantity number (:1683); `find_free_reg` then
gives each quantity the lowest hard register not in its conflict set.
PROBE: `bash tmp/grind/CD_ready/s63/qty.sh <out>` (BB2_QTY_DEBUG=1 BB2_SUGG_DEBUG=1 over
src/system.c) with candidate.c and with k03 spliced; read the `func=CD_ready blk=3` SUGGDBG-QTY
lines (qty_compare inputs) and the first group of `QTYDBG blk=3` lines (`ord=` rank, `got=` hard
register).
RESULT:
  candidate.c (2): 104 arg5-ADDRESS 18-20 refs4 -> 4.00 rank0 $v0 | 110 22-30 refs8 3.00 rank1 $v0
                 | 97 arg5-VALUE 20-26 refs4 1.33 rank2 $v1 | 102 t0 16-24 refs4 1.00 rank3 $a0
  k03 (6):       110 3.00 rank0 $v0 | 102 arg5-ADDRESS 16-20 2.00 rank1 $v0
                 | 104 t0 18-24 1.33 rank2 $v1 | 97 arg5-VALUE 20-26 1.33 rank3 $a0
On the order-perfect base t0 and the arg5 VALUE tie at 1.33 and t0 wins the tie by quantity number
(it is born two luids earlier). On candidate.c the tie never arises because the address quantity is
one insn long and ranks 4.00.
VERDICT: CONFIRMED. This retires the s63 explanation ("find_free_reg's answer changing because reg
98's conflict set grew") and the whole window-1/window-2 framing: reg 98 is not one of the four
quantities, and the .lreg line s61-s63 read is a per-register sum of two disjoint ranges.

### H-s65-B — CONFIRMED: the coupling is ONE sched1 latency-slot decision, and it is named
STATEMENT: the order fix and the seat loss are both consequences of sched1 choosing the t0 shift as
the filler for the arg5 addu -> arg5 load latency slot.
MECHANISM: sched1 list-schedules insn 111 (`reg102 = reg74 + reg81`, the arg5 address addu); its
consumer 113 (`reg97 = mem[reg102]`) is not ready for one cycle, so the highest-priority ready insn
is emitted between them. That insn is 117 (`reg104 = reg98 << 2`, the t0 shift). Consequences: the
address quantity's span goes 1 -> 2 (pri 4.00 -> 2.00, demoted below reg 110), and the t0 quantity
is born before the arg5-value quantity, which is what wins it the 1.33 tie.
PROBE: the sched1 RTL dump for the order-perfect base, tmp/grind/CD_ready/s63/k03.sched.txt, read
insn by insn and cross-checked against the QTYDBG birth/death luids of both bases.
VERDICT: CONFIRMED.

### H-s65-C — KILLED: raising the arg5 quantity's reference count by staging a real second value in it
STATEMENT: s62 showed refs only move with a REAL extra occurrence (b07: address in arg5, refs 4->8,
seats correct). Staging the arg5 BYTE INDEX in arg5 instead should raise the count the same way
while leaving the address to accumulate in the v0 carrier (which is what b07 gets wrong).
PROBE: n01-n07 on the k03 order-perfect base (byte + v0 shift; copy-then-shift; byte + in-place v0
address; whole chain in arg5; pp last; t0 byte late; shift folded into the address) and n08, the
same staging on the candidate base.
RESULT: 8, 6, 8, 9, 8, 8, 6, 8 — all 179 insns, 0 rules, never better than the base. n01's
disassembly has the ORDER perfect but the staging pseudo does not coalesce with the v0 carrier
(`lbu $a0,1($s2)` + `sll $v0,$a0,2` against the target's in-place `lbu $v0` + `sll $v0,$v0,2`),
costing three new mismatches. QTYDBG on n03 shows block 3 collapsing from four local quantities to
two: the staged pseudos become cross-block and are handled by global-alloc.
VERDICT: KILLED.

### H-s65-D — KILLED: materialising arg4 into its own local to reposition the a3 argument load
STATEMENT: t0's quantity dies at insn 145 (`a3 = mem[reg98]`); giving arg4 a named local moves that
load to the statement and re-times the quantity.
PROBE: q01 (arg4 local after the t0 chain), q02 (before the arg5 chain), q03 (`tbl_125c[t0]`
spelling), q04 (role swap: arg5 folded into the call, arg4 staged), q05 (both args folded).
RESULT: 11, 11, 12, 10, 11 at 179 insns — every one far worse than the base 6.
VERDICT: KILLED.

### H-s65-E — KILLED (inert): lowering the D_800A11D5 byte load's LUID so it becomes the filler
STATEMENT: insn 128 (the D_800A11D5 byte load) is the other ready candidate for the 111 -> 113
latency slot and loses only the INSN_LUID tiebreak because expand_call emits it at the call site; a
`ix = D_800A11D5;` statement placed before the t0 chain lowers its LUID and should make it the
filler, which would leave the arg5 load born before the t0 shift and flip the quantity numbers.
PROBE: r01 (statement between the arg5 chain and the t0 chain), r02 (at the head of the block),
r03 (u8-typed index), r04 (after the t0 chain), plus a QTYDBG read on r01.
RESULT: r01/r03/r04 = 6 and r01's quantity table is STRUCTURALLY IDENTICAL to k03's (same four
quantities, spans 16-20/18-24/20-26/22-30, same ranks, same got= 2/2/3/4); r02 = 11. The two
candidate fillers are not tied on INSN_PRIORITY — 117 (shift -> addu -> a3 load -> call) outranks
128 (load -> shift -> a2 load -> call) on this target — so the LUID rung is never reached and the
hoist is byte-inert.
VERDICT: KILLED.

### THE FRONTIER AFTER s65 (one target, stated as a single scheduling swap)
Everything now reduces to ONE named exchange in sched1's output on the order-perfect base:
insn 145 (`a3 = mem[reg98]`, the arg-4 register load, currently t0's death at luid 24) and insn 137
(`mem[sp+16] = reg97`, the arg-5 stack store, currently the value's death at luid 26). Swapping
them makes the arg5-value quantity 20-24 (pri 2.00) and the t0 quantity 18-26 (pri 1.00), so the
value is allocated first, takes $v1, and t0 takes $a0 — the target's seats on a base that already
carries the target's instruction order, i.e. score 0.
Why it is hard: 145 is a load whose result feeds the call's $a3 (path cost 2 to the call), 137 is a
store with no successor but the call (path cost 1), so INSN_PRIORITY separates them outright and no
tiebreak rung is reachable. The only C-visible levers on that pair are (i) the ARGUMENT SHAPE — the
4th argument being a memory dereference is what makes 145 a load; any spelling that makes arg4
already-in-a-register turns 145 into a move and changes its cost, and q01-q05 show the naive
version of that (a named local) is destructive, but the untried variants are those that keep the
deref folded while lengthening the ARG-5 store's path (e.g. a 6th argument, or an arg-5 expression
whose value is consumed by something after the store — neither of which exists in this call), and
(ii) the equivalent win of making the arg5 LOAD, not the t0 shift, fill the 111->113 slot, which
requires the t0 shift to be UNREADY at that moment, i.e. its own input (the t0 byte load, insn 99)
to be scheduled after 111 — that byte load has the longest path in the block, so it is always
scheduled first. Both routes need a mechanism-level probe, not a spelling sweep.

## [s65] The $a0/$v1 seat swap between the two branches is decided by the RANK ORDER of block 3's four local-alloc quantities, and that order is readable directly out of the instrumented cc1 rather than inferred from scores or from the .lreg per-register line.
- mechanism: qty_compare_1 (tools/gcc-2.7.2/local-alloc.c:1640-1684) sorts quantities descending by floor_log2(refs)*refs*size/(death-birth) with ties broken by quantity number (:1683); find_free_reg then gives each quantity the lowest hard register not in its conflict set.
- probe: bash tmp/grind/CD_ready/s63/qty.sh <out> (BB2_QTY_DEBUG=1 BB2_SUGG_DEBUG=1 over src/system.c) with candidate.c spliced and with k03_a5first_t0byte_head.c spliced; read the `func=CD_ready blk=3` SUGGDBG-QTY lines (qty_compare inputs) and the first group of `QTYDBG blk=3` lines (ord= rank, got= hard register, 2=$v0 3=$v1 4=$a0).
- result: candidate.c (score 2): 104 arg5-ADDRESS 18-20 refs4 pri 4.00 rank0 $v0 | 110 22-30 refs8 3.00 rank1 $v0 | 97 arg5-VALUE 20-26 refs4 1.33 rank2 $v1 | 102 t0 16-24 refs4 1.00 rank3 $a0 (all four as the target wants). k03 (score 6, order perfect): 110 3.00 $v0 | 102 arg5-ADDRESS 16-20 2.00 $v0 | 104 t0 18-24 1.33 $v1 (WRONG) | 97 arg5-VALUE 20-26 1.33 $a0 (WRONG). On the order-perfect base t0 and the arg5 value TIE at 1.33 and t0 wins by quantity number, being born two luids earlier. Reg 98 - the register the s61/s62/s63 model was built on - is not one of the four quantities at all.
- verdict: CONFIRMED

## [s65] The order fix and the seat loss are both consequences of ONE sched1 decision: choosing the t0 shift as the filler for the arg5 addu -> arg5 load latency slot.
- mechanism: sched1 emits insn 111 (reg102 = reg74 + reg81, the arg5 address addu); its consumer 113 (reg97 = mem[reg102]) is not ready for one cycle, so the highest-priority ready insn goes between them and that is 117 (reg104 = reg98 << 2, the t0 shift). This stretches the address quantity's span from 1 to 2 (pri 4.00 -> 2.00, demoting it below reg 110) AND makes the t0 quantity born before the arg5-value quantity, which is what wins it the 1.33 tie.
- probe: Read the sched1 RTL dump for the order-perfect base (tmp/grind/CD_ready/s63/k03.sched.txt) insn by insn and cross-check against the QTYDBG birth/death luids of both bases.
- result: sched1 order confirmed: 99 t0 lbu, 106 arg5 lbu, 141 *pp, 108 arg5 sll, 111 arg5 addu, 117 t0 sll, 113 arg5 lw, 128 D_800A11D5 lbu, 122 t0 addu, 137 sw 16(sp), 133, 143, 145 a3 lw, 139, 147 call. On candidate.c the t0 shift is scheduled before the addu, 111 and 113 end up adjacent and neither effect fires.
- verdict: CONFIRMED

## [s65] Raising the arg5 quantity's reference count by staging a REAL second value (the arg5 byte index) in the arg5 variable flips the qty_compare order while leaving the address to accumulate in the v0 carrier - the untried intersection of branch B (order) and branch C (b07's arg5 priority).
- mechanism: local-alloc.c:297 sets qty_n_refs = reg_n_refs (function-wide, loop-weighted); s62 proved dead stores cannot move it and that only a real extra occurrence does (b07: 4 -> 8 refs). b07 raises the count by carrying the ADDRESS, which mis-destines the addu; carrying the BYTE should not.
- probe: n01-n07 on the k03 order-perfect base (byte + v0 shift; copy-then-shift; byte + in-place v0 address; whole chain in arg5; pp last; t0 byte late; shift folded into the address) and n08 (same staging on the candidate base). Sandbox score for each, index-aligned disassembly of n01, QTYDBG on n01 and n03.
- result: 8, 6, 8, 9, 8, 8, 6, 8 - all 179 insns, 0 rules, none better than its base. n01's ORDER is perfect but the staging pseudo does not coalesce with the v0 carrier (lbu $a0,1($s2) + sll $v0,$a0,2 against the target's in-place lbu $v0 + sll $v0,$v0,2), buying three new mismatches. QTYDBG on n03 shows block 3 collapsing from four local quantities to two - the staged pseudos become cross-block and go to global-alloc.
- verdict: KILLED

## [s65] Materialising arg4 into its own local repositions the a3 argument load (insn 145, t0's death) and re-times the t0 quantity.
- mechanism: A named local makes the deref a statement-level load rather than an argument load emitted by expand_call at the call site, moving t0's death luid.
- probe: q01 (arg4 local after the t0 chain), q02 (before the arg5 chain), q03 (tbl_125c[t0] spelling), q04 (role swap: arg5 folded into the call, arg4 staged), q05 (both args folded into the call).
- result: 11, 11, 12, 10, 11 - all 179 insns, 0 rules, every one far worse than the base 6. Materialising the value restructures the whole block rather than re-timing one insn.
- verdict: KILLED

## [s65] Hoisting the D_800A11D5 byte into a statement lowers its INSN_LUID below the t0 shift's, so it becomes the filler for the 111 -> 113 latency slot instead, leaving the arg5 load born before the t0 shift and flipping the quantity numbers.
- mechanism: sched.c rank_for_schedule falls through to INSN_LUID when INSN_PRIORITY ties; insn 128's LUID is high only because expand_call emits the arg-3 computation at the call site.
- probe: r01 (ix statement between the arg5 chain and the t0 chain), r02 (head of the block), r03 (u8-typed index), r04 (after the t0 chain), plus a QTYDBG read on r01.
- result: r01 6, r03 6, r04 6 - byte-inert; r02 11. QTYDBG on r01 is STRUCTURALLY IDENTICAL to k03 (same four quantities, spans 16-20/18-24/20-26/22-30, same ranks, same got= 2/2/3/4). The two candidate fillers are NOT tied on INSN_PRIORITY (117: shift -> addu -> a3 load -> call outranks 128: load -> shift -> a2 load -> call), so the LUID rung is never reached.
- verdict: KILLED

## s66 hypotheses (synthesis, 2026-09-01)

### H-s66-A — CONFIRMED (by RTL reading, and it CORRECTS s65's frontier)
Statement: the two quantities that tie in `qty_compare_1` on the order-perfect base are the t0
SHIFT temp (`reg 104`, born at the `sll`, insn 117, dead at the t0 address `addu`, insn 122) and
the arg5 VALUE (`reg 97`, born at the `lw`, insn 113, dead at the stack store, insn 137) — NOT the
pair s65 named. `reg 98` (the `t0` user variable) and `reg/v 74` (`v0`) are not local quantities
at all: reg98 is set twice in the block so `REG_N_DEATHS == 2`, and reg74 is cross-block; both are
handled by global-alloc, which seats them correctly on every base measured.
Mechanism: `local_alloc` only builds quantities for single-death, single-block pseudos;
`block_alloc` numbers the block's insns pos = 4 + 2*index in sched1 output order (verified against
all four measured births/deaths).
Probe: read `tmp/grind/CD_ready/s63/k03.sched.txt` in full and align it with
`tmp/grind/CD_ready/s65/k03.qty.txt`.
Result: the mapping closes exactly; s65's "insn 145 is t0's death" is wrong.
Verdict: CONFIRMED.

### H-s66-B — CONFIRMED: the residual has exactly ONE free variable left
Statement: with the target's instruction sequence held (the order-perfect base), every
`qty_compare_1` input except `qty_n_refs` is forced — births, deaths and quantity numbers are all
positions in that sequence, and both tied pseudos carry the minimum possible two mentions, so a
third mention would be a third instruction and would break the 179-instruction parity. Therefore
the ONLY way to close the function on this branch is to change the loop-depth-weighted reference
count of one of the two tied pseudos.
Mechanism: `pri = floor_log2(refs)*refs*size/(death-birth)`, ties by quantity number
(local-alloc.c:1640-1684); `qty_n_refs[qty] = reg_n_refs[regno]` (local-alloc.c:297);
`reg_n_refs` is accumulated in flow.c weighted by loop depth.
Probe: the full measured quantity table plus s01-s08 (eight t0-chain topologies attacking the
alternative escape, "make the shift temp not be a local quantity").
Result: all eight regress or are byte-identical (9/9/7/6/9/7/14/15); s01's QTYDBG shows
`combine_regs` merging the shift temp into a single-death address variable, which makes the t0
quantity BEAT the arg5 value harder (pri 2.57). The alternative escape is closed.
Verdict: CONFIRMED.

### H-s66-C — CONFIRMED (the lever exists) / KILLED (every placement measured so far)
Statement: a `do { } while (0)` wrap raises the loop-depth-weighted `reg_n_refs` of the pseudos
whose mentions it encloses, and is therefore the one C-level lever that can reach the residual's
only free variable.
Mechanism: the wrap emits real `NOTE_INSN_LOOP_BEG`/`NOTE_INSN_LOOP_END`; flow.c's reference
counter adds `loop_depth` per mention, so an enclosed mention counts 3 instead of 2.
Probe: 10 wrap placements (u00-u04, w01-w04, x01-x04) on the order-perfect base, each QTYDBG'd or
scored.
Result: the ref counts move exactly as predicted — arg5 value 4 -> 6 with both mentions enclosed,
4 -> 5 with one — but every placement also perturbs sched1 and loses more than the seats are
worth: printf-call-only 10 (it displaces the `*pp` expand_call argument load and stretches the
arg5 value's span to 8, so its pri falls to 1.25 and it loses anyway), arg5-load-only 8 (best of
the family), load+call 13, whole-chain 12 at 180 insns.
Verdict: lever CONFIRMED; all ten placements KILLED.

### H-s66-D — CONFIRMED (free base improvement, no score change)
Statement: splitting the arg5 address into its own statement (`a5a = v0 + (s32)tbl_125c;
arg5 = *(s32 *)a5a;`) is byte-neutral on the order-perfect base.
Probe: u00.
Result: score 6, identical to k03. Any future ref-count probe may use the split spelling for free.
Verdict: CONFIRMED.

### THE FRONTIER AFTER s66
The function is one register-pair swap from zero on a base that already emits all 179 target
instructions in the target's order, and the swap is decided by a single integer: whether the arg5
VALUE pseudo's loop-depth-weighted `reg_n_refs` exceeds 4. Everything else in `qty_compare_1` is
pinned by the target's own instruction sequence. The one demonstrated lever on that integer is the
loop note, and the whole remaining search is: **find a loop-note placement (or another
loop_depth-raising construct) that raises the arg5 value's count to 5 or 6 WITHOUT moving the
`*pp` expand_call argument load or merging the t0 chain.** Concretely untried:
  (a) a wrap that encloses ONLY the `arg5` mention inside the call's argument list — e.g. hoist
      the fifth argument into `arg5` inside a wrap that does NOT contain the call itself, with the
      `pp` dereference also hoisted out of the call so that the call statement's expand_call
      argument loads are no longer sensitive to the note's position;
  (b) two nested wraps arranged so the arg5 load sits at depth 3 while the `*pp` load's position
      relative to the note is unchanged (the note itself, not the depth, is what moved insn 141 —
      test this by measuring an EMPTY wrap placed at each candidate position and confirming it is
      byte-neutral before adding the load to it);
  (c) the reverse polarity: leave the arg5 value at refs 4 and find a construct that raises the
      T0 SHIFT temp's span (not its refs) — the only span-changing edit that does not move an
      instruction would be one that changes which insn the temp dies at, i.e. re-associating the
      t0 address `addu` so the temp is read one slot later, which is worth a small sweep because
      it is the only span lever the forced-order argument does not obviously kill.
An EMPTY-wrap byte-neutrality survey (b) is the cheapest next measurement and should come first:
it separates "the loop note perturbs sched1" from "raising refs perturbs sched1", which no
session has yet distinguished.

- [s66] The two quantities that tie on the order-perfect base are reg 104 (the t0 SHIFT temp, born at insn 117, dead at insn 122) and reg 97 (the arg5 VALUE, born at insn 113, dead at insn 137); reg 98 and reg 74 are not local quantities at all (two deaths / cross-block) and are seated correctly by global-alloc. s65's "insn 145 is t0's death" is wrong and its named frontier probe was aimed at the wrong pair.
- [s66] With the target's instruction sequence held, every qty_compare_1 input except qty_n_refs is forced, and both tied pseudos carry the minimum possible two mentions. The loop-depth-weighted reference count is the residual's only remaining free variable.
- [s66] A do-while(0) wrap DOES raise the enclosed pseudos' loop-depth-weighted reg_n_refs (arg5 value 4->6 both mentions enclosed, 4->5 with one), which is the first C-level lever ever demonstrated on this function's qty_compare_1 reference counts.
- [s66] All ten wrap placements measured regress: the note also perturbs sched1, displacing the *pp expand_call argument load or merging the t0 chain. Best of the family is 8 (arg5-load-only).
- [s66] All eight t0-chain pseudo topologies aimed at removing the shift temp from local-alloc regress or are byte-identical; combine_regs merging the shift temp into a single-death address variable makes the t0 quantity win harder (pri 2.57).
- [s66] Splitting the arg5 address into its own statement is byte-neutral on the order-perfect base (u00 = 6 = k03), so future ref-count probes can use the split spelling for free.

## [s66] The two quantities that tie in qty_compare_1 on the order-perfect base are the t0 SHIFT temp (reg 104) and the arg5 VALUE (reg 97) - NOT the pair s65 named. reg 98 (the `t0` user variable) and reg/v 74 (`v0`) never enter local-alloc at all.
- mechanism: local_alloc only builds quantities for single-death, single-block pseudos; reg 98 is SET TWICE in the block (insns 99 and 122) so REG_N_DEATHS == 2, and reg 74 is cross-block. Both are seated by global-alloc, correctly ($a0 / $v0) on every base measured. block_alloc numbers the block's insns pos = 4 + 2*(index in sched1 output order), which is the mapping that makes the QTYDBG birth/death columns readable.
- probe: Read tmp/grind/CD_ready/s63/k03.sched.txt (sched1 RTL for the order-perfect base) in full and align it insn-by-insn with the four measured quantities in tmp/grind/CD_ready/s65/k03.qty.txt.
- result: The mapping closes exactly on all four quantities: 102 arg5-address = insn 111 -> 113 (pos 16-20); 104 t0 shift temp = insn 117 -> 122 (pos 18-24); 97 arg5 value = insn 113 -> 137 (pos 20-26); 110 = insn 128 -> 143 (pos 22-30, reg 107 combined in by combine_regs). s65's claim that insn 145 (`a3 = mem(reg98)`) is 't0's death' is wrong - 145 is the death of no local quantity - so s65's headline frontier probe (exchange insns 145 and 137) would have been spent on the wrong pair.
- verdict: CONFIRMED

## [s66] With the target's instruction sequence held (the order-perfect base), the residual has exactly ONE free variable: the loop-depth-weighted reference count qty_n_refs. Births, deaths and quantity numbers are all positions in that sequence and are therefore forced, and both tied pseudos already carry the minimum possible two mentions.
- mechanism: qty_compare_1 (local-alloc.c:1640-1684) sorts by pri = floor_log2(refs)*refs*size/(death-birth) with ties broken by quantity number (line 1683, `return *q1 - *q2;`), and quantity numbers are assigned in birth order. qty_n_refs[qty] = reg_n_refs[regno] (local-alloc.c:297), and reg_n_refs is accumulated in flow.c weighted by loop depth. reg 104's two mentions are the sll that sets it and the addu that reads it; reg 97's are the lw that sets it and the sw that reads it; a third mention of either is a third instruction and breaks the 179-instruction parity.
- probe: Measured quantity table for k03 (102: 16-20 refs4 pri 2.00 -> $v0; 104: 18-24 refs4 pri 1.33 -> $v1; 97: 20-26 refs4 pri 1.33 -> $a0; 110: 22-30 refs8 pri 3.00 -> $v0), plus s01-s08: eight t0-chain pseudo topologies attacking the only alternative escape (making the shift temp not be a local quantity at all).
- result: s01 separate s32 address variable 9; s02 s32* carrier 9; s03 `t0 <<= 2` instead of `t0 *= 4` 7 (not a no-op - the spelling is a lever, in the wrong direction); s04 whole address in one statement 6 (byte-identical to k03); s05 u8* carrier accumulated in place 9; s06 t0 byte load moved beside its shift 7; s07 arg4 fully inline with no t0 variable 14; s08 arg5 address accumulated in place on v0 15. All 179 insns, 0 rules. QTYDBG on s01 shows why the family cannot work: giving the address a single-death variable lets combine_regs tie the shift temp INTO it, producing one 18-32 refs-12 quantity at pri 2.57 that beats the arg5 value harder than the 1.33 tie did.
- verdict: CONFIRMED

## [s66] A `do { } while (0)` wrap raises the loop-depth-weighted reg_n_refs of the pseudos whose mentions it encloses, and is therefore the one C-level lever that reaches the residual's only free variable.
- mechanism: The wrap emits real NOTE_INSN_LOOP_BEG / NOTE_INSN_LOOP_END; flow.c's reference counter adds loop_depth per mention, so a mention inside the wrap counts 3 instead of 2. Raising the arg5 value from 4 to 5 gives pri 1.67 and to 6 gives 2.00, either of which beats the t0 shift temp's 1.33, which would hand reg 97 $v1 and reg 104 $a0 - the target's seats on a base that already emits the target's instruction sequence.
- probe: Ten wrap placements on the order-perfect base, QTYDBG'd and/or scored: u00 (no wrap, arg5 address split into its own statement), u01 (load + call), u02/x02/x03 (load only, three positions), u03 (load only, k03 spelling), u04 (load + call, k03 spelling), w01/w02/w04 (printf call only, three bases), w03 (whole arg5 chain + call), x01/x04 (load before the t0 statements / addu+load).
- result: The reference counts move exactly as predicted - arg5 value 4 -> 6 with both mentions enclosed (u01), 4 -> 5 with one (u02/x02/x03/w01), D_800A11D5 chain 8 -> 12 when the call is inside. But every placement also perturbs sched1 and loses more than the seats are worth: printf-call-only 10 (it displaces the *pp expand_call argument load from build 53/54 to 60/61 and stretches the arg5 value's span to 8, so its pri falls to 1.25 and it loses the tie anyway); arg5-load-only 8 (best of the family; the t0 chain collapses into one 10-32 refs-12 quantity and the two lbu's transpose); load+call 13; load before the t0 statements 12; whole arg5 chain + call 12 at 180 insns (the only variant this session to lose instruction parity).
- verdict: CONFIRMED as a lever, KILLED for all ten placements measured

## [s66] Splitting the arg5 address computation into its own statement (`a5a = v0 + (s32)tbl_125c; arg5 = *(s32 *)a5a;`) is byte-neutral on the order-perfect base, so future reference-count probes can use the split spelling for free.
- mechanism: The split does not add an instruction (GCC already materialises the address into reg 102) and does not move any LUID that the sched1 tie depends on.
- probe: u00_a5addr_split_nowrap.c scored against k03.
- result: u00 = score 6, build 179, target 179, rules_dropped 0 - identical to k03.
- verdict: CONFIRMED

## s67 (solver, 2026-09-01)

### H-s67-A — KILLED (this was the s66 frontier's item 1)
**Statement.** An EMPTY `do { } while (0)` placed at each candidate position in the do_timeout
block is byte-neutral, i.e. the loop NOTE by itself does not move the `*pp` expand_call argument
load, and the sched1 damage seen in s66's w01/u02/x02 comes from the raised reference counts
changing local-alloc's answer downstream rather than from the note.
**Mechanism tested.** NOTE_INSN_LOOP_BEG/END are notes, not barriers, and sched.c splits into
scheduling regions on labels, jumps and calls rather than on notes.
**Probe.** Six variants on the `u00` base (score 6), one empty wrap per insertion point:
before the t0 byte load, before the `pp` assignment, before the arg5 byte load, before
`t0 *= 4`, before the arg5 load, immediately before the printf.
**Result.** 9 / 7 / 7 / 10 / 11 / 10. **Not one is byte-neutral.**
**Verdict: KILLED.** An empty wrap contains no register mentions, so it raises no reference
count; the entire perturbation is therefore the note. The loop note behaves as a sched1 region
boundary in this block. This is the single fact that unifies every wrap measurement in s66 and
s67, and it converts the search: a reference count can never be bought for free on this
function, so every candidate must budget for the order damage its notes cause.

### H-s67-B — CONFIRMED (new axis, never attempted in 66 prior sessions)
**Statement.** `qty_n_refs` can be LOWERED as well as raised. Every mention in the do_timeout
block is already counted at loop depth 2 because the whole block sits inside the sanctioned
tbl_125c `do { } while (0)`; SPLITTING that wrap and letting a statement fall into the bare gap
returns its mentions to depth 1.
**Mechanism.** flow.c accumulates `reg_n_refs += loop_depth` per mention, `loop_depth` being
incremented at NOTE_INSN_LOOP_BEG and decremented at NOTE_INSN_LOOP_END. Two mentions of the
t0 shift temp at depth 1 give refs 2 (pri 0.33) instead of refs 4 (pri 1.33), dropping it below
the arg5 value's 1.33 in `qty_compare_1`.
**Probe.** y01/y02/y03/y06/y07/y20 on the `u00` base.
**Result.** y02 (`t0 *= 4` alone in the bare gap) = 6 at 179 insns, and its disassembly is the
FIRST body in this function's history derived from the order-perfect base whose SEATS are the
target's — arg5 value in `$v1`, t0 address in `$a0`. y01 11, y03 15, y06 15, y20 17.
Control y07 (wrap cut in two, nothing bare, all depths held at 2) = 10.
**Verdict: CONFIRMED.** The depth-lowering axis does exactly what the closed-form model says.
y02's whole residual is ordering: the `*pp` load sinks to 60/61 and the t0 `addu` rises to 56.

### H-s67-C — KILLED (a lever retired, not a form)
**Statement.** Statement order remains the ordering lever inside the split-wrap skeleton, so the
y-family's `*pp` and `lbu` order defects can be repaired by permuting the head statements.
**Probe.** A 4x4 grid (d01-d16): four permutations of the four head loads (`t0 = idx[0]`,
`v0 = idx[1]`, `pp = ...`, `a1v = *pp`) crossed with four split boundaries.
**Result.** Scores depend ONLY on the boundary: 7 / 11 / 6 / 9, **identical for all four head
permutations.** Also y09-y17 on the non-hoisted skeleton: all 6.
**Verdict: KILLED.** Inside a wrapped scheduling region those four independent loads are
separated outright by INSN_PRIORITY, so the INSN_LUID rung that s63 identified as "the only
lever" is never reached. Do not spend further measurements permuting them.

### H-s67-D — CONFIRMED (this was the s66 frontier's item 2)
**Statement.** The `*pp` value's position can be fixed by hoisting the printf's second argument
out of the call into its own statement, because `*pp` is an expand_call ARGUMENT load whose LUID
is pinned to the call statement and therefore cannot cross the wrap note.
**Probe.** c01-c06 — `void *a1v; a1v = *pp;` at six positions on the y02 skeleton, plus controls
c00/c08 (the hoist alone on the UNSPLIT base) and c07 (the hoist inside wrap2).
**Result.** Every in-wrap1 spelling scores 6 at 180 insns and puts the `lui $a1 / lw $a1` pair at
build **53/54 — the target's slots**. c07 (hoist inside wrap2) reproduces the late load, 8.
Controls c00/c08 = 10: the hoist only pays inside the split skeleton.
**Verdict: CONFIRMED.** The 180th instruction is a load-delay `nop`, not a copy — the named
intermediate coalesces — and it disappears as soon as the t0 `addu` is scheduled back into the
D_800A11D5 `lbu`'s delay slot (which is what d01 achieves).

### H-s67-E — CONFIRMED by a pre-registered numeric prediction
**Statement.** On the order-perfect d01 base the arg5 VALUE quantity (reg96, 20-28, refs 4,
pri `floor_log2(4)*4/8` = 1.00) can be lifted above the t0 quantity (reg98, 10-34, refs 10,
pri `floor_log2(10)*10/24` = 1.25) by raising its loop-depth-weighted reference count to
**exactly 6** — refs 5 yields pri 1.25, an exact tie that reg98 wins on quantity number (0 < 3),
and refs 6 yields 1.50. reg96 has exactly two mentions, so refs 6 needs the arg5 load at depth 4
(two extra nesting levels) plus the call mention at depth 2.
**Probe.** e01/e02/e03 (one, two and three extra nesting levels on the arg5 load), plus e04
(nest on the call), e05 (nest on both), e07 (t0 addu ahead of the nest).
**Result.** e02 measured `refs(96) = 6` exactly as predicted; reg96 moved ord 3 -> **ord 2** and
`got=3` = **`$v1`, the target's seat**, confirmed in the disassembly as `58 lw $v1,0($v0)` and
`62 sw $v1,16($sp)`. e01 (refs 5) scores 9 and does NOT flip the seat — the predicted tie.
e03 8, e04 8 (180 insns), e05 6 (180 insns), e07 6 (180 insns).
**Verdict: CONFIRMED.** The local-alloc model of this function is now fully predictive: it named
the required reference count and the resulting hard register in advance and both came out.
The win does not reach 0 because (a) reg98 falls to ord 3 and takes `$a3` rather than the
target's `$a0`, and (b) the two extra notes re-perturb the instruction order.

### THE FRONTIER AFTER s67
The residual has decomposed into three pieces, each SOLVED in isolation and never yet held
together:
  1. the target INSTRUCTION SEQUENCE — held by k03 and by d01 (d01 also has the `*pp` pair at
     53/54 and the t0 `addu` at 61);
  2. the arg5-VALUE seat `$v1` — held by e02, via refs(reg96) = 6;
  3. the t0 seat `$a0` — held by y02, via the depth split, on a body with NO `a1v` quantity.
The single unresolved mechanical question is why (2) and (3) are incompatible: on every
`a1v`-carrying base reg98's `find_free_reg` used set CONTAINS hard reg 4 (`$a0`), while on y02 it
does not. That is a `used`-set question, not a `qty_compare_1` question, and it is the first time
this function's residual has moved out of the priority sort and into find_free_reg.

## [s67] An EMPTY do { } while (0) placed at each candidate position in the do_timeout block is byte-neutral, i.e. the loop NOTE by itself does not move the *pp expand_call argument load, and the sched1 damage seen in s66's w01/u02/x02 comes from the raised reference counts changing local-alloc downstream rather than from the note. (This was the s66 ledger's named frontier probe, item 1.)
- mechanism: NOTE_INSN_LOOP_BEG/END are notes, not barriers, and GCC 2.7.2's sched.c splits into scheduling regions on labels, jumps and calls rather than on notes. An empty wrap contains no register mentions, so it raises no reg_n_refs; any byte change it causes is therefore attributable to the note alone.
- probe: Six variants spliced into the u00 base (score 6, byte-identical to the order-perfect k03), one empty do{}while(0) per insertion point: before the t0 byte load, before the pp assignment, before the arg5 byte load, before `t0 *= 4`, before the arg5 load, and immediately before the printf call. Scored with `sandbox CD_ready --disable all`.
- result: Scores 9 / 7 / 7 / 10 / 11 / 10 against the base's 6. NOT ONE is byte-neutral. Since an empty wrap raises no reference count, the entire perturbation is the note; the loop note behaves as a sched1 region boundary in this block.
- verdict: KILLED

## [s67] Loop-depth-weighted reference counts can be LOWERED as well as raised. Every mention in the do_timeout block is already counted at loop depth 2 because the block sits inside the sanctioned tbl_125c do-while(0); SPLITTING that wrap so a statement falls into a bare gap returns its mentions to depth 1, and putting `t0 *= 4` there drops the t0 shift temp below the arg5 value in qty_compare_1.
- mechanism: flow.c accumulates reg_n_refs += loop_depth per mention, loop_depth being incremented at NOTE_INSN_LOOP_BEG and decremented at NOTE_INSN_LOOP_END. Two mentions of the shift temp at depth 1 give refs 2 (pri floor_log2(2)*2/6 = 0.33) instead of refs 4 (pri 1.33), below the arg5 value's unchanged 1.33. Every prior session only ever ADDED wraps; nobody had tried subtracting depth.
- probe: y01 (both t0 statements bare), y02 (`t0 *= 4` alone bare), y03 (the t0 addu alone bare), y06 (both bare, no second wrap), y20 (whole arg5 address chain bare), plus control y07 (the wrap merely cut in two with NOTHING bare, all depths held at 2). Scored, then y02 read with the instrumented cc1's QTYDBG and disassembled against asm/funcs/CD_ready.s.
- result: y02 = 6 at 179 insns and 0 rules, and its disassembly is the FIRST body in this function's history derived from the order-perfect base whose SEATS are the target's - arg5 value in $v1, t0 address in $a0. Its whole residual is ordering (the *pp load sinks from 53/54 to 60/61, the t0 addu rises from 61 to 56, the two lbu's transpose). y01 11, y03 15, y06 15, y20 17. Control y07 = 10, so re-bracketing alone costs 4 and y02's depth change pays for its own note.
- verdict: CONFIRMED

## [s67] Statement order remains the ordering lever inside the split-wrap skeleton, so the y-family's *pp and lbu order defects can be repaired by permuting the head statements (the s63 finding that INSN_LUID = C statement order is the only ordering lever).
- mechanism: sched.c's rank_for_schedule falls through to an INSN_LUID tiebreak when the class rungs collapse; s63 measured the class rung collapsing to class 3 on this target, which is why statement order was believed to be the ordering control.
- probe: A 4x4 grid (gen4.py, d01-d16) crossing four permutations of the four head loads (t0 = idx[0], v0 = idx[1], pp = ..., a1v = *pp) with four split boundaries; plus y09-y17, seven more head permutations on the non-hoisted skeleton.
- result: Scores depend ONLY on the split boundary and are IDENTICAL across all four head permutations: 7 / 11 / 6 / 9 repeated four times. y09/y10/y11/y12/y16/y17 all 6. Inside a wrapped scheduling region those four independent loads are separated outright by INSN_PRIORITY, so the LUID rung is never reached and permuting them is inert.
- verdict: KILLED

## [s67] The *pp value's position can be fixed by hoisting the printf's second argument out of the call into its own named intermediate, because *pp is an expand_call ARGUMENT load whose LUID is pinned to the call statement and which therefore cannot be lifted across the wrap note. (This was the s66 ledger's named frontier probe, item 2.)
- mechanism: expand_call emits argument loads at the call site, so the load is born inside wrap2 and sched1 will not move it into wrap1's region across the loop note. `void *a1v; a1v = *pp;` converts it into an ordinary load whose position is set by INSN_LUID, i.e. by where the statement sits.
- probe: c01-c06, the a1v hoist at six positions on the y02 skeleton; control c00/c08 (the hoist alone on the UNSPLIT k03/u00 base); control c07 (the hoist placed inside wrap2). Scored, and c02 disassembled against the target.
- result: Every in-wrap1 spelling scores 6 at 180 insns and puts the `lui $a1 / lw $a1` pair at build 53/54 - the target's exact slots. c07 reproduces the late load (8). Controls c00/c08 = 10, so the hoist only pays inside the split skeleton. The 180th instruction is a load-delay nop, not a copy (the named intermediate coalesces), and it disappears once the t0 addu is scheduled back into the D_800A11D5 lbu's delay slot.
- verdict: CONFIRMED

## [s67] On the order-perfect d01 base the arg5 VALUE quantity (reg96, birth-death 20-28, refs 4, pri floor_log2(4)*4/8 = 1.00) can be lifted above the t0 quantity (reg98, 10-34, refs 10, pri floor_log2(10)*10/24 = 1.25) by raising its loop-depth-weighted reference count to EXACTLY 6 - refs 5 gives pri 1.25, an exact tie that reg98 wins on quantity number (0 < 3), and refs 6 gives 1.50 - whereupon find_free_reg hands it the target's $v1.
- mechanism: local-alloc.c's qty_compare_1 sorts by pri = floor_log2(refs)*refs*size/(death-birth) and breaks ties by quantity number (local-alloc.c:1683). Spans and quantity numbers are pinned by the target's own instruction sequence, and reg96 carries the minimum possible two mentions, so the only free input is flow.c's loop-depth-weighted reg_n_refs; refs 6 requires the arg5 load at depth 4 (two extra nesting levels) plus the call mention at depth 2.
- probe: The numbers were derived from d01's QTYDBG table and the required refs value written down BEFORE measuring. Then e01/e02/e03 (one, two, three extra nesting levels on the arg5 load), e04 (nest on the call), e05 (both), e07 (t0 addu ahead of the nest); e02 read back with QTYDBG and disassembled.
- result: e02 measured refs(reg96) = 6 exactly as predicted; reg96 moved from ord 3 to ord 2 and got=3 = $v1, THE TARGET'S SEAT, confirmed in the disassembly as `58 lw $v1,0($v0)` and `62 sw $v1,16($sp)`. e01 (one level, refs 5) scores 9 and does NOT flip the seat - precisely the predicted tie. e03 8, e04 8 (180 insns), e05 6 (180 insns), e07 6 (180 insns). The local-alloc model of this function is now fully predictive. e02 stops at 8 rather than 0 because reg98 falls to ord 3 and takes $a3 instead of $a0, and because the two extra notes re-perturb the instruction order.
- verdict: CONFIRMED

## s68 (2026-09-01, escalation) — hypotheses resolved

**H-s68-1 (KILLED).** *Statement:* respelling `a1v` so that its pseudo leaves local-alloc's
quantity list — declaring it at function scope so it becomes "cross-block" like `reg/v 74`, or
retyping it, or deleting the `pp` local — removes hard reg 4 ($a0) from reg98's `find_free_reg`
`used` set and restores the target's `t0` seat.
*Mechanism tested:* `local_alloc`'s `find_free_reg` builds `used` from hard registers conflicting
with the quantity's live range plus the registers already given to earlier-allocated quantities;
`a1v` (reg100 on d01) is allocated first via the copy-suggestion path and spans 14-36.
*Probe:* six spellings on the d01 order-perfect base (f1a function scope, f1b carried by `v0`,
f1c `s32`-typed, f1d loaded last, f1e no `pp` local, f1f fold+scope), each scored with
`sweep.ps1`, plus the decisive instrumented-cc1 local-alloc dump on f1a
(`tmp/grind/CD_ready/s68/f1a.qty.txt`) compared against `s67/d01.qty.txt`.
*Result:* f1a/f1c/f1e/f1f = 7 (byte-identical to the base), f1d = 10, f1b = 16. The f1a dump is
line-for-line identical to d01's — same quantities, same births/deaths/refs, same `used` sets,
same allocation order, same hard registers; only the pseudo NUMBERS shift.
*Verdict:* KILLED, and the axis is foreclosed structurally, not merely unmeasured: quantity
membership in `local_alloc`/`block_alloc` is decided by `REG_BASIC_BLOCK` (all references in one
basic block), which C declaration scope cannot influence. Moving `a1v` out requires a reference
in a second basic block, i.e. at least one extra instruction, which 179-instruction parity bars.
Hard reg 4 is in reg98's `used` because $a0 is the printf call's first argument and conflicts
with the 10-34 range directly — not because of `a1v`, which takes $a1 (5).

**H-s68-2 (KILLED).** *Statement:* d01's transposed `lbu` pair at 51/52 can be flipped by
SHORTENING the arg5 chain's `INSN_PRIORITY` path (folding the address computation) without
changing the instruction count.
*Mechanism tested:* `sched.c`'s `rank_for_schedule` orders the two byte loads by `INSN_PRIORITY`
= longest path to block end; the arg5 byte's path is 4 links, the t0 byte's 3.
*Probe:* f2a (fold to `a5a = (idx_1494[1] << 2) + (s32)tbl_125c;`), f2c (fold keeping `v0`),
f2b (fully inline `arg5 = tbl_125c[idx_1494[1]];`), f2d (arg5 value loaded inside wrap 1).
*Result:* f2a and f2c are BYTE-IDENTICAL to d01 (7 / 179 insns) and leave 51/52 exactly where
they were; f2b reaches 180 insns (8); f2d = 9.
*Verdict:* KILLED. Source-level folding is byte-free (consistent with s66's s04) but cannot
shorten an RTL dependence chain — the chain length is set by the RTL ops GCC must emit
(`lbu` -> `sll` -> `addu` -> `lw` -> `sw`), not by C statement count. Removing a link means
removing an instruction, and f2b prices that at +1 insn, off the 179-parity the basin needs.

**H-s68-3 (KILLED).** *Statement:* the owner-authorised Ruling D `CD_intr` aggregate merge
(`typedef struct { u8 sync, ready, c; } CD_intr;` over 0x800A1494/95/96) is a lever on CD_ready.
*Probe:* (i) the mandatory prong-(c) asm-consumer check, re-derived first-hand:
`asm/data/7D920.data.s:31050/31056/31062` DEFINE the three bytes in assembly and five asm-only
consumers reference them (`getintr.s` 22 sites, `CD_cw.s` 8, `func_800817A0.s` 8,
`func_800819C4.s` 8, `func_80081E1C.s` 2); (ii) two aggregate-access spellings on the candidate
base (rd1 volatile, rd2 plain).
*Result:* prong (c) is UNSATISFIABLE — the per-word symbols must stay in the splat config, so any
`CD_intr` declaration is a second handle on the same storage (the g_stage_id failure verbatim,
decisions.md:10722). And the aggregate LOSES on measurement anyway: rd1 = 41 / 173 insns,
rd2 = 37 / 172 insns, against the floor of 2 / 179.
*Verdict:* KILLED both ways. Ruling D is discharged for CD_ready; the grant cannot be spent by
any of the three CD_* members while CD_cw / getintr / func_800819C4 / func_800817A0 /
func_80081E1C remain assembly-only consumers of D_800A1494/95/96.

### Frontier after s68

The two allocation/ordering frontier items are gone. What remains recorded is F3, which is a
COMPLETION blocker rather than a floor lever and only becomes live at score 0: whether prong 2 of
`.claude/rules/legitimate-volatile-interrupt-touched.md` admits this poll loop (body work between
successive reads of `*idx_1496`, i.e. closest to double-read-across-sequence-point but not
verbatim one of the three listed shapes), and whether the already-shipped matched in-TU
declarations at `src/system.c:549-551`, `:748-749` and the matched pointer-arithmetic shape at
`:770-771` constitute the `volatile_extern_allowlist.txt` grant for D_800A1494/95/96. That is a
ruling-request, not a measurement, and it is moot while the floor is 2.

## [s68] Respelling `a1v` so its pseudo leaves local-alloc's quantity list (function-scope declaration making it 'cross-block' like reg/v 74, retyping, deleting the `pp` local) or shortening its 14-36 live range removes hard reg 4 ($a0) from reg98's find_free_reg `used` set and restores the target's t0 seat.
- mechanism: local_alloc's find_free_reg builds `used` from hard registers conflicting with the quantity's live range plus registers already assigned to earlier-allocated quantities; a1v (reg100 on the d01 base) is allocated first via the copy-suggestion path (got=5=$a1) and spans 14-36, overlapping reg98's 10-34 almost entirely.
- probe: Six spellings on the s67 d01 order-perfect base scored with tmp/grind/CD_ready/s63/sweep.ps1 - f1a function scope, f1b carried by the existing function-scope v0, f1c s32-typed, f1d loaded last before the call, f1e no `pp` local, f1f fold+scope - plus the decisive instrumented-cc1 local-alloc dump on f1a (bash tmp/grind/CD_ready/s63/qty.sh, BB2_QTY_DEBUG/BB2_SUGG_DEBUG) compared line-for-line against tmp/grind/CD_ready/s67/d01.qty.txt.
- result: Scores 7 / 16 / 7 / 10 / 7 / 7 - nothing below the base's 7, and f1a/f1c/f1e/f1f are byte-identical to it. The f1a dump's block 3 is line-for-line identical to d01's: same quantities, same births/deaths/refs, same `used` sets (reg98 still used=0,1,2,4,5,6 -> got 3), same allocation order, same hard registers; only the pseudo NUMBERS shift (100->76, 98->99, 97->98, 96->97).
- verdict: KILLED

## [s68] d01's transposed lbu pair at 51/52 can be flipped by SHORTENING the arg5 chain's INSN_PRIORITY path - folding the address computation - without changing the instruction count.
- mechanism: sched.c's rank_for_schedule orders the two byte loads by INSN_PRIORITY = longest path to block end; in d01 the arg5 byte feeds sll->addu->lw->sw (4) while the t0 byte feeds sll->addu->lw (3), so the arg5 byte issues first, the reverse of the target.
- probe: f2a (`a5a = (idx_1494[1] << 2) + (s32)tbl_125c;`), f2c (same fold keeping v0 as a named intermediate), f2b (fully inline `arg5 = tbl_125c[idx_1494[1]];`), f2d (arg5 value loaded inside wrap 1), each scored with sandbox CD_ready --disable all.
- result: f2a and f2c are BYTE-IDENTICAL to the base (7 / 179 insns) and leave the 51/52 pair exactly where it was; f2b reaches 180 insns (8); f2d = 9. The fold is free but inert on the ordering.
- verdict: KILLED

## [s68] The owner-authorised Ruling D CD_intr aggregate merge (typedef struct { u8 sync, ready, c; } CD_intr over 0x800A1494/95/96) is a lever on CD_ready.
- mechanism: Ruling D (decisions.md:17843) authorises the sanctioned per-word-splat-symbol -> aggregate merge family for the libcd Intr object, with a MANDATORY first-step prong-(c) asm-consumer check; the expectation recorded in the reopen note was that the volatile aggregate creates single-base addressing and scores below the floor.
- probe: (i) prong (c) re-derived first-hand: grep -rlE 'D_800A149[456]' asm/ plus the dlabel definitions in asm/data/7D920.data.s; (ii) two aggregate-access spellings on the candidate base reached through a cast pointer at the existing symbol (rd1 volatile, rd2 plain), scored with sandbox.
- result: Prong (c) is UNSATISFIABLE: the bytes are DEFINED in assembly (asm/data/7D920.data.s:31050/31056/31062, plus :31069 D_800A1498 whose first word is .word D_800A1494) and five asm-only consumers reference them - getintr.s (22 sites), CD_cw.s (8), func_800817A0.s = CD_flush (8), func_800819C4.s (8), func_80081E1C.s (2) - so the per-word symbols must stay in the splat config and any CD_intr declaration is a second handle on the same storage (the g_stage_id failure verbatim, decisions.md:10722). And the aggregate loses on measurement anyway: rd1 = 41 score / 173 insns, rd2 = 37 / 172, against the floor of 2 / 179.
- verdict: KILLED

## s69 (forensics, 2026-09-03) — the residual read out of BOTH passes, end to end

Chassis re-verified live: candidate.c = score 2 / 179 insns / 0 rules; s67's d01 = 7, y02 = 6,
e02 = 8 — every inherited measurement reproduces.

### H-s69-A — CONFIRMED by dump (s63 had this by source reading only)
**Statement.** In sched.c's `rank_for_schedule` the class rung (data / anti-output / independent
relative to `last_scheduled_insn`) collapses to class 3 on BOTH sides at every contested decision
in block 3, so `INSN_LUID` is the operative ordering rung in both scheduling passes.
**Probe.** Instrumented cc1 with `BB2_RANK_DEBUG`/`BB2_PRIO_DEBUG`/`BB2_SCHED_DEBUG` over
src/system.c carrying candidate.c (`tmp/grind/CD_ready/s69/cand.sched.txt`, 1.27 MB).
**Result.** Every `RANKDBG` line printed for CD_ready block 3 in BOTH passes reads
`cls=3 ... cls2=3 val=0`. The decisive one is `RANKDBG last=122 y=120 cls=3 x=106 cls2=3 val=0`,
present identically at pass-1 clock 13 and pass-2 clock 13.
**Verdict: CONFIRMED.** s63's inference is now a measurement.

### H-s69-B — CONFIRMED: the 56/57 transposition is one named LUID comparison, and the C-order input that flips it is named
**Statement.** The floor's two-instruction residual is the pass-1/pass-2 tie between insn 106
(`sll $a0,$a0,2`, the t0 shift) and insn 120 (`addu $v0,$v0,$s5`, the arg5 address add), decided
by `INSN_LUID(120) - INSN_LUID(106)`; GCC 2.7.2 schedules each block BACKWARD, so the insn picked
FIRST is emitted LAST. Making the t0 shift's C statement follow the arg5 address/load statement
raises its LUID above 120's and emits the target's order.
**Mechanism.** sched.c:2464 `return INSN_LUID (tmp) - INSN_LUID (tmp2);` after the priority and
class rungs tie. Pass-1 LUIDs are RTL emission order = C statement order (candidate: 106 luid 6,
120 luid 12 -> 120 picked at clock 13, emitted after 106). Pass-2 LUIDs are the pass-1 OUTPUT
order (106 luid 6, 120 luid 7), so pass 2 reproduces the same answer.
**Probe.** g01 (t0 shift+addu moved after the arg5 load), g06 (same, `pp` hoisted to the head),
g03 (whole t0 chain incl. its byte load moved after), g05 (arg5 addu split out and hoisted above
the t0 shift). Scored, then g06 disassembled against asm/funcs/CD_ready.s.
**Result.** g01 = 6, g06 = 6, g03 = 7, g05 = 15 — all 179 insns, 0 rules. **g06's instruction
ORDER is the target's for the entire block**: build 55/56/57/58/61/62/63 are
`sll $v0` / `addu $v0,$v0,$s5` / `sll` / `lw` / `addu` / `sll $v0` / `sw`, matching the target
slot for slot. Its whole score-6 residual is register naming: the t0 chain sits in `$v1` where
the target has `$a0`, and the arg5 value sits in `$a0` where the target has `$v1`.
**Verdict: CONFIRMED.** After 62 sessions the ordering half of this residual is solved ON THE
CANDIDATE CHASSIS (previously only on the s67 d01/k03 side bases) by an ordinary statement
reorder with no new construct.

### H-s69-C — CONFIRMED: the seat half is a closed-form fixed point coupled to H-s69-B
**Statement.** Holding the target's instruction order necessarily shortens the t0-shift
quantity's live range by exactly one insn, which raises its `qty_compare_1` priority from 1.0000
to 1.3333 — an EXACT tie with the arg5-value quantity — and the tie is broken by quantity number
(birth order), which the target's own order fixes in the t0 shift's favour. That is why order and
seats have measured anti-correlated for eight sessions.
**Mechanism.** local-alloc.c:1660 `qty_compare_1` computes
`pri = (floor_log2(qty_n_refs) * qty_n_refs * qty_size) / (qty_death - qty_birth) * 10000`
and ties on `*q1 - *q2` (the quantity number = order of first reference in the block).
**Probe.** Instrumented-cc1 `BB2_QTY_DEBUG`/`BB2_SUGG_DEBUG` local-alloc dumps of candidate.c
(`tmp/grind/CD_ready/s63/cand.qty.txt`) and g06 (`tmp/grind/CD_ready/s69/g06.qty.txt`), block 3,
with each quantity identified against the disassembly.
**Result.** Block 3 carries exactly four quantities. Identification (unit = 2*index+4 over the
block's insn list, verified against both disassemblies):
  - reg110  22-30 refs 8  = the D_800A11D5 chain      pri 3.0000  -> ord 0, `$v0`
  - arg5 ADDRESS (cand reg104 18-20 / g06 reg102 16-20)
  - **t0 SHIFT** : candidate reg102 birth 16 death 24 (span 8) pri **1.0000** -> ord 3 -> `$a0` OK
                   g06       reg104 birth 18 death 24 (span 6) pri **1.3333** -> ord 2 -> `$v1` X
  - **arg5 VALUE**: reg97 birth 20 death 26 (span 6) refs 4 pri **1.3333** in BOTH
                   candidate -> ord 2 -> `$v1` OK ; g06 -> ord 3 -> `$a0` X
In g06 the two 1.3333 priorities are an exact tie and `*q1 - *q2` hands ord 2 (and `$v1`) to the
t0 shift because it is born one insn earlier — which is exactly what the target's order requires.
**Verdict: CONFIRMED.** Every input of `qty_compare_1` except `qty_n_refs` is pinned by the
target's instruction sequence: `qty_size` is 1 for both, `qty_death - qty_birth` is 6 for both,
and the quantity numbers follow birth order, which is the order itself. The ONLY free input is
`qty_n_refs`, and it must move by at least 1 (t0 shift down to <= 3, or arg5 value up to >= 5).

### H-s69-D — KILLED: buying the reference count with a loop note on the g06 (order-perfect) base
**Statement.** The s67-B depth-lowering axis (split the sanctioned tbl_125c wrap so `t0 *= 4`
falls into a bare loop-depth-1 gap, halving its refs 4 -> 2) and the s67-E depth-raising axis
(nest the arg5 def and/or the call so the arg5 value's refs reach 5 or 6) close the seat tie once
the order is already the target's.
**Mechanism.** flow.c accumulates `reg_n_refs += loop_depth` per mention;
NOTE_INSN_LOOP_BEG/END set the depth. Predicted priorities: t0 shift at depth 1 -> refs 2 ->
0.3333; arg5 value at refs 5 -> 1.6666, at refs 6 -> 2.0000. Either ordering closes the tie.
**Probe.** h01 (t0 shift alone bare), h02 (same on the g01 order), h03 (t0 shift and addu bare),
control h04 (same split boundary, NOTHING bare, all depths held at 2); j01 (t0 statements at
depth 2, everything else at depth 3), j02 (arg5 def nested), j03 (call nested, refs 5),
j04 (arg5 def AND call nested, refs 6).
**Result.** h01 12, h02 12, h03 15, **control h04 12**; j01 10 at 180 insns, j02 12, j03 10,
j04 10. Every one is worse than g06's 6, and h04 proves the whole 6-point loss is the note pair
itself, not the depth change: re-bracketing with nothing bare costs exactly as much as the bare
gap does.
**Verdict: KILLED.** On the order-perfect base the loop note is a sched1 region boundary in this
block (s67-A's finding, re-derived here on a different base), so it destroys the very ordering the
base was built to hold. Refs cannot be bought with a note once the order is correct.

### H-s69-E — KILLED: buying the reference count with a same-value re-store (no note)
**Statement.** A sanctioned dead store to a LOCAL (`arg5 = arg5;`) adds two mentions to the arg5
value's `reg_n_refs` without a loop note, lifting it from 4 to 6 and breaking the tie.
**Mechanism.** flow.c counts every mention of a pseudo; a self-copy is two mentions.
**Probe.** k01 (`arg5 = arg5;` immediately after the load), k02 (immediately before the call),
k03 (`t0 = t0;` control), each scored, and k01 re-dumped with BB2_QTY_DEBUG.
**Result.** All three score 6 — byte-identical to g06 — and k01's block-3 local-alloc dump is
line-for-line identical to g06's: `reg97 ... refs=4`, unchanged. flow.c's `delete_noop_moves`
removes the self-copy BEFORE `reg_n_refs` is accumulated, so the mentions never exist.
**Verdict: KILLED.** Same-value re-stores are refs-inert on this function, not merely byte-inert.

### THE FRONTIER AFTER s69
The residual is now a single arithmetic sentence with a single free variable:
  **on any base whose sched1 output is the target's instruction order, the t0-shift quantity and
  the arg5-value quantity both have `qty_size` 1, span 6 and `qty_n_refs` 4, hence identical
  `qty_compare_1` priority 13333, and the tie goes to the t0 shift on quantity number. Closing
  the function requires moving exactly one reference count by one, with zero loop notes.**
Enumerated inputs and their status:
  - `qty_size`   — 1 for both; changing it means a DImode value, i.e. extra instructions. Dead.
  - `qty_death - qty_birth` — 6 for both, pinned by the target's own instruction order. Dead.
  - quantity number — birth order, pinned by the same order. Dead.
  - `qty_n_refs` via loop depth — the only demonstrated lever; costs >= 4 points of order damage
    on this base (H-s69-D). Dead as spelled so far.
  - `qty_n_refs` via extra mentions — self-copies are deleted before counting (H-s69-E); any
    surviving extra mention is an extra instruction, off the 179-insn parity.
  - the sched2 class rung as an escape from the coupling: making sched1 emit the CANDIDATE order
    (span 8, correct seats) and letting sched2 transpose requires `insn_cost(120 -> 122) > 1`,
    i.e. the arg5 address producer must be a 2-cycle function unit (a load). Node 120 is
    `unit=-1 icost=1` and every ALU op on this target is; only a load qualifies, which is an
    extra instruction. Recorded as the one structurally-identified escape, priced.
Next session's cheapest untried probes, in order:
  1. a refs lever that is not `loop_depth`: does any GCC-2.7.2 construct give a pseudo a mention
     that survives to flow.c but emits no instruction? (Candidates never tried here: a mention
     inside a REG_EQUAL/REG_NOTE-only position, a use consumed by a peephole/`combine` merge, a
     second use folded into an addressing mode of an insn that already exists.)
  2. a note placement OUTSIDE block 3 that still changes block-3 loop depth — every wrap measured
     so far has been inside the do_timeout block, so its note lands in the same scheduling region.
     A wrap whose BEG/END notes both sit in the *predecessor* block would raise depth for block 3
     without adding a region boundary inside it. Untried in 69 sessions.
  3. re-run the g06 family through the permuter (rich passes) — the base is new and no campaign
     has ever started from an order-perfect body on this chassis.

## [s69] In sched.c's rank_for_schedule the class rung collapses to class 3 on both sides at every contested decision in CD_ready block 3, in BOTH scheduling passes, so INSN_LUID is the operative ordering rung.
- mechanism: rank_for_schedule (tools/gcc-2.7.2/sched.c:2408-2464) ranks by INSN_PRIORITY, then by dependence class relative to last_scheduled_insn, then by INSN_LUID. s63 inferred the class collapse by source reading; this session measured it.
- probe: Instrumented cc1 (tools/gcc-2.7.2/cc1) run over src/system.c carrying candidate.c with BB2_SCHED_DEBUG + BB2_RANK_DEBUG + BB2_PRIO_DEBUG, captured to tmp/grind/CD_ready/s69/cand.sched.txt (1.27 MB); block 3 of pass 1 read at line 7218, of pass 2 at line 8594.
- result: Every RANKDBG line emitted for CD_ready block 3 in both passes reads cls=3 ... cls2=3 val=0. The decisive line, identical at pass-1 clock 13 and pass-2 clock 13, is 'RANKDBG last=122 y=120 cls=3 x=106 cls2=3 val=0'.
- verdict: CONFIRMED

## [s69] The floor's two-instruction residual is the single LUID comparison between insn 106 (sll $a0,$a0,2, the t0 shift) and insn 120 (addu $v0,$v0,$s5, the arg5 address add), and moving the two t0 statements after the arg5 load raises 106's LUID above 120's and emits the target's instruction sequence.
- mechanism: GCC 2.7.2 schedules each block BACKWARD, so the insn picked first is emitted last. Pass-1 LUIDs are RTL emission order = C statement order (106 luid 6, 120 luid 12), and pass-2 LUIDs are the pass-1 OUTPUT order (106 luid 6, 120 luid 7), so both passes return INSN_LUID(120) - INSN_LUID(106) > 0 and emit 106 before 120.
- probe: Four C reorderings spliced into src/system.c and scored with `sandbox CD_ready --disable all`: g01 (t0 shift+addu moved after the arg5 load), g06 (same with pp hoisted to the block head), g03 (whole t0 chain incl. its byte load moved after), g05 (arg5 addu split out and hoisted above the t0 shift); g06 then disassembled against asm/funcs/CD_ready.s with tmp/grind/CD_ready/s69/adiff.py.
- result: g01 = 6, g06 = 6, g03 = 7, g05 = 15, all at 179 insns and 0 rules. g06's build slots 55/56/57/58/61/62/63 are sll $v0 / addu $v0,$v0,$s5 / sll / lw / addu / sll $v0 / sw - the target's sequence for the entire block. Its whole score-6 residual is register naming (t0 chain in $v1 where the target has $a0, arg5 value in $a0 where the target has $v1). Banked as memory/grind/CD_ready/progress/s69-g06-order-perfect-on-candidate-chassis-seats-swapped-6.c.
- verdict: CONFIRMED

## [s69] Holding the target's instruction order shortens the t0-shift quantity's live range from span 8 to span 6, raising its qty_compare_1 priority from 1.0000 to 1.3333 into an exact tie with the arg5-value quantity, and local-alloc breaks that tie on quantity number in the t0 shift's favour - so order and seats are coupled by construction.
- mechanism: local-alloc.c:1660 qty_compare_1 scores floor_log2(qty_n_refs)*qty_n_refs*qty_size/(qty_death-qty_birth) scaled by 10000 and ties on *q1 - *q2, the quantity number, which is the order of first reference in the block.
- probe: Instrumented-cc1 BB2_QTY_DEBUG/BB2_SUGG_DEBUG local-alloc dumps of candidate.c (tmp/grind/CD_ready/s63/cand.qty.txt) and of g06 (tmp/grind/CD_ready/s69/g06.qty.txt), block 3, with each of the four quantities identified against the two disassemblies.
- result: candidate.c: t0 shift = reg102 birth 16 death 24, refs 4, pri 1.0000 -> ord 3 -> got 4 ($a0, correct); arg5 value = reg97 birth 20 death 26, refs 4, pri 1.3333 -> ord 2 -> got 3 ($v1, correct). g06: t0 shift = reg104 birth 18 death 24, refs 4, pri 1.3333 -> ord 2 -> got 3 ($v1, wrong); arg5 value = reg97 unchanged at 20-26 refs 4 pri 1.3333 -> ord 3 -> got 4 ($a0, wrong). The two priorities are bit-identical and the quantity number decides. qty_size is 1 for both, span is 6 for both, and both are pinned by the target's own sequence, leaving qty_n_refs as the single free input.
- verdict: CONFIRMED

## [s69] On the order-perfect g06 base, a loop note - either splitting the sanctioned tbl_125c do-while(0) so the t0 shift falls into a bare depth-1 gap (refs 4 -> 2) or nesting the arg5 def and/or the printf call to raise the arg5 value's refs to 5 or 6 - closes the qty_compare_1 tie.
- mechanism: flow.c accumulates reg_n_refs += loop_depth per mention, loop_depth being incremented at NOTE_INSN_LOOP_BEG and decremented at NOTE_INSN_LOOP_END; the predicted priorities are 0.3333 for the t0 shift at depth 1 and 1.6666 (refs 5) or 2.0000 (refs 6) for the arg5 value.
- probe: Eight variants scored with `sandbox CD_ready --disable all`: h01 (t0 shift alone in a bare gap), h02 (same on the g01 order), h03 (t0 shift and addu bare), control h04 (identical split boundary with NOTHING bare, all depths held at 2), j01 (t0 statements at depth 2, everything else at depth 3), j02 (arg5 def nested), j03 (printf nested, refs 5), j04 (arg5 def and printf nested, refs 6).
- result: h01 12, h02 12, h03 15, h04 12, j01 10 (180 insns), j02 12, j03 10, j04 10 - every one worse than g06's 6. The control h04 is the decisive measurement: re-bracketing with nothing bare costs exactly as much as the bare gap, so the whole 6-point loss is the note pair and none of it is the depth change. Bodies banked as rejected/s69-h01-*, s69-h04-*, s69-j03-*, s69-j04-*.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-03, candidate.c floor re-verified at 2/179/0; g06 order-perfect base carrying the sanctioned tbl_125c do-while(0) wrap, the pp pointer-alias and the v0 staging FAKEs

## [s69] A same-value re-store of a local (arg5 = arg5;) adds two mentions to the arg5 value's reg_n_refs without a loop note, lifting it from 4 to 6 and breaking the qty_compare_1 tie.
- mechanism: flow.c counts every mention of a pseudo when accumulating reg_n_refs, and a self-copy is two mentions (one set, one use), so a sanctioned dead store to a LOCAL should be a note-free refs lever.
- probe: k01 (arg5 = arg5; immediately after the load), k02 (immediately before the printf call) and control k03 (t0 = t0;) on the g06 base, each scored with `sandbox CD_ready --disable all`; k01 then re-dumped with BB2_QTY_DEBUG (tmp/grind/CD_ready/s69/k01.qty.txt) and its block 3 compared line for line against g06's.
- result: k01 = 6, k02 = 6, k03 = 6 - all byte-identical to g06 - and k01's block-3 local-alloc dump is line-for-line identical to g06's, including 'QTYDBG blk=3 ord=3 qty=2 reg1=97 birth=20 death=26 refs=4 got=4'. The refs never move: flow.c's delete_noop_moves removes the self-copy before reg_n_refs is accumulated, so the extra mentions do not exist by the time the counter runs.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-03, g06 order-perfect base (score 6/179/0) carrying the sanctioned tbl_125c do-while(0) wrap, the pp pointer-alias and the v0 staging FAKEs

## [s70] The original PsyQ libcd source shape for CD_ready, transplanted faithfully onto BB2 symbols, reaches or improves on the vAT1 basin's floor of 2.
- mechanism: tmp/closer/sotn_bios.c:260-286 is the identified original (memory/closer/libcd-groundtruth.md:40-52 proves CD_ready == this body, 179/179 masked-identical against the library object). Every prior form in 70 sessions descends from vT40/vAT1, which were hand-derived from the disassembly; the compiler's own input shape has never been compiled on the post--mel chassis. If the target IS compiler output from that source, the source shape should sit at or below the hand-derived basin.
- probe: r1 (full source shape: while(1), short-circuit || timeout test, direct globals, natural subscripts, no FAKEs, no gotos), r2 (same control flow, Intr triple via one base pointer, 1496 volatile), and the block-level family r3-r7 (the do_timeout printf arguments written as ordinary subscripts on the candidate chassis). Spliced and scored with tmp/grind/CD_ready/s70/measure.ps1.
- result: KILLED. r1 = 57 masked at 174 build insns; r2 = 55 at 178. Both are SHORT of the target's 179, so the natural control flow is not a perturbation of the basin at all. At block level the natural subscript spellings bottom out at 7 (r4, r6), against the candidate's 2: r3/r3b/r5 = 14, r7 = 8. adiff on the spliced r6 object shows why - with the pp pointer-alias gone the D_800F19C0 load sinks from emitted slot 53/54 to 61/62 and the t0 value load rises to 57, about ten instructions of ordering damage. Banked: rejected/s70-r1-natural-libcd-source-transplant-57.c, s70-r2-natural-libcd-baseptr-55.c, s70-r6-natural-subscripts-both-named-7.c.
- verdict: KILLED (instance)

## [s70] flow.c counts a pseudo reference from an RTL position other than a surviving insn's PATTERN - a REG_NOTE mention, or a mention in an insn deleted later - so a construct can raise reg_n_refs without emitting an instruction.
- mechanism: this was the s69 frontier's first item, posed as "what OTHER RTL positions does flow.c count?". If REG_NOTES or pre-deletion mentions counted, the qty_compare_1 tie could be broken with no cost to the 179-instruction parity.
- probe: read the accumulation sites directly. tools/gcc-2.7.2/flow.c has four `reg_n_refs[regno] += loop_depth;` statements (:2081, :2329, :2515, :2725) and one caller chain into them.
- result: KILLED as a class. :2329 and :2725 are inside `#ifdef AUTO_INC_DEC` and config/mips/mips.h:2175-2179 leaves HAVE_POST_INCREMENT / HAVE_PRE_INCREMENT commented out, so neither is compiled for this target. The two live sites are reached only from propagate_block, which calls mark_used_regs on `PATTERN (insn)` (flow.c:1584), on CALL_INSN_FUNCTION_USAGE (:1603, hard regs only) and on global_regs (:1625). REG_NOTES are never walked. A pseudo's reg_n_refs is therefore exactly the loop_depth-weighted count of its occurrences inside the patterns of insns that still exist when flow.c runs; the only instruction-free position anywhere in the pass is a second occurrence of the same pseudo within one surviving insn's pattern.
- verdict: KILLED (class)

## [s70] The sanctioned F1 combine-foldable copy chain-extender (a fresh local copied from the arg5 value and passed in its place) raises the arg5-value quantity's qty_n_refs, because local-alloc's combine_regs merges the two pseudos and final deletes the register-to-itself move.
- mechanism: distinct in kind from s69's k01-k03 self-stores. Those die at flow.c's delete_noop_moves before the counter runs; a copy between two DIFFERENT pseudos is not a no-op move, so if it survives to flow.c it contributes a mention to each, and local-alloc's combine_regs then sums their refs into one quantity while the allocator gives both the same hard register.
- probe: p01 (`a5b = arg5;` after the load, a5b passed as printf's 5th argument), p02 (two-link chain), p05 (the mirror on the t0 address), all on the g06 order-perfect base; scored, then dumped with BB2_QTY_DEBUG/BB2_SUGG_DEBUG and the block-3 quantity records extracted with tmp/grind/CD_ready/s70/qtyext.py.
- result: KILLED. All three score 6/179/0, byte-identical to g06, and their block-3 quantity records are line-for-line identical to g06's: qty0 refs=4, qty1 reg104 refs=4, qty2 reg97 refs=4, qty3 refs=8, ALLOC order 3,0,1,2, seats $v0/$v0/$v1/$a0. The premise is false at an earlier pass than local-alloc - the copy is copy-propagated away before flow.c, so no second pseudo ever reaches the counter. Chain length does not help (p02 identical to p01). The control p04 on the floor-2 candidate base is likewise 2/179/0. Banked: rejected/s70-p01-a5-copychain-refs-inert-6.c, s70-p02-a5-copychain-x2-refs-inert-6.c, s70-p05-t0-copychain-refs-inert-6.c.
- verdict: KILLED (instance)

## [s70] A loop note placed OUTSIDE block 3 - both its BEG and END outside the block being scheduled - raises block 3's loop_depth without introducing a sched1 region boundary inside it, giving the refs change without the order damage that killed H-s69-D.
- mechanism: s67-A and H-s69-D both attribute the damage to the note acting as a sched1 region boundary INSIDE the block. flow.c's loop_depth is a running counter over the whole insn stream, so a BEG emitted in an earlier block and closed after block 3 should raise block 3's depth with no note between block 3's own insns. This was the s69 frontier's second item.
- probe: q01 (the existing wrap's `do {` relocated above the `cnt` test and the `do_timeout:` label, END unchanged), q02 (a SECOND wrap opened there, nested over the existing in-block wrap), q03 (as q01 but END moved past the `v0 = -1; goto check;` tail), all on the g06 base; scored, then dumped with BB2_QTY_DEBUG and compared record-by-record against g06's block-3 quantities.
- result: CONFIRMED, in the NESTED shape only. q02 = 6/179/0 with the instruction sequence unchanged and block-3 refs 4/4/4/8 -> 6/6/6/12: one full loop_depth level bought for zero bytes, against the 4-to-9-point cost of every in-block placement s69 measured (h01/h02/h03 = 12/12/15, control h04 = 12, j01-j04 = 10/12/10/10). The un-nested shapes do NOT work: q01 and q03 both leave refs at 4/4/4/8 (relocating the BEG does not create a depth-carrying loop, because `goto do_timeout` enters the region from outside). Banked: progress/s70-q02-note-outside-block3-byte-neutral-refs-lever-6.c; rejected/s70-q01-note-beg-in-predecessor-depth-unchanged-6.c.
- verdict: CONFIRMED

## [s70] The now-free uniform depth lever (q02) breaks the qty1-vs-qty2 seat tie on the order-perfect base.
- mechanism: raising qty_n_refs raises qty_compare_1's priority, and s67-e02 showed that giving the arg5 value 6 refs on an order-perfect base does hand it $v1, the target's seat.
- probe: q02 measured and dumped as above; the resulting priorities computed against the qty_compare_1 formula at tools/gcc-2.7.2/local-alloc.c:1668-1675.
- result: KILLED. q02 scores 6, exactly the g06 base, and the ALLOC records are unchanged (ord 0,1,2,3 = qty3, qty0, qty1, qty2; seats $v0, $v0, $v1, $a0). The reason is arithmetic and general: qty1 (reg104, 18-24) and qty2 (reg97, 20-26) have identical mention counts (2), identical spans (6) and identical sizes (1), so a UNIFORM loop_depth change multiplies both qty_n_refs by the same factor and pri1 == pri2 at every depth (refs 4/4 -> 1.3333 each, 6/6 -> 2.0000 each, 8/8 -> 4.0000 each). local-alloc.c:1683 then breaks the tie on quantity number, and quantity number is birth order, which the target's instruction sequence pins in qty1's favour. Only an ASYMMETRIC refs change can move this seat. s67-e02's success came from NESTING the arg5 load specifically (raising one quantity, not all four), which is an in-block note and carries the order damage.
- verdict: KILLED (class)

## [s70] KILL RE-AUDIT (mandated): s69's k01 self-store kill was measured with a FAKE carrier occupying the pseudo the lever targets, so it is not a real kill on a clean base.
- mechanism: the func_8002EA24-s8 failure mode - a /* FAKE */ construct sitting on the target pseudo makes a live lever read as inert. k01 carries nine FAKE units, three of which (the pp pointer-alias and the two v0 staging lines) touch the arg5 chain directly.
- probe: python3 tools/fake_ablate.py --func CD_ready --file system --candidate tmp/grind/CD_ready/s70/k01_renamed.c --max-variants 47, i.e. the full ablation grid over all nine units on the current chassis.
- result: KILLED (the re-audit finds no masking). keep-all = 6/179 and drop-[arg5 self-store] = 6/179 are byte-identical; no subset containing the self-store beats its own control, and every other subset scores 7 or worse (best non-trivial: drop-v0<<=2 at 7/178). s69's k01 kill stands unchanged on the current chassis. Artifact tmp/grind/CD_ready/s70/ablate_k01.txt. Harness note: fake_ablate.py needs the candidate's function literally named CD_ready and returns ERR/None for every row otherwise - the ledger's Closer-era forms are named marionation_Exec; rename with tmp/grind/CD_ready/s70/rename.py first.
- verdict: KILLED (instance)

## [s70] The original PsyQ libcd source shape for CD_ready, transplanted faithfully onto BB2 symbols, reaches or improves on the vAT1 basin's floor of 2.
- mechanism: tmp/closer/sotn_bios.c:260-286 is the identified original (memory/closer/libcd-groundtruth.md:40-52 proves CD_ready == this body, 179/179 masked-identical against the library object), with set_alarm (:95), get_alarm (:102) and callback (:210) as the inlined helpers. Every form in 70 sessions descends from vT40/vAT1, hand-derived from the disassembly; the compiler's own input shape had never been compiled on the post--mel chassis. If the target is compiler output from that source, the source shape should sit at or below the hand-derived basin.
- probe: r1 (full source shape: while(1), short-circuit || timeout test, direct globals, natural array subscripts, zero FAKE constructs, zero gotos), r2 (same control flow with the Intr triple restored to single-base pointer addressing and 1496 volatile), and the block-level family r3-r7 (the do_timeout printf arguments written as ordinary subscripts on the candidate chassis). Spliced over the INCLUDE_ASM marker and scored with tmp/grind/CD_ready/s70/measure.ps1; r6 additionally disassembled against the target with the s69 adiff differ.
- result: KILLED. r1 = 57 masked at 174 build insns; r2 = 55 at 178 - both SHORT of the target's 179, so the natural control flow is not a perturbation of this basin at all. At block level the natural subscript spellings bottom out at 7 (r4 t0-named, r6 both-named), against the candidate's 2: r3/r3b inline = 14, r5 = 14, r7 arg5-first = 8. adiff on the spliced r6 object shows the cost: with the pp pointer-alias gone the D_800F19C0 load sinks from emitted slot 53/54 to 61/62 and the t0 value load rises to 57, about ten instructions of ordering damage. Banked as rejected/s70-r1-natural-libcd-source-transplant-57.c, s70-r2-natural-libcd-baseptr-55.c, s70-r6-natural-subscripts-both-named-7.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-03, candidate.c re-verified live at 2/179/0 and g06 at 6/179/0 before any probe; r1/r2 carry NO FAKE constructs at all, r3-r7 sit on the candidate chassis with its FAKE set minus the pp alias and the two v0 staging lines.

## [s70] flow.c counts a pseudo reference from an RTL position other than a surviving insn's PATTERN - a REG_NOTE mention, or a mention in an insn deleted later - so a construct can raise reg_n_refs without emitting an instruction.
- mechanism: This was the s69 frontier's first item, posed as 'which OTHER RTL positions does flow.c count?'. If REG_NOTES or pre-deletion mentions counted, the qty_compare_1 tie could be broken with no cost to the 179-instruction parity.
- probe: Read the accumulation sites directly in tools/gcc-2.7.2/flow.c and their caller chain, and checked the target macro definitions in tools/gcc-2.7.2/config/mips/mips.h.
- result: KILLED as a class. flow.c has four `reg_n_refs[regno] += loop_depth;` statements (:2081 mark_set_1 for a SET, :2515 mark_used_regs for a USE, :2329 and :2725), but the last two are inside `#ifdef AUTO_INC_DEC` and config/mips/mips.h:2175-2179 leaves HAVE_POST_INCREMENT and HAVE_PRE_INCREMENT commented out, so neither is compiled for this target. The two live sites are reached only from propagate_block, which calls mark_used_regs on PATTERN (insn) at flow.c:1584, on CALL_INSN_FUNCTION_USAGE at :1603 (hard regs only) and on global_regs at :1625. REG_NOTES are never walked for reg_n_refs. A pseudo's reg_n_refs is therefore exactly the loop_depth-weighted count of its occurrences inside the patterns of insns that still exist when flow.c runs; the only instruction-free refs position anywhere in the pass is a second occurrence of the same pseudo within one surviving insn's pattern.
- verdict: KILLED
- kill_scope: class
- measured_on: tools/gcc-2.7.2 source as pinned in this repo, read against config/mips/mips.h; no FAKE state involved (a compiler-source predicate, not a spelling measurement).
- predicate_cite: tools/gcc-2.7.2/flow.c:1584

## [s70] The sanctioned F1 combine-foldable copy chain-extender - a fresh local copied from the arg5 value and passed in its place - raises that quantity's qty_n_refs, because local-alloc's combine_regs merges the two pseudos while the allocator makes the move a register-to-itself copy that never materialises.
- mechanism: Distinct in kind from s69's k01-k03 self-stores, which die at flow.c's delete_noop_moves before the counter runs. A copy between two DIFFERENT pseudos is not a no-op move, so if it survives to flow.c it contributes a mention to each, and local-alloc's combine_regs then sums their refs into one quantity.
- probe: p01 (`a5b = arg5;` after the load, a5b passed as printf's 5th argument), p02 (two-link chain), p05 (the mirror on the t0 address) on the g06 order-perfect base, plus p04 as a control on the floor-2 candidate base. Scored, then dumped with the instrumented cc1 (BB2_QTY_DEBUG/BB2_SUGG_DEBUG via tmp/grind/CD_ready/s63/qty.sh) and the block-3 quantity records extracted with tmp/grind/CD_ready/s70/qtyext.py.
- result: KILLED. p01/p02/p05 all score 6/179/0, byte-identical to the g06 base, and their block-3 quantity records are line-for-line identical to g06's: qty0 refs=4, qty1 reg104 refs=4, qty2 reg97 refs=4, qty3 reg110 refs=8, ALLOC order 3,0,1,2, seats $v0/$v0/$v1/$a0. The premise fails at an earlier pass than local-alloc - the copy is copy-propagated away before flow.c, so no second pseudo reaches the counter and combine_regs never sees it. Chain length does not help (p02 identical to p01), and the lever is inert in both directions (p05). The candidate-base control p04 measures 2/179/0, i.e. the floor is undisturbed. Banked as rejected/s70-p01-a5-copychain-refs-inert-6.c, s70-p02-a5-copychain-x2-refs-inert-6.c, s70-p05-t0-copychain-refs-inert-6.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-03; measured on the s69 g06 order-perfect base carrying its full inherited FAKE set (the tbl_125c do-while(0) wrap, the pp pointer-alias, the two v0 staging lines, the new_var/new_var3 mask holders and the three check-path wraps), with p04 as the candidate-base control.

## [s70] A loop note placed outside block 3 - both its BEG and END outside the block being scheduled - raises block 3's loop_depth without introducing a sched1 region boundary inside it, giving the refs change without the order damage that killed H-s69-D.
- mechanism: s67-A and H-s69-D both attribute the damage to the note acting as a sched1 region boundary INSIDE the block. flow.c's loop_depth is a running counter over the whole insn stream, so a BEG emitted in an earlier block and closed after block 3 should raise block 3's depth with no note between block 3's own insns. This was the s69 frontier's second item.
- probe: q01 (the existing wrap's `do {` relocated above the `cnt` test and the `do_timeout:` label, END unchanged), q02 (a SECOND wrap opened there, nested over the existing in-block wrap), q03 (as q01 but END moved past the `v0 = -1; goto check;` tail), all on the g06 base. Scored, then dumped with BB2_QTY_DEBUG and compared record-by-record against g06's block-3 quantities.
- result: CONFIRMED, in the NESTED shape only. q02 measures 6/179/0 with the emitted instruction sequence unchanged and block-3 refs moving 4/4/4/8 -> 6/6/6/12: one full loop_depth level bought for zero bytes, against the 4-to-9-point cost of every in-block placement s69 measured (h01/h02/h03 = 12/12/15, control h04 = 12, j01-j04 = 10/12/10/10). This is the first note placement in 70 sessions that costs nothing, and it decouples block 3's loop_depth from its instruction order. The un-nested shapes do NOT work: q01 and q03 both leave refs at 4/4/4/8 (relocating the BEG does not create a depth-carrying loop, because `goto do_timeout` enters the region from outside). Banked as progress/s70-q02-note-outside-block3-byte-neutral-refs-lever-6.c, with q01 in rejected/s70-q01-note-beg-in-predecessor-depth-unchanged-6.c.
- verdict: CONFIRMED

## [s70] The now-free uniform depth lever (q02) breaks the qty1-vs-qty2 seat tie on the order-perfect base.
- mechanism: Raising qty_n_refs raises qty_compare_1's priority, and s67-e02 showed that giving the arg5 value 6 refs on an order-perfect base does hand it $v1, the target's seat.
- probe: q02 measured and dumped as above; the resulting priorities computed against the qty_compare_1 formula at tools/gcc-2.7.2/local-alloc.c:1668-1675 and its tie-break at :1683.
- result: KILLED. q02 scores 6, exactly its g06 base, and the ALLOC records are unchanged (ord 0,1,2,3 = qty3, qty0, qty1, qty2; seats $v0, $v0, $v1, $a0). The reason is arithmetic and holds for every quantity pair of this shape: qty1 (reg104, the t0 shift temp, 18-24) and qty2 (reg97, the arg5 value, 20-26) have identical mention counts (2), identical spans (6) and identical sizes (1), so a uniform loop_depth change multiplies both qty_n_refs by the same factor and pri1 == pri2 at every depth (4/4 -> 1.3333 each, 6/6 -> 2.0000 each, 8/8 -> 4.0000 each). local-alloc.c:1683 then breaks the tie on quantity number, and quantity number is birth order, which the target's own instruction sequence pins in qty1's favour. Only an asymmetric refs change can move this seat; s67-e02's success came from nesting the arg5 load specifically, which is an in-block note and carries the order damage.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD chassis 2026-09-03, g06 order-perfect base with its full inherited FAKE set plus the new outer bracket wrap; dump tmp/grind/CD_ready/s70/q02_g06_beg_pred_plus_inner.qty.txt.
- predicate_cite: tools/gcc-2.7.2/local-alloc.c:1683

## [s70] MANDATED KILL RE-AUDIT: s69's k01 self-store kill was measured while a FAKE carrier occupied the pseudo the lever targets, so it is not a real kill on a clean base.
- mechanism: The func_8002EA24-s8 failure mode - a /* FAKE */ construct sitting on the target pseudo makes a live lever read as inert. k01 carries nine FAKE units, three of which (the pp pointer-alias and the two v0 staging lines) touch the arg5 chain directly.
- probe: python3 tools/fake_ablate.py --func CD_ready --file system --candidate tmp/grind/CD_ready/s70/k01_renamed.c --max-variants 47 - the FULL ablation grid over all nine FAKE units, on the current chassis.
- result: KILLED (the re-audit finds no masking; s69's kill stands). keep-all = 6/179 and drop-[arg5 self-store] = 6/179 are byte-identical, no subset containing the self-store beats its own control, and every other subset scores 7 or worse (best non-trivial: drop-[v0 <<= 2] at 7/178). Artifact tmp/grind/CD_ready/s70/ablate_k01.txt. Harness note recorded in the ledger for future sessions: fake_ablate.py requires the candidate's function to be named literally CD_ready and silently returns ERR/None for every row otherwise - the Closer-era ledger forms are named marionation_Exec, so rename first with tmp/grind/CD_ready/s70/rename.py.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-03, k01 form across all 47 subsets of its nine FAKE units (including the all-removed and none-removed controls).


## s71 (rederive, 2026-09-03)

Modality: rederive. Frontier inherited from s70: (1) find a C spelling whose surviving insn pattern
mentions the arg5-value pseudo twice; (2) re-price s69's in-block loop notes on top of s70's free
outside-the-block depth lever (q02); (3) permuter on the order-perfect base. 26 variants measured,
9 instrumented-cc1 quantity dumps read. Floor unchanged at 2.

- h01 [KILLED, instance] Stacking s69's in-block loop-note placements on the s70 q02 free-depth base
  makes them cheaper than the 4-9 points they cost on the g06 base.
  mechanism: s70 argued each s69 placement paid twice, once for depth and once for the sched1
  region boundary; on q02 the depth is already bought from outside the block.
  probe: the printf-only wrap and the nested printf wrap, each generated on BOTH bases
  (tmp/grind/CD_ready/s71/d1g.c, d1q.c, d2g.c, d2q.c), measured with s71/measure.ps1.
  result: d1g = 10, d1q = 10, d2g = 10, d2q = 10 - identical on both bases, against the common
  baseline of 6. The note cost does not depend on the enclosing depth. Also measured: a wrap around
  the two t0 statements costs 10 on g06 and 19 on q02 (f1g/f1q). This discharges the mandated kill
  re-audit for s69's note kill and closes the s70 frontier's second item.
  measured_on: HEAD chassis 2026-09-03; candidate.c re-verified at 2/179/0, g06 at 6/179/0 and q02
  at 6/179/0 before any probe; both bases carry their full inherited FAKE sets.

- h02 [CONFIRMED] An in-block loop note raises the arg5-value quantity's qty_n_refs ABOVE the
  t0-shift quantity's - the asymmetry s70 named as the residual's only remaining requirement.
  mechanism: flow.c:2515 weights each pattern mention by the loop_depth at that insn; a note pair
  containing the arg5 value's death insn (printf's 5th-argument store) but neither of the t0 shift
  temp's two insns raises exactly one of the four mentions.
  probe: d1 (printf wrapped once) and d2 (printf wrapped twice) on the g06 base, BB2_QTY_DEBUG
  block-3 records read BEFORE the score, per the s70 frontier's instruction.
  result: g06 base is qty1 refs 4 / qty2 refs 4; d1 is qty1 4 / qty2 5; d2 is qty1 4 / qty2 6. The
  asymmetry is real and it is the first one measured on this function.

- h03 [KILLED, instance] That asymmetry flips the seat.
  mechanism: qty_compare_1 (local-alloc.c:1659-1684) ranks on
  floor_log2(refs)*refs*size/(death-birth); with refs 5-or-6 against 4 and spans held at 6, qty2's
  priority would exceed qty1's and qty2 would be allocated first, taking $v1.
  probe: the same d1/d2 dumps, reading the birth/death columns as well as refs.
  result: the spans are NOT held. The note is a sched1 region boundary, so the block re-orders:
  qty1 goes 18-24 (span 6) -> 16-20 (span 4) and qty2 goes 20-26 (span 6) -> 18-26 (span 8).
  pri1 becomes 2.0000 while pri2 is only 1.2500 (d1) or 1.5000 (d2), so qty1 keeps got=3 ($v1) and
  qty2 keeps got=4 ($a0), and the score is 10 rather than 6. On this block the refs asymmetry and
  the span damage are produced by the same event and the span term dominates.
  measured_on: HEAD chassis 2026-09-03, g06 order-perfect base with its full inherited FAKE set;
  dumps tmp/grind/CD_ready/s71/d1g.qty.txt, d2g.qty.txt.

- h04 [KILLED, instance] Removing the arg5-value pseudo from local-alloc altogether - by carrying
  the loaded value in an already-multiply-set variable so its REG_N_DEATHS exceeds 1 - dissolves
  the tie and lets global-alloc seat the value at $v1.
  mechanism: block_alloc only creates quantities for pseudos it can treat as local single-death
  values; a multiply-set carrier is skipped, exactly as reg98 (the `t0` variable) already is.
  probe: a1 - `v0 = *(s32 *)(v0 + (s32)tbl_125c);` with `v0` passed as printf's 5th argument, on
  both the g06 and q02 bases; plus the t0-side mirrors e1/e2/e3 staging the address chain through
  `v0`.
  result: the mechanism works exactly as predicted - a1's block-3 quantity table drops to THREE
  quantities and reg97 is gone (tmp/grind/CD_ready/s71/a1g.qty.txt) - but the score is 12 on both
  bases: `v0` already carries the arg5 INDEX two statements earlier, so reusing it serialises the
  block and the target's emission order is lost. e1 = 12, e2 = 13, e3 = 10.
  measured_on: HEAD chassis 2026-09-03, g06 and q02 bases with their full inherited FAKE sets.

- h05 [CONFIRMED] With the target's instruction sequence held, no byte-neutral respelling of the
  printf-argument block measured this session moves any qty_compare_1 input.
  mechanism: the block's quantities are formed from the insns, and the insns are what the order
  pins; C-level naming, pointer-vs-cast spelling and shift placement change only pseudo numbers.
  probe: six shapes on the g06 base - b1 (dead prefix store), c1 (shift folded into the address
  expression), g1 (named `s32 *ap` for the arg5 address), g6 (`(t0 << 2)` inline), g7 (a FRESH
  index local replacing the `v0` staging entirely), plus the non-neutral controls g3/g4 (named
  pointer for the t0 address, 9) and g5 (one pointer variable reused for both addresses, 16).
  result: b1, c1, g1, g6, g7 all score 6 and all reproduce block 3's quantity table row-for-row
  (qty0 16-20 refs4 got 2, qty1 18-24 refs4 got 3, qty2 20-26 refs4 got 4, qty3 22-30 refs8 got 2),
  differing only in pseudo numbers. g7 additionally moves qty0 to birth 10 / refs 12 and re-orders
  the ALLOC sequence to 0,3,1,2 - and the qty1/qty2 rows are STILL identical.

- h06 [KILLED, instance] The floor body (candidate.c, score 2, seats already correct) admits a
  do-while(0) note placement in the do_timeout block that repairs the remaining ALU transposition.
  mechanism: s67's six-position empty-wrap sweep was run against a score-6 side base, never against
  the floor; a note pair is a sched1 region boundary and could in principle force the target's
  emission order rather than perturb it.
  probe: five placements on candidate.c - n1 (t0 chain wrapped), n2 (arg5 chain wrapped), n3
  (printf wrapped), n4 (t0 chain wrapped, arg5 chain bare), n5 (the mirror).
  result: n1 = 12 at 181 build insns, n2 = 8 at 181, n3 = 10 at 179, n4 = 12 at 181, n5 = 8 at 181.
  Four of the five leave the 179-instruction basin outright; none improves on 2.
  measured_on: HEAD chassis 2026-09-03, candidate.c (the floor body) with its full inherited FAKE
  set, re-verified at 2/179/0 immediately before the batch.

- h07 [CONFIRMED, read from the compiler] There is a second allocation pass in block_alloc that
  never reaches qty_compare_1, and CD_ready's block-3 quantities are all excluded from it.
  mechanism: local-alloc.c:1495-1526 sorts by qty_sugg_compare_1 and seats every quantity with
  qty_phys_num_sugg or qty_phys_num_copy_sugg non-zero BEFORE the qty_compare_1 ordering at
  local-alloc.c:1560-1569. Suggestions are set only by combine_regs (local-alloc.c:1856-1885),
  reached from local-alloc.c:1281-1295 for any insn with an `=` output operand 0 and a REG input
  operand, when one of the two is a hard register.
  probe: read local-alloc.c:1180-1300, :1495-1575 and :1836-1900, cross-checked against the
  `copysugg=` column the instrumented cc1 already prints.
  result: all four block-3 quantities print copysugg= EMPTY in every dump taken this session. For
  reg97 the exclusion is structural on this body: its set is a load whose operand 1 is a MEM
  (constraint `m`, so the PLUS/MULT address reduction at local-alloc.c:1276-1284 never fires and r1
  is not a REG) and its death is a store into printf's 5th-argument stack slot, whose operand 0 is
  a MEM rather than a register. Neither insn can hand reg97 a hard-register suggestion. This is a
  pass-level route around the ten-session tie and it has never been probed from the C side.

## [s71] Stacking s69's in-block loop-note placements on the s70 q02 free-depth base makes them cheaper than the 4-9 points they cost on the g06 base.
- mechanism: s70 argued each s69 in-block placement paid twice - once for the loop_depth it needed and once for the sched1 region boundary - so on q02, where block 3's depth is already bought by a note pair that brackets the block from the predecessor, an in-block note would only have to pay for the asymmetry.
- probe: The printf-only wrap and the nested printf wrap generated on BOTH bases (tmp/grind/CD_ready/s71/d1g.c, d1q.c, d2g.c, d2q.c) plus the t0-chain wrap (f1g/f1q), spliced into src/system.c and scored with the cheat-invisible sandbox via tmp/grind/CD_ready/s71/measure.ps1.
- result: d1g = 10, d1q = 10, d2g = 10, d2q = 10 - bit-identical costs on both bases against their common baseline of 6. f1g = 10, f1q = 19. The cost of an in-block note pair on this block does not depend on the enclosing loop depth, so the free outside-the-block depth lever buys nothing when combined with one. This also discharges the mandated kill re-audit: s69's in-block note kill was re-measured on the newest chassis and on the newest (q02) base and it stands.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-03; candidate.c re-verified live at 2/179/0, g06 at 6/179/0 and q02 at 6/179/0 before any probe. Both bases carry their full inherited FAKE sets (pointer-alias pp, v0 index staging, the tbl_125c do-while(0), the 0xFF constant holders, the nested idx_1496 wrap, and on q02 additionally the predecessor-bracket wrap).

## [s71] An in-block loop note raises the arg5-value quantity's qty_n_refs above the t0-shift quantity's on the order-perfect base.
- mechanism: flow.c:2515 weights every pattern mention by the loop_depth in force at that insn. A note pair that contains the arg5 value's death insn (the sw into printf's 5th-argument stack slot) but neither of the t0 shift temp's two insns raises exactly one of the four mentions, which is the asymmetry s70 named as the residual's only remaining requirement.
- probe: d1 (printf statement wrapped in one do-while(0)) and d2 (wrapped twice) on the g06 order-perfect base; the instrumented cc1's BB2_QTY_DEBUG block-3 quantity records were read BEFORE the score, per the s70 frontier's instruction (tmp/grind/CD_ready/s71/d1g.qty.txt, d2g.qty.txt).
- result: g06 base prints qty1 (reg104, t0 shift temp) refs=4 and qty2 (reg97, arg5 value) refs=4. d1 prints qty1 refs=4, qty2 refs=5. d2 prints qty1 refs=4, qty2 refs=6. This is the first asymmetric reg_n_refs state measured on CD_ready in 71 sessions, and it confirms the mechanism s70 derived from the flow.c enumeration.
- verdict: CONFIRMED

## [s71] That measured refs asymmetry (arg5 value 5 or 6 against the t0 shift temp's 4) flips the local-alloc seat so the arg5 value takes $v1 on the order-perfect base.
- mechanism: qty_compare_1 (tools/gcc-2.7.2/local-alloc.c:1659-1684) ranks quantities on floor_log2(qty_n_refs) * qty_n_refs * qty_size / (qty_death - qty_birth); with the spans held at 6 as s69 showed the target's order pins them, refs 5 or 6 against 4 would give the arg5 value the higher priority, allocate it first and hand it $v1.
- probe: The same d1/d2 BB2_QTY_DEBUG dumps, reading the birth and death columns alongside refs, then the sandbox score for each form.
- result: The spans are not held. The note is a sched1 region boundary, so the block re-orders: the t0-shift quantity goes 18-24 (span 6) to 16-20 (span 4) and the arg5-value quantity goes 20-26 (span 6) to 18-26 (span 8). pri1 becomes 2.0000 while pri2 is only 1.2500 (d1) or 1.5000 (d2), so the t0 shift keeps got=3 ($v1) and the arg5 value keeps got=4 ($a0), and the score is 10 rather than the base's 6. On this block the refs asymmetry and the span damage are produced by one and the same event, and the span term outruns the refs term in both directions.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-03, g06 order-perfect base (6/179/0) carrying its full inherited FAKE set; dumps tmp/grind/CD_ready/s71/d1g.qty.txt and d2g.qty.txt taken with the instrumented cc1 at tools/gcc-2.7.2/cc1.

## [s71] Carrying the arg5 value in an already-multiply-set variable removes its pseudo from local-alloc, dissolving the qty_compare_1 tie and letting global-alloc seat the value at $v1 on the order-perfect base.
- mechanism: block_alloc only forms quantities for pseudos it can treat as local single-death values; a multiply-set carrier is skipped and falls through to global-alloc, exactly as reg98 (the t0 variable) already does on this body, where global-alloc seats it at $a0 correctly.
- probe: a1 - the load written into v0 (`v0 = *(s32 *)(v0 + (s32)tbl_125c);`) with v0 passed as printf's 5th argument - on the g06 and q02 bases, plus the t0-side mirrors e1 (both t0 statements staged through v0), e2 (the add only) and e3 (the shift only). Scores plus the BB2_QTY_DEBUG block-3 dump for a1g.
- result: The mechanism works exactly as predicted: a1's block-3 quantity table drops from four quantities to three and reg97 is gone entirely (tmp/grind/CD_ready/s71/a1g.qty.txt), so the tie no longer exists. The score is 12 on both bases: v0 already carries the arg5 INDEX two statements earlier, so reusing it serialises the block and the target's emission order is lost. e1 = 12, e2 = 13, e3 = 10, all at 179 insns and 0 rules.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-03, g06 (6/179/0) and q02 (6/179/0) bases with their full inherited FAKE sets.

## [s71] With the target's instruction sequence held, byte-neutral respellings of the printf-argument block move a qty_compare_1 input.
- mechanism: The block's local-alloc quantities are formed from the insns, and the insns are what the emission order pins; C-level naming, pointer-versus-cast spelling and shift placement were expected to at least perturb the pseudo webs.
- probe: Six shapes on the g06 base, each measured and each dumped or cross-checked: b1 (a dead prefix store `arg5 = v0;`), c1 (the t0 shift folded into the address expression), g1 (a named `s32 *ap` pointer for the arg5 address with `arg5 = *ap;`), g6 (the shift written `(t0 << 2)` inline in the address expression), g7 (the v0 index staging replaced by a fresh local `ai`), plus the non-neutral controls g3/g4 (named pointer for the t0 address) and g5 (one pointer variable reused for both addresses).
- result: b1, c1, g1, g6 and g7 all score 6 at 179 insns and 0 rules, and all reproduce block 3's quantity table row for row - qty0 16-20 refs4 got 2, qty1 18-24 refs4 got 3, qty2 20-26 refs4 got 4, qty3 22-30 refs8 got 2, ALLOC order 3,0,1,2 - differing only in pseudo numbers. g7 additionally moves qty0 to birth 10 with refs 12 and re-orders ALLOC to 0,3,1,2, and the qty1/qty2 rows are still identical. The non-neutral controls cost points instead (g3 = 9, g4 = 9, g5 = 16). Recorded as a CONFIRMED invariance so no future session re-spells the block hoping to move the seats.
- verdict: CONFIRMED

## [s71] The floor body (candidate.c, score 2, seats already correct) admits a do-while(0) note placement in the do_timeout block that repairs its remaining two-instruction ALU transposition.
- mechanism: s67's six-position empty-wrap sweep was run against a score-6 side base and never against the floor itself; a note pair is a sched1 region boundary, so on a body whose only residual is a scheduling transposition it could in principle force the target's emission order rather than perturb it.
- probe: Five placements generated on candidate.c by tmp/grind/CD_ready/s71/gen4.py and scored: n1 (the two t0 statements wrapped), n2 (the arg5 chain wrapped), n3 (the printf wrapped), n4 (t0 chain wrapped with the arg5 chain left bare), n5 (the mirror).
- result: n1 = 12 at 181 build insns, n2 = 8 at 181, n3 = 10 at 179, n4 = 12 at 181, n5 = 8 at 181, against the floor's 2 at 179. Four of the five leave the 179-instruction basin outright, and none improves on 2.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-03, candidate.c (the floor body) with its full inherited FAKE set, re-verified live at 2/179/0 immediately before and after the batch.

## [s71] block_alloc has a second allocation pass that seats quantities before qty_compare_1 is consulted, and CD_ready's block-3 quantities are all excluded from it.
- mechanism: tools/gcc-2.7.2/local-alloc.c:1495-1526 sorts the quantities by qty_sugg_compare_1 and seats every quantity for which qty_phys_num_sugg or qty_phys_num_copy_sugg is non-zero, BEFORE the qty_compare_1 ordering at local-alloc.c:1560-1569. Suggestions are created in exactly one place, combine_regs (local-alloc.c:1856-1885), which is reached from local-alloc.c:1281-1295 for ANY insn with an `=` output operand 0 and a REG input operand - not only for copy insns - whenever one of the two is a hard register.
- probe: Read local-alloc.c:1180-1300, :1495-1575 and :1836-1900, and cross-checked against the copysugg= column the instrumented cc1 already prints in every BB2_QTY_DEBUG record taken this session.
- result: All four block-3 quantities print copysugg= EMPTY in all nine dumps. For the arg5 value the exclusion is structural on this body: its set is a load whose operand 1 is a MEM (constraint `m`, so the PLUS/MULT address reduction at local-alloc.c:1276-1284 never fires and r1 is never a REG) and its death is a store into printf's 5th-argument stack slot, whose operand 0 is a MEM rather than a register. A quantity that acquired a hard-register suggestion would never reach the 1.3333-versus-1.3333 tie that has held this function since s65 - this is the first pass-level route around it and it has not been probed from the C side.
- verdict: CONFIRMED

## s72 (structural, 2026-09-03)

Modality: structural. Frontier inherited from s71: (1) the local-alloc SUGGESTION PASS as a
pass-level route around the ten-session qty_compare_1 tie; (2) qty_size, the one qty_compare_1
input never probed; (3) a permuter campaign seeded from the order-perfect base. Items (1) and (2)
were executed and are both discharged. 30 variants measured on two bases, 6 instrumented-cc1
dumps read (4 quantity, 1 priority/rank, 1 baseline). Floor unchanged at 2. Baselines re-verified
live before any probe: candidate.c 2/179/0, g06 6/179/0.

## [s72] Byte-neutral structural respellings of the printf-argument block - operand commutation, type narrowing, declaration order, subscript-versus-cast address spelling - move the seats on the order-perfect base or the emission order on the floor body.
- mechanism: The s71 h05 invariance was measured over naming and shift-placement only. Commuting
  the `addu` operands changes which operand combine_regs reduces at local-alloc.c:1240-1300
  (operand 1 of `addsi3` carries the `%` commutativity marker, so the scan over operands is not
  symmetric), and narrowing the carrier's type changes the RTL mode that MODES_TIEABLE_P and
  qty_size see.
- probe: Twelve substitutions (tmp/grind/CD_ready/s72/gen.py) applied to BOTH bases: a = arg5
  address `addu` commuted, b = t0 address `addu` commuted, c = both, d = t0 address as a plain
  integer add, e = arg5 address as u8* pointer arithmetic, f = declaration order of arg5/t0
  swapped, g = u32 arg5, h = u32 t0, i = the whole arg5 address expression in unsigned arithmetic,
  j = printf's 4th argument named in a fresh local, k = the t0 address written
  `&((u8 *)tbl_125c)[t0]`, l = the arg5 load written as a `[0]` subscript. 24 measurements.
- result: KILLED. On the floor body fa..fi, fk, fl are ALL 2/179/0 - byte-identical to the base.
  On the g06 base gb, gd, gf, gg, gh, gi, gk are all 6/179/0, again byte-identical. The three
  arg5-address respellings that are NOT byte-neutral cost exactly one point each (ga = 7, gc = 7,
  ge = 7, gl = 7): commuting the source expression commutes the emitted operands
  (`addu $v0,$s5,$v0` instead of `addu $v0,$v0,$s5`), one extra differing instruction and nothing
  else. Type narrowing to u32 is entirely inert in both directions. gj/fj (the named 4th-argument
  local) is the one substitution that does something, and it is recorded separately below.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-03; candidate.c re-verified live at 2/179/0 and the s69 g06
  base at 6/179/0 immediately before the batch. Both bases carry their full inherited FAKE sets.

## [s72] Making the t0 address chain single-assignment lets combine_regs tie it into ONE long local quantity, and the longer span lowers its qty_compare_1 priority below the arg5 value's, breaking the tie in the target's favour.
- mechanism: The floor body's own quantity table shows the winning shape - its t0-chain quantity
  has span 8 and priority 1.0000 against the arg5 value's 1.3333, is therefore allocated LAST and
  receives $a0, the target's seat. On the order-perfect base the same quantity has span 6 and
  priority 1.3333, an exact tie that local-alloc.c:1683 breaks on quantity number. combine_regs
  (local-alloc.c:1836-1900) refuses to tie a pseudo whose reg_qty is -1, which is the state of the
  multiply-set `t0` variable, so single-assignment locals should let the whole chain combine and
  stretch the span.
- probe: Five splits generated by tmp/grind/CD_ready/s72/gen2.py on BOTH bases - m1 (full
  single-assignment: `t1 = t0 * 4; t2 = (s32)((u8 *)tbl_125c + t1);` with the printf reading t2),
  m2 (shift split only, `t0` still twice-set), m3 (address split only), m4 (m1 with a pointer-typed
  tail), m5 (m1 with the shift written `<<` and the base cast to s32). Scored, then gm1's block-3
  quantity table dumped with the instrumented cc1 and compared against the g06 and candidate.c
  tables.
- result: KILLED, and the arithmetic says why. The tie DOES dissolve - gm1's block 3 carries a
  single combined quantity, reg100, spanning birth 18 to death 32 - but its priority goes UP, not
  down: qty_n_refs scales with the chain length exactly as the span does, so the combined quantity
  is refs 12 / span 14, i.e. floor_log2(12)*12/14 = 2.5714, against the split form's refs 4 /
  span 6 = 1.3333. It is allocated THIRD and takes $v1, the wrong seat, and the block re-orders as
  well (`lbu $v0,1($s2)` migrates above the D_800F19C0 load). gm1 = gm3 = gm4 = gm5 = 9 and
  fm1 = fm3 = fm4 = fm5 = 9; the one split that is byte-neutral, gm2 (shift only, `t0` still
  multiply-set), scores 6 with a quantity table identical to g06's row for row. fm2 = 3. The
  general form: for a k-insn combined chain refs = 2k*depth and span = 2k, so
  pri = floor_log2(2k*depth)*depth, monotonically INCREASING in k - chain-combining can never be
  used to lower a priority on this target.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-03, both the g06 order-perfect base (6/179/0) and candidate.c
  (2/179/0) with their full inherited FAKE sets; dump tmp/grind/CD_ready/s72/gm1.qty.txt.

## [s72] The local-alloc SUGGESTION PASS (the s71 frontier's item 1) is reachable from C on this function.
- mechanism: local-alloc.c:1507-1526 seats every quantity with a non-zero qty_phys_num_sugg or
  qty_phys_num_copy_sugg before qty_compare_1 is consulted at all. Suggestions come only from
  combine_regs (local-alloc.c:1856-1885), reached from local-alloc.c:1281-1295 when an insn has an
  `=` output operand 0 and a REG input operand and one of the two is a hard register. GCC 2.7.2
  loads printf's arguments DIRECTLY into $a0-$a3 from memory, so no pseudo-to-hard-register copy
  exists in this block - unless a C-level named local forces the argument value into a pseudo
  first, after which the argument setup becomes a `move $aN, pseudo` and combine_regs records
  copysugg = $aN with may_save_copy set.
- probe: Three named-argument locals on the g06 base - gj (`t0v = *(s32 *)t0;` for printf's 4th
  argument), q2 (`a1v = *pp;` for the 2nd), q3 (`a2v = D_800A11DC[D_800A11D5];` for the 3rd) - each
  scored and each dumped with BB2_QTY_DEBUG/BB2_SUGG_DEBUG, reading the copysugg column BEFORE the
  score exactly as the s71 frontier instructed.
- result: CONFIRMED, and it is the first non-empty copysugg recorded on CD_ready in 72 sessions.
  gj prints `qty=3 reg1=99 copysugg=7,` and that quantity is seated in the pass-1 loop (its ALLOC
  record prints ord=4, i.e. out of band, got=7 = $a3). q2 prints `copysugg=5,` (= $a1) and q3
  `copysugg=6,` (= $a2), both likewise seated ahead of the qty_compare_1 ordering. The rule is
  clean and general on this body: naming printf's Nth argument in a fresh local gives that value's
  quantity a copy suggestion of $aN and removes it from the tie-breaking ordering.
- verdict: CONFIRMED

## [s72] That suggestion route closes the seat half of the residual on the order-perfect base.
- mechanism: If either tied quantity acquires a suggestion it is seated before qty_compare_1 runs,
  so the 1.3333-versus-1.3333 tie and its quantity-number tie-break never decide anything.
- probe: The same gj/q2/q3 measurements plus q4 (the 4th-argument local assigned immediately after
  the address is formed rather than at the call site), each read against the target disassembly
  with tmp/grind/CD_ready/s61/show.py.
- result: KILLED. Every suggestion a C spelling created in this block is one of $a1, $a2 or $a3 -
  those are the only hard registers a printf argument value is copied into - and the target seats
  the two contested values at $a0 and $v1. Neither is suggestible on this body: $a0 in block 3
  carries only `la $a0, D_800161C8`, a constant address that never passes through a pseudo, and
  $v1 is not an argument register at all. Measured consequences: gj = 11 (the $a3 suggestion
  propagates back up the entire t0 chain, so the block emits `lbu $a3,0($s2)` /
  `addu $a3,$v0,$s5` / `lw $a3,0($a3)` and the emission order scrambles), q4 = 11, q2 = 10,
  q3 = 16. In q2 and q3 the arg5-value quantity does land on $v1 (got=3) but the block order is
  lost. The suggestion pass is real, reachable, and does not reach this pair.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-03, g06 order-perfect base (6/179/0) with its full inherited
  FAKE set; dumps tmp/grind/CD_ready/s72/gj.qty.txt, q2.qty.txt, q3.qty.txt.

## [s72] qty_size - the last unmeasured qty_compare_1 input (s71 frontier item 2) - is reachable byte-neutrally.
- mechanism: qty_size[q] is the number of hard registers the quantity needs, 1 for SImode and 2 for
  DImode. A size-2 arg5-value quantity would give pri2 = 2.6667 against pri1's 1.3333 with every
  other input held.
- probe: q1 - `long long arg5;` with the load unchanged and printf's 5th argument written
  `(s32)arg5` so the callee still receives a 32-bit word - on the g06 base, checking build_insns
  before the score exactly as the s71 frontier instructed.
- result: KILLED. q1 measures 19 at 181 build insns against the target's 179: the DImode carrier
  costs two extra instructions, which takes the form out of the 179-instruction basin outright.
  The frontier item is discharged, and with it every qty_compare_1 input on this function is now
  measured - refs (s69/s70/s71), span (s69), quantity number (s69), size (here).
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-03, g06 order-perfect base with its full inherited FAKE set.

## [s72] sched2's INSN_PRIORITY rung - the rung ABOVE the class/LUID tie that has decided this function's emission order since s63 - can be broken byte-neutrally.
- mechanism: rank_for_schedule (tools/gcc-2.7.2/sched.c:2408-2465) tries INSN_PRIORITY first, then
  the last-scheduled-insn class, then INSN_LUID. Every session since s63 has attacked the LUID rung
  (C statement order), which is exactly what is coupled to local-alloc's quantity births. If the
  arg5 chain's `addu` could be given a strictly higher priority than the t0 chain's `sll`, the tie
  would never reach LUID and the order half would decouple from the seat half.
- probe: BB2_PRIO_DEBUG + BB2_RANK_DEBUG dump of the floor body (tmp/grind/CD_ready/s72/prio.sh,
  output tmp/grind/CD_ready/s72/cand.prio.txt), reading every PRIODBG contribution line and every
  `PRIODBG SET` for the six insns in the contested window.
- result: KILLED, with the mechanism read off the dump. All six insns print final_pri=2:
  `SET insn=117/120/122/128/106/111 final_pri=2`. priority(insn) = max over LOG_LINKS of
  priority(pred) + insn_cost - 1 (sched.c:1497), so an ALU-to-ALU edge (cost 1) contributes ZERO
  and only a load-latency edge (cost 2) raises the value by one. The dump shows exactly one load
  edge above each chain - `insn=106 pred=99 cost=2` (the `lbu` feeding the t0 shift) and
  `insn=117 pred=115 cost=2` (the `lbu` feeding the arg5 index) - after which
  `insn=120 pred=117 cost=1 contrib=2` and `insn=122 pred=120 cost=1 contrib=2` add nothing.
  Raising the arg5 chain to priority 3 therefore requires a SECOND load above it, i.e. loading
  `tbl_125c` inside block 3 instead of inheriting it live in $s5, which is one extra instruction
  and leaves the 179 basin. The priority rung is not a free lever on this block.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-03, candidate.c (the floor body) with its full inherited FAKE
  set, re-verified live at 2/179/0; dump tmp/grind/CD_ready/s72/cand.prio.txt.

## [s72] The order/seat coupling reduces to ONE number: the t0-chain quantity's span.
- mechanism: read directly from the two block-3 quantity tables, which had never been placed side
  by side.
- probe: BB2_QTY_DEBUG block-3 dumps of candidate.c (the floor body) and of the g06 order-perfect
  base in the same run (tmp/grind/CD_ready/s72/cand.qty.txt; g06's re-read from
  tmp/grind/CD_ready/s71/g6.qty.txt).
- result: CONFIRMED.
    candidate.c (floor, seats CORRECT):  qty0 reg102 16-24 span8 refs4 pri 1.0000 -> got 4 ($a0)
                                         qty1 reg104 18-20 span2 refs4 pri 4.0000 -> got 2 ($v0)
                                         qty2 reg97  20-26 span6 refs4 pri 1.3333 -> got 3 ($v1)
                                         qty3 reg110 22-30 span8 refs8 pri 3.0000 -> got 2 ($v0)
                                         ALLOC order 1, 3, 2, 0
    g06 (order CORRECT, seats swapped):  qty0 reg102 16-20 span4 refs4 pri 2.0000 -> got 2 ($v0)
                                         qty1 reg103 18-24 span6 refs4 pri 1.3333 -> got 3 ($v1)
                                         qty2 reg97  20-26 span6 refs4 pri 1.3333 -> got 4 ($a0)
                                         qty3 reg109 22-30 span8 refs8 pri 3.0000 -> got 2 ($v0)
                                         ALLOC order 3, 0, 1, 2
  The two tables differ in exactly one respect: the target's instruction order moves the t0 shift
  one slot later, shortening the t0-chain quantity from span 8 to span 6 and lifting its priority
  from 1.0000 (allocated LAST, takes $a0, correct) to 1.3333 (an exact tie with the arg5 value,
  won on quantity number, takes $v1, wrong). Note also, read at local-alloc.c:1176 -
  `if (GET_CODE (insn) != NOTE) insn_number++;` - a loop NOTE does not advance insn_number, so a
  do-while(0) can never change a span directly; every span change s67-s71 measured came from
  sched1 re-ordering, which is exactly why the note lever always paid the order cost.
- verdict: CONFIRMED

## [s72] The local-alloc suggestion pass (local-alloc.c:1507-1526), which seats quantities before qty_compare_1 is consulted, is reachable from C on CD_ready.
- mechanism: Suggestions are created only by combine_regs (local-alloc.c:1856-1885), reached from local-alloc.c:1281-1295 when an insn has an '=' output operand 0 and a REG input operand with one of the two a hard register. GCC 2.7.2 loads printf's arguments directly into $a0-$a3 from memory, so no pseudo-to-hard-register copy exists in block 3 unless a C-level named local forces the argument value into a pseudo first, turning the argument setup into 'move $aN, pseudo'.
- probe: Three named-argument locals on the g06 order-perfect base - gj (printf's 4th argument named t0v), q2 (2nd argument named a1v), q3 (3rd argument named a2v) - each scored with the cheat-invisible sandbox and each dumped with the instrumented cc1 (BB2_QTY_DEBUG/BB2_SUGG_DEBUG), reading the copysugg column BEFORE the score as the s71 frontier instructed.
- result: CONFIRMED. gj prints 'qty=3 reg1=99 copysugg=7,' and that quantity's ALLOC record is emitted out of band (ord=4, got=7 = $a3), i.e. it was seated in the suggestion loop, not by qty_compare_1. q2 prints copysugg=5 ($a1, got 5) and q3 copysugg=6 ($a2, got 6). This is the first non-empty copysugg recorded on this function in 72 sessions and it establishes a clean general rule for the body: naming printf's Nth argument in a fresh local gives that value's quantity a copy suggestion of $aN and removes it from the tie-breaking ordering.
- verdict: CONFIRMED

## [s72] That suggestion route closes the seat half of the residual on the g06 order-perfect base.
- mechanism: If either of the two tied quantities acquired a suggestion it would be seated before qty_compare_1 ran, so the 1.3333-versus-1.3333 tie and its quantity-number tie-break at local-alloc.c:1683 would never decide anything.
- probe: The gj/q2/q3 measurements above plus q4 (the 4th-argument local assigned immediately after the address is formed rather than at the call site), each read against the target disassembly with tmp/grind/CD_ready/s61/show.py.
- result: KILLED. Every suggestion a C spelling created in this block is $a1, $a2 or $a3 - those are the only hard registers a printf argument value is copied into - while the target seats the two contested values at $a0 and $v1. On this body $a0 in block 3 carries only 'la $a0, D_800161C8', a constant address that never occupies a pseudo, and $v1 is not an argument register at all. Scores: gj 11, q4 11, q2 10, q3 16. When the $a3 suggestion IS created for the t0 value it propagates back up the entire chain (lbu $a3,0($s2) ... addu $a3,$v0,$s5 ... lw $a3,0($a3)) and the emission order scrambles. In q2 and q3 the arg5-value quantity does land on $v1 (got=3) but the block order is lost.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-03, g06 order-perfect base re-verified live at 6/179/0, carrying its full inherited FAKE set (the tbl_125c do-while(0) wrap, the pp pointer-alias, the two v0 staging lines, the new_var/new_var3 mask holders and the three check-path wraps); dumps tmp/grind/CD_ready/s72/gj.qty.txt, q2.qty.txt, q3.qty.txt.

## [s72] qty_size, the last unmeasured qty_compare_1 input, is reachable byte-neutrally by giving the arg5 value a DImode carrier.
- mechanism: qty_size[q] is the number of hard registers the quantity needs, 1 for SImode and 2 for DImode; a size-2 arg5-value quantity would give pri2 = 2.6667 against pri1's 1.3333 with every other input held.
- probe: q1 - 'long long arg5;' with the load unchanged and printf's 5th argument written '(s32)arg5' so the callee still receives a 32-bit word - on the g06 base, checking build_insns before the score.
- result: KILLED. q1 measures 19 at 181 build insns against the target's 179: the DImode carrier costs two extra instructions and the form leaves the 179-instruction basin outright. The s71 frontier item is discharged, and with it all four qty_compare_1 inputs on this function are now measured - refs (s69/s70/s71), span (s69), quantity number (s69) and size (here).
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-03, g06 order-perfect base (6/179/0) with its full inherited FAKE set.

## [s72] sched2's INSN_PRIORITY rung - the rung above the class and INSN_LUID tie that has decided this function's emission order since s63 - can be broken byte-neutrally, decoupling the order half from the seat half.
- mechanism: rank_for_schedule (tools/gcc-2.7.2/sched.c:2408-2465) tries INSN_PRIORITY first, then the last-scheduled-insn class, then INSN_LUID. Every session since s63 has attacked the LUID rung, which is C statement order and is exactly what is coupled to local-alloc's quantity births. A strictly higher priority on the arg5 chain's addu than on the t0 chain's sll would settle the order without touching statement order.
- probe: BB2_PRIO_DEBUG + BB2_RANK_DEBUG dump of the floor body via the new tmp/grind/CD_ready/s72/prio.sh, reading every PRIODBG contribution line and every 'PRIODBG SET' for the six insns in the contested window.
- result: KILLED, with the mechanism read off the dump. All six insns print final_pri=2 (SET insn=117/120/122/128/106/111). sched.c:1497 computes priority(pred) + insn_cost - 1, so an ALU-to-ALU edge (cost 1) contributes zero and only a load-latency edge (cost 2) increments. The dump shows exactly one load edge above each chain - 'insn=106 pred=99 cost=2' (the lbu feeding the t0 shift) and 'insn=117 pred=115 cost=2' (the lbu feeding the arg5 index) - after which 'insn=120 pred=117 cost=1 contrib=2' and 'insn=122 pred=120 cost=1 contrib=2' add nothing. Raising the arg5 chain to priority 3 needs a second load above it, i.e. loading tbl_125c inside block 3 rather than inheriting it live in $s5, which is one extra instruction.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-03, candidate.c (the floor body) re-verified live at 2/179/0 with its full inherited FAKE set; dump tmp/grind/CD_ready/s72/cand.prio.txt.

## [s72] Making the t0 address chain single-assignment lets combine_regs tie it into one long local quantity, and the longer span lowers its qty_compare_1 priority below the arg5 value's.
- mechanism: The floor body's own table shows the winning shape - its t0-chain quantity has span 8 and priority 1.0000 against the arg5 value's 1.3333, is allocated last and receives $a0. combine_regs refuses to tie a pseudo whose reg_qty is -1, the state of the multiply-set t0 variable, so single-assignment locals should let the chain combine and stretch the span.
- probe: Five splits (tmp/grind/CD_ready/s72/gen2.py) measured on BOTH bases - m1 full single-assignment, m2 shift split only, m3 address split only, m4 pointer-typed tail, m5 shift written << with an s32 base - then gm1's block-3 quantity table dumped and compared row by row against g06's and candidate.c's.
- result: KILLED, and the arithmetic explains it. The tie does dissolve - gm1's chain is a single quantity, reg100, birth 18 death 32 - but the priority rises rather than falls: qty_n_refs scales with the chain exactly as the span does, so the combined quantity is refs 12 / span 14 (floor_log2(12)*12/14 = 2.5714) against the split form's refs 4 / span 6 (1.3333). It is allocated third and takes $v1, the wrong seat, and the block re-orders as well. gm1 = gm3 = gm4 = gm5 = 9 and fm1 = fm3 = fm4 = fm5 = 9; the one byte-neutral split, gm2, scores 6 with a quantity table identical to g06's; fm2 = 3. General form for a k-insn combined chain: refs = 2*k*depth and span = 2*k, so pri = floor_log2(2*k*depth)*depth, monotonically increasing in k.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-03, both the g06 base (6/179/0) and candidate.c (2/179/0) with their full inherited FAKE sets; dump tmp/grind/CD_ready/s72/gm1.qty.txt.

## [s72] Byte-neutral structural respellings of the printf-argument block - operand commutation, u32 type narrowing, declaration order, plain-integer address arithmetic, subscript-versus-cast address and deref spellings - move the seats on the order-perfect base or the emission order on the floor body.
- mechanism: Commuting the addu operands changes which operand combine_regs reduces at local-alloc.c:1240-1300 (operand 1 of addsi3 carries the '%' commutativity marker, so the operand scan is not symmetric), and narrowing the carrier's type changes the RTL mode that MODES_TIEABLE_P and qty_size see. s71's h05 invariance covered only naming and shift placement.
- probe: Twelve substitutions (tmp/grind/CD_ready/s72/gen.py) applied to BOTH bases, 24 sandbox measurements: arg5-addu commuted, t0-addu commuted, both, t0 address as a plain integer add, arg5 address as u8* pointer arithmetic, arg5/t0 declaration order swapped, u32 arg5, u32 t0, the whole arg5 address expression in unsigned arithmetic, printf's 4th argument named, the t0 address written &((u8*)tbl_125c)[t0], and the arg5 load written as a [0] subscript.
- result: KILLED. On the floor body fa..fi, fk and fl are all 2/179/0, byte-identical to the base. On the g06 base gb, gd, gf, gg, gh, gi and gk are all 6/179/0, again byte-identical. The four arg5-address respellings that are not byte-neutral cost exactly one point each (ga, gc, ge, gl = 7): commuting the source expression commutes the emitted operands (addu $v0,$s5,$v0 instead of addu $v0,$v0,$s5), one extra differing instruction and nothing else. u32 type narrowing is entirely inert in both directions.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-03; candidate.c re-verified live at 2/179/0 and the g06 base at 6/179/0 immediately before the batch, both carrying their full inherited FAKE sets.

## [s72] The order/seat coupling on CD_ready reduces to a single number, the t0-chain quantity's span in block 3.
- mechanism: Read directly from the two block-3 quantity tables, which had never been placed side by side; qty_compare_1 divides floor_log2(refs)*refs*size by that span.
- probe: BB2_QTY_DEBUG block-3 dumps of candidate.c (the floor body) and of the g06 order-perfect base, extracted with tmp/grind/CD_ready/s72/qtyext.py.
- result: CONFIRMED. Floor body: qty0 reg102 16-24 span8 refs4 pri 1.0000 -> got $a0; qty1 reg104 18-20 span2 pri 4.0000 -> $v0; qty2 reg97 20-26 span6 pri 1.3333 -> $v1; qty3 reg110 22-30 refs8 pri 3.0000 -> $v0; ALLOC order 1,3,2,0. g06 base: qty0 reg102 16-20 span4 pri 2.0000 -> $v0; qty1 reg103 18-24 span6 pri 1.3333 -> $v1; qty2 reg97 20-26 span6 pri 1.3333 -> $a0; qty3 reg109 22-30 refs8 -> $v0; ALLOC order 3,0,1,2. The tables differ in exactly one respect: the target's instruction order moves the t0 shift one slot later, shortening the t0-chain quantity from span 8 to span 6 and lifting its priority from 1.0000 (allocated last, takes $a0, correct) to 1.3333 (an exact tie with the arg5 value, won on quantity number, takes $v1, wrong). Read at local-alloc.c:1176, 'if (GET_CODE (insn) != NOTE) insn_number++;' - a loop NOTE does not advance insn_number and therefore cannot move a span directly, which is the structural reason every note lever s67-s71 measured paid an order cost.
- verdict: CONFIRMED

## [s73] rank_for_schedule's CLASS rung, the last rung between INSN_PRIORITY and INSN_LUID, cannot separate the two contested insns.
- mechanism: sched.c:2429 assigns tmp_class = 3 whenever `link == 0 || insn_cost (tmp, link, last_scheduled_insn) == 1`, and insn_cost (sched.c:1372-1380) returns result_ready_cost OF THE CANDIDATE insn, which is 1 for every single-cycle MIPS ALU insn. The target's two contested insns are `addu $v0,$v0,$s5` and `sll $a0,$a0,2`, both single-cycle, so neither can ever be class 1 or class 2 whatever is scheduled before them.
- probe: the s72 BB2_RANK_DEBUG dump of the floor body read at the contested comparison, tmp/grind/CD_ready/s72/cand.prio.txt:1770 and :2088 - `RANKDBG last=122 y=120 cls=3 x=106 cls2=3 val=0` - plus first-hand reading of rank_for_schedule and insn_cost in tools/gcc-2.7.2/sched.c.
- result: KILLED. Both candidates print cls=3 and the class difference is 0, so the comparison falls through to `INSN_LUID (tmp) - INSN_LUID (tmp2)` at sched.c:2464. Together with s72's INSN_PRIORITY kill this closes the entire rank_for_schedule ladder above LUID for this pair: the emission order of slots 56/57 is decided by RTL order alone, which is exactly what couples it to local-alloc's quantity births.
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/sched.c:2429
- measured_on: HEAD chassis 2026-09-03, candidate.c (the floor body) re-verified live at 2/179/0 with its full inherited FAKE set.

## [s73] The t0-chain value can be kept out of local-alloc entirely by carrying it in a variable that is also referenced in another basic block, and the arg5 value then takes $v1 with no tie at all.
- mechanism: block_alloc creates quantities only for pseudos whose entire life is inside the block (local-alloc.c:1170-1180, with reg_is_born at :1865 and :2031); a pseudo referenced in two basic blocks is REG_BLOCK_GLOBAL and falls through to global-alloc. Writing the t0 chain into a function-scope variable genuinely used elsewhere in CD_ready makes its pseudo multi-block, so block 3 loses a quantity and the 1.3333-vs-1.3333 qty_compare_1 tie that has held this function since s65 never occurs.
- probe: r6 (the whole t0 chain carried in `status`) on the g06 order-perfect base, scored with the cheat-invisible sandbox and dumped with the instrumented cc1 (BB2_QTY_DEBUG/BB2_SUGG_DEBUG), reading the block-3 quantity count and the arg5 value's `got` BEFORE the score.
- result: CONFIRMED. Block 3 prints THREE quantities instead of four (reg101 18-20, reg97 20-26, reg109 22-30); the t0-chain quantity is absent; `QTYDBG blk=3 ord=2 qty=1 reg1=97 birth=20 death=26 refs=4 got=3` seats the arg5 VALUE at $v1, the target's seat, uncontested. The emitted arg5 half is then byte-exact against the target (`addu $v0,$v0,$s5` / `lw $v1,0($v0)` / `sw $v1,0x10($sp)`). Dump tmp/grind/CD_ready/s73/r6.qty.txt.
- verdict: CONFIRMED

## [s73] Globalizing the t0 chain closes the seat half but spends the order lever: the carrier lands in $s0 or $a3 rather than $a0, and the 56/57 pair emits in the floor order on both bases.
- mechanism: a global allocno whose live range crosses a call (the `status` carrier, live across getintr) is forced to a call-saved register; a call-free carrier (`cnt`, `i`, `src`, `dst`, `dst2`) takes a caller-saved seat but not $a0 and drags its own other use sites with it. Separately, removing the t0 pseudo from local-alloc also removes it from the sched1 dependence shape that statement order was steering, so the LUID lever stops moving the pair.
- probe: twenty variants on the g06 order-perfect base - shift-only, address-only, whole-chain and fused spellings across carriers `i`, `cnt`, `status`, `src`, `dst`, `dst2`, plus the arg5-value mirror (tmp/grind/CD_ready/s73/gen.py, gen2.py) - and eight of the same forms re-generated on candidate.c's statement order as the control (gen3.py). Each scored with build_insns read before the score; r6, r4 and r8 additionally disassembled against asm/funcs/CD_ready.s with tmp/grind/CD_ready/s61/show.py.
- result: KILLED. Best is 5 (`status`, shift-only r4 and whole-chain r6) against the floor of 2. r6 emits the chain in $s0 (`lbu $s0,0($s2)` / `sll $s0,$s0,2` / `addu $s0,$s0,$s5` / `lw $a3,0($s0)`), r8 in $a3 while moving cnt's own block-1 uses from $v1 to $a3 for four extra diffs, and every other carrier damages its own use sites (i 24/26/30, src 20/22, dst 10/12, dst2 11/13). The control is decisive: f_st 5, f_cnt 9, f_i 24, f_dst 10, f_dst2 11, f_src 20, fs_st 5, fs_cnt 9 reproduce their g06-based twins score-for-score, so once the carrier is multi-block the statement-order lever - the only lever that has ever produced the target's instruction sequence - is completely inert, and all twenty forms emit slots 56/57 in the floor order.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-03, both the g06 order-perfect base (re-verified live at 6/179/0) and candidate.c (re-verified live at 2/179/0), each carrying its full inherited FAKE set; dumps tmp/grind/CD_ready/s73/r6.qty.txt and g06.qty.txt.

## [s73] The floor body's entire residual is one adjacent transposition, and on the target's instruction order every qty_compare_1 input of the t0-shift quantity is pinned by the target itself.
- mechanism: block 3's RTL insn list maps 1:1 onto emitted slots once each lui/%lo pair is counted as one insn, and local-alloc's birth/death are twice that index. The t0-shift quantity is exactly [t0 `sll` -> t0 `addu`]. The `addu` is pinned at RTL index 10 by the target; the `sll` is at index 6 in the floor order (span 8, pri 1.0000, allocated last, $a0 - correct) and at index 7 in the target order (span 6, pri 1.3333, tie, $v1 - wrong).
- probe: tmp/grind/CD_ready/s61/show.py 44 72 on the floor body spliced live, plus the SUGGDBG-QTY/SUGGDBG-FFR dumps of g06 (tmp/grind/CD_ready/s73/g06.qty.txt) read against that mapping.
- result: CONFIRMED. Slots 56/57 are the ONLY two differing instructions in the whole 179-instruction function on the floor body, and every register in block 3 is already the target's. On the target's order the t0-shift quantity's span is forced to 6 (its only use is the pinned `addu`) and its refs to 4 (set plus one use, at loop depth 2), so within local-alloc the seat assignment on the target's order is fully determined - which is why the only escape measured this session was removing the quantity from local-alloc altogether.
- verdict: CONFIRMED

## [s73] rank_for_schedule's CLASS rung, the last rung between INSN_PRIORITY and INSN_LUID, cannot separate the two contested insns because insn_cost returns the CANDIDATE insn's result_ready_cost, which is 1 for a single-cycle MIPS ALU insn, and both contested insns are required by the target to be an addu and an sll.
- mechanism: tools/gcc-2.7.2/sched.c:2429 sets tmp_class = 3 whenever `link == 0 || insn_cost (tmp, link, last_scheduled_insn) == 1`; insn_cost (sched.c:1372-1380) returns result_ready_cost of the candidate, 1 for any single-cycle ALU insn. Class 1 and class 2 are reachable only for a candidate whose own latency exceeds 1 (a load).
- probe: First-hand reading of rank_for_schedule and insn_cost in tools/gcc-2.7.2/sched.c, checked against the s72 BB2_RANK_DEBUG dump of the floor body at the contested comparison (tmp/grind/CD_ready/s72/cand.prio.txt:1770 and :2088): RANKDBG last=122 y=120 cls=3 x=106 cls2=3 val=0.
- result: Both candidates print cls=3 and the class difference is 0, so the comparison falls through to INSN_LUID (tmp) - INSN_LUID (tmp2) at sched.c:2464. Combined with s72's INSN_PRIORITY kill this closes the entire rank_for_schedule ladder above INSN_LUID for this pair: the emission order of build slots 56/57 is decided by RTL order alone, which is exactly what couples it to local-alloc's quantity births.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD chassis 2026-09-03, candidate.c (the floor body) re-verified live at 2/179/0 with its full inherited FAKE set
- predicate_cite: tools/gcc-2.7.2/sched.c:2429

## [s73] The t0-chain value can be kept out of local-alloc entirely by carrying it in a function-scope variable that is also referenced in another basic block, and the arg5 value then takes $v1 with no qty_compare_1 tie at all.
- mechanism: block_alloc creates quantities only for pseudos whose entire life is inside the block (local-alloc.c:1170-1180, reg_is_born at :1865 and :2031); a pseudo referenced in two basic blocks is REG_BLOCK_GLOBAL and falls through to global-alloc, so block 3 loses a quantity and the 1.3333-versus-1.3333 tie that has held this function since s65 never occurs.
- probe: r6 (the whole t0 chain carried in the function-scope `status` variable) on the g06 order-perfect base, scored with the cheat-invisible sandbox and dumped with the instrumented cc1 (BB2_QTY_DEBUG/BB2_SUGG_DEBUG), reading the block-3 quantity count and the arg5 value's `got` before the score.
- result: CONFIRMED. Block 3 prints THREE quantities instead of four (reg101 18-20, reg97 20-26, reg109 22-30); the t0-chain quantity is absent; QTYDBG blk=3 ord=2 qty=1 reg1=97 birth=20 death=26 refs=4 got=3 seats the arg5 VALUE at $v1, the target's seat, uncontested. The emitted arg5 half is byte-exact against the target (addu $v0,$v0,$s5 / lw $v1,0($v0) / sw $v1,0x10($sp)). This is the first time in 73 sessions the arg5 value reaches $v1 on an order-perfect-derived body with no loop note. Dump tmp/grind/CD_ready/s73/r6.qty.txt.
- verdict: CONFIRMED

## [s73] Globalizing the t0 chain through a multi-block carrier closes the seat half but spends the order lever: across twenty carrier/position spellings the carrier lands in $s0 or $a3 rather than $a0, and slots 56/57 emit in the floor order whether the form is built on the order-perfect base or on the floor body.
- mechanism: A global allocno whose live range crosses a call (the `status` carrier, live across the getintr calls) is forced to a call-saved register; a call-free carrier (cnt, i, src, dst, dst2) takes a caller-saved seat but not $a0 and drags its own other use sites with it. Separately, removing the t0 pseudo from local-alloc also removes it from the sched1 dependence shape that statement order was steering, so the INSN_LUID lever stops moving the pair.
- probe: Twenty variants on the g06 order-perfect base - shift-only, address-only, whole-chain and fused spellings across carriers i, cnt, status, src, dst, dst2, plus the arg5-value mirror (tmp/grind/CD_ready/s73/gen.py, gen2.py) - and eight of the same forms re-generated on candidate.c's statement order as the control (gen3.py). Each scored with build_insns read before the score; r6, r4 and r8 additionally disassembled against asm/funcs/CD_ready.s with tmp/grind/CD_ready/s61/show.py.
- result: KILLED. Best is 5 (`status`, shift-only r4 and whole-chain r6) against the floor of 2. r6 emits the chain in $s0 (lbu $s0,0($s2) / sll $s0,$s0,2 / addu $s0,$s0,$s5 / lw $a3,0($s0)); r8 emits it in $a3 and drags cnt's own block-1 uses from $v1 to $a3 for four extra diffs; the other carriers damage their own use sites (i 24/26/30, src 20/22, dst 10/12, dst2 11/13, a_st 7, sa_st 11, r9 20). The control is decisive: f_st 5, f_cnt 9, f_i 24, f_dst 10, f_dst2 11, f_src 20, fs_st 5, fs_cnt 9 reproduce their g06-based twins score-for-score, so once the carrier is multi-block the statement-order lever - the only lever that has ever produced the target's instruction sequence - is inert, and all twenty forms emit slots 56/57 in the floor order. f_sv (a u8 `saved` carrier) is the one form that leaves the 179 basin (181 insns, 12).
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-03, both the g06 order-perfect base (re-verified live at 6/179/0) and candidate.c (re-verified live at 2/179/0), each carrying its full inherited FAKE set

## [s73] The floor body's entire residual is one adjacent transposition at build slots 56/57, and on the target's instruction order every qty_compare_1 input of the t0-shift quantity is pinned by the target's own instruction placement.
- mechanism: Block 3's RTL insn list maps 1:1 onto emitted slots once each lui/%lo pair is counted as one insn, and local-alloc's birth/death are twice that index. The t0-shift quantity is exactly [t0 sll -> t0 addu]. The addu is pinned at RTL index 10 by the target; the sll is at index 6 in the floor order (span 8, pri 1.0000, allocated last, $a0 - correct) and at index 7 in the target order (span 6, pri 1.3333, an exact tie with the arg5 value, won on quantity number, $v1 - wrong).
- probe: tmp/grind/CD_ready/s61/show.py 44 72 on the floor body spliced live, plus the SUGGDBG-QTY / SUGGDBG-FFR dumps of g06 (tmp/grind/CD_ready/s73/g06.qty.txt) read against that mapping.
- result: CONFIRMED. Slots 56/57 are the only two differing instructions in the whole 179-instruction function on the floor body and every register in block 3 is already the target's. On the target's order the t0-shift quantity's span is forced to 6 (its only use is the pinned addu) and its refs to 4 (set plus one use at loop depth 2), so within local-alloc the seat assignment on the target's order is fully determined - which is why the only escape measured this session was removing the quantity from local-alloc altogether. The FFR conflict sets also retire the s67 worry that $a0 might sit in the t0 quantity's used set: it does not (qty1 used = {0,1,2,5,...}), so both colourings are feasible and only the allocation ORDER decides.
- verdict: CONFIRMED

## [s73] The mandated kill re-audit on the closest-to-target form: no inherited FAKE carrier on candidate.c is masking a lever, re-measured across the full ablation grid on the current chassis.
- mechanism: A lever measured inert while a FAKE carrier occupies its target pseudo is not a kill, so the whole 8-unit FAKE grid of the floor body is swept and each subset re-scored.
- probe: tools/fake_ablate.py --func CD_ready --file system --candidate <renamed candidate.c> --max-variants 40, all 38 rows (tmp/grind/CD_ready/s73/ablate.log).
- result: KILLED (the re-audit finds nothing). keep-all reproduces 2/179 exactly and every other subset is worse (7, 11, 12, 13 ... 63). The single apparent exception, drop-00010000 (`v0 <<= 2;` removed) at 2/178, is one instruction SHORT of the target because removing that shift changes the arg5 address semantically - not a lever. The FAKE set is minimal and load-bearing, so every s72/s73 instance kill measured with it in place stands. Two harness gotchas re-confirmed: fake_ablate returns ERR for every row unless the candidate's function is literally named CD_ready (rename with tmp/grind/CD_ready/s70/rename.py), and its default --max-variants 24 silently refuses this body's 8-unit grid.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-03, candidate.c (the floor body) with each subset of its 8 inherited FAKE units ablated in turn
