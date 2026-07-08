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
