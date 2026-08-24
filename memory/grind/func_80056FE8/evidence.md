> **ALIAS NOTE (2026-08-24):** this ledger's function is `func_80056FE8` (formerly `ang_hosei_80056FE8` — decisions.md rulings and interior prose may use either name; any `memory/grind/ang_hosei_80056FE8/` path is this dir).

# Evidence bank — ang_hosei_80056FE8

- WIP rejected_form: {'form': 'drop the `var_v0 =` test-assignment (test the load directly)', 'score': 14, 'reason': 'Worse: target reuses the loaded value, so removing the assignment changes the load. Keep `(var_v0 = *(s16*)(arg0+0x5E))`.'}

- WIP rejected_form: {'form': 'compute base = a3*40 AFTER the if/else / inline at use', 'score': 27, 'reason': 'Much worse (build 39): moves the multiply past the load. base must be computed early (as target does).'}

- WIP rejected_form: {'form': 'base += var_v0 in place / partial += 0x12C / group const', 'score': '15-16', 'reason': 'Addition reordering does not force partial before the load; all worse than the named-partial floor 10.'}

- WIP rejected_form: {'form': 'structural variants of accumulator/split-init/declaration-order (2026-06-22 session, 10 forms)', 'score': 10, 'reason': "All produce masked floor 10 (= candidate.c floor) but differ in WHICH physical register hosts var_v0. NONE produce var_v0 -> $v0. GCC 2.7.2's allocator routes var_v0 to $a1 / $v1 / $v0(via accumulator) depending on form; even when var_v0 lands in $v0 (full-accumulator variant), the partial-add (`addu` of base+var_v0) doesn't displace the lh delay slot fill because base lives in a different reg (e.g. $a2), so the scheduler still fills the lh's slot with the partial-add. The wall isn't 'var_v0 in $v0' — it's the FULL coupling: var_v0 in $v0 AND base in $a1 simultaneously, so that `addu $a1, $a1, $v0` is FORCED before the `lw $v0, 0($a0)` (otherwise the lw clobbers live var_v0)."}

- == imported from memory/wip notes.md ==
# ang_hosei_80056FE8 (text1b.c) — WIP, floor 10, BLOCKED (RA tie)

## TL;DR (2026-06-14)
40-insn angle-correction lookup. HEAD matched only via a forbidden
`register s32 partial asm("$5")` pin + `asm volatile("")` scheduling barrier.
Removed both → clean candidate, floor sandbox 10 / build_insns 42 (target 43).
Full build SHA1 mismatch (650213f4...) confirms a real register-allocation
gap, NOT a masking artifact. No pure-C lever closed it this session.

## The gap
Target: `base = a3*40` lives in **$a1**, `var_v0` in **$v0** throughout; then
`addu a1,a1,v0` computes `partial = base+var_v0` BEFORE the final
`lh v0,1034(v0)` (so the lh's load-delay slot is a `nop`).
Mine: `var_v0`'s test `lh a1,94(a0)` claims **$a1** first, so `base` is parked
in **$a3**; the partial-add then fills the lh delay slot → build is 1 insn
SHORT (42 vs 43). It's a `base`<->`var_v0` register swap (masked-invisible in
the sandbox score) plus the delay-slot-fill consequence.

## Resume steps
1. Paste candidate.c; confirm sandbox 10.
2. Push `var_v0` into $v0 and `base` into $a1 in pure C. The masked sandbox
   CANNOT see the a1/a3 choice (scores 10 either way) — verify ONLY via the
   FULL build SHA1.
3. Likely modality: decomp-permuter (tiny fn, pure RA/scheduling residual).

## Ruled out (do not re-derive)
- Dropping the `var_v0 =` test assignment: floor 14 (target reuses the value).
- Computing base late / inline: floor 27 (multiply moves past the load).
- Addition reordering (base+=, partial+=0x12C, group const): floor 15-16.

### 2026-06-22 — 10 more structural variants, all stay at masked 10 / build 42 insns
- No `partial` intermediate (inline `base+var_v0` in return): same alloc.
- `var_v0` declared FIRST (lower LUID): same alloc.
- Remove `a3` intermediate (inline def): same alloc.
- `var_v0 += base; return var_v0 + lookup + 0x12C;`: DIFFERENT alloc (base→$a2, var_v0→$v1), still 42.
- `s32 partial = base + var_v0; s32 lookup = ...; return partial+lookup+0x12C;`: same alloc as canonical candidate.
- Full accumulator (`var_v0 += base; var_v0 += lookup; var_v0 += 0x12C; return var_v0;`): DIFFERENT alloc (base→$a2, var_v0→$v1, lookup→$v0); still 42.
- Same-var split-init (`s32 partial = a3 * 40;` … `partial += var_v0;`): DIFFERENT alloc (partial→$a2, var_v0→$v1); still 42.
- `return (base + lookup + 0x12C) + var_v0;` (var_v0 as LAST addend): addiu 0x12C fills lh slot; still 42.
- `register s32 var_v0;` (storage-class hint, NOT asm-pin): same alloc; NOTE: cheat_asm_stripped counter increments → dropped.

