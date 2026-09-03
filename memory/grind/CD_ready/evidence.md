# Evidence bank — marionation_Exec

- WIP rejected_form: {'form': 'vT33 (in-call tbl-add) / vT34 (fresh sum) / vT35-vT36 (wrapper/nesting reweights)', 'score': '16 / 11 / 15 / 14', 'reason': 'fresh single-set temps LAUNCH (birthing_insn_p) and re-time the head; do-while(0) note streams shift LUIDs and perturb sched. The pair fix must not add pseudos or notes inside the do_timeout window.'}

- WIP rejected_form: {'form': 'vU1/vU2 (u8-typed checks, mask vars removed, real-loop family)', 'score': '17', 'reason': "PROMOTE_MODE keeps u8 locals SI-extended; combine merges lbu+extension when the value dies (lbu a2 direct, andi GONE). The immediate andi REQUIRES the mask-var reload-substitution, which requires the goto-loop refs==2 update_equiv_regs fold. Real-loop family stays dead (s8 conservation when masks exist; no andi when they don't)."}

- WIP rejected_form: {'form': 'vRD (check2 do-while-break) / vRK (tail while-goto)', 'score': '14 / 4 (inert)', 'reason': 'exit-label restructure perturbs the region; while-goto compiles identically to the goto tail.'}

- WIP rejected_form: {'form': 'R_reb_y1_iq3 (rejected/cross-symbol-fake-plus-double-split.c)', 'score': 'masked 6 (mirage lineage)', 'reason': 'layer-1 FAIL 2026-07-05 + user DO-NOT-SANCTION ruling. Historical; superseded by the honest vDT30/vT31 lineage which reached 4 without it.'}

- WIP rejected_form: {'form': 'outer cycle as real do-while / while(1) (any spelling with LOOP notes around the checks)', 'score': '18+ (vDT48 family)', 'reason': "flow loop-weighting breaks the masks' refs==2 fold; LICM promotes exactly one invariant to s8 (conservation barrier). Goto-loop is mandatory for the check region."}

- == imported from memory/wip notes.md ==
# marionation_Exec — WIP (session-10 final: masked 4 SANCTIONED; 2 root-caused residuals)

## TL;DR (current state) — FINAL RULING: SANCTIONED (construct-honesty line)
**candidate.c = vT40, masked 4 (verified), sanctioned.** After a full SOTN-evidence
re-evaluation the owner issued the FINAL 2026-07-06 ruling (do-while-zero-exception.md
rewritten): do-while(0) wraps are allowed for ANY codegen effect incl. register
allocation, FAKE-annotated per site; nested wraps need a single-level-insufficient
note (candidate complies). The interim not-sanctioned ruling (<1 day) is superseded;
rev-vt31's FAIL correctly applied the OLD rule and is moot. STILL FORBIDDEN:
regfix/pins/__asm__/semantic-lie C (cross-symbol derivation per 2026-07-05).
`saved` widening temp dropped (plain `& 3` scores identically). Fresh layer-1
(rev-vt40) reviewing under the new rule — verdict to be recorded. Remaining gap =
the two ROOT-CAUSED residuals below. Permuter running on the vT40 base.
`src/system.c` untouched, oracle green.

## Residual 1 — pair-swap @56/57 (2 masked pts): FULLY CHARACTERIZED
- **Order half SOLVED (vT32, progress/)**: put the `arg5 = *(s32*)(v0+(s32)tbl_125c);`
  statement BEFORE `t0 *= 4; t0 = tbl + t0;` (loads stay first). LUID tie flips →
  `addu v0,v0,s5` then `sll a0` ✓ target order.
- **Cost: the two temps exchange seats** (t0-web→v1, arg5val→a0; target wants a0/v1).
  Root (QTYDBG, local-alloc.c qty_compare): pri = floor_log2(refs)·refs·size/life;
  ours: addr-temp 102 {r4 l4}=8.0 → v0 ✓; t0-web 104 {r4 l6}=5.33; arg5val 97 {r4 l6}
  =5.33 — EXACT TIE, broken by qty birth order (104 first → v1). Target needs 97 ≥
  104: arg5val needs weighted refs 6 (density 8.0) or t0-web needs refs 2.
- **Measured dead**: fresh temps for shift or sum re-time the head (launch: vT34=11,
  vT33=16); ANY note insertion inside the do_timeout window re-times the head — wraps
  around the arg5 stmt (vT35=15, vT36=14 double-nest, vT42=14 single-nest) AND around
  the call itself (vT43=12) all scramble sched. Post-sanction probes 2026-07-06: the
  wrap toolbox CANNOT fix the pair. Only a natural second arg5 ref (identity ops all
  tree-fold) or different statement geometry. Flat respellings canonicalize (6 forms = 8).
- **Session-10b decode (sched1 dump, tmp/sched1dump.py): the pair is a COUPLED FIXED
  POINT.** vT40 (t0-stmts first): arg5-addr temp is TIGHT (add+lw adjacent, density 32
  -> v0) and seats all come out right, but the t0-sll's lower LUID wins the sched2 tie
  -> order swapped. vT32 (arg5 first): LUIDs flip the order right, but sched1 stretches
  the arg5-addr temp (life 4) and the t0-web/arg5 qtys tie (5.33 v 5.33) -> seats trade.
  Statement order controls BOTH the sched tie AND the qty lives; every hand decomposition
  fixes one and breaks the other. 9-variant tail/check2 topology sweep: ALL identical
  4/178 (the region-3 steal is invariant to C-level topology).
- Twin lore (cpu_side_move_dir_4 g3): same pair; same coupling likely.

## Residual 2 — region-3 dbr steal @149 (2 masked pts): MECHANISM BYTE-PROVEN
- reorg processes fill_simple for ALL insns first: check1's `dst=a1` move is eaten by
  beqz-s4's slot (simp nearest-first) → out of the steal window → check1 nop ✓ free.
  check2's window keeps its move (li v1,7 is nearest to beqz-a1 instead) → the eager
  fall-through fill steals `move a1,s4` into check2-beqz's slot (WINNER trial=450).
- **Knob byte-proof**: canonical cc1 has env-gated what-if knobs;
  `BB2_ALLLIVE_LABEL=513,627` (the tail thread insn + its pass-2 SEQUENCE uid) forces
  all-live at the tail → move REJECTED (setsopp=1) → asm == target region-3 EXACTLY
  (beqz;nop;sb;move + backedge beqz s7;move v0,zero). Diagnostic only — NOT a lever.
- **Impossibility results (don't re-derive)**: genuine a1-liveness at the tail is
  impossible in ANY spelling (every pseudo live there crosses the loop's calls →
  callee-saved only; flow/forward-scan both accurate). own_fallthrough=0 needs a
  CODE_LABEL right after beqz-a2 with surviving uses — semantically impossible
  (do-while(0) top labels get deleted; while(cond-false-but-unprovable) backedges add
  bytes). Young-label→find_basic_block(-1)→all-live is the only reachable route and
  needs a post-flow label; cross-jump does NOT fire in this toolchain config (proven:
  our two identical `j epi; move v0,a2` tails stay unmerged in ours AND target).
- **Prediction is NOT the blocker**: vT31's tail-wrap (`do { tail: if (a0==0) goto
  loop; } while(0); return 0;` — LOOP_BEG right before the interior label) makes
  mostly_true_jump return 2 (verified likely=1 in DBRDBG) — but after the target-thread
  fill fails, reorg still fills from an owned fall-through. Keep the wrap anyway
  (harmless, matches the backedge/v0=0 slot shape naturally).

## The goto-loop recipe (vDT30/vT31 base — unchanged, still load-bearing)
GCC 2.7.2 weights refs only inside LOOP_BEG loops; the outer cycle stays a goto-loop
(masks fold via update_equiv_regs refs==2 → immediate `andi ,0xff`; no LICM → no s8);
do{}while(0) wrappers weight chosen regions: do_timeout (tbl→s5), poll (i1494/i1495),
idx_1496 clears (double-nest first clear → i1496 over arg1). saved widening temp
`{s32 _b; _b=*D_800A147C_2; saved=_b&3;}`. Masks + copy blocks stay unwrapped.
REAL-LOOP family is a dead end: LICM promotes exactly one invariant to s8 (conservation
barrier); u8-typed checks kill the masks entirely (vU1/vU2=17: PROMOTE_MODE keeps u8
locals SI-extended, combine merges lbu+extension when the value dies — andi needs the
mask-var reload-substitution, which needs the goto-loop refs==2 fold).

## DBR/QTY tooling (all in tmp/, worktree bb2-work-marion)
- probe.py (splice+sandbox+greg ledger), adiff.py (LCS diff), dumpours.py.
- dbrdbg.py / candbr*.py (BB2_DBR_DEBUG traces), qtydbg.py (BB2_QTY_DEBUG),
  rtlorder.py / notecheck.py / pseudomap.py (RTL dumps), knobs.py / knobsb.sh
  (what-if knobs vs canonical cc1). Canonical cc1 knobs: BB2_ALLLIVE_LABEL,
  BB2_DBR_DEBUG, BB2_NO_FT_STEAL (env-gated, inert unset; oracle green proves it).
- gccdbg cc1 lacks ALLLIVE — use ../../tools/gcc-2.7.2/cc1 for that knob.

## SESSION-10c CLOSURES (2026-07-07) — read before trying anything
- **EXHAUSTIVE ordering sweep (tmp/ordersweep.log): all 140 dependency-valid
  do_timeout interleavings measured — floor is masked 4 (63×4, 49×6, 21×9, 7×8,
  zero hits, insns pinned 178).** The pair cannot fall to statement order alone.
- **cc1psx parity (tmp/psxregion3.py): PsyQ's own cc1 emits the IDENTICAL region-3
  steal** for our source — the compiler fork is NOT the variable; the original
  SOURCE was shaped differently in a way not yet guessed.
- **Permuter masked-3 signpost (archive: output-160-1 / tmp/vP160.c): `while(status)`
  backedge on the clear keeps the loop-top label alive → own_fallthrough=0 → steal
  dies, region-3 aligns — but pays an extra bnez (180 insns, unmatchable) AND reads
  status uninitialized on {first iteration ∧ VBlank==0} (semantically divergent —
  REJECTED).** Confirms: every label route pays a visible byte; a label between the
  sb and the move stops the scan (own_thread=0 after lose=1) but no semantic jumper
  exists and manufactured ones cost a jump insn (jump1 cleans adjacent-jump forms
  back to the attractor).
- **vT45 dst/dst2 merge on the vT40 chassis: masked 19** (s-web collapses; W1-style
  compensation is the known dead end at 9). Merge axis CLOSED.
- Inline-helper shape: unviable a priori (the two copy blocks have DIFFERENT byte
  shapes; a shared helper forces them identical).

## NEXT SESSION
1. The ONE active lever: the rich-pass permuter (tmp/perm_mar, vT40 base, watcher
   auto-triages sub-200 finds into triage.log). It found the masked-3 signpost in
   <1h — it samples the unknown-source-shape space directly. Check triage.log.
2. If a find reaches masked ≤2 with true semantics: verify (probe+adiff), vet
   constructs (no volatile/cross-symbol/uninitialized reads), integrate.
3. Region-3 unknown-shape ideas not yet tried: none remaining from analysis — trust
   the sampler, or dump target-adjacent functions for structural analogies (Kengo
   was a dead end per slog-kengo-dead-end).
4. On masked 0: retire 42 rules, full SHA1, LAYER-2 review (MUST independently rule
   on the nested wrap — see meta.json reviewer entry), queue done, delete WIP.

## Variant ladder (masked)
candidate.c/vT31: 4 ← BEST. vT32 (order fix, temps traded): 8. vDT10: 6 (pre saved-fix).
vT33 in-call add: 16. vT34 sum-split: 11. vT35/vT36 nest-reweight: 15/14. vU1/vU2
(u8 checks, real loop, no s8): 17. vDT48 real-loop: 18. m2c rebuild: 65.


- [s1] Baseline on main 2026-07-07: sandbox --disable all = 4 (178/179 insns, rules_dropped 42), canonical verdict C distance 4 — ledger floor reproduced exactly; src/system.c restored to HEAD after measurement, oracle green.

- [s1] diagnose reports LARGE d70 — that is the rules-ENABLED view and is expected with 42 rules; the honest disable-all score (4) is the governing gradient. Do not treat d70 as a regression.

- [s1] INFRASTRUCTURE: all session-10 scratch (rich-pass permuter workspace, watcher/triage.log, vP160.c, ordersweep.log, sched1dump/psxregion3/probe/adiff/qtydbg/dbrdbg/knobs scripts) lived in the removed bb2-work-marion worktree's gitignored tmp/ and is GONE from main. The next permuter session rebuilds from scratch: vT40 base.c + single-function offset-0 target.o (asm/funcs/marionation_Exec.s + decomp-permuter prelude.inc, drop `.set gp=64`), rich pass set, --stop-on-zero, watcher auto-triaging sub-200 finds; reject finds with untrue semantics (vP160-class uninitialized reads).

- [s1] tmp/perm_mar's July-3 campaign scored ~237k = whole-file offset-noise mode (the exact failure mode difficult-is-not-impossible warns about); tmp/perm_mar6 is an offset-0 workspace but pre-vT40 (best 200). Neither is resumable as-is.

- [s1] tmp/marion_handoff/FINDINGS.md is session-6-era (honesty-correction, masked-22/30 accounting) — SUPERSEDED by the sanctioned vT40 lineage at 4; keep only as history, do not act on its 'next steps'.

- [s1] Twin cpu_side_move_dir_4: parked, distance 7, 5 rules — no transferable pair-swap lever exists yet.

- [s1] tmp/duplicates_leads.txt does not exist on main (find_duplicates.py not run since merge).

- [s2] [s2] 26 structural variants measured across 4 sweeps (tmp/grind/marionation_Exec/s2/sweep1-4.json); NONE below 4. The 4-club: baseline vT40, v08/v20 (third order, seats intact), v09/v21 (emission-identical to baseline), 6 decl/type forms (inert).

- [s2] [s2] NEW mechanism fact: v08/v20 (v0-shift merged into arg5's load expression) emit window order (sll a0; sll v0; addu v0,v0,s5) at masked 4 with all seats matching target — proves the sched2 tie moves under geometry WITHOUT the vT32 seat trade. But only the two addu-LAST orders are reachable from t0-first source; addu-in-middle (target) requires arg5-first, and arg5-first = seats trade in EVERY geometry (v13 split=8, v14 merged=8, v15 t0-merged=8).

- [s2] [s2] Decl order of block locals is qty-INERT (all 6 permutations byte-identical): pseudo/qty birth order follows RTL first-use, not declaration order. Type narrowing s32->u32 on t0/arg5 also inert. Do not re-sweep these axes.

- [s2] [s2] arg5 self-staged address (the refs-6 tie-winner idea from the s1 frontier) is DEAD: masked 11 in both positions — the eliminated addr-temp (qty 102, seats v0 correctly in vT40) is load-bearing; absorbing it into arg5's web re-times the head like a launch.

- [s2] [s2] adiff tooling rebuilt on main: tmp/grind/marionation_Exec/s2/adiff.py (splice + cheat-stripped sandbox + objdump + LCS vs asm/funcs/marionation_Exec.s). Baseline diff re-confirms exactly the 2 ledger residuals: pair order at insns 55-57, region-3 steal at 149 (build: move a1,s4 in check2-beqz slot; target: nop there, move at 151). Reusable for triaging permuter finds by window order.

- [s2] [s2] src/system.c restored to HEAD after all measurement; oracle untouched; candidate.c remains vT40 (reviewed, annotated best).

- [s3] [s3] Floor 4 re-confirmed on main at session start (sandbox --disable all: 4, 178/179, 42 rules dropped) with candidate.c spliced; src/system.c restored to HEAD after all measurement, oracle untouched.

- [s3] [s3] NEW mechanism fact: the pair-swap seat trade follows addu-LUID-early per se, NOT arg5val's priority/life - 8 forms with the addr in the v0 staging web (1-stmt/2-stmt, deref early/mid/late) all score 8 with the identical order-right/seats-traded window. The s2 hypothesis that the trade was a qty-tie artifact breakable by life-shaping is dead.

- [s3] [s3] NEW mechanism fact: extending the sanctioned v0-staging web from 2 to 3 statements (split load/shift/add) re-times the head at 17, uniformly across ALL statement positions - the launch pathology applies to web GROWTH of existing pseudos, not just fresh temps.

- [s3] [s3] Plus-operand order is fully canonicalized at expand (tbl-first spellings emission-identical); array-index spelling reaches no new window order. The t0-first reachable-order set {addu-last x2} is now confirmed from 3 independent spelling families.

- [s3] [s3] 22 new structural forms measured this session (sweep1/2/3.json), none below 4; combined with s2 the hand-structural axis for the pair-swap is closed across: 140 orderings, 26+22 geometries/decompositions, decl order, types, staging, operand order, web decomposition, deref position.

- [s3] [s3] rejected/ additions: decoupled-addu-3stmt-v0web-head-retime-17.c, addr-in-v0web-addu-early-seats-trade-any-deref-8.c

- [s4] [s4] Floor 4 re-confirmed on main at session start (sandbox --disable all: 4, 178/179, 42 rules dropped) with candidate.c spliced; src/system.c restored after every measurement, working tree src/ clean.

- [s4] [s4] Campaign infrastructure rebuilt at tmp/grind/marionation_Exec/s4/: perm (vT40 base, permuter base score 220), perm_b (find145 base), perm_c (find105 base); watcher.py auto-triages every find's pair window [50:63] + region-3 window [144:157] into triage.log; adiff2.py (fixed normalizer: $-strip, hex->dec, %hi/%lo->SYM) shows true hunks only.

- [s4] [s4] NEW mechanism fact: reorg's region-3 steal dies WITHOUT paying a byte when the stolen move's dest is callee-saved and live at the backedge target - reachable honestly by merging the idx web into dst2 (staged-value-reused-variable, sanctioned family). 179/179 insns, true semantics. Cost is structural: the merged web spans the loop calls, forcing a callee-saved seat, but target's move dest is a1 (caller-saved) - so this route can NEVER byte-match; family floor masked 10.

- [s4] [s4] The permuter metric and the engine masked metric DIVERGE on this function once reg diffs dominate: permuter 145 = masked 17, permuter 105 = masked 10, permuter 95 = masked 10. Triage every find through the sandbox (adiff2), and read permuter scores only as: <60 would imply the pair reordering fixed (reorder=60), 220 = baseline.

- [s4] [s4] Sampler independently re-confirmed the vP160-class closure: all non-liveness steal-kills in ~10k vT40-base iterations pay +1 visible insn (180 total: dead sb zero,24(sp) store or extra bnez s0) and some are semantically divergent (sb s0,-1(s3) stores non-zero) - none usable.

- [s4] [s4] Pair-swap coupling (addu-LUID-early <=> seats-trade) verified CHASSIS-INVARIANT: on the find105 chassis all arg5-first geometries pay the same +4 (14/15 vs 10) that vT32 paid on vT40 (8 vs 4); t0-first shift-merge is neutral (10). Sweep: tmp/grind/marionation_Exec/s4/f105vars/ (4 forms).

- [s4] [s4] Prior invalid-session scratch (02:12-04:29 attempts, perm2-perm7, campaign1b-7 logs, old triage entries with 'permuter-score=' format) coexists in s4/; the old attempts' best was 195 with sandbox 23 - fully superseded by this session's finds. Old perm workspace output-160-1/200-1 dirs were stale carryovers; every dir this session's watcher triaged was re-verified fresh.

- [s4] [s4] Campaigns left RUNNING detached under WSL at session end for s5 to harvest: perm_c (find105 base, -j24, pid in s4/campaign_c.pid), perm (vT40 base, -j6, pid in s4/campaign.pid), watchers on both; check s4/triage.log + output-* dirs. Kill via the pid files if rebuilding.

- [s5] Baseline confirmed: HEAD src/system.c sandbox = 56 (176 insns); candidate.c (vT40) sandbox = masked 4 (178 insns, 42 rules dropped, 20 cheat-asm stripped) — floor unchanged after 5 sessions.

- [s5] Permuter metric vs engine masked metric divergence FURTHER quantified in the alias-merge family: perm score 145 -> 105 -> 95-1 -> 85 tracks masked 17 -> 10 -> 10 -> 14 -> 15 (the 95-1 masked-10 point remains the family's true minimum; 85 is a REGRESSION on masked despite better permuter score).

- [s5] perm campaign (vT40 base, still running under WSL, PID in s4/campaign.pid) has produced NO sub-145 finds since s4 recorded (~8h+ of additional sampling). The vT40 base's local basin around masked-4 is empty of masked<=3 forms except the label-alive class (+1 insn).

- [s5] perm_c campaign (find105 base, still running under WSL, PID in s4/campaign_c.pid) has produced output-85-1 and output-95-2 since s4; both are masked-worse than ledger's family floor 10.

- [s5] NEW label-alive spelling class discovered: 0xFF-through-local (`new_var = 0xFF; ... & new_var`) + `while (status = 0)` sentinel — outputs the masked-3 target region but at 180 insns and with cross-block status-read semantics (same rejection tier as vP160's `while(status)`). No new lever, but records the sampler's third distinct route to the +1 masked-3 attractor, tightening the impossibility argument for zero-cost liveness/label kills of the region-3 steal.

- [s5] perm/output-145-2 = masked 17 confirmed (find145-class alias-merge with dst2->s1 seat). Sampler has re-derived the family's masked-17 entry point at least twice from the vT40 base, confirming the alias-merge attractor is the dominant sub-200 mutation trajectory.

- [s5] src/system.c untouched after all splice/measure cycles (extract_and_score.py restores; git status clean); oracle green.

- [s6] Baseline reconfirmed: candidate.c (vT40) spliced -> sandbox --disable all = masked 4 (178/179, 42 rules dropped, 20 cheat-asm stripped); src/system.c restored to HEAD after all measurement (oracle green).

- [s6] cc1 -da dumps produced fresh on main via tools/gcc-2.7.2/build/cc1 through the standard cpp | cc1 pipeline with candidate spliced; 13 RTL pass dumps + emitted .s written to tmp/grind/marionation_Exec/s6/dumps/ (rtl, jump, cse, loop, cse2, flow, combine, sched, lreg, greg, jump2, dbr, sched2).

- [s6] Pair-swap insn identities from greg dump: insn 106 = (set (reg:SI 4 a0) (ashift:SI a0 2)), insn 111 = (set a0 (plus a0 s5)), insn 117 = (set v0 (ashift v0 2)), insn 120 = (set v0 (plus v0 s5)), insn 122 = (set v1 (mem v0)), insn 128 = (set v0 (zero_extend (mem D_800A11D5))), insn 137 = (set (mem sp+16) v1) — arg5 stack push.

- [s6] sched2 BB #3 schedule (T-index → insn scheduled, backward walk): T-6 emits 145, T-7 143, T-8 137, T-9 133, T-10 111, T-11 128 (before 111), T-12 122, T-13 120, T-14 106, T-15 117, T-16 141 (99 before 117), T-17 115, T-18 99, T-19 93, T-20 91. Chronological emission order (T-20 first, T-1 last): the pair window emits as 117, 106, 120, 122, 128 → matches mar_system_s6.s lines 1155-1159 exactly.

- [s6] T-14 tie mechanism: ready list = [insn 106 (ashift a0, pri=2), insn 141 (lw a1 = D_800F19C0, pri=1), insn 117 (ashift v0, pri=2)]. sched.c reorders to [106, 117, 141]. Neither 106 nor 117 flagged 'greater potential hazard' (unlike the T-8/T-12/T-13/T-14/T-15/T-16 memory-hazard reorders visible on 585/583/581/579/577/575/573/569 in BB #0). Tie broken by insn UID (106 < 117 → 106 emitted 7th).

- [s6] Target order requires either (a) UID(v0-sll) < UID(a0-sll) which comes from arg5-tree expanded before t0-tree — the arg5-first C source class (measured in s2 as coupled fixed point: seat trade at qty_compare tie 5.33 v 5.33), or (b) insn 120 (addu v0,v0,s5) ready at T-14 or earlier — blocked by 120's user chain 120→122→137→jal at T-12/T-8/T-3.

- [s6] Region-3 mechanism from mar.dbr pass summary: 'Reorg pass #1: 3 insns needing delay slots, 2 got 0 delays, 1 got 1 delays'. The 1 filled slot is the check2 branch_zero (line 1276 beq $6,$0,.L137) with the fall-through move $5,$20 (dst2 = a1). Target keeps this slot nop by having its own check2 branch_zero (71A80 beqz $a2,.L800812C4) not steal — because all pseudos live at .L800812C4 cross the outer loop's calls (callee-saved only) and $5=a1 is caller-saved, absent from target-live set.

- [s6] The dep-DAG explaining why no C-level lever fixes the pair: greg shows insn 137 (sw v1,16(sp)) is the arg5 stack-arg push for debug_printf (jal insn 152). All args (5, 6, 7 = D_800F19C0, D_800A11DC[D_800A11D5], *(s32*)t0) are dead-set into their arg regs at jal setup, so their loads emit T-6..T-8. This tightly packs backward ready-times; no C source rewrite that preserves the debug_printf 5-arg signature can slide 137 into a different T-index.

- [s6] Restored src/system.c to HEAD via git checkout (extract_and_score's Python-side backup produced CRLF via the Write tool; used git checkout to guarantee LF cleanliness). Oracle unchanged; running s4/s5 permuter campaigns undisturbed (PIDs 1935645, 1935593 still active per WSL ps at session start).

- [s7] s7 baseline: candidate.c (vT40) sandbox --disable all = masked 4 (178/179), 42 rules dropped, 20 cheat-asm stripped. src/system.c restored via git checkout (Python write introduced CRLF; git restore normalized). Oracle green.

- [s7] GCC 2.7.2 jump2 pass CREATES young code_labels post-greg: labels {591, 301, 85} newly present in mar_system_s6.i.jump2 vs mar_system_s6.i.greg (diff in tmp/grind/marionation_Exec/s7/jump2_label_delta.txt); label 493 concurrently DELETED (fold consolidation). This is the young-label mechanism required by session-10's BB2_ALLLIVE_LABEL byte-proof — it arises naturally in this codegen, no knob needed.

- [s7] Label 591's placement: absorbs BOTH check1's copy-skip target AND an in-body reference from check2's copy-loop pre-check (SEQUENCE insn 610 → label_ref 591 at dbr line 5057). Physically emitted BEFORE the check2 branch (insn 424) — sits between check1 body and check2 label 416 (dbr lines 4990-5005). Off the check2 beqz fall-through walk entirely.

- [s7] Check2's fall-through walk from insn 424 (branch_zero a2==0 -> label 505 'tail') carries NO code_label at any pass between insn 424 and the steal-victim insn 445 (move a1,s4). Only labels in emission range: 458 (later, top of check2's own copy loop) and 505 (tail, the TAKEN target). Confirmed by grepping all pass dumps.

- [s7] Note 493 = NOTE_INSN_DELETED_LABEL in dbr at line 5131 (position AFTER check2's copy loop, before check2's 'j tail; v0=a2') was the natural young-label candidate for check2. It was deleted by jump2 because both check1's and check2's skip-past sequences fold to the same 'j tail; v0=a2' — jump2 collapses them under label 591 (which is placed at CHECK1's emission position, absorbing check2's reference).

- [s7] Consequence for any C-level young-label lever: repositioning the fold outcome onto check2 requires either reversing check1/check2 emission order (semantically forbidden — they order prior-state-check then current-frame-check) OR adding an extra branch-target reference at check2's skip position that check1 lacks (impossible without an additional semantically-live jump, which itself is a visible-byte penalty).

- [s8] s8 baseline confirmed: candidate.c (vT40) spliced -> sandbox --disable all = masked 4 (178/179 insns, 42 rules dropped, 22 cheat-asm stripped). Floor unchanged since s0 wip-import.

- [s8] HEAD src/system.c (pre-splice) scores masked 56 / 176 insns — reproduces ledger's HEAD-vs-candidate gap exactly.

- [s8] vT40's t0-web decomposition (`t0 = idx_1494[0]; t0 *= 4; t0 = (u8*)tbl_125c + t0;`) contributes ~12 masked points to the floor — replacing with clean `tbl_125c[idx_1494[0]]` inline regresses to 16 EVEN WITH the pp alias preserved (v02 measurement). The web is not decorative.

- [s8] vT40's pp pointer-alias (`pp = (void**)&D_800F19C0; ... *pp`) contributes ~12 masked points — removing it while also removing the web (twin-style v01) matches v02's +12 exactly; no evidence pp alone contributes beyond the web (both regress to 16, ceiling-bounded by the compound loss).

- [s8] t0's type axis (s32 address-value vs s32* pointer) is genuinely load-bearing at ~7 masked points (v03). NEW mechanism fact — s2/s3 tested type narrowing on `s32 t0` to u32 and found inert, but did NOT test s32 -> s32* transition. Now measured: pointer-typed is worse.

- [s8] src/system.c restored to HEAD via git checkout after all measurements; working tree clean; oracle green (splice_apply.py --restore + git checkout for CRLF safety, per s6/s7 convention).

- [s8] Rederive modality is EXHAUSTED for hand-authored structural axis: sibling-transplant (twin cpu_side_move_dir_4 shape), pointer-alias-elimination axis, and pointer-type axis all measured NEGATIVE this session; combined with s2 (26 forms), s3 (22 forms), s6/s7 (140-ordering sweep, 9-topology sweep, insn-level sched2/dbr forensics) and s4/s5 (~10k+ permuter samples on vT40 base), the hand-structural + local-sampler search around vT40 is comprehensively closed at masked 4.

- [s8] Twin cpu_side_move_dir_4 (system.c:388) confirmed still parked (distance 7, 5 rules per queue); its clean array-index shape does NOT transplant as a lever — its scheduling landscape differs (no idx_1496, no double-check, single mask). Twin work would need to be done independently, not mirror-transplanted.

- [s9] s9 baseline: candidate.c (vT40) spliced -> sandbox --disable all = masked 4 (178/179, 42 rules dropped, 20 cheat-asm stripped). Floor unchanged since s0.

- [s9] NEW mechanism fact: pp alias with named-local args (no v0 web, no byte-cast) scores masked 9 - RECOVERS 7 pts vs s8's pp-preserved-inline v02 (16). Named locals interpose distinct pseudos that let pp's staging benefit realize without the (u8*)+byte-offset web. s8 could only measure pp+web JOINT loss; s9 isolates the named-local sub-axis.

- [s9] NEW mechanism fact: arg5-first source order regresses even in named-local form (11 > 9). The pair-swap seats-trade coupling is NOT specific to inline/staged-temp forms - it holds when arg5's evaluation is fully materialized to a pseudo before the call.

- [s9] NEW mechanism fact: arg3 mem-load hoisted to named local (+14 masked = 18) has FRESH-TEMP-LAUNCH signature analogous to s2's vT33/vT34 (16/11) - confirms the launch pathology extends to hoisting existing arg-tree loads, not just synthesized temps.

- [s9] NEW mechanism fact: outer control-flow rewrite (if/else, drop gotos, invert vsync test) drops 2 build insns AND regresses masked by 3. The two-goto outer form is load-bearing for target byte-count - proves the outer CFG structure is not a free rewrite axis.

- [s9] src/system.c restored to HEAD (git checkout) after every measurement; working tree clean; oracle unchanged. candidate.c remains vT40 (best-known masked 4).

- [s9] s9 rederive angles catalogued as EXHAUSTED (extends s8's structural closure): outer CFG rewrite, arg3 hoisting, named-local pp-preserved (both source orders), arg5-first named-local seats survive. Combined with s2/s3 (48 forms), s8 (3 forms) and now s9 (4 forms), the hand-authored rederive axis around vT40 - across statement decomposition, argument staging shape, and outer flow structure - is comprehensively measured negative.

- [s10] [s10] Baseline reconfirmed on main: candidate.c (vT40) spliced -> sandbox --disable all = masked 4 (178/179, 42 rules dropped). src/system.c restored to HEAD after every measurement; oracle green; working tree clean.

- [s10] [s10] Frontier-1 partial-web sweep measured: s10v01 (arg5 v0-staging preserved, t0 natural) = masked 13; s10v02 (t0 byte-cast preserved, arg5 natural) = masked 6. Both banked rejected.

- [s10] [s10] NEW mechanism fact: the vT40 staging pair is NON-ADDITIVE. Removing arg5-web while keeping t0-web (v02) recovers 3 masked pts vs the fully-clean s9v01; removing t0-web while keeping arg5-web (v01) is +4 WORSE than fully-clean s9v01. The v0-staging web is a CONDITIONAL lever - beneficial only alongside the t0 byte-cast web. Isolated v0-staging destabilizes the pair-window compute.

- [s10] [s10] Quantification of independent staging components (masked pts recovered vs no-pp / no-web baseline of s8v01=16): pp alias ~7 (s9v01=9); t0 byte-cast web ~3 standalone / ~5-7 in combination (v02=6, vT40=4); v0<<2 staging web ~2 in combination / NEGATIVE standalone. All three are load-bearing; no two capture the third's contribution.

- [s10] [s10] With s2/s3 (48 hand forms), s4/s5 (~10k permuter samples), s6/s7 (140-ordering sweep, 9-topology sweep, insn-level sched2/dbr forensics, jump2 label enumeration), s8 (3 rederive forms), s9 (4 rederive forms), and s10 (2 partial-web forms), the vT40-basin hand-derivable + local-sampler search space is COMPREHENSIVELY closed. The remaining honest levers are: (i) cross-function transfer from twin cpu_side_move_dir_4 (untried; queue-routing item), (ii) fresh rich-pass permuter campaign on vT40 base with pair-window/region-3 targeted directives (untried post-s5 in current tooling), (iii) sanctioned FAKE constructs not yet tried on the pair window.

- [s10] [s10] SYNTHESIS: the two residuals (pair-swap @56/57, region-3 @149) are both root-caused at the insn level (s6/s7 forensics: sched2 T-14 UID tie insn 106<117; reorg pass-1 fill_simple check2-beqz caller-saved a1 dest). Neither has a hand-derivable pure-C closer within the vT40 basin. Progress requires (a) different basin (cross-fn transfer from twin), (b) larger sampling (fresh permuter with directives), or (c) an unrecognized construct outside current classification. The partial-web result strengthens (b/c): the coupled non-additive interaction between t0-web and v0-web suggests the target's source may have used a SINGLE unified staging expression that's beyond current hand enumeration.

- [s11] s11 baseline: candidate.c (vT40) spliced -> sandbox --disable all = masked 4 (178/179, 42 rules dropped, 20 cheat-asm stripped). Floor unchanged since s0.

- [s11] u10 (t0=idx0; v0=idx1; t0*=4; v0<<=2; t0=tbl+t0; arg5=deref(tbl+v0)) reaches masked 4 - novel interleaved spelling of vT40. Banked as rejected/s11-u10-interleaved-alt-masked4.c for reference.

- [s11] w03 (u10 chassis with pp placed AFTER both idx loads, before mults) also reaches masked 4. Confirms pp placement is inert within the interleaved layout.

- [s11] w10 (u10 with arg5 deref via (s32*)((u8*)tbl_125c+v0) instead of (s32*)(v0+(s32)tbl_125c)) also reaches masked 4. Confirms cast spelling is inert.

- [s11] u06 mirrored form (BOTH t0 and arg5 as address-values, both via u8*+idx*4) = masked 7. Best sub-vT40 mirror; +3 over floor. The symmetric qty birth signatures do not break the coupling.

- [s11] w01 (u06 mirror + v0-web restored for arg5 as addrval) = masked 6. Matches s10v02 attractor - the closest non-vT40 form remains 6 across the mirror + partial-web axes.

- [s11] NEW ATTRACTOR discovered s11: arg5-first with v0-web preserved and BOTH indices loaded before either compute, then interleaved shifts+adds, stabilizes at masked 6 across 7 spellings (x01-x05, x07, x08). Prior arg5-first data (s2/s3/s4/s9) had this at 8-11 because those forms ran arg5's full compute before t0's. The tight interleaved form recovers 2-5 masked pts.

- [s11] Non-interleaved arg5-first still regresses: x06 (arg5 fully computed before t0 starts) = 9; x09 (arg5 deref inserted before t0's tbl-add finishes) = 9. Confirms x01-x05/x07/x08 plateau is specifically from parallel-tree-expand geometry.

- [s11] u04 and u09 (both use shared offset variable via v0 reuse with split computes) both add 1 build insn (180 vs 179) at masked 11 - v0-reuse-as-offset-only pathway births an extra move that scrambles sched.

- [s11] u03 (ptr-advance form: s32 *p0 = tbl_125c + idx_1494[0]; s32 *p1 = tbl_125c + idx_1494[1]; *p0; *p1) = masked 12, novel pattern. Pointer-advance form gets penalized for callee-savedeness or seat placement of p0/p1 but avoids the launch-fresh-temp signature. Banked as rejected/s11-ptr-advance-p0p1-12.c.

- [s11] u01/u02/u05/u08 (unified-no-web variants: lifted tblb, tblp array, expression-fused, tblp both) all = masked 16 - matches s8v01 (no-web-no-pp) baseline. The unified-single-lifted-base form does NOT itself carry any of the two-web scheduling contribution.

- [s11] src/system.c restored to HEAD via git checkout after every measurement; working tree clean; oracle green.

- [s11] Cumulative structural coverage now: s2 (26 forms), s3 (22 forms), s8 (3 forms), s9 (4 forms), s10 (2 forms), s11 (30 forms) = 87 hand-authored structural variants of do_timeout measured. Combined with s6/s7 forensics (140-ordering sweep, 9-topology sweep, insn-level sched2/dbr) and s4/s5 permuter (~10k samples), the hand-derivable + local-sampler basin around vT40 remains comprehensively closed at masked 4.

- [s11] Multiple novel masked-4 spellings (u10, w03, w10 - all interleaved with variations in pp placement and deref cast) prove vT40's floor is not spelling-unique; it's a basin-of-attraction under multiple compound-restructure spellings. The pair-swap + region-3 residuals are structurally invariant to spelling.

- [s12] s12 baseline reconfirmed: HEAD src/system.c sandbox = 56 (176 build/179 target); candidate.c (vT40) spliced = masked 4 (178/179, 42 rules dropped, 22 cheat-asm stripped). Floor unchanged since s0.

- [s12] 25 novel structural variants measured across 3 sweeps (tmp/grind/marionation_Exec/s12/sweep1.json, sweep2.json, sweep3.json); floor at 4 unchanged.

- [s12] NEW mechanism fact: volatile qualifier on any staging temp adds 1-5 build_insns via mandatory memory ops (stack sw/lw sequences) and REGRESSES masked by +22 to +37. The volatile-as-qty-reweight lever is a NON-STARTER because the memory ops themselves are visible bytes.

- [s12] NEW mechanism fact: `*idx_1495` substitution for `idx_1494[1]` (semantically identical byte load, idx_1495 = idx_1494+1) regresses uniformly by +6 to +7 across chassis (vT40 vT40+pp-last x02 mirror). Root: reading idx_1495 in the do_timeout window extends its life across the debug_printf call, forcing callee-saved seat competition with existing callee-saved qtys (the alias-merge callee-saved-seat cascade documented in s4). Any pseudo referenced in the do_timeout window that is ALSO used post-callback pays this seat penalty.

- [s12] NEW mechanism fact: mirror-both-as-s32*-pointer-typed reaches +8 uniformly (masked 12), STRICTLY WORSE than mirror-both-as-address-values (x10=8). Canonical ptr+index addressing (`s32 *pt = tbl_125c + idx[i]; *pt`) emits a distinct pair-window compute chain from (u8*)+byte-offset arithmetic (`t0 = (u8*)tbl+t0*4; *(s32*)t0`); the +8 cost is invariant to pp position and birth order across 4 spellings.

- [s12] NEW positive finding: 6 additional novel masked-4 spellings discovered in the tbase-alias family (v08/w05/w08/z01/z02/z07): (v08) u8* tbase symmetric applied to both; (w05) s32 tbaseS symmetric; (w08) s32 tbase with mixed u8*/s32 casts; (z01) u8* tbase applied ONLY to arg5's compute; (z02) u8* tbase applied ONLY to t0's compute; (z07) u8* tbase with v0-interleaved compute. Combined with s11's u10/w03/w10, vT40's floor basin now has 9 known distinct spellings - the pair-swap+region-3 residuals are basin-wide invariants.

- [s12] NEW mechanism fact: split-init accumulation on arg5 (`arg5 = v0+(s32)tbase; arg5 = *(s32*)arg5;` - sanctioned family per split-init-accumulation-sanctioned 2026-06-13) regresses +7 uniformly (masked 11 in w04/w06). Extends arg5's life without changing final compute cost but retimes the pair-window sched to a suboptimal shape. The sanctioned split-init family does NOT help this pair.

- [s12] 3 disproven forms banked to memory/grind/marionation_Exec/rejected/: s12-idx1495-substitution-regresses-10.c (the canonical *idx_1495 regression case), s12-mirror-both-s32ptr-typed-12.c (pointer-typed mirror regression), s12-volatile-arg5-adds-store-26.c (volatile-as-qty-lever regression).

- [s12] candidate.c unchanged - remains vT40 masked 4 (best-known form). src/system.c restored to HEAD via git checkout after every measurement; working tree clean (except metrics/events.jsonl); oracle green.

- [s12] Cumulative hand-authored structural coverage after s12: s2 (26 forms) + s3 (22) + s8 (3) + s9 (4) + s10 (2) + s11 (30) + s12 (25) = 112 hand-authored structural variants of do_timeout measured. Combined with s6/s7 forensics (140-ordering sweep, 9-topology sweep, insn-level sched2/dbr forensics) and s4/s5 permuter (~10k+ samples), the hand-derivable + local-sampler basin around vT40 is comprehensively closed at masked 4.

- [s13] s13 baseline reproduced by inference from continuous ledger (candidate.c untouched since s10; vT40 masked 4 confirmed s1-s12). src/system.c untouched this session; oracle green.

- [s13] perm campaign (vT40-base): PID 1935645, elapsed 10:47:26 at session end. output-* set = [145-1, 145-2, 160-1, 160-2, 160-3, 165-1, 165-2, 165-3, 170-1, 175-1, 200-*, 205-*, 210-*, 215-*]. IDENTICAL to s5 snapshot - vT40-basin sampling has plateaued.

- [s13] perm_c campaign (find105-base): PID 1935593, elapsed 10:47:26. output-* set = [85-1, 95-1, 95-2]. IDENTICAL to s5 snapshot - alias-merge family floor holds at masked 10.

- [s13] perm_z07 campaign (z07-base, NOVEL): PID 3540094 launched this session, base score 220 verified matches vT40 base's residual signature (2 reorderings 60ea + 1 deletion 100).

- [s13] Total live campaigns at handoff = 3 (vT40, find105, z07 basins). Two remaining known masked-4 basins (s11 u10/w03/w10, s12 v08/w05/w08, z01/z02) not yet sampled as permuter bases - available for future permuter sessions.

- [s13] PERM_RANDOMIZE_TYPE macro is NOT recognized by decomp-permuter (only PERM_FACTORIES = GENERAL/ONCE/RANDOMIZE/FORCE_SAMELINE/VAR/LINESWAP/LINESWAP_TEXT/INT/IGNORE/PRETEND). Any future annotation attempts must use PERM_RANDOMIZE alone.

- [s13] PERM_GENERAL is a multi-alternate picker (splits args on commas), NOT a scope wrapper - PERM_GENERAL({compound-stmt}) is a pycparser syntax error at expansion. Correct use is PERM_GENERAL(expr1, expr2, ...) with alternate rewrites of the SAME node.

- [s14] s14 baseline: NO source measurements were taken this session (permuter modality); candidate.c untouched, src/system.c untouched, oracle green throughout.

- [s14] s14 running-campaign inventory at start: PID 1935645 (vT40 -j6, 10:49:43 elapsed), PID 1935593 (find105 -j24, 10:49:43), PID 3540094 (z07 -j6, 03:26 elapsed - z07 elapsed short because s13's PID likely died/relaunched; log shows only iterations 1..30 recorded so this is a fresh <=1h run).

- [s14] s14 output snapshot: vT40 basin outputs unchanged since s13 (21 distinct scores from 145-215, no sub-145); find105 basin unchanged since s5 (3 outputs: 85-1/95-1/95-2); z07 basin empty (0 outputs).

- [s14] s14 z07 log iteration spread: base=220 hit by iters 1/2/6/10/26/27 (approximately 20% at base); mutations up to 3610 seen - default randomization is exploring broadly but with no <220 attractor found in the first 30 iters.

- [s14] s14 w05 workspace built at tmp/grind/marionation_Exec/s14/perm_w05/ via build_ws_w05.py (adapts z07 build script for w05 chassis; extract_fn strips /* */ comments and swaps into base_full.c skeleton from perm_mar6). No PERM_* directives (s13 confirmed PERM_RANDOMIZE_TYPE unrecognized and PERM_GENERAL(compound-stmt) trips pycparser).

- [s14] s14 w05 permuter --debug base = 220 (Reorderings 2*60 + Deletions 1*100 - identical residual signature to vT40 and z07): confirms w05 is a valid basin seed with the same 2 root residuals but different structural neighborhood.

- [s14] s14 w05 campaign launched: PID 3756068, -j6 --better-only --stop-on-zero, log tmp/grind/marionation_Exec/s14/campaign_w05.log, pidfile tmp/grind/marionation_Exec/s14/campaign_w05.pid. Total concurrent load now: 4 campaigns, 42 threads (24 + 6 + 6 + 6).

- [s14] s14 confirms s13's frontier item #2 mechanism: distinct masked-4 spellings do serve as independent permuter basins with the same base score but different starting neighborhoods. Portfolio approach is mechanically viable.

- [s15] s15 dumps produced fresh on main: tmp/grind/marionation_Exec/s15/vt40_dumps/{mar_system_s6.i.rtl,jump,cse,loop,cse2,flow,combine,sched,lreg,greg,jump2,dbr,sched2,mar_system_s6.s} for candidate.c (vT40) and identically for tmp/grind/marionation_Exec/s15/x02_dumps/ (s11 x02 = rejected/s11-arg5first-v0web-interleaved-6.c). src/system.c restored to HEAD (bash -c 'sed -i s/\r$//' cleaned CRLF injected by Windows splice; git status src/system.c clean; oracle green).

- [s15] vT40 emitted mar_system_s6.s and x02 emitted mar_system_s6.s differ by ONE line (a single transposition at lines 1152-1153): vT40 has `lbu $4,0($18)` at the tslTm2LoadImage_2 delay slot then `lbu $2,1($18)`; x02 has them swapped. All other 2710 lines are byte-identical between the two dumps.

- [s15] vT40 sched2 BB #3 ready-list initial has insns {91 pri1, 93 pri1, 99 pri1 refs5, 115 pri1 refs4, 141 pri1, 117 pri2 refs3, 106 pri2 refs3, 120 pri2 refs4, 122 pri2 refs4, 128 pri2 refs3, 111 pri2 refs4, 137 pri3 refs4, 133 pri3 refs3, 143 pri3 refs2, 145 pri3 refs3, 139 pri3 refs1, 147 pri4 refs1, 152 pri4 refs1, 165 pri4 refs1, 167 term}. T-16 trace: `launching 99 before 117 with no stalls at T-16` — insn 99 is the t0-idx lbu (refs 5 fits t0's 4 downstream users + jal-arg = 5).

- [s15] x02 sched2 BB #3 ready-list initial has insns {91 pri1, 93 pri1, 99 pri1 refs4, 103 pri1 refs5, 141 pri1, 108 pri2 refs3, 112 pri2 refs3, 117 pri2 refs4, 119 pri2 refs4, 128 pri2 refs3, 122 pri2 refs4, 137 pri3 refs4, ...}. UIDs shifted vs vT40 because expand renumbered when the source rearranged the two idx-loads. T-16 trace: `launching 103 before 108 with no stalls at T-16` — insn 103 (refs=5) is x02's t0-idx-lbu; insn 99 (refs=4) is x02's arg5-idx-lbu.

- [s15] The pair-window sll insns tie at pri=2 in BOTH forms and both forms reorder to emit v0-sll chronologically before a0-sll (vT40 `now 106 117 141` at T-14; x02 `now 112 108 141` at T-14). vT40 emits target-mismatched (sll v0; sll a0; addu v0,v0,s5) — same in x02. The pair-swap residual is INVARIANT under the x02 source rearrangement; the arg5-first source geometry did NOT change the T-14 tie outcome as s11 hypothesized it might.

- [s15] Region-3 dbr fill_simple_delay_slots decision (check2 beqz stealing move a1,s4) is identical between vT40 and x02 dumps. mar.dbr summary lines match; the caller-saved a1-dest liveness argument (s6 forensics) still governs. x02 has no C-level lever on region-3.

- [s15] The x02 delta is thus fully accounted-for as a delay-slot fill divergence at the tslTm2LoadImage_2 jal only, NOT a partial pair-swap fix. arg5-first-idx-load source order is a strict-worse geometry: it inherits the same T-14 tie outcome as t0-first while flipping T-16 launch order to lose the delay slot.

- [s15] sched2 tie-break for the T-14 sll pair operates on INSN_LUID (scheduler-internal ID from rank_for_schedule), not expand-time UID, because BOTH forms select v0-sll chronologically first (higher-numbered UID wins in vT40: 117 emitted at T-15 chronologically before 106 at T-14, chronologically 117 first; x02: 108 chronologically first at T-15 before 112 at T-14). This is a NEW mechanism fact refining s6's `broken by insn UID` finding: at the sched2 pair-window tie, the tie-break follows LUID which reflects sched2's own dep-DAG walk, not source-order UID. Expand-time UID DOES determine which insn appears first in sched2's ready-list-initial list (affects T-16), but the T-14 pair-window tie has both candidates present and picks by LUID.

- [s16] [s16] Baseline reconfirmed: candidate.c (vT40) spliced via s6/splice_apply.py -> sandbox --disable all = masked 4 (178/179, 42 rules dropped, 20 cheat-asm stripped). src/system.c restored to HEAD via git checkout after all measurement; oracle green.

- [s16] [s16] Frontier item #1 (early-use of the loaded byte before its shift to retime the sched2 T-14 LUID tie without flipping T-16) is KILLED with byte-level dump evidence. Both t0-early-compare (v01: `if (!t0) { }`) and v0-early-compare (v04: `if (!v0) { }`) score masked 4 / 178 build_insns AND emit BYTE-IDENTICAL asm to baseline across the pair window + tslTm2 delay slot + debug_printf setup (lines 1150-1165 in mar_system_s16.s). The empty-body branch is folded by jump.c before sched2 (or by flow.c's `if_then_else_cond` reductions); the compare insn occupies UID space at .rtl/.jump but leaves no dep-chain contribution by sched2's backward walk. Sched2 BB #3 UIDs in v01/v04 SHIFT (T-8 shows insns 111/145/141/137 instead of baseline's 133/137/111/141 at that T-index), but the final schedule collapses to identical bytes.

- [s16] [s16] NEW MECHANISM FACT: `t0 *= 4` and `t0 = t0 << 2` are NOT expand-equivalent in GCC 2.7.2. v03 (candidate.c with only `t0 *= 4;` -> `t0 = t0 << 2;`) scores masked 9 / 178 insns (+5 REGRESSION). RTL pass-count telemetry: baseline .rtl = 59 ashift + 53 mult, v03 .rtl = 59 ashift + 52 mult - the mult-to-ashift conversion for `t0 *= 4` happens at CSE (baseline drops from 53 to 48 mult and 59 to 47 ashift by CSE), one pass later than v03's user-authored ashift which is already an ashift at .rtl. Downstream effect: baseline's canonicalized-at-CSE ashift takes UID N_c (assigned by CSE); v03's user-written ashift takes UID N_e (assigned at expand, LOWER). Different UIDs propagate through sched2 rank_for_schedule LUID assignment. Emitted asm at pair window: baseline `sll $2; sll $4; addu $2,$2,$21` (arg5-sll chronologically first via UID<t0-sll), v03 `sll $3; sll $2; addu $2,$2,$21` (t0-sll chronologically first via UID<arg5-sll) AND t0's register seat trades from $4 (a0, target-matching) to $3 (v1, WRONG - target uses $a0/a0-16).

- [s16] [s16] Corollary: canonicalization-time-of-mult-vs-shift is a NEW documented axis. The s3 y-family sweep did test operand-swap and array-index spellings (canonicalization proved TOTAL for those tree-shape variants; y03 fused operand swap = 4 emission-identical to baseline). But no s2-s15 sweep varied MULT vs SHIFT for t0 or arg5. All hand-authored variants used `t0 *= 4`. This finding does NOT provide a closer (v03 regressed +5) but it maps a previously-untested axis and adds a MOVE-BACK-TO-4 constraint for any future rewrite: mult-by-4 must NOT be rewritten as shift-by-2.

- [s16] [s16] v02 (fresh dead local `saved_debug = t0`) is inert (masked 4, 178). Confirms that any zero-cost early-use (compare, mov to dead local) is completely eliminated before sched2 and does not reweight LUIDs; only a construct with an EMITTED SIDE-EFFECT (visible byte) could contribute a real dep-chain intervening between load and shift - and such a construct would exceed build_insns and regress.

- [s16] [s16] Frontier item #1 general form (any early use of the loaded byte before its shift that does not add build insns) is thus COMPLETELY closed: (a) empty-body compares fold in jump.c pre-sched2 (v01/v04 evidence); (b) dead-local reads are DCE'd before final schedule (v02 evidence); (c) side-effectful early-uses add insns by construction (a-priori impossibility). No zero-cost T-14 LUID lever exists via early-use in C source.

- [s16] [s16] Refinement of s15 T-14 LUID mechanism: the ready-list at T-14 shows two same-priority ashift insns tie. In baseline the tie-break selects insn 106 (a0-sll) at chronological pos 7 (T-14) and insn 117 (v0-sll) at pos 6 (T-15). In the four v0*_ variants tested (v01/v02/v04 + a-priori for arbitrary compare/mov inserts), the eventual schedule collapses to baseline's regardless of UID-space perturbations - proving the T-14 tie is byte-resilient to expand-time UID shifts. The pair-swap residual is now proven unreachable by any C-level early-use construct.

- [s16] [s16] src/system.c restored to HEAD via git checkout after each measurement; working tree clean at session end; oracle unchanged (candidate.c untouched). Dump artifacts under tmp/grind/marionation_Exec/s16/dumps/{baseline,v01_dead_compare,v03_dead_shift,v04_v0_early_read}/ (14 files each: cc1 -da .rtl/.jump/.cse/.loop/.cse2/.flow/.combine/.sched/.lreg/.greg/.jump2/.sched2/.dbr + emitted .s).

- [s17] s17 baseline: candidate.c (vT40) spliced -> sandbox --disable all = masked 4 (178/179, 42 rules dropped, 20 cheat-asm stripped). Floor unchanged since s0. src/system.c restored to HEAD after every measurement (git checkout); oracle green throughout.

- [s17] s17 19-variant sweep (tmp/grind/marionation_Exec/s17/sweep.json + variants/): 6 stay at masked 4 (baseline, a4 4*t0, a5 t0*(2*2), b1-b6 all v0 rewrites, c3 both-mult, c4 both-lhs-4mul), 8 regress to masked 9 (a1 nested shift, a2 add-tree flat, a3 add-double-step, a6 shift-by-2 confirming s16 v03=9, c1 both add-tree, c2 both nested shift, d1 swap t0shift/v0mult, d2 both add-double).

- [s17] NEW MECHANISM FACT: t0's mult-by-4 canonicalization creates a binary +5 masked boundary at expand-time RTL: `(mult t0 4)` (any of `t0*=4`, `t0*4`, `4*t0`, `t0*(2*2)`) = 4; `(ashift t0 2)` (any of `t0<<=2`, `(t0<<1)<<1`) = 9; plus-tree (`t0+t0+t0+t0`, `t0+t0; t0+t0;`) = 9 (canonicalized to ashift at expand, NOT mult). This refines s16's canonicalization-time-of-mult-vs-shift finding: the axis is BINARY (mult vs non-mult), not a continuous canonicalization-timing gradient. Add-tree spellings do NOT create a novel expand-time RTL family - they collapse into the same expand-time ashift landscape as user shifts.

- [s17] NEW MECHANISM FACT: v0's arithmetic spelling is COMPLETELY inert to the pair-swap residual (0 masked delta across shift/mult/add/LHS-const/paren-folded). v0's expand RTL shape does not enter the sched2 T-14 tie decision - the tie's inputs are the two sll insns' INSN_LUIDs, both are pri=2 regardless of expand opcode, and rank_for_schedule breaks the tie deterministically. This is a CORRELATED PROOF that the T-14 tie is decided by scheduler-internal state (INSN_LUID from the backward walk), not by expand-time opcode-family placement.

- [s17] The frontier item #3 hypothesis is FALSIFIED: no arithmetic spelling of t0 or v0 produces LUID(t0-sll) > LUID(arg5-sll) with correct seat. The mult family produces LUID(t0-sll) < LUID(arg5-sll) with correct seat (masked 4, target-mismatched pair order); the shift/add family produces LUID(t0-sll) < LUID(arg5-sll) with WRONG seat (masked 9, t0 in $3 not $4). No spelling reaches target's LUID(t0-sll) > LUID(arg5-sll) AND correct seat.

- [s17] Cumulative hand-authored structural coverage after s17: s2 (26) + s3 (22) + s8 (3) + s9 (4) + s10 (2) + s11 (30) + s12 (25) + s16 (4) + s17 (19) = 135 hand-authored structural variants measured. Combined with s6/s7/s15/s16 forensics (140-ordering sweep, 9-topology sweep, insn-level sched2/dbr forensics + x02 delta accounting) and s4/s5 permuter (~10k+ samples), the hand-derivable basin around vT40 is comprehensively closed at masked 4 across the mult/shift/add-tree canonicalization axis.

- [s17] Two novel spellings added to the known masked-4 basin: a4_t0_4mul_lhs (`4 * t0` LHS-const-mult) and a5_t0_paren_folded (`t0 * (2*2)`) - both structurally distinct from the 9 previously-catalogued basin members but score-invariant. Basin now has 11 known distinct masked-4 spellings; the two residuals are basin-wide invariant.

- [s17] candidate.c unchanged (remains vT40 masked 4 best-known); src/system.c untouched (git status clean); oracle green.

- [s17] 4 forms banked to memory/grind/marionation_Exec/rejected/: s17-t0-nested-shift-masked9.c, s17-t0-add-tree-masked9.c, s17-t0-4mul-lhs-masked4-basin-equiv.c, s17-both-mult-masked4-basin-equiv.c (two of each class for future reference).

- [s18] s18 baseline: candidate.c (vT40) spliced -> sandbox --disable all = masked 4 (178/179, 42 rules dropped, 20 cheat-asm stripped). Floor unchanged since s0.

- [s18] s18 NEW mechanism fact: idx_1495 life shape is doubly load-bearing. s12 measured +6-7 masked regression when idx_1495 life is EXTENDED (substitution *idx_1495 for idx_1494[1] in do_timeout window). s18 v03/v04 measure BOTH other directions: elimination (inline idx_1494[1] at callback, +11 masked / -3 build_insns) and shrinkage (late birth in check region, +21 masked / -1 build_insn). The function-top-birth-with-callback-only-use signature is a coupled fixed point; no shape movement recovers.

- [s18] s18 NEW mechanism fact: check-region control-flow structure (goto+label chain vs structured if/else) is spelling-inert at masked 4. v02 rewrote check1/check2/tail as structured if/else with returns and reached masked 4 at 178 build_insns exactly. jump.c's CFG folding collapses the two forms to identical post-jump2 layout. Basin membership now 12 known distinct masked-4 spellings.

- [s18] s18 NEW mechanism fact: outer polling as for(;;) with sentinel-tagged timeout (v0=-2 mid-loop, if(v0==-2) after) regresses +13 masked vs floor at same 179 build_insns. Different regression profile from s9v04's inverted-vsync if/else (+3 masked, -2 build_insns). Confirms outer-flow rewrites are all structurally distinct energy shapes, none dominant over vT40's two-goto polling.

- [s18] s18 cumulative hand-authored coverage now includes 3 novel structural rederivations (v01 sentinel-outer, v03 pseudo-elimination, v04 late-birth) and 1 novel basin member (v02). Combined with prior 135 hand forms + 10k+ permuter samples + s6/s7/s15 forensics, the rederive-modality search around vT40 is comprehensively exhausted across outer flow, check region CFG, and pseudo life-shape axes.

- [s18] s18 candidate.c unchanged (remains vT40 best-known masked 4). src/system.c restored to HEAD via git checkout after each measurement; working tree clean; oracle green.

- [s18] s18 4 forms banked to rejected/: s18-forloop-outer-sentinel-17.c, s18-check-ifelse-structured-masked4-basin-equiv.c, s18-eliminate-idx1495-inline-15.c, s18-idx1495-late-birth-25.c.

- [s19] [s19] Baseline unchanged: vT40 floor masked 4 (178/179, 42 rules dropped, 20 cheat-asm stripped); src/system.c restored to HEAD after triage; oracle green.

- [s19] [s19] NEW permuter finds since s14: s13/perm_z07/{output-160-1, output-200-1} and s14/perm_w05/{output-145-1, output-200-1, output-200-2}. Both z07 and w05 basins now have first sub-200 outputs.

- [s19] [s19] z07-160-1 triaged: masked 3 / 180 insns; diff = `status = 0;` before check1 clear + `while (status);` after clear — SAFER semantic variant of vP160 (status is locally-assigned, not uninit cross-block), but still pays +1 build_insn = unmatchable at 180 vs 179 target. Third distinct label-alive spelling documented in ledger (vP160 `while (status)` uninit + s5 0xFF-through-local `while (status = 0)` + this z07 pre-assign-then-check).

- [s19] [s19] w05-145-1 triaged: masked 17 / 179 insns; reproduces find145-class alias-merge attractor from w05 basin at identical masked score to s4 output-145-2 measurement. Confirms alias-merge attractor is chassis-invariant (family floor masked 10 governs regardless of source basin).

- [s19] [s19] SYNTHESIS finding: portfolio approach has CONVERGENT attractors — both novel sub-200 finds from previously-unsampled basins reproduced known attractor CLASSES rather than surfacing new ones. The two residuals map to a small closed set of universal attractors (alias-merge callee-saved family floor masked 10; label-alive +1-insn; find165/175/200-class permuter progress that trades reg-diff points without moving masked). This TIGHTENS the impossibility argument for hand-derivable / local-sampler closure and elevates non-local levers (compiler flags, twin-first exchange) to primary frontier status.

- [s19] [s19] Cumulative ledger state after s19: 135 hand-authored structural variants (s2/s3/s8/s9/s10/s11/s12/s16/s17/s18) + ~10k+ permuter samples across 4 basins (vT40/find105/z07/w05) + s6/s7/s15/s16 insn-level forensics (140-ordering sweep, 9-topology sweep, dbr/sched2 dumps) + 5 sub-200 finds triaged in-ledger. The hand-derivable + local-sampler basin around vT40/z07/w05 is COMPREHENSIVELY closed at masked 4. Movement requires (a) scheduler-behavior modification (compiler flag), (b) twin-first exchange lever, or (c) reseeding permuter on CFG-restructured chassis (s18v02 if/else check-region) not yet sampled — the tbase/interleaved chassis explored in s13/s14 has confirmed the same attractor set as vT40.

- [s19] [s19] Ledger digest 'live frontier item 1' (w05/z07 campaigns producing novel sub-220 finds) is now RESOLVED — the finds materialized (5 total across both basins) and were triaged NEGATIVE as closers. That frontier item is closed.

- [s20] s20 baseline reconfirmed on main: candidate.c (vT40) spliced -> sandbox --disable all = masked 4 (178/179, 42 rules dropped, 20 cheat-asm stripped). Floor unchanged since s0.

- [s20] s20 NEW mechanism fact: `s32 status` decl position is qty/schedule-inert between function-top and check1-body scopes. v01 = masked 4/178 exactly; adds a 13th distinct known masked-4 basin member spelling.

- [s20] s20 NEW mechanism fact: the `int new_var; int new_var3;` mask-holder decl+init position IS load-bearing at 2 build_insns and 4 masked pts. Moving init to immediately-before-use lets combine forward-substitute the 0xFF constant into the `& new_var` and `& new_var3` expressions and fold away both `andi ,0xff` insns (byte-load auto-clears high bits). The current position (between vsync-check and check1) is FAR ENOUGH from the uses that combine's local substitution window does not reach; that separation is the mechanism keeping the two andi insns alive. Confirms the candidate.c FAKE annotation on new_var: alternatives are NOT interchangeable spellings, they are structurally distinct against combine's fold reach.

- [s20] s20 corollary: the mask-holder-scope axis is a NEW constraint documented for future rewrites. Any decl-scope-tightening of new_var / new_var3 (or renaming to a shorter/tighter-scope name) will re-trip combine's fold and drop 2 insns. Complementary to the 'symbolic mask is the one spelling combine cannot fold' finding recorded in candidate.c line 74 comment (which addressed u8-typed and split-init alternatives, not scope).

- [s20] s20 v01 saved to memory/grind/marionation_Exec/rejected/s20-status-local-scope-masked4-basin-equiv.c (novel basin member #13). v02 saved to memory/grind/marionation_Exec/rejected/s20-newvar-tight-scope-combine-folds-andi-8.c (KILLED regression, mask-fold mechanism documented).

- [s20] s20 candidate.c UNCHANGED (remains vT40, best-known masked 4). src/system.c restored to HEAD via git checkout after all measurement; working tree clean; oracle green.

- [s20] s20 cumulative hand-authored structural coverage after s20: 135+ prior (s2/s3/s8/s9/s10/s11/s12/s16/s17/s18) + 2 new (v01/v02) = 137+ hand-authored structural variants measured. The non-do_timeout-local-scope axis is now catalogued: function-top vs body-local for status is inert; mask-holder scope position is load-bearing at -2 build_insns / +4 masked.

- [s21] [s21] Baseline reconfirmed on main at session start: candidate.c (vT40) spliced -> sandbox --disable all = masked 4 (178/179, 42 rules dropped, 20 cheat-asm stripped). Floor unchanged since s0.

- [s21] [s21] 15 novel structural variants measured (tmp/grind/marionation_Exec/s21/sweep_results.json). 11/15 = masked 4 INERT (novel basin members #14-#24); 4/15 KILLED with regressions (v02=19, v12=27, v13=18, v15=6).

- [s21] [s21] NEW mechanism fact: idx_1496 birth-site is load-bearing (v13 = masked 18 / 177 build_insns when moved from function-top into check-region prologue). Extends s18v04's idx_1495-late-birth finding (masked 25) to a SECOND pseudo. Both idx_1495 and idx_1496 have doubly-load-bearing function-top birth signatures.

- [s21] [s21] NEW mechanism fact: check1-scoped idx_1495 birth (v12 = masked 27 / 177 build_insns) is WORSE than s18v04's callback-scoped late birth (masked 25). Narrower delay-birth scope strengthens the regression; the ideal birth site is function-top exactly.

- [s21] [s21] NEW mechanism fact: split-init on saved (`saved = *D_800A147C_2; saved &= 3;`) is a byte-neutral +2 masked regression despite the sanctioned family status (2026-06-13 owner directive). saved-life extension creates a callee-saved seat competition analog to the s12 *idx_1495 mechanism. Split-init family is SANCTIONED but NOT universally helpful; on this pseudo it costs.

- [s21] [s21] NEW mechanism fact: fresh named-local `s32 i = 7;` per-copy-block (v02 = masked 19) births two separate pseudos with the fresh-temp-launch signature s2/s9 documented for vT33/vT34. The function-top single `s32 i;` is qty-collapse-canonical for the copy loops.

- [s21] [s21] NEW mechanism confirmation: mask-holder decl axis is comprehensively closed - decl-list order (v08), init-site statement order (v07), type width (v05 u32, v06 s32, baseline int) are all inert; only scope-tightening (s20 v02) folds combine and drops 2 build_insns for a masked regression.

- [s21] [s21] NEW mechanism confirmation: pointer decl scope-tightening from function-top to inner check block (v10) is inert; supports s2's decl-order-follows-first-use finding at the pointer scope level.

- [s21] [s21] NEW mechanism confirmation: sanctioned split-init family on D_800F19B8 prologue (v03) is inert - byte-neutral, masked-neutral. Distinct from split-init on saved which regresses; family effect is context-dependent per pseudo life.

- [s21] [s21] NEW mechanism confirmation: sub-expression hoisting at the prologue (v11 vsync-hoist) is inert. Prologue fresh-temps do NOT launch the same way check-region fresh-temps do (s9 arg3 = 18) because the debug_printf-call dep chain is confined to do_timeout region.

- [s21] [s21] Cumulative hand-authored structural coverage after s21: 137+ prior (s2/s3/s8/s9/s10/s11/s12/s16/s17/s18/s20) + 15 new = 152+ hand-authored structural variants measured. Basin membership expanded from 13 known masked-4 spellings (s20) to 24 known distinct spellings. The pair-swap + region-3 residuals are basin-wide invariant across every hand-derivable structural axis.

- [s21] [s21] src/system.c restored to HEAD via python3 tmp/grind/marionation_Exec/s6/splice_apply.py --restore + git checkout after every measurement; working tree src/ clean; oracle green (only metrics/events.jsonl modified).

- [s21] [s21] candidate.c UNCHANGED (remains vT40 masked 4 best-known). 4 disproven forms banked to memory/grind/marionation_Exec/rejected/: s21-i-fresh-local-per-copy-block-19.c, s21-idx1495-birth-in-check1-27.c, s21-idx1496-birth-in-check-region-18.c, s21-split-init-saved-masked6.c.

- [s22] s18v02 chassis base score = 220 CONFIRMED at permuter --debug (Reorderings 2 x60 + Deletions 1 x100 = 220), exactly matching vT40/z07/w05 base signatures.

- [s22] s18v02 permuter workspace built via tmp/grind/marionation_Exec/s22/build_ws_s18v02.py (adapted from s14/build_ws_w05.py); fn body chars: old=2604 new=2650; default randomization only, no PERM_* macros (s13 lesson).

- [s22] First ~450 iterations of the s22 s18v02 permuter campaign: 0 sub-220 finds. Distribution: 124 iters at 220 (~27.6% of iterations landed at the local basin minimum via random mutation), 12 at 230, tail up to 11140. Minimum floor after ~5 min: 220.

- [s22] Campaign PID 1808915 launched with nohup -j6 --better-only --stop-on-zero, log at tmp/grind/marionation_Exec/s22/campaign_s18v02.log; will continue running past session end (matches s13/s14 detached-campaign discipline).

- [s22] Portfolio-scale sub-200 sampling now covers 5 distinct-chassis basins: vT40 (11h+, no new finds since s5) + find105 (11h+, no new finds since s5) + z07 (post-s13, minor label-alive/reg-shuffle finds) + w05 (post-s14, minor alias-merge/reg-shuffle finds) + s18v02 (this session, 0 sub-220 in first 5 min). Consistent with the s19 SYNTHESIS conclusion: the {2 reorderings + 1 deletion} residual is universal-attractor-bounded across permuter-reachable basins.

- [s23] [s23] Baseline reconfirmed: candidate.c (vT40) spliced -> sandbox --disable all = masked 4 (178/179 insns, 42 rules dropped, 20 cheat-asm stripped). Floor invariant across 23 sessions.

- [s23] [s23] s22 s18v02 campaign extended KILL: 247 iterations recorded in campaign_s18v02.log, 0 sub-220 finds materialized, no output-* directory. Frontier item #2 (s22-extended-sampling-yields-novel-attractor) is now KILLED at 200+ iterations past s22's snapshot.

- [s23] [s23] vT40 basin (s4/perm, -j24 alive) extended KILL: 4+ hours since s19's 17:55 last-recorded output produced 0 new outputs. Cumulative vT40-basin sampling now >15 hours with 0 sub-145 finds since s5/s13/s19.

- [s23] [s23] w05 basin (s14/perm_w05, alive) extended: 1 new output-215-1 (permuter-score 215 — regression, not closer). Basin's mutation trajectory continues to explore alias-merge-class terrain without moving toward masked<=9.

- [s23] [s23] z07 basin (s13/perm_z07, alive): 0 new outputs since s19 snapshot. Output set [160-1, 200-1] unchanged.

- [s23] [s23] find105 basin (s4/perm_c, alive): 0 new outputs since s13/s19. Output set [85-1, 95-1, 95-2] unchanged. Family floor confirmed at masked 10.

- [s23] [s23] NEW PORTFOLIO EVIDENCE: 5-basin sampling at 40+ CPU-hour scale yields exactly 22 sub-215 finds total, EVERY ONE mapping to the 3 known attractor classes. Convergence is CFG-shape-invariant (s18v02's structured if/else did not open new terrain). This is the strongest empirical KILL of the 'a novel attractor exists locally' hypothesis to date.

- [s23] [s23] permuter_annotate.py --hint enumeration on marionation_Exec: only hint that could nominally target this function is loop-counter-fills-load-delay (copy blocks), but the tool re-derives the base from src/system.c (HEAD, masked 56, twin-inline-style clean form) — the annotated candidate is NOT vT40-anchored and would restart from a masked-16 basin, adding no directed lever over vT40. shared-end-label, loop-rotation-two-shift, and register-asm-pins do not structurally match this function's residuals.

- [s23] [s23] src/system.c restored to HEAD via splice_apply.py --restore + git checkout; working tree clean modulo metrics/events.jsonl; oracle green throughout.

- [s23] [s23] candidate.c unchanged (vT40 remains the best-known form at masked 4).

- [s24] s24 baseline sandbox re-confirmed: score=4, build_insns=178, target_insns=179, rules_dropped=42, cheat_asm_stripped=20 on vT40 candidate.c spliced.

- [s24] cc1 flag sweep (15 toggles) tested on src/system.c-with-candidate.c-spliced preprocessed to a single .i, cc1 emissions per flag captured at tmp/grind/marionation_Exec/s24/full_*.s and marionation_Exec extractions at asm2/*.s.

- [s24] IDENTICAL-emission set on marionation_Exec (8 flags KILL as levers): -fno-caller-saves, -fno-cse-follow-jumps, -fno-defer-pop, -fno-force-mem, -fforce-mem, -fno-peephole, -fno-strength-reduce, -fno-thread-jumps.

- [s24] -fno-rerun-cse-after-loop's 11-line system.c diff is entirely OUTSIDE marionation_Exec (fn emission identical to baseline).

- [s24] -fno-schedule-insns2, -fno-schedule-insns, and -fno-expensive-optimizations each perturb marionation_Exec (26/25/4 diff lines respectively) but leave the pair-window emission (sll v0; sll a0; addu v0,v0,s5) EXACTLY as baseline - no flag toggle flips the T-14 tie.

- [s24] -fno-delayed-branch produces a structurally distinct emission (147 vs 204 lines total, no delay-slot fills anywhere) but the pair-window at insns 46-58 remains byte-identical to baseline: sll v0; sll a0; addu. dbr does NOT feed back into sched2's T-14 UID decision.

- [s24] -fno-omit-frame-pointer (208 lines, fp scaffolding) and -fforce-addr (205 lines, hoisted address loads) preserve the pair-window t0-first sll pair - no path to target order.

- [s24] MECHANISM CONCLUSION strengthening s6/s17: sched2's rank_for_schedule INSN_LUID assignment at T-14 (deciding the arg4-sll/arg5-sll tie) is derived from the pass's own backward dep-DAG walk, not from any pre-sched2 IR shape a -f flag can re-arrange. All tested pre-sched2 (jump/cse/loop/cse2/flow/combine) and post-sched2 (dbr) flags leave the T-14 emission unchanged.

- [s24] Global fp/dbr/sched-off flags additionally regress OTHER functions in system.c drastically (large line-count deltas 57-118 lines from small pass toggles), so even a per-file wrapper (unbuilt this session) would not resolve the tie without collateral fn-level damage.

- [s25] s25 baseline reconfirmed: candidate.c (vT40) spliced -> cc1 -da dump produced fresh (13 RTL pass files + emitted .s) via tmp/grind/marionation_Exec/s25/rankdbg.sh; src/system.c restored to HEAD via splice_apply.py --restore + git checkout; oracle green.

- [s25] Pass and line named at source level: rank_for_schedule at sched.c:2399-2456; T-14 tie decision at CLASS-compare line 2448 (correction of hypothesis 1 which named line 2455 - the LUID fallback is unreached for this pair). Neither compare's outcome is source-C reachable without changing arg5's expression tree topology (which measured KILLED at masked 8 uniformly across arg5-first-source variants).

- [s25] Chain order at sched2 entry (from greg dump lines 3838-3893 for the marionation_Exec block): 99(LUID 0) -> 115(1) -> 141(2) -> 117(3) -> 106(4) -> 120(5) -> 122(6) -> 128(7) -> 111(8) -> 137(9) -> 133(10).

- [s25] Data-dep chain confirmed: insn 117 SETS v0, insn 120 USES v0 as source of the plus. LOG_LINKS(120) contains 117; find_insn_list(117, ...) returns non-null; insn_cost(117, ..., 120) > 1 (ashift is not zero-latency); REG_NOTE_KIND == 0 (data-dep) -> class(117) = 1. For 106 (a0-sll), no LOG_LINK to 120 -> class(106) = 3.

- [s25] sched2 T-14 trace: `ready list at T-14: 106 (2) 141 (1) 117 (2), now 106 117 141` (mar_system_s25.i.sched2 line 147). 106 emitted at chronological pos 7.

- [s25] Novel infrastructure fact: sched.c source has BB2_RANK_DEBUG env-gated stderr diagnostic (lines 2436-2446) but neither built cc1 has it compiled in (build/cc1 dated May 18 predates the July 3 sched.c mod; strings|grep RANKDBG returns 0 on both build/cc1 and tmp/gccdbg/cc1). Rebuilding cc1 to activate the diagnostic is forbidden by no-compiler-divergence.md; the diagnostic is source-only until a legitimately-authorized rebuild.

- [s25] Frontier item #3 refined: rank_for_schedule tie is decided at CLASS compare (line 2448), not LUID (line 2455). A source patch would need to modify class-preference (not just LUID-preference), which is a MORE INTRUSIVE change than the ledger digest imagined. This does not change the KILL verdict - .claude/rules/no-compiler-divergence.md categorically bans sched.c patches.

- [s25] Verifies s16/s17 finding at the source-line level: 'the T-14 tie is decided by scheduler-internal state (INSN_LUID from the backward walk), not by expand-time opcode-family placement' is corrected to: 'decided at the class-compare via LOG_LINKS to last_scheduled_insn (which is fixed by arg5's expression-tree data-dep from 117 into 120)'. The class-compare outcome is INVARIANT to expand-time UID assignment because it depends on the DEP-DAG shape, which is a semantic property of the arg5 expression.

- [s26] [s26] baseline confirmed: candidate.c (vT40) spliced -> sandbox --disable all = masked 4 (178/179, 42 rules dropped, 20 cheat-asm stripped). src/system.c restored to HEAD via git checkout after every measurement; oracle green.

- [s26] [s26] NEW mechanism fact: stack-allocated s32[2] staging for the two tbl values births 4 sp-relative mem ops (2 sw + 2 lw) unconditionally, +5 build_insns over target. Stack-array-based staging is a NON-STARTER at the RTL level for this fn - the mem ops are visible bytes independent of any scheduling benefit.

- [s26] [s26] NEW basin spelling: u8 *idxp local aliasing idx_1494 in the do_timeout window compiles to byte-identical emission as vT40's direct-array form. GCC's copy-fold recognizes the trivial pointer alias; no launch penalty, no perturbation. 10th known distinct masked-4 spelling of the vT40 floor basin.

- [s26] [s26] Cumulative hand-authored structural coverage after s26: s2 (26) + s3 (22) + s8 (3) + s9 (4) + s10 (2) + s11 (30) + s12 (25) + s17 (19) + s18 (4) + s20 (>=3) + s21 (15) + s26 (2) = ~155 hand-authored structural variants. Combined with s6/s7 forensics, s4/s5/s13/s14/s22/s23 permuter campaigns (~15h+ sampling across 5 basins), the hand-derivable + local-sampler search space around the vT40 basin remains comprehensively closed at masked 4.

- [s26] [s26] Rederive modality's remaining unexplored axes: (a) cross-function transplant from the twin cpu_side_move_dir_4 - per frontier item #1 this is queue-routing (twin session needed to develop the exchange lever) and not addressable from marionation_Exec's src alone; (b) fresh m2c rebuild - ledger records masked 65 for m2c output, unlikely to structurally differ from prior hand-derived forms; (c) decomp.me corpus search for analogous scheduling patterns - untried this session but portfolio convergence at 10-spelling scale makes a novel closer highly improbable.

- [s27] s27 baseline reconfirmed: candidate.c (vT40) spliced -> sandbox --disable all = masked 4 / 178 build_insns / 179 target / 42 rules dropped / 20 cheat-asm stripped. Floor unchanged since s0 across 26 prior sessions.

- [s27] NEW mechanism refinement: the fresh-named-local launch penalty (vT33/vT34 family, ~+11 masked) applies position-invariantly to arg3 hoisting. s9v03 measured hoist-FIRST at masked 18; s27 v01 hoist-LAST-before-call also masked 18. Same magnitude regardless of whether the pseudo's textual life is wide (fn-scope) or narrow (1-stmt window between last compute and jal). Confirms the launch pathology is birthing-driven, not life-extent-driven — the mere existence of a fresh named pseudo in the do_timeout window at expand time fires the birthing_insn_p re-time.

- [s27] NEW inert axis measured: callback function-pointer cast type (void(*)(u8,void*)) vs (void(*)(s32,void*)) is spelling-inert. GCC 2.7.2 emits identical byte sequence for both prototypes because ABI zero-extension of u8 to arg register happens whether the callee declares u8 or s32 first-param. This closes an untested-but-plausible axis on idx_1495's doubly-load-bearing signature — the second (callback) use is NOT sensitive to callee prototype width.

- [s27] Cumulative structural coverage after s27: 26 sessions of measured hand-authored variants (110+ forms in s2/s3/s8/s9/s10/s11/s12/s16/s17/s18/s20/s21/s26/s27) + insn-level forensics (s6/s7/s15/s24/s25) + 5-basin permuter (~40+ CPU-hours across vT40/find105/z07/w05/s18v02) all converge on masked-4 floor with 2 root-caused residuals. The masked-4 basin now has 25 known distinct spellings — the residuals are provably spelling-invariant at basin scale.

- [s27] src/system.c restored to HEAD via splice_apply.py --restore + git checkout after every measurement; working tree clean except metrics/events.jsonl (engine-managed); oracle green throughout.

- [s28] s28 synthesis: 27-session ledger consolidated (evidence.md 553 lines + hypotheses.md 602 lines); no new sandbox measurements taken this session, floor remains masked 4 with candidate.c (vT40) unchanged.

- [s28] Pair-swap residual: sched.c:2448 class-compare selects insn 106 (class=3) over insn 117 (class=1, data-dep to last_scheduled_insn=120 via v0 pseudo). Any C rewrite must break the LOG_LINK from 120 to 117 without launching a fresh temp - measured impossible across 155+ hand-forms.

- [s28] Region-3 residual: reorg fill_simple_delay_slots steals caller-saved a1-dest move because target's callee-saved-only live-set at fall-through label rejects only the target's fill. jump2's young-label mechanism (s7: labels 591/301/85) always places the new label at check1's position (fold absorbs check2), off check2's fall-through walk.

- [s28] Portfolio convergence at 5-basin scale (vT40/find105/z07/w05/s18v02), 40+ CPU-hrs, 22 sub-215 finds - all map to 3 known attractor classes; CFG-shape-invariant (s18v02's structured if/else did NOT open new terrain).

- [s28] 25 known distinct masked-4 basin spellings enumerate the two residuals as basin-invariant; 20 remain untried as permuter seeds but s19/s23 predict convergent.

- [s28] Policy-killed levers (no ruling required): sched.c/reorg.c/flow.c patches (.claude/rules/no-compiler-divergence.md HARD RULE); cheat family (regfix/pins/__asm__ injection); cross-symbol FAKE (2026-07-05 semantic-lie ruling).

- [s28] Actionable frontier: (a) twin cpu_side_move_dir_4 exchange lever via queue-routing (NOT addressable from this function's mandate); (b) untried permuter basin with SVR-on-arg5-shift chassis; (c) ruling-request placeholder for beyond-classification constructs.

- [s28] Synthesis document: tmp/grind/marionation_Exec/s28/SYNTHESIS.md.

- [s28] src/system.c untouched; candidate.c unchanged (remains vT40, best-known masked 4); oracle green.

- [s29] s29 baseline reconfirmed: candidate.c (vT40) spliced -> sandbox --disable all = masked 4 (178/179 insns, 42 rules dropped, 20 cheat-asm stripped). Floor unchanged since s0 wip-import.

- [s29] s29 NEW mechanism fact: the copy-loop body's internal statement axis (excluding i-decrement position) is spelling-inert across 5 measured spellings (no-temp / post-inc-idiom / s32-temp / stores-before-advance / pre-decrement-while); all reach masked 4 with 178 build_insns.

- [s29] s29 NEW mechanism fact: i-decrement POSITION within the copy-loop body is load-bearing. Moving `i--` to the top of the loop body (before the bb load) regresses by +4 masked at same build count 178. This is a novel load-bearing axis not previously catalogued in the 25+ prior masked-4 spelling census.

- [s29] s29 basin census: known distinct masked-4 spellings now 30 (was 25 at s28), extending across the copy-loop-body internal-structure axis. Adds: v01_no_bb_temp, v02_postinc_idiom, v04_s32_bb, v05_stores_before_advance, v06_predec_while.

- [s29] s29 src/system.c restored to HEAD via splice_apply.py --restore + git checkout after every measurement; working tree clean (except metrics/events.jsonl and the new rejected/ file); oracle green throughout.

- [s29] s29 cumulative hand-authored structural coverage: prior 155+ variants (s2/s3/s8/s9/s10/s11/s12/s16/s17/s18/s20/s21/s26/s27) + s29 6 copy-loop-body variants = 161+ measured. The copy-loop-body internal-structure axis was previously untested per s21 note (only i-scoping was tested, not body-statement-order or bb-typing or ptr-advance-position). Now measured negative.

- [s30] s30 baseline reconfirmed: candidate.c (vT40) spliced -> sandbox --disable all = masked 4 (178/179 insns, 42 rules dropped, 20 cheat-asm stripped). Floor unchanged since s0.

- [s30] s30 v01 (cnt as arg5-shift SVR carrier): masked 6 / 178 - existing s32-typed cross-region pseudo staging regressions +2 masked; cnt's callee-saved lifetime through the vsync-check store into do_timeout creates seat competition at the debug_printf call.

- [s30] s30 v02 (status as arg5-shift SVR carrier): masked 6 / 178 - identical regression to cnt; the mechanism is pseudo-life-agnostic among existing-pseudo cross-region carriers.

- [s30] s30 v03 (fresh named-local shift_carrier as arg5-shift SVR): masked 4 / 178 - INERT, adds 31st known distinct masked-4 basin spelling. NEW mechanism fact: the vT33/vT34 fresh-temp launch pathology does NOT fire when the fresh pseudo's birth is a plain byte load followed by self-mutating shift (structurally identical to v0's staging). The launch penalty is opcode-and-shape driven, not fresh-pseudo-driven per se.

- [s30] s30 refines s28 frontier item #2: the SVR-on-shift-carrier axis is now measured across v0 (vT40 base=4), cnt/status (existing dead cross-region=6), and fresh named-local (=4). The axis is closed - v0 is not uniquely load-bearing among structurally-analogous carriers.

- [s30] s30 refines vT33/vT34 launch mechanism from s2/s9: `fresh named local` alone does NOT fire launch; the launch fires when the fresh pseudo births an ARITHMETIC SUM or intermediate (vT34=fresh_sum), not a plain load-then-shift.

- [s30] s30 cumulative hand-authored structural coverage: prior 161+ variants (s2/s3/s8/s9/s10/s11/s12/s16/s17/s18/s20/s21/s26/s27/s29) + 3 new = 164+ measured. Basin membership now 31 distinct masked-4 spellings.

- [s30] src/system.c restored to HEAD via splice_apply.py --restore + git checkout after all measurement; working tree clean; oracle green.

- [s31] vT40 candidate reproduces masked 4 / build 178 / target 179 with 42 rules dropped, 20 cheat-asm stripped — baseline confirmed at session start.

- [s31] Frontier item #3 (arg5-first via load-then-self-shift fresh carriers): 3 refinements measured (v01 both-fresh arg5-first = 11; v02 both-fresh t0-first = 10; v03 arg5-only-fresh arg5-first = 9). Fresh-carrier launch-escape (s30 v03) is BOTH source-order-conditional (does not survive arg5-first flip) AND single-side-only (composability across both temps regresses +6 even in winning t0-first order). Escape is a fragile privilege of one specific spelling.

- [s31] s30 v03 basin permuter campaign (26 min, 6 CPU-hr): 5 finds all 200-220 permuter score; 0 sub-200 attractors. Portfolio convergence confirmed on the 6th sampled basin chassis. Cumulative cross-basin coverage: 6 chassis / ~46+ CPU-hr / 0 novel-attractor find.

- [s31] Basin census extends to 31 known distinct masked-4 spellings (s30 count) — none tested by permuter beyond the 6 named chassis. Basin-invariance remains a strong prior at 6-basin scale.

- [s31] Src restored to HEAD (candidate stays in memory/grind/marionation_Exec/candidate.c). Campaign PID 592 confirmed dead post-harvest; no simmering processes.

- [owner 2026-07-08] cc1psx CALIBRATION — toolchain-divergence hypothesis KILLED for the order residual. Experiment: HEAD src/system.c preprocessed once (exact Makefile cpp flags), compiled by BOTH tools/gcc-2.7.2/build/cc1 (port) and tools/cc1psx_wrapper.sh (PsyQ GNU C 2.7.2.SN.1 via dosemu), identical maspsx pipeline, marionation_Exec extracted at offset 0, objdump-normalized + register-name-masked. RESULT: port vs psx masked diff = 0 lines (177 insns each) — the ORIGINAL compiler emits the IDENTICAL instruction ORDER from this C; only register SEATS differ (operationally irrelevant: the oracle is built with the port). Port vs target = psx vs target = 38 masked lines, in the SAME windows (prologue save-order/slots, pair-window addu/lw split, region-3 andi/beqz/sb). cc1psx is not one instruction closer. CONSEQUENCE: the pair-swap + region-3 residuals are NOT port-vs-SN scheduler divergence; the target order came from DIFFERENT C source. The novel-statement-geometry search is the confirmed path (compiler-parity doctrine now proven for THIS function on THIS source). Artifacts: tmp/cal_mar/ (port.s, psx.s, *_insns.txt, *_m.txt; scripts tmp/cc1psx_marionation_cal.sh, tmp/cc1psx_mar_score.sh, tmp/cal_mar_masked.sh).

- [s32] s32 perm_s26idxp base score = 220 confirmed (permuter --debug -j1) - identical residual signature (2 reorderings x60 + 1 deletion x100) to vT40/z07/w05/s18v02/s30v03/s29v06.

- [s32] s32 perm_s26idxp campaign: 34034+ iterations at -j6 across ~22 minutes; 6 sub-220 finds triaged: output-215-1 (2 hits, do-while(0) hoist), output-200-1 (2 hits, regression), output-170-1 (label-alive class variant), output-165-1 (label-alive class).

- [s32] s32 perm_s26idxp output-165-1 / output-170-1 are the FOURTH and FIFTH documented spellings of the label-alive attractor class: (i) vP160 `while(status)` uninit-read; (ii) s5 0xFF-through-local `while(status=0)`; (iii) z07-160-1 `status=0; ... while(status);`; (iv) s32-s26idxp-165-1 `*idx_1496=(status=0); ... while(status); return status;`; (v) s32-s26idxp-170-1 explicit `status=0` before check2 cascaded through `if (a1 != status)`, `while (status);`, `return status;`. Each pays visible bytes (180 insns) or exhibits semantic divergence; class REJECTED as a matcher regardless of basin.

- [s32] s32 perm_s29v06 pre-existing campaign log documents 1049 iterations with 0 sub-220 finds - portfolio convergence extends to the 7th sampled basin.

- [s32] Cumulative portfolio scale after s32: 8 sampled basins {vT40, find105, z07, w05, s18v02, s30v03, s29v06, s26idxp} across s4/s5/s13/s14/s22/s23/s31/s32 (~46+ CPU-hr per s31 + ~5.6 CPU-hr this session + prior s32 abandoned ~1049 iters), 0 novel-mechanism sub-200 attractors, all sub-200 finds classify into 3 known families (label-alive +1-insn, alias-merge masked>=10, permuter reg-diff shuffles).

- [s32] Basin-invariance of the {pair-swap @56/57, region-3 steal @149} residuals is now measured negative across 8 distinct chassis; the 25-30 remaining basin members per s28/s31 census very likely fall into the same convergence pattern.

- [s32] src/system.c untouched this session (git status src/ clean); candidate.c unchanged (remains vT40 masked 4 best-known); oracle green.

- [s33] s33 baseline: HEAD src/system.c sandbox --disable all = 56 (176/179); candidate.c (vT40) spliced = masked 4 (178/179, 42 rules dropped, 22 cheat-asm stripped). Floor unchanged. src restored to HEAD via git checkout after all dumps; working tree src/ clean.

- [s33] s33 infrastructure fact: tmp/gccdbg/cc1 (Jul 6 build) has ALL env knobs compiled in (BB2_QTY_DEBUG, BB2_ALLOC_DEBUG, BB2_RANK_DEBUG, BB2_FINDREG_DEBUG, BB2_PRIO_DEBUG, BB2_SLL_DEBUG, BB2_DBR_DEBUG, BB2_ALLLIVE_LABEL, BB2_NO_FT_STEAL); tools/gcc-2.7.2/build/cc1 (May 18) has NONE of them. QTYDBG output has no function markers - marionation_Exec's cluster is anchored by the 'conflicting types' parse warning (system.c:500) that immediately precedes it; do_timeout = blk 3.

- [s33] s33 QTYDBG vT40 blk3: addu-arg5addr reg104 (b18 d20 r4 pri4.00)->v0; arg3-web sll reg110 (b22 d30 r8 pri3.00)->v0; arg5val reg97 (b20 d26 r4 pri1.33)->v1; t0-sll reg102 (b16 d24 r4 pri1.00)->a0. No tie in vT40: t0-sll's early birth stretches life to 8, making it the strict priority loser that allocates LAST into a0. Correct seats are a consequence of the wrong order.

- [s33] s33 QTYDBG trade8 blk3: addu reg103 (b14 d20 r8 pri4.00)->v0; arg3-web reg111 (b22 d30 r8 pri3.00)->v0; t0-sll reg105 (b18 d24 r4 pri1.33)->v1; arg5val reg97 (b20 d26 r4 pri1.33)->a0. EXACT 1.33v1.33 tie (current-chassis correction of the o1-era 5.33v5.33 ledger numbers), broken at local-alloc.c:1646 (qty_compare_1 'return *q1 - *q2') by qty number = block birth order, which byte order pins in t0-sll's favor (addu < t0-sll < arg5-lw stream requirement).

- [s33] s33 seat propagation: one comparison decides both seats - the tie winner (t0-sll 18-24) conflicts v0 (regs 103/111) and takes v1, pushing arg5val (20-26, v1 now blocked) to a0; had arg5val allocated first it would take v1 (free at 20-26) and t0-sll would land a0 = exact target seats.

- [s33] s33 qty_sugg KILL: qty_phys_(copy_)sugg is created ONLY in combine_regs (local-alloc.c:1822-1861) from hardreg<->pseudo COPY insns; the pair window's insn set has no copy insn and neither pseudo can acquire one in any residual-preserving spelling; zero QTYDBG-SUGG blk=3 lines in both chassis. Suggested qtys allocate before ALL unsuggested (1469-1490), so the lever WOULD flip the trade if it were reachable - it is not.

- [s33] s33 flip condition (closed form, for vetting future candidates): strict pri(arg5val)>pri(t0-sll) requires life(arg5val)<life(t0-sll) (refs equal) = the arg5 sw within 1 sched1 slot of the t0-deref lw (sched1 normalization fixes both at 6 across all measured orderings), OR refs(arg5val)>=5 (only via loop-note ref-weighting scoped to arg5val alone - wrap toolbox Judge-banked dead at this window). Artifacts: tmp/grind/marionation_Exec/s33/.

- [s34] Baseline reconfirmed at s34 start: candidate.c (vT40) spliced -> sandbox --disable all = masked 4 (178/179, 42 rules dropped, 20 cheat-asm stripped); src/system.c restored to HEAD (git checkout) after all measurement, oracle untouched.

- [s34] v04 QTYDBG blk=3 table (anchor stderr line 155): addr-temp reg105 pri 4.00 -> v0; arg3-web reg111 pri 4.00 -> v0; TIED t0-web reg98 birth18-death32 refs12 pri 2.57 -> v1 (target a0); arg5val reg97 birth16-death20 refs4 pri 2.00 -> v0 (target v1).

- [s34] v04 emitted window achieves TARGET pair order (sll v0; addu v0,v0,s5; sll) from t0-first source - the tied web flips sched1's launch order; adiff2 confirms the arg5 half sits in the unchanged LCS.

- [s34] Fresh-carrier escape asymmetry settled: arg5-side carrier = masked 4 (s30v03, 31st basin member), t0-side carrier = masked 11 (NOT a basin member). Frontier item #2's 32nd-member hypothesis dead.

- [s34] Tied-web refs decomposition: 12 = (3 sets + 3 uses) x2 loop-note weight from the do_timeout do-while(0) wrap; removing the wrap halves both sides (tied 0.86 vs arg5val 0.5 - trade persists) and loses tbl_125c's s5 seat.

- [s34] Pre-vet method validated as a triage filter for permuter finds (frontier item #3): compute pri(arg5val) vs pri(t0-side) from the find's QTYDBG blk=3 table; only pri(arg5val) strictly greater is worth a sandbox run in an order-correct form.

- [s35] s35 baseline reconfirmed: candidate.c (vT40) spliced -> sandbox --disable all = masked 4 (178/179, 42 rules dropped, 20 cheat-asm stripped); src/system.c restored to HEAD after all measurement, working tree src/ clean, oracle green.

- [s35] s35 decomp.me corpus search run for the FIRST time on this function (s26 had it catalogued untried): 3754 local scratches in tmp/decomp_me_corpus, top similarity 0.094 - noise tier. The corpus rederive lever is exhausted; no BB2-toolchain-class scratch shares the do_timeout pattern.

- [s35] s35 NEW mechanism fact: the t0-side fresh-carrier launch penalty (masked 11) is holding-type-invariant - s32 (s34 v04), s32* (s8 v03), u8* (s35 v01), and u8*-with-split-init-accumulation (s35 v02, bpAxD corpus idiom) all measure 11; symmetric u8* carriers both sides = 12. The penalty is web-shape-driven (fresh carrier absorbing the addr-temp), not type-driven; no pointer type escapes it.

- [s35] s35 NEW basin member #32: m2c-derived ||-comma outer flow (v04) = masked 4 / 178 with adiff2-verified byte-identical residual windows (pair 56/57, region-3 149/151). Fresh m2c yields no structurally new information beyond this - its do_timeout window is the known killed inline-args shape (s8 v02 = 16 class) and its whole-body rebuild remains the ledger's masked-65 shape.

- [s35] s35 rederive modality is now fully exhausted per s26's own residual-axis list: (a) twin transplant killed s8, (b) fresh m2c rebuild yields only basin member #32, (c) decomp.me corpus searched and killed. No rederive-class lever remains; the live frontier items (v05 carrier+deferred-add pre-vet, twin exchange lever, basin-seed permuter) belong to structural/operator/permuter modalities respectively.

- [s36] s36 baseline reconfirmed: candidate.c (vT40) spliced -> sandbox --disable all = masked 4 (178/179, 42 rules dropped, 20 cheat-asm stripped); src/system.c restored to HEAD (git checkout) after all measurement, working tree src/ clean.

- [s36] s36 KILL: Kengo's marionation_Exec (0x141400, name-unique match, 180 insns) is a full PS2 rewrite - FPU/motion/sound calls, no polling loop, no debug_printf window; the Marionation-engine-preserved assumption fails for this function; kengo transplant lever closed.

- [s36] s36 KILL: find_duplicates.py (first run since merge, tmp/duplicates_leads.txt now exists) has zero marionation_Exec rows - no whole-function near-clone lead.

- [s36] s36 NEW SIBLING FACT: saEft01Init (system.c:806, queue dist 18, 15 rules) has a byte-shape-IDENTICAL do_timeout debug_printf window to marionation_Exec vs both targets (same D_800F19C0/D_800A11D5/D_800161C8 globals + fmt string; only table-base seat s0-vs-s5 and arg3-table addressing s3-hoisted-vs-at-direct differ). Its current C (named arg5-first, no pp, no webs) emits the target middle-addu order but suffers the SAME seat trade vs its own target. One unknown original spelling decides both windows. Custom scanner proved NO completed function in the binary contains the pair shape (3 hits, all queued).

- [s36] s36 NEW mechanism fact: the pp pointer-alias is INERT in the named-local family - v02 (named arg4-first, no pp) = 9 exactly matches s9v01 (with pp) = 9; v01 (arg5-first no pp) = 10 vs s9v02 (with pp) = 11. The lw a1 D_800F19C0 lands in the target slot from a plain direct arg. pp's ~7pt contribution is specific to the web-staging chassis.

- [s36] s36 NEW mechanism fact: v01 (sibling shape) is the simplest order-correct spelling ever measured - target middle-addu pair order from honest plain C with zero FAKE constructs in the window, at masked 10 (seat trade + delay-slot lbu flip + arg4 addu/deref placement drift).

- [s36] s36 KILL: index-load hoisting cannot fix the arg5-first lbu-order flip - v03's i0 hoist copy-propagates away (emission byte-identical to v01); lbu launch order follows expression-tree order at sched2 T-16, not statement order of index loads.

- [s36] s36 rejected/ additions: s36-sibling-saeft01init-shape-order-correct-seats-traded-10.c, s36-named-arg4-arg5-nopp-9.c, s36-i0-hoist-copyprop-folds-emission-identical-10.c. candidate.c unchanged (vT40 masked 4 remains best).

- [s37] s37 baseline reconfirmed: candidate.c (vT40) spliced -> sandbox --disable all = masked 4 (178/179, 42 rules dropped, 20 cheat-asm stripped); src/system.c restored to HEAD, working tree src/ clean, oracle green.

- [s37] s37 synthesis: pair-swap flip condition consolidated to ONE open arithmetic route - fresh t0 carrier (s30v03 launch-free load-then-self-shift shape) + tbl-add deferred into the call deref gives web refs 8 (pri 1.71) vs arg5val pri 2.00; its two measured neighbors s2 v30 (native web + deferral = 16) and s34 v04 (carrier + add = 11) are mechanism-distinct so neither kills the combination a priori.

- [s37] s37 synthesis: the three named pair-swap decision routes (sched.c:2448 class-compare for t0-first; local-alloc.c:1646 birth-order tiebreak for arg5-first; combine_regs tied-web strict-priority for carrier forms) are mutually exhaustive over all 165+ measured hand forms; order-correctness and seat-correctness were mutually exclusive on every measured route.

- [s37] s37 synthesis: region-3 falls only to the unknown original source spelling - liveness route can never byte-match (a1 caller-saved), label routes always pay +1 insn (5 catalogued spellings), cc1psx parity proves toolchain is not the variable; the saEft01Init exchange addresses the pair window only (sibling has no check region).

- [s37] s37 synthesis document written to tmp/grind/marionation_Exec/s37/SYNTHESIS.md (proven-facts consolidation, merged attack P1/P2/P3, dead-end list); no new structural forms measured this session, candidate.c unchanged (vT40 remains best-known).

- [s38] s38 baseline reconfirmed: candidate.c (vT40) spliced -> sandbox --disable all = masked 4 (178/179, 42 rules dropped, 20 cheat-asm stripped); src/system.c restored to HEAD (git checkout) after all measurement, working tree src/ clean.

- [s38] AUTHORING FACT: the frontier P1's literal single-carrier reading (load-then-self-shift + deferred add) is alpha-equivalent to banked v30 (deferred-t0-tbladd-in-call-16.c) - variable renaming is invisible to GCC; the two-pseudo tied form (tc 1s+1u, t2 1s+1u) is the only novel spelling satisfying the refs-8 arithmetic.

- [s38] NEW mechanism fact: C-level deferral of the tbl-add does NOT reduce the t0-side web's RTL refs - expand materializes the addu (compiler temp) and a deref value-holder (reg/v) as real insns in the call-arg tree; QTYDBG blk=3 shows a 12-ref qty (b20-32) reconstituting in all three spellings. qty_compare refs are pinned by semantic operation count, not C statement count.

- [s38] NEW mechanism fact: the deferred-add retime (s2 v30 class) is carrier-invariant and holding-type-invariant - v05a (s32 tied pair), v05b (both sides fresh carriers), v05c (u8* t2) all emit the byte-identical serialized window: arg5 chain complete first, t0 chain inside the call-arg region, arg5 lbu in the tslTm2 delay slot.

- [s38] FLIP-CONDITION EXHAUSTION: all four arithmetic axes of the s33 closed-form pair-flip condition are now measured or proven closed (raise refs(arg5val) / shorten life(arg5val) / lengthen life(t0-side) / lower refs(t0-side)). The pair-swap residual has NO remaining arithmetic route; it falls only to the unknown original source spelling (P2 sibling exchange) or unknown-shape sampling (P3).

- [s38] s34 dump-first pre-vet protocol re-validated as a zero-sandbox KILL gate: all three verdicts issued from emitted-window order alone; no sandbox runs spent on order-incorrect forms.

- [s38] 3 disproven forms banked: rejected/s38-v05a-tiedpair-deferred-add-order-incorrect-dumpvet.c, rejected/s38-v05b-both-carriers-deferred-add-order-incorrect-dumpvet.c, rejected/s38-v05c-u8ptr-carrier-deferred-add-order-incorrect-dumpvet.c. candidate.c unchanged (vT40 masked 4 remains best-known).

- [s39] s39 baseline reconfirmed: candidate.c (vT40) spliced -> sandbox --disable all = masked 4 (178/179, 42 rules dropped, 20 cheat-asm stripped); src/system.c restored to HEAD via git checkout after all measurement, working tree src/ clean, oracle green.

- [s39] saEft01Init rule decode (frontier bullet 3 executed): the @35-48 reorder rule maps its plain-C build to target as: idx-lbu pair swapped (idx[0] first), arg5-deref hoisted before arg3's lbu, arg4's tbl-addu deferred after arg3's lbu, arg4-deref deferred to last; seat rules put arg4's chain in a0 and arg5's addr/deref in v0/v1. Fingerprint shape-identical to marionation_Exec's target window - concrete constraints for the P2 exchange session recorded in tmp/grind/marionation_Exec/s39/findings.md.

- [s39] NEW mechanism fact: arg3-index staged through the dead v0 (staged-value-reused-variable) creates WAR edge 122(lw v1)->128(lbu arg3), lengthening arg5's chain-to-jal; the pair emits in TARGET middle-addu order from t0-first source - order-correctness is NOT arg5-first-exclusive (refines s37's three-route synthesis: a fourth route exists via anti-dependence chain-lengthening).

- [s39] NEW mechanism fact: the WAR-edge route is web-growth-dominated - v0's 3rd set extends its web across the call-arg region at ~+12 masked (s3 web-growth class), uniformly across 6 spellings/chassis (16/16/18/18/20/16). v0 is the only dead-in-window variable, so the WAR edge cannot be had without the web growth; axis comprehensively closed.

- [s39] P3 triage corollary: any permuter find showing middle-addu order from t0-first source should be QTYDBG-pre-vetted for a 3-set v0 web (blk=3) - that signature predicts masked >=16 with no sandbox run.

- [s39] v07 (plain unshifted idx locals, inline derefs) = masked 16, matching the s8v01 no-web class - staging webs remain load-bearing under the sibling-decode cue.

- [s39] 6 disproven forms banked to rejected/: s39-arg3-idx-staged-v0-war-edge-order-flips-webgrowth-16.c, s39-arg5first-arg3v0-16.c, s39-sibling-chassis-arg3v0-both-orders-18.c, s39-arg3v0-byteoffset-20.c, s39-t0add-deferred-arg3v0-16.c, s39-plain-idx-locals-inline-derefs-16.c. candidate.c unchanged (vT40 masked 4 remains best-known).

- [s40] s40 inherited a discarded prior-attempt's scratch: 3 built campaign workspaces (perm_s35orcomma, perm_s20status, perm_s11u10 - exactly the frontier P3 seeds) + 5 banked rejected forms; all 3 campaigns were dead processes with no outcome JSON. Resumed rather than re-derived.

- [s40] All 3 campaigns harvested + stopped via tools/permuter_campaign.py with telemetry (owner directive 2026-07-07): s35orcomma 5 finds/all-220; s20status 14 finds over 6.5h (145-1 @ 9936s, 170-1 @ 14051s); s11u10 relaunch 6720 iters/25min/1 find-220.

- [s40] perm_s20status output-145-1 sandbox = masked 17 @ 179/179 - the alias-merge attractor (idx_1494 -> dead dst2) re-derived at EXACTLY s4's find145 family entry score on a 9th distinct chassis. Family remains KILLED (target move dest a1 caller-saved; family floor 10).

- [s40] perm_s20status output-170-1 sandbox = masked 21 @ 179/179 - NEW family variant: idx_1496 staged through dst2 with check-region reads (*dst2, *(dst2-1)); the check-side alias forces a deeper callee-saved seat cascade than the idx_1494-side variant (21 > 17). Banked rejected/s40-perm-s20status-aliasmerge-dst2-idx1496-checkside-21.c.

- [s40] Portfolio convergence now measured across 11 sampled basins (prior 8 + s35orcomma + s20status + s11u10): every sub-200 find across ~50+ CPU-hours maps to the 3 known attractor classes (alias-merge masked>=10, label-alive +1-insn, score-inert shuffles). Frontier P3's three NAMED unsampled members are now all sampled and dry.

- [s40] vT40 baseline floor 4 (178/179, 42 rules dropped, 20 cheat-asm stripped) reconfirmed this session after triage; src/system.c restored to HEAD via splice_apply.py --restore + git checkout, working tree src/ clean, oracle untouched.

- [s40] s20status's 11x220-tier finds all classify to the clear-store-repositioning and spelling-inert classes already banked from s35orcomma (v0 = v0 << 2, idx_1494+1 operand swap, split-init idx_1496, dst2-guard brace shuffle) - no new banking needed.

- [s41] s41 baseline reconfirmed: candidate.c (vT40) spliced -> sandbox --disable all = masked 4 (178/179, 42 rules dropped, 20 cheat-asm stripped); src/system.c restored to HEAD via s6/splice_apply.py --restore + git checkout, oracle untouched, working tree src/ clean at session end.

- [s41] s41 workspace tmp/grind/marionation_Exec/s41/perm_v11 built on the previously-unsampled s21 v11_vsync_hoist_local basin member (sv prologue local for VSync value); permuter --debug reported base score = 220 = 2 reorderings * 60 + 1 deletion * 100 (identical residual signature to vT40/z07/w05/s18v02/s30v03/s29v06/s26idxp/s35orcomma/s20status/s11u10 = 11 prior sampled basins).

- [s41] s41 v11 campaign launched via tools/permuter_campaign.py launch (owner directive 2026-07-07 telemetry compliance, label s21v11-vsync-hoist, pid 622, jobs 6, --stop-on-zero); 25-min fresh-seed window per permuter-fresh-seed-discipline; ~1740 iterations logged; harvested via tools/permuter_campaign.py harvest --stop.

- [s41] s41 find output-200-1 (35.8s from launch, permuter score 200): extracted via tmp/grind/marionation_Exec/s41/extract_find.py, spliced into src/system.c via s6/splice_apply.py, sandbox --disable all = masked 8 (+4 regression vs vT40 floor 4); build_insns 178 vs target 179; novel construct is `status = *((s32 *) (v0 + ((s32) tbl_125c))); arg5 = status;` extending status's life across the do_timeout debug_printf call (same class as s12's *idx_1495 cross-window-alias regression pattern).

- [s41] s41 rejected bank: memory/grind/marionation_Exec/rejected/s41-perm-v11-arg5-eq-status-alias-8.c (the v11 find200 form; novel construct killed at masked 8).

- [s41] s41 elapsed_s field in campaign_meta.json harvest output (88.5) is a metadata artifact (log mtime-derived elapsed calculation); actual fresh-seed window was 25 min from launch_epoch 1783525141 per campaign_meta.json to harvest, with campaign.log recording iter counts up to 1740+ during that period; harvest telemetry accepted for the ~1740 iters / 1 find data point.

- [s41] s41 portfolio convergence extends: 11 sampled basins {vT40, find105, z07, w05, s18v02, s30v03, s29v06, s26idxp, s35orcomma, s20status, s11u10} -> 12 including v11; cumulative ~50+ CPU-hr sampling; ZERO novel-mechanism sub-200 attractors across all 12; all sub-200 finds classify to 3 known families (alias-merge masked>=10, label-alive +1 insn, cross-window-alias/reg-shuffle score-inert-to-slight-regression).

- [s41] s41 candidate.c unchanged (remains vT40 masked 4 best-known); no src/ edits persist; oracle green.

- [s42] s42 baseline reproduced: candidate.c (vT40) spliced -> sandbox --disable all = masked 4 (178/179, 42 rules dropped, 20 cheat-asm stripped). Fresh cc1 -da dumps at s42/baseline/.

- [s42] Baseline insn 122 direct successors in sched2: 128 (anti-dep via v0 register-allocator reuse, ALREADY PRESENT), 137 (true-dep via v1), 147 (call anti-dep via memflush). Longest 122->jal forward path = 122->128->133->143->147, length 4.

- [s42] s39's arg3-through-v0 staging FORCED the 122->128 edge from source but the RA-produced edge was already there; s39's +12 masked penalty came from v0-web growth (3-set pseudo), not from edge-adding.

- [s42] GCC 2.7.2 sched.c memrefs_conflict_p returns provably-disjoint for SYMBOL_REF vs different-SYMBOL_REF (line 748-750). Any store to a fixed global address that isn't tbl_125c creates NO edge with 122.

- [s42] REG-based writes require materializing the address REG. Any new local pointer birth grows a pseudo web (defeats P4 constraint 'no in-window pseudo growth').

- [s42] Existing param a1 could carry a mem-store without new pseudo births (a1 already live for check1/check2 copies), but *a1 = <expr> is a user-observable buffer write (Judge-verboten semantic-lie).

- [s42] 137's dep entry (`insn_list:REG_DEP_ANTI 99 (REG_DEP_ANTI 115 (93 (122 (nil)))))`) shows 122 as a true-dep predecessor via v1 — sched.c find_insn_list dedups any would-be mem-anti-dep. Corollary: adding a stack-slot store cannot create a new 122-successor edge.

- [s42] The 122-forward-chain length in baseline IS its natural maximum under RA v0-reuse. No C-level statement can extend it via mem-edges without violating pseudo-web-growth or semantic-lie constraints.

- [s42] Portfolio synthesis with s6/s25/s33/s41: pair-swap and region-3 residuals are compiler-internal decisions with no C-level lever addressable within the vT40 basin's mechanism space. P2's cross-function operator-mediated exchange is the only remaining pathway.

- [s43] s43 baseline dump: fresh cc1 -da at tmp/grind/marionation_Exec/s43/mar_s43.i.{rtl,jump,cse,loop,cse2,flow,combine,sched,lreg,greg,jump2,sched2,dbr} + mar_s43.s (candidate.c spliced then reverted; src/system.c clean at HEAD after).

- [s43] MECHANISM FACT (new, forensic): MIPS gcc-2.7.2 defines function-units only for memory (load/store/xfer) and imuldiv (hilo/imul/idiv) — arith insns (sll, addu, ori, add, etc.) have NO define_function_unit and default to latency 1. This makes the sched.c:2420 cost==1 shortcut always fire for arith->arith and arith->mem edges within a single cycle, forcing class=3 in rank_for_schedule for BOTH sides of every pair-window ready-list tie in BB #3. The class branch is inert; LUID always decides.

- [s43] s6's characterization of the T-14 tie as 'INSN_UID / LUID decided' is mechanism-correct — the class branch that precedes it in rank_for_schedule cannot differentiate the arith candidates in this window.

- [s43] IMPLICATION: any hypothesis in which a C-level construct alters the sched2 tie via changing 117's dep cost or class relative to 120 (or 120 vs 122, or 128 vs 111) is structurally KILLED — the MIPS latency model provides no differentiating cost > 1 for arith-only chains.

- [s43] The LUID lever remains the ONLY sched2-tie-flip axis in the pair window, and s2/s3 measured all 140 dependency-valid orderings + 26+22 geometry/decomposition forms + the y-family operand-swap probes without a masked-3 or lower closer.

- [s43] INSTRUMENTATION NOTE: BB2_RANK_DEBUG env-gated debug in sched.c:2436-2446 exists in tools/gcc-2.7.2/build/cc1 (strings|grep BB2_RANK confirms) but did not emit RANKDBG lines when invoked via tools/wsl.sh with `export BB2_RANK_DEBUG=1` in three attempts (rank_debug*.txt empty of RANKDBG). Likely wsl.sh nested-shell env-forwarding drop. Not needed for this KILL since the mips.md read directly proved insn_cost==1 for arith. Future forensic sessions wanting live RANKDBG capture should invoke cc1 via a shell script that exports before the first spawn.

- [s44] s44 baseline reproduced: candidate.c (vT40) spliced -> sandbox --disable all = masked 4 (178/179, 42 rules dropped, 20 cheat_asm stripped). Floor unchanged since s0.

- [s44] s44 v01 (cnt = D_800F19BC++ single-statement postfix) = masked 4 exactly - novel outer-counter spelling not in the 89-entry rejected bank, banked as rejected/s44-postinc-cnt-masked4-basin-equiv.c. Outer counter idiom is qty-INERT at the do_timeout / region-3 residuals; the postfix single-statement compound emits identically to the split 2-statement read-then-write form for this GCC 2.7.2 codegen.

- [s44] 10 distinct masked-4 spellings now catalogued for the vT40 floor basin: vT40, s11 u10/w03/w10, s12 v08/w05/w08/z01/z02/z07, s44 postinc-cnt. Every basin member preserves the pair-swap @56/57 seat-trade coupling and the region-3 @149 dbr steal; the two residuals are basin-wide invariants across outer-counter, staging, cast-type, and pp-placement axes.

- [s44] src/system.c restored to HEAD via splice.py restore (git checkout) after every measurement; working tree clean; oracle green throughout.

- [s45] s45 baseline reconfirmed: candidate.c (vT40) spliced -> sandbox --disable all = masked 4 (178/179, 42 rules dropped, 20 cheat-asm stripped). src/system.c restored via git checkout after every measurement; oracle green; working tree clean

- [s45] s45 rederive angle: prologue-setup stmt-order axis (NEW - not in the s2/s3/s8/s9/s10/s11/s12 rejected forms bank which covered only do_timeout-window structural axes)

- [s45] NEW basin members: vA (D_800F19BC/D_800F19C0 stmt reorder) and vC (tbl_125c/idx_1494 stmt swap) both masked 4. Combined with prior 9 basin members (vT40, s11 u10/w03/w10, s12 v08/w05/w08, z01/z02/z07), vT40 floor basin now has >=11 known distinct spellings across compound-restructure, mirror, tbase-alias, and prologue-reorder axes - the residuals are structural basin-invariants

- [s45] NEW mechanism fact: idx_1494/1495/1496 forward-derivation chain is LOAD-BEARING; backward-chain from idx_1496 via subtraction regresses +10 (adds subtract insns that fuse in canonicalization but restructure the prologue-vs-do_timeout qty cascade)

- [s45] NEW mechanism fact: comma-expression fold of the setup pool regresses +6 - comma sequence points do not free-fold at expand for these global-store stmts; introduces intermediate temps for sub-expression values that couple with do_timeout-window qty allocation

- [s45] Cumulative structural coverage after s45: s2 (26) + s3 (22) + s8 (3) + s9 (4) + s10 (2) + s11 (30) + s12 (25) + s45 (4) = 116 hand-authored structural variants of marionation_Exec measured. Combined with s6/s7 forensics + s4/s5 permuter (~10k samples), the hand-derivable + local-sampler basin around vT40 is comprehensively closed at masked 4

- [s46] Floor invariant at masked 4 across 45 sessions (vT40 candidate.c reproduces at s1-s45)

- [s46] Pair-swap residual: sched2 BB #3 T-14 tie between insn 106 (a0-sll) and insn 117 (v0-sll); LUID decides because mips.md defines no arith function-unit and cost=1 shortcut collapses class to 3 for both (s43)

- [s46] Pair-swap seat trade: qty_compare_1 pri 1.33 v 1.33 tie broken by birth order at local-alloc.c:1646 (s33 QTYDBG blk=3); byte order pins tie in t0-web's favor

- [s46] Region-3 residual: dbr fill_simple_delay_slots pass #1 steals move a1,s4 into check2 beqz slot because a1 is caller-saved and absent from fall-through label's callee-saved-only live-set (s6/s7)

- [s46] Cross-symbol FAKE forbidden per 2026-07-05 semantic-lie ruling; do-while(0) wraps around the do_timeout window retime head (Judge-banked vT35/vT36/vT42/vT43)

- [s46] cc1psx (PsyQ GNU C 2.7.2.SN.1 via dosemu) emits IDENTICAL instruction ORDER to port cc1 from THIS source (owner 2026-07-08 calibration); target order came from DIFFERENT C source

- [s46] Portfolio convergence: 12 sampled basins {vT40, find105, z07, w05, s18v02, s30v03, s29v06, s26idxp, s35orcomma, s20status, s11u10, s21v11} × ~50+ CPU-hr yield 0 novel-mechanism sub-200 attractors; all sub-200 finds classify to 3 known families (alias-merge >=10, label-alive +1 insn, reg-shuffle score-inert)

- [s46] Basin has 11+ distinct known masked-4 spellings enumerating structural axes to closure

- [s46] Sibling saEft01Init (system.c:806) has byte-shape-IDENTICAL do_timeout window fingerprint; one unknown original spelling decides both windows (s36/s39)

- [s46] Twin cpu_side_move_dir_4 (system.c:388, queue dist 7, 5 rules) has same pair-swap coupling as development bench per 2026-07-07 fable-blitz strategy; pair-only residual (no region-3), cheaper to develop the exchange there first

- [s47] s47 baseline: candidate.c (vT40) spliced -> sandbox --disable all = masked 4 (178/179, 42 rules dropped, 20 cheat_asm stripped). Floor unchanged since s0.

- [s47] s47 NEW mechanism fact: named-local constant-holder mask preservation is LIFETIME-DEPENDENT, not naming-dependent. Function-scope int new_var = 0xFF keeps both andi's alive; block-scope int new_var = 0xFF (single-use within compound stmt) causes combine to fold the mask into the surrounding lbu/andi, DELETING 2 build_insns. The named-local family requires function-scope lifetime to preserve mask insns; block-scope collapses like inline constants.

- [s47] s47 NEW mechanism fact: a merged-single-mask across both check regions inflates the mask pseudo's life across the intermediate check1 copy loop's calls, forcing a callee-saved seat competition (identical pathology to s12's *idx_1495 crossing-window ref). The two independent masks new_var/new_var3 are LIFE-SEPARATED by design; merging them re-lands them in the alias-merge callee-saved seat cascade zone.

- [s47] s47 confirms pp-position-within-do_timeout-block inert (13th novel masked-4 spelling), extends s11/s12 pp-inertia across a new (hoisted-to-block-head) position not previously in the sweep.

- [s47] s47 confirms saved-block-scoped inert (14th novel masked-4 spelling), extends s2's decl-order-vs-first-use finding to declaration-scope.

- [s47] s47 total: 4 novel structural forms measured (2 KILLs at +4/+10, 2 basin-invariant); running cumulative hand-structural coverage: 165+ hand forms, 32+ basin members (now 14 confirmed on-basin masked-4 spellings), consistent with s46 synthesis's exhaustion conclusion.

- [s47] src/system.c restored to HEAD via git checkout after every measurement; working tree clean (except metrics/events.jsonl); oracle untouched.

- [s48] [s48] Baseline reconfirmed on main: candidate.c (vT40) spliced -> sandbox --disable all = masked 4 (178/179 insns, 42 rules dropped, 20 cheat-asm stripped). Floor unchanged since s0.

- [s48] [s48] NEW mechanism fact (check-var split): shortening check's life via block-scope split into check1v/check2v with per-region returns REGRESSES +10 masked (14 / 178). Same magnitude as s47b's cross-region merge regression - confirms check's function-scope life is load-bearing (both shorter AND longer lives regress). The check-region cascade needs check's specific life-range to seat correctly.

- [s48] [s48] NEW mechanism fact (asymmetric copy-loop guard shape): check1's `i=7` INSIDE the `if (a1!=0)` guard is structurally target-required. Hoisting to before the guard drops 1 build_insn (177 vs 178) and adds +1 masked - the setup insn count itself is target-sensitive, and the asymmetry between check1 (guarded i=7) and check2 (hoisted i=7) is deliberate in target's byte pattern.

- [s48] [s48] Both KILLs banked to memory/grind/marionation_Exec/rejected/: s48-check-split-two-blocks-14.c and s48-check1-i7-hoist-loses-target-insn-5.c

- [s48] [s48] candidate.c unchanged - remains vT40 masked 4 (best-known form). src/system.c restored to HEAD after every measurement via splice_apply --restore + git checkout; working tree src/ clean; oracle green.

- [s48] [s48] Cumulative structural coverage after s48: 167+ hand-authored structural variants across s2/s3/s8/s9/s10/s11/s12/s34/s35/s36/s37/s38/s39/s44/s45/s47/s48. Combined with s6/s7/s42/s43 forensics (140-ordering sweep, 9-topology sweep, insn-level sched2/dbr/mem-edge/class-lever) and 12+ permuter basins (~50+ CPU-hr), the hand-derivable + local-sampler space around vT40 remains comprehensively closed at masked 4. Both s48 KILLs extend the check-region axis coverage (previously s47 covered mask holders + pp position + saved scope; s48 adds check-var life AND copy-loop guard shape).

- [s49] vT40 candidate.c reproduces masked 4 on main at session start (score=4, build 178, target 179, 42 rules dropped, 20 cheat-asm stripped) - src restored clean, oracle green.

- [s49] PERM_IGNORE wrapping a mid-function compound statement is invalid in this decomp-permuter (2026 vintage): base compile fails downstream at maspsx because the b64literal pragma stub replaces a syntactic compound-stmt slot with a pragma line and the parser cannot reconstitute the surrounding function's AST. --debug printed base=220 (misleading; the parse-succeed happens in the score path via ast_util special-cased normalize) but the multi-worker campaign path fails at compile.sh's maspsx stage. This closes the naive reading of frontier item #2 as 'add PERM_IGNORE and launch'.

- [s49] vS47c (pp hoisted to do_timeout inner-block head) confirmed as a valid permuter seed: base = 220 with identical residual signature (2 reorderings + 1 deletion) to all 12 prior sampled basins (vT40, find105, z07, w05, s18v02, s30v03, s29v06, s26idxp, s20status, s11u10, s35orcomma, s21v11).

- [s49] s49-fresh-basin campaign metrics: 26min fresh-seed window, ~40k iterations at -j6, 3 sub-220 finds. Time-to-first-find 4:41, time-to-novel-find 12:38, time-to-second-inert 21:50. Yield curve consistent with s31 s30v03 (5 finds/26min) and s41 v11 (1 find/25min) - basin is neither denser nor sparser than portfolio norms.

- [s49] output-195-1 masked-8 semantic-lie construct (new_var2 = *(idx_1496-1) & new_var3 AFTER the clear, return new_var2 instead of `check`) is the FIRST measured attractor at 179/179 insns (target-matching insn count) but pays via a wrong return value: when reached via the check2 path, original returns nonzero check while mutation returns 0 (post-clear read). Bank: memory/grind/marionation_Exec/rejected/s49-new_var2-post-clear-read-semantic-lie-masked8.c

- [s49] output-220-1 reproduces the s48v02 attractor (i=7 hoisted inside the dst2-guard on check2's copy loop, symmetric to s48v02's masked 5 finding on check1). Since vS47c already has check1's i=7 conditional-scoped (as target requires per s48), moving check2's i=7 inside the dst2 guard trades the +1 build_insn saved on check2 for a +1 lost on structural equivalence - net score-equal at 220.

- [s49] output-220-2 = canonicalization/scope-shift score-inert (addu operand swap already proven inert in s2 y03, dummy_label folded pre-sched2 per s7 and s16, src stmt hoist into the do-while(0) wrap has no effect on emitted asm).

- [s49] Portfolio census extends: 13 basins sampled, 0 sub-vT40 novel attractors, closed attractor set {alias-merge, semantic-lie/label-alive, reg-shuffle-inert} holds invariantly.

- [s50] 154 output-*/source.c files exist across 20 permuter campaigns (s4/perm-perm7, s4/perm_b, s4/perm_c, s13/perm_z07, s14/perm_w05, s22/perm_s18v02, s31/perm_s30v03, s32/perm_s26idxp, s40 x3, s41/perm_v11, s49/perm_vS47c); 133 compiled cleanly through the s4 pipeline, 21 hit maspsx/cc1 edge cases.

- [s50] Region-3 fingerprint scanner: identifies the check2 beq $?,$0,.L? insn (second beq following jal sys_GetVblankCount whose fall-through contains sb $0/$zero) and inspects its immediate delay slot. Target's slot is nop (dbr rejected fill via all-live-pseudo path per session-6 dbr analysis); vT40's slot is a stolen move insn. 28/133 finds match target's nop-slot pattern; 105 exhibit vT40-class stolen fill.

- [s50] perm3/output-80-1: masked 2, build_insns 179, target_insns 179, rules_dropped 42. Only find in the entire archive to beat vT40's masked-4 floor. Cheat: `volatile u8 *new_var2` local pointer with two coercion sites, one per prime-directive-forbidden pattern.

- [s50] Removing `volatile` from perm3/80-1: masked reverts to 4, build 178 (vT40-equivalent). Confirms volatile is the sole score-moving mechanism.

- [s50] D_800A147C_2 in src/system.c:498 IS declared `extern volatile u8 * D_800A147C_2 asm("D_800A147C");` - a same-symbol alias-rename to add volatile qualifier, sanctioned as legitimate-volatile-interrupt-touched per prior review because status/vblank handler modifies the underlying byte. But perm3/80-1 additionally propagates volatile-write semantics onto idx_1496 (which points to D_800A1496, a plain counter NOT interrupt-touched) via `new_var2 = idx_1496; *(new_var2-1) = 0` - that second site is unambiguous volatile-coercion, not legitimate-volatile-interrupt-touched.

- [s50] Second-best non-cheat target-matching finds are score=3 at build=180 (perm3/180-1, perm2/160-1, perm_z07/160-1) - all in the vP160 label-alive class already KILLED at s5 as +1-insn semantically-divergent (some emit sb $s0 instead of sb $0, some use `while(status)` in check2 reading status across BBs).

- [s50] Any pure-C region-3 fix in this archive costs >=1 build insn AND lands in a semantically-divergent form; the ONLY 179-insn pure-C spelling reaching masked<4 is the volatile-coercion cheat.

- [s51] vT40 candidate reproduces masked 4 / 178 build vs 179 target on s51 start.

- [s51] cc1 -da dumps produced this session at tmp/grind/marionation_Exec/s51/{vt40,hoist}.{dbr,s} and vt40.{sched2,greg}.

- [s51] vt40.dbr sequence 620 (at insn 620): jump_insn 424 (branch_zero eq a2 0, label_ref 505) + insn 445 (set reg a1 reg s4), 445 has insn_list REG_DEP_ANTI 433.

- [s51] vt40.dbr insn 433 immediately after sequence 620 (through NOTE_INSN_LOOP_BEG/CONT): set (mem:QI (plus s3 -1)) 0 — the byte-clear `*(idx_1496-1)=0`.

- [s51] vt40.dbr summary for marionation_Exec: reorg pass #1 '3 insns needing delay slots, 2 got 0 delays, 1 got 1 delay'; the 1 filled is check2's beqz + move a1,s4.

- [s51] vt40.s lines 1274-1277: `.set noreorder / .set nomacro / beq $6,$0,.L137 / move $5,$20 / .set macro / .set reorder` — the visible fill.

- [s51] hoist.s at check2 region: identical fill signature. Additionally check1's slot now also steals its sb clear (`beq $6,$0,.L121 / sb $0,0($19)` at line 1238) — an orthogonal side-effect of the hoist, unrelated to region-3.

- [s51] inline-a1 form regresses to masked 25 (RA cascade), banked rejected/s51-inline-a1-eliminates-dst2-temp-25.c.

- [s51] hoist form is masked-4-inert, banked rejected/s51-hoist-dst2-at-check2-inert-4.c.

- [s51] GCC reorg.c fill_simple_delay_slots reject predicates that DO fire for perm3/80-1: MEM_VOLATILE_P on the intervening/adjacent mem-store; that predicate is NOT satisfiable at the check2 clear position without a type-coercion cheat because D_800A1495 is ordinary DRAM (not touched by any interrupt handler in this codebase).

- [s52] vT40 baseline floor unchanged: candidate.c yields masked 4 / build 178 / target 179 on this session (splice_apply + sandbox --disable all; s51 evidence intact).

- [s52] v01 (cdrom_ClearIrq() inserted at check2 branch/clear boundary): masked 14 / build 181 / target 179 (+10 masked, +3 build). Emitted at mar_system_s6.s L1273-77.

- [s52] GCC 2.7.2 reorg.c fill_simple_delay_slots dbr pass #1 header on v01: '3 insns needing delay slots; 2 got 0 delays, 1 got 1 delays' — IDENTICAL to vT40 baseline (s51). The fill count did not decrease.

- [s52] check2 branch delay-slot fill on v01 is `jal cdrom_ClearIrq` (the inserted call itself). vT40's fill was `move a1,s4` (=dst2=a1 reg-move insn 445). Either fill is a visible byte vs target's nop.

- [s52] CALL_INSN dep-list at check2 site: REG_DEP_ANTI 133 (do_timeout debug_printf), REG_UNUSED reg:SI 31 ra. No forward-block on the call's own placement. Confirms F1(c)'s memory-clobber prediction misidentifies the direction of the block (clobber restricts OTHER insns crossing the call, not the call crossing).

- [s52] Semantic side effect: the inserted jal in the delay slot executes unconditionally — cdrom_ClearIrq() runs whether check2's flag was set or not. GCC 2.7.2 with no pure/const attribute on the external cdrom_ClearIrq declaration still elects the fill. This confirms F1(c) is not only mechanically wrong but would produce semantically-incorrect execution even if the fill outcome were desirable.

- [s52] Callee-set of marionation_Exec (8 distinct call sites in asm/funcs/marionation_Exec.s): sys_VSync x2 (outer loop head), tslTm2LoadImage_2/debug_printf/cdrom_ClearIrq (do_timeout), sys_GetVblankCount/func_80080828 (polling region), jalr $v0 x2 (D_800A11B4/B8 status2/status4 callbacks inside func_80080828 poll). None fit the check2 branch/clear boundary at C-source altitude.

- [s52] Frontier F1 (call-insertion route) is now fully closed: F1(a) killed s51 (no vblank-touched global available at position), F1(b) killed s51 (dst2 hoist/elimination), F1(c) killed s52 (this session, dbr fills with the call itself; no honest callee has a semantic role at position).

- [s53] s53 baseline reproduced: candidate.c (vT40) spliced -> sandbox --disable all = masked 4 (178/179, 42 rules dropped, 20 cheat-asm stripped). Floor unchanged since s0.

- [s53] s53 NEW MECHANISM FACT: prologue base-pointer hoist for D_800A11DC (`s32 *tbl_A11DC = D_800A11DC;` at function scope) adds +3 build_insns (lui/addiu materialization + callee-saved spill/reload pair) and regresses masked +10 vs vT40 floor. Base-only hoist is distinct from the s9/s27 value-hoist family (which regressed +14 via fresh-temp-launch inside do_timeout); base hoist regresses via register-pressure spill traffic + callee-saved-seat competition instead.

- [s53] s53 STRUCTURAL DIFFERENCE FROM SIBLING: saEft01Init's masked-10 sibling-shape includes the arg3 base at $s3 by way of naturally-available callee-saved capacity (sibling uses only 4 callee-saveds: s0/s1/s2/s3); marionation_Exec's outer state uses 6+ callee-saveds (s0-s5) so the added tbl_A11DC pseudo forces either spill traffic or seat trade with existing longer-lived state. The sibling shape does NOT transfer directly - register-pressure landscape is a hidden gate on the transplant.

- [s53] s53 rederive census: (a) fresh m2c yields only basin member #32 (s35 v04 = masked 4), (b) decomp.me corpus top-similarity 0.094 noise tier (s35 killed), (c) Kengo transplant killed (s36 - PS2 rewrite), (d) saEft01Init sibling-shape value-form killed at masked 10 (s36), (e) 2 other sibling functions (func_8005C6D0 base=$s6, func_80067D14 base=$a2) inspected via asm; func_8005C6D0's pair-window is a two-lw chain (double-pointer indirection) not applicable to marionation_Exec; func_80067D14 is asm-replaced (u8 func_80067D14(s32, s32) - not usable as C template), (f) NEW s53 result: sibling's arg3-base-hoist lever KILLED under marionation_Exec's tighter register pressure at masked 14.

- [s53] s53 CONFIRMS s36's conclusion via a distinct mechanism: sibling saEft01Init's structural advantages (target-order middle-addu, single-base hoisted arg3) exploit the sibling's LIGHTER register landscape and cannot be transplanted verbatim to marionation_Exec without paying the register-pressure cost. This joins s8's twin-transplant KILL (twin cpu_side_move_dir_4 clean shape = masked 16), s35's Kengo KILL, and s35's fresh-m2c basin-equivalence in the rederive-exhaustion evidence bank.

- [s53] src/system.c restored to HEAD via splice_apply.py --restore + git checkout after every measurement; working tree src/ clean; oracle green throughout. candidate.c unchanged (vT40 masked 4 remains best-known).

- [s54] s54 baseline reconfirmed: candidate.c (vT40) spliced -> sandbox --disable all = masked 4 (178/179, 42 rules dropped, 20 cheat-asm stripped). Floor unchanged since s0.

- [s54] s54 NEW mechanism fact: arg3 hoist penalty is a two-part decomposition. Part 1 = fresh-pseudo launch cost (+9, floor for arg3 hoist ANYWHERE in the pair-window scope). Part 2 = pseudo-life-crossing-arg4/arg5-birth cost (+5, activated when arg3val's set precedes ANY arg4/arg5 compute stmt). Late-hoist (after arg5) pays only Part 1 = 13; early or between = 18.

- [s54] s54 NEW mechanism fact: do-while(0) wrap-count (single vs nested) at the arg3 hoist site is INERT (13 both). Consistent with the frontier's mechanism theory being wrong here: doubling loop-note weighting does not overcome the launch pathology's baseline +9. The launch of a new pseudo dominates any refs-reweighting the wrap could provide.

- [s54] s54 confirms the frontier's arg3-only-wrap hypothesis is CLOSED across position (early/between/late) and wrap-count (single/nested) axes. Only untried combination outside these 6 measurements would be a statement-expression form ({ ...; expr; }) inline in the debug_printf arg-list, which requires GNU extension review but per s54 measurements would also carry the +9 launch floor (arg3val pseudo births identically regardless of enclosing-syntax spelling).

- [s54] src/system.c restored to HEAD via git checkout after every measurement; working tree clean; oracle green.

- [s55] s55 baseline reconfirmed: candidate.c (vT40) spliced -> sandbox --disable all = masked 4 (178/179, 42 rules dropped, 20 cheat-asm stripped); src/system.c restored to HEAD after all measurement; oracle green.

- [s55] s55 NEW MECHANISM FACT: GNU statement-expression `({ expr; })` embedded in debug_printf arg-list does NOT birth a named-local pseudo at the enclosing scope. The launch pathology (s54 arg3 late-hoist +9) is bypassed - stmt-expr scores masked 4, identical to vT40 baseline. This CONFIRMS the launch is pseudo-materialization-driven, not syntax-position-driven. s54's stmt-expr prediction (+9 identical to named-local) was WRONG on mechanism.

- [s55] s55 NEW BASIN MEMBER: stmt-expr form is the 15th known distinct masked-4 spelling (joining vT40, s11 u10/w03/w10, s12 v08/w05/w08/z01/z02/z07, s44 postinc-cnt, s45 vA/vC, s47 vS47c/vS47d). Stmt-expr axis is basin-invariant for arg3 specifically because arg3 isn't sched2-tie-decisive (it emits at fixed BB #3 position via debug_printf's arg-flow chain per s6 T-6..T-8 anchor).

- [s55] s55 SYNTHESIS: after 55 sessions the vT40 basin's honest closer space is CLOSED across every axis probed: hand-structural (165+ forms), permuter local-sampler (~50+ CPU-hr across 13 basins), mechanism forensics (sched2 rank/class/LUID, reorg dbr fill/reject predicates, mem-edge/WAR/anti-dep DAG, mips.md function-unit enumeration, jump2 label creation, local-alloc qty_compare, combine_regs tied-web), cross-symbol transfer (sibling saEft01Init s36/s53, twin cpu_side_move_dir_4 s8, Kengo s35, decomp.me corpus s35, fresh m2c s35). Portfolio convergence: every sub-200 permuter find across all 13 basins classifies to the closed 3-family attractor set {alias-merge>=10, label-alive/semantic-lie, reg-shuffle-inert}.

- [s55] s55 STRATEGIC READOUT: two viable macro-pathways survive - (a) the fable-blitz twin-first strategy (develop the exchange lever on cpu_side_move_dir_4's cheaper pair-swap-only residual, then mirror-transfer allocation equations to marionation_Exec, per task-brief pre-merge commits aa8cad24 cd65f9c2 e5aa2dd4 f4bc8e67 df749bfd 622620cb sessions 6f-6h) - OUT OF THIS SESSION'S MANDATE (marionation_Exec only), (b) fresh permuter campaigns on the two truly-unsampled basin members per s14 accounting: s39 arg3-v0-WAR-edge basin (masked 16 base per rejected/s39-arg3-idx-staged-v0-war-edge-order-flips-webgrowth-16.c - not a masked-4 basin member, permuter-seed value unclear) and s31 s30v03 chassis at masked 6-11 (s31 already sampled per evidence - task brief's UNSAMPLED claim may be stale).

- [s55] s55 FRONTIER ITEM VALIDATION: task brief's F3 permuter reference to 's39 arg3-v0-WAR-edge basin at masked 16' is a masked-16 REJECTED form, not a permuter base worth seeding (base score at masked 16 = permuter target 340+, no local basin advantage over vT40's 220-signature seed); its inclusion in frontier is misdirected. The s31 s30v03 chassis IS documented in evidence [s31] as sampled (perm_s30v03 campaign under tmp/grind/marionation_Exec/s31/); task brief's UNSAMPLED claim is inaccurate.

- [s56] [s56] Baseline reconfirmed: candidate.c (vT40) spliced -> sandbox --disable all = masked 4 (178/179, 42 rules dropped, 20 cheat-asm stripped). Floor unchanged since s0.

- [s56] [s56] NEW MECHANISM FACT: decl-scope-shortening for load-derived locals (`saved`, `status`) is qty-INERT at the pair-swap @56/57 AND region-3 @149 residuals. Confirms s2's decl-order-inert finding extends to lifetime-scope for value-carrying (non-constant-holder) locals. Combine folds only trigger for constant-holder locals (s47: new_var/new_var3) whose value is compile-time-known - a run-time-computed value (saved = *D_800A147C_2 & 3; status = func_80080828()) has no fold pathway even at block-scope.

- [s56] [s56] NEW BASIN MEMBERS: v01 (saved block-scope), v02 (status block-scope), v03 (both) - 16th/17th/18th known distinct masked-4 spellings of vT40's floor basin, extending the catalog begun with vT40, s11 u10/w03/w10, s12 v08/w05/w08/z01/z02/z07, s44 postinc-cnt, s45 vA/vC, s47 vS47c/vS47d, s55 stmt-expr. Scope-shortening axis is basin-invariant.

- [s56] [s56] Cumulative hand-authored structural coverage after s56: 168+ hand forms across 21 sessions probing decompositions, orderings, types, staging webs, mirror forms, prologue reorders, mask holders, check-var life, copy-loop guard shapes, arg3 hoisting/wrapping, stmt-expr, AND now decl-scope lifetime. Combined with 13+ permuter basins (~50+ CPU-hr), 140-ordering sweep, 9-topology sweep, insn-level sched2/dbr/mem-edge/mips.md/local-alloc forensics, the hand-derivable + local-sampler basin around vT40 remains comprehensively closed at masked 4.

- [s56] [s56] 3 disproven forms banked to memory/grind/marionation_Exec/rejected/: s56-saved-block-scope-basin-equiv-4.c, s56-status-block-scope-basin-equiv-4.c, s56-saved-status-both-block-scope-basin-equiv-4.c. candidate.c unchanged (vT40 masked 4 remains best-known).

- [s56] [s56] src/system.c restored to HEAD via splice_apply.py --restore + git checkout after every measurement; working tree src/ clean; oracle green throughout.

- [s57] vT40 candidate.c baseline confirmed masked 4 / build=178 / target=179 / 42 rules dropped at s57 start (measured via s6/splice_apply.py + sandbox --disable all)

- [s57] s57 basin catalog additions: idx_1496 = idx_1495 + 1 (v05) and cnt split-init (v07) both hit masked 4 — 19th and 20th distinct spellings of the vT40 basin (ledger tracked 18 known through s56). Basin exhaustion argument continues to strengthen.

- [s57] Copy-block scope-tightening for src/dst pointer locals regresses +7 masked (v06=11 vs vT40=4). Same regression class as s16 dead-local-saved-debug-inert / s21 fresh-i-per-copy-block — RA relandscaping around the copy loops scrambles seats.

- [s57] GCC canonicalizes `1 + idx_1494` and `idx_1494 + 2` chain-vs-fork derivations to identical addiu emission — the addr-arithmetic dep chain is codegen-invariant to the split source form (v05 result).

- [s57] sanctioned split-init accumulation on cnt is loop-body-local so its extra global-store folds without cross-BB effect on do_timeout expand context (v07 result). Confirms the split-init lever is scope-local when applied to loop-body variables.


## [s58] Disposition session (escalation modality) - chassis re-measured, last frontier item killed, both endgame gates evaluated

- **Chassis re-measure (this session, on HEAD).** `memory/grind/CD_ready/candidate.c` (vT40) spliced
  into `src/system.c` in place of `INCLUDE_ASM("asm/funcs", CD_ready);` with the naming-wave symbol
  map applied -> `score=4, build_insns=178, target_insns=179, rules_dropped=0`. The floor of 4 is
  live, not a stale ledger number. Harness plus log: `tmp/grind/CD_ready/s58/splice.py`,
  `tmp/grind/CD_ready/s58/measurements.txt`.
- **Naming map for any future session** (the ledger and `candidate.c` predate the naming wave):
  `marionation_Exec`->`CD_ready`, `sys_VSync`->`VSync`, `tslTm2LoadImage_2`->`puts`,
  `debug_printf`->`printf`, `cdrom_ClearIrq`->`CD_flush`, `sys_GetVblankCount`->`CheckCallback`,
  `func_80080828`->`getintr`. All the externs the body needs are already present in `src/system.c`
  at lines 353-375; nothing has to be added to splice the candidate.
- **F1 family formally exhausted.** `CheckCallback()` at the check2 fill site measures masked 14 /
  build 181 - identical to s52 `cdrom_ClearIrq`. `reorg.c fill_simple_delay_slots` consumes any
  inserted CALL_INSN as the check2-beqz fill regardless of callee, so no honest call can create the
  fill the region-3 steal needs. Banked
  `rejected/s58-checkcallback-call-at-check2-fill-site-14.c`. Combined with the s51 kill of the
  volatile-load shape, the F1 "honest fill" frontier item is closed; there is no third shape.
- **Remaining frontier items are both dead ends, not untried levers.**
  (1) *Permuter on an unsampled masked-4 chassis*: the ledger already records 13 permuter basins /
  ~50+ CPU-hr / ~10k+ samples across 6 chassis with **0 novel basin closures** (s4/s5/s40/s41/s49/s50);
  s50 sampled 154 finds and the only sub-vT40 result (`perm3/output-80-1`, masked 2) reached it *via
  volatile coercion*, which the prime directive forbids. This is a re-run of a measured-negative
  axis, not a new lever.
  (2) *Cross-function exchange lever developed on the twin `CD_sync`*: **the twin is gone as a bench** -
  `CD_sync` was itself dispositioned REFUSED / OWNER-ACCEPTED INCOMPLETE on 2026-08-25
  (`docs/grind/decisions.md:11814`) after 105 sessions and >=6 modalities, with both endgame gates
  failing and its own residual proven to be a `global.c` allocno birth-tie with no C spelling. The
  cheaper development bench does not exist.
- **Gate (a) canonical-asm - FAILS.** `python3 tools/scan_hand_coded.py --single CD_ready` ->
  `tier=LOW score=2/8 (179 insns)`; only S4 (4 loads in an 8-insn window @ insn 51) and S5
  (1 approx-sibling `CD_sync`, jaccard 0.64) fire; **S1 / S2 / S6 all absent**, and S1/S2/S6 are the
  only signals that can carry a STRONG tier. Artifact `tmp/grind/CD_ready/s58/scan_hand_coded.txt`.
  Independently, this function is a confirmed PsyQ libcd `CD_ready` (`memory/closer/libcd-identity.md`),
  i.e. **compiled Sony C**, not hand-written asm - the canonical-asm claim is false on provenance too.
- **Gate (b) SOTN precedent for the CLOSING construct - FAILS by construction.** The residual is two
  root-caused items with no C-level spelling at all: (i) the pair-swap, a `sched.c` sched2 T-14 tie
  between two equal-`INSN_PRIORITY` `ashlsi3` insns broken by LUID (source emission order), where
  every reordering that flips them pays the `qty_compare` 5.33-vs-5.33 seat trade; and (ii) the
  region-3 `reorg.c` delay-slot steal, where every honest fill route is now measured dead (s51/s52/s58).
  There is no construct to find a precedent FOR. (The constructs that HOLD the floor at 4 - the
  do-while(0) wraps, the staged-value reuse, the constant holders, the pointer alias - are each already
  inside a sanctioned family and FAKE-annotated in `candidate.c`; they are not the blocker.)
- **Nothing is bytes-proven and nothing is blocked by an untouchable surface.** `src/system.c` carries
  `INCLUDE_ASM("asm/funcs", CD_ready);` with **0 regfix/asmfix rules** (`rules_dropped: 0` in every
  sandbox run this session). This is NOT an integration handoff - there is no byte-match being held
  back by `regfix.txt`/`asmfix.txt`/`prologue_config.json`; the best honest form is simply 4 away.
- **Exhaustion tally at disposition:** 58 sessions; >=6 distinct modalities (structural s2/s3/s8-s12/
  s45-s49/s56/s57, permuter s4/s5/s40/s41/s49/s50, forensics s6/s7/s15/s16/s24/s25/s51/s52,
  rederive s53/s54, synthesis s55, escalation s58); 165+ hand-written forms; **115 banked rejected
  forms plus this session 116th**; 20 distinct spellings all converging on the same masked-4/178
  attractor; ~50+ CPU-hr of permuter across 13 basins with zero novel closures.

- [s58] Chassis re-measured first-hand this session (the driver reported 'measurement unavailable' and the chassis had changed twice since the ledger): vT40 spliced under the current names gives score=4, build_insns=178, target_insns=179, rules_dropped=0 - the ledger's floor of 4 is live.

- [s58] NAMING MAP for future sessions (the ledger and candidate.c predate the naming wave): marionation_Exec->CD_ready, sys_VSync->VSync, tslTm2LoadImage_2->puts, debug_printf->printf, cdrom_ClearIrq->CD_flush, sys_GetVblankCount->CheckCallback, func_80080828->getintr. All required externs already exist at src/system.c:353-375. Harness: tmp/grind/CD_ready/s58/splice.py. Recorded in candidate.c's header and evidence.md [s58].

- [s58] F1 'honest delay-slot fill at the region-3 steal site' family is now FORMALLY EXHAUSTED: the volatile-load member died in s51 (semantic lie), the call member died twice with identical numbers - cdrom_ClearIrq in s52 and CheckCallback in s58, both masked 14 / build 181. reorg.c fill_simple_delay_slots takes any inserted CALL_INSN as the fill; the callee's identity is irrelevant.

- [s58] Frontier item 'permuter on an unsampled masked-4 chassis' is a measured-negative re-run, not an untried lever: 13 basins, ~50+ CPU-hr, ~10k+ samples across 6 chassis, 0 novel basin closures (s4/s5/s40/s41/s49/s50); s50's only sub-vT40 find (perm3/output-80-1, masked 2) got there via volatile coercion, forbidden by the prime directive.

- [s58] Frontier item 'develop the exchange lever on the cheaper twin CD_sync, then mirror' is dead because the bench no longer exists: CD_sync was itself dispositioned REFUSED / OWNER-ACCEPTED INCOMPLETE on 2026-08-25 (docs/grind/decisions.md:11814) after 105 sessions and >=6 modalities, both gates failing, its residual proven a global.c allocno birth-tie with no C spelling.

- [s58] Gate (a) canonical-asm FAILS: scan_hand_coded --single CD_ready -> tier=LOW score=2/8 (179 insns), S4+S5 only, S1/S2/S6 absent. Also false on provenance (identified PsyQ libcd CD_ready = compiled Sony C).

- [s58] Gate (b) SOTN precedent FAILS by construction: the residual is a sched2 LUID tie plus a reorg.c delay-slot steal, neither of which has any C-level spelling, so there is no closing construct to cite.

- [s58] This is NOT an integration handoff: src/system.c carries INCLUDE_ASM("asm/funcs", CD_ready); with 0 regfix/asmfix rules (rules_dropped: 0 in every sandbox run this session). Nothing is bytes-proven and nothing is blocked by a surface this session may not touch - the best honest form is simply 4 away.

- [s58] No decision packet filed, per the owner's 2026-08-24 auto-reject class: the only questions left here ('sanction a new family for the LUID tie', 'grant canonical-asm despite a LOW tier') are standard-lowering and PRE-DECIDED NO. Provenance and routing are both already settled.

- [s58] Exhaustion at disposition: 58 sessions, >=6 distinct modalities (structural, permuter, forensics, rederive, synthesis, escalation), 165+ hand-written forms, 116 banked rejected forms, 20 distinct spellings all converging on the same masked-4 / 178-insn attractor.

- [s58] Disposition entry FILED THIS SESSION at docs/grind/decisions.md:12356 - '2026-08-25 - CD_ready (src/system.c) - OWNER-ESCALATION - RESOLVED BY STANDING RULING (2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE'. src/system.c was restored to HEAD (git clean) after measurement.

- [campaign-sweep 2026-08-30] FIRST actual solver-chain run on this function
  (sched-tie endgame campaign, docs/superpowers/specs/2026-08-30-sched-tie-endgame-campaign.md;
  no sched_solver/ra_solver artifact existed in this ledger before today).
  Executes the solver-modality directive from the 2026-08-30 ruling-10
  return-to-active. Artifacts: tmp/grind/CD_ready/s59_campaign/.
  **TWIN CONFIRMATION:** CD_ready's divergent sched block is byte-for-byte the
  same WINDOW and the same TWO-INTENT structure as CD_sync's (banked same day in
  memory/grind/CD_sync/evidence.md) — the debug_printf argument block, 20 insns,
  39 depth-1 vectors: (i) source-order flip (addu-statement uid120
  `addu $2,$2,$21` ahead of sll uid106 `sll $4,$4,2`) and (ii) `cost 120 := 2`
  (C-unreachable; the insn is an addu in target's own bytes). Even the register
  substitution pattern matches the twin exactly (6x $v1->$a0 + 2x $a0->$v1).
  A lever found on either twin transfers.
  **Sched half (vT40 candidate, sandbox 4, 178/179):** perturb.py depth-1
  exhaustive (1034 atoms): only pass2 block 3 diverges in-model; block 23
  (5 insns, the region-3 reorg steal area) is OUTSIDE the sched model (post-
  sched2, reorg.c) and was skipped — consistent with s51/s58's F1-family
  exhaustion being the right tool for that half.
  **RA half (vT32 pair-order chassis, sandbox 8):** the exchange is
  GLOBAL-vs-LOCAL here: p87 (idx chain, global, $v1) <-> p97 (arg5 value,
  local qty 2, $a0). inverse.py local --block 3 --goal qty2->$v1 --depth 2:
  25 single-atom vectors, minimal size 1. Dominant family: DEMOTE qty 1
  = pseudo 104 (birth 18 death 24 refs 4, currently $v1 — the L6=L6 birth-tie
  competitor named in the twin's prose): refs_down 4->3/2, live_extend
  die-later 24->25/26 or born-earlier 18->17 (span 7 loses the tie).
  Printed caveat applies: local birth/span vectors are NECESSARY not
  SUFFICIENT (alloc-time vs emission order, camera_set_zoom 2026-08-05) —
  re-derive from a QTYDBG dump before believing any single spelling.
  **Candidate-unprobed axis for the next session:** a byte-neutral one-insn
  extension or ref-demotion of p104's variable specifically (the bank's wrap /
  fresh-temp / mask-fold families all reweighted MULTIPLE qtys at once; the
  single-qty p104 demotion was never isolated). Map p104 to its C variable
  first (QTYDBG blk=3), then check the rejected/ bank before spelling.

- [s59] Floor re-confirmed live on the current HEAD chassis a third time: vT40 (memory/grind/CD_ready/candidate.c) spliced under the current names gives score=4, build_insns=178, target_insns=179, rules_dropped=0. src/system.c restored to HEAD (git clean) after all measurement.

- [s59] The 2026-08-30 solver campaign's ONE candidate-unprobed axis is now measured and KILLED. That axis was: "a byte-neutral one-insn extension or ref-demotion of p104's variable specifically (the bank's wrap / fresh-temp / mask-fold families all reweighted MULTIPLE qtys at once; the single-qty p104 demotion was never isolated)". p104 is the `t0` address web in the do_timeout printf-argument block; its C identity did not need a fresh QTYDBG dump because the ledger already carries the exact QTYDBG reading at evidence.md:31-36 (addr-temp qty 102 {r4 l4}=8.0 -> $v0 correct; t0-web qty 104 {r4 l6}=5.33; arg5val qty 97 {r4 l6}=5.33 exact tie broken by birth order).

- [s59] 10 variants measured (tmp/grind/CD_ready/s59/sweep1.txt + sweep2.txt). vT40 chassis: base 4 (control), t0-refs-3 mul-in-add 5, t0-refs-3 mul-in-load 11, t0-refs-2 fully folded 11. vT32 (order-correct/arg5-first) chassis: base 9, t0-load hoisted 8, t0-refs-3 mul-in-add 9, t0-refs-3 mul-in-load 11, t0-refs-2 folded 11, t0-refs-2 folded pp-first 11, load-hoisted+refs-3 8. **Nothing below 4; the vT32 chassis floor stays at its historical 8.** All builds 178 insns, so this is pure seat/order relandscaping, not an insn-count effect. 9 of the 10 banked to memory/grind/CD_ready/rejected/ (s59-*).

- [s59] MECHANISM (new, transferable to the twin CD_sync and to any function whose residual is a local-alloc qty_compare tie): demoting the tie loser by FOLDING its arithmetic is self-defeating here, because the fold also collapses the neighbouring addr-temp qty 102 — the tight {r4 l4}=8.0 temp that currently seats $v0 CORRECTLY — so the whole three-qty cascade re-seats and costs more than the tie it wins. The mul-in-load spelling is uniformly worst (11 on BOTH chassis), which pins the cause: the load and the multiply must stay in separate statements for qty 102 to stay tight. This is a tenth confirmation of the ra_solver local-mode printed caveat (camera_set_zoom 2026-08-05): single-qty demotion vectors are NECESSARY but not SUFFICIENT, because the model scores one qty in isolation while local-alloc re-runs the whole block.

- [s59] Gate (a) re-measured this session: `tools/scan_hand_coded.py --single CD_ready` -> tier=LOW score=2/8 (179 insns), only S4 + S5 fire, S1/S2/S6 absent. Artifact tmp/grind/CD_ready/s59/scan_hand_coded.txt. FAILS, identically to s58 — and independently false on provenance (identified PsyQ 3.5 libcd CD_ready = compiled Sony C).

- [s59] Gate (b) FAILS by construction, unchanged from s58: the residual is a sched.c sched2 T-14 LUID tie plus a reorg.c fill_simple_delay_slots steal. Neither has any C-level spelling, so there is no closing CONSTRUCT for which a SOTN-master precedent could be cited. The constructs that HOLD the floor at 4 (FAKE-annotated do-while(0) wraps, staged-value reuse, constant holders, pointer alias) are all already inside sanctioned families and are not the blocker.

- [s59] Exhaustion at disposition: 59 sessions, >=7 distinct modalities (structural, permuter, forensics, rederive, synthesis, escalation, solver), 175+ hand-written forms, 125 banked rejected forms, 20 distinct spellings converging on the same masked-4/178 attractor, ~50+ CPU-hr of permuter across 13 basins with zero novel closures, plus the full 2026-08-30 solver-chain run (perturb.py depth-1 exhaustive over 1034 atoms; inverse.py local depth-2 over 118 atoms) whose only unprobed vector is the one killed above.

- [s59] Disposition entry FILED THIS SESSION in docs/grind/decisions.md ('2026-08-30 - CD_ready (src/system.c) - OWNER-ESCALATION - RESOLVED BY STANDING RULING (2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE'), superseding the 2026-08-25 entry at decisions.md:12356 by re-measuring both gates AND closing the solver axis that the owner's 2026-08-30 ruling-10 return-to-active was issued for. No decision packet filed: per the owner's 2026-08-24 auto-reject class the only remaining questions here ('sanction a new family for a LUID tie with no C spelling', 'grant canonical-asm despite a LOW scan tier') are standard-lowering and PRE-DECIDED NO; provenance and routing are both already settled.

- [s59] NEXT-SESSION NOTE if this function is ever re-activated: do NOT re-run the vT32/vT40 chassis sweep, the t0-web refs/fold axis, the F1 delay-slot-fill family, the 140-ordering sweep, the 9-variant topology sweep, the four qty_compare axes, or another permuter basin — all closed negative with measurements. The ONLY thing that would re-open this function is a toolchain-model advance that can search the JOINT sched2-LUID x local-alloc-birth space (the two residuals are a coupled fixed point: every C change that fixes the order breaks the seats and vice versa, proven across 59 sessions).

- [s59] Floor re-confirmed live on the current HEAD chassis a third time: vT40 (memory/grind/CD_ready/candidate.c) spliced under the current names gives score=4, build_insns=178, target_insns=179, rules_dropped=0. src/system.c restored to HEAD (git clean) after all measurement.

- [s59] The owner's 2026-08-30 ruling-10 return-to-active has now been EXECUTED IN FULL: the 2026-08-30 sched-tie endgame campaign ran the solver chain on this function for the first time and left exactly one candidate-unprobed axis (single-qty p104 demotion); this session spelled it in C and measured it dead. Nothing the return-to-active was issued to test remains untested.

- [s59] p104 was identified from the ledger's existing QTYDBG reading rather than a fresh dump: evidence.md:31-36 already names it as the `t0` address web in the do_timeout printf-argument block, with the exact three-qty priority arithmetic (102 = 8.0 correct on $v0; 104 = 5.33; 97 = 5.33 tie broken by birth order).

- [s59] 10 variants measured (tmp/grind/CD_ready/s59/sweep1.txt, sweep2.txt). vT40 chassis: base 4, t0-refs-3 mul-in-add 5, t0-refs-3 mul-in-load 11, t0-refs-2 folded 11. vT32 (order-correct) chassis: base 9, t0-load hoisted 8, refs-3 mul-in-add 9, refs-3 mul-in-load 11, refs-2 folded 11, refs-2 folded pp-first 11, load-hoisted+refs-3 8. Nothing below 4; the vT32 chassis stayed at its historical floor of 8. All builds 178 insns.

- [s59] NEW TRANSFERABLE MECHANISM (applies to the twin CD_sync and to any function whose residual is a local-alloc qty_compare tie): demoting the tie loser by FOLDING its arithmetic is self-defeating when a neighbouring tight addr-temp depends on that arithmetic's statement structure - the fold demotes the neighbour too and the whole block cascade re-seats. Here the mul-in-load spelling is uniformly worst (11 on BOTH chassis), which pins the cause: the load and the multiply must stay in separate statements for qty 102 to stay tight at {r4 l4}=8.0.

- [s59] 9 of the 10 forms banked to memory/grind/CD_ready/rejected/ (s59-*); the rejected bank is now 125 forms.

- [s59] Gate (a) re-measured this session: tools/scan_hand_coded.py --single CD_ready -> tier=LOW score=2/8, only S4+S5 fire, S1/S2/S6 absent. FAILS. Artifact tmp/grind/CD_ready/s59/scan_hand_coded.txt.

- [s59] Gate (b) FAILS by construction: the residual is a sched2 LUID tie plus a reorg.c delay-slot steal, neither of which has a C-level spelling, so there is no closing construct for which a SOTN-master precedent could be cited.

- [s59] No decision packet filed, per the owner's 2026-08-24 auto-reject class: the only questions this residual could pose ('sanction a new family for a LUID tie with no C spelling', 'grant canonical-asm despite a LOW scan tier') are standard-lowering and PRE-DECIDED NO. Provenance (PsyQ libcd CD_ready) and routing (C, not canonical-asm) are both already settled, so no fidelity/routing/provenance question remains open.

- [s59] NOT an integration handoff: src/system.c carries INCLUDE_ASM("asm/funcs", CD_ready); with 0 regfix/asmfix rules (rules_dropped: 0 in every sandbox run this session). Nothing is bytes-proven and nothing is blocked by a surface this session may not touch - the best honest form is simply 4 masked points away.

- [s59] Exhaustion at disposition: 59 sessions; >=7 distinct modalities (structural, permuter, forensics, rederive, synthesis, escalation, solver); 175+ hand-written forms; 125 banked rejected forms; 20 distinct spellings converging on the same masked-4/178 attractor; ~50+ CPU-hr of permuter across 13 basins with zero novel closures; plus the full 2026-08-30 solver-chain run (perturb.py depth-1 exhaustive over 1034 atoms; inverse.py local depth-2 over 118 atoms).

- [s59] Disposition entry FILED THIS SESSION at docs/grind/decisions.md:15629 - '2026-08-30 - CD_ready (src/system.c) - OWNER-ESCALATION - RESOLVED BY STANDING RULING (2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE'. It supersedes the 2026-08-25 entry at decisions.md:12356 by re-measuring both gates AND closing the solver axis the ruling-10 return-to-active was issued for.

## [s60] 2026-09-01 — FLOOR 4 -> 2. Owner Ruling-A directive executed; the banked vAT1 form re-scores at 2 post-`-mel`

Modality dispatched: `escalation` (disposition session). It did NOT end in a disposition,
because the owner's named probe worked: **the honest floor dropped from 4 to 2** and the
exhaustion counter resets. Nothing was foreclosed and no decisions.md entry was filed.

**The directive.** `docs/grind/decisions.md:17795`, 2026-09-01 FORECLOSED-BUCKET REVIEW,
Ruling A row `CD_ready (d4)`: *"Ruling D session; re-score the banked vAT1 form
post-`-mel`"*, on the reopen ground that the s50 masked-2/179 form's volatile
disqualification rested on a false premise. Both halves were addressed.

### Half 1 — Ruling D (CD_intr aggregate merge): DEAD, and deliberately not re-measured here

Grind session 107 on the twin `CD_sync` executed Ruling D in full on 2026-09-01 and recorded
the result at `docs/grind/decisions.md:18287-18400`. Its prong-(c) finding is **symbol-level,
not function-level**, and its own record says so verbatim: *"The prong-(c) finding is
symbol-level, not function-level: it holds identically for `CD_ready` and `CD_datasync`."*
The storage 0x800A1494/95/96 is DEFINED IN ASSEMBLY (`asm/data/7D920.data.s:31048-31076`,
plus `dlabel D_800A1498` whose first word is `.word D_800A1494`) and is referenced by name
from eight assembly files (`CD_cw.s` 6 sites, `getintr.s` 5, `func_800819C4.s` 5,
`func_800817A0.s` 4, `func_80081E1C.s` 1, plus the three INCLUDE_ASM bodies). The per-word
symbols must therefore survive in the splat symbol config, so any `CD_intr` declaration is
necessarily a SECOND handle on the same storage — exactly the prong the g_stage_id merge died
on (decisions.md:10722). CD_sync additionally MEASURED all five prong-conformant spellings at
18-34 against a floor of 2, with volatile strictly WORSE than plain. Re-running that battery
here would burn a session re-deriving a banked negative; this session spent the budget on
half 2, which is what moved the floor. The Ruling D re-activation trigger is unchanged: the
asm-only consumers (CD_cw, getintr, func_800819C4, func_800817A0, func_80081E1C) must reach C.

### Half 2 — the vAT1 re-score: THE FLOOR MOVED

Recovered with `git show 043e4b80^:memory/closer/candidates/marionation_vAT1_notailwrap.c`
(Closer phase 2, 2026-07-09) into `tmp/grind/CD_ready/s60/vAT1.c`, spliced over
`INCLUDE_ASM("asm/funcs", CD_ready);` at src/system.c:379 with
`tmp/grind/CD_ready/s60/splice.py` (the s59 harness, re-pathed).

| # | body | score | build_insns | target_insns | rules |
|---|---|---|---|---|---|
| baseline | the 59-session `candidate.c` (vT40) | **4** | 178 | 179 | 0 |
| vAT1 | the recovered Closer form, verbatim | **2** | **179** | 179 | 0 |
| v01 | vAT1 with `volatile` removed from `idx_1496` and NOTHING else | 4 | 178 | 179 | 0 |
| v02 | vAT1 + `extern volatile u8 g_cd_status_c;`, `idx_1496 = &g_cd_status_c;` | 4 | 180 | 179 | 0 |
| v03 | vAT1 + all three Intr pointers (1494/1495/1496) volatile | 8 | 179 | 179 | 0 |
| v04 | v03 with the base taken as `&g_cd_status_a` (cdrom_IrqHandler's exact shape) | 8 | 179 | 179 | 0 |

The chassis is current: the baseline row re-measures the ledger's floor 4 live this session, so
every number above is chassis-valid. `candidate.c` (rewritten to the vAT1 body with the full
header) was re-spliced and re-scored at 2/179/179/0 AFTER being written. `src/system.c` was
restored to `INCLUDE_ASM` and the tree verified clean at session end.

**What vAT1 buys over vT40.** Two independent things, and only the second is contested:
1. *Structural, uncontested.* Each check arm is self-contained with its own `return check;`
   (the inlined-`_memcpy` shape: the `a1 == 0` guard lives inside the inlined copy and falls
   through to the arm's own return). GCC's cross-jumping then forward-redirects arm 1's guard
   into arm 2's identical tail, which is the target's byte layout. This FIXES BOTH unmasked
   branch-destination diffs vT40 never fixed, with no done-label gymnastics, and it drops the
   tail `do { } while (0)` wrap entirely (measured inert). Ordinary C.
2. *The volatile, and it is LOAD-BEARING — v01 is the controlled measurement.* Removing the
   qualifier and changing nothing else costs exactly one instruction (179 -> 178) and two
   score points (2 -> 4): GCC hoists the `*idx_1496` flag load out of the `goto loop` poll.
   The target contains a load GCC emits only when that object is volatile.

### The volatile: prong 1 of legitimate-volatile-interrupt-touched is SATISFIED WITH CITATIONS

The rule demands a NAMED writer with function + file:line, not an assertion. Banked:
- **The byte is written by `getintr`** — `asm/funcs/getintr.s:304` `sb $v0, %lo(D_800A1496)($at)`
  (and :244 writes D_800A1494, :274 / :309 write D_800A1495). `asm/funcs/func_800819C4.s:71`
  writes D_800A1496 as well.
- **`getintr()` is called from the installed IRQ handler.** `cdrom_IrqHandler` (src/system.c:770)
  calls `getintr()` in its service loop; its entry carries `glabel D_80081F1C` (the file-scope
  `__asm__` block at src/system.c:758-768); and `InterruptCallback(2, &D_80081F1C);` installs it
  as the IRQ-2 (CD-ROM) callback at **src/system.c:609 and :630**. That is the rule's
  "installed via InterruptCallback ... or a function called from such an installed handler"
  clause, satisfied by citation at every link.

**In-TU precedent that the project ALREADY ships these bytes as volatile in matched code:**
src/system.c:549-551 and :748-749 declare `extern volatile u8 g_cd_status_a/b/c`
(= 0x800A1494/95/96 per named_syms.txt:68-70, symbol_addrs.txt:85-87) inside byte-matching
committed functions, and src/system.c:770-771 — inside matched `cdrom_IrqHandler` — writes
`volatile u8 *s1 = &g_cd_status_b; volatile u8 *s3 = s1 - 1;`, i.e. a matched function derives
a volatile byte pointer BY POINTER ARITHMETIC, structurally the same shape vAT1 uses
(`idx_1496 = idx_1494 + 2`). The owner's reopen ground for this function was exactly this
correction, and it is confirmed first-hand.

### The volatile: prong 2 is OPEN, and it is the gate on any future candidate-ready

The rule's prong-2 list is EXACT, not illustrative: spin-wait / double-read-across-sequence-
point / IRQ-mutated-loop-bound. This use-site is the `loop:` ... `tail: if (a0 == 0) goto loop;`
poll, whose body calls `VSync(-1)` and `getintr()` between successive reads of `*idx_1496`.
That is closest to **double-read-across-sequence-point** (`x = G; foo(); y = G;` where `foo()`
services the IRQ), and the v01 measurement is direct evidence of exactly the harm the prong
describes — GCC does in fact merge the reads when the qualifier is absent. But it is not
VERBATIM any of the three listed shapes: the loop is not body-free, so it is not the listed
spin-wait. Also, D_800A1494/95/96 carry **no entry in `volatile_extern_allowlist.txt`** (the
file exists; the nearest entries are D_800A14D0 / D_800A14D4, the granted cdread.c CdlREAD
block). **Consequence for the next session: do NOT take this body to candidate-ready on the
strength of the score alone. Prong 2 + the allowlist grant is a `ruling-request`, and it should
be asked with the prong-1 citations and the v01 controlled measurement in hand — that is a far
stronger packet than anything this ledger has previously put to a ruling.**

### The residual at 2 is a DIFFERENT residual — the old frontier does not describe it

Disassembly comparison (`tmp/grind/CD_ready/s60/cmp2.py` against `asm/funcs/CD_ready.s`) puts
both remaining differing instructions in the `do_timeout` printf-argument block, build insns
~51-67: the build carries the staged table index in `$v1` where the target uses `$a0`, and the
`sll ,2` / `lw` pair around insns 57-58 is transposed against the target.

The 59-session frontier — "a coupled fixed point: vT40 (t0 first) gets the seats right and the
order wrong (4); vT32 (arg5 first) gets the order right and the seats wrong (8); every
single-axis search fixes one and breaks the other" — was formulated against a base that was
178 instructions long with two wrong branch destinations. **vAT1 is 179/179 with correct branch
destinations already**, so the instruction-count and branch legs of that trade are already
paid; the seat/order tie is now the only thing left, from a strictly better starting point.
Everything killed in s53-s59 (the t0-address-web refs/fold axis, arg3 wrap position/count,
stmt-expr arg3, decl-scope shortening, chain-idx1496, cnt split-init, the single-qty demotion
of pseudo 104) was killed AGAINST THE vT40 MASKED-4 BASE. Those kills do not transfer: they
measured "does this move 4", not "does this move 2", and the register assignment in the block
differs ($v1 vs $a0). **They are all re-openable against vAT1 and should be re-run there before
anything exotic is attempted.** That, and not a disposition, is this function's frontier now.

### Process lesson worth carrying to other foreclosed items

The 2026-09-01 review's systemic finding 1 is confirmed on this function in the strongest
possible form: a form banked at masked 2 sat unspent in `memory/closer/candidates/` (a
directory outside `memory/grind/CD_ready/`, deleted from the tree by commit 043e4b80) while
59 sessions ground a masked-4 base and three separate records declared the function
exhausted. The cross-knowledge sweep in the session brief pointed at `memory/closer/*` and
that is where the answer was. **When a ledger reports a long flat floor, recover and re-score
every banked form from every sibling campaign directory BEFORE spending a session on new
spellings — including forms deleted from HEAD, via `git show <commit>^:<path>`.**

- [s60] FLOOR DROPPED 4 -> 2 on a live-measured chassis. Baseline (the 59-session vT40 candidate.c) re-measured THIS session at score 4 / build 178 / target 179 / rules 0; the new candidate.c (vAT1 body) at score 2 / build 179 / target 179 / rules 0, re-verified after the file was rewritten. This resets the exhaustion counter; no decisions.md entry was filed and nothing was foreclosed.

- [s60] The owner's 2026-09-01 FORECLOSED-BUCKET REVIEW Ruling A row for CD_ready (decisions.md:17795) named two things. Half 2 ('re-score the banked vAT1 form post--mel') is the one that worked. Half 1 ('Ruling D session') is dead for this function by CD_sync s107's symbol-level prong-(c) finding, which that record itself extends to CD_ready and CD_datasync by name.

- [s60] The recovered form was NOT in memory/grind/CD_ready/. It lived at memory/closer/candidates/marionation_vAT1_notailwrap.c, a path deleted from HEAD by commit 043e4b80, and was recovered with `git show 043e4b80^:<path>`. Three separate records (2026-08-25, 2026-08-30, and the s58/s59 sessions) declared this function exhausted while a banked masked-2 form sat one git-show away. This is the 2026-09-01 review's systemic finding 1 confirmed in its strongest form.

- [s60] The volatile qualifier on idx_1496 is load-bearing by controlled measurement, not by argument: s60 v01 removes it and changes nothing else, and build_insns falls 179 -> 178 with the score rising 2 -> 4, because GCC hoists the flag load out of the poll loop.

- [s60] Prong 1 of legitimate-volatile-interrupt-touched is satisfied with citations at every link: getintr writes D_800A1496 (asm/funcs/getintr.s:304); getintr() is called from cdrom_IrqHandler (src/system.c:770); cdrom_IrqHandler's entry is glabel D_80081F1C (src/system.c:758-768) and is installed as the IRQ-2 CD-ROM callback by InterruptCallback(2, &D_80081F1C) at src/system.c:609 and :630.

- [s60] The project ALREADY ships these bytes as volatile in matched, committed, byte-correct code in the same TU: `extern volatile u8 g_cd_status_a/b/c` at src/system.c:549-551 and :748-749 (= 0x800A1494/95/96 per named_syms.txt:68-70 and symbol_addrs.txt:85-87), and matched cdrom_IrqHandler derives a volatile byte pointer by pointer arithmetic at src/system.c:770-771 - structurally the same shape vAT1 uses.

- [s60] PRONG 2 IS OPEN AND IS THE GATE ON COMPLETION, NOT THE SCORE. The rule's shape list is exact (spin-wait / double-read-across-sequence-point / IRQ-mutated-loop-bound); this use-site is a poll loop whose body calls VSync(-1) and getintr() between successive reads of *idx_1496 - closest to double-read-across-sequence-point but not verbatim any of the three, since the loop is not body-free. D_800A1494/95/96 also carry no entry in volatile_extern_allowlist.txt (nearest entries: D_800A14D0 / D_800A14D4, the granted cdread.c CdlREAD block). A future session must resolve this by ruling-request BEFORE any candidate-ready.

- [s60] The residual at masked 2 is a DIFFERENT residual from the one 59 sessions chased. Both differing instructions sit in the do_timeout printf-argument block (build insns ~51-67): the build holds the staged table index in $v1 where the target uses $a0, and the sll,2 / lw pair around insns 57-58 is transposed. The old 'coupled fixed point' frontier assumed fixing the order costs the seats - but it was formulated against a 178-instruction base with two wrong branch destinations, and vAT1 pays both of those legs already.

- [s60] CONSEQUENCE FOR THE KILL LIST: every axis killed in s53-s59 (t0-address-web refs/fold, arg3 wrap position/count, stmt-expr arg3, decl-scope shortening, chain-idx1496, cnt split-init, single-qty demotion of pseudo 104) was killed against the vT40 masked-4/178 base. Those kills measured 'does this move 4', not 'does this move 2', and the register holding the staged index differs. They are re-openable against vAT1 and should be replayed there first - the s59 variant sources are still in tmp/grind/CD_ready/s59/.

- [s60] Alternative volatile spellings all lose: type-level on the global with a direct &g_cd_status_c costs a lui/%lo (score 4, build 180 - CD_sync s107 found the same thing at decisions.md:18333); making idx_1494/idx_1495 volatile as well scores 8, in both the cast-base and the &g_cd_status_a-base spellings.

- [s60] Scope: src/system.c was restored to INCLUDE_ASM("asm/funcs", CD_ready); and verified clean against HEAD at session end. Only memory/grind/CD_ready/ files and metrics/events.jsonl are dirty. Old vT40 body retained at memory/grind/CD_ready/candidate-vT40-masked4-no-volatile.c; 4 forms added to rejected/ (bank now 129).

## s61 (rederive, 2026-09-01) — the masked-2 residual is a DECOUPLED order/seat pair, and the exact allocator inequality that couples them is now written down

Base for every measurement below: `memory/grind/CD_ready/candidate.c` (the vAT1 form recovered in
s60), spliced into `src/system.c` with `tmp/grind/CD_ready/s61/splice.py`. Re-measured live at
session start: **score 2, build_insns 179 == target_insns 179, rules_dropped 0** — the s60 floor
reproduces exactly on today's HEAD chassis.

### [s61-1] The masked-2 residual, disassembled: TWO instructions, TRANSPOSED, registers already correct
`tmp/grind/CD_ready/s61/show.py` (objdump -dr of the sandbox .o against `asm/funcs/CD_ready.s`,
index-aligned) prints the whole function; the ONLY divergence in all 179 instructions is:

    idx  BUILD                 TARGET
     55  sll  $v0,$v0,2        sll  $v0,$v0,2
     56  sll  $a0,$a0,2        addu $v0,$v0,$s5      <-- transposed
     57  addu $v0,$v0,$s5      sll  $a0,$a0,2        <-- transposed
     58  lw   $v1,0($v0)       lw   $v1,0($v0)

Every register in the block already matches the target, including both seats ($a0 for the
`*(s32 *)t0` chain, $v1 for the `arg5` stack argument). The residual is a pure emission-ORDER
swap of two independent ALU insns inside the `do_timeout` printf-argument block. This is a much
smaller and much better-characterised residual than the 59-session floor-4 one; it is NOT the old
coupled fixed point (that one also cost an instruction and two branch destinations).

### [s61-2] Pass attribution READ, not guessed: sched2, and the two insns are a priority tie
`pwsh tools/grinder/dump.ps1 CD_ready` on the vAT1 base (dumps in `tmp/grind/CD_ready/dumps/`).
`system.sched2` for CD_ready shows the post-schedule order 99(lbu a0), 115(lbu v0), 141(lw a1),
117(sll v0), **106(sll a0), 120(addu v0)**, 122(lw v1), 128, 111, 133, 137, 143. The target needs
120 before 106. Both are `{ashlsi3}`/`{addsi3_internal}` with equal-length dependency paths to the
`printf` call (106->111->145(lw a3)->call ; 120->122->137(sw 16(sp))->call), so INSN_PRIORITY ties
and `rank_for_schedule` falls through to `INSN_LUID` — i.e. to the RTL emission order, i.e. to the
C statement order. NOTE FOR FUTURE SESSIONS: `dump.ps1` prints a spurious
`src/system.c:619: too few arguments to function 'printf'` on stderr while the candidate is
spliced (CD_init calls the 5-arg prototype with 2 args). The dumps are still written and are
valid — check the mtimes, do not conclude the dump failed.

### [s61-3] CONFIRMED (F2): the ORDER is independently reachable — `t0 *= 4` position is the lever
Deferring the t0 shift until AFTER the arg5 chain (`w04_t0_shift_deferred.c`) raises the a0-shift's
LUID above the v0-addu's and the scheduler emits the TARGET order:

     55 sll v0,v0,2 | 56 addu v0,v0,s5 | 57 sll <t0>,<t0>,2 | 58 lw <arg5>,0(v0)   <-- exact target shape

This is the first time in 61 sessions that the 56/57 pair has been emitted in the target order in a
179-instruction body. F2's prediction — that vAT1 decouples the order from the seat — is CONFIRMED
for the order half.

### [s61-4] ...but the same lever inverts the SEAT: order-correct costs the $a0/$v1 assignment
`w04` scores **6**, and all six differing instructions (51,57,58,61,63,67) differ ONLY in that the
t0 chain sits in `$v1` where the target uses `$a0`, and the arg5 value sits in `$a0` where the
target uses `$v1`. Order right, seats swapped. Baseline = seats right, order wrong (2). Same
information content, opposite halves.

### [s61-5] The exact allocator equation behind the seat swap (read from source + the .lreg dump)
`tools/gcc-2.7.2/local-alloc.c:1660-1685`, `qty_compare_1`, sorts quantities DESCENDING by

    pri(q) = floor_log2(qty_n_refs[q]) * qty_n_refs[q] * qty_size[q] / (qty_death[q] - qty_birth[q]) * 10000

with ties broken by qty number (birth order). `qty_n_refs[qty] = reg_n_refs[regno]` (line 297) is
the FUNCTION-WIDE, loop-depth-weighted reference count of the pseudo, not a block-local count.
`find_free_reg` then hands out hard regs in ascending register number, so the first-allocated of
the two block pseudos takes `$v1` (because `$v0` is already taken by the higher-priority
function-scope `v0` variable, pseudo 74: refs 13 / len 8 -> pri 4.875) and the second takes `$a0`.

From `tmp/grind/CD_ready/s61/w04.lreg` (`;; Function CD_ready`), for the w04 body:
  - pseudo 98 = the t0 chain: "used 8 times across 11 insns" -> pri = floor_log2(8)*8/11 = **2.18**
  - pseudo 97 = arg5:         "used 4 times across 4 insns"  -> pri = floor_log2(4)*4/4  = **2.00**
t0 wins, is allocated first, takes `$v1`; arg5 takes `$a0`. The target needs the reverse.

**THE INEQUALITY THE NEXT SESSION MUST SATISFY** (holding the w04 order-correct shape fixed):

    pri(arg5) > pri(t0)     i.e.   floor_log2(R5)*R5/L5  >  floor_log2(Rt)*Rt/Lt

with the current values R5=4, L5=4, Rt=8, Lt=11 giving 2.00 vs 2.18. Two ways to flip it, both
tiny: (a) lengthen the t0 live range to Lt >= 13 (24/13 = 1.85 < 2.00) without changing its ref
count — note Lt = 12 gives an exact 2.00 TIE, which then resolves by qty number and still loses,
so 13 is the real threshold; or (b) raise arg5's weighted ref count to R5 = 5 or 6 at L5 = 4
(pri 2.5 / 3.0), which must stay BELOW pseudo 74's 4.875 or arg5 steals `$v0` instead.

### [s61-6] Measurements taken this session (all 179/179 insns, 0 rules, on the vAT1 base)
Fresh natural-C rederivation of the printf-argument block (modality work — a structurally
different shape, not a tweak):
  - fully natural `printf(&D_800161C8, D_800F19C0, D_800A11DC[D_800A11D5], tbl_125c[idx_1494[0]],
    tbl_125c[idx_1494[1]])` — **14**; with a `pp` alias — 14; with named byte indices — 14
  - one arg hoisted to a named intermediate: arg5-first 14, arg4-first 7, both-named 7/8
Source-position sweep of the staged block:
  - v0/arg5 chain first, pp last / pp first — 7 / 7
  - pp moved after the t0 chain — **2** (basin-equivalent to baseline)
  - `t0 *= 4` deferred past the arg5 chain (**w04**) — **6**, ORDER CORRECT (see s61-3/4)
  - only the `+tbl` deferred, shift left early — **2** (basin-equivalent; confirms the SHIFT, not
    the add, is the LUID that matters)
  - arg5 natural + t0 staged — 4; t0 natural + arg5 staged — 9; `&tbl_125c[t0]` fold — 3
Seat-control sweep on the w04 (order-correct) base — ALL score 6, i.e. the seat swap is INVARIANT
to every source-level perturbation that does not change the pri equation:
  - declaration order t0-first / pp-first, a fresh index pseudo instead of the outer `v0`,
    `pp` assigned first / last, arg5 written naturally, `&tbl_125c[t0]` fold, t0 loaded before pp
Ref-count/live-range attacks on the w04 base:
  - splitting the byte and the pointer into two variables (4 spellings) — **9**, and it reshuffles
    the whole block (lbu order flips, the D_800A11D5 sub-block migrates): a different, worse basin,
    not a seat fix
  - variable-reuse of an existing function-scope local to change `reg_n_refs`:
    arg5 := `status` 6, := `cnt` 10, := `i` 20; t0 := `i` 24, := `cnt` 9,
    t0 := `status` **5** — and z05 is a trap: the extra function-wide refs push the t0 quantity
    over the callee-saved threshold and it lands in `$s0`, while the ORDER reverts to wrong. A
    lower score here is NOT progress toward the target seat.

- [s61] Chassis check: the s60 vAT1 floor reproduces exactly on today's HEAD - score 2, build_insns 179 == target_insns 179, rules_dropped 0, cheat_asm_stripped 5.

- [s61] The masked-2 residual is now fully localised and is NOT what the s60 header guessed: all 179 instructions already carry the target's registers, both seats included ($a0 for the *(s32*)t0 chain, $v1 for the arg5 stack argument). The only divergence is that build insns 56/57 are `sll $a0,$a0,2` then `addu $v0,$v0,$s5` where the target has them transposed.

- [s61] Pass attribution READ from tmp/grind/CD_ready/dumps/system.sched2 (not hypothesised): the two insns are sched2 insns 106 and 120, they tie on INSN_PRIORITY (equal-cost dependency paths 106->111->145->call and 120->122->137->call), and rank_for_schedule falls through to INSN_LUID, so C statement order decides.

- [s61] THE ALLOCATOR EQUATION, from tools/gcc-2.7.2/local-alloc.c:1660-1685: qty_compare_1 sorts quantities DESCENDING by pri(q) = floor_log2(qty_n_refs[q]) * qty_n_refs[q] * qty_size[q] / (qty_death[q] - qty_birth[q]) * 10000, ties broken by qty number (birth order); and local-alloc.c:297 sets qty_n_refs[qty] = reg_n_refs[regno], the FUNCTION-WIDE loop-depth-weighted count. find_free_reg then hands out hard regs in ascending register number, so the first-allocated of the two block pseudos takes $v1 ($v0 is already held by the higher-priority function-scope `v0` variable, pseudo 74: refs 13 / len 8 -> pri 4.875) and the second takes $a0.

- [s61] Measured inputs from tmp/grind/CD_ready/s61/w04.lreg (;; Function CD_ready): pseudo 98 = the t0 chain, 'used 8 times across 11 insns' -> pri = 3*8/11 = 2.18; pseudo 97 = arg5, 'used 4 times across 4 insns' -> pri = 2*4/4 = 2.00. t0 sorts first, takes $v1; arg5 takes $a0 - the exact inversion of the target.

- [s61] THE INEQUALITY THE NEXT SESSION MUST SATISFY, holding the w04 order-correct shape fixed: pri(arg5) > pri(t0). Two knobs. (a) lengthen the t0 live range to >= 13 insns at unchanged refs - 24/13 = 1.85 < 2.00 wins, but Lt = 12 is an exact 2.00 TIE that resolves by qty number and still loses, so 13 is the real threshold. (b) raise arg5's weighted ref count to 5 or 6 at live range 4 (pri 2.5 / 3.0), which must stay strictly below pseudo 74's 4.875 or arg5 steals $v0 instead.

- [s61] Source-position sweep on the vAT1 base (all 179/179, 0 rules): pp moved after the t0 chain = 2 (basin-equivalent); deferring only the `+tbl` while leaving the shift early = 2, which proves the SHIFT and not the add is the LUID that matters; arg5 natural + t0 staged = 4; &tbl_125c[t0] fold = 3; v0-chain-first = 7; t0 natural + arg5 staged = 9.

- [s61] TOOLING NOTE for future sessions: `pwsh tools/grinder/dump.ps1 CD_ready` prints a spurious `src/system.c:619: too few arguments to function 'printf'` on stderr while a CD_ready candidate is spliced in (CD_init calls the 5-argument prototype with 2 arguments). The dumps are still written and are valid - check the file mtimes rather than concluding the dump failed.

- [s61] tmp/grind/CD_ready/s61/show.py is a reusable index-aligned disassembly differ (objdump -dr of tmp/sandbox/CD_ready/system.o against asm/funcs/CD_ready.s, with relocations printed) - it is what turned a bare 'score 6' into 'six instructions, register-swapped, order correct'. Use it on every scored variant in this neighbourhood; the z05 trap shows score alone lies here.

- [s61] src/system.c was restored to HEAD at end of session (tmp/grind/CD_ready/s61/splice.py --restore); the only dirt is the engine's own metrics/events.jsonl.

## s62 (structural, 2026-09-01) — the residual is now split into TWO complementary, fully-characterised branches

Live chassis re-measurement at session start: `candidate.c` (vAT1) = **score 2, build 179,
target 179, rules_dropped 0**. The ledger floor of 2 reproduces exactly; nothing drifted.

### [s62] The masked-2 floor form has EVERY REGISTER CORRECT — its whole residual is one adjacent transposition
Disassembly of `candidate.c` (tmp/grind/CD_ready/s61/show.py 50 70) against asm/funcs/CD_ready.s:
all 179 instructions match opcode-and-register EXCEPT build 56/57, which are the target's 57/56:

    build 55 sll  $v0,$v0,2   | target 55 sll  $v0,$v0,2
    build 56 sll  $a0,$a0,2   | target 56 addu $v0,$v0,$s5
    build 57 addu $v0,$v0,$s5 | target 57 sll  $a0,$a0,2
    build 58 lw   $v1,0($v0)  | target 58 lw   $v1,0($v0)

i.e. t0's `sll` and arg5's address `addu` are swapped. Registers, count, branch destinations,
everything else: exact. This is the sched2 INSN_PRIORITY tie broken by INSN_LUID that the ledger
has described since s6, now isolated with NOTHING else wrong.

### [s62] There is a second, disjoint branch whose ORDER is 100% correct and whose only defect is a pure 2-way seat swap
Respelling the arg5 address computation as an IN-PLACE accumulation on the reused `v0` carrier —
`v0 = idx_1494[1] << 2;  v0 += (s32)tbl_125c;  arg5 = *(s32 *)v0;` — on the s61 order-deferred
w04 base (variant `c02`, tmp/grind/CD_ready/s62/c02_v0_inplace_addr_shifthead.c) gives **score 6,
179 insns, 0 rules** with EVERY INSTRUCTION IN THE TARGET'S POSITION AND OPCODE. The six differing
instructions (51, 57, 58, 61, 63, 67) differ only by a consistent 2-way register swap: the t0
quantity takes `$v1` where the target uses `$a0`, and the arg5 value takes `$a0` where the target
uses `$v1`. This is the cleanest statement of the seat residual in 62 sessions.

### [s62] Best form ever measured on the order-correct branch: score 3
`v0 = idx_1494[1] << 2;  arg5 = v0 + (s32)tbl_125c;  arg5 = *(s32 *)arg5;` (variant `b07`) scores
**3** at 179/179/0. Its seats are CORRECT (t0 -> $a0, arg5 value -> $v1); the only defect is that
the address addu writes arg5's register instead of accumulating in place on `$v0`
(`addu $v1,$v0,$s5` vs the target's `addu $v0,$v0,$s5`), which then transposes 58/61.
Ten follow-up spellings of the t0 chain on this base (d01-d10: one-statement, `+=`, inline deref,
chain-first, shift interleaved at four positions, `&tbl[t0]` address-of) ALL score exactly 3
except the inline-deref (14) — the b07 basin is invariant to the t0 chain's spelling and position.

### [s62] MEASURED qty_compare_1 INPUTS on all three branches (read from .lreg, not hypothesised)
`pwsh tools/grinder/dump.ps1 CD_ready`, then the CD_ready section of
tmp/grind/CD_ready/dumps/system.lreg. Pseudo 98 = t0, pseudo 97 = the arg5 value, pseudo 74 = the
reused function-scope `v0`. "used N times" is already the loop-weighted count (this block is inside
the `goto loop` poll, so every mention counts 2).

| base | score | t0 (98) | arg5 (97) | seats |
|---|---|---|---|---|
| candidate.c (vAT1) | 2 | 8 refs / 10 insns = 2.40 | 4 / 4 = 2.00 | **CORRECT** |
| c02 + f01 (order-perfect) | 6 | 8 / 11 = 2.18 | 4 / 4 = 2.00 | swapped |
| b07 | 3 | 8 / 12 = 2.00 | 8 / 6 = 4.00 | **CORRECT** |
| e01 (t0 byte-load folded) | 10 | 8 / 10 = 2.40 | 4 / 4 = 2.00 | swapped (order broke) |

Two facts fall straight out and they CORRECT the s61 frontier: (i) the seats come out right at
t0 range **10** and at arg5 pri 4.00, and wrong at t0 range 11 — so the needed change on the
order-perfect branch is to SHORTEN t0's live range by ONE insn (11 -> 10), not to lengthen it to
>= 13 as F1 predicted; (ii) t0's reference count is 8 on every single base measured, including the
one where the byte load was folded into the shift statement — it is not tunable from C.

### [s62] Statement POSITION inside the do_timeout block is provably inert (both branches)
- f01-f08 (8 orderings of the t0 chain / arg5 load / `pp` on the c02 base): ALL score exactly 6.
- i01-i08 (8 orderings on the candidate base, incl. deferring only t0's `addu`): 2,2,4,2,2,2,4,2.
- The .lreg for f01 is IDENTICAL to c02's (97 = 4/4, 98 = 8/11), i.e. sched1 normalises source
  statement order before local-alloc sees it. Position cannot move the qty_compare inputs.
This closes, with a mechanism reading rather than a score, the axis s61's x-series killed by score.

### [s62] Dead stores, self-assigns and identity ops on a block-local scalar are removed before reg_scan
g01-g08 on the c02 base: `arg5 = 0;` before the load, `arg5 = v0;`, `arg5 = arg5;`, `arg5 |= 0;`,
`arg5 += 0;`, `arg5 = t0;`, and two positional variants — ALL score exactly 6 with 179 insns, i.e.
byte-identical to c02. The F2 "knob (b)" (retune arg5's weighted reference count) is therefore
INERT for a block-local scalar: nothing survives to reg_scan. The only way arg5's count moved (to 8,
in b07) was by giving it a REAL extra occurrence — carrying the address.

### [s62] Named intermediates for the arg-2 / arg-3 printf arguments are destructive
a01-a09 (hoisting `*pp` and/or `D_800A11DC[D_800A11D5]` into fresh locals at five positions
relative to t0's birth/death): 10, 16, 20, 20, 13, 13, 20, 16, 10 — every one worse than the base 6.
The extra pseudo perturbs sched1, exactly as s61's y-series found for the byte/pointer split.
This is the direct disproof of F1's proposed "free lengthening" probe.

### [s62] Other spellings measured on the order-deferred base (all worse, banked)
b01 9 / b02 10 / b03 9 / b04 9 / b05 10 / b06 14 / b08 9; c01 15 / c03 15 / c04 15 / c05 15 /
c06 15 / c07 15; e01-e08 10,10,10,13,10,9,10,10. Notable: a FOUR-statement arg5 chain
(`v0 <<= 2; v0 += tbl; arg5 = *v0;` written as four separate statements, c01) costs 9 points over
the three-statement form (c02) — statement COUNT in this block matters even though position does not.

### Artifacts
tmp/grind/CD_ready/s62/{gen.py,gen2.py,...,gen8.py} (variant generators, each documenting its
mechanism), the 49 variant .c files, and the three banked .lreg extracts
b07_lreg_regs.txt / f01_lreg_regs.txt / candidate_lreg_regs.txt.

- [s62] Live chassis check: candidate.c (vAT1) re-measures score 2, build_insns 179, target_insns 179, rules_dropped 0. The ledger floor reproduces exactly on the current chassis.

- [s62] The masked-2 floor form has EVERY REGISTER CORRECT. Index-aligned disassembly shows all 179 instructions matching opcode and register except build 56/57, which are the target's 57/56: build emits `sll $a0,$a0,2` then `addu $v0,$v0,$s5`, the target the reverse. The whole residual on this branch is one adjacent transposition.

- [s62] New best form on the order-correct branch: b07 (v0 = idx_1494[1] << 2; arg5 = v0 + (s32)tbl_125c; arg5 = *(s32 *)arg5;) scores 3 at 179/179/0 - the best ever recorded there (previous best 6). Its seats are CORRECT; the only defect is that the address addu writes arg5's register instead of accumulating in place on $v0, which then transposes 58/61.

- [s62] b07's basin is invariant to the t0 chain: d01-d10 (one-statement, +=, chain-first, shift interleaved at four positions, address-of-index) ALL score exactly 3 except the inline-deref spelling (14).

- [s62] Measured qty_compare_1 table (tmp/grind/CD_ready/dumps/system.lreg; pseudo 98 = t0, 97 = arg5 value): candidate.c 8/10 = 2.40 vs 4/4 = 2.00 -> correct seats; c02 and f01 8/11 = 2.18 vs 4/4 = 2.00 -> swapped; b07 8/12 = 2.00 vs 8/6 = 4.00 -> correct seats.

- [s62] t0's loop-weighted reference count is 8 on EVERY base measured, including the one where the byte load was folded into the shift statement. It is not tunable from C at this call site.

- [s62] Statement COUNT in the block matters even though statement POSITION does not: writing the arg5 chain as four statements (c01) instead of three (c02) costs 9 score points (15 vs 6).

- [s62] Other spellings measured and banked, all worse than their base: b01 9, b02 10, b03 9, b04 9, b05 10, b06 14, b08 9; c01/c03/c04/c05/c06/c07 all 15; e01-e08 10,10,10,13,10,9,10,10; a01-a09 10,16,20,20,13,13,20,16,10.

- [s62] src/system.c was restored to its committed INCLUDE_ASM state at end of session; the tree is clean. candidate.c is unchanged (vAT1, still the floor form at 2).

- [s62] Six new rejected forms banked under memory/grind/CD_ready/rejected/ (139 total), each slug naming why it is dead.

## s63 (structural, 2026-09-01) — the transposition's tie-breaker is READ from sched.c, not inferred; the two branches are proven to be a coupled fixed point in ONE variable

OWNER DIRECTIVE ACKNOWLEDGEMENT. The queue item's owner directive (2026-09-01 FORECLOSED-BUCKET
REVIEW, decisions.md Ruling A, "re-score the banked vAT1 form post-`-mel`") was EXECUTED IN s60 —
that is exactly what moved the floor 4 -> 2 and produced the current `candidate.c`. The
auto-audit's "DIRECTIVE NOT YET IN LEDGER" warning fired only because no session had said so in
these words. It is done; no further action is owed on it.

Live chassis re-measurement at session start: `candidate.c` (vAT1) = **score 2, build 179,
target 179, rules_dropped 0**. Floor reproduces exactly.

### [s63] MECHANISM READ FROM SOURCE: the 56/57 tie-breaker is INSN_LUID, and nothing else can reach it
`tools/gcc-2.7.2/sched.c:2408-2465` (`rank_for_schedule`), read in full this session:
  1. `INSN_PRIORITY` difference — the two insns tie (see below).
  2. the three-way class test against `last_scheduled_insn` — `if (link == 0 || insn_cost (tmp,
     link, last_scheduled_insn) == 1) tmp_class = 3;`. On this MIPS target every `insn_cost` in
     the window is 1, so BOTH candidates land in class 3 and the test contributes nothing.
  3. `return INSN_LUID (tmp) - INSN_LUID (tmp2);` — i.e. original RTL emission order = SOURCE
     statement order.
There is no fourth tie-breaker. This retires, with a source reading rather than a score, the
whole family of "perturb the dependence graph so the scheduler prefers the other insn" probes that
s62's frontier proposed for both branches: the class test is unreachable on this target, so the
ONLY C-visible lever on this tie is which of the two statements is written first.

### [s63] The two competing insns are named, and their INSN_PRIORITYs are structurally equal
From the CD_ready section of tmp/grind/CD_ready/dumps/system.sched (sched1 output, candidate base;
extracted with tmp/grind/CD_ready/s63/ext.py):
  insn 106  `(set (reg 102) (ashift (reg/v 98) (const_int 2)))`   = t0's `sll`    = build 56
  insn 120  `(set (reg 104) (plus (reg/v 74) (reg/v 81)))`        = arg5's `addu` = build 57
Their dependence paths to the call are isomorphic — 106 -> 111 (addu) -> 145 (lw into $a3) -> 147
(call) and 120 -> 122 (lw) -> 137 (sw to 16(sp)) -> 147 — three edges each with the load on the
same rung. The priorities are therefore equal BY SHAPE, not by accident, and no re-association or
cast change can separate them without adding an instruction (which breaks 179).

### [s63] KILLED — re-basing either byte load onto the other index pointer (j01-j05, j10)
`idx_1495 == idx_1494 + 1`, so `idx_1494[1]` and `idx_1495[0]` name the same byte through
different pseudos, as do `idx_1494[0]` and `idx_1495[-1]`. Swapping which pointer pseudo each
load hangs off is byte-identical C and DOES change the dependence graph — and it strictly
regresses: j01 (`v0 = idx_1495[0]`) 8, j02 (`v0 = *idx_1495`) 8, j03 (`t0 = idx_1495[-1]`) 8,
j04 (`t0 = *(idx_1495 - 1)`) 8, j05 (both) 13, j10 (both + reversed addu operands) 13.
The target's two byte loads are both based on the `D_800A1494` pointer; `idx_1495` exists only for
the two `*idx_1495` reads later in the body.

### [s63] KILLED (inert) — operand order and cast shape on either address `addu` (j06-j09)
`*(s32 *)((s32)tbl_125c + v0)`, `*(s32 *)((u8 *)tbl_125c + v0)`, `(s32)(t0 + (u8 *)tbl_125c)` and
`t0 += (s32)tbl_125c` all score exactly 2 with 179 insns — byte-identical to candidate.c. GCC
canonicalises `plus` operand order before RTL emission, so the C-level operand order of a
register+register add is not a lever here. (It IS a lever when the two operands are DIFFERENT
pseudos of different provenance — see k07 below, where it flips `addu $a0,$a0,$s5` to
`addu $a0,$s5,$a0` and costs a point.)

### [s63] KILLED — role swap of the two carriers (k04-k06)
Making the function-scope reused `v0` carry the t0 ADDRESS and a fresh block-local carry the arg5
index (the exact mirror of candidate.c's assignment of roles) scores 8 / 9 / 8. The carrier
choice is not free: `v0` is a cross-block pseudo that goes to global-alloc, so putting the
longer-lived t0 chain in it removes t0 from local-alloc's qty_compare entirely and the seats
come out worse, not merely different.

### [s63] KILLED — t0's shift into a second fresh local (k07, k08)
`t0b = t0 * 4; t0 = (s32)((u8 *)tbl_125c + t0b);` scores 3: it keeps candidate's 56/57
transposition AND adds a new one, `addu $a0,$s5,$a0` where the target has `addu $a0,$a0,$s5`
(build 61). Two distinct pseudos as the addu's operands change GCC's canonical operand order.

### [s63] THE RESIDUAL IS A COUPLED FIXED POINT IN ONE MEASURED VARIABLE — window 1 of reg 98
`.lreg`'s "Register 98 used 8 times across N insns" is the SUM of reg 98's TWO disjoint live
ranges (98 dies at its own `sll` and is REBORN by its `addu`), counted in sched1 OUTPUT positions.
Measured, on the sched1 dumps of both bases:

| base | window 1 (t0 lbu .. t0 sll) | window 2 (t0 addu .. a3 lw) | sum | pri = 3*8/sum | seats | order |
|---|---|---|---|---|---|---|
| candidate.c | 5 (between: a5 lbu, `*pp` load 141, a5 sll) | 5 | 10 | 2.40 | CORRECT | 56/57 swapped |
| k03 / c02 / f01 | 6 (between: a5 lbu, 141, a5 sll, **a5 addu**) | 5 | 11 | 2.18 | swapped | PERFECT |

Window 2 is IDENTICAL on both bases. The entire difference is that the order fix (moving arg5's
`addu` ahead of t0's `sll`, which is the only thing that can win the LUID tie) necessarily parks
that `addu` inside window 1 and lengthens it by exactly one. Order and seat are therefore the
same variable read twice, not two variables — the anti-correlation s61 observed by score is now
explained by a single counter.

### [s63] The full qty_compare_1 priority tables, computed from the measured .lreg inputs
`pri = floor_log2(n_refs) * n_refs * size / (death - birth)` (tools/gcc-2.7.2/local-alloc.c:1640-1684):
  candidate (seats CORRECT): 104 a5-addr 4.00 | 110 4.00 | **98 t0 2.40** | 97 a5-val 2.00 |
                             107 2.00 | 102 t0-shift 1.60
  k03/c02   (seats SWAPPED): 110 4.00 | 102 a5-addr 2.67 | **98 t0 2.18** | 97 a5-val 2.00 |
                             104 t0-shift 2.00 | 107 2.00
98 is allocated before 97 in BOTH orderings, so the seat swap is NOT a simple "who goes first"
between the two — it is find_free_reg's answer changing because 98's conflict set grew with its
range. Ties at 2.00 break by quantity NUMBER (`return *q1 - *q2;`, local-alloc.c:1683).

### [s63] KILLED — every t0-chain topology that could shorten window 1 on the order-perfect base (m01-m10)
m01 byte-load folded into the shift 7 | m02 whole address in one expression 9 | m03 byte-fold with
the chain last 8 | m04 two-statement arg5 chain 6 | m05 both folds 10 | m06 t0 byte load between
the arg5 shift and add 7 | m07 t0 byte load after the arg5 shift 7 | m08 `pp` last 6 | m09
`&((u8 *)tbl)[idx*4]` 9 | m10 `pp` between the arg5 statements 6. The order-perfect basin's floor
is 6 and none of these removes an insn from window 1: the `*pp` load is insn 141 (an ARGUMENT load
emitted by expand_call, LUID fixed at the call site), so it cannot be moved out of window 1 by
moving the `pp = ...` statement — which is why f04/f05/k02/m08/m10 are all inert.

### Artifacts
tmp/grind/CD_ready/s63/{gen.py,gen2.py,gen3.py} (28 variants, each series documenting its
mechanism), ext.py + qty.sh (dump extractors), cand.sched.txt / k03.sched.txt (the two sched1
extracts the window table is read from), look.ps1 / sweep.ps1 / splice.py / show.py.

- [s63] The owner directive on this queue item (Ruling A, re-score banked vAT1 post--mel) was executed in s60 and is what produced the current floor of 2; this session records that acknowledgement explicitly so the auto-audit stops flagging it.
- [s63] Live chassis check: candidate.c re-measures score 2, build 179, target 179, rules_dropped 0.
- [s63] sched.c:2408-2465 read in full: rank_for_schedule's only tie-breakers are INSN_PRIORITY, a three-way class test that collapses to class 3 for every insn on this target (insn_cost == 1), and INSN_LUID. There is no fourth. The 56/57 transposition is therefore decidable ONLY by C statement order.
- [s63] The two tied insns are sched1 106 (`reg 102 = reg 98 << 2`, t0's sll) and 120 (`reg 104 = reg 74 + reg 81`, arg5's address addu). Their dependence paths to the printf call are isomorphic three-edge chains with the load on the same rung, so their priorities are equal by SHAPE - no re-association can separate them without adding an instruction.
- [s63] KILLED: re-basing either byte load onto the other (identical-address) index pointer - j01 8, j02 8, j03 8, j04 8, j05 13, j10 13. Both target byte loads are based on the D_800A1494 pointer.
- [s63] KILLED (inert): operand order / cast shape on either address addu - j06, j07, j08, j09 all score exactly 2 with 179 insns, byte-identical to candidate.c. GCC canonicalises same-provenance plus operands before RTL emission.
- [s63] KILLED: carrier role swap (function-scope v0 carries t0's address, fresh local carries the arg5 index) - k04 8, k05 9, k06 8. v0 is a cross-block pseudo handled by global-alloc, so this removes t0 from qty_compare entirely.
- [s63] KILLED: t0's shift into a second fresh local (k07 3, k08 6) - it keeps candidate's transposition AND flips build 61 to `addu $a0,$s5,$a0`; two distinct pseudos as an addu's operands change GCC's canonical operand order.
- [s63] STRUCTURAL RESULT: `.lreg`'s "across N insns" for reg 98 is the SUM of its two disjoint live ranges, counted in sched1 output positions. Window 2 (t0 addu .. a3 load) is 5 on BOTH branches. Window 1 (t0 lbu .. t0 sll) is 5 on candidate and 6 on the order-perfect base, and the extra insn IS arg5's addu - the very insn the order fix must move there. Order and seat are one variable, not two.
- [s63] KILLED: all ten t0-chain topologies that could shorten window 1 on the order-perfect base (m01-m10, scores 6-10). The `*pp` load is insn 141, an ARGUMENT load emitted by expand_call with a call-site-fixed LUID, so it cannot be evicted from window 1 by moving the `pp = ...` statement - which explains why every pp-position probe (s62 f04/f05, s63 k02/m08/m10) is inert.
- [s63] src/system.c restored to its committed INCLUDE_ASM state; tree clean. candidate.c unchanged (vAT1, still the floor form at 2). Seven new rejected forms banked (146 total).

- [s64] The owner directive on this queue item (2026-09-01 FORECLOSED-BUCKET REVIEW, decisions.md Ruling A, 're-score the banked vAT1 form post--mel') was EXECUTED IN s60 - that is what moved the floor 4 -> 2 and produced the current candidate.c. This session records the acknowledgement explicitly; the consistency warning was a bookkeeping gap, not unexecuted work.

- [s64] Live chassis check: memory/grind/CD_ready/candidate.c re-measures score 2, build_insns 179, target_insns 179, rules_dropped 0. The ledger floor reproduces exactly.

- [s64] tools/gcc-2.7.2/sched.c:2408-2465 (rank_for_schedule) read in full: the only tie-breakers are INSN_PRIORITY, a three-way class test that collapses to class 3 for every insn on this target (insn_cost == 1), and INSN_LUID. There is no fourth. The 56/57 transposition is decidable ONLY by C statement order.

- [s64] The two tied insns are named from the sched1 dump: insn 106 (set (reg 102) (ashift (reg/v 98) (const_int 2))) = t0's sll = build 56, and insn 120 (set (reg 104) (plus (reg/v 74) (reg/v 81))) = arg5's address addu = build 57. Their paths to the call are isomorphic, so INSN_PRIORITY ties by shape and cannot be separated without adding an instruction.

- [s64] MEASURED window table (sched1 dumps + .lreg, both bases): candidate.c window1 = 5, window2 = 5, sum 10, pri 2.40, seats CORRECT, order wrong; k03/c02/f01 window1 = 6, window2 = 5, sum 11, pri 2.18, seats swapped, order PERFECT. Window 2 is identical on both; the whole difference is arg5's addu sitting inside window 1, which is exactly where the order fix must put it.

- [s64] Full qty_compare_1 priority tables computed from the measured .lreg inputs. candidate: 104 arg5-addr 4.00, 110 4.00, 98 t0 2.40, 97 arg5-val 2.00, 107 2.00, 102 t0-shift 1.60. k03/c02: 110 4.00, 102 arg5-addr 2.67, 98 t0 2.18, 97 arg5-val 2.00, 104 t0-shift 2.00, 107 2.00. Reg 98 is allocated BEFORE reg 97 on both branches, so the seat swap is find_free_reg's answer changing as 98's conflict set grows - not a rank inversion between the two.

- [s64] Ties in qty_order break by quantity NUMBER (local-alloc.c:1683, `return *q1 - *q2;`), which is why the three 2.00-priority quantities on the order-perfect base allocate in the order 97, 104, 107.

- [s64] KILLED: pointer re-basing of either byte load (j01-j05, j10 = 8/8/8/8/13/13); operand-order and cast reassociation on either address addu (j06-j09, all inert at 2); carrier role swap (k04-k06 = 8/9/8); t0's shift into a second fresh local (k07 = 3, k08 = 6); all ten window-1 topologies on the order-perfect base (m01-m10 = 7/9/8/6/10/7/7/6/9/6).

- [s64] src/system.c was restored to its committed INCLUDE_ASM state at end of session; the tree is clean apart from metrics/events.jsonl. candidate.c is unchanged (vAT1, still the floor form at score 2). Seven new rejected forms banked under memory/grind/CD_ready/rejected/ (146 total).

## s65 (synthesis, 2026-09-01) — the seat residual is READ OUT OF THE ALLOCATOR for the first time: QTYDBG names every quantity, its qty_compare inputs, its allocation rank and the hard register it got

OWNER DIRECTIVE: the queue item's directive (2026-09-01 FORECLOSED-BUCKET REVIEW, Ruling A row
`CD_ready (d4)`) has two halves. Half 1 (the Ruling-D CD_intr aggregate merge) is KILLED for this
function by CD_sync s107's symbol-level prong-(c) finding; half 2 (re-score the banked vAT1 form
post-`-mel`) was EXECUTED in s60 and is what produced the current floor. Acknowledged again here;
no action was owed.

Live chassis re-measurement at session start: `candidate.c` (vAT1) = **score 2, build 179,
target 179, rules_dropped 0**; `k03_a5first_t0byte_head.c` (the order-perfect base) = **6**.
Both reproduce exactly.

### [s65] THE INSTRUMENT: BB2_QTY_DEBUG/BB2_SUGG_DEBUG give the ACTUAL local-alloc answer, and it retires the .lreg proxy s61-s63 reasoned from
`bash tmp/grind/CD_ready/s63/qty.sh <out>` runs the instrumented cc1 (tools/gcc-2.7.2/cc1) over
src/system.c with the local-alloc hooks. Two line kinds matter, and both carry `func=CD_ready`:

    SUGGDBG-QTY func=CD_ready blk=3 qty=N reg1=R birth=B death=D refs=F ...   (qty_compare inputs)
    QTYDBG blk=3 ord=K qty=N reg1=R ... got=H                                 (rank K -> hard reg H)

`got=` is the hard register NUMBER (2 = $v0, 3 = $v1, 4 = $a0). `blk=3` is CD_ready's do_timeout
block; the FIRST group of `QTYDBG blk=3` lines in the file belongs to CD_ready (later groups are
other functions in the TU). birth/death are luids at 2 per insn.
**This supersedes the .lreg "Register 98 used 8 times across N insns" reading that s61/s62/s63
built their whole model on.** That line is per-REGISTER and sums two disjoint live ranges; the
quantities local-alloc actually sorts are different objects, and reg 98 is not even one of them.

### [s65] MEASURED quantity tables — the seat swap, root-caused

Block 3, candidate.c (score 2, SEATS CORRECT, order wrong):

| qty | reg1 | birth-death | refs | pri | rank | got |
|---|---|---|---|---|---|---|
| 1 | 104 arg5 ADDRESS | 18-20 (span 1 insn) | 4 | 4.00 | 0 | 2 = $v0 |
| 3 | 110 D_800A11DC chain | 22-30 | 8 | 3.00 | 1 | 2 = $v0 |
| 2 | 97 arg5 VALUE | 20-26 | 4 | 1.33 | 2 | 3 = **$v1 (target)** |
| 0 | 102 t0 chain | 16-24 | 4 | 1.00 | 3 | 4 = **$a0 (target)** |

Block 3, k03 / c02 / f01 (score 6, order PERFECT, seats swapped):

| qty | reg1 | birth-death | refs | pri | rank | got |
|---|---|---|---|---|---|---|
| 3 | 110 | 22-30 | 8 | 3.00 | 0 | 2 = $v0 |
| 0 | 102 arg5 ADDRESS | 16-20 (span 2 insns) | 4 | 2.00 | 1 | 2 = $v0 |
| 1 | 104 t0 chain | 18-24 | 4 | 1.33 | 2 | 3 = $v1 (WRONG) |
| 2 | 97 arg5 VALUE | 20-26 | 4 | 1.33 | 3 | 4 = $a0 (WRONG) |

THE MECHANISM, exactly: on the order-perfect base the t0 quantity and the arg5-VALUE quantity TIE
at pri 1.33 (identical refs 4, identical span 6 luid) and `qty_compare_1` breaks the tie by
QUANTITY NUMBER (local-alloc.c:1683 `return *q1 - *q2;`), i.e. by birth order — which t0 wins
because its `sll` is born two luids before the arg5 load. `find_free_reg` then hands the earlier
quantity the lower free hard register, so t0 takes $v1 and the value takes $a0. On candidate.c the
tie does not arise at all: the arg5 ADDRESS quantity is only ONE insn long (its `addu` and its
dependent `lw` are adjacent in sched1's output), so it ranks 4.00, is allocated first, and the
whole ordering rotates into the target's assignment.
This CORRECTS s63's H-s63-F ("find_free_reg's answer changing because 98's conflict set grew"):
the conflict sets are incidental; what changes is the RANK ORDER, and it changes because of a
sched1 latency-slot decision (below), not because of anything about reg 98.

### [s65] THE sched1 DECISION THAT COSTS THE SEATS, read from the RTL dump
tmp/grind/CD_ready/s63/k03.sched.txt, sched1 output for the order-perfect base, in order:

    99  reg98 = lbu(t0 byte)  | 106 reg74 = lbu(arg5 byte) | 141 a1 = *pp | 108 reg74 <<= 2 |
    111 reg102 = reg74 + reg81  (arg5 ADDRESS addu)  |  117 reg104 = reg98 << 2  (t0 sll)  |
    113 reg97 = mem[reg102]  (arg5 LOAD)  | 128 reg107 = lbu D_800A11D5 | 122 reg98 = reg104+reg81
    | 137 mem[sp+16] = reg97 (arg5 stack store) | 133 reg110 = reg107<<2 | 143 a2 = ... |
    145 a3 = mem[reg98] | 139 a0 = fmt | 147 call printf

The arg5 load (113) depends on the addu (111) with one cycle of latency, so sched1 fills the slot
with an independent ready insn — and it picks 117, the t0 shift. THAT one filler choice does both
kinds of damage at once: it stretches the address quantity from span 1 to span 2 (4.00 -> 2.00,
demoting it below reg 110) AND it makes the t0 quantity born before the arg5-value quantity, which
is how t0 wins the 1.33 tie. On candidate.c the t0 shift is already scheduled before the addu, so
111 and 113 end up adjacent and neither effect fires.

### [s65] THE SINGLE NAMED CHANGE THAT WOULD CLOSE THE FUNCTION (the frontier, stated as one swap)
Hold the order-perfect base. Exchange the sched1 positions of insn 145 (`a3 = mem[reg98]`, the
arg-4 register load, currently t0's death) and insn 137 (`mem[sp+16] = reg97`, the arg-5 stack
store, currently the value's death). That makes the value quantity 20-24 (pri 2.00) and the t0
quantity 18-26 (pri 1.00): the value is then allocated BEFORE t0, takes $v1, and t0 takes $a0 —
the target's seats, on a base that already has the target's instruction order. Equivalently: any
change that makes the arg5 LOAD, rather than the t0 shift, fill the 111->113 latency slot also
wins, because it flips the qty numbers.
Why 145 currently beats 137 in the ready list (read from sched.c, corroborated by the dump): 145
is a load feeding the call's $a3 (path cost 2), 137 is a store with no successor but the call
(path cost 1), so INSN_PRIORITY separates them and the LUID tiebreak never runs.

### [s65] KILLED — arg5 carrying a real second value to raise its reference count (n01-n08, 8 forms)
`arg5 = idx_1494[1]; v0 = arg5 << 2; ...` and six variants (copy-then-shift, in-place v0 address,
whole chain in arg5, pp last, t0 byte late, shift folded into the address) plus the same staging on
the candidate base: 8, 6, 8, 9, 8, 8, 6, 8 — all 179 insns, 0 rules, none better than its base.
Disassembly of n01: the ORDER comes out perfect but the byte no longer lands in the v0 carrier
(build `lbu $a0,1($s2)` + `sll $v0,$a0,2` vs target `lbu $v0` + `sll $v0,$v0,2`), i.e. the staging
pseudo does not coalesce and buys three new mismatches. QTYDBG on n03 shows block 3 dropping from
four local quantities to TWO: the staged pseudos become cross-block and leave local-alloc for
global-alloc entirely. This is the mirror image of b07 (which raises arg5's refs by carrying the
ADDRESS, gets the seats right, and pays with `addu $v1,$v0,$s5` instead of the target's in-place
`addu $v0,$v0,$s5`).

### [s65] KILLED — materialising arg4 into its own local to move the a3 load (q01-q05)
`arg4 = *(s32 *)t0;` at three positions, `arg4 = tbl_125c[t0];`, the role swap (arg5 folded into
the call and arg4 staged), and both args folded into the call: 11, 11, 12, 10, 11 — all 179 insns,
all far worse than the base 6. The a3 argument load cannot be repositioned by materialising its
value; doing so restructures the whole block.

### [s65] KILLED (inert) — hoisting the D_800A11D5 byte to lower its LUID (r01-r04)
Hypothesis: the competing latency-slot filler is insn 128 (the D_800A11D5 byte load) and it loses
only the INSN_LUID tiebreak because expand_call emits it at the call site; a `ix = D_800A11D5;`
statement placed before the t0 chain would lower its LUID and make it the filler instead. Measured:
r01 6, r03 (u8 index) 6, r04 6 — and QTYDBG on r01 is STRUCTURALLY IDENTICAL to k03 (same four
quantities, same spans 16-20/18-24/20-26/22-30, same ranks, same `got=` 2/2/3/4). The hoist is
byte-inert: the two fillers are NOT tied on INSN_PRIORITY, so the LUID rung is never reached.
r02 (the same statement at the head of the block) regresses to 11.

### Artifacts
tmp/grind/CD_ready/s65/{gen.py,gen2.py,gen3.py} (the n/q/r variant generators, each documenting
its mechanism), probe.sh (splice + disassemble + quantity-table extractor), the 17 variant .c
files, and the quantity dumps k03.qty.txt / n01.qty.txt / n03.qty.txt / b07.qty.txt / r01.qty.txt.

- [s65] The instrumented cc1's BB2_QTY_DEBUG/BB2_SUGG_DEBUG output (tmp/grind/CD_ready/s63/qty.sh) prints, for every local-alloc quantity in CD_ready's block 3, its qty_compare_1 inputs (birth/death/refs), its rank in qty_order, and the hard register find_free_reg gave it (got=, 2 = $v0, 3 = $v1, 4 = $a0). This is the direct readout of the thing 62 sessions inferred from scores and from the .lreg per-register line.
- [s65] The .lreg "Register 98 used 8 times across N insns" number that s61/s62/s63 built their model on is NOT a qty_compare input: reg 98 is not one of block 3's four local quantities at all (it is defined twice, and its second range is a different quantity). Every window-1/window-2 argument in the s63 ledger is measuring a proxy.
- [s65] MEASURED, candidate.c (score 2, seats CORRECT): block-3 quantities are 104 arg5-address 18-20 refs4 pri 4.00 -> rank 0 -> $v0; 110 22-30 refs8 3.00 -> rank 1 -> $v0; 97 arg5-value 20-26 refs4 1.33 -> rank 2 -> $v1; 102 t0 16-24 refs4 1.00 -> rank 3 -> $a0.
- [s65] MEASURED, k03/c02/f01 (score 6, order PERFECT, seats swapped): 110 22-30 refs8 3.00 -> $v0; 102 arg5-address 16-20 refs4 2.00 -> $v0; 104 t0 18-24 refs4 1.33 -> $v1; 97 arg5-value 20-26 refs4 1.33 -> $a0. The t0 and arg5-value quantities TIE at 1.33 and the tie is broken by QUANTITY NUMBER (local-alloc.c:1683), which t0 wins by being born two luids earlier.
- [s65] ROOT CAUSE (supersedes the s63 conflict-set explanation): on the order-perfect base sched1 fills the latency slot between the arg5 address addu (insn 111) and its dependent load (insn 113) with the t0 shift (insn 117). That single filler choice both stretches the address quantity from span 1 to span 2 (pri 4.00 -> 2.00, demoting it below reg 110) and makes the t0 quantity born before the arg5-value quantity, which is how t0 wins the tie. Read from tmp/grind/CD_ready/s63/k03.sched.txt.
- [s65] THE ONE CHANGE THAT WOULD CLOSE THE FUNCTION: on the order-perfect base, exchange the sched1 positions of insn 145 (a3 = mem[reg98], the arg-4 register load = t0's death) and insn 137 (mem[sp+16] = reg97, the arg-5 stack store = the value's death). Value becomes 20-24 (pri 2.00), t0 becomes 18-26 (pri 1.00), the value is allocated first and takes $v1, t0 takes $a0 - target seats on a base that already has the target order. 145 currently precedes 137 because it is a load feeding the call's $a3 (path cost 2) against a store with path cost 1, so INSN_PRIORITY separates them and no LUID tiebreak is available.
- [s65] KILLED: giving the arg5 variable a real second value to raise its reference count - n01-n08 (byte staged through arg5 in seven spellings plus the same on the candidate base) score 8/6/8/9/8/8/6/8. n01's disassembly gets the ORDER perfect but the staging pseudo does not coalesce with the v0 carrier (lbu into $a0 then sll into $v0 vs the target's in-place lbu/sll on $v0). QTYDBG on n03 shows block 3 dropping to two local quantities: the staged pseudos leave local-alloc for global-alloc.
- [s65] KILLED: materialising arg4 into its own local to move the a3 argument load (q01-q05) - 11, 11, 12, 10, 11 at 179 insns.
- [s65] KILLED (inert): hoisting the D_800A11D5 byte into a statement to lower its LUID so it, rather than the t0 shift, fills the 111->113 latency slot (r01/r03/r04 = 6, byte-inert; r02 = 11). QTYDBG on r01 is structurally identical to k03 - same quantities, spans, ranks and hard registers. The two candidate fillers are not tied on INSN_PRIORITY, so the LUID rung is never reached.
- [s65] src/system.c was restored to its committed INCLUDE_ASM state at end of session (tmp/grind/CD_ready/s63/splice.py --restore); the only dirt is metrics/events.jsonl. candidate.c is unchanged (vAT1, still the floor form at score 2). Six new rejected forms banked (152 total).

- [s65] Live chassis check: memory/grind/CD_ready/candidate.c re-measures score 2, build_insns 179, target_insns 179, rules_dropped 0; the order-perfect k03 base re-measures 6. The ledger floor reproduces exactly.

- [s65] The owner directive on this queue item (2026-09-01 FORECLOSED-BUCKET REVIEW, Ruling A row CD_ready (d4)) has two halves: half 1 (the Ruling-D CD_intr aggregate merge) is KILLED for this function by CD_sync s107's symbol-level prong-(c) finding, and half 2 (re-score the banked vAT1 form post--mel) was EXECUTED in s60 and produced the current floor. Nothing was owed this session.

- [s65] NEW INSTRUMENT: the instrumented cc1's BB2_QTY_DEBUG/BB2_SUGG_DEBUG output (bash tmp/grind/CD_ready/s63/qty.sh <out>) prints, for every local-alloc quantity in CD_ready's block 3, its qty_compare_1 inputs (birth/death/refs), its rank in qty_order and the hard register find_free_reg gave it (got=, 2=$v0 3=$v1 4=$a0). The first group of `QTYDBG blk=3` lines in the file is CD_ready's.

- [s65] The .lreg 'Register 98 used 8 times across N insns' line that s61/s62/s63 built their entire model on is NOT a qty_compare input: reg 98 is not one of block 3's four local quantities (it is defined twice and its second range belongs to a different quantity). The window-1/window-2 framing of s63 is a proxy measurement.

- [s65] MEASURED, candidate.c (score 2, seats CORRECT): 104 arg5-address 18-20 refs4 pri 4.00 rank0 -> $v0; 110 22-30 refs8 3.00 rank1 -> $v0; 97 arg5-value 20-26 refs4 1.33 rank2 -> $v1; 102 t0 16-24 refs4 1.00 rank3 -> $a0.

- [s65] MEASURED, k03/c02/f01 (score 6, order PERFECT, seats swapped): 110 3.00 -> $v0; 102 arg5-address 16-20 refs4 2.00 -> $v0; 104 t0 18-24 refs4 1.33 -> $v1; 97 arg5-value 20-26 refs4 1.33 -> $a0. t0 and the arg5 value TIE at 1.33 and the tie breaks by QUANTITY NUMBER (local-alloc.c:1683), which t0 wins by being born two luids earlier.

- [s65] ROOT CAUSE (supersedes s63's conflict-set explanation): sched1 fills the latency slot between the arg5 address addu (insn 111) and its dependent load (insn 113) with the t0 shift (insn 117). That one filler choice both stretches the address quantity from span 1 to span 2 (4.00 -> 2.00, demoting it below reg 110) and makes t0 born before the arg5 value. Read from tmp/grind/CD_ready/s63/k03.sched.txt.

- [s65] THE SINGLE CHANGE THAT WOULD CLOSE THE FUNCTION: on the order-perfect base, exchange the sched1 positions of insn 145 (a3 = mem[reg98], the arg-4 register load = t0's death, luid 24) and insn 137 (mem[sp+16] = reg97, the arg-5 stack store = the value's death, luid 26). Value becomes 20-24 (pri 2.00), t0 becomes 18-26 (pri 1.00), value allocates first and takes $v1, t0 takes $a0 - the target's seats on a base that already has the target's instruction order.

- [s65] Why that swap is hard: 145 is a load feeding the call's $a3 (path cost 2 to the call) and 137 is a store with no successor but the call (path cost 1), so INSN_PRIORITY separates them outright and no tiebreak rung is reachable.

- [s65] KILLED: arg5 carrying a real second value (its byte index) to raise reg_n_refs - n01-n08 score 8/6/8/9/8/8/6/8 at 179 insns. The staging pseudo does not coalesce with the v0 carrier, and on n03 block 3 collapses from four local quantities to two (pseudos leave local-alloc for global-alloc).

- [s65] KILLED: materialising arg4 into its own local to move the a3 argument load (q01-q05) - 11, 11, 12, 10, 11 at 179 insns.

- [s65] KILLED (inert): hoisting the D_800A11D5 byte to lower its LUID so it fills the 111->113 slot instead of the t0 shift (r01/r03/r04 = 6 with a quantity table structurally identical to k03; r02 = 11).

- [s65] src/system.c was restored to its committed INCLUDE_ASM state at end of session; the only dirt is metrics/events.jsonl plus memory/grind/CD_ready/. candidate.c is unchanged as a body (vAT1, floor 2); only its header comment gained the s65 correction. Six new rejected forms banked (152 total).

## s66 (synthesis, 2026-09-01) — the seat residual is reduced to a CLOSED-FORM statement over local-alloc's inputs, s65's named frontier probe is CORRECTED, and a new working lever on those inputs (loop-note ref weighting) is demonstrated

Owner directive acknowledgement (third time, for the auto-audit): the queue item's
2026-09-01 FORECLOSED-BUCKET REVIEW Ruling-A probe ("re-score the banked vAT1 form
post-`-mel`") was executed in s60 and is what produced the current `candidate.c` and the floor of
2. Nothing further is owed on it.

Live chassis check at session start: clean tree (`INCLUDE_ASM`) scores 179;
`memory/grind/CD_ready/candidate.c` re-measures **score 2 / build 179 / target 179 /
rules_dropped 0**; the order-perfect base `k03` re-measures **6**. Both ledger numbers reproduce.

### [s66] THE BLOCK-3 RTL, READ IN FULL — every pseudo in the residual is now named from the dump
From `tmp/grind/CD_ready/s63/k03.sched.txt` (sched1 output for the ORDER-PERFECT base, i.e. the
base whose instruction sequence IS the target's), the do_timeout block is exactly:

    91  a0 = "D_800161B8"        93  call puts
    99  reg/v 98 = lbu(reg76)                 <- `t0` byte
    106 reg/v 74 = lbu(reg76+1)               <- arg5 index, into the reused `v0` VARIABLE
    141 a1 = mem(D_800F19C0)                  <- *pp, an expand_call ARGUMENT load
    108 reg/v 74 = reg74 << 2                 <- arg5 sll, in place on v0
    111 reg 102   = reg74 + reg81             <- arg5 ADDRESS (reg74 dies here)
    117 reg 104   = reg98 << 2                <- t0 SHIFT temp (reg98 dies here)
    113 reg/v 97  = mem(reg102)               <- arg5 VALUE (reg102 dies here)
    128 reg 107   = lbu(D_800A11D5)
    122 reg/v 98  = reg104 + reg81            <- t0 ADDRESS, reg98 REBORN (reg104 dies here)
    137 mem(sp+16) = reg97                    <- the 5th argument store (reg97 dies here)
    133 reg 110   = reg107 << 2
    143 a2 = mem(reg110 + D_800A11DC)   145 a3 = mem(reg98)   139 a0 = fmt   147 call printf

`block_alloc` numbers this block's insns as **pos = 4 + 2*(index of the insn in the list above)**,
verified against all four measured quantities. That mapping is what makes the QTYDBG birth/death
columns readable, and it is the thing s65 got wrong.

### [s66] CORRECTION TO s65 — the frontier probe s65 named was aimed at the wrong pair of insns
s65's headline frontier was "exchange the sched1 positions of insn 145 (`a3 = mem(reg98)`, *t0's
death*) and insn 137 (the arg-5 stack store)". With the position mapping above, **insn 145 is not
the death of any local quantity**: the quantity s65 called "the t0 chain" is `reg 104`, the
SHIFT temp, and it is born at 117 (pos 18) and dies at **122** (pos 24), the t0 address `addu`.
`reg 98` (the user variable `t0`) never enters local-alloc at all — it is SET TWICE in the block
(99 and 122) and therefore has `REG_N_DEATHS == 2`, which disqualifies it from `local_alloc`'s
quantity list; global-alloc seats it, and seats it correctly at `$a0`. The same exclusion applies
to `reg/v 74` (`v0`), which is cross-block. Any future probe built on "move insn 145" is
therefore attacking an insn that has no bearing on the tie.

### [s66] THE RESIDUAL, IN CLOSED FORM (this is the whole remaining problem, stated once)
On the order-perfect base the four local quantities and their `qty_compare_1` inputs are
(`pri = floor_log2(refs) * refs * size / (death - birth)`; measured, `s65/k03.qty.txt`):

| qty# | reg | role | birth-death | refs | pri | got |
|---|---|---|---|---|---|---|
| 0 | 102 | arg5 ADDRESS | 16-20 | 4 | 2.00 | $v0 |
| 1 | 104 | t0 SHIFT temp | 18-24 | 4 | **1.33** | $v1 (target wants $a0) |
| 2 |  97 | arg5 VALUE   | 20-26 | 4 | **1.33** | $a0 (target wants $v1) |
| 3 | 110 | D_800A11D5 chain | 22-30 | 8 | 3.00 | $v0 |

Quantities 1 and 2 tie; `qty_compare_1` breaks the tie by quantity NUMBER
(`local-alloc.c:1683 return *q1 - *q2;`), i.e. by birth order, which the t0 temp wins. If the
order were reversed, `find_free_reg` would hand reg97 `$v1` (used set {0,1,2,5}) and then reg104
`$a0` (used set {0,1,2,3,5}) — exactly the target's seats, on a base that already emits the
target's instruction sequence, i.e. score 0.

**Everything except `refs` is FORCED by the target's own instruction sequence.** Births and deaths
are positions in that sequence; quantity numbers are birth order; both pseudos carry the minimum
possible TWO mentions (reg104: the `sll` that sets it and the `addu` that reads it; reg97: the
`lw` that sets it and the `sw` that reads it), and a third mention of either would be a third
instruction, which breaks the 179-instruction parity. **The only free input left in the whole
residual is `qty_n_refs` = `reg_n_refs`, the LOOP-DEPTH-WEIGHTED reference count.**

### [s66] NEW, WORKING LEVER — `do { } while (0)` really does move `reg_n_refs`, measured
`reg_n_refs` is accumulated in flow.c weighted by loop depth, and a `do { } while (0)` emits real
`NOTE_INSN_LOOP_BEG/END` notes. Measured with QTYDBG on the k03 base:
  - u01 (arg5 load AND the printf call inside one wrap): arg5 value refs **4 -> 6**, the
    D_800A11D5 chain 8 -> 12, the arg5 address 4 -> 5.
  - u02 / x02 / x03 (only the arg5 load statement wrapped): arg5 value refs **4 -> 5**.
  - w01 (only the printf call wrapped): arg5 value refs 4 -> 5, D_800A11D5 chain 8 -> 12.
This is the first C-level lever ever demonstrated on this function's `qty_compare_1` reference
counts. It is exactly the axis s62's g-series proved unreachable by ordinary statements (dead
stores / self-assigns on a block-local scalar are removed before reg_scan), and it is orthogonal
to the span and quantity-number inputs, both of which the target's instruction order pins.

### [s66] ...but every wrap measured so far also perturbs sched1, and the perturbation costs more than the seats are worth

| variant | wrap placement | score | what the wrap did besides moving refs |
|---|---|---|---|
| u00 / x02-base | none (arg5 address split into its own `a5a` statement) | **6** | byte-identical to k03: the split is FREE |
| w01 | printf call only, k03 spelling | 10 | the `*pp` argument load (insn 141) is pushed from build 53/54 to 60/61, stretching the arg5 value's span to 8 so its pri drops to 1.25 and it loses anyway |
| w02 | printf call only, a5a spelling | 10 | same |
| w04 | printf call only, candidate (t0-first) statement order | 10 | same |
| u01 | arg5 load + printf call | 13 | block restructured |
| u02 / x02 / x03 | arg5 load only, after the t0 statements | **8** | the t0 chain collapses into ONE 10-32 quantity (refs 12) and the two `lbu`s transpose |
| x01 / x04 / u03 | arg5 load only before the t0 statements; addu+load wrapped | 12 | order breaks harder |
| w03 | whole arg5 chain + call | 12 | **180 insns** — the only variant this session that lost instruction parity |

CAUTION for the next session: for the WRAP variants the QTYDBG `reg1=` -> role mapping is NOT
established (x02's `got=` columns read as if the seats had flipped, but its disassembly shows the
t0 chain still in `$v1` and the arg5 value still in `$a0`). Only the k03 mapping above is
RTL-verified. Re-derive the mapping from a fresh sched dump before trusting a wrap variant's table.

### [s66] KILLED — every t0-chain pseudo topology that could remove the shift temp from local-alloc (s01-s08)
The model says the tie disappears if `reg 104` stops being a local quantity, and it can only stop
being one by acquiring a second death (as `reg 98` does) or by leaving the block. Measured on the
order-perfect base: s01 separate `s32 t0a` address variable **9**; s02 `s32 *` carrier 9;
s03 `t0 <<= 2` instead of `t0 *= 4` **7** (not a no-op — the spelling IS a lever, just the wrong
way); s04 whole address in one statement 6 (byte-identical to k03); s05 `u8 *` carrier accumulated
in place 9; s06 t0 byte load moved down beside its shift 7; s07 arg4 fully inline with no `t0`
variable at all 14; s08 arg5 address accumulated in place on `v0` 15.
QTYDBG on s01 shows exactly what the model predicts and why it does not help: giving the address
its own single-death variable lets `combine_regs` tie the shift temp INTO it, producing ONE
quantity `reg 99` at 18-32 with refs 12 -> pri 2.57, which then beats the arg5 value even more
decisively (rank 2 vs 3) and takes `$v1`. Merging the t0 chain is the wrong direction.

### Artifacts
tmp/grind/CD_ready/s66/{gen.py,gen2.py,gen3.py,gen4.py} (the s/u/w/x generators, each documenting
its mechanism), probe2.sh (WSL-side QTYDBG driver — the s65 probe.sh cannot run from the Windows
Git-Bash tool, its objdump/venv calls need `bash tools/wsl.sh`), the 21 variant .c files, and the
quantity dumps s01/u01/u02/w01/x02 `.qty.txt`.

- [s66] Live chassis check: clean tree 179; candidate.c score 2 / build 179 / target 179 / rules_dropped 0; k03 (order-perfect base) 6. Both ledger numbers reproduce exactly.
- [s66] block_alloc's insn numbering for CD_ready's do_timeout block is pos = 4 + 2*(index in sched1's output order), verified against all four measured quantities. This is what makes the QTYDBG birth/death columns interpretable.
- [s66] CORRECTION TO s65: insn 145 (`a3 = mem(reg98)`) is NOT the death of any local quantity. The quantity s65 called "the t0 chain" is reg 104, the SHIFT temp, born at insn 117 and dead at insn 122 (the t0 address addu). s65's named frontier probe (exchange 145 and 137) was aimed at the wrong pair of insns.
- [s66] reg 98 (the `t0` user variable) is excluded from local-alloc because it is set twice in the block (insns 99 and 122) and so has REG_N_DEATHS == 2; local_alloc only takes single-death, single-block pseudos. Global-alloc seats it, and seats it CORRECTLY at $a0 on every base measured. reg/v 74 (`v0`) is excluded for being cross-block.
- [s66] CLOSED-FORM STATEMENT OF THE RESIDUAL: on the order-perfect base the four local quantities are 102 arg5-address 16-20 refs4 pri 2.00; 104 t0-shift-temp 18-24 refs4 pri 1.33; 97 arg5-value 20-26 refs4 pri 1.33; 110 22-30 refs8 pri 3.00. 104 and 97 TIE and the tie breaks by quantity number (local-alloc.c:1683), which 104 wins by being born one slot earlier. Births, deaths and quantity numbers are all positions in the target's own instruction sequence and are therefore FORCED; both tied pseudos carry the minimum possible two mentions. qty_n_refs (= reg_n_refs, the loop-depth-weighted count) is the ONLY free input left in the entire residual.
- [s66] CONFIRMED, NEW LEVER: a do-while(0) wrap emits real NOTE_INSN_LOOP_BEG/END and does raise the enclosed pseudos' loop-depth-weighted reg_n_refs. Measured by QTYDBG on the k03 base: both arg5-value mentions wrapped -> refs 4->6 (u01); only the load wrapped -> 4->5 (u02/x02/x03); only the printf call wrapped -> 4->5 (w01). The D_800A11D5 chain goes 8->12 when the call is inside the wrap. This is the first demonstrated C-level lever on this function's qty_compare_1 reference counts, and it is orthogonal to the span/quantity-number inputs that the target's order pins.
- [s66] KILLED (as scored): every wrap placement measured so far also perturbs sched1 and loses more than it gains - printf-call-only 10 (it displaces the *pp expand_call argument load from build 53/54 to 60/61 and stretches the arg5 value's span to 8, dropping its pri to 1.25); arg5-load-only 8 (best of the family; the t0 chain collapses into one 10-32 refs-12 quantity and the two lbu's transpose); load+call 13; whole arg5 chain + call 12 and 180 insns (loses instruction parity).
- [s66] KILLED: every t0-chain pseudo topology aimed at removing the shift temp from local-alloc - s01 separate address variable 9, s02 s32* carrier 9, s03 `t0 <<= 2` 7, s04 one-statement address 6, s05 u8* carrier 9, s06 byte load beside the shift 7, s07 arg4 fully inline 14, s08 arg5 address in place on v0 15. QTYDBG on s01 shows combine_regs ties the shift temp INTO the new single-death address variable, giving one 18-32 refs-12 quantity at pri 2.57 that beats the arg5 value harder. Merging the t0 chain is the wrong direction.
- [s66] MEASURED (byte-neutral, useful as a base): splitting the arg5 address into its own `a5a = v0 + (s32)tbl_125c;` statement is FREE - u00 scores 6, identical to k03. Any future ref-count probe can use the split spelling without paying for it.
- [s66] CAUTION: for the wrap variants the QTYDBG reg1->role mapping is NOT established. x02's `got=` columns read as though the seats flipped, but its disassembly shows the t0 chain still in $v1 and the arg5 value still in $a0. Only the k03 mapping is RTL-verified (from k03.sched.txt). Re-derive the mapping from a fresh sched dump before trusting any wrap variant's quantity table.
- [s66] TOOLING: tmp/grind/CD_ready/s65/probe.sh cannot be run from the Windows Git-Bash tool (its objdump and .venv calls are WSL-side). tmp/grind/CD_ready/s66/probe2.sh is the working splice-then-QTYDBG driver; disassembly is `bash tools/wsl.sh "python3 tmp/grind/CD_ready/s63/show.py 48 72"`.
- [s66] src/system.c was restored to its committed INCLUDE_ASM state at end of session; the only dirt is metrics/events.jsonl plus memory/grind/CD_ready/. candidate.c is unchanged as a body (vAT1, floor 2). Eight new rejected forms banked (160 total).

- [s66] Live chassis check: clean tree (INCLUDE_ASM) 179; memory/grind/CD_ready/candidate.c re-measures score 2 / build 179 / target 179 / rules_dropped 0; the order-perfect base k03 re-measures 6. Both ledger numbers reproduce exactly on today's HEAD.

- [s66] The do_timeout block's sched1 RTL is now transcribed insn-by-insn in evidence.md s66, with every pseudo's role named: reg/v 98 = the `t0` variable (set at insn 99, re-set at 122), reg/v 74 = the reused `v0` variable, reg 102 = the arg5 address, reg 104 = the t0 shift temp, reg/v 97 = the arg5 value, reg 107/110 = the D_800A11D5 chain, insn 141 = the *pp expand_call argument load.

- [s66] block_alloc numbers CD_ready's block-3 insns as pos = 4 + 2*(index in sched1's output order), verified against all four measured quantities. Prior sessions could not read the QTYDBG birth/death columns without this.

- [s66] CORRECTION TO s65: insn 145 is the death of NO local quantity, so s65's named frontier probe (exchange insns 145 and 137) was aimed at the wrong pair. The t0 quantity in the tie is reg 104, which dies at insn 122 (the t0 address addu), not at 145.

- [s66] reg 98 is excluded from local-alloc because it is set twice in the block (REG_N_DEATHS == 2); reg/v 74 is excluded for being cross-block. Global-alloc seats both, and seats them correctly, on every base measured - so the seat residual lives entirely inside local-alloc's four-quantity sort.

- [s66] CLOSED-FORM RESIDUAL: on the order-perfect base the quantities are 102 arg5-address 16-20 refs4 pri 2.00 -> $v0; 104 t0-shift-temp 18-24 refs4 pri 1.33 -> $v1; 97 arg5-value 20-26 refs4 pri 1.33 -> $a0; 110 22-30 refs8 pri 3.00 -> $v0. 104 and 97 TIE and the tie breaks by quantity number, which 104 wins by being born one slot earlier. If reg 97 sorted first, find_free_reg would give it $v1 (used {0,1,2,5}) and then give reg 104 $a0 (used {0,1,2,3,5}) - the target, i.e. score 0.

- [s66] NEW LEVER, first ever demonstrated on this function's qty_compare_1 reference counts: a do-while(0) wrap raises the enclosed pseudos' loop-depth-weighted reg_n_refs (arg5 value 4 -> 6 with both mentions inside, 4 -> 5 with one; D_800A11D5 chain 8 -> 12). This is exactly the axis s62's g-series proved unreachable by ordinary statements.

- [s66] KILLED (scored, 179 insns, 0 rules): all ten do-while(0) wrap placements - u01 13, u02 8, u03 12, u04 13, w01 10, w02 10, w03 12 (180 insns), w04 10, x01 12, x02 8, x03 8, x04 12. The printf-call-only wraps lose because the note displaces the *pp expand_call argument load; the load-only wraps lose because the t0 chain collapses into one merged quantity and the two lbu's transpose.

- [s66] KILLED (scored): all eight t0-chain pseudo topologies aimed at removing the shift temp from local-alloc - s01 9, s02 9, s03 7, s04 6, s05 9, s06 7, s07 14, s08 15. QTYDBG on s01 shows combine_regs merging the shift temp into a single-death address variable, which makes the merged t0 quantity (pri 2.57) beat the arg5 value harder.

- [s66] MEASURED and free: the arg5 address split (u00) is byte-identical to k03 at score 6.

- [s66] CAUTION banked for the next session: for the WRAP variants the QTYDBG reg1 -> role mapping is NOT established - x02's got= columns read as though the seats had flipped, but its disassembly shows the t0 chain still in $v1 and the arg5 value still in $a0. Only the k03 mapping is RTL-verified.

- [s66] TOOLING: tmp/grind/CD_ready/s65/probe.sh cannot run from the Windows Git-Bash tool (its objdump and .venv calls are WSL-side). tmp/grind/CD_ready/s66/probe2.sh is the working splice-then-QTYDBG driver; disassembly is `bash tools/wsl.sh "python3 tmp/grind/CD_ready/s63/show.py 48 72"`.

- [s66] Owner directive acknowledgement: the queue item's 2026-09-01 FORECLOSED-BUCKET REVIEW Ruling-A named probe (re-score the banked vAT1 form post--mel) was executed in s60 and produced the current floor of 2; nothing was owed this session. Recorded here for the third time because the auto-audit still flags it.

- [s66] src/system.c was restored to its committed INCLUDE_ASM state at end of session; the only dirt is metrics/events.jsonl plus memory/grind/CD_ready/. candidate.c is unchanged as a body (vAT1, floor 2) - only its header comment gained the s66 correction, and it was re-scored at 2 after that edit. Eight new rejected forms banked (160 total).

## s67 (solver, 2026-09-01) — the loop-note lever is characterised in BOTH directions, the *pp order defect is SOLVED, and the arg5-value seat is SOLVED; a new order-perfect base (d01) replaces k03 as the frontier

Owner directive acknowledgement (fourth time, for the auto-audit): the queue item's
2026-09-01 FORECLOSED-BUCKET REVIEW Ruling-A named probe ("re-score the banked vAT1 form
post-`-mel`") was executed in s60 and produced the current `candidate.c` and the floor of 2.
Nothing further is owed on it.

Live chassis check at session start: clean tree (`INCLUDE_ASM`) scores 179;
`memory/grind/CD_ready/candidate.c` re-measures **score 2 / build 179 / target 179 /
rules_dropped 0**; the order-perfect base `k03`/`u00` re-measures **6**. Both ledger numbers
reproduce exactly on today's HEAD.

### [s67] KILLED — the ledger's named frontier probe: an EMPTY `do { } while (0)` is NOT byte-neutral anywhere
The s66 frontier asked whether the loop NOTE by itself moves code, or whether the sched1 damage
in w01/u02/x02 comes only from the raised reference counts changing local-alloc downstream.
Six empty wraps were spliced into the `u00` body (score 6), one per insertion point:

| position | score |
|---|---|
| before the t0 byte load        | 9 |
| before the `pp` assignment     | 7 |
| before the arg5 byte load      | 7 |
| before `t0 *= 4`               | 10 |
| before the arg5 load           | 11 |
| immediately before the printf  | 10 |

**Not one is byte-neutral** (base 6). An empty wrap raises no pseudo's reference count — it
contains no mentions — so the entire perturbation is attributable to the NOTE. The loop note
behaves as a sched1 region boundary in this block. Consequence, stated as a rule for future
sessions: **on this function you can never buy a reference count for free; every note you add
costs instruction order somewhere, and the question is only whether the order damage is
repairable.** This closes the s66 frontier item 1 as a KILL, and it is the explanation that
unifies every wrap result s66 measured.

### [s67] CONFIRMED, NEW AXIS — reference counts can be LOWERED as well as raised, by SPLITTING the existing wrap
s66 only ever ADDED wraps. But the whole do_timeout block already sits inside the sanctioned
tbl_125c `do { } while (0)`, so every mention in it is counted at loop depth 2. Cutting that wrap
in two and letting a statement fall into the BARE gap puts its mentions back at depth 1.
Measured on the `u00` base (score 6), varying which statements sit in the bare gap:

| variant | bare gap | score | insns |
|---|---|---|---|
| y02 | `t0 *= 4` only            | **6** | 179 |
| y01 | `t0 *= 4` + `t0 = tbl+t0` | 11 | 179 |
| y03 | `t0 = tbl+t0` only        | 15 | 179 |
| y06 | both, no second wrap      | 15 | 179 |
| y07 | control: wrap merely CUT IN TWO, nothing bare (all depths held at 2) | 10 | 179 |
| y20 | whole arg5 address chain bare | 17 | 179 |

y07 is the important control: re-bracketing alone, with no depth change, costs 4 points. y02 is
therefore not "free"; it is a depth change that happens to pay for its own note.

**y02's disassembly is the first form in this function's history whose SEATS are the target's on
a base derived from the order-perfect body**: the arg5 value lands in `$v1` and the t0 address in
`$a0`, exactly as the target has them. Its whole residual is ordering — the `*pp` argument load
sinks from the target's 53/54 to 60/61 and the t0 `addu` rises from 61 to 56 to fill the hole,
plus the two `lbu`s transpose. This is the exact mirror of k03 (order right, seats wrong) and it
proves the depth-lowering axis does what the closed-form model says it does.

### [s67] MEASURED — statement order is INERT inside the split-wrap skeleton
A 4x4 grid (gen4.py, variants d01-d16) crossed four permutations of the four head loads
(`t0 = idx[0]`, `v0 = idx[1]`, `pp = ...`, `a1v = *pp`) against four split boundaries. The scores
depend ONLY on the boundary and are **identical across all four head permutations**:
7 / 11 / 6 / 9 repeated four times. Inside a wrapped scheduling region the four independent
loads are ordered by INSN_PRIORITY alone; no two of them tie, so the INSN_LUID rung — the single
ordering lever s63 identified — is never reached. Future sessions should not spend measurements
permuting those four statements.

### [s67] CONFIRMED — s66's frontier item 2: the `a1v = *pp` named intermediate puts the *pp load back at 53/54
s66 predicted that the y-family's *pp defect is structural: `*pp` is an expand_call ARGUMENT load,
emitted at the call site, so it is born inside wrap2 and sched1 will not lift it across the note.
Giving it its own statement converts it into an ordinary load positioned by INSN_LUID. Measured
(c01-c06, all score 6 at 180 insns): the `lui $a1 / lw $a1` pair moves to build **53/54, the
target's slots**, on every spelling. The 180th instruction is NOT a copy — the intermediate
coalesces — it is a load-delay `nop` maspsx must emit because the t0 `addu` no longer fills the
D_800A11D5 `lbu`'s delay slot. Control c00/c08 (the hoist alone on the UNSPLIT base) scores 10, so
the hoist only pays inside the split skeleton.

### [s67] THE NEW FRONTIER BASE — d01: order-perfect at 179 instructions, residual is purely seats
`d01` = wrap1 `{puts, t0 byte, arg5 byte, pp, a1v, arg5 sll, arg5 addr}` / bare `{t0 *= 4}` /
wrap2 `{arg5 load, t0 addr, printf, CD_flush}`. Score 7, 179 instructions, 0 rules. Its
disassembly (tmp/grind/CD_ready/s67/, `show.py 50 69`) is the target's instruction SEQUENCE from
insn 53 to the end — including the `*pp` pair at 53/54 and the t0 `addu` back at 61 filling the
D_800A11D5 delay slot. The seven differing instructions are the transposed `lbu` pair at 51/52 and
five register mismatches. Local-alloc read directly (`s67/d01.qty.txt`):

| qty | reg | role | birth-death | refs | pri | ord | got | target |
|---|---|---|---|---|---|---|---|---|
| 0 | 98  | t0            | 10-34 | 10 | 1.25 | 2 | $v1 | **$a0** |
| 1 | 100 | a1v           | 14-36 | 4  | (copy-suggested) | SUGG | $a1 | $a1 OK |
| 2 | 97  | arg5 address  | 16-20 | 4  | 2.00 | 1 | $v0 | $v0 OK |
| 3 | 96  | arg5 VALUE    | 20-28 | 4  | 1.00 | 3 | $a2 | **$v1** |
| 4 | 111 | D_800A11D5    | 24-32 | 8  | 3.00 | 0 | $v0 | $v0 OK |

### [s67] CONFIRMED BY PRE-REGISTERED PREDICTION — double-nesting the arg5 load gives the arg5 value its target seat
From the d01 table the model predicts: reg96 must outrank reg98, i.e. `pri(96) > 1.25`. With
`pri = floor_log2(refs)*refs*size/(death-birth)` and span 8, `refs=5` gives exactly 1.25 (a TIE,
which reg98 wins on quantity number, 0 < 3) and `refs=6` gives 1.50 (a WIN). reg96 has exactly two
mentions, so refs 6 requires depth 4 on the load plus depth 2 on the call. Predicted BEFORE
measuring; then measured (e02, arg5 load wrapped in two extra nesting levels):
  - `refs(96)` = **6**, exactly as predicted;
  - reg96 moves from ord 3 to **ord 2** and `got=3` = **`$v1`, the target's seat**;
  - the disassembly confirms it end-to-end: build `58 lw $v1,0($v0)` / `62 sw $v1,16($sp)` against
    target `58 lw $v1,0($v0)` / `63 sw $v1,0x10($sp)` — same register, the seat 62 sessions chased.
  - e01 (ONE nesting level, `refs=5`) scores 9 and does NOT flip the seat — the predicted tie.
The local-alloc model of this function is now fully predictive: it named the required reference
count and the resulting hard register in advance, and both came out.

### [s67] WHAT REMAINS, stated precisely
e02 scores 8, not 0, because the win costs two things:
  1. **reg98 (t0) falls to ord 3 and takes `$a3`, not the target's `$a0`.** Its find_free_reg
     `used` set already contains 4 (`$a0`) on this base — in y02, which has no `a1v` quantity,
     reg98's used set does NOT contain 4 and it correctly takes `$a0`. So introducing the a1v
     quantity is what removes `$a0` from t0's reach. Establishing WHY `$a0` enters that used set
     is the single unresolved mechanical question left in the residual.
  2. The two extra nesting notes re-perturb the instruction order (the D_800A11D5 chain and the
     t0 `addu` shift), costing back what d01 had won.
So the function now has three separately-solved pieces that have never been held simultaneously:
the target instruction sequence (k03/d01), the arg5-value seat (e02), and the t0 seat (y02).

### Artifacts
tmp/grind/CD_ready/s67/{gen.py,gen2.py,gen3.py,gen4.py,gen5.py} (each documenting its mechanism),
the 47 variant .c files, and the quantity dumps y02.qty.txt / d01.qty.txt / e02.qty.txt.

- [s67] Live chassis check: clean tree (INCLUDE_ASM) 179; memory/grind/CD_ready/candidate.c re-measures score 2 / build 179 / target 179 / rules_dropped 0; the order-perfect base u00/k03 re-measures 6. Both ledger numbers reproduce exactly.
- [s67] KILLED, and it answers the s66 frontier item 1: an EMPTY `do { } while (0);` is NOT byte-neutral at ANY of six insertion points in the do_timeout block (scores 9/7/7/10/11/10 against the base's 6). An empty wrap raises no reference count, so the whole perturbation is the NOTE. The loop note acts as a sched1 region boundary here. RULE FOR FUTURE SESSIONS: on this function a reference count can never be bought for free - every added note costs instruction order somewhere.
- [s67] CONFIRMED, NEW AXIS never tried before s67: reference counts can be LOWERED as well as raised, by SPLITTING the already-present tbl_125c do-while(0) so a statement falls into a BARE depth-1 gap. y02 (`t0 *= 4` alone in the gap) scores 6 at 179 insns and is the FIRST form in this function's history whose SEATS are the target's on a body derived from the order-perfect base: arg5 value in $v1, t0 address in $a0. Its residual is purely ordering.
- [s67] MEASURED control: re-bracketing the wrap into two with NOTHING in the bare gap (all depths held at 2) costs 4 points (y07 = 10). y02's depth change therefore pays for its own note; it is not free.
- [s67] KILLED: y01 both t0 statements bare 11; y03 t0 addu alone bare 15; y06 both bare with no second wrap 15; y20 whole arg5 address chain bare 17; y14 both bare with the arg5 load wrapped 11.
- [s67] MEASURED, and it retires an entire lever: statement ORDER is INERT inside the split-wrap skeleton. A 4x4 grid (d01-d16) crossing four permutations of the four head loads against four split boundaries produced scores that depend ONLY on the boundary - 7/11/6/9, identical for all four head permutations. The four loads are separated by INSN_PRIORITY, so the INSN_LUID rung s63 identified as "the ONLY lever" is never reached inside a wrapped region.
- [s67] CONFIRMED, s66 frontier item 2: hoisting the printf's second argument into a named intermediate (`void *a1v; a1v = *pp;`) moves the *pp load from build 60/61 back to the target's 53/54 on every spelling measured (c01-c06, score 6). The load is an expand_call ARGUMENT load born inside wrap2; giving it a statement makes its position depend on INSN_LUID instead. The extra 180th instruction is a load-delay nop, not a copy - the intermediate coalesces. Control: the hoist alone on the UNSPLIT base scores 10 (c00/c08), so it only pays inside the split skeleton.
- [s67] NEW FRONTIER BASE d01 (wrap1 = puts/t0byte/arg5byte/pp/a1v/arg5sll/arg5addr, bare = `t0 *= 4`, wrap2 = arg5load/t0addr/printf/CD_flush): score 7, 179 insns, 0 rules, and its instruction SEQUENCE is the target's from insn 53 to the end - the *pp pair at 53/54 and the t0 addu back at 61 filling the D_800A11D5 delay slot. The residual is the transposed lbu pair at 51/52 plus five register mismatches. This base replaces k03 as the frontier.
- [s67] MEASURED, d01's local-alloc table: qty0 reg98 t0 10-34 refs10 pri 1.25 -> ord2 -> $v1 (target $a0); qty1 reg100 a1v 14-36 refs4 copy-suggested -> $a1 OK; qty2 reg97 arg5 address 16-20 refs4 pri 2.00 -> ord1 -> $v0 OK; qty3 reg96 arg5 VALUE 20-28 refs4 pri 1.00 -> ord3 -> $a2 (target $v1); qty4 reg111 D_800A11D5 24-32 refs8 pri 3.00 -> ord0 -> $v0 OK.
- [s67] CONFIRMED BY PRE-REGISTERED PREDICTION: the model predicted that reg96 needs refs=6 (refs=5 gives pri 1.25, an exact tie reg98 wins on quantity number; refs=6 gives 1.50 > 1.25), reachable by nesting the arg5 load two extra levels. Measured e02: refs(96)=6 exactly, reg96 moves ord3 -> ord2 and got=3 = $v1, THE TARGET'S SEAT, confirmed in the disassembly as `lw $v1,0($v0)` / `sw $v1,16($sp)`. e01 (one level, refs=5) does not flip it, exactly as the predicted tie says. The local-alloc model of this function is now fully predictive.
- [s67] THE REMAINING QUESTION, stated mechanically: on the a1v-carrying bases (d01/e02) reg98's find_free_reg `used` set CONTAINS 4 ($a0), so t0 cannot reach its target seat and takes $v1 (d01) or $a3 (e02). On y02, which has no a1v quantity, reg98's used set does NOT contain 4 and it takes $a0 correctly. Why introducing the a1v quantity puts $a0 into t0's used set is the single unresolved mechanical question in the whole residual.
- [s67] The function now has three separately-SOLVED pieces that have never been held at once: the target instruction sequence (k03/d01), the arg5-value seat (e02, via double-nesting the arg5 load), and the t0 seat (y02, via the depth split with no a1v).
- [s67] src/system.c was restored to its committed INCLUDE_ASM state at end of session; the only dirt is metrics/events.jsonl plus memory/grind/CD_ready/. candidate.c is unchanged as a body (vAT1, floor 2). Fifteen new rejected forms banked (175 total).

- [s67] Live chassis check: clean tree (INCLUDE_ASM) scores 179; memory/grind/CD_ready/candidate.c re-measures score 2 / build 179 / target 179 / rules_dropped 0; the order-perfect base u00/k03 re-measures 6. Both ledger numbers reproduce exactly on today's HEAD, and candidate.c was re-verified at 2 again after its header comment was updated.

- [s67] Owner directive acknowledgement (fourth recording, for the auto-audit): the queue item's 2026-09-01 FORECLOSED-BUCKET REVIEW Ruling-A named probe ('re-score the banked vAT1 form post--mel') was executed in s60 and is what produced the current candidate.c and the floor of 2. Nothing further was owed this session.

- [s67] KILLED, answering the s66 frontier item 1: an EMPTY do { } while (0); is byte-neutral at NONE of six insertion points in the do_timeout block (9/7/7/10/11/10 against the base's 6). An empty wrap raises no reference count, so the whole perturbation is the NOTE - the loop note acts as a sched1 region boundary here. STANDING RULE for future sessions on this function: a reference count can never be bought for free; every added note costs instruction order somewhere, and the only question is whether that damage is repairable.

- [s67] CONFIRMED, an axis no prior session attempted: reg_n_refs can be LOWERED by SPLITTING the already-present tbl_125c do-while(0) so a statement falls into a bare loop-depth-1 gap. y02 (`t0 *= 4` alone in the gap) scores 6 at 179 insns and is the FIRST body derived from the order-perfect base whose SEATS are the target's: arg5 value in $v1, t0 address in $a0.

- [s67] MEASURED control: cutting the wrap in two with NOTHING in the bare gap (all loop depths held at 2) costs 4 points (y07 = 10). y02's depth change therefore pays for its own note rather than being free.

- [s67] KILLED: y01 both t0 statements at depth 1 = 11; y03 the t0 addu alone at depth 1 = 15; y06 both at depth 1 with no second wrap = 15; y14 both at depth 1 with the arg5 load wrapped = 11; y20 the whole arg5 address chain at depth 1 = 17.

- [s67] MEASURED, and it retires a lever the ledger had called 'the ONLY lever': statement ORDER is INERT inside the split-wrap skeleton. A 4x4 grid (d01-d16) crossing four permutations of the four head loads against four split boundaries gave scores that depend only on the boundary - 7/11/6/9, identical for all four permutations - and y09-y17 on the non-hoisted skeleton are all 6. The four independent loads are separated by INSN_PRIORITY, so sched.c's INSN_LUID rung is never reached inside a wrapped region.

- [s67] CONFIRMED, answering the s66 frontier item 2: hoisting the printf's second argument into a named intermediate (`void *a1v; a1v = *pp;`) moves the *pp load from build 60/61 back to the target's 53/54 on every in-wrap1 spelling measured (c01-c06, score 6 at 180 insns). *pp is an expand_call ARGUMENT load born inside wrap2; giving it a statement makes its position depend on INSN_LUID. The extra 180th instruction is a load-delay nop, NOT a copy - the intermediate coalesces. Controls: the hoist alone on the UNSPLIT base scores 10 (c00/c08); the hoist inside wrap2 reproduces the late load (c07 = 8).

- [s67] NEW FRONTIER BASE d01 - wrap1 {puts, t0 byte, arg5 byte, pp, a1v, arg5 sll, arg5 addr} / bare {t0 *= 4} / wrap2 {arg5 load, t0 addr, printf, CD_flush}: score 7, 179 insns, 0 rules, and its instruction SEQUENCE is the target's from insn 53 to the end, including the *pp pair at 53/54 and the t0 addu back at 61 filling the D_800A11D5 delay slot. The seven differing instructions are the transposed lbu pair at 51/52 plus five register mismatches. Saved as memory/grind/CD_ready/progress/s67-d01-order-perfect-base.c; this base replaces k03 as the frontier.

- [s67] MEASURED, d01's local-alloc table (tmp/grind/CD_ready/s67/d01.qty.txt): qty0 reg98 t0 10-34 refs10 pri 1.25 -> ord2 -> $v1 (target wants $a0); qty1 reg100 a1v 14-36 refs4 copy-suggested -> $a1, correct; qty2 reg97 arg5 address 16-20 refs4 pri 2.00 -> ord1 -> $v0, correct; qty3 reg96 arg5 VALUE 20-28 refs4 pri 1.00 -> ord3 -> $a2 (target wants $v1); qty4 reg111 D_800A11D5 24-32 refs8 pri 3.00 -> ord0 -> $v0, correct.

- [s67] CONFIRMED BY PRE-REGISTERED PREDICTION: the model said reg96 needs refs exactly 6 (refs 5 gives pri 1.25, an exact tie reg98 wins on quantity number; refs 6 gives 1.50 > 1.25), reachable by nesting the arg5 load two extra levels. Measured e02: refs(96) = 6 exactly, reg96 moved ord3 -> ord2 and got=3 = $v1, the target's seat, confirmed in the disassembly as `lw $v1,0($v0)` / `sw $v1,16($sp)`. e01 (one level, refs 5) does not flip it, exactly as the predicted tie says.

- [s67] THE REMAINING QUESTION, stated mechanically and for the first time located OUTSIDE the priority sort: on every a1v-carrying base (d01, e02) reg98's find_free_reg `used` set CONTAINS hard reg 4 ($a0), so t0 cannot reach its target seat and takes $v1 (d01) or $a3 (e02). On y02, which has no a1v quantity, reg98's used set does NOT contain 4 and it takes $a0 correctly. Why introducing the a1v quantity puts $a0 into t0's used set is the single unresolved mechanical question in the whole residual.

- [s67] The function now has three separately-SOLVED pieces that have never been held simultaneously: the target instruction sequence (k03 and d01), the arg5-value seat $v1 (e02, via refs(reg96) = 6), and the t0 seat $a0 (y02, via the depth split on a body with no a1v quantity).

- [s67] F3 is carried unchanged and remains blocking for COMPLETION only, not for progress: the `volatile u8 *idx_1496;` in candidate.c needs a legitimate-volatile-interrupt-touched prong-2 ruling plus a volatile_extern_allowlist.txt grant for D_800A1494/95/96. It should be raised as a ruling-request the moment the score reaches 0, not before.

- [s67] src/system.c was restored to its committed INCLUDE_ASM state at end of session; the only dirt is metrics/events.jsonl plus memory/grind/CD_ready/. candidate.c's BODY is unchanged (vAT1, floor 2); only its header comment gained the s67 summary. Fifteen new rejected forms banked (175 total).

## s68 (2026-09-01, escalation) — chassis re-measured; both live frontier items KILLED; Ruling D executed first-hand

**Chassis (measured this session, not inherited).** `memory/grind/CD_ready/candidate.c` (the s60
vAT1 body) spliced over `INCLUDE_ASM("asm/funcs", CD_ready);` at `src/system.c:379` via
`tmp/grind/CD_ready/s63/splice.py` -> `sandbox CD_ready --disable all` = **score 2,
target_insns 179, build_insns 179, rules_dropped 0, scorable true**. The ledger floor is
chassis-current. `src/system.c` restored to `INCLUDE_ASM` and verified clean (`git diff` empty)
after all measurement.

**Owner directive status (2026-09-01 FORECLOSED-BUCKET REVIEW, Ruling A row `CD_ready (d4)`).**
The row names two things: (i) "re-score the banked vAT1 form post-`-mel`" — EXECUTED in s60
(floor 4 -> 2, and that is the floor re-verified above); (ii) "Ruling D session" (the CD_intr
aggregate merge) — EXECUTED IN FULL THIS SESSION, both prongs failing (below). The review's
premise-correction (the s50 volatile disqualification rested on a false premise; the same TU's
matched `cdrom_IrqHandler`/`CD_flush` do declare these bytes volatile) is ACCEPTED and is
already reflected in candidate.c's `volatile u8 *idx_1496`, which is load-bearing and measured
so. The directive is now fully discharged.

### Ruling D — prong (c) asm-consumer check: FAILS structurally (independently re-verified here)

The sanctioned per-word-splat-symbol -> aggregate merge family
(`.claude/rules/no-new-park-categories.md`, owner ruling 2026-08-17) requires at prong (c) that
the merge be COMPLETE: every merged per-word symbol removed from C *and* from the splat symbol
config, leaving exactly one C handle per storage location. Verified first-hand this session,
not inherited from the CD_sync record:

  - the storage is DEFINED IN ASSEMBLY — `asm/data/7D920.data.s:31050/31056/31062` carry
    `dlabel D_800A1494` / `D_800A1495` / `D_800A1496`, and `:31069` `dlabel D_800A1498`
    (whose first word is `.word D_800A1494`);
  - nine assembly files reference those names (`grep -rlE 'D_800A149[456]' asm/`): the three
    INCLUDE_ASM bodies `CD_sync.s` (2) / `CD_ready.s` (2) / `CD_datasync.s` (2), the data file,
    and FIVE asm-only consumers — `getintr.s` (22 sites), `CD_cw.s` (8), `func_800817A0.s`
    (= CD_flush, 8), `func_800819C4.s` (8), `func_80081E1C.s` (2).

The per-word symbols must therefore survive in the splat symbol config, so any `CD_intr`
declaration is necessarily a SECOND handle on the same storage — verbatim the failure that
killed the g_stage_id merge (decisions.md:10722) and that killed Ruling D on CD_sync
(decisions.md:18305-18317). Ruling D named this check mandatory and first precisely because it
is the prong g_stage_id died on. The finding is symbol-level, so it is the same finding for all
three CD_* members; it is re-derived here rather than cited so this ledger stands alone.

### Ruling D — the aggregate measured on THIS function: 18x-20x worse than the floor

Two probes on the candidate base, the aggregate reached through a cast pointer at the existing
symbol so no splat-config change was needed (`tmp/grind/CD_ready/s68/rd1.c`, `rd2.c`):

| probe | spelling | score | build_insns |
|---|---|---|---|
| baseline | candidate.c | **2** | 179 |
| rd1 | `volatile CD_intr *ip = (volatile CD_intr *)&D_800A1494;` + `ip->sync/ready/c` | 41 | 173 |
| rd2 | same, non-volatile | 37 | 172 |

Both LOSE 6-7 instructions off the 179-instruction target as well as scoring 18x-20x worse.
This reproduces CD_sync's s107 finding (its prong-conformant spellings scored 18-34) on a second
member of the set. Combined with the s60 measurements already banked here (all three Intr
pointers volatile = 8/179; type-level `extern volatile u8 g_cd_status_c` with the pointer taken
directly = 4/180), the aggregate is not a lever on this function under any spelling tried.

### Frontier item 1 (a1v out of local-alloc) — KILLED, with the mechanism named

Hypothesis (s67 frontier): reg98's (`t0`) `find_free_reg` `used` set contains hard reg 4 ($a0)
only because the `a1v` quantity exists; respelling `a1v` so it leaves local-alloc's quantity
list — or shortens its 14-36 live range — should restore $a0 to t0.

Six probes on the s67 `d01` order-perfect base (score 7, 179 insns), scored with
`tmp/grind/CD_ready/s63/sweep.ps1`:

| probe | spelling | score |
|---|---|---|
| f1a | `a1v` declared at FUNCTION scope (the strongest form: "make it cross-block") | 7 |
| f1b | `a1v` carried by the existing function-scope `v0` (variable-reuse family) | 16 |
| f1c | `a1v` typed `s32` (mode change) | 7 |
| f1d | `a1v` loaded LAST, immediately before the call (live-range shortening) | 10 |
| f1e | `pp` local deleted, `a1v = D_800F19C0` directly | 7 |
| f1f | f1a + the f2a address fold | 7 |

The decisive measurement is not the score but the local-alloc dump. `bash
tmp/grind/CD_ready/s63/qty.sh tmp/grind/CD_ready/s68/f1a.qty.txt` (instrumented cc1,
BB2_QTY_DEBUG/BB2_SUGG_DEBUG), block 3 of `CD_ready`, compared line-for-line against
`tmp/grind/CD_ready/s67/d01.qty.txt`:

```
d01 : qty1 reg100 born14 dead36 refs4 copysugg=5 -> got 5   qty0 reg98  born10 dead34 refs10 used=0,1,2,4,5,6 -> got 3
f1a : qty1 reg76  born14 dead36 refs4 copysugg=5 -> got 5   qty0 reg99  born10 dead34 refs10 used=0,1,2,4,5,6 -> got 3
```

Every quantity's birth, death, refs, `used` set, allocation order and assigned hard register is
IDENTICAL; only the pseudo NUMBERS shift (100->76, 98->99, 97->98, 96->97). **Declaring `a1v` at
function scope does not remove it from local-alloc's quantity list.** The mechanism is exact and
it forecloses the whole axis: `local_alloc`/`block_alloc` decides quantity membership from
`REG_BASIC_BLOCK` — a pseudo is local iff *all its references* lie in one basic block — and C
declaration scope has no bearing on that. `reg/v 74` (the s67 note's model of a "cross-block"
pseudo) is excluded because it is genuinely referenced in more than one block, not because it is
declared at function scope. To move `a1v` out of local-alloc one must add a reference in another
basic block, which costs at least one instruction and is barred by 179-instruction parity.
The live-range-shortening variant (f1d) also fails: it moves the score the wrong way (10) because
the load then competes for the call-argument slot. `used` for reg98 is therefore not reducible
through `a1v` at all — hard reg 4 is in it because $a0 is the printf call's first argument and
conflicts with the 10-34 range directly, not through `a1v` (which takes $a1 = 5).

### Frontier item 2 (shorten the arg5 dependence path) — KILLED

Hypothesis (s67 frontier): d01's last two ordering defects (the transposed `lbu` pair at 51/52)
follow from the arg5 byte's chain carrying a longer `INSN_PRIORITY` path than the t0 byte's;
folding the arg5 address computation should shorten it by one link and flip the pair.

| probe | spelling | score | insns |
|---|---|---|---|
| f2a | `a5a = (idx_1494[1] << 2) + (s32)tbl_125c;` (fold, drop the `v0` staging) | 7 | 179 |
| f2c | fold but keep `v0` as the named intermediate | 7 | 179 |
| f2b | `arg5 = tbl_125c[idx_1494[1]];` fully inline | 8 | **180** |
| f2d | arg5 VALUE loaded inside wrap 1 | 9 | 179 |

f2a and f2c are BYTE-IDENTICAL to d01 (7/179) — the source-level fold is free, exactly as s66's
s04 measured for the analogous t0 fold, **and it does not move the 51/52 pair at all**. That
falsifies the mechanism: source-level folding does not shorten the RTL dependence chain, because
the chain's length is set by the RTL ops GCC must emit (`lbu` -> `sll` -> `addu` -> `lw` -> `sw`),
not by how many C statements they were written across. The only way to remove a link is to remove
an instruction, and f2b shows what that costs: 180 insns, i.e. off the 179-instruction parity the
whole basin depends on. The arg5 chain's path length is structurally fixed.

### Gates re-measured this session

- **Gate (a), canonical-asm.** `python3 tools/scan_hand_coded.py --single CD_ready` ->
  `tier=LOW score=2/8 (CD_ready, 179 insns) — no strong hand-coded indicators`. Only S4 (4 loads
  in an 8-insn window @ insn 51) and S5 (1 approx-sibling, CD_sync, jaccard 0.64) fire; S1
  multu-pacing, S2 empty-branch and S6 BIOS-jumptable are all NEGATIVE, as are S3/S7/S8.
  Artifact `tmp/grind/CD_ready/s68/scan_hand_coded.txt`. Fourth independent reproduction
  (s59, s104-era sibling, s67, s68). Provenance independently bars the claim: this body is
  identified PsyQ 3.5 libcd `CD_ready(int mode, u_char *result)`, i.e. compiler output.
- **Gate (b), SOTN-master precedent for the CLOSING construct.** There is no closing construct in
  hand: at floor 2 the residual is a two-instruction ALU transposition attributed to a sched2
  `INSN_LUID` tiebreak plus a `qty_compare_1` tie, and no C construct — sanctioned or otherwise —
  has been measured to move it. A census of `docs/reference/sotn-construct-index.md` for the
  shapes at play returns nothing usable: all 23 `volatile` hits are throwaway-pad locals
  (`pad`/`sp10`-style, e.g. `src/st/sel/stream.c:80`), none is an IRQ-status poll qualifier, and
  the index carries no class for statement-order-for-scheduling. Artifact
  `tmp/grind/CD_ready/s68/gate_b_census.txt`. A negative census is a FAILED gate, not an open
  question (AUTO-REJECT class, owner ruling 2026-08-24 reaffirmed 2026-08-31).

- [s68] CHASSIS re-measured this session, not inherited: memory/grind/CD_ready/candidate.c (the s60 vAT1 body) spliced over INCLUDE_ASM at src/system.c:379 -> sandbox CD_ready --disable all = score 2, target_insns 179, build_insns 179, rules_dropped 0, scorable true. src/system.c restored to INCLUDE_ASM and verified clean (git diff empty) after all measurement.

- [s68] OWNER DIRECTIVE DISCHARGED IN FULL. The 2026-09-01 FORECLOSED-BUCKET REVIEW Ruling A row `CD_ready (d4)` (decisions.md:17795) named two items: (i) 're-score the banked vAT1 form post--mel' - executed in s60, and it WORKED (floor 4 -> 2, build 178 -> 179); (ii) 'Ruling D session' - executed in full this session, both components failing. The review's premise-correction (the s50 volatile disqualification rested on a false premise; matched in-TU cdrom_IrqHandler/CD_flush declare these bytes volatile at src/system.c:549-551, :748-749, :770-771) is ACCEPTED and is already load-bearing in candidate.c's `volatile u8 *idx_1496` (strip the qualifier alone: 2 -> 4, 179 -> 178 insns).

- [s68] MECHANISM (new, general): local_alloc/block_alloc decides quantity membership from REG_BASIC_BLOCK - a pseudo is local iff ALL its references lie in one basic block - so C declaration scope cannot move a pseudo out of local-alloc. reg/v 74 (the s67 note's model of a 'cross-block' pseudo) is excluded because it is genuinely referenced in more than one block, not because it is declared at function scope. Moving a1v out requires a reference in a second basic block = at least one extra instruction, barred by 179-instruction parity. Measured, not inferred: the f1a function-scope dump is identical to d01's.

- [s68] MECHANISM (new, general): source-level folding of an address computation is byte-free on this compiler (f2a/f2c byte-identical to d01, consistent with s66's s04 t0-fold finding) but cannot shorten an RTL dependence chain - the chain length is set by the RTL ops GCC must emit (lbu -> sll -> addu -> lw -> sw), not by how many C statements they were written across. Removing a link costs an instruction (f2b = 180 insns).

- [s68] COUNTER-EVIDENCE to the s67 frontier's premise: hard reg 4 is in reg98's find_free_reg `used` set because $a0 is the printf call's first argument and conflicts with the 10-34 live range directly - NOT because the a1v quantity exists (a1v takes $a1 = 5 via the copy-suggestion path). The s67 frontier item was built on a false attribution.

- [s68] GATE (a) canonical-asm FAILS: python3 tools/scan_hand_coded.py --single CD_ready -> tier=LOW score=2/8 (179 insns), 'no strong hand-coded indicators'. Only S4 (4 loads in an 8-insn window @ insn 51) and S5 (1 approx-sibling CD_sync, jaccard 0.64) fire; S1 multu-pacing, S2 empty-branch and S6 BIOS-jumptable all negative, as are S3/S7/S8. Fourth independent reproduction. Independently barred by provenance: this body is identified PsyQ 3.5 libcd CD_ready(int mode, u_char *result), 179/179 masked-identical against the library object (memory/closer/libcd-groundtruth.md:40-52) - compiler output, not hand-written asm.

- [s68] GATE (b) SOTN precedent FAILS: there is no closing construct in hand to cite. The whole floor-2 residual is a transposition of two independent ALU insns in the do_timeout printf-argument block (build: sll $v0,$v0,2 / sll $a0,$a0,2 / addu $v0,$v0,$s5; target: sll $v0,$v0,2 / addu $v0,$v0,$s5 / sll $a0,$a0,2), attributed by dump reading (tmp/grind/CD_ready/dumps/system.sched2) to a sched2 INSN_PRIORITY tie falling through to INSN_LUID, coupled to a qty_compare_1 tie in local-alloc, with every measured C lever moving the two halves in opposite directions. A census of docs/reference/sotn-construct-index.md returns nothing usable: all 23 `volatile` hits are throwaway-pad locals (pad/sp10-style, e.g. src/st/sel/stream.c:80), none an IRQ-status poll qualifier, and the index carries no class for statement-order-for-scheduling. A negative census is a failed gate (AUTO-REJECT class, owner ruling 2026-08-24 reaffirmed 2026-08-31).

- [s68] EXHAUSTION: 68 sessions (plus pre-Grinder memory/wip/marionation_Exec/ and Closer-phase work). Floor flat at 2 since s60, flat at 4 for the 59 sessions before it. Modalities spent: recon, structural, permuter, forensics, rederive, synthesis, solver, escalation. 187 disproven forms banked in memory/grind/CD_ready/rejected/ (12 added this session).

- [s68] SIBLING CONSEQUENCE (a fact for CD_datasync's next session, not a disposition of it): the prong-(c) finding is symbol-level. The Ruling D scope grant cannot be spent by any of CD_sync / CD_ready / CD_datasync while CD_cw, getintr, func_800819C4, func_800817A0 and func_80081E1C remain assembly-only consumers of D_800A1494/95/96. This session re-derived that first-hand for CD_ready rather than inheriting CD_sync's s107 statement.

- [operator 2026-09-02] owner ruling 2026-09-02 (decisions.md 'foreclosure mechanics'): re-activated with the exhaustion window RESET — the 2026-09-01 Ruling-A unpark was re-foreclosed after one session because the window did not reset. The 09-01 named probe is spent (see ledger); work the ladder from its next rung. All standing banned_constructs remain in force. exhaustion_base=68

## s69 (forensics, 2026-09-03) — banked facts

- [s69] Chassis re-measured live before any probe: `sandbox CD_ready --disable all` on
  memory/grind/CD_ready/candidate.c = **score 2, build_insns 179, target_insns 179,
  rules_dropped 0**. The three s67 side bases also reproduce exactly: d01 = 7, y02 = 6, e02 = 8,
  all 179/0. Nothing in the inherited ledger has drifted.

- [s69] **GCC 2.7.2 schedules each basic block BACKWARD.** The `SCHEDDBG PICK clock=N` stream for
  CD_ready block 3 begins with the block's JUMP and ends with its first insn; the emitted order is
  the reverse of the pick order. Any future session reading a `.sched` dump on this project must
  apply that inversion — reading the pick stream forwards inverts every conclusion. Confirmed on
  both passes (`tmp/grind/CD_ready/s69/cand.sched.txt`, pass 1 at line 7218, pass 2 at line 8594).

- [s69] The block-3 insn identities are now pinned (candidate.c compile, cross-checked against the
  disassembly and against asm/funcs/CD_ready.s):
  `91` = `la $a0,D_800161B8`, `93` = `jal puts`, `99` = `lbu` of `idx_1494[0]` (t0 byte),
  `115` = `lbu` of `idx_1494[1]` (arg5 byte), `141` = `lw $a1,D_800F19C0`,
  `117` = `sll` of the arg5 index, `106` = **`sll $a0,$a0,2`, the t0 shift**,
  `120` = **`addu $v0,$v0,$s5`, the arg5 address add**, `122` = `lw` of the arg5 VALUE,
  `128` = `lbu D_800A11D5`, `111` = `addu $a0,$a0,$s5` (t0 address), `133` = `sll` of the
  D_800A11D5 byte, `137` = `sw $v1,0x10($sp)` (the 5th argument store), `143` = `lw $a2`,
  `145` = `lw $a3,0($a0)`, `139` = `la $a0,D_800161C8`, `147` = `jal printf`, `152` = `jal
  CD_flush`, `165`/`167` = the block tail.

- [s69] The floor's entire two-instruction residual is ONE comparison:
  `RANKDBG last=122 y=120 cls=3 x=106 cls2=3 val=0` — the class rung ties at 3/3, so
  sched.c:2464 falls through to `INSN_LUID(120) - INSN_LUID(106)`. On candidate.c that is
  12 - 6 = +6 in pass 1 and 7 - 6 = +1 in pass 2, both positive, so 120 is picked first and
  therefore EMITTED LAST — the transposition. Pass-2 LUIDs are simply the pass-1 output order,
  which is why the two passes cannot disagree.

- [s69] **The ordering half of the residual is solved on the candidate chassis.** Moving the two
  t0 statements (`t0 *= 4; t0 = (s32)((u8 *)tbl_125c + t0);`) to AFTER the arg5 load raises the
  t0 shift's LUID above the arg5 addu's and emits the target's exact instruction sequence for the
  whole block. Bodies: `memory/grind/CD_ready/progress/s69-g06-order-perfect-on-candidate-chassis-seats-swapped-6.c`
  (score 6, 179 insns, 0 rules) and the `pp`-unmoved twin g01 (also 6). This is an ordinary
  statement reorder — no new construct, no annotation needed for the reorder itself.

- [s69] **The seat half is a closed-form fixed point, and it is COUPLED to the ordering fix by
  construction.** local-alloc.c:1660 `qty_compare_1` scores
  `floor_log2(qty_n_refs) * qty_n_refs * qty_size / (qty_death - qty_birth)`, tie-broken by
  quantity number. Block 3 has four quantities. On candidate.c the t0-shift quantity spans 8
  (birth 16, death 24) giving pri 1.0000 — the lowest — so it is allocated LAST and receives
  `$a0`, the target's seat, while the arg5-value quantity (span 6, pri 1.3333) takes `$v1`.
  Fixing the order moves the t0 shift one insn later, shortening its span to 6; its pri becomes
  1.3333, an EXACT tie with the arg5 value, and `*q1 - *q2` gives ord 2 (and `$v1`) to the t0
  shift because it is born first — which the target's order itself guarantees. Dumps:
  `tmp/grind/CD_ready/s63/cand.qty.txt` vs `tmp/grind/CD_ready/s69/g06.qty.txt`.

- [s69] Consequence, stated as the search space: with the target's instruction sequence held,
  `qty_size` (1 for both), `qty_death - qty_birth` (6 for both) and the quantity numbers (birth
  order) are all pinned by that sequence. `qty_n_refs` is the ONLY free input, and it must move by
  at least one step: the t0 shift down to <= 3 refs, or the arg5 value up to >= 5.

- [s69] Loop notes cannot buy that reference count on the order-perfect base. Eight variants
  measured: the wrap-split family h01/h02/h03 = 12/12/15 and the nesting family j01/j02/j03/j04 =
  10/12/10/10, against g06's 6. The decisive one is the control **h04 (same split boundary,
  nothing left bare, every depth held at 2) = 12** — identical to h01, so the entire 6-point loss
  is the note pair itself and none of it is the depth change. The loop note is a sched1 region
  boundary in this block (s67-A), so any note that changes the depth also destroys the ordering
  the base exists to hold.

- [s69] Same-value re-stores are **refs-inert**, not merely byte-inert, on this function.
  `arg5 = arg5;` after the load (k01), before the call (k02) and `t0 = t0;` (k03) all score 6 and
  are byte-identical to g06; k01's block-3 local-alloc dump is line-for-line identical to g06's
  (`reg97 ... refs=4`). flow.c's `delete_noop_moves` runs before `reg_n_refs` is accumulated, so
  the extra mentions never reach the counter. This retires the sanctioned dead-store family as a
  refs lever on CD_ready.

- [s69] The one structurally-identified escape from the order/seat coupling, priced: if sched1
  emitted the CANDIDATE order (t0-shift span 8, correct seats) and sched2 then transposed the
  pair, both halves would hold at once. sched2 would need the class rung to break the tie, i.e.
  `insn_cost(120 -> 122) > 1`. Node 120 is `unit=-1 icost=1`; on this machine description only
  function-unit-0 insns (loads) carry icost 2, so the arg5 ADDRESS would have to be produced by a
  load. That is an extra instruction and breaks 179-parity. Recorded so no future session
  re-derives it.

- [s69] Artifacts: `tmp/grind/CD_ready/s69/cand.sched.txt` (1.27 MB, BB2_SCHED/RANK/PRIO over the
  whole TU), `g06.qty.txt`, `k01.qty.txt`, plus the generators `gen.py`/`gen2.py`/`gen3.py`/
  `gen4.py`, the dump driver `sched.sh`, the block extractor `ext.py` and the target-aligned
  disassembly differ `adiff.py`.

- [s69] Chassis re-measured live before any probe: candidate.c = score 2, build_insns 179, target_insns 179, rules_dropped 0; the s67 side bases also reproduce exactly (d01 = 7, y02 = 6, e02 = 8, all 179/0). No drift.

- [s69] GCC 2.7.2 schedules each basic block BACKWARD: the SCHEDDBG PICK stream starts at the block's JUMP and ends at its first insn, so the emitted order is the REVERSE of the pick order. Any future reading of a .sched dump on this project must apply that inversion.

- [s69] Block-3 insn identities are pinned: 99 = lbu idx_1494[0] (t0 byte), 115 = lbu idx_1494[1] (arg5 byte), 141 = lw $a1,D_800F19C0, 117 = sll of the arg5 index, 106 = sll $a0,$a0,2 (t0 shift), 120 = addu $v0,$v0,$s5 (arg5 address), 122 = lw of the arg5 VALUE, 128 = lbu D_800A11D5, 111 = addu $a0,$a0,$s5 (t0 address), 133 = sll of the D_800A11D5 byte, 137 = sw $v1,0x10($sp), 143 = lw $a2, 145 = lw $a3,0($a0), 147 = jal printf.

- [s69] The ordering half of the residual is SOLVED on the candidate chassis, not just on the s67 side bases: an ordinary statement reorder (t0 shift and t0 addu moved after the arg5 load) emits the target's exact instruction sequence for the whole do_timeout block, at 179 insns and 0 rules, with no new construct.

- [s69] The seat half is a closed-form fixed point coupled to that fix: with the target's order held, the t0-shift and arg5-value quantities both have qty_size 1, span 6 and qty_n_refs 4, hence bit-identical qty_compare_1 priority 13333, and the tie goes to the t0 shift on quantity number - which is birth order, which is the order itself.

- [s69] Search space after s69: of the four qty_compare_1 inputs, qty_size (1 for both), qty_death-qty_birth (6 for both) and the quantity number are all pinned by the target's instruction sequence. qty_n_refs is the only free input and must move by one step (t0 shift to <= 3 refs, or arg5 value to >= 5).

- [s69] Both known ways of moving qty_n_refs are now dead on the order-perfect base: loop notes cost >= 4 points of order damage (control h04 proves the loss is the note, not the depth), and same-value re-stores never reach the counter (delete_noop_moves).

- [s69] The one structurally-identified escape from the order/seat coupling, priced so nobody re-derives it: for sched1 to emit the CANDIDATE order (span 8, correct seats) while sched2 transposes the pair, sched2's class rung would have to break the tie, i.e. insn_cost(120 -> 122) > 1. Node 120 is unit=-1 icost=1; only function-unit-0 insns (loads) carry icost 2 on this machine description, so the arg5 address would have to be produced by a load - an extra instruction, off 179-parity.

- [s69] F3 is unchanged and still the only completion-blocker on this body: whether prong 2 of .claude/rules/legitimate-volatile-interrupt-touched.md admits the idx_1496 poll loop, and whether the shipped in-TU declarations constitute the volatile_extern_allowlist.txt grant. Moot while the floor is 2.

## s70 (rederive, 2026-09-03) - banked facts

- [s70] Chassis re-measured live before any probe: memory/grind/CD_ready/candidate.c spliced over
  the INCLUDE_ASM marker = **score 2, build_insns 179, target_insns 179, rules_dropped 0**; the
  s69 order-perfect base g06 reproduces exactly at **6, 179, 0**. No drift. Harness this session:
  `pwsh tmp/grind/CD_ready/s70/measure.ps1 <form.c>...` (splice -> sandbox -> restore, one call per
  batch), built on the s60 splice.py. src/system.c verified clean (`git status --porcelain`) after
  every batch.

- [s70] **MANDATORY KILL RE-AUDIT DISCHARGED.** s69's closest instance kill (k01, `arg5 = arg5;`
  on the order-perfect base, score 6) was re-measured on the CURRENT chassis across the FULL
  FAKE-ablation grid: `python3 tools/fake_ablate.py --func CD_ready --file system --candidate
  tmp/grind/CD_ready/s70/k01_renamed.c --max-variants 47` -> **47/47 variants, keep-all = 6/179 and
  drop-[arg5 self-store] = 6/179, byte-identical**; every other subset scores 7 or worse. No FAKE
  carrier was sitting on the pseudo the lever targets (the func_8002EA24-s8 failure mode does not
  apply here). s69's k01 kill STANDS on this chassis. Artifact:
  `tmp/grind/CD_ready/s70/ablate_k01.txt`. NOTE FOR FUTURE SESSIONS: fake_ablate.py requires the
  candidate's function to be literally named `CD_ready`; the Closer-era ledger forms are named
  `marionation_Exec` and the tool returns `ERR/None` for every row without saying why. Rename first
  with `tmp/grind/CD_ready/s70/rename.py <in.c> <out.c>`.

- [s70] **THE REDERIVE AXIS IS MEASURED AND DEAD, at both the whole-function and the block level.**
  The original source is IN HAND and has been for the whole grind: `tmp/closer/sotn_bios.c:260-286`
  is `CD_ready(int mode, u_char *result)` with `set_alarm` (:95), `get_alarm` (:102) and `callback`
  (:210) as the inlined helpers, and it maps onto BB2 one-for-one (Alarm.unk0/unk4/unk8 =
  D_800F19B8/BC/C0; Intr.sync/.ready/.c = D_800A1494/95/96; the command-name table D_80032AC8 =
  D_800A11DC indexed by CD_com = D_800A11D5; the status-name table D_80032B48 = D_800A125C; the
  three Result_t buffers D_80039260/68/70 = D_800F19A0/A8/B0). Transplanted faithfully:
  | form | shape | score | build insns |
  |---|---|---|---|
  | r1 | full source shape, direct globals, no FAKEs, no gotos | 57 | **174** |
  | r2 | same control flow, Intr triple via one base pointer, 1496 volatile | 55 | **178** |
  | r3/r3b | candidate chassis, both printf table args as inline subscripts | 14 | 179 |
  | r4 | t0 named, arg5 inline | 7 | 179 |
  | r5 | arg5 named, t0 inline | 14 | 179 |
  | r6 | both named (`t0 = tbl_125c[idx_1494[0]];` etc.), no shift/addu split | **7** | 179 |
  | r7 | both named, arg5 first | 8 | 179 |
  The natural whole-function shape is not a perturbation of the vAT1 basin - it is 5 (r1) or 1 (r2)
  instructions SHORT of the target's 179 and 55+ masked. At block level the best natural spelling
  is 7: with the `pp` pointer-alias gone the D_800F19C0 load sinks from emitted slot 53/54 to 61/62
  and the t0 value load rises to 57 (adiff over slots 51-67, r6 spliced). This is the first time in
  70 sessions the original source shape has been measured end-to-end rather than assumed.

- [s70] **flow.c's `reg_n_refs` accumulation is now ENUMERATED, not guessed** (the s69 frontier's
  first item asked exactly this). There are four textual accumulation sites in
  `tools/gcc-2.7.2/flow.c` - :2081 (mark_set_1, a SET), :2515 (mark_used_regs, a USE), :2329 and
  :2725 - but the last two are inside `#ifdef AUTO_INC_DEC`, and `config/mips/mips.h:2175-2179`
  leaves `HAVE_POST_INCREMENT` / `HAVE_PRE_INCREMENT` COMMENTED OUT, so neither is compiled on this
  target. The two live sites are reached only from `propagate_block`, which walks
  `PATTERN (insn)` (flow.c:1584) plus `CALL_INSN_FUNCTION_USAGE` (:1603, hard regs only) and
  `global_regs` (:1625). **REG_NOTES are never walked for reg_n_refs.** Consequences, all now
  facts rather than conjecture: a mention in a REG_EQUAL/REG_DEAD note is worth ZERO; a REG inside
  a MEM address IS counted, but only because mark_used_regs recurses through the pattern of an insn
  that exists; and an insn deleted before flow.c contributes nothing. The only instruction-free
  refs position left in the whole compiler is a SECOND occurrence of the same pseudo inside ONE
  surviving insn's pattern.

- [s70] **The sanctioned F1 combine-foldable copy chain-extender is refs-INERT here, proven in the
  dump.** `a5b = arg5;` (p01), a two-link chain (p02) and the mirror on the t0 address (p05) are
  all byte-identical to the g06 base at 6/179/0, and their block-3 local-alloc records are
  line-for-line identical to g06's (qty0 refs=4, qty1 reg104 refs=4, qty2 reg97 refs=4, qty3
  refs=8; same ALLOC order 3,0,1,2; same seats $v0/$v0/$v1/$a0). The copy pseudo is
  copy-propagated away before flow.c runs, so it never becomes a mention - a DIFFERENT mechanism
  from s69's k01-k03 self-stores (which die at delete_noop_moves) with the same outcome. Dumps:
  `tmp/grind/CD_ready/s70/p01_g06_a5copy1.qty.txt`, `p02_...qty.txt`, `p05_...qty.txt`.

- [s70] **NEW LEVER, POSITIVE: a do-while(0) whose note pair BRACKETS block 3 from outside raises
  every block-3 qty_n_refs by one loop_depth level at ZERO byte cost.** Base g06 (order-perfect,
  6/179). Form q02: a second `do {` opened above the `cnt` test and the `do_timeout:` label - i.e.
  its NOTE_INSN_LOOP_BEG is emitted in the PREDECESSOR basic block - closing after the existing
  in-block wrap. Measured 6/179/0, instruction sequence unchanged, and the quantity dump moves
  4/4/4/8 -> **6/6/6/12**. Every in-block note placement s69 measured cost 4-9 points
  (h01/h02/h03 = 12/12/15, control h04 = 12, j01-j04 = 10/12/10/10). This is the first placement in
  70 sessions that buys refs for free, and it decouples block 3's loop_depth from its instruction
  order. Banked as
  `memory/grind/CD_ready/progress/s70-q02-note-outside-block3-byte-neutral-refs-lever-6.c`.

- [s70] The variant of that lever WITHOUT the inner wrap (q01: relocate the existing wrap's BEG into
  the predecessor rather than nesting a new one) leaves block-3 refs UNCHANGED at 4/4/4/8 (score 6,
  179). Relocating the BEG does not raise the depth - `goto do_timeout` enters the region from
  outside, so the relocated pair does not survive as a depth-carrying loop. The lever requires the
  NESTED shape (outer bracket + the existing in-block wrap retained), which is why q02 works and
  q01 does not. q03 (BEG in predecessor, END moved past the `v0 = -1; goto check;` tail) also
  measures 6/179 with no refs change.

- [s70] **Why the free depth lever does not close the function BY ITSELF, stated as arithmetic.**
  `qty_compare_1` (tools/gcc-2.7.2/local-alloc.c:1659-1684) ranks on
  `floor_log2(qty_n_refs) * qty_n_refs * qty_size / (qty_death - qty_birth)` and, on a tie, on
  quantity number (:1683). The residual seat is qty1 (reg104, the t0 shift temp, 18-24) vs qty2
  (reg97, the arg5 value, 20-26): identical mention count (2), identical span (6), identical size
  (1). A UNIFORM depth change multiplies both qty_n_refs by the same factor, so pri1 == pri2 for
  every depth (4/4 -> 1.3333 each; 6/6 -> 2.0000 each; 8/8 -> 4.0000 each) and :1683 hands qty1 the
  $v1 seat on quantity number - which the target's own instruction order pins. Only an ASYMMETRIC
  change closes it: a third pattern mention for reg97, or a first-and-only one for reg104.

- [s70] Consequence for the frontier, stated precisely so no session re-derives it: with the
  target's instruction sequence held, `qty_size`, the span and the quantity numbers are pinned
  (s69), a uniform loop_depth is now free but tie-preserving (s70), REG_NOTES and deleted insns
  contribute nothing (s70), self-stores die at delete_noop_moves (s69) and copies die at
  copy-propagation (s70). The ONLY surviving shape for the needed asymmetry is an insn whose
  PATTERN mentions reg97 twice, or a note boundary lying strictly between reg104's set and its use
  (emitted slots 57 and 61) - and the latter is exactly the in-block placement s69 priced at 4-9
  points.

- [s70] Chassis re-measured live before any probe: candidate.c = score 2, build_insns 179, target_insns 179, rules_dropped 0; the s69 order-perfect base g06 = 6, 179, 0. No drift. src/system.c verified clean by git status --porcelain after every measurement batch.

- [s70] The ORIGINAL SOURCE for this function is in the repo and maps one-for-one onto BB2: tmp/closer/sotn_bios.c:260-286 is CD_ready(int mode, u_char *result), with set_alarm at :95, get_alarm at :102 and callback at :210 as the inlined helpers. Mapping: Alarm.unk0/unk4/unk8 = D_800F19B8/BC/C0; Intr.sync/.ready/.c = D_800A1494/95/96; command-name table D_80032AC8 = D_800A11DC indexed by CD_com = D_800A11D5; status-name table D_80032B48 = D_800A125C; the three Result_t buffers D_80039260/68/70 = D_800F19A0/A8/B0. The do_timeout block IS get_alarm's inlined failure path.

- [s70] Transplanting that source shape faithfully measures 57 at 174 build insns (r1, direct globals, zero FAKE constructs) and 55 at 178 (r2, Intr triple via one base pointer, 1496 volatile). Being SHORT of the target's 179 instructions is the decisive fact: the natural while(1) + short-circuit || control flow is a different basin, not a perturbation of the vAT1 one.

- [s70] At block level the natural spellings of the printf table arguments measure: both inline as subscripts = 14 (and identically 14 with the pp alias retained, so pp is irrelevant once subscripts are used), t0 named + arg5 inline = 7, arg5 named + t0 inline = 14, both named = 7, both named with arg5 first = 8. The floor form's staged decomposition is worth 5 points over the best natural spelling.

- [s70] flow.c's reg_n_refs accumulation is now enumerated rather than guessed: four textual sites (:2081, :2329, :2515, :2725), of which :2329 and :2725 are inside #ifdef AUTO_INC_DEC and config/mips/mips.h:2175-2179 leaves HAVE_POST_INCREMENT / HAVE_PRE_INCREMENT commented out. The two live sites are reached only from propagate_block, which walks PATTERN (insn) (flow.c:1584), CALL_INSN_FUNCTION_USAGE (:1603, hard regs only) and global_regs (:1625). REG_NOTES are never walked, so a mention in a REG_EQUAL/REG_DEAD note is worth zero.

- [s70] The only instruction-free refs position that survives that enumeration is a SECOND occurrence of the same pseudo inside ONE surviving insn's pattern. Everything else that mentions a pseudo either emits an instruction or is deleted before the counter runs.

- [s70] The sanctioned F1 combine-foldable copy chain-extender is refs-inert here and it is proven in the dump, not merely by score: p01/p02/p05 are byte-identical to the g06 base and their block-3 local-alloc records are line-for-line identical (qty0 refs=4, qty1 reg104 refs=4, qty2 reg97 refs=4, qty3 refs=8, ALLOC order 3,0,1,2). The copy pseudo is copy-propagated away before flow.c - a different mechanism from s69's self-stores (delete_noop_moves) with the same outcome.

- [s70] NEW POSITIVE LEVER: a do-while(0) whose note pair BRACKETS block 3 from outside - a second wrap opened above the cnt test and the do_timeout: label, NESTED over the existing in-block wrap - raises every block-3 qty_n_refs by one loop_depth level (4/4/4/8 -> 6/6/6/12) at ZERO byte cost, instruction sequence unchanged, score 6 == the g06 base. Every in-block placement s69 measured cost 4-9 points. This is the first free refs lever found in 70 sessions and it decouples block 3's loop_depth from its instruction order.

- [s70] The un-nested form of that lever does nothing: merely relocating the existing wrap's BEG into the predecessor (q01) or additionally moving its END past the tail (q03) leaves block-3 refs at 4/4/4/8, because `goto do_timeout` enters the region from outside and the relocated pair does not survive as a depth-carrying loop. The NESTED shape is load-bearing.

- [s70] The free depth lever cannot close the function alone, and the arithmetic says why: qty1 (reg104, t0 shift temp, 18-24) and qty2 (reg97, arg5 value, 20-26) have identical mention counts, spans and sizes, so a uniform loop_depth change multiplies both qty_n_refs equally and qty_compare_1's pri1 == pri2 at every depth; local-alloc.c:1683 breaks the tie on quantity number, which is birth order, which the target's own instruction sequence pins in qty1's favour.

- [s70] The residual is therefore reduced to a single named requirement: an ASYMMETRIC refs change on the order-perfect base - a third pattern mention for reg97, or a first-and-only one for reg104 - since qty_size, span and quantity number are pinned by the target's order (s69), uniform depth is free but tie-preserving (s70), REG_NOTES and deleted insns contribute nothing (s70), self-stores die at delete_noop_moves (s69) and copies die at copy-propagation (s70).

- [s70] Mandated kill re-audit discharged: s69's k01 self-store kill re-measured across the full 47-variant fake_ablate grid on the current chassis - keep-all and drop-[self-store] both 6/179 and byte-identical, every other subset >= 7. No FAKE carrier was masking the lever; the kill stands.

- [s70] Harness gotcha for future sessions: tools/fake_ablate.py requires the candidate's function to be literally named CD_ready and returns ERR/None for every row otherwise, without saying why. The Closer-era ledger forms are named marionation_Exec. Rename first with tmp/grind/CD_ready/s70/rename.py <in.c> <out.c>.

- [s70] Harness gotcha 2: the Windows python used for splicing defaults to cp1252, so any script that rewrites evidence.md/hypotheses.md must open BOTH read and write with encoding='utf-8' - a write without it truncates the file to zero bytes before failing. Caught and restored from git this session; the ledger appends are intact and re-verified by size.


## s71 (rederive, 2026-09-03) - banked facts

- [s71] Chassis re-measured live before any probe: `memory/grind/CD_ready/candidate.c` spliced over
  the INCLUDE_ASM marker = **score 2, build_insns 179, target_insns 179, rules_dropped 0**; the s69
  order-perfect base g06 = **6/179/0** and the s70 free-depth base q02 = **6/179/0**. No drift.
  Harness: `pwsh tmp/grind/CD_ready/s71/measure.ps1 <form.c>...` (splice -> sandbox -> restore) and
  `bash tmp/grind/CD_ready/s71/qtyrun.sh <form.c>...` for the instrumented-cc1 BB2_QTY_DEBUG block-3
  quantity records. `git status --porcelain src/system.c` verified clean after every batch.
  NOTE: qtyrun.sh must be run through WSL with the venv active; under Git-Bash the Windows python
  cannot spawn the splice/cc1 chain and dies with WinError 2.

- [s71] **MANDATED KILL RE-AUDIT DISCHARGED, and it kills the s70 frontier's second item.** s70
  proposed that its free outside-the-block depth lever (q02) would let s69's in-block loop notes
  become cheap, because "an in-block note no longer has to buy depth, only asymmetry". Measured
  directly: the printf-only wrap scores **10 on the g06 base and 10 on the q02 base** (d1g/d1q), and
  the nested printf wrap scores **10 on both** (d2g/d2q). The cost of an in-block note pair is
  therefore independent of the enclosing depth - exactly what s69's h04 control implied - and
  stacking it on q02 buys nothing. The re-priced placements are banked in
  `rejected/s71-d1-*`, `s71-d1q-*`, `s71-d2-*`.

- [s71] **THE ASYMMETRIC reg_n_refs THE RESIDUAL NEEDS IS REACHABLE - AND IT IS SELF-CANCELLING.**
  This is the first measurement in 71 sessions of a block-3 state with qty_n_refs(arg5 value) >
  qty_n_refs(t0 shift temp). BB2_QTY_DEBUG, block 3, on the g06 order-perfect base:
  | form | qty1 reg104 (t0 shift) | qty2 reg97 (arg5 value) | pri1 | pri2 | score |
  |---|---|---|---|---|---|
  | g06 (base) | birth 18 death 24 refs 4 | birth 20 death 26 refs 4 | 1.3333 | 1.3333 | 6 |
  | d1 (printf wrapped once) | 16-20 refs **4** | 18-26 refs **5** | **2.0000** | 1.2500 | 10 |
  | d2 (printf wrapped twice) | 16-20 refs **4** | 18-26 refs **6** | **2.0000** | 1.5000 | 10 |
  The wrap raises ONLY the arg5 value's death mention (its death insn is the one inside the note
  region) - the asymmetry is real. But the same note is a sched1 region boundary, and the resulting
  re-order shortens the t0-shift quantity's span from 6 to 4 while stretching the arg5 value's from
  6 to 8. `qty_compare_1` (local-alloc.c:1659-1684) divides by that span, so the span damage
  outruns the refs gain in both directions and qty1 keeps the `$v1` seat (got=3) with qty2 taking
  `$a0` (got=4). Refs asymmetry and span damage are the SAME event on this block, not two levers.
  Dumps: `tmp/grind/CD_ready/s71/d1g.qty.txt`, `d1q.qty.txt`, `d2g.qty.txt`.

- [s71] **The qty1/qty2 pair is INVARIANT at (span 6, refs 4, size 1) across every byte-neutral
  respelling of the printf-argument block measured this session.** Six structurally different C
  shapes on the g06 order-perfect base all reproduce block 3's quantity table row-for-row
  (qty0 16-20 refs4 -> hard 2, qty1 18-24 refs4 -> hard 3, qty2 20-26 refs4 -> hard 4,
  qty3 22-30 refs8 -> hard 2; ALLOC order 3,0,1,2), differing only in pseudo NUMBERS:
    * `b1` - a dead prefix store `arg5 = v0;` before the load (score 6)
    * `c1` - the t0 shift folded into the address expression (score 6)
    * `g1` - a named `s32 *ap` pointer for the arg5 address, `arg5 = *ap;` (score 6)
    * `g6` - the shift written `(t0 << 2)` inside the address expression (score 6)
    * `g7` - the `v0` index staging replaced by a FRESH local `ai` (score 6); this one also moves
      qty0 to birth 10 refs 12 (the fresh local is born at the lbu) and re-orders ALLOC to
      0,3,1,2 - and the qty1/qty2 rows are still identical.
  The pointer shapes that are NOT byte-neutral are `g3` (named `s32 *tp` for the t0 address, 9),
  `g4` (both pointers named, 9) and `g5` (ONE pointer variable reused for both addresses, 16).
  Conclusion for the ledger: once the target's instruction sequence is held, respelling the block's
  expressions does not move any qty_compare_1 input. All banked in `rejected/s71-*`.

- [s71] **KILLED: carrying the arg5 value in an already-multiply-set variable.** Writing the load
  into `v0` (`v0 = *(s32 *)(v0 + (s32)tbl_125c);` with `v0` passed as printf's 5th argument, a1)
  DOES do what the mechanism predicts - the block-3 quantity table drops from four quantities to
  three and reg97 disappears entirely, so the qty_compare_1 tie ceases to exist and the value is
  seated by global-alloc instead. It measures **12/179/0**: the extra dependence on `v0` (which
  carries the arg5 INDEX two statements earlier) serialises the block and the emission order is
  lost. Staging the t0 side through `v0` instead is the same story: e1 (both t0 statements) 12,
  e2 (the add only) 13, e3 (the shift only) 10. Dump: `tmp/grind/CD_ready/s71/a1g.qty.txt`.

- [s71] **KILLED: loop notes on the FLOOR body itself (candidate.c, score 2).** s67's six-position
  empty-wrap sweep was run against a score-6 side base, never against the floor. Measured this
  session on candidate.c: wrapping the t0 chain (n1) = 12 at **181 build insns**, wrapping the arg5
  chain (n2) = 8 at **181**, wrapping the printf (n3) = 10 at 179, wrapping the t0 chain with the
  arg5 chain left bare (n4) = 12 at 181, the mirror (n5) = 8 at 181. Four of the five leave the
  179-instruction basin outright; none improves on 2.

- [s71] **NEWLY READ PREDICATE - the local-alloc SUGGESTION PASS, which bypasses the tie entirely.**
  `block_alloc` allocates in TWO passes. The first (local-alloc.c:1507-1526) walks the quantities
  ordered by `qty_sugg_compare_1` and seats every quantity for which `qty_phys_num_sugg[q] != 0 ||
  qty_phys_num_copy_sugg[q] != 0`, BEFORE `qty_compare_1` is ever consulted (local-alloc.c:1560,
  :1569). A quantity carrying a hard-register suggestion therefore never reaches the
  1.3333-vs-1.3333 tie that has held this function for ten sessions. Suggestions are created in
  exactly one place, `combine_regs` (local-alloc.c:1856-1885), reached from local-alloc.c:1281-1295
  for ANY insn with an `=` output operand 0 and a REG input operand - not only for copy insns -
  when one of the two is a HARD register. On CD_ready block 3 all four quantities print
  `copysugg=` EMPTY, and the reason is structural for the two that matter: reg97's set is a load
  whose operand 1 is a MEM (constraint `m`, so the PLUS/MULT address reduction at
  local-alloc.c:1276-1284 never fires and r1 is not a REG), and reg97's death is a store into
  printf's 5th-argument stack slot whose operand 0 is a MEM, not a register. Neither of the arg5
  value's two insns can hand it a suggestion as the block is currently spelled. This is the first
  pass-level route around the tie identified since s65 and it is UNPROBED - see the frontier.

- [s71] Artifacts: `tmp/grind/CD_ready/s71/` - measure.ps1, qtyrun.sh, qtyext.py, gen.py/gen2.py/
  gen3.py/gen4.py (variant generators), and the BB2_QTY_DEBUG dumps a1g.qty.txt, b1g.qty.txt,
  c1g.qty.txt, d1g.qty.txt, d1q.qty.txt, d2g.qty.txt, g1.qty.txt, g6.qty.txt, g7.qty.txt.

- [s71] [s71] Chassis re-measured live before any probe: memory/grind/CD_ready/candidate.c = score 2, build_insns 179, target_insns 179, rules_dropped 0; the s69 order-perfect base g06 = 6/179/0; the s70 free-depth base q02 = 6/179/0. No drift. src/system.c verified clean by git status --porcelain after every measurement batch.

- [s71] [s71] The mandated kill re-audit is discharged on the closest-to-target instance kill (s69's in-block loop note) and it also kills the s70 frontier's second item: the printf-only wrap costs 10 on the g06 base and 10 on the q02 free-depth base, and the nested version costs 10 on both. In-block note cost is independent of the enclosing loop depth.

- [s71] [s71] FIRST ASYMMETRIC reg_n_refs EVER MEASURED ON THIS FUNCTION: a do-while(0) around the printf statement alone gives the arg5-value quantity refs 5 (nested: 6) against the t0-shift quantity's 4, exactly the asymmetry s70 named as the residual's only remaining requirement.

- [s71] [s71] That asymmetry is self-cancelling. The same note is a sched1 region boundary; the re-order it causes moves the t0-shift quantity's span from 6 to 4 and the arg5-value quantity's from 6 to 8, and qty_compare_1 divides by span, so pri1 = 2.0000 beats pri2 = 1.2500 (single wrap) or 1.5000 (nested) and the seats do not move. On this block the refs gain and the span damage are one event.

- [s71] [s71] Carrying the arg5 value in an already-multiply-set variable (v0) really does delete its pseudo from local-alloc - block 3 drops from four quantities to three and reg97 disappears - but it measures 12 on both bases because v0 already carries the arg5 index two statements earlier and reusing it serialises the block. The t0-side mirrors through v0 measure 12 / 13 / 10.

- [s71] [s71] INVARIANCE: with the target's instruction sequence held, six byte-neutral respellings of the printf-argument block (dead prefix store, shift folded into the address expression, named address pointer, inline `(t0 << 2)`, a fresh index local replacing the v0 staging) all score 6 and reproduce block 3's quantity table row for row, differing only in pseudo numbers. Respelling the block does not move any qty_compare_1 input.

- [s71] [s71] Loop notes are dead on the FLOOR body itself, not just on the side bases: five placements on candidate.c score 12/8/10/12/8, and four of the five leave the 179-instruction basin (181 build insns).

- [s71] [s71] NEW PASS-LEVEL FRONTIER read from the compiler: block_alloc seats every quantity carrying a hard-register suggestion in a separate pass (local-alloc.c:1507-1526) BEFORE qty_compare_1 is consulted (local-alloc.c:1560-1569). Suggestions come only from combine_regs (local-alloc.c:1856-1885), reached for any insn with an `=` output operand 0 and a REG input operand when one of the two is a hard register. All four block-3 quantities print copysugg= EMPTY; reg97's set is a load with a MEM operand 1 and its death is a store with a MEM operand 0, so as currently spelled neither insn can give it one.

- [s71] [s71] Harness note for future sessions: tmp/grind/CD_ready/s71/qtyrun.sh must be run through WSL with the venv active. Invoked from Git-Bash the Windows python cannot spawn the splice/cc1 chain and dies with WinError 2 inside splice.py, with no useful message.

## s72 (structural, 2026-09-03) — measured facts

- [s72] Baselines re-verified live on the HEAD chassis BEFORE any probe: `memory/grind/CD_ready/candidate.c`
  = score 2, build 179, target 179, rules_dropped 0; `progress/s69-g06-order-perfect-on-candidate-chassis-seats-swapped-6.c`
  = 6/179/0. Harness: `tmp/grind/CD_ready/s72/measure.ps1` (splice + cheat-invisible sandbox),
  `tmp/grind/CD_ready/s72/qtyrun.sh` (BB2_QTY_DEBUG/BB2_SUGG_DEBUG block-3 extraction),
  `tmp/grind/CD_ready/s72/prio.sh` (BB2_PRIO_DEBUG/BB2_RANK_DEBUG, new this session).
- [s72] The g06 residual, read instruction-for-instruction (tmp/grind/CD_ready/s61/show.py 44 70):
  the emitted SEQUENCE is the target's for the whole block; the six differing instructions are a
  pure two-register swap. The t0 address chain (`lbu` 51, `sll` 57, `addu` 61, `lw $a3` 67) is in
  $v1 where the target uses $a0, and the arg5 value (`lw` 58, `sw 0x10($sp)` 63) is in $a0 where the
  target uses $v1. Nothing else in 179 instructions differs.
- [s72] The floor body's block-3 local-alloc quantity table, recorded for the first time:
  qty0 reg102 birth 16 death 24 refs 4 size 1 -> got $a0; qty1 reg104 18-20 refs 4 -> $v0;
  qty2 reg97 20-26 refs 4 -> $v1; qty3 reg110 22-30 refs 8 -> $v0; ALLOC order 1, 3, 2, 0.
  Against g06's 16-20 / 18-24 / 20-26 / 22-30 with ALLOC order 3, 0, 1, 2. The ONLY structural
  difference is the t0-chain quantity's span, 8 on the floor body and 6 on the order-perfect base.
  At span 8 its qty_compare_1 priority is 1.0000 and it is allocated last (taking $a0, correct);
  at span 6 it is 1.3333, an exact tie with the arg5 value, broken on quantity number in its favour
  (taking $v1, wrong).
- [s72] FIRST non-empty `copysugg` on this function in 72 sessions. Naming printf's Nth argument in
  a fresh local makes GCC compute the value into a pseudo and then emit `move $aN, pseudo`;
  combine_regs (local-alloc.c:1856-1885) records that as a copy suggestion and local-alloc.c:1507-1526
  seats the quantity in the suggestion pass, ahead of qty_compare_1 entirely. Measured:
  4th argument -> `copysugg=7,` ($a3), got 7 (tmp/grind/CD_ready/s72/gj.qty.txt, score 11);
  2nd argument -> `copysugg=5,` ($a1), got 5 (q2.qty.txt, score 10);
  3rd argument -> `copysugg=6,` ($a2), got 6 (q3.qty.txt, score 16).
- [s72] The suggestion pass cannot reach either contested quantity. The only hard registers in
  block 3 are printf's $a0-$a3; a C spelling can create a suggestion of $a1, $a2 or $a3 (argument
  values that pass through a pseudo), but the target seats the two contested values at $a0 and $v1.
  $a0 in this block carries only `la $a0, D_800161C8` — a constant address that never occupies a
  pseudo — and $v1 is not an argument register. When the $a3 suggestion IS created for the t0 value
  it propagates back up the whole chain (`lbu $a3,0($s2)` … `lw $a3,0($a3)`) and the block re-orders.
- [s72] qty_size is not reachable byte-neutrally: `long long arg5` (with `(s32)arg5` at the call)
  builds 181 instructions against the target's 179 and scores 19. All four qty_compare_1 inputs are
  now measured on this function: refs, span, quantity number, size.
- [s72] sched2 INSN_PRIORITY, read from tmp/grind/CD_ready/s72/cand.prio.txt: insns 106, 111, 117,
  120, 122 and 128 ALL carry final_pri=2. sched.c:1497 computes
  `priority(pred) + insn_cost - 1`, so an ALU-to-ALU edge contributes zero and only a load-latency
  edge (cost 2) increments. Each chain has exactly one load above the contested pair
  (`insn=106 pred=99 cost=2`, `insn=117 pred=115 cost=2`). Breaking the priority rung needs a second
  load above the arg5 chain — i.e. loading `tbl_125c` inside block 3 rather than inheriting it live
  in $s5 — which costs an instruction.
- [s72] local-alloc.c:1176 reads `if (GET_CODE (insn) != NOTE) insn_number++;`. Loop notes therefore
  do not advance insn_number and cannot change a quantity's birth/death/span directly. Every span
  change measured in s67-s71 came from sched1 re-ordering caused by the note acting as a region
  boundary — which is the structural reason the note lever has always paid the order cost.
- [s72] Chain-combining raises priority, it does not lower it. For a k-insn single-assignment chain
  combined into one quantity, qty_n_refs = 2*k*depth and the span = 2*k, so
  pri = floor_log2(2*k*depth) * depth — monotonically increasing in k. Measured: gm1's combined
  quantity is refs 12 / span 14 (pri 2.5714) against the split form's refs 4 / span 6 (pri 1.3333).
- [s72] Byte-neutral respelling invariance extended (24 measurements): operand commutation, u32
  type narrowing, declaration-order swap, plain-integer address arithmetic, `&arr[i]` address
  spelling and `[0]` deref spelling are all byte-identical on both bases. Commuting the arg5
  address expression is the only respelling that changes bytes at all, and it changes exactly one:
  the emitted `addu` operands commute with it.

- [s72] Baselines re-verified live on the HEAD chassis before any probe: memory/grind/CD_ready/candidate.c = 2/179/0 and progress/s69-g06-order-perfect-on-candidate-chassis-seats-swapped-6.c = 6/179/0.

- [s72] The g06 residual read instruction-for-instruction: the emitted sequence is the target's for the whole block and the six differing instructions are a pure two-register swap - the t0 address chain (lbu 51, sll 57, addu 61, lw $a3 67) sits in $v1 where the target uses $a0, and the arg5 value (lw 58, sw 0x10($sp) 63) sits in $a0 where the target uses $v1.

- [s72] First non-empty copysugg on CD_ready in 72 sessions: naming printf's 2nd/3rd/4th argument in a fresh local yields copysugg=$a1/$a2/$a3 respectively and seats that quantity in the local-alloc suggestion pass ahead of qty_compare_1.

- [s72] The suggestion pass cannot reach either contested quantity: the only hard registers in block 3 are printf's $a0-$a3; $a0 carries only a constant address that never occupies a pseudo and $v1 is not an argument register, and those two are exactly the seats the target needs.

- [s72] All four qty_compare_1 inputs are now measured on this function - refs, span, quantity number and size. qty_size is unreachable at 179 insns (DImode carrier builds 181).

- [s72] sched2 INSN_PRIORITY for insns 106, 111, 117, 120, 122 and 128 is uniformly 2; sched.c:1497's 'insn_cost - 1' makes ALU-to-ALU edges contribute zero, so only a second load above the arg5 chain could break the priority rung, and that costs an instruction.

- [s72] Chain-combining raises qty_compare_1 priority rather than lowering it: for a k-insn combined chain refs = 2*k*depth and span = 2*k, so pri = floor_log2(2*k*depth)*depth is monotonically increasing in k (measured: refs 12 / span 14 = 2.5714 versus refs 4 / span 6 = 1.3333).

- [s72] local-alloc.c:1176 does not advance insn_number over NOTEs, so a do-while(0) can never move a quantity's span directly; every span change s67-s71 observed came from the note acting as a sched1 region boundary.

- [s72] Byte-neutral respelling invariance extended by 24 measurements: operand commutation, u32 type narrowing, declaration-order swap, plain-integer address arithmetic, &arr[i] and [0] spellings are all byte-identical on both bases; commuting the arg5 address expression is the only respelling that changes bytes, and it changes exactly one instruction.

## s73 (structural, 2026-09-03) - measured facts

- [s73] Baselines re-verified live on the HEAD chassis BEFORE any probe:
  `memory/grind/CD_ready/candidate.c` = score 2, build 179, target 179, rules_dropped 0;
  `progress/s69-g06-order-perfect-on-candidate-chassis-seats-swapped-6.c` = 6/179/0.
  `git status --porcelain src/system.c` verified clean after every batch. Harness copied into
  `tmp/grind/CD_ready/s73/` (measure.ps1, dumpq.sh, qtyext.py, gen.py/gen2.py/gen3.py).

- [s73] THE FLOOR BODY'S RESIDUAL IS EXACTLY TWO ADJACENT INSTRUCTIONS, read off the disassembly
  at slot granularity: build slots 56/57 are `sll $a0,$a0,2` then `addu $v0,$v0,$s5`, and the
  target is `addu $v0,$v0,$s5` then `sll $a0,$a0,2`. EVERY register in the block is already the
  target's on the floor body - t0 chain $a0, arg5 address $v0, arg5 value $v1, a2 chain $v0. The
  whole 179-instruction function differs from the target in that one adjacent transposition and
  nothing else.

- [s73] rank_for_schedule's CLASS rung - the only rung between INSN_PRIORITY (killed s72) and
  INSN_LUID - is structurally unavailable to this pair, and the predicate is one line.
  `tools/gcc-2.7.2/sched.c:2429` reads
  `if (link == 0 || insn_cost (tmp, link, last_scheduled_insn) == 1) tmp_class = 3;`
  and `insn_cost` (sched.c:1372-1380) returns `result_ready_cost` OF THE CANDIDATE INSN, which is
  1 for every single-cycle MIPS ALU insn. The target's two contested insns are an `addu` and an
  `sll`, both cost 1, so both candidates are class 3 on every call regardless of what is scheduled
  around them, and the rung always falls through to `INSN_LUID` at sched.c:2464. The s72 RANKDBG
  dump records the decision directly: `RANKDBG last=122 y=120 cls=3 x=106 cls2=3 val=0`
  (tmp/grind/CD_ready/s72/cand.prio.txt:1770 and :2088). With s72's priority kill this closes the
  ENTIRE rank_for_schedule ladder above INSN_LUID for this pair - the emission order of slots
  56/57 is decided by RTL order alone.

- [s73] The block-3 quantity table maps 1:1 onto emitted slots once each lui/%lo pair is counted as
  a single RTL insn. Block 3's RTL insn list is
  0 `la $a0,D_800161B8` / 1 call puts / 2 `lbu` t0 byte / 3 `lbu` arg5 index / 4 `lw $a1,D_800F19C0`
  / 5 `sll` arg5 index / 6 `addu` arg5 addr / 7 `sll` t0 / 8 `lw` arg5 value / 9 `lbu` D_800A11D5 /
  10 `addu` t0 addr / 11 `sll` a2 index / 12 `sw 0x10($sp)` / 13 `lw $a2` / 14 `lw $a3,0(t0)` /
  15 `la $a0,D_800161C8` / 16 call printf, and local-alloc's birth/death are twice that index.
  The t0-shift quantity is therefore literally [t0 `sll` -> t0 `addu`]: the floor order puts the
  `sll` at index 6 and the `addu` at 10 (span 8, pri 1.0000, allocated last, $a0 - correct); the
  target order puts the `sll` at 7 (span 6, pri 1.3333, an exact tie with the arg5 value, won on
  quantity number, $v1 - wrong). The `addu` slot is pinned by the target, so on the target's order
  the span cannot be anything but 6 and the refs anything but 4.

- [s73] find_free_reg's conflict sets on the g06 order-perfect base, read for the first time
  (tmp/grind/CD_ready/s73/g06.qty.txt, SUGGDBG-FFR lines): qty3 used={0,1,5,...} -> $v0;
  qty0 used={0,1,5,...} -> $v0; qty1 (t0) used={0,1,2,5,...} -> $v1; qty2 (arg5)
  used={0,1,2,3,5,...} -> $a0. Hard reg 5 ($a1) is in every set because the `pp` alias places
  printf's $a1 argument load early, exactly as the target does. Nothing but the allocation ORDER
  of qty1/qty2 decides the two contested seats - both colourings are feasible - and the s67 worry
  that $a0 might be in the t0 quantity's used set does NOT hold on this base.

- [s73] **CONFIRMED, A NEW AND WORKING MECHANISM (the s72 frontier's item 1): the t0 chain can be
  removed from local-alloc entirely by carrying it in a function-scope variable that is also
  referenced in another basic block.** `block_alloc` creates quantities only for pseudos whose
  whole life is inside the block (local-alloc.c:1170-1180); a multi-block pseudo is
  REG_BLOCK_GLOBAL and falls through to global-alloc. Measured on the g06 base with `status` as
  the carrier (r6): block 3 drops from four local quantities to THREE -
  `SUGGDBG-QTY blk=3 qty=0 reg1=101 birth=18 death=20`, `qty=1 reg1=97 birth=20 death=26`,
  `qty=2 reg1=109 birth=22 death=30` - the t0-chain quantity is gone, the 1.3333-vs-1.3333
  qty_compare_1 tie ceases to exist, and reg97 (the arg5 VALUE) is allocated
  `QTYDBG blk=3 ord=2 qty=1 reg1=97 birth=20 death=26 refs=4 got=3` = **$v1, the target's seat**.
  This is the first time in 73 sessions that the arg5 value reaches $v1 on a body derived from the
  order-perfect base without a loop note, and the arg5 half is then byte-exact against the target
  (`addu $v0,$v0,$s5` / `lw $v1,0($v0)` / `sw $v1,0x10($sp)`).
  Dump: tmp/grind/CD_ready/s73/r6.qty.txt.

- [s73] **KILLED on the same battery: the globalized carrier never reaches $a0, and the emitted
  order collapses to the non-target one.** Twenty variants on the g06 base across carriers `i`,
  `cnt`, `status`, `src`, `dst`, `dst2` and three chain positions (shift-only, address-only,
  whole-chain), plus the arg5-value mirror. Best = 5 (`status`, shift-only r4 and whole-chain r6)
  against the floor of 2. The disassembly says why: `status`, whose other live range crosses the
  `getintr` calls, forces a call-saved seat and the chain emits in $s0; `cnt` is call-free but
  lands the chain in $a3 and drags cnt's own block-1 uses from $v1 to $a3 for four extra diffs;
  `i`, `src`, `dst`, `dst2` damage their own use sites. Scores (all build 179): r1 i-shift 24,
  r2 i-fused 9, r3 cnt-shift 9, r4 status-shift **5**, r5 i-whole 24, r6 status-whole **5**,
  r8 cnt-whole 9, r9 arg5-value-globalized 20, a_cnt 11, a_i 26, a_st 7, a_dst 12, a_dst2 13,
  a_src 22, w_dst 10, w_dst2 11, w_src 20, sa_cnt 15, sa_i 30, sa_st 11.

- [s73] **THE DECISIVE CONTROL: once the t0 chain is globalized the ORDER degree of freedom
  disappears - the order-perfect base and the floor body converge to the SAME code.** The battery
  was re-generated on `candidate.c`'s statement order (t0 chain first) instead of g06's (t0 chain
  after the arg5 load): f_st 5, f_cnt 9, f_i 24, f_dst 10, f_dst2 11, f_src 20, fs_st 5,
  fs_cnt 9 - score-for-score identical to their g06-based twins r6/r8/r5/w_dst/w_dst2/w_src/r4/r3.
  Moving the t0 statements, the ONLY lever that has ever produced the target's instruction
  sequence, is completely inert once the carrier is multi-block, and all twenty forms emit slots
  56/57 in the floor order. Globalization buys the arg5 seat and spends the order lever to do it;
  it is not a partial win a later session can finish by re-tuning statement order.

- [s73] `f_sv` (the `saved` u8 carrier) is the one form that leaves the 179-instruction basin
  (build 181, score 12) - a u8 carrier costs the two extra insns of the byte round-trip.

- [s73] Harness note for future sessions: `bash tools/wsl.sh ...` works from the Bash tool but NOT
  from the PowerShell tool on this machine (`wsl: command not found`, exit 127), while
  `tools/wteng.ps1` only works from PowerShell. A splice -> sandbox -> objdump-compare cycle
  therefore has to alternate between the two tools; there is no single-tool form.

- [s73] MANDATED KILL RE-AUDIT DISCHARGED on the closest-to-target form (candidate.c itself, the
  floor body): the full 38-variant `tools/fake_ablate.py` grid re-run on the CURRENT chassis
  (tmp/grind/CD_ready/s73/ablate.log). keep-all reproduces 2/179 exactly; every other subset is
  worse (7, 11, 12, 13 ... 63) with one apparent exception, drop-00010000 (`v0 <<= 2;` removed),
  which scores 2 at build 178 - it is one instruction SHORT of the target because removing the
  shift changes the arg5 address semantically, so it is not a lever. No inherited FAKE carrier is
  masking anything on this body; the FAKE set is minimal and load-bearing, and every s72/s73
  instance kill measured with it in place stands.
  Harness gotcha re-confirmed (s70's note): fake_ablate returns ERR for every row unless the
  candidate's function is literally named `CD_ready` - rename with tmp/grind/CD_ready/s70/rename.py
  first - and its default --max-variants 24 silently refuses this body's 8-unit grid; pass
  --max-variants 40.

- [s73] Chassis re-verified live before any probe: memory/grind/CD_ready/candidate.c = 2/179/0 and progress/s69-g06-order-perfect-on-candidate-chassis-seats-swapped-6.c = 6/179/0; src/system.c verified clean by git status --porcelain after every batch.

- [s73] The floor body's residual is EXACTLY the two adjacent instructions at build slots 56/57 - sll $a0,$a0,2 then addu $v0,$v0,$s5 where the target has addu $v0,$v0,$s5 then sll $a0,$a0,2 - and every register in block 3 is already the target's.

- [s73] rank_for_schedule's class rung is closed as a class at sched.c:2429: insn_cost returns the candidate's own result_ready_cost, 1 for any single-cycle ALU insn, so an addu and an sll are both class 3 on every call and the rung always falls through to INSN_LUID.

- [s73] Block 3's RTL insn list maps 1:1 onto emitted slots once each lui/%lo pair is counted as one insn; the t0-shift quantity is literally [t0 sll -> t0 addu], span 8 in the floor order (pri 1.0000, allocated last, $a0) and span 6 in the target order (pri 1.3333, tie, $v1).

- [s73] find_free_reg's conflict sets on g06, read for the first time: qty1 (t0) used={0,1,2,5,...} -> $v1 and qty2 (arg5) used={0,1,2,3,5,...} -> $a0. $a0 is NOT in the t0 quantity's used set, so both colourings are feasible and only the allocation order decides - retiring the s67 open question.

- [s73] CONFIRMED MECHANISM: a multi-block carrier removes the t0 chain from local-alloc (block 3 drops to three quantities) and seats the arg5 value at $v1 uncontested - the first arg5-value-in-$v1 on an order-perfect-derived body with no loop note in 73 sessions.

- [s73] KILLED: the globalized carrier lands in $s0 (status, call-crossing) or $a3 (cnt) and never $a0; best score 5 against the floor's 2, across 28 measurements.

- [s73] DECISIVE CONTROL: once the carrier is multi-block the statement-order lever is inert - eight forms generated on the floor body's statement order score identically to their g06-based twins (5/9/24/10/11/20/5/9), and all twenty forms emit slots 56/57 in the floor order.

- [s73] Mandated kill re-audit discharged: the full 38-row fake_ablate grid on the floor body reproduces keep-all at 2/179 and every other subset is worse; no FAKE carrier is masking a lever.

- [s73] Harness: bash tools/wsl.sh works from the Bash tool but not from the PowerShell tool (wsl: command not found, exit 127), while tools/wteng.ps1 only works from PowerShell - a splice/sandbox/objdump cycle must alternate tools.

## s74 (synthesis, 2026-09-03) - measured facts

- [s74] Chassis re-verified live BEFORE any probe, four bases in one batch:
  `memory/grind/CD_ready/candidate.c` = score 2 / build 179 / target 179 / rules 0;
  `progress/s69-g06-order-perfect-on-candidate-chassis-seats-swapped-6.c` = 6/179/0;
  `progress/s73-r6-multiblock-carrier-arg5-value-takes-v1-uncontested-5.c` = 5/179/0;
  `progress/s67-y02-seats-correct-base.c` = 6/179/0. No drift from the s73 numbers.
  `git status --porcelain src/system.c` clean after every batch (splice harness restores).

- [s74] MANDATED KILL RE-AUDIT discharged on the CLOSEST-TO-TARGET instance kill that had never
  been ablated - s73's globalized-carrier form r6 (score 5), whose kill statement was "once the
  carrier is multi-block the statement-order lever goes inert". Full 38-row `tools/fake_ablate.py`
  grid on the CURRENT chassis (`tmp/grind/CD_ready/s74/abl.sh`, log in the session transcript):
  keep-all reproduces 5/179 exactly and NO subset of the 8 inherited FAKE units improves on it
  (next best is drop-00010000 at 5/178, one instruction short because removing `v0 <<= 2;` changes
  the arg5 address semantically - the same non-lever s73 recorded on the floor body; every other
  subset is 10 or worse, up to 61 for drop-all). In particular drop-10000000 (removing the
  sanctioned tbl_125c do-while(0) around block 3) costs 8 points on r6, so the wrap is load-bearing
  on the globalized body too. **No inherited FAKE carrier was masking the order lever on r6; the
  s73 kill stands as measured.**

- [s74] THE sched1 DEPENDENCE GRAPH AND PICK STREAM FOR BLOCK 3 ARE NOW READ OUT IN FULL
  (tmp/grind/CD_ready/s69/cand.sched.txt, floor body, pass 1 block 3 at line 7218). The pass-1
  PICK stream reversed = sched1's OUTPUT order, and it is
  `91 93 99 115 141 117 106 120 122 128 111 137 133 143 145 139 147 152 165 167`
  i.e. index 2 `lbu` t0 byte, 3 `lbu` arg5 index, 4 `lw *pp`, 5 `sll` arg5 index, 6 `sll` t0,
  7 `addu` arg5 addr, 8 `lw` arg5 value, 9 `lbu` D_800A11D5, 10 `addu` t0, 11 `sw 0x10($sp)`,
  12 `sll` a2 index, 13 `lw $a2`, 14 `lw $a3`. That is EXACTLY the emitted slot order, so on this
  function sched1's output order and sched2's emission order coincide instruction-for-instruction
  in block 3 - the premise the s69-s73 closed form rests on ("every qty_compare_1 input except
  refs is pinned by the target's instruction sequence") is now measured, not assumed.
  Priorities in the same dump: insn 137 (the `sw` of arg5) carries pri=3, insns 106/111/117/120/
  122/133 carry pri=2, 99/115/128/139/141 carry pri=1 - correcting the s72 shorthand that "all six
  are 2" (137 is 3, and it is the arg5-value quantity's DEATH).

- [s74] BLOCK 3's PSEUDO MAP READ OFF THE .lreg DUMP FOR THE FIRST TIME
  (tmp/grind/CD_ready/s74/g06dump/system.lreg, g06 base). `reg/v 98` is the C variable `t0`,
  `reg/v 74` is `v0`, `reg/v 81` is `tbl_125c`. The chain is
  `insn 102: reg98 = idx_1494[0]` / `insn 117: reg104 = reg98 << 2` (REG_DEAD reg98) /
  `insn 122: reg98 = reg104 + reg81` (REG_DEAD reg104) / `insn 145: $a3 = mem(reg98)`.
  So the contested "t0 shift temp" is **reg104, a compiler-created intermediate between two SETS
  of the user variable reg98**, and reg98 itself has two sets/two deaths inside the block, which
  is why local-alloc skips it and global-alloc seats it at `$a0` (the s66 reading, now confirmed
  from RTL). The arg5 value is `reg97` (`insn 113: reg97 = mem(reg102)` -> `insn 137: sw`).

- [s74] **KILLED (instance): moving the multi-death boundary one link DOWN the t0 chain.** The
  synthesis hypothesis was that if the ADDRESS link is given a fresh single-death local
  (`taddr = (s32)((u8 *)tbl_125c + t0);`) then reg98 keeps its two sets (still global-allocated,
  still `$a0`), the shift result lands in reg98 rather than in a contested local quantity, and the
  arg5 value is left uncontested at `$v1` - i.e. the s73 r6 outcome WITHOUT globalizing across
  basic blocks and therefore without spending the statement-order lever. Ten variants measured,
  all at build 179 / rules 0: on the g06 order m1 (fresh `s32 taddr`) 9, m2 (pointer-typed
  `s32 *taddr`) 9, m3 (`t0 = t0 << 2` spelling) 8, m6 (BOTH links fresh) 9; on the floor order
  n0 9, n1 (address statement moved after the arg5 load) 9, n4 (arg5 load moved between the two
  links) 9. **The result is 9 on every statement order tried** - the axis is order-invariant,
  exactly like s73's multi-block carriers, and it is 7 points worse than the floor.

- [s74] THE MECHANISM, read from global-alloc (`tmp/grind/CD_ready/s74/n0.greg`): once the address
  link is a separate local, the t0 variable's allocno joins the 18-allocno global list and its
  conflict set CONTAINS hard reg 4 - `;; 118 conflicts: 72 73 76 77 78 81 82 83 118 4 5 29` - so
  `$a0` is excluded outright and it is seated at `$v0` (`118 in 2`). This unifies with s73: EVERY
  route that pushes the t0 chain out of local-alloc - multi-block carrier (s73: `$s0` / `$a3`) or
  multi-death boundary shift (s74: `$v0`) - loses `$a0`, because the value's live range then
  conflicts with the `$a0` uses around it instead of being seated last by local-alloc.

- [s74] **NEW AND UNEXPLORED RUNG, read from the same dump: global.c keeps HARD-REGISTER
  PREFERENCES and this function has live ones.** The n0 `.greg` header prints
  `;; 74 preferences: 2` and `;; 86 preferences: 5` - the `v0` variable prefers `$v0` because its
  value arrives as a function return value, and reg86 prefers `$a1` because it is copied into
  printf's second argument. Preferences are consulted by `find_reg` BEFORE the plain
  lowest-free-hard-reg scan, which is the direct answer to the s73 frontier's open mechanical
  question ("why did the call-free `cnt` carrier land `$a3` rather than `$a0`?"): it had neither a
  preference for 4 nor a free 4. No session in 74 has attempted to CREATE a `$a0` preference for
  the t0 chain. This is the first pass-level lever identified above local-alloc's qty_compare_1
  since s71's suggestion pass.

- [s74] Two controls worth banking. (a) `m5` - putting the SHIFT (rather than the address) in a
  fresh local on the g06 base is byte-IDENTICAL to g06 (6/179/0), confirming the s71/s72
  canonicalisation invariance extends to this respelling. (b) `n5` - adding an unused
  `s32 taddr;` declaration to the floor body and changing nothing else scores 2/179/0, i.e. an
  unreferenced scalar local is completely inert here (it contributes no mention to flow.c and no
  quantity to local-alloc), so declaration count is not a lever on this function.
  (c) `n3` - the s69 `g05` geometry (arg5 address split out and hoisted above the t0 shift)
  re-measured on the FLOOR body instead of the g06 base scores 7, against 15 on g06.

- [s74] Chassis re-verified live before any probe, four bases in one batch: candidate.c 2/179/0, progress/s69-g06-order-perfect-on-candidate-chassis-seats-swapped-6.c 6/179/0, progress/s73-r6-multiblock-carrier-arg5-value-takes-v1-uncontested-5.c 5/179/0, progress/s67-y02-seats-correct-base.c 6/179/0. No drift from s73. src/system.c verified clean by git status --porcelain after every batch.

- [s74] The mandated kill re-audit was run on r6 (score 5), the closest-to-target instance kill that had never been ablated; s73 had ablated only the floor body. Keep-all reproduces 5/179 and no FAKE subset improves it, so the s73 order-lever kill is not carrier-masked.

- [s74] sched1's block-3 output order for the floor body, read from the pass-1 PICK stream in reverse, is instruction-for-instruction the emitted order. This is the first direct measurement of the premise the whole s69-s73 closed form rests on, and it holds.

- [s74] The .lreg pseudo map for block 3 is now on record: reg/v 98 = the C variable t0 (two sets, two deaths, skipped by local-alloc, seated $a0 by global-alloc), reg104 = the contested shift intermediate between those two sets, reg97 = the arg5 value, reg/v 74 = v0, reg/v 81 = tbl_125c.

- [s74] Insn 137 (the arg5 sw, the arg5-value quantity's death) has sched priority 3, not 2 - a correction to the s72 shorthand that all six insns in the contested window carry final_pri=2.

- [s74] KILLED at 9, order-invariantly: putting the t0 ADDRESS link in a fresh single-death local (m1/m2/m3/m6 on g06, n0/n1/n4 on the floor order). Control m5 (the SHIFT in a fresh local) is byte-identical to g06, and control n5 (an unused scalar declaration) is byte-identical to the floor.

- [s74] Unifying mechanism across s73 and s74: three distinct routes push the t0 chain out of local-alloc - multi-block carrier crossing calls ($s0), call-free multi-block carrier ($a3), multi-death boundary shift ($v0) - and none of them reaches $a0, because global-alloc computes the conflict set over the whole live range and the surrounding $a0 uses land in it (n0.greg: ';; 118 conflicts: ... 4 5 29').

- [s74] The s69 g05 geometry (arg5 address split out and hoisted above the t0 shift) re-measured on the FLOOR body scores 7, against 15 on the g06 base - the geometry is base-sensitive and the floor-body number was not previously on record.

- [s74] NEW: global.c hard-reg preferences are live on CD_ready (';; 74 preferences: 2', ';; 86 preferences: 5'). No session in 74 has attempted to create an $a0 preference for the t0 chain; this is the first pass-level rung identified above local-alloc's qty_compare_1 since s71's suggestion pass.

## s75 (synthesis, 2026-09-03) - measured facts

- [s75] Chassis re-verified live BEFORE any probe: `memory/grind/CD_ready/candidate.c` = score 2 /
  build 179 / target 179 / rules 0; `progress/s69-g06-order-perfect-on-candidate-chassis-seats-swapped-6.c`
  = 6/179/0. No drift from s72-s74. Harness: `tmp/grind/CD_ready/s75/measure.ps1` (splice + sandbox),
  `tmp/grind/CD_ready/s75/qtyrun.sh` (BB2_QTY_DEBUG/BB2_SUGG_DEBUG block-3 extraction), generators
  gen.py/gen2.py/gen3.py/gen5.py/gen6.py. 35 forms measured, all at build 179 / rules_dropped 0.

- [s75] THE PARITY THEOREM FOR THE REFS LEVER (derived first, then confirmed by measurement).
  In the TARGET's block-3 emission order the two tied quantities' mentions interleave
  t0, arg5, t0, arg5: index 7 = t0 `sll`, 8 = arg5 `lw`, 10 = t0 `addu`, 12 = arg5 `sw`.
  A `do {} while (0)` raises loop_depth - and so `reg_n_refs`, and so `qty_n_refs` - uniformly for
  every mention inside its note pair, so a wrap breaks the 1.3333-vs-1.3333 tie only if its region
  contains an ODD number of one quantity's mentions and an EVEN number of the other's. Enumerating
  every contiguous region of the target order, exactly four qualify in the arg5 value's favour -
  [8..9], [8..12], [11..12] and [12..12]; [7..10] favours the t0 temp (the wrong direction); every
  other region is symmetric and cannot move the tie at all. [8..9] and [8..12] place the note
  BETWEEN the two `t0` statements; [11..12] and [12..12] place it between the arg5 load and the
  arg5 store, i.e. around the call's own argument setup. This enumeration is what makes the s66
  wrap sweep (twelve placements, best 8) interpretable: it never separated the two cases.

- [s75] MEASURED CONSEQUENCE OF THE [8..9]/[8..8] HALF - a note placed between the two `t0`
  statements COLLAPSES the t0 chain. p1 (wrap = `{ arg5 = *a5a; a2i = D_800A11D5; }`, the exact
  [8..9] geometry) = 11; p2 (`{ arg5 = *a5a; }`, [8..8]) = 12; p3 ([8..10]) = 11; p4 (double-wrapped
  [8..9]) = 11; p6 (same with the t0 shift first) = 11. The QTYDBG table
  (tmp/grind/CD_ready/s75/p1_wrap_arg5_and_a2idx.qty.txt) says why: block 3 stops having a t0 SHIFT
  TEMP at all. `reg98` - the `t0` variable, normally two-sets/two-deaths and therefore skipped by
  local-alloc - becomes ONE local quantity, birth 10 death 32 refs 12, priority 1.636, and it beats
  the arg5 value outright. This reproduces AND explains s66's "the t0 chain collapses into one
  merged quantity" for the load-only wraps: cutting the RTL between the two sets of reg98 with a
  loop note changes reg98's death count and hence its eligibility for local-alloc.

- [s75] **CONFIRMED - THE SEAT HALF OF THE RESIDUAL IS SOLVED BY A PURE `reg_n_refs` LEVER. This is
  the first time in 75 sessions that BOTH contested seats come out at the target's registers on a
  body derived from the order-perfect base, with no globalized carrier and no loss of the order.**
  Geometry: the [11..12] region - a wrap containing ONLY the `debug_printf(...)` statement, with the
  a2 index (`a2i = D_800A11D5;`) and the t0 address (`t0 = (s32)((u8 *)tbl_125c + t0);`) hoisted
  above it. Depth arithmetic, measured exactly as predicted: base depth is 2 (the `goto loop`
  back-edge plus the sanctioned tbl_125c wrap), the arg5 `lw` stays outside at depth 2 and the arg5
  `sw` moves inside at depth 2+n, so arg5 refs = 4+n while the t0 shift temp stays at 4. QTYDBG
  confirms n=1 -> refs 5, n=2 -> refs 6, n=3 -> refs 7. The wrap also stretches the arg5 value's
  span from 6 to 10, so its priority is `floor_log2(4+n)*(4+n)/10` against the t0 temp's 8/6 =
  1.3333: n=1 gives 1.000 (loses), n=2 gives 1.200 (loses), n=3 gives **1.400 (WINS)**. The scores
  follow the arithmetic exactly - q1 (n=1) 10, q5 (n=2) 10, q9 (n=3) **5**, q10 (n=4) 5,
  q11 (n=5) 5. On q9 the ALLOC record flips:
  `ord=2 qty=2 reg1=97 refs=7 got=3` (the arg5 value at **$v1**, the target's seat) and
  `ord=3 qty=1 reg1=105 refs=4 got=4` (the t0 shift temp at **$a0**, the target's seat).
  Dump: tmp/grind/CD_ready/s75/q9_wrap_call_nest3.qty.txt.

- [s75] **NEW BEST NON-FLOOR BASE: `r3`, score 5, whose block-3 emission order is the target's
  instruction-for-instruction from slot 53 through slot 67.** r3 = q9 plus the `*pp` load hoisted
  out of the wrap into a named `a1v` - which is exactly what s66's w01 needed and never had: the
  note otherwise displaces the D_800F19C0 expand_call argument load from slots 53/54 down to 61/62.
  Saved as `progress/s75-r3-nested-note-refs-flip-seats-block3-order-exact-5.c` (and the no-hoist
  variant as `progress/s75-q9-nested-note-refs-flip-seats-no-pp-hoist-5.c`). r3's ENTIRE residual is
  five instructions with two causes:
    (a) the two leading `lbu`s are transposed - build emits `lbu $v0,1($s2)` then `lbu $a3,0($s2)`,
        the target `lbu $a0,0($s2)` then `lbu $v0,1($s2)` (slots 51/52);
    (b) `reg98` (the `t0` variable, global-allocated) is seated at **$a3** instead of $a0, which
        costs slots 57 (`sll $a0,$a3,2` vs `sll $a0,$a0,2`), 61 (`addu $a3,$a0,$s5`) and
        67 (`lw $a3,0($a3)`).
  Everything else - the whole arg5 chain, the D_800A11D5 chain, the `sw 0x10($sp)`, the a2 load and
  both `la` pairs - is byte-exact against the target.

- [s75] THE MECHANISM OF RESIDUAL (b), READ FROM `.greg` (tmp/grind/CD_ready/s75/r3.greg, the
  block near line 2876): `;; 98 preferences: 7` and
  `;; 98 conflicts: 72 73 74 76 77 78 81 98 2 3 4 5 6 29`, with `98 in 7` in the dispositions.
  Hard reg 4 ($a0) is IN reg98's conflict set on this body - local-alloc has already seated the t0
  shift temp there (`106 in 4`) and the two now overlap - so `find_reg` cannot give reg98 $a0 at
  all, and its preference for hard reg 7 (recorded from the `lw $a3, 0(reg98)` argument load) takes
  it to $a3. On the floor body reg98 and the shift temp SHARE $a0. This is the s74 frontier's
  hard-register-preference rung, now observed live with a concrete symbol and a concrete number,
  and it shows the two halves are coupled: winning the local-alloc seat for the shift temp is what
  puts $a0 into reg98's conflict set.

- [s75] KILLED (instance): moving the note boundary one slot earlier so that the arg5 LOAD is also
  inside the wrap (the w/x/y series - the wrap opens immediately before `arg5 = *(s32 *)a5a;` and
  closes after the call). Both arg5 mentions then sit at depth 2+n and the t0 `addu` at 2+n with the
  t0 `sll` outside, so arg5 refs = 2*(2+n) against the t0 temp's 4+n - a larger asymmetry that
  arrives already at n=1. QTYDBG on w1 confirms arg5 refs 6 span 8 -> `got=3` ($v1, correct) but the
  t0 shift temp now measures refs 5 span 8 and `got=6` ($a2, wrong): it is allocated AFTER the arg5
  value and $a0 is no longer free for it. Scores: w1 5, w2 5, w3 14, w4 19, x1 5, x2 5, x3 14,
  x4 19, y1 5, y2 5. This boundary buys the arg5 seat and spends the shift temp's $a0.

- [s75] KILLED (instance): region-1 statement order is INERT on the r3 base. Twelve permutations of
  the pre-wrap statements (t0 byte load, pp alias, v0 byte load, v0 shift, arg5 address, t0 shift,
  arg5 load, a2 index, t0 address, pp deref) that respect the data dependences all score exactly 5 -
  u01 v0-first, u02 t0-byte-late, u03 pp-load-early, u04, u05, u06, u08, u09, u10, u11, u12 - the
  single exception being u07 (the a2 index moved above the arg5 load) at 15. Neither the `lbu`
  transposition nor reg98's $a3 seat responds to statement order.

- [s75] KILLED (instance): three respellings of the t0 chain on the r3 base aimed at removing the
  `preferences: 7` - z1 (the 4th printf argument named in a local hoisted above the wrap) 11,
  z2 (pointer-typed address `s32 *t0p`) 7, z3 (t0 folded to a single expression
  `t0 = idx_1494[0] * 4;`, one set instead of two) 7. All worse than r3's 5.

- [s75] CONTROLS. p5 (the a5a split + a2i naming + the t0 shift moved after the a5a statement, NO
  wrap) = 6/179/0 and its quantity table is g06's row for row (qty0 reg99 16-20 refs4, qty1 reg105
  18-24 refs4 -> $v1, qty2 reg97 20-26 refs4 -> $a0, qty3 reg110 22-30 refs8) - i.e. the geometry r3
  is built on is byte-identical to the order-perfect base before the note is added, so every
  difference r3 shows is attributable to the note alone. q6 (the call wrap with the a2 index left
  inline) = 10, identical to q1, so naming the a2 index is byte-neutral. q2/q3/q4 (nest 1 with the
  a1v, a2v and a3v hoists) = 7 / 20 / 14.

- [s75] `qty_compare`, used for blocks with next_qty <= 3 (local-alloc.c:1540-1560), has NO
  quantity-number tie-break, unlike `qty_compare_1` (local-alloc.c:1683) used by the qsort path.
  Checked and it is NOT a lever here: on an all-tie input the hand-rolled 3-element sort leaves
  qty_order at the identity permutation, which is exactly what the number tie-break produces, so
  reducing block 3 to three quantities cannot flip the contested pair by that route.

- [s75] Nested `do {} while (0)` levels beyond the minimum are measurably load-bearing on this
  function and their effect saturates: on the r3 geometry n=1 and n=2 both score worse than n=3,
  and n=3, n=4, n=5, n=6 all score 5 with only reg97's refs (7, 8, 9, 10) differing. The
  single-level-insufficient justification the do-while-zero rule requires is therefore a MEASURED
  fact here, not an assertion: n=1 gives priority 1.000 and n=2 gives 1.200, both below the t0
  shift temp's 1.3333.

- [s75] src/system.c restored to its committed INCLUDE_ASM state after every batch
  (`git status --porcelain src/system.c` clean). candidate.c is UNCHANGED as a body and remains the
  floor at 2/179/0; only its header comment gained the s75 summary. Twelve new rejected forms
  banked (253 total) and two new progress bases saved.

- [s75] Chassis re-verified live before any probe: memory/grind/CD_ready/candidate.c = 2/179/0 and progress/s69-g06-order-perfect-on-candidate-chassis-seats-swapped-6.c = 6/179/0; no drift from s72-s74. src/system.c verified clean by git status --porcelain after every batch, and candidate.c re-scored at 2/179/0 after its header edit.

- [s75] PARITY THEOREM, derived then measured: in the target's block-3 order the contested mentions interleave t0 sll (7), arg5 lw (8), t0 addu (10), arg5 sw (12), so a do-while(0) region breaks the qty_compare_1 tie only with an odd/even mention split. Exactly four contiguous regions favour the arg5 value ([8..9], [8..12], [11..12], [12..12]); [7..10] favours the t0 temp; every other region is symmetric and inert.

- [s75] The [8..9]/[8..8] family is dead for a structural reason now on record: a note between the two t0 statements changes reg98's death count, so the t0 VARIABLE enters local-alloc as one birth-10 death-32 refs-12 quantity (pri 1.636) instead of leaving the shift temp contested. Scores 11/12/11/11/11. This root-causes s66's 'the t0 chain collapses into one merged quantity'.

- [s75] CONFIRMED, first in 75 sessions: the [11..12] geometry (wrap only the debug_printf statement, a2 index and t0 address hoisted above it) at nesting depth 3 seats the arg5 value at $v1 and the t0 shift temp at $a0 - both target seats - by refs alone. Measured refs 5/6/7 for n=1/2/3 and priorities 1.000/1.200/1.400 against the t0 temp's 1.3333; scores 10/10/5/5/5 for n=1..5.

- [s75] NEW BEST NON-FLOOR BASE r3 = 5/179/0 (progress/s75-r3-nested-note-refs-flip-seats-block3-order-exact-5.c): block 3's emission order is the target's instruction-for-instruction from slot 53 to slot 67, the entire arg5 chain is byte-exact, and the five remaining diffs are the transposed leading lbu pair (51/52) plus reg98 at $a3 (57/61/67).

- [s75] The mechanism of r3's remaining seat diff, read from .greg: ';; 98 preferences: 7' and ';; 98 conflicts: 72 73 74 76 77 78 81 98 2 3 4 5 6 29', '98 in 7'. Hard reg 4 is in reg98's conflict set BECAUSE local-alloc seated the shift temp there ('106 in 4'); on the floor body the two share $a0. This is the s74 frontier's hard-reg-preference rung observed live, and it shows the two halves of the residual are coupled through that one conflict.

- [s75] Nested do-while(0) depth is measurably load-bearing and saturating here, which supplies the single-level-insufficient justification the rule requires as a MEASURED fact: n=1 gives priority 1.000 and n=2 gives 1.200, both below the t0 shift temp's 1.3333, while n=3..6 all score 5 with only reg97's refs differing (7, 8, 9, 10).

- [s75] Region-1 statement order is inert on r3 (twelve permutations all exactly 5, u07 alone at 15), as are three t0-chain respellings (z1 11, z2 7, z3 7). The w/x/y boundary family wins the arg5 seat at n=1 but spends the shift temp's $a0 (w1 5, w3 14, w4 19).

- [s75] Controls: p5 (the a5a split + a2i naming + t0 shift after the a5a statement, no wrap) is 6/179/0 with g06's quantity table row for row, so every difference r3 shows is attributable to the note alone; q6 (a2 index left inline) is 10, identical to q1, so naming the a2 index is byte-neutral.

- [s75] qty_compare (local-alloc.c:1540-1560, the next_qty<=3 path) has no quantity-number tie-break unlike qty_compare_1 (local-alloc.c:1683), but the hand-rolled exchange sequence leaves qty_order at the identity permutation on ties, so it is not a lever.

- [s75] Twelve new rejected forms banked (253 total) and two new progress bases saved (r3 and the no-pp-hoist q9). candidate.c is unchanged as a body and remains the floor at 2/179/0; only its header gained the s75 summary.

## s76 (synthesis, 2026-09-03) - measured facts

- [s76] Chassis re-verified live BEFORE any probe: `memory/grind/CD_ready/candidate.c` = score 2 /
  build 179 / target 179 / rules 0; `progress/s75-r3-nested-note-refs-flip-seats-block3-order-exact-5.c`
  = 5/179/0. No drift from s72-s75. Harness: `tmp/grind/CD_ready/s76/one.ps1` (splice + sandbox +
  leave the .o for a diff), `tmp/grind/CD_ready/s76/sched.sh` + `run_sched.sh` (instrumented cc1 with
  BB2_SCHED_DEBUG, per-variant block-3 extraction). 13 forms measured, all at build 179 /
  rules_dropped 0. `src/system.c` verified clean by `git status --porcelain` after every batch.

- [s76] **THE FLOOR BODY'S ENTIRE RESIDUAL, WRITTEN OUT INSTRUCTION BY INSTRUCTION** (first time it
  is on record as a full objdump-vs-target listing rather than a summary). Target block-3 slots
  51-67 hold two structurally identical four-insn chains, B = the `*(s32 *)t0` printf argument and
  A = the `arg5` stack argument:
    51 `lbu $a0,0($s2)` [B]   52 `lbu $v0,1($s2)` [A]   53/54 `la/lw $a1,D_800F19C0`
    55 `sll $v0,$v0,2` [A]    56 `addu $v0,$v0,$s5` [A] 57 `sll $a0,$a0,2` [B]
    58 `lw $v1,0($v0)` [A]    59/60 `lui/lbu $v0,D_800A11D5`  61 `addu $a0,$a0,$s5` [B]
    62 `sll $v0,$v0,2`        63 `sw $v1,0x10($sp)` [A]  64/65/66 `lui/addu/lw $a2,D_800A11DC`
    67 `lw $a3,0($a0)` [B]
  The floor body emits every one of these with the TARGET'S REGISTERS and differs only by
  transposing slots 56 and 57 (`sll $a0,$a0,2` before `addu $v0,$v0,$s5`). Both seats are correct;
  the residual is one adjacent swap of two independent ALU insns.

- [s76] **NEW PASS-LEVEL RUNG, NEVER EXAMINED IN 75 SESSIONS: sched.c's `adjust_priority` /
  `birthing_insn_p`.** Read from the block-3 BB2_SCHED_DEBUG dump of the floor body
  (tmp/grind/CD_ready/s76/candidate.sched.txt, pass=1 block=3): the contested pair is insn 106
  (the B shift, luid 6) against insn 120 (the A addu, luid 12), and at clock 13 the ready list is
  `[ 120(p=2130706433,l=12) 106(p=2130706433,l=6) 141(p=1,l=22) ]`. Their nominal priorities are
  both 2; the value 2130706433 is `max_priority`, applied by `adjust_priority`
  (tools/gcc-2.7.2/sched.c:2571-2590) to any ready insn with zero REG_DEAD notes whose pattern is
  `birthing`. `birthing_insn_p` (sched.c:2504-2528) returns `reg_n_sets[REGNO (SET_DEST (pat))] == 1`.
  So the s69 record ("the class rung ties 3/3 and sched.c falls through to INSN_LUID") is correct
  but incomplete: the pair only REACHES the LUID rung because BOTH insns are boosted to
  max_priority, and the boost is C-controllable - it disappears the moment the destination pseudo
  has more than one set in the function.

- [s76] **CONFIRMED, and this is the session's real result: de-boosting insn 120 reproduces the
  TARGET'S EXACT block-3 emission order with NO loop note at all.** Form b2
  (`status = v0 + (s32)tbl_125c; arg5 = *(s32 *)status;`, i.e. the arg5 address carried in an
  existing multi-set local). Its dump (tmp/grind/CD_ready/s76/b2.sched.txt) shows
  `ADJPRI insn=120 deaths=0 birth=0` where the floor body shows `birth=1`, and the pick stream
  flips exactly as predicted - `PICK clock=13 picked=106`, `PICK clock=14 picked=120` against the
  floor's 13:120 / 14:106 - giving the emission sequence 99, 115, 142, 117, 120, 106, 123, 129, 111
  = B.lbu, A.lbu, `lw $a1`, A.sll, A.addu, B.sll, A.lw, lbu, B.addu, the target's order
  instruction for instruction. This is the SECOND independent route to the order-perfect emission
  (the first is s69's g06, which reorders the C statements) and the first that needs neither a
  statement reorder nor a `do {} while (0)`.
  Saved as `progress/s76-b2-birth-deboost-target-block3-order-no-loop-note-12.c` (score 12).

- [s76] KILLED (instance): every C spelling of the de-boost measured this session pays more than
  the order is worth, because the only carriers that survive to flow.c with `reg_n_sets > 1` are
  variables that ALSO leave block 3, so the arg5-address pseudo stops being a local quantity and
  global-alloc seats it in a callee-saved or argument register. Scores: a1 (address into `v0`,
  `v0 = v0 + (s32)tbl_125c`) 15; a3 (same with `v0 +=`) 15; b1 (`cnt`) 16; b2 (`status`) 12;
  b3 (`i`) 29; c1 (`arg5` holds its own address, which de-boosts the lw as well) 9; c4 (a block-local
  `a5a` replacing the whole `v0` staging, 3 sets) 6; c6 (`a5a` with two REAL sets, the t0 deref
  staged early into `t0`) 11; c9 (same with a fresh `t0v`) 10. b2's diff shows the cost precisely:
  `addu $s0,$v0,$s5` and `lw $a0,0($s0)` - the address in $s0 instead of $v0.

- [s76] CONTROLS, both byte-identical to the floor at 2/179/0: c3 (`a5a = v0; a5a = a5a + (s32)tbl_125c;`)
  and c5 (`a5a = (s32)tbl_125c; a5a = v0 + a5a;`). A first set that is a plain COPY is propagated
  away before flow.c runs, so `reg_n_sets` returns to 1 and the boost survives. This is the exact
  mirror of s69's k01 result (`delete_noop_moves` runs before `reg_n_refs` is accumulated): a
  de-boost carrier must have two sets that both SURVIVE to flow.c, which is why every working
  spelling this session had to route a real value through the carrier.

- [s76] Control a5 (the B shift routed through `cnt`, i.e. de-boosting insn 106 instead of 120)
  scores 8. With both insns unboosted the comparison returns to INSN_LUID, which still favours
  insn 120 (luid 12 > luid 6), so the order does not flip - confirming the attribution: it is
  120's boost specifically, not "a boost", that pins the transposition.

- [s76] THE COUPLING, now stated exactly. The target's block-3 order needs B.lbu BEFORE A.lbu
  (slots 51/52) and A.addu BEFORE B.sll (slots 56/57). Both pairs are decided by INSN_LUID on the
  floor body, and LUID is the RTL stream order, so a spelling that reverses the two chains
  wholesale flips BOTH pairs and can never satisfy the target. The floor body satisfies the lbu
  pair and fails the ALU pair; the s75 r3 base satisfies the ALU pair and fails the lbu pair
  (its dump shows the lbu order there is decided by unit BLOCKAGE, not by LUID:
  `BLOCKAGE unit=0 clock=19 ... exec=106 last=99`). The two routes that satisfy BOTH - s69's g06
  (interleaved statement order) and s76's b2 (birth de-boost) - do so by splitting chain B, and
  both then lose on the local-alloc seats.

- [s76] The mandated KILL RE-AUDIT could not be executed with `tools/fake_ablate.py` on the
  closest-to-target non-floor form: the r3 body's 11 FAKE units include three nested
  `do {} while (0)` wraps and two staging lines whose removal is not semantics-preserving, and
  67 of the 68 grid variants fail to build (`ERR None`), the 68th scoring 179. Dump:
  tmp/grind/CD_ready/s76/r3_ablate.txt. Both closest-to-target forms were instead re-measured LIVE
  on the current chassis at the top of the session (candidate.c 2/179/0, r3 5/179/0), and the
  s73/s74 ablation grids on candidate.c and r6 remain the closest executable evidence.

- [s76] Chassis re-verified live before any probe: candidate.c = 2/179/0, progress/s75-r3-nested-note-refs-flip-seats-block3-order-exact-5.c = 5/179/0. No drift from s72-s75. src/system.c verified clean by git status --porcelain after every batch, and candidate.c re-scored at 2/179/0 after its header edit.

- [s76] The floor body's residual is now on record as a full instruction listing: it emits every one of target slots 51-67 with the TARGET'S REGISTERS and differs only by transposing slots 56 and 57 (sll $a0,$a0,2 before addu $v0,$v0,$s5). Both contested seats are already correct on this body; the residual is one adjacent swap of two independent ALU insns.

- [s76] NEW pass-level rung, never examined in 75 sessions: sched.c's adjust_priority (2571-2590) raises a ready insn with zero REG_DEAD notes to max_priority when birthing_insn_p (2504-2528) holds, and that predicate is reg_n_sets[SET_DEST]==1. Both contested insns are boosted, which is WHY the comparison reaches the INSN_LUID rung s69 recorded.

- [s76] b2 (arg5 address carried in the existing multi-set local 'status') emits the target's exact block-3 order with NO loop note - the third independent route to order-perfection after g06 and r3, and the only one needing neither a statement reorder nor a do-while(0). Saved as progress/s76-b2-birth-deboost-target-block3-order-no-loop-note-12.c.

- [s76] A de-boost carrier needs two sets that BOTH survive to flow.c: c3 and c5 (copy-initialised carriers) are byte-identical to the floor at 2/179/0 because the copy is propagated away, the mirror of s69's delete_noop_moves result for same-value re-stores.

- [s76] Control a5 (de-boosting insn 106 rather than 120) scores 8: with both insns unboosted the comparison returns to INSN_LUID and 120 (luid 12) still beats 106 (luid 6), so the order does not flip. Attribution is on insn 120's boost specifically.

- [s76] The coupling is exact: the target needs B.lbu before A.lbu AND A.addu before B.sll, both INSN_LUID ties, so no wholesale reversal of the two chains can satisfy both. On r3 the lbu order is decided by a functional-unit BLOCKAGE rather than by LUID.

- [s76] Every qty_compare_1 input for the seat half is now measured across s70-s76: refs move only with loop_depth (a note), span and quantity number are pinned by the target's own emission order (birth order), and qty_size costs an instruction (s72's DImode kill at 181 insns).

- [s76] 13 forms measured this session, all at build 179 / rules_dropped 0; 11 new rejected forms banked (264 total) and one new progress base saved. candidate.c is unchanged as a body and remains the floor at 2/179/0; only its header gained the s76 summary.
