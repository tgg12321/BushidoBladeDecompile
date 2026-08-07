# Hypothesis ledger — ang_hosei_80056FE8

## s1 recon (2026-07-23) — baseline map

Floor: sandbox 10, build_insns 42 vs target 43, canonical verdict C. Applied clean
candidate to src (removed HEAD's `register asm("$5")` pin + `asm volatile` barrier).

### Mechanism (confirmed from asm + ledger)
40-insn angle-correction leaf. The single missing insn is a load-delay NOP:
- TARGET: `base=a3*40` -> `$a1` (via `sll $a1,$v0,3` early); `var_v0` test load
  `lh $v0,0x5E($a0)` -> `$v0`; both live to the join. At `.L80057074`:
  `addu $a1,$a1,$v0` (partial) is FORCED before `lw $v0,0($a0)` (arg0 reload)
  because the reload would clobber live var_v0 in $v0 -> the following
  `lh $v0,0x40A($v0)` keeps a genuine load-delay `nop` = 43 insns.
- OURS: var_v0's test load claims `$a1`, base parks in `$a3`; partial-add
  `addu ?,$a3,$a1` has no conflict with the `$v0` reload, so the scheduler fills
  the lh's delay slot with it -> 42 insns (1 short).
- The wall is the FULL coupling: `var_v0->$v0 AND base->$a1 SIMULTANEOUSLY`. Every
  structural variant tried so far produces build 42 with DIFFERENT reg choices;
  the masked sandbox scores 10 for all of them (can't see the a1/a3 swap).

### Recon findings
- No sibling/duplicate analog: find_duplicates at threshold 0.85 AND 0.6 returns
  ZERO pairs for this function. `ang_hosei` (base name) is also INCOMPLETE and
  structurally unrelated; `ang_hosei_8003F62C` is a non-leaf with many calls —
  name-only kinship, not a code analog.
- m2c reference saved (tmp/grind/ang_hosei_80056FE8/s1/m2c_reference.c): m2c
  inlines base into the return, but target computes base EARLY. candidate's
  early-base ordering matches target; base-late was measured floor 27.

## Frontier (un-tried, mechanism-grounded — for next drill/permuter session)
1. RTL .greg dump (cc1 -da on the candidate's base.i): read `;; Register
   dispositions:` + conflict lists for the base pseudo and the var_v0 pseudo.
   NOT yet done (prior sessions only did objdump-tail comparison). Determines
   whether base->$a3 is a priority TIEBREAKER (C-lever reachable) or a real
   liveness conflict. This is the mandated pre-claim diagnostic.
2. Clean single-function-target permuter (--stop-on-zero + directed PERM_*),
   target.o built from asm/funcs + prelude at offset 0 so base score is the real
   weighted RA diff (per difficult-is-not-impossible §3). NOT yet run — prior
   sweeps were manual structural variants only, no permuter campaign.
3. Steer base into $a1 by shrinking var_v0's competing live range so the
   allocator's priority for the arg-reg-resident base rises (specific C form to
   derive from the .greg dump in #1).

## [s1] A completed sibling/duplicate function provides a known-C analog to seed the match.
- mechanism: find_duplicates.py opcode-class Levenshtein near-clone search over asm/funcs/.
- probe: Ran find_duplicates.py at threshold 0.85 and 0.6; grepped tmp/duplicates.txt for 80056FE8.
- result: Zero near-clone pairs at either threshold. ang_hosei/ang_hosei_8003F62C are name-only kin (latter is a non-leaf with jal chains).
- verdict: KILLED

## [s2] The {base->$a1, var_v0->$v0, partial-first} coupling is reachable by structural reassociation / declaration-order / type levers.
- mechanism: If some dataflow-preserving C form makes GCC keep the partial-add ahead of the *arg0 reload, var_v0 stays in $v0, base takes $a1, and the lh(+0x40A) load-delay nop survives (43 insns).
- probe: RTL diagnosis (.combine vs .sched vs .greg on a standalone mini unit) + 12 new forms (sweep.py 8 tail forms, sweep2.py 4 head forms) + 2 real-sandbox measurements.
- result: KILLED. .combine shows source order is ALREADY partial-first; sched1 hoists the reload+lookup chain above the partial-add because that chain is intrinsically one load longer (higher INSN_PRIORITY) for EVERY grouping of the final adds. Forms that force var_v0->$v0 (0x12C-into-partial, grouped) displace base to $a2 and fill both delay slots (build 41, score 14). s16-narrow adds a truncating sll (wrong bytes). Head decl-order/pointer-local/base-mul give the identical candidate alloc. The target fixpoint {a2=$a2,base=$a1,var_v0=$v0,partial-first} is not reachable by any structural transform.
- verdict: KILLED

## [s1] The 1-insn shortfall (42 vs 43) is a single load-delay NOP produced by a coupled register allocation {base->$a1, var_v0->$v0}.
- mechanism: Target keeps base=a3*40 in $a1 and var_v0 in $v0 across the join; the arg0 reload `lw $v0,0($a0)` would clobber live var_v0, forcing `addu $a1,$a1,$v0` before it, so `lh $v0,0x40A($v0)` keeps a genuine load-delay nop. Our build lands var_v0's test in $a1 and base in $a3, so the partial-add fills the lh delay slot instead.
- probe: Read asm/funcs/ang_hosei_80056FE8.s instruction-by-instruction; cross-checked against ledger's masked-invisible RA-swap finding; m2c reference confirmed dataflow (arg0 reload, early base).
- result: Confirmed: target's partial-add precedes the reload (43 insns); every prior structural variant produces 42 with different reg choices, masked-invisible at sandbox 10.
- verdict: CONFIRMED

## [s2] The {base->$a1, var_v0->$v0, partial-add-before-reload} coupling that yields the 43rd (load-delay nop) insn is reachable by structural reassociation / declaration-order / type-narrowing levers.
- mechanism: If a dataflow-preserving C form keeps the partial-add (base+var_v0) ahead of the *arg0 reload, var_v0 stays in $v0, the reload clobbers it forcing partial-first, base takes $a1, and the lh(+0x40A) load-delay nop survives.
- probe: Ran the mandated RTL diagnostic on a standalone mini unit (cc1 -da): compared .combine (pre-sched1), .sched (post-sched1), and .greg dispositions/conflicts; then swept 8 tail-reassociation forms (sweep.py) + 4 head-structural forms (sweep2.py) + 2 real-sandbox measurements.
- result: .combine shows source order is ALREADY partial-first (insn107 partial, then 110 reload, 114 lookup, 116 sum); sched1 HOISTS reload+lookup above the partial-add, manufacturing var_v0's hard conflict with $v0 (greg: pseudo 82 conflicts hard reg 2). The reload->lookup->sum chain is intrinsically one load longer than partial->sum, so INSN_PRIORITY(reload)>=INSN_PRIORITY(partial) for every grouping -> sched1 always hoists. Forms forcing var_v0->$v0 (0x12C-into-partial, grouped) displace base to $a2 (never $a1) and fill BOTH delay slots (build 41, masked score 14). s16-narrow injects a truncating sll (wrong bytes). Head decl-order/pointer-local/base-mul reproduce the candidate alloc exactly. Target fixpoint {a2=$a2,base=$a1,var_v0=$v0,partial-first} unreachable by structural transform.
- verdict: KILLED

## [s3] The {base->$a1, var_v0->$v0, partial-before-reload} coupling is reachable by ARM-DUPLICATION (sinking the partial-add into predecessor blocks so sched1's block-local scheduler cannot hoist the join reload above it).
- mechanism: GCC 2.7.2 sched1 is basic-block-local; a partial-add duplicated into each arm lives in a predecessor block, so the join-block reload cannot be scheduled ahead of it. cross-jump would re-merge the identical duplicated adds back to one at codegen (sanctioned duplicated-statement-into-arms family).
- probe: vA (partial=base+var_v0 in all 3 arms) + vB (var_v0=...+base in all 3 arms), mini proxy then full sandbox; objdump of the full-context build.
- result: KILLED. In the standalone mini it produces the EXACT target tail (partial -> reload -> nop -> lookup -> nop -> sum; var_v0->$v0; the 43rd load-delay nop present). In FULL context it is WORSE — sandbox 16 / build_insns 47 (5 over) — because cross-jump cannot merge the three partial-adds: base occupies different hard regs per arm (objdump `addu v1,a2,v0` x2 + `addu v1,a3,v0` x1), so the byte sequences differ and no suffix-merge fires. Pinning base to one register to force the merge is a register-asm cheat. The duplication family therefore cannot buy the fixpoint.
- verdict: KILLED

## [s3] Winning the sched1 priority tie (balancing partial's dep-chain length vs the reload->lookup->sum chain) keeps the single partial-add before the reload.
- mechanism: if partial's chain-to-block-end equals the reload chain's, the list scheduler's tiebreak might keep partial first (earlier source order).
- probe: sweep3.py, 7 single-partial-add forms (G1 staged +0x12C on partial; G2 p+0x12C+lk; G3 double-stage; G4 lk-named-after-p; G5 commuted sum; G6 0x12C-in-base; G7 named-reload), mini proxy reading reg alloc + reload/partial order.
- result: KILLED. All 7 stay `reload_first`. The divergence is NOT a chain-length tie — it is sched1 FRONT-LOADING the memory reload to hide load latency (a load-priority boost independent of add-side reassociation). No reassociation/staging of the ADD side can suppress a load hoist. This refines s2's "reload chain is one load longer" to the sharper "loads are front-loaded regardless of add structure."
- verdict: KILLED

## Structural axis status after s3
CLOSED. Union of s2 (8 tail-reassoc + 4 head decl/type/pointer) and s3 (7 chain-balance + 2 arm-duplication) = 21 measured structural forms, none reaching the target fixpoint {base->$a1, var_v0->$v0, partial-before-reload}. Root cause: sched1 front-loads the arg0 reload in every single-basic-block form; only a real BB boundary between the partial-add and the reload keeps the load late, and the only straight-line-C way to create that (arm-duplication) is defeated by cross-jump's need for identical per-arm bytes (base occupies different hard regs per arm). The remaining reachable modality is NON-structural: directed permuter over the coupled sched1/RA fixpoint, or a scheduling-technique-catalog lever that suppresses the load front-load without changing dataflow (do-while(0)/LABEL_OUTSIDE_LOOP_P / reorg-level effects — a separate modality).

## [s3] The {base->$a1, var_v0->$v0, partial-before-reload} coupling is reachable by ARM-DUPLICATION (sink the partial-add into predecessor blocks so sched1's block-local scheduler cannot hoist the join reload above it).
- mechanism: GCC 2.7.2 sched1 is basic-block-local; a partial-add duplicated into each arm lives in a predecessor block, so the join-block reload cannot be scheduled ahead of it. cross-jump would re-merge the identical duplicated adds back to one add at codegen.
- probe: vA (partial=base+var_v0 in all 3 arms) and vB (var_v0=...+base in all 3 arms); mini proxy then full sandbox; objdump of the full-context build.
- result: In the standalone mini it produces the EXACT target tail (partial -> reload -> nop -> lookup -> nop -> sum; var_v0->$v0; the 43rd load-delay nop present). In FULL context WORSE: sandbox 16 / build_insns 47 (5 over). objdump: three partial-adds `addu v1,a2,v0` x2 + `addu v1,a3,v0` x1 — base occupies different hard regs per arm, so cross-jump cannot suffix-merge the non-identical bytes. Pinning base to one reg = register cheat.
- verdict: KILLED

## [s3] Winning the sched1 priority tie (balancing partial's dep-chain length against the reload->lookup->sum chain) keeps the single partial-add before the reload.
- mechanism: If partial's chain-to-block-end equals the reload chain's, the list scheduler's tiebreak might keep partial first (earlier source order).
- probe: sweep3.py: 7 single-partial-add forms (staged +0x12C on partial, p+0x12C+lk, double-stage, lk-named-after-p, commuted sum, 0x12C-in-base, named-reload) through the mini proxy reading reg alloc + reload/partial order.
- result: All 7 stay reload_first. The divergence is NOT a chain-length tie: sched1 FRONT-LOADS the memory reload to hide load latency (a load-priority boost independent of add-side reassociation), so no reassociation/staging of the ADD side suppresses the hoist. Refines s2's 'reload chain is one load longer' to 'loads are front-loaded regardless of add structure.'
- verdict: KILLED

## [s4] A directed/randomized permuter on a clean single-function target closes the coupled sched1/RA fixpoint that structural transforms cannot.
- mechanism: A permuter search over the mutation space (incl. non-dataflow-preserving spellings) may perturb sched1's load-hoist / register pressure into the target fixpoint {base->$a1, var_v0->$v0, partial-before-reload}.
- probe: Built the honest standalone workspace (target.o offset-0 from asm/funcs+prelude). Ran 2 campaigns --stop-on-zero, fresh-seed discipline, waited in-turn, harvested --stop both. Chassis1 = clean candidate (20k iters); chassis2 = base-reuse-for-reload seeded from chassis1's best (45k iters).
- result: PARTIALLY KILLED / MAJOR REFINEMENT. Chassis1 plateaued at weighted-70 (pure reg-rename, build 42) — search confirms no semantics-preserving mutation on the single-BB candidate reaches the fixpoint. Chassis2 (base-reuse) DID reach build_insns 43 (== target) in full context and matched the entire HEAD, isolating the residual to a pure 6-register TAIL coalescing swap — but the only permuter form that flips that swap uses a `base++; base--;` dead-op cheat (weighted 30, still not a byte match). No CLEAN permuter form beat weighted-50 / sandbox-10. The permuter cannot close it cleanly; it flips the tail RA only via forbidden coercions.
- verdict: KILLED (clean permuter closure). The permuter reproduces the s2/s3 structural wall from the search angle and additionally proves the base-reuse chassis's 43-insn residual is closable only by a dead-op cheat.

## [s4] The build-42 (1-insn-short) wall is intrinsic to every single-basic-block C form.
- mechanism: s2/s3 claimed only arm-duplication reaches build 43 (and cross-jump defeats it). Tested whether a single-BB variable-reuse form can reach 43.
- probe: Measured 3 base-reuse/var-reuse single-BB forms live in the full-context sandbox.
- result: KILLED. A single-BB "reuse `base` for the *arg0 reload" form reaches build_insns 43 in full context (form-50: sandbox 10 / build 43). So the 1-insn shortfall is NOT intrinsic to single-BB forms — reusing a dying variable's register for the reload creates the pressure that forces the partial-add before the reload (the 43rd nop appears). HOWEVER reaching 43 does not lower the floor: the freed 1-insn is replaced by an equivalent register-rename residual (sandbox 10-16, never < 10). The wall moved from "insn count" to "tail RA coalescing", not removed.
- verdict: KILLED (the "42 is intrinsic" framing); floor unchanged.

## [s4] A directed/randomized permuter on a clean single-function target closes the coupled sched1/RA fixpoint that structural transforms cannot.
- mechanism: A permuter search over the mutation space may perturb sched1's load-hoist / register pressure into the target fixpoint {base->$a1, var_v0->$v0, partial-before-reload}.
- probe: Built honest standalone workspace (target.o offset-0 from asm/funcs+prelude, single-fn maspsx assembled whole; base 42 vs target 43 reproduces the exact known gap). Ran 2 campaigns --stop-on-zero with fresh-seed discipline, waited in-turn, harvested --stop both. Chassis1=clean candidate 20115 iters; chassis2=base-reuse-for-reload seeded from chassis1 best, 45349 iters.
- result: Chassis1 plateaued at weighted-70 (pure register-rename, build 42) fast and never improved. Chassis2 reached build_insns 43 (==target) and byte-matched the entire HEAD, isolating the residual to a pure 6-register TAIL coalescing swap; but the only permuter form that flips it uses `base++; base--;` (dead no-op pair = forbidden dead-computation coercion, weighted 30, still not a byte match). No CLEAN form beat weighted-50 / sandbox-10.
- verdict: KILLED

## [s4] The build-42 (1-insn-short) wall is intrinsic to every single-basic-block C form (s2/s3 claimed only arm-duplication reaches 43).
- mechanism: Reusing a dying variable's register for the *arg0 reload creates register pressure that forces the partial-add before the reload, so the lh load-delay nop (43rd insn) survives even in a single basic block.
- probe: Measured 3 base-reuse/var-reuse single-BB forms live in the full-context sandbox: form-50 (a2=base+var_v0; base=*arg0), clean base-reuse-in-tail, var_v0-reuse-for-reload.
- result: KILLED. A single-BB 'reuse base for the reload' form reaches build_insns 43 in the REAL full-context sandbox (form-50: sandbox 10 / build 43; clean base-reuse: 11/43; var_v0-reuse: 16/43). So 42 is NOT intrinsic to single-BB forms. HOWEVER reaching 43 does not lower the floor: the freed insn is replaced by an equivalent register-rename residual (sandbox 10-16, never <10). The wall moved from 'insn count' to 'tail RA coalescing', not removed.
- verdict: KILLED

## [s5] A double variable-reuse tail (`base += var_v0; var_v0 = *arg0;`) reaches the target 43-insn structure AND lowers the floor below 10.
- mechanism: partial computed in base's register (target $a1), arg0 reload reuses var_v0's dying register (target $v0) — exactly target's tail dataflow. Reaching build_insns 43 (the load-delay nop) requires this reuse; single-BB non-reuse forms stall at 42 (s2/s3).
- probe: Wrote f_doublereuse.c, measured live in the real sandbox (--disable all); objdump-diffed vs target; scored 3 partial variants (fB/fC/fD) to confirm both reuses are load-bearing.
- result: CONFIRMED + FLOOR IMPROVED 10 -> 9. Sandbox score 9 / build_insns 43 == target (first floor drop since import). Both reuses load-bearing (dropping either -> diff 16/17). The entire score-9 residual is a single $a1<->$a2 register swap between a2local(*arg0) and base(a3*40): target a2local->$a2/base->$a1; ours swapped.
- verdict: CONFIRMED

## [s5] The score-9 residual is a pure RA priority tiebreak; flipping base->$a1 needs base to out-ref a2local, unreachable by any target-faithful clean structure.
- mechanism: global.c colors pseudos by priority (~floor_log2(n_refs)/live_length). a2local (pseudo 73, 5 refs) outranks base (pseudo 77, 2 refs), so a2local is colored first and grabs $a1, forcing base to $a2. Target has base->$a1, i.e. base must be colored first.
- probe: cc1 -da greg dump on f_doublereuse (f_doublereuse.c.greg): read the 4-pseudo priority order + dispositions + conflict/preference lists. Tried clean flips: declare-base-first (v3), base-via-shifts (v5), a2-as-pointer (v6), a3-reuse-as-base (w1/w2), do-while(0) wrap (f_dowhile). Ran a 28.5k-iter directed permuter from the score-9 seed.
- result: KILLED (clean flip). v3/v6 = diff 9 (same swap), v5 = diff 12, w2 = 42/diff8 (loses reload nop), do-while alone = diff 9. The permuter's ONLY weighted-0 form is `base++; base--;` (dead-op supplying base's 2 missing refs) = forbidden dead-op cheat (same as s4, reconfirmed on better chassis). Its weighted-10 forms overwrite base inside an arm = semantically broken. Target-faithful base has exactly 2 refs (def + partial-add); no clean structure adds refs without diverging from target's instruction stream. Neither a2local nor base carries a copy-preference to steer via, either.
- verdict: KILLED (for the clean-flip levers tried); floor now 9. Un-tried: manufacturing a legitimate copy-preference to bias base->$a1 (frontier).

## [s5] A double variable-reuse tail (base += var_v0; var_v0 = *arg0;) reaches the target 43-insn structure and lowers the floor below 10.
- mechanism: partial is computed in base's register (target $a1) and the arg0 reload reuses var_v0's dying register (target $v0) — exactly target's tail asm (addu a1,a1,v0; lw v0,0(a0); nop; lh v0,1034(v0); nop; addu v0,a1,v0). Reaching build_insns 43 (the load-delay nop) requires this reuse; single-BB non-reuse forms stall at 42.
- probe: Wrote f_doublereuse.c, measured live in the real sandbox --disable all (score 9, build_insns 43); objdump-diffed vs target; scored partial variants fB/fC/fD to confirm both reuses are load-bearing.
- result: CONFIRMED: sandbox 9 / build 43 == target (first floor drop since import). Both reuses load-bearing (drop either -> diff 16/17). Entire residual is a single $a1<->$a2 swap of a2local(*arg0) and base(a3*40).
- verdict: CONFIRMED

## [s5] The score-9 residual is a pure RA priority tiebreak; flipping base->$a1 needs base to out-reference a2local, unreachable by any target-faithful clean structure.
- mechanism: global.c colors pseudos by priority (~floor_log2(n_refs)/live_length). a2local (pseudo 73, 5 refs) outranks base (pseudo 77, 2 refs) so a2local is colored first and grabs $a1, forcing base to $a2. Target has base->$a1 (base must be colored first). Neither carries a copy-preference to steer.
- probe: cc1 -da greg dump on f_doublereuse (priority order 82,73,77,72; dispositions + conflicts + preferences). Clean-flip attempts: declare-base-first (v3), base-via-shifts (v5), a2-as-pointer (v6), a3-reuse-as-base (w2), do-while(0) wrap (f_dowhile). 28.5k-iter directed permuter from the score-9 seed.
- result: KILLED for clean flips: v3/v6 diff 9 (same swap), v5 diff 12, w2 42/diff8, do-while-alone diff 9. Permuter's ONLY weighted-0 form = base++; base--; (dead-op supplies base's 2 missing refs) = forbidden dead-op cheat, same as s4, reconfirmed on the better chassis. Its weighted-10 forms overwrite base inside an arm = semantically broken (target computes arms into fresh $v0). No clean form beat sandbox 9.
- verdict: KILLED

## [s6] The score-9 residual ($a1<->$a2 swap) is flippable by a legitimate copy-preference biasing base->$a1 (frontier-1).
- mechanism: prune_preferences propagates a lower-priority conflicting allocno's full-preference into the higher-priority allocno's regs_someone_prefers; if base had a full-preference for $a1, a2local (colored first) would avoid $a1 in find_reg pass 0 (global.c:970), take $a2, and free $a1 for base.
- probe: Instrumented cc1 (tmp/gccdbg/cc1) ALLOCDBG + FINDREGDBG dumps on the double-reuse floor-9 form; read set_preference (global.c:1591) + expand_preferences (797) + find_reg (921-1126); sweep.py (3 structural variants); confirm.py + verify_pin.sh (diagnostic $5 pin).
- result: KILLED. A hard-reg preference is created ONLY by a reg<->hard-reg COPY insn (set_preference), and $a1 has NO ABI anchor in this 1-argument leaf — base is defined by sll and consumed by addu, no copy to a hard reg; expand_preferences can't merge one in because base conflicts with var_v0. No target-faithful C produces a base->$a1 preference. The diagnostic $5 pin confirms base->$a1/a2local->$a2 IS the target allocation but reschedules to 41 insns (loses 2 nops), so even a pin can't reproduce the coupled 43-insn fixpoint; only the anchor-less soft preference would.
- verdict: KILLED

## [s6] A sanctioned scheduling wrapper (do-while(0)) shrinks base's live_length below a2local's (flips the allocno priority) without dead refs (frontier-3).
- mechanism: if a scheduling wrapper shortened base's live range so base's priority (floor_log2(nref)*nref/len) exceeds a2local's 8571, base would be colored first and take $a1.
- probe: sweep_sched.py — do-while(0) around the tail, the whole dispatch+tail, and the multiply; ALLOCDBG after each.
- result: KILLED. base's live_length stays 21 in every placement; a2local's priority always exceeds base's (do-while inflates ref counts via loop notes but never shrinks base's live range). base stays $a2 in all three. Confirms the mechanistic prediction: base MUST cross the join (used in the tail) so its live range cannot shrink, while a2local MUST die at the join (the 43rd load-delay nop requires a FRESH *arg0 reload, not a2 reuse) — so a2local is inherently shorter-lived and higher-priority. Priority-flip requires base n_refs >= 8 (byte-neutral ref-lift), which reduces to the s3-killed cross-jump-merge-fails arm-duplication.
- verdict: KILLED

## Sanctioned-axis status after s6 — OWNER-ESCALATION
CLOSED. Structural (s2/s3, 21 forms), permuter (s4/s5, 4 chassis ~150k iters), copy-preference (s6), scheduling-wrapper (s6), and ref-lift (=s3) all measured dead for a committable COMPLETED-C form. Honest floor 9 (single $a1<->$a2 RA swap), byte-match on main held by a register asm("$5") pin. scan_hand_coded LOW 0/8 (canonical-asm not supportable). OWNER-ESCALATION filed docs/grind/decisions.md 2026-07-23 (grind s6 forensics); owner-gated per endgame-lock-disposition-policy + frontier-2. Do not self-resolve; re-attempt only on a novel whole-function reshape that changes the base/a2local live-range or preference relationship.

## [s6] The score-9 residual ($a1<->$a2 swap) is flippable by a legitimate copy-preference biasing base->$a1 (frontier-1).
- mechanism: prune_preferences (global.c:851) propagates a lower-priority conflicting allocno's full-preference into the higher-priority allocno's regs_someone_prefers; if base had a full-preference for $a1 (reg 5), a2local (colored first) would avoid $a1 in find_reg pass 0 (global.c:970), take $a2, and free $a1 for base.
- probe: Instrumented cc1 (tmp/gccdbg/cc1) ALLOCDBG + FINDREGDBG dumps on the double-reuse floor-9 form; read set_preference (global.c:1591), expand_preferences (797), find_reg (946-1126); sweep.py (3 structural variants); confirm.py + verify_pin.sh (diagnostic $5 pin).
- result: A hard-reg preference is created ONLY by a reg<->hard-reg COPY insn (set_preference), and $a1 has NO ABI anchor in this 1-argument leaf: base is defined by sll and consumed by addu (no copy to a hard reg); expand_preferences cannot merge one in because base conflicts with var_v0. No target-faithful C produces a base->$a1 preference. FINDREGDBG confirms neither base(p77) nor a2local(p73) carries any register preference (only var_v0->$v0 return-pref, arg0->$a0 arg-pref exist).
- verdict: KILLED

## [s6] A sanctioned scheduling wrapper (do-while(0)) shrinks base's live_length below a2local's, flipping the allocno priority so base is colored first and takes $a1 (frontier-3).
- mechanism: allocno_compare priority = floor_log2(n_refs)*n_refs/live_length*10000*size; a2local(p73)=8571 [6ref/len14] > base(p77)=3809 [4ref/len21]. Shrinking base's live_length below ~9 (or lengthening a2local's) would flip the coloring order.
- probe: sweep_sched.py: do-while(0) around the tail, the whole dispatch+tail, and the multiply; ALLOCDBG after each.
- result: base's live_length stays 21 in every placement; a2local's priority always exceeds base's (do-while inflates ref counts via loop notes but never shrinks base's live range). base stays $a2 in all three. base MUST cross the join (used in the tail); a2local MUST die at the join (the 43rd load-delay nop needs a FRESH *arg0 reload, not a2 reuse), so a2local is inherently shorter-lived/higher-priority. Cutting a2local refs (sweep v3, hoist 0xE) backfires: refs 6->5 but live range 14->11 raises pri 8571->9090. A diagnostic register-asm $5 pin DOES give base->$a1/a2local->$a2 but reschedules to 41 insns (loses 2 nops) -- even a pin cannot reproduce {base$a1, a2local$a2, 43-insn}; only the anchor-less soft preference would.
- verdict: KILLED