**Pattern:** every variant produces the same TOTAL insn count (42) but with DIFFERENT register choices. Masked sandbox cannot distinguish — all score 10. The wall is the FULL coupling `var_v0→$v0 AND base→$a1 SIMULTANEOUSLY` — only then does the partial-add `addu $a1, $a1, $v0` become forced-before the `lw $v0, 0($a0)` (since the lw would clobber live var_v0 in $v0). When base is in $a2/$a3, the addu doesn't conflict with the lw's $v0 destination, so the scheduler is free to put it in the lh delay slot.

## Pointers
- `.claude/rules/register-alloc-pure-c.md` (Lever A; pins/barriers forbidden)
- Same masked-10 RA-coupling class as this batch's other backlog items.


- == s1 recon (2026-07-23) ==
- FLOOR confirmed live: sandbox 10, build_insns 42 vs target 43, canonical verdict C. Clean candidate (no pin/barrier) applied to src/text1b.c; cheat_asm_stripped 352 with clean form vs 355 with HEAD's pin form.
- NO ANALOG: find_duplicates.py at threshold 0.85 AND 0.6 -> zero near-clone pairs for ang_hosei_80056FE8. Function is a standalone leaf; the "ang_hosei" name family is structurally unrelated (ang_hosei_8003F62C is a non-leaf with jal chains).
- m2c reference generated (tmp/grind/ang_hosei_80056FE8/s1/m2c_reference.c): confirms the candidate's dataflow (base=a3*40, 3-arm var_v0, arg0 RELOAD at 0x40A not a2-reuse). m2c inlines base into return; target/candidate compute it early (base-late measured floor 27).
- The 1-insn gap is a single load-delay NOP: target FORCES partial-add (`addu $a1,$a1,$v0`) before the arg0 reload (`lw $v0,0($a0)`) because the reload clobbers live var_v0 in $v0; our build lets the partial-add fill the lh's delay slot. Requires coupled {base->$a1, var_v0->$v0} simultaneously.
- NOT yet attempted (open avenues): RTL .greg register-disposition dump; clean single-function-target permuter campaign. Prior 2 sessions only did manual structural sweeps + objdump-tail comparison.

- [s1] sandbox --disable all: score 10, target_insns 43, build_insns 42, verdict C (canonical). Floor unchanged from imported wip.

