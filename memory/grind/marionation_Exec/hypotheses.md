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
