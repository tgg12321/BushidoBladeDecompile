# Hypothesis ledger — func_800600C8

## s2 (structural) — floor 6 -> 0 MATCH

### CONFIRMED
- **H-s2-1 reverse-scheduler emit==chain for tied leaves**: among the five
  pri-1 leaf insns (78 copy, 81 sh4A, 84 sh48, 87 sym, 89 sw18), emit
  order == RTL chain order (reverse scheduler picks highest-LUID first;
  launched producers follow their consumer immediately). Verified with
  BB2_SCHED_DEBUG full pick trace + three sandbox measurements matching
  predictions (6 / 2 / 5).
- **H-s2-2 p0-first + d0-then-d1 ("Form P")**: predicted 2 (only sh pair
  swapped), measured 2.
- **H-s2-3 overwrite-order swap unlocks the pair order**: putting
  s.d1=hi%10 BEFORE s.d0=arg0%10 in source makes flow's backward DSE
  scan reach the init pair with tracker=fp+74, so init order [d1, d0]
  survives; chain order then equals target emit order end-to-end.
  Predicted 0, measured 0. THE MATCH.

### KILLED
- **Frontier-3 (d0; p0; d1)**: measured 5, worse than Form P's 2; trace
  shows group splits [move, sh48] ... [sh4A]. rejected/
  d0-p0-d1-init-order-score5.c.
- s1 frontier hypotheses 1 (priority split via RANK_DEBUG steering) and
  2 (class-2/3 lever) were built on a FORWARD-scheduler model; the
  scheduler is reverse. Superseded by H-s2-1..3, no probe needed.

## s1 (recon) — floor 13 -> 6, cheat-free, 121/121 insns

### CONFIRMED
- **H1 per-arm width stores**: dropping the `width` local and assigning
  `s.width` directly in each if/else arm (both call sites) lets jump2
  cross-jump-merge the identical `sw` suffixes -> target's `j`+delay-li
  diamond, width in $v0, store-before-addiu-a0. Fixed ~7 of the 13 diffs in
  one lever (both `addu $4,$sp,24<->sw` regfix clusters + v0/v1 renames +
  missing `j`). Mechanism: [[cross-jump-store-tail-merge]] family.
- **H2 pair-then-p0 source order**: `hi=arg0; s.d1=hi; s.d0=hi; s.p0=&D_8009B6FC;`
  keeps both dead init stores + the `move v0,s1` copy. Mechanism: flow.c
  single-slot `last_mem_set` DSE (flow.c:1740 insn_dead_p, :1988 mark_set_1,
  :2393 any-mem-read clears, calls clear at :1630); the p0 store between
  d0-init and d0-overwrite evicts the tracked slot. Also flipped cur_tex/magic/
  mfhi-temp onto target regs t1/t0/t2 and restored the prologue `addu t1,s2`.

### KILLED (measured, see rejected/)
- p0-first + any address-respelling of the remainder stores
  ((&s.d0)[k] constant index: folds at expand; s16 *dp pointer: cse
  canonicalizes to fp+48 before flow). Both -> d0-init deleted, score 16.
- chain forms: `s.d0=(s.d1=arg0)` (4A-first but d0-init deleted, 16);
  old HEAD `s.d1=(s.d0=arg0)` keeps both but wrong order 48,4A (regfix-needing).

### OPEN — the last 6 diffs (single block transposition)
ours: [copy; sh 4A; sh 48] then [lui/addiu B6FC; sw 0x18]
target: [lui/addiu B6FC; sw 0x18] then [copy; sh 4A; sh 48]
Both blocks are sched1-ready simultaneously (after `mult a3,t0` issues), both
chains priority ~2, rank_for_schedule (sched.c:2399) ties -> class check vs
last_scheduled_insn (likely both class 3) -> INSN_LUID = source order. Flow
forces pair-before-p0 in source, so LUID puts the pair first. Target needs the
B6FC chain first.

Frontier hypotheses for s2:
1. **Priority split**: find a spelling that raises the B6FC chain's priority
   above 2 (or drops the pair's). E.g. if `reg=symbol_ref` insn latency/cost
   can be made 2 (HIGH+LO_SUM split earlier?), or the copy made latency-1
   consumer class. Probe: BB2_RANK_DEBUG=1 (this cc1 has env-gated rank
   tracing, sched.c:2436-2446) on the floor-6 form; log the actual
   priorities/classes of the four insns at the decision point; then target
   whichever term is actually tied.
2. **Class-2/3 lever**: if the immediately-prior scheduled insn can be made
   something the COPY (but not the B6FC head) depends on with latency>1, the
   copy drops to class 1/2 and B6FC (class 3) wins the tie regardless of LUID.
   Candidate: make the pair carrier depend on the second mult's operand chain
   (e.g. carrier = value that is a consumer of a late insn) WITHOUT changing
   emitted bytes — needs care, coercion smell must be avoided.
3. **Different pair/p0 arrangement that satisfies flow AND LUID**: e.g.
   `s.d0 = hi; s.p0 = ...; s.d1 = hi;` survives flow (measured logic, not yet
   built) but predicts emit order sh48-first — target is sh4A-first. Predicted
   dead but cheap to measure; would also pin down whether sched reorders
   same-priority independent sh stores (it shouldn't — LUID-stable sort).

## [s1] Per-arm s.width stores (no width local) at both call sites reproduce the target's j+delay-li diamond with width in $v0 and sw-before-addiu-a0
- mechanism: jump2 cross-jump merges the identical `sw v0,0x30(sp)` arm suffixes (cross-jump-store-tail-merge family); short per-arm pseudos land in $v0
- probe: Rewrote both width sites as if/else direct s.width stores; sandbox --disable all
- result: Diamond, register, and store-order all flipped to target shape in one lever; explains both addu/sw regfix swap clusters + 1 missing insn
- verdict: CONFIRMED

## [s1] d1/d0 init pair survives dead-store elimination only if a store to another slot sits between d0-init and its overwrite; ordering `s.d1=hi; s.d0=hi; s.p0=&D_8009B6FC;` gives target's 4A,48 order plus the move v0,s1 copy
- mechanism: GCC 2.7.2 flow.c single-slot last_mem_set DSE (flow.c:1740/1988): the p0 store evicts the tracked slot; combine cascade-deletes the copy when a store dies
- probe: cc1 -da pass tracing (insn 89 present in .cse2, deleted in .flow) + reorder probe; sandbox
- result: Score 13 -> 6, build insns 119 -> 121, sh order now 0x4A then 0x48, cur_tex/magic/mfhi-temp land on t1/t0/t2 exactly as target
- verdict: CONFIRMED

## [s1] The d0-init store can instead be protected by spelling the overwrite through a non-frame-equal address (constant-index (&s.d0)[k] or s16* pointer var), allowing p0-first source order
- mechanism: flow.c rtx_equal_p requires structural mem equality; hoped expand/cse would preserve a (mem (reg)) form
- probe: Built both spellings; sandbox each
- result: Both score 16/119: expand folds constant-index to direct frame mem; cse canonicalizes the pointer deref to fp+48 before flow. Address respelling cannot hide a stack slot from flow DSE
- verdict: KILLED
