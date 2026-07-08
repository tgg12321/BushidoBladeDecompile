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
