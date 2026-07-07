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
