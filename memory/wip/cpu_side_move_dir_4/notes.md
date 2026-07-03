# cpu_side_move_dir_4 (system.c) — WIP, masked floor 4 / raw floor 8

## TL;DR (session 3d — CARRY-FORMS solve the ORDER; the STAGING THEOREM is the last wall)
The faithful permuter found (and sandbox verified) the ORDER-half unlock:
**route val5 through a multi-set carrier** (`i = tbl_125c[idx_1494[1]]` — 2nd
set = check2's `i = 7`) — the lw5's dest is then non-birthing → no LAUNCH →
sched1 re-times the WHOLE TAIL into target order (raw diff shows every slot
matching in the carry forms; e1/90-x = raw 12-13, all registers). Combining
with the split-addu frame gives THREE near-miss decompositions (best raw 8):
- t2-TRIPLE (w=idx<<2∪src, i=val5∪counter, k=addr∪-1): raw 8 = the state-A
  HEAD trap (lbu5 up-2/w-sll down-2/pp) + k=a3 (needs a0) at addu/lw-a3.
- split-addu+plain-arg5 (3b): raw 8 = perfect head, 6-slot tail rotation+p_t.
- carry-only (e1): raw 12, PERFECT order, p_t=v1 steals the cascade.
**THE STAGING THEOREM (why no tested form closes)**: locals allocate before
globals; any in-call/lone address temp p_t is LOCAL and steals v1 (no local
v1-blocker exists once val5 is carried=global); the only a0-compatible addu
dest is w itself (3-stmt) whose homing mem(w) sets the a3-pref, unmaskable
because the only a3-holder (-1h, pri 2142) allocates after w (pri≥11142) and
someone_prefers needs a CONFLICTING lower-pri allocno WITH an a3-pref (none
has prefs). k∪-1 as addr gets the a3-pref where k needs a0 (t2's G2). Dead
stores are INERT (d1: flow recounts refs+sets — the stale-reg_n_sets door is
closed). k-value forms blow up (21-25). reu-arg5: chain grabs a0 (14).
NEXT LEVERS: (1) permuter grinding from the 90-forms (honest metric, running);
(2) t2's G1 head trap with i-carry has DIFFERENT mechanics than state-A's
(lbu5-i is pri-2 leftover, loses T-16 to nothing — readiness analysis
unfinished — trace t2 with -dS!); (3) enumerate second-role pairings giving
p_t a v1-blocking LOCAL (none found: pp/fp sugg-diverted, arg5-copy folds).

## TL;DR (2026-07-03 session 3b — SPLIT-ADDU BREAKTHROUGH: w=a0 ACHIEVED, pref dead)
**candidate_splitaddu.c = masked 4 with the REGISTER HALF ~SOLVED**: split the
address chain so the addu lives IN THE CALL ARG — `w = idx[0]; arg5-stmt; pp;
w <<= 2; call(..., *(s32 *)(w + (s32)tbl_125c), arg5)` + the check2 src→w
unification. Effects (all verified): homing's mem-address = the in-call addu
temp p_t (NOT w) → w gets NO a3-pref → and w's last use = the in-call addu
(luid ~30) → w [8,30] covers val5 → v1-conflicted → **w = a0** (slots 49-54
byte-match: lbu a0/lbu5-v0/pp/sll5-addu5-v0). `w + (s32)tbl` operand order
gives `addu ?,a0,s3` ✓. Residual = 6-slot schedule permutation {sll4@58→55,
addu4@61→59, lw5/11D5/sll-11D5/sw 1-slot shifts} + p_t=v1→a0 (masked-blind;
FOLLOWS automatically if addu4 emits @59 < sw: val5-overlap → v0/v1 blocked).
-dS trace (tmp/rtl/t1.i.sched): the T-11 memory-unit hazard (sw@T-10) swaps
11D5-lbu/w-sll (fix = sw off T-10 ⟹ sll4@55 exact); addu4@T-8-pick forced by
its in-call luid > a2-sll's (arg 4 > arg 3) — the last wall. u-as-statement
kills w's range (v1-hole); u∪jalr coalesces back to the poisoned form; a2v
hoists blow up (24-26); i5/t1 spellings sink/normalize (invariant 4s).
NEXT: the FAITHFUL permuter workspace is BUILT and running (session 3c):
tmp/perm_csmd4/ — full-TU compile + fn extraction; **compile.sh splices the
permuted fn back into base_full.c via tmp/csmd4_splice.py** because the
permuter PRUNES other fn bodies and the pruned-TU metric is UNFAITHFUL
(its "90-finds" = arg5-through-var reuse scored 13-17 in the real sandbox —
always re-verify finds via engine sandbox). Regenerate: tmp/csmd4_perm_setup.sh
+ csmd4_perm_base.py (+ sanitize: csmd4_sanitize_base.py escapes multi-line
asm strings for pycparser) + cp base.c base_full.c. Launch: permuter.py
tmp/perm_csmd4 --best-only --stop-on-zero -j24. base=265 (the 6-line diff).
State-A's trap re-confirmed luid-fork-forced by its -dS trace (T-16 LAUNCH
beats pp; both fork arms map to t0-luid vs arg5-luid — no third state).

## TL;DR (session 3a — the mechanism map; superseded details in git history)
Cross-block reuse discovered: w = chain∪src (a0-family), k = val∪-1 (a3).
The T1/T2 trap pair: inline deref → a3-pref on w (set_preference one-level
MEM unwrap, global.c:1601; local arg4 var recreates it via renumbered
copy-sugg); k-carry kills the pref but shortens w below val5's birth
(v1-hole). Statement order invariant. [Session 3d's staging theorem subsumes
and extends this — read 3d first.]

## The decisive mechanism facts (all read from tools/gcc-2.7.2, verified by dumps)
- find_reg pass 0 excludes `regs_someone_prefers` (prefs of CONFLICTING
  lower-pri allocnos, merged by prune_preferences) and scans ASCENDING regno
  (no MIPS REG_ALLOC_ORDER) → a0 before a3 when both eligible.
- **COPY-pref upgrade short-circuits (`goto no_prefs`) BEFORE the full-pref
  loop** — an unfoldable `(set a0 (reg w))` reg-reg copy-pref would beat the
  a3 full-pref outright. Jalr-arg-temp attempt FAILED: cse folds a single-use
  load into the call homing (w vanishes from the jalr block).
- Preferences masked by AND_COMPL vs `used` — a3 enters `used` only via
  hard_reg_conflicts[w] (an a3-holding CONFLICTING allocno assigned BEFORE w).
- Priorities (ALLOCDBG, pri = floor_log2(refs)×refs/livelen×10000): w-inline
  16956 (13refs/23len), w-k1-form ~20500 (worse: shorter live), i 15555,
  temp 4210, -1holder 2142. i-before-w needs w refs 13→11: impossible (the
  3-stmt multi-set chain shape is byte-pinned: sll a0,a0 / addu a0,a0).
- adjust_priority: LAUNCH (0x7f000001) goes to insns passing birthing_insn_p
  = SET to live reg with **reg_n_sets==1** (flow-time). Multi-set dests never
  launch → k-load/w-chain place early-forward (the T2 hole is scheduler-forced).
- Target's own qty geometry is UNREACHABLE with any block-local temp (every
  single-set temp ties/beats val5 for v1; multi-set w-local [8,32] qty also
  takes v1 — enumerated & measured). Cross-block is the only door. 

