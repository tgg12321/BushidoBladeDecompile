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