- [s1] Clean candidate (no register-asm pin, no asm-volatile barrier) applied to src/text1b.c; sandbox floor holds at 10 (cheat_asm_stripped 352 vs 355 for HEAD's pin form).

- [s1] find_duplicates.py at threshold 0.85 AND 0.6: zero near-clone pairs for ang_hosei_80056FE8 — no analog to seed from.

- [s1] m2c reference saved (tmp/grind/ang_hosei_80056FE8/s1/m2c_reference.c): base=a3*40, 3-arm var_v0 dispatch, arg0 RELOADED at +0x40A (not a2-reuse). Matches candidate.

- [s1] The single missing insn is the lh(+0x40A) load-delay nop; it appears only when partial-add is forced before the arg0 reload, i.e. only under {base->$a1 AND var_v0->$v0} simultaneously.

- [s1] NOT yet attempted on this function: RTL .greg register-disposition dump, and a clean single-function-target permuter campaign. Prior 2 sessions did manual structural sweeps + objdump-tail comparison only.

- == s2 structural (2026-07-23) — RTL diagnosis RUN; structural-reassociation axis KILLED ==
- RTL .greg dump RUN (the mandated pre-claim diagnostic, never run before). Minimal standalone unit (tmp/grind/ang_hosei_80056FE8/s2/mini.c) reproduces the sandbox alloc exactly: base->$a3(reg7), var_v0->$a1(reg5). greg conflict set for the 4 globals {72=arg0,73=a2,77=base,82=var_v0}: EACH conflicts with hard reg 2 ($v0), 3 ($v1), 29 ($sp). So var_v0 (82) has a HARD conflict with $v0 — it is NOT a mere priority tiebreaker (frontier item 1 answered: it is a real liveness conflict, and that conflict is MANUFACTURED by sched1, see next).
- ROOT CAUSE PINNED via .combine (pre-sched1) vs .sched (post-sched1): source order in .combine is ALREADY partial-first — insn107 (partial=base+var_v0), then insn110 (reload *arg0), insn114 (lookup lh), insn116 (sum). sched1's list scheduler HOISTS insn110+114 ABOVE insn107 (reload-first) to hide load latency. That hoist makes var_v0 (live to the partial-add) overlap the reload's $v0 lifetime -> the $v0 conflict -> var_v0 evicted from $v0. Target's sched1 kept partial-first.
- WHY sched1 always hoists (structural, unflippable by reassociation): the reload chain reload->lookup->sum is INTRINSICALLY one load longer than partial->sum, so INSN_PRIORITY(reload) >= INSN_PRIORITY(partial) for every associativity/grouping of the final adds. Proven by sweep (tmp/grind/.../s2/sweep.py, 8 tail forms): F0/F2/F3/F4/F7 stay reload_first base->$a3 var_v0->$a1; F1(0x12C into partial)/F8(grouped) push var_v0->$v0 but DISPLACE base to $a2 (never $a1) and fill BOTH delay slots.
- Real-sandbox confirmations this session: base+=var_v0 => score 15 build 42 (base->$a2,var_v0->$v1). (base+var_v0)+(lookup+0x12C) grouped => score 14 build 41 (var_v0->$v0,base->$a2, both slots filled — 2 SHORT). Neither improves; both worse than floor-10/42.
- Head-structural variants (sweep2.py) all dead: s16-narrow var_v0 injects a truncating `sll 16` (extra insn, wrong bytes); pointer-local reload, var-before-base decl order, named-lookup-local => identical alloc to candidate (var_v0->$a1, reload_first).
- CONCLUSION: the target fixpoint {a2=$a2, base=$a1, var_v0=$v0, partial-add-before-reload} is unreachable by ANY structural transform of dataflow-preserving C (12 new forms this session with RTL confirmation + ~20 prior). The nop only survives if the partial-add is dead before the lh, which requires the reload to clobber var_v0's reg ($v0), which requires partial-first, which sched1 refuses because the reload chain is strictly longer. This is a coupled sched1/RA fixpoint — a directed permuter search over the fixpoint space is the indicated modality (frontier item 2), NOT further structural reassociation.
- Artifacts: tmp/grind/ang_hosei_80056FE8/s2/{mini.c,mini.i.combine,mini.i.sched,mini.i.greg,mini.s,sweep.py,sweep2.py,dis.sh,rtl.sh,tails.sh}. Rejected form saved: memory/grind/ang_hosei_80056FE8/rejected/reassoc-var_v0-into-v0-displaces-base.c.

- [s2] Baseline reconfirmed: sandbox score 10, target_insns 43, build_insns 42, verdict C.

- [s2] RTL .greg (mandated diagnostic, first run for this fn): mini unit reproduces sandbox alloc base->$a3(reg7), var_v0->$a1(reg5); each of the 4 global pseudos {arg0,a2,base,var_v0} conflicts with hard regs 2($v0),3($v1),29($sp). var_v0's conflict with $v0 is a HARD liveness conflict, not a priority tiebreaker.

- [s2] .combine vs .sched proves the divergence is sched1: pre-sched order is partial-first (insn107) then reload(110)+lookup(114)+sum(116); sched1 reorders to reload-first, moving the partial-add (107) down below 110/114.

- [s2] The reload chain (reload->lookup->sum) is intrinsically one load longer than partial->sum, so sched1's list scheduler always gives it higher INSN_PRIORITY -> reassociation of the final adds cannot flip the order (verified across 8 tail forms).

- [s2] Real-sandbox: base+=var_v0 => score 15 build 42 (base->$a2,var_v0->$v1); (base+var_v0)+(lookup+0x12C) => score 14 build 41 (var_v0->$v0 but base->$a2, both delay slots filled, 2 short). Both worse than floor-10/42.

- [s2] Head-structural forms all dead: s16 var_v0 adds a truncating `sll 16`; pointer-local reload, var-before-base decl order, and named-lookup-local give the identical candidate allocation (var_v0->$a1, reload_first).

- [s2] Restored src/text1b.c to the clean floor-10 candidate; sandbox reconfirms score 10 / build_insns 42.

- == s3 structural (2026-07-23) — structural axis fully closed; mechanism refined to load-front-loading ==
- Baseline reconfirmed: sandbox score 10, target_insns 43, build_insns 42, verdict C. Applied clean candidate to src (HEAD carried the forbidden `register asm("$5")` pin + asm-volatile barrier again; removed).
- NEW SUB-HYPOTHESIS TESTED (not in s2): duplicate the partial-add `partial = base+var_v0` into EVERY arm (vA), or fold `+ base` into var_v0 inside each arm (vB), so the partial-add sits in a PREDECESSOR block. Rationale: GCC 2.7.2 sched1 is basic-block-LOCAL, so it cannot hoist the join-block reload above a partial-add living in a predecessor. In the standalone mini this WORKS — produces the exact target tail (partial -> reload -> nop -> lookup -> nop -> sum), var_v0 -> $v0, and the 43rd (2nd) load-delay nop appears.
- BUT vA/vB DIE in full context: sandbox 16 / build_insns 47 (5 OVER). objdump shows THREE physical partial-adds — `addu v1,a2,v0` (arm1), `addu v1,a2,v0` (arm2), `addu v1,a3,v0` (arm3): base occupies DIFFERENT hard regs per arm ($a2 vs $a3), so cross-jump (find_cross_jump) cannot suffix-merge the non-identical bytes. Duplication persists as +5 insns. Forcing base to one reg = register pin = cheat. KILLED.
- NEW SWEEP (sweep3.py, 7 single-partial-add forms s2 did NOT test): G1 staged-0x12C-on-partial, G2 p+0x12C+lk, G3 double-stage, G4 lk-named-after-p, G5 commuted sum, G6 0x12C-in-base, G7 named-reload. ALL 7 remain `reload_first`. Chain-length balancing (making partial->p2->sum equal-depth to reload->lookup->sum) does NOT flip the tie.
- MECHANISM REFINED: the divergence is NOT merely "reload chain is one load longer" (s2's framing) — it is sched1 FRONT-LOADING the memory reload `lw $v0,0($a0)` to the top of the join block to hide load latency. A load-priority boost, independent of add-side reassociation. That is why 15+ single-basic-block structural forms (8 s2 tail + 4 s2 head + 7 s3) ALL hoist the reload. The only thing that keeps the load after the partial-add is a real basic-block boundary between them; pure straight-line C expresses that only via arm-duplication, which cross-jump then cannot re-merge here.
- greg (v0): base=pseudo77 computed directly into $a3 (`insn23 set (reg7 a3) (ashift (reg2 v0) 3)`), var_v0=pseudo82 in $a1 — both downstream consequences of the sched1 reload-hoist. Allocator never routes base to $a1 in any structural form because var_v0 (evicted from $v0 by the manufactured conflict) claims $a1 first.
- CONCLUSION: the STRUCTURAL axis (reassociation + declaration-order + type-narrowing + statement-placement + arm-duplication/cross-jump) is comprehensively closed. The target fixpoint {base->$a1, var_v0->$v0, partial-before-reload} requires suppressing the sched1 load-hoist, which no single-basic-block C form does and which arm-duplication cannot buy (cross-jump merge blocked by per-arm base-reg divergence). Indicated next modality is NON-structural: directed permuter (frontier 1) or scheduling-technique catalog re: suppressing the load front-load (frontier 2).
- Artifacts: tmp/grind/ang_hosei_80056FE8/s3/{probe.sh,v0.c,vA.c,vB.c,sweep3.py,v0.i.greg,sw3.s,...}. Rejected forms: rejected/dup-partial-into-arms-cross-jump-fails.c, rejected/staged-chain-balance-loses-to-load-frontload.c.

- [s3] Baseline reconfirmed sandbox 10 / build 42 vs target 43 after re-applying the clean candidate (HEAD had reverted to the pin+barrier form).
- [s3] Arm-duplication of the partial-add (vA/vB) reproduces the TARGET tail structure (partial-first, var_v0->$v0, 43rd nop) in the standalone mini but is WORSE in full context: sandbox 16 / build 47, because base lands in different hard regs per arm ($a2 x2, $a3 x1) and cross-jump cannot merge the 3 non-identical partial-adds.
- [s3] 7 new single-partial-add chain-balance forms (sweep3.py G1-G7) all stay reload_first; staging +0x12C onto the partial side to tie the dep-chain length does not flip the sched1 order.
- [s3] Mechanism refined: sched1 front-loads the arg0 reload (load-latency hiding), a load-priority boost independent of add-side reassociation — the reason every single-basic-block structural form hoists the reload and manufactures var_v0's $v0 conflict.
- [s3] Restored src/text1b.c to the clean floor-10 candidate; sandbox reconfirms score 10 / build_insns 42.

- [s3] Baseline reconfirmed: sandbox score 10, target_insns 43, build_insns 42, verdict C. HEAD had reverted to the forbidden register-asm-$5 pin + asm-volatile barrier; clean candidate re-applied (cheat_asm_stripped 352).

- [s3] Arm-duplication (vA/vB) reproduces the target tail structure (partial-first, var_v0->$v0, 43rd nop) in the standalone mini but is WORSE in full context (sandbox 16 / build 47): base lands in $a2 x2 and $a3 x1 across arms, so cross-jump cannot merge the three non-identical partial-adds.

- [s3] 7 new single-partial-add chain-balance forms (sweep3 G1-G7) all stay reload_first; staging +0x12C onto the partial side to tie dep-chain length does not flip the sched1 order.

- [s3] Mechanism refined: sched1 front-loads the arg0 reload lw $v0,0($a0) (load-latency hiding) to the top of the join block in every single-basic-block form, manufacturing var_v0's hard conflict with $v0 and evicting it to $a1, which then denies base the $a1 slot.

- [s3] greg(v0): base=pseudo77 computed directly into $a3 (insn23 set (reg7 a3) (ashift (reg2 v0) 3)); var_v0=pseudo82 in $a1 — both downstream of the reload-hoist.

- [s3] Structural axis CLOSED: union of s2 (8 tail-reassoc + 4 head decl/type/pointer) and s3 (7 chain-balance + 2 arm-duplication) = 21 measured forms, none reaching {base->$a1, var_v0->$v0, partial-before-reload}. Floor unchanged at 10; src restored to clean candidate (sandbox 10 / build 42).

- == s4 permuter (2026-07-23) — clean single-function-target campaigns; NEW chassis reaches build 43 ==
- Built the honest standalone permuter workspace (tmp/grind/ang_hosei_80056FE8/s4): base.c=clean candidate, target.o from asm/funcs+prelude (gp=64 dropped), single-fn maspsx assembled whole (no extraction). Base vs target objdump reproduces the exact known gap (var_v0->$a1/base->$a3, 42 insns vs target var_v0->$v0/base->$a1, 43 insns). permuter base_score 345 (weighted) <-> sandbox 10 (raw) — same gap, different metric.
- CHASSIS 1 (clean candidate), 20115 iters / ~9 min, --stop-on-zero: best weighted score 70 = PURE register-rename (14 reg diffs), still build 42 (1 insn short). Plateaued fast, never improved. Search-based confirmation of s2/s3: semantics-preserving mutation on the single-BB candidate cannot manufacture the reload-clobbers-var_v0 fixpoint.
- CHASSIS 2 (base-reuse-for-reload: `base = *arg0` after the partial-add), seeded from chassis-1's score-70 find, 45349 iters / ~19 min: gradient 70->60->50->30. score-50/60 forms are build_insns 43 (== target) with pure register residual. score-30 (best) matches the ENTIRE HEAD byte-for-byte; residual is a 6-register TAIL swap only. BUT score-30 uses `base++; base--;` (dead no-op pair) — a forbidden dead-computation coercion. No CLEAN form beat weighted 50.
- KEY NEW FINDING (contradicts s2/s3 "only arm-duplication reaches 43"): a SINGLE-basic-block variable-reuse form reaches build_insns 43 in the REAL full-context sandbox. Measured live: form-50 (`a2 = base+var_v0; partial=a2; base=*arg0; ...`) => sandbox 10 / build 43. clean base-reuse-in-tail => sandbox 11 / build 43. var_v0-reuse-for-reload => sandbox 16 / build 43. ALL reach 43 but NONE lower the floor — the 1-insn shortfall is traded for an equivalent register-rename residual.
- RESIDUAL PINNED: on the base-reuse chassis the head byte-matches; the wall is now a pure tail RA coalescing choice: target coalesces partial with base ($a1, base dead) and reloads into $v0 (var_v0 dead); our build coalesces partial with var_v0 ($v0) and reloads into $a1 (base dead). The permuter flips this only via a dead-op cheat. This is the SAME coupled sched1/RA fixpoint from s1-s3, now isolated to a clean 6-reg tail swap on a 43-insn chassis.
- Directed clean-form attempts to flip the tail coalescing (commute, var_v0-reuse, base-reuse) all measured sandbox 10/11/16 — none < 10. Floor unchanged at 10.
- Both campaigns harvested --stop before session end (0 permuter procs alive at reap). Artifacts: tmp/grind/ang_hosei_80056FE8/s4/{base.c,compile.sh,target.o,build_ws.sh,diffform.sh,campaign.log, output-*/, c2/campaign.log, c2/output-*/}. Rejected form: rejected/base-reuse-reload-reaches43-tail-ra-swap-residual.c.

- [s4] Standalone permuter workspace built + validated (base 42 vs target 43, exact known gap). Chassis1 (clean candidate) 20k iters plateau weighted-70 pure-reg, build 42. Chassis2 (base-reuse) 45k iters, gradient to weighted-30 with full head-match; best clean 50.
- [s4] NEW: base-reuse-for-reload single-BB form reaches build_insns 43 in full-context sandbox (form-50 sandbox 10/build 43; measured live). Contradicts s2/s3 "only arm-dup reaches 43". Does NOT lower floor — trades missing insn for equal register residual (sandbox 10-16).
- [s4] Residual isolated to a pure 6-register TAIL swap (partial/reload coalescing) on the 43-insn base-reuse chassis; permuter flips it only via `base++;base--;` dead-op cheat (weighted 30, still not 0). No clean sub-floor form.
- [s4] Floor holds at sandbox 10 / build 42 (clean candidate re-verified in src after all probes).

- [s4] Standalone permuter workspace validated: base.o 42 insns vs target.o 43 insns, reproducing the exact known gap (var_v0->$a1/base->$a3 vs target var_v0->$v0/base->$a1). permuter weighted base_score 345 <-> sandbox raw 10 (same gap, different metric).

- [s4] Chassis1 (clean candidate): 20115 iters / ~9 min, best weighted 70 = 14 pure register-rename diffs, still build 42. Plateaued immediately; search-based confirmation of s2/s3 that semantics-preserving mutation on the single-BB candidate cannot manufacture the reload-clobbers-var_v0 fixpoint.

- [s4] Chassis2 (base-reuse-for-reload, seeded from chassis1 best): 45349 iters / ~19 min, gradient weighted 70->60->50->30. score-50/60 forms are build 43 with pure register residual; score-30 byte-matches the ENTIRE HEAD (residual = 6-register tail swap only) but uses `base++; base--;` dead-op cheat.

- [s4] NEW (contradicts s2/s3 'only arm-duplication reaches 43'): a single-basic-block variable-reuse form reaches build_insns 43 in the REAL full-context sandbox. Live measurements: form-50 sandbox 10/build 43; clean base-reuse-in-tail 11/43; var_v0-reuse 16/43. None lower the floor.

- [s4] Residual pinned: on the base-reuse 43-insn chassis the head byte-matches; the wall is a pure tail RA coalescing choice - target coalesces partial with base ($a1, base dead) and reloads into $v0 (var_v0 dead); our build coalesces partial with var_v0 ($v0) and reloads into $a1 (base dead). Permuter flips it only via a dead-op cheat.

- [s4] Directed clean-form flips (commute, var_v0-reuse, base-reuse) all measured sandbox 10/11/16 - none <10. Floor unchanged at sandbox 10 / build 42 (clean candidate re-verified in src after all probes).

- [s4] Both campaigns harvested --stop before session end; 0 permuter processes alive at final reap (--ttl 0 --dry-run: groups_seen 0).

- == s5 permuter (2026-07-23) — FLOOR IMPROVED 10 -> 9; double-reuse chassis; residual = pure $a1<->$a2 RA tiebreak ==
- NEW CHASSIS (not tried s1-s4): the DOUBLE variable-reuse tail `base += var_v0; var_v0 = *((s32*)arg0);` — partial computed in base's reg, arg0 reload reuses var_v0's dying reg. Measured live: sandbox score 9 / build_insns 43 (== target). This LOWERS THE FLOOR from 10 to 9 (first floor drop since import). src/text1b.c holds this form; re-verified score 9 twice.
- TARGET-FAITHFUL, not gratuitous: target tail asm is `addu a1,a1,v0 ; lw v0,0(a0) ; nop ; lh v0,1034(v0) ; nop ; addu v0,a1,v0` = partial in base's $a1 (base dead) + reload in var_v0's $v0 (var_v0 dead). The double-reuse encodes exactly this. Both reuses load-bearing: drop `base+=` -> diff 16; drop reload-reuse (fC: `partial=base+var_v0; var_v0=*arg0`) -> diff 16; reload into a2 instead (fD) -> 41 insns/diff 17. Only the base+=var_v0 AND var_v0=reload combo gives 43/9.
- RESIDUAL FULLY ISOLATED (objdump vs target): a single $a1<->$a2 swap. Target: a2local(*arg0)->$a2, base(a3*40)->$a1. Ours: a2local->$a1, base->$a2. All 9 diff lines are that one swap propagating (lw a1 vs a2, sll a1 vs a2, the field reads lh v0,x(a1) vs (a2), addu a1 vs a2 x2).
- greg PROOF (tmp/grind/ang_hosei_80056FE8/s5/f_doublereuse.c.greg): 4 pseudos, colored priority order 82,73,77,72. Dispositions: 82->$v0(var_v0), 73->$a1, 77->$a2, 72->$a0(arg0). insn10 `set (reg 5 a1) (mem (reg 4 a0))` => pseudo 73 = a2local (5 refs). pseudo 77 = base (2 refs). a2local(73) colored BEFORE base(77) => a2local grabs $a1, base forced to $a2. Neither has a copy-preference (only 82 pref $v0, 72 pref $a0). To flip, base must out-prioritize a2local; priority ~ floor_log2(n_refs)/live_length, base's 2 refs lose to a2local's 5. Target-faithful base has EXACTLY 2 refs (def + partial-add), so no clean structure adds refs without diverging from target's instruction stream.
- PERMUTER (fresh seed = double-reuse, base_score weighted 50, 28560 iters / ~16 min, --stop-on-zero): gradient 50->40->10->0. The ONLY weighted-0 form = double-reuse + `base++; base--;` (dead-op pair supplies base's 2 missing refs -> flips the tiebreak). SAME forbidden dead-op cheat s4 found on the base-reuse chassis, reconfirmed on the better double-reuse chassis. The weighted-10 forms (output-10-1/-2) overwrite `base = <arm load>` inside an if/else arm (corrupts base=a3*40 before the tail add) = semantically divergent from target (target computes arms into fresh $v0, never touches base) -> cheats/non-matches. NO clean permuter form beat sandbox 9.
- do-while(0) alone on the double-reuse chassis (f_dowhile) = diff 9 (no help); the swap-flip in output-10-1 came from the else-arm base-reuse, not the do-while. a3-reuse-as-base + do-while (w2) = 42 insns / diff 8 (loses the reload nop).
- Campaign harvested --stop; reap --ttl 0 dry-run groups_seen 0, pgrep permuter empty (no orphans).
- Artifacts: tmp/grind/ang_hosei_80056FE8/s5/{f_doublereuse.c, f_doublereuse.c.greg, fB/fC/fD_*.c, f_dowhile.c, v3/v5/v6_*.c, w1/w2_*.c, f_out10.c, perm/ (campaign ws + output-*), *.sh}. candidate.c updated to double-reuse (floor 9). Rejected: double-reuse-plus-baseincdec-deadop.c.

- [s5] FLOOR IMPROVED 10 -> 9. Double-reuse chassis (base += var_v0; var_v0 = *((s32*)arg0);) measured live sandbox --disable all: score 9, build_insns 43 == target 43, verdict C. src/text1b.c holds this form; re-verified score 9 twice (no campaign contention).

- [s5] The chassis is TARGET-FAITHFUL: target tail asm computes partial in base's reg ($a1, base dead) and reloads *arg0 into var_v0's reg ($v0, var_v0 dead) — exactly this dataflow. Contradicts s1-s4's 'single-BB stuck at 42' framing (reaching 43 needs the reuse, confirmed).

- [s5] Both reuses load-bearing: dropping base+= -> diff 16; dropping the var_v0 reload-reuse (fC) -> diff 16; reload into a2 (fD) -> 41 insns / diff 17. Only base+=var_v0 AND var_v0=reload together gives 43 insns / score 9.

- [s5] Residual fully isolated (objdump vs target): a single $a1<->$a2 swap. Target a2local(*arg0)->$a2, base(a3*40)->$a1; ours swapped. All 9 diff lines are that one swap propagating through field reads + tail adds.

- [s5] greg proof (tmp/grind/ang_hosei_80056FE8/s5/f_doublereuse.c.greg): 4 pseudos colored priority order 82,73,77,72. 82->$v0(var_v0), 73->$a1, 77->$a2, 72->$a0(arg0). insn10 sets (reg 5 a1)=(mem (reg 4 a0)) => pseudo 73 = a2local (5 refs); pseudo 77 = base (2 refs). a2local colored before base -> a2local takes $a1, base forced to $a2. Neither has a copy-preference.

- [s5] Permuter (fresh double-reuse seed, weighted base_score 50, 28560 iters/~16 min, --stop-on-zero): gradient 50->40->10->0. ONLY weighted-0 form = double-reuse + `base++; base--;` (dead-op adds base's 2 missing refs, flipping the tiebreak) = the same forbidden dead-op cheat s4 found, reconfirmed on the better chassis. Weighted-10 forms overwrite base inside an if/else arm (corrupt base=a3*40) = semantically divergent from target. No clean form beat sandbox 9.

- [s5] do-while(0) alone on the double-reuse chassis = diff 9 (no help); output-10-1's swap-flip came from the else-arm base-reuse, not the do-while.

- [s5] Campaign harvested --stop; reap --ttl 0 dry-run groups_seen 0; pgrep permuter empty (no orphans).

- == s6 forensics (2026-07-23) — copy-preference + scheduling frontiers KILLED; mechanism named at global.c find_reg; OWNER-ESCALATION filed ==
- Baseline reconfirmed: applied s5 double-reuse candidate to src, sandbox --disable all = score 9, build_insns 43 == target 43, verdict C. (src RESTORED to HEAD pin form at session end to hold the byte-match while parked.)
- INSTRUMENTED CC1 (tmp/gccdbg/cc1) ALLOCDBG dump — allocno priority (floor_log2(nref)*nref/live_length*10000*size): p82 var_v0 24000 [8ref/len10]; p73 a2local 8571 [6ref/len14]; p77 base 3809 [4ref/len21]; p72 arg0 3333 [4ref/len24]. Priority order 82,73,77,72. a2local(73) colored BEFORE base(77).
- FINDREGDBG (77=base): own_copy_prefs EMPTY, own_full_prefs EMPTY, someone_prefers {4=$a0}. FINDREGDBG(73=a2local): prefs EMPTY, someone_prefers {4}. FINDREGDBG(82=var_v0): own_full_prefs {2=$v0} (return-value pref). FINDREGDBG(72=arg0): own_copy/full_prefs {4=$a0} (incoming-arg pref). Neither base nor a2local carries ANY register preference.
- MECHANISM NAMED: the a1<->a2 swap is a global.c find_reg decision. a2local (higher priority, colored first) takes lowest free reg $a1 (find_reg pass-0, line 970 excludes only regs_someone_prefers={$a0}); base takes $a2. Target needs base->$a1, reachable ONLY if base has a full-preference for $a1 -> prune_preferences(global.c:851) would then add reg5 to a2local's regs_someone_prefers -> a2local avoids $a1 -> takes $a2 -> base gets $a1. Verified against find_reg source (lines 946-1126) + prune_preferences.
- COPY-PREFERENCE FRONTIER KILLED: set_preference(global.c:1591) makes a hard-reg preference ONLY from a reg<->hard-reg COPY insn. $a1 has NO ABI anchor in this 1-arg leaf (base def'd by sll, consumed by addu; no move to a hard reg). expand_preferences(global.c:797) can't merge one in (base conflicts with var_v0, the only set-insn that kills a live reg). No target-faithful C creates a base->$a1 preference.
- PRIORITY-FLIP DEAD (empirical, sweep.py): declare-base-last (v1) + a2-via-pointer (v2) = identical alloc (a2local->$a1). Cutting a2local refs by hoisting the 0xE read (v3) DROPS refs 6->5 but SHORTENS live range 14->11 -> pri RISES 8571->9090 (backfires) AND diverges from target (target reads 0xE twice). base MUST cross the join (len21); a2local MUST die at join (43rd nop needs a FRESH *arg0 reload, not a2 reuse) -> a2local inherently shorter-lived/higher-priority.
- SCHEDULING-WRAPPER FRONTIER KILLED (empirical, sweep_sched.py): do-while(0) around tail / whole-body / the multiply all keep base->$a2, a2local->$a1; base's live_length stays 21 in every case (do-while inflates ref counts via loop notes but never shrinks base's live range). No sanctioned scheduling wrapper flips the priority.
- COUPLING PROOF (confirm.py + verify_pin.sh): a diagnostic `register base asm("$5")` pin removes base from the allocno set; a2local then colors to $a2 (correct swap!). BUT the pinned build reschedules to 41 insns (loses 2 load-delay nops) because the register-asm hard-reg is visible to sched1. So even a blunt pin does NOT reproduce the coupled {base$a1, a2local$a2, 43-insn} fixpoint — only the anchor-less soft preference would. The 43-insn (fresh reload) and base->$a1 (a2local must avoid $a1) requirements are mutually locked.
- DISPOSITION: scan_hand_coded --single ang_hosei_80056FE8 = LOW 0/8 (canonical-asm not supportable). Every sanctioned axis (structural s2/s3, permuter s4/s5, copy-preference s6, scheduling-wrapper s6, ref-lift=s3-killed) measured dead. OWNER-ESCALATION filed docs/grind/decisions.md 2026-07-23; returned owner-gated.
- Artifacts: tmp/grind/ang_hosei_80056FE8/s6/{base.c, dump.sh, dump.allocdbg.txt, dump.findreg72/73/77/82.txt, sweep.py, sw_*.c/.allocdbg, confirm.py, confirm_pinned.c, verify_pin.sh, sweep_sched.py, sc_*.c}.

- [s6] Instrumented-cc1 ALLOCDBG names the swap: allocno priority a2local(p73)=8571 > base(p77)=3809, so a2local colored first takes $a1, base takes $a2. FINDREGDBG confirms neither base nor a2local carries any register preference (only var_v0->$v0 return-pref and arg0->$a0 arg-pref exist).
- [s6] Copy-preference frontier KILLED: base needs a full-preference for $a1 to flip it (prune_preferences->regs_someone_prefers->find_reg pass0), but set_preference only creates hard-reg prefs from reg<->hard-reg copies and $a1 has no ABI anchor in this 1-arg leaf; expand_preferences can't merge one (base conflicts with var_v0). No target-faithful C creates the anchor.
- [s6] Scheduling-wrapper frontier KILLED: do-while(0) at 3 placements never shrinks base's live_length (21) nor flips the priority; base stays $a2.
- [s6] Priority-flip dead: base must cross the join (long live range), a2local must die at the join (43rd nop needs fresh *arg0 reload) -> a2local inherently higher priority; cutting a2local refs backfires (shorter live range raises priority).
- [s6] Coupling proof: diagnostic register base asm("$5") pin gives base->$a1/a2local->$a2 but reschedules to 41 insns (loses 2 nops) — even a pin can't reproduce {base$a1, a2local$a2, 43-insn}; only the anchor-less soft preference would.
- [s6] scan_hand_coded LOW 0/8; OWNER-ESCALATION filed; owner-gated. src restored to HEAD pin form (byte-match held) with no net diff.

- [s6] Baseline reconfirmed: s5 double-reuse candidate in src, sandbox --disable all = score 9, build_insns 43 == target 43, verdict C. src RESTORED to HEAD register-asm-$5 pin form at session end (git diff clean) to hold the byte-match while parked.

- [s6] The entire 9-diff residual is one $a1<->$a2 register swap: target a2local(*arg0)->$a2, base(a3*40)->$a1; ours swapped. Scheduled instruction streams are otherwise byte-identical (43 insns each).

- [s6] ALLOCDBG (instrumented cc1) allocno priority: var_v0(p82) 24000, a2local(p73) 8571, base(p77) 3809, arg0(p72) 3333; order 82,73,77,72. a2local colored before base, takes lowest free reg $a1; base takes $a2. This IS the swap.

- [s6] Mechanism named: to flip, base needs a full-preference for $a1 so a2local's regs_someone_prefers gains reg 5 (prune_preferences) and a2local avoids $a1 in find_reg pass 0. set_preference only creates hard-reg prefs from reg<->hard-reg copies; $a1 has no ABI anchor in this 1-arg leaf; expand_preferences blocked by the base/var_v0 conflict. Frontier-1 mechanically unreachable.

- [s6] do-while(0) at 3 placements never shrinks base's live_length (21) nor flips the priority (sweep_sched). Frontier-3 dead. Priority ref-lift to n_refs>=8 reduces to the s3-killed cross-jump-merge-fails arm-duplication.

- [s6] Coupling proof: register base asm("$5") pin yields base->$a1/a2local->$a2 (target alloc) but reschedules to 41 insns (2 nops lost) -- the 43-insn (fresh reload) and base->$a1 (a2local avoids $a1) requirements are mutually locked; only an anchor-less soft preference would satisfy both.

- [s6] scan_hand_coded.py --single ang_hosei_80056FE8 = LOW 0/8 (no S1-S8; 43 insns, 0 spills, 5 regs) -> ordinary GCC RA output, canonical-asm NOT supportable.

- [s6] All sanctioned axes dead: structural (s2/s3, 21 forms), permuter (s4/s5, 4 chassis ~150k iters, only weighted-0 = base++;base-- dead-op cheat), copy-preference (s6), scheduling-wrapper (s6), ref-lift (=s3). OWNER-ESCALATION filed docs/grind/decisions.md 2026-07-23 naming ang_hosei_80056FE8.