## Open levers for the last flip (w: v1→a0), in order of promise
1. **someone_prefers[w] ∋ v1 or a3**: a conflicting LOWER-pri allocno with that
   full-pref. Surveyed: i/temp/-1h/dst — none has a v1/a3 pref source. Untried:
   give an EXISTING conflicting global a v1-full-pref via a real one-unwrap
   insn shape ((set X (op reg-renumbered-v1))...). Enumerate check2/poll vars.
2. **Unfoldable a0-copy for w**: (set a0 (reg w)) that cse/combine can't fold —
   needs load and call in DIFFERENT blocks... jalr-arg load hoist breaks bytes
   (lbu must stay after beqz). Look for an a0-homing whose source could be w
   with a REAL second use pinning the copy.
3. **T1-frame + a3-conflict**: any allocno > 16956 pri, conflicting w, that
   would take a3 — none exists naturally; -1h∪arg5 merge = 8333 (too low).
4. Faithful directed permuter over the k-frame (compile.sh = sandbox pipeline).

## Ruled out this session (do NOT re-derive)
- G1-G4 hoists of D_800A11D5 (refs-2 equiv-sink puts the load back), inline-arg
  forms (14: wholesale restructure), dual temp-free chains D1-D8 (6-10),
  tie-battery of 10 arg5 spellings (identical qty tables), 192-combo grid
  (floor 4), a2v hoists (14-16), jalr-temp copy-pref (cse-folded), arg5-first
  k-frames (hole is luid-invariant).
- The 2026-07-01 "stretch the chain density" framing is SUPERSEDED by the
  cross-block discovery (density can't fix it; the pref/hole can).

## Pointers
- Probes: tmp/csmd4_{k_unify,jalr_w,crossblock,final_grid,n1n4,hapr,allocdbg,
  qty_both,rankdbg,hoist11d5,tiebattery}.py (gitignored; regenerate from here).
- FINDREGDBG: BB2_FINDREG_DEBUG=<pseudo> (new, global.c find_reg); rebuild
  gccdbg with `make cc1 CFLAGS="-g -fgnu89-inline"` (c-gperf inline fix).
- marionation_Exec shares the wall: its region-1 = the same two-state trap;
  the cross-block door likely transfers (check marionation's sibling vars).
- Target block: asm/funcs/cpu_side_move_dir_4.s lines 49-73.
