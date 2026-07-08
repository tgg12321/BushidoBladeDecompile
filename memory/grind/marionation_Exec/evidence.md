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
