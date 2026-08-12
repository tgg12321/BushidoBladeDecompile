# Hypothesis ledger — motion_Close

## DEAD (do not re-open)

### H1 — the target's 16-byte frame is reachable by some pure-C spelling
**Verdict: KILLED (session 1), with a corpus-wide measurement.**
Mechanism: gcc-2.7.2's MIPS o32 backend reserves `REG_PARM_STACK_SPACE` = 16
bytes of outgoing-argument area for every C-level call, so the minimum pure-C
frame here is 16 (args) + 12 (s0/s1/ra) = 28 → aligned 32. The target's frame is
16 with `ra@12`, which back-solves through GCC's `[args][vars][saves]` layout to
an outgoing-arg area of **zero**. Census of all 854 call-making functions in the
oracle-matching build: zero have a sub-16 arg area, except `motion_Close` and
`func_80083794`, both of which get there only because their `jalr` is inline asm
(which never sets `current_function_outgoing_args_size`).
Consequence: **no pure-C body containing a call can match this function's
prologue/epilogue.** Frame size and all six save/restore offsets are permanently
off. Do not spend a session on frame shrinking, local-count tuning, phantom-slot
levers, or callee-signature reshaping — none of them can remove the 16.

### H2 — declaration order controls the s0/s1 role assignment
**Verdict: KILLED (session 1).** Declaring `count` before `p` left the score at
20 and moved no register. Both orders emit the address materializations in
source order, so emission order is not the discriminator; the role split comes
from allocator priority. Banked as
`rejected/decl-order-swap-no-register-effect.c`. Subordinate to H1 anyway —
closing the registers alone cannot produce a match while the frame differs.

### H3 — the target's $s0=p / $s1=count role split is reachable from pure C
**Verdict: session 2 said KILLED. SESSION 3 FALSIFIED THAT — H3 is CONFIRMED
TRUE and has MOVED TO THE LIVE FRONTIER (see F4 below). The text that follows
is session 2's, kept verbatim as the record of a wrong inference drawn from a
correct measurement; do not act on its conclusion.**

> **Why it was wrong.** Session 2 read only the priority formula in
> `allocno_compare` and stopped one line early. `global.c:633-654` falls through
> to `return *v1 - *v2;` — the ALLOCNO NUMBER, i.e. pseudo creation order, i.e.
> DECLARATION order — whenever priorities are EQUAL. Session 1's H2
> ("declaration order is inert") was measured in the no-tie regime, where that
> code never runs. Session 3 created the tie (p carrying the D_800A2668 guard
> value lifts p to n_refs=9 / live_length=9 / pri=30000, exactly count's) and
> declared p first; p took `$s0` and the floor dropped 17 -> 16. The register
> axis is OPEN.

**Session 2's original entry (superseded):**
Mechanism: `global.c`'s `allocno_compare` (global.c:642-655) ranks allocnos by
`floor_log2(n_refs)*n_refs/live_length` and assigns hard regs in that order.
`BB2_ALLOC_DEBUG=1` reports, for EVERY spelling emitting the target's
instruction sequence: `count` n_refs=8 live_length=8 pri=30000 -> `$s0`;
`p` n_refs=7 live_length=7 pri=20000 -> `$s1`. `count` is structurally one
reference richer (init + outer guard test + decrement set/use + loop test vs
init + load + bump), and the target's instruction sequence contains those same
references — so a matching C source would have the same counts. The target has
`p` in `$s0`: the inverse. The floor_log2 step at 8 makes the margin wide
(3*8 vs 2*7), not a tie-break.
The only two role-flipping spellings both emit instructions the target lacks
(end-pointer loop: 27 insns, score 21; guard laundered through `p`: 27 insns,
and a T1/T2/T3 cheat). A byte-free eighth reference for `p` would be a pure
allocator-coercion construct — forbidden. **Do not reopen the register axis.**

### F2 — is the residual EXACTLY the nine regfix rules?
**Verdict: RESOLVED YES (session 2).** Block attribution of the 17-point
residual sums to exactly the measured score: guard-load temp 2, frame size 1,
save block 3, beqz delay slot 2, inner guard register 1, loop-body registers 4,
epilogue offsets 4. Every bucket is dead-by-H1 (frame), dead-by-H3 (registers),
or hand-asm signal S-b (the wasted delay slot). No unexplained instruction.

### F3 — the s0/s1 role + v0-vs-t0 temp assignment
**Verdict: KILLED (session 2) — subsumed by H3.** 20 structural spellings
measured; the only movement on the whole axis was statement order (floor
20 -> 17), which fixes the emission POSITIONS of the two materializations but
cannot touch the ROLES.

## LIVE FRONTIER

### F1 — motion_Close's original source was hand-written assembly (crt0 runtime)
> **Session 3 amendment: the (S-c) leg below is RETRACTED — the register
> assignment IS producible from C (see F4 / the H3 entry). Only S-a (zero
> outgoing-arg area) and S-b (the unfilled beqz delay slot) survive, plus the
> crt0-neighbourhood corroboration. Any escalation packet must cite two signals,
> not three, and must not repeat S-c.**

**Mechanism / evidence already in hand (see evidence.md):** (S-a) no outgoing-arg
area despite making a call, unique in an 854-function corpus and impossible from
compiled C per H1; (S-b) an unfilled `beqz` delay slot at insn 7 that GCC's
`reorg.c` demonstrably fills (the honest C form fills it with `sw s0,16(sp)`) —
`regfix.txt:117` exists solely to re-insert that `nop`; (S-c) the function sits
in the PsyQ/BIOS crt0 neighborhood (`_start`, `InitHeap`, a `.word`-syscall
trampoline, `bios_FileRead`) and its body is the classic
constructor/destructor-table walk over two linker-provided symbols.
**Next probe:** extract `asm/funcs/motion_Close.s` (it does not currently exist —
the function is C-routed, which is why `tools/scan_hand_coded.py --single
motion_Close` aborts with "asm/funcs/motion_Close.s not found"), then run
`scan_hand_coded` to obtain a signal tier. The endgame-lock canonical-asm gate
requires **STRONG** signals (S1/S2/S6) — this must be measured, not asserted.
Note the `canonical` gate currently routes the function **C** purely on
"distance 21 <= 50", i.e. on size, having never seen an honest C measurement.

> **Session 3 update — SUPERSEDES the session-2 update below.** F1 is NO LONGER
> the only live frontier and its S-c leg is RETRACTED. H3 is falsified (the
> register roles are C-reachable), so F2's "every bucket is dead" attribution no
> longer holds either: the loop-body-register and inner-guard buckets have been
> CLOSED by actual C, not merely attributed. The honest floor is 16. The live
> frontier is now F4 (below), with F1 reduced to its two surviving signals.

### F4 — [LIVE, session 3] close the order/priority coupling in the role-flipped family
**State.** The role-flipped body (guard carried in `p`, `p` declared first)
scores 16 and reproduces the target's loop-body registers exactly. Two things
still cost:
  (a) the guard load lands in `$s0` (p is callee-saved and holds the guard),
      forcing `sw s0` above it and `sw s1` into the beqz delay slot, where the
      target loads the guard into `$t0` before the frame exists;
  (b) the two address materialisations emit count-pair-then-p-pair while the
      target emits p-pair-then-count-pair.
**The bind.** Fixing (b) by assigning `p` before `count` shortens count's live
length 8 -> 7 (pri 30000 -> 34285) and stretches p's 9 -> 10 (pri 30000 -> 27000),
destroying the tie and handing `$s0` back to count (score 20; banked as
`rejected/pfirst-body-order-returns-s0-to-count.c`). Order and roles are coupled
through `live_length`.
**Also measured dead:** lowering count's priority instead of raising p's —
references and live length shrink together, so deleting a count reference raises
its priority (`rejected/staged-guard-local-shortens-count-liverange.c`, 22).
**Next probes, in order:**
1. Find a reference for `p` that does NOT extend its live range — the priority is
   `floor_log2(n)*n/live_length`, so a ref inside p's existing range is worth
   strictly more than one that extends it. A second real read of `*p` inside the
   loop body would be weighted by loop depth (worth 2) without changing the
   range; the question is whether any such read is byte-neutral.
2. Attack (a) instead of (b): any form where `p` reaches priority parity WITHOUT
   holding the guard leaves the guard load in a caller-saved temp and should be
   worth more than the 1 point (b) is worth. This is the same "byte-free extra
   reference for p" question session 2 declared forbidden — but session 3 shows
   the requirement is only PARITY, not dominance, which is a much smaller ask.
3. Re-run the `.lreg` local-alloc dump for the loop temp now that the roles are
   right: the temp is `$v0` and the target's is `$t0`, and it is now the largest
   single remaining non-frame bucket.
**Hard cap to remember:** H1 still stands, so nothing in this frontier can reach
distance 0. Its value is a lower floor and a correct escalation packet.

> **Session 2 update (SUPERSEDED by the session-3 update above):** F2 and F3
> below are CLOSED — see the H3 / F2 entries in the DEAD section above. F1 is
> the only live frontier. The honest floor is 17 and every one of its seven
> residual buckets is attributed to a dead axis.

### F2 — [CLOSED s2] pin down whether the residual is EXACTLY the nine regfix rules
**Mechanism:** if the honest C form differs from target only in (a) frame size,
(b) the six save/restore offsets, (c) the s0/s1 roles + v0-vs-t0 temp, and (d)
the one delay-slot nop, then the nine rules at `regfix.txt:115-124` are a
complete and exact description of the gap, and the function is a clean
"bytes-provable only through a surface the grinder may not touch" case rather
than an open grind. Session 1's dumps already show this to a high confidence;
what is missing is a mechanical instruction-by-instruction attribution.
**Next probe:** write a differ that pairs the 26 target insns against the 25
build insns and classifies each mismatch into one of those four buckets, and
assert the buckets are exhaustive. Cheap, and it turns the escalation packet
from narrative into a table.

### F3 — [CLOSED s2 — killed by H3] the s0/s1 role + v0-vs-t0 temp assignment
**Mechanism:** allocator priority in `local-alloc`/`global-alloc` — `p` and
`count` have near-identical reference counts and identical live ranges, so the
tie-break decides. A lever would have to change reference counts or live ranges
via a natural C spelling (e.g. the `*p++` idiom, a `while` rather than
`if`+`do-while`, or hoisting the guard load). **Do not invest here before F1
resolves:** under H1 this axis cannot produce a match on its own, so its only
value is making the escalation packet's "everything except the frame closes"
claim airtight. If F1 returns STRONG, F3 is moot entirely.

## [s1] The HEAD baseline of 21 was a real measurement of a C form, so the function is a normal 21-instruction pure-C grind.
- mechanism: The committed body builds the entire loop out of register pins (asm("s0"/"s1"/"t0")) plus __asm__ volatile("jalr %0") and a hardcoded __asm__ volatile("addiu $17, $17, -1"). The cheat-stripper removes both __asm__ blocks — which ARE the loop body — so the stripped function has no call and no loop work at all.
- probe: sandbox motion_Close --disable all at HEAD, then dump normalized_insns for target vs build.
- result: HEAD stripped build is 13 insns vs target 26: no jalr, no lw/addiu loop, no counter decrement. The 21 was an artifact of a degenerate stripped function; no honest C form of motion_Close had ever been measured before this session.
- verdict: KILLED

## [s1] A natural pure-C function-pointer-table walk reproduces the target's structure.
- mechanism: The target is the classic crt0 constructor/destructor-table loop: guard on a global, materialize a table pointer and a count from two linker-provided symbols, then do { f = *p; p++; f(); count--; } while (count).
- probe: Replaced the pinned/inline-asm body with `void (**p)(void); s32 count; if (D_800A2668 != 0) { p = &D_8008D070; count = (s32)&D_00000000; if (count != 0) { do { void (*f)(void) = *p; p++; f(); count--; } while (count != 0); } }` and ran sandbox --disable all.
- result: score 20, build_insns 25 vs target 26. CFG, operation sequence, and even the count-- scheduled into the jalr delay slot are identical instruction-for-instruction. The entire residual is register names and stack offsets — no missing or extra computation anywhere.
- verdict: CONFIRMED

## [s1] H1 — the target's 16-byte frame (ra@12, s1@8, s0@4) is reachable by some pure-C spelling.
- mechanism: GCC 2.7.2 lays the MIPS frame out as [outgoing args][vars][saved regs], with ra at args+vars+gp_size-4. Formula validated on this session's own build, whose C has zero stack locals: 16+0+12-4 = 24, and the build emits `sw ra,24(sp)`. Back-solving the target (ra@12, gp_size 12) gives args+vars = 4, i.e. an outgoing-arg area of ZERO. But the o32 backend defines REG_PARM_STACK_SPACE = 16, forcing >=16 bytes of outgoing-arg area for every C-level call, so the minimum pure-C frame here is 16+12 = 28 -> aligned 32 (exactly what the honest form emits).
- probe: Census over all 854 call-making functions in the oracle-matching build/src/*.o, computing implied (args+vars) = ra_offset - (gp_size - 4). Script: tmp/grind/motion_Close/s1/census.py.
- result: Distribution: 4->2, 16->538, 20->1, 24->101, 32->30, 40->21, 48->31, 56->23, 64->10, 72->15, 80->14, 88->16. ZERO functions have a sub-16 outgoing-arg area except the two at 4 — which are motion_Close and its sibling func_80083794, both in ings2.c, and both of which reach it ONLY because their jalr is inline asm (an __asm__ block never sets current_function_outgoing_args_size). The frame shape the target has is precisely the shape the cheat produces and that compiled C provably cannot.
- verdict: KILLED

## [s1] H2 — declaration order controls the s0/s1 role assignment (target: s0=p, s1=count; honest C: s1=p, s0=count).
- mechanism: If the allocator assigned registers in declaration/emission order, swapping the two declarations would swap the roles.
- probe: Declared `count` before `p`, leaving body assignment order unchanged; re-ran sandbox --disable all.
- result: score 20, build_insns 25 — identical, no register moved. Both orders emit the address materializations in source order (p first), so emission order is not the discriminator; the split is an allocator-priority tie-break. Banked as rejected/decl-order-swap-no-register-effect.c.
- verdict: KILLED

## [s2] The loop's C spelling (pointer idiom, loop form, temp placement, guard shape, count signedness) can move the honest floor below 20.
- mechanism: The residual is register names and stack offsets, not computation, so if any of it is reachable from C it must come from a spelling that changes GCC's emission order, reference counts, or live ranges.
- probe: Two sandbox sweeps over 27 spellings total (tmp/grind/motion_Close/s2/sweep.py 13 forms, sweep2.py 14 forms), each patched into src/ings2.c and measured with `sandbox motion_Close --disable all`.
- result: Exactly ONE lever moved: assigning `count` before `p` inside the guard drops the floor 20 -> 17 (GCC emits the two address materializations in source order, so count-first puts the $s0 lui/addiu pair at insns 7-8 and the $s1 pair at 9-10 — the target's register order at those slots). Everything else was inert at 17: *p++ post-increment read, plain `while`, `--count` in the loop condition, for-loop, (*p)() with no temp, p[0]()+p=&p[1], f hoisted out of the loop, block-local initialised decls, early-return guard chain, hoisted guard local, u32 count. Worse: count hoisted above the D_800A2668 guard (19), decrement before the call (19), index walk (19), end-pointer loop (21, 27 insns).
- verdict: CONFIRMED

## [s2] H3 — the target's $s0=p / $s1=count role split is reachable from a pure-C spelling.
- mechanism: GCC 2.7.2's global.c ranks allocnos by floor_log2(n_refs)*n_refs/live_length (allocno_compare, global.c:642-655) and hands out hard registers in that order, so the higher-priority allocno takes $s0. If p and count are near-tied, a natural C spelling that shifts reference counts or live ranges could flip which one wins $s0.
- probe: Ran the instrumented cc1 (tools/gcc-2.7.2/cc1, BB2_ALLOC_DEBUG=1) over 8 further spellings, reading the per-allocno n_refs / live_length / priority table for motion_Close directly (tmp/grind/motion_Close/s2/allocsweep.py + allocsweep.log); corroborated by the .greg RTL dump (greg.sh) showing pseudo 72=count -> $16 and pseudo 73=p -> $17.
- result: For EVERY spelling that emits the target's instruction sequence: count n_refs=8 live_length=8 pri=30000 (takes $s0), p n_refs=7 live_length=7 pri=20000 (takes $s1). count is structurally one reference richer than p (init + outer guard test + in-loop decrement set/use + loop test, versus init + in-loop load + in-loop bump), and the target's own instruction sequence contains all of those same references, so a hypothetical matching C source would carry the same counts. The target has p in $s0 — the inverse — and the floor_log2 step at 8 makes the margin wide (3*8 vs 2*7), not a nudgeable tie-break. The only two spellings that DID flip the roles both emit instructions the target does not contain: the end-pointer loop (p pri=36666, 27 insns, score 21) and a guard laundered through p, `if (p != &D_8008D070 + count)` (p pri=30000, 27 insns, score 17 — and a T1/T2/T3 cheat: the predicate is exactly `count != 0` and its only purpose is to add a p reference). Any BYTE-FREE eighth reference for p would by definition be a construct with no emitted effect whose sole function is to move GCC's allocator, i.e. a forbidden coercion. Closed on measurement and on policy.
- verdict: KILLED

## [s2] F2 — the residual between the honest pure-C form and the target is EXACTLY the nine regfix rules at regfix.txt:115-124, with no unexplained instruction.
- mechanism: If every mismatch classifies into frame size, save/restore offsets, the s0/s1 roles plus the v0-vs-t0 temp, and the one unfilled delay slot, the gap is fully characterised and no live grind axis remains inside it.
- probe: Block-level attribution of the new 17-point residual against the target's 26 normalized instructions, using the target and build dumps produced by the sweeps.
- result: The buckets sum to exactly the measured score of 17: guard-load temp ($t0 vs $v0) 2, frame size 1, register-save block (offsets + order) 3, beqz delay slot 2, inner guard register 1, loop-body registers 4, epilogue restore offsets + sp adjust 4. Every bucket is dead-by-H1 (frame), dead-by-H3 (registers), or hand-asm signal S-b (the wasted delay slot). No unexplained instruction remains.
- verdict: CONFIRMED

## [s3] H3 revisited — the target's $s0=p / $s1=count role split IS reachable from pure C, because global.c breaks an exact priority TIE by allocno number (= declaration order).
- mechanism: allocno_compare (tools/gcc-2.7.2/global.c:633-654) returns pri2-pri1 and, only when that is zero, falls through to `return *v1 - *v2;` — the allocno number, which follows pseudo creation order, which for block locals follows DECLARATION order. Session 2 read only the priority formula; session 1 measured declaration order as inert but did so in the no-tie regime where the fall-through never executes. Priority is floor_log2(n_refs)*n_refs/live_length*10000, so lifting p from 7 refs to 9 (with live_length also 9) moves it from 20000 to exactly count's 30000 — a tie, not dominance, which is all the tie-break needs.
- probe: A 2x2 crossing {guard value carried in p, not carried} x {p declared first, count declared first}, each cell measured with `sandbox motion_Close --disable all` AND with the instrumented cc1 (BB2_ALLOC_DEBUG=1) reading the per-allocno table. Script tmp/grind/motion_Close/s3/tiebreak.py, log tiebreak.log; the tie itself was first spotted in allocsweep3.py/.log.
- result: {count first, no carry} p 7/7 pri 20000, count 8/8 pri 30000, count takes $s0, score 17. {p first, no carry} identical priorities, count still takes $s0, score 17. {count first, carry} p 9/9 pri 30000 TIED with count 8/8 pri 30000, count wins on the lower allocno, score 17. {p first, carry} same tie, p wins on the lower allocno and takes $s0, score 16 — the target's role split, with the target's loop-body registers exactly (lw from $s0, addiu $s0,$s0,4, addiu $s1,$s1,-1, bnez $s1) and the inner guard on $s1. New honest floor 16. Both halves are load-bearing; neither alone moves anything. Hand-asm signal S-c is therefore RETRACTED.
- verdict: CONFIRMED

## [s3] The role flip can be had without the prologue cost by lowering count's allocno priority instead of raising p's.
- mechanism: The tie-break only needs parity, so removing one of count's 8 references (the outer `if (count != 0)` guard test, staged instead through a separate short-lived local) should drop count to 7 refs and tie p's 7, letting declaration order win $s0 for p with no guard carried in p and hence no guard load in a callee-saved register.
- probe: Three declaration/body-order arrangements of the staged-guard-local form, each measured with sandbox and the instrumented cc1 (tmp/grind/motion_Close/s3/rolesweep.py, rolesweep.log).
- result: count does drop to 7 refs but its live_length drops 8 -> 6 at the same time, so its priority RISES from 30000 to 23333; p's live_length is simultaneously stretched to 9 by the extra local, dropping p to 15555. The gap widens instead of closing, no tie forms, and the extra local costs 3 more points of positional residual — score 22 in all three arrangements, with identical allocno tables. Banked as rejected/staged-guard-local-shortens-count-liverange.c. General rule extracted: references and live_length shrink together, so an allocno's priority can never be lowered by deleting references — the only workable direction on this function is adding references to the allocno you want to win.
- verdict: KILLED

## [s3] With the roles flipped, restoring the target's emission order (p pair before count pair) is compatible with keeping the roles.
- mechanism: The target emits `lui/addiu s0` (= p) at insns 8-9 and `lui/addiu s1` (= count) at 10-11. Session 2's count-before-p statement order was derived while count owned $s0 and is backwards in the flipped family, so assigning p first should fix the two materialisations' positions at no cost.
- probe: The p-before-count body order applied on top of the guard-carrying, p-declared-first form, measured twice with sandbox + instrumented cc1 (t5_pdecl_pguard_ptrorder in tiebreak.py, w5_pfirst_count_inside in ordersweep.py).
- result: Assigning p first moves count's birth one statement later: count's live_length falls 8 -> 7 and its priority RISES 30000 -> 34285, while p's live_length grows 9 -> 10 and its priority FALLS 30000 -> 27000. The tie is destroyed, count takes $s0 back, the roles revert and the score returns to 20. Order and roles are coupled through live_length; any closing form must satisfy both at once. Banked as rejected/pfirst-body-order-returns-s0-to-count.c. This coupling is frontier F4's central problem.
- verdict: KILLED

## [s3] Some further structural spelling of the loop can move the floor.
- mechanism: If any of the residual is reachable from C it must come from a spelling that changes emission order, reference counts, or live ranges — the same premise as session 2, re-tested with 18 forms disjoint from the 27 already banked.
- probe: sweep3.py (12 forms: guard carried in count / in p / in a hoisted f, p assigned inside the inner guard, count as s16, count as u8, `*p++` with `--count`, guard folded into an && chain, f hoisted and re-read at the loop tail, decrement before the call, nested block declarations with initialisers, a u32* walk casting at the call) and ordersweep.py (6 forms on top of the role-flipped body).
- result: In the s2 family everything was 17 or worse — s16 23, u8 21, p-inside-inner-guard 20, &&-chain 20, f-hoisted-and-re-read 20, decrement-before-call 19. In the role-flipped family, post-increment+pre-decrement, f hoisted out of the loop, an early-return guard, a scalar-cast guard test and a plain `while` are ALL byte-identical at 16. The loop's C spelling is confirmed a free variable in BOTH families; the residual lives in the prologue/guard region and the frame, not in the loop.
- verdict: KILLED

## [s3] H3 revisited - the target's $s0=p / $s1=count role split IS reachable from pure C, because global.c breaks an exact allocno priority TIE by allocno number (= pseudo creation order = declaration order).
- mechanism: allocno_compare (tools/gcc-2.7.2/global.c:633-654) computes pri = floor_log2(n_refs)*n_refs/live_length*10000*size, returns pri2-pri1, and ONLY when that is zero falls through to `return *v1 - *v2;` - the allocno number, which follows pseudo creation order, which for block locals follows DECLARATION order. Session 2 read only the priority formula and concluded a 30000-vs-20000 margin was unnudgeable; session 1 measured declaration order as inert but did so in the no-tie regime where the fall-through never executes. Lifting p from n_refs=7/live_length=7 (pri 20000) to 9/9 (pri 30000) reaches EXACT parity with count's 8/8/30000 - parity, not dominance, is all the tie-break needs - and the parity is reached by having p carry the D_800A2668 guard value before it is loaded with the table address.
- probe: A 2x2 crossing {guard value carried in p, not carried} x {p declared first, count declared first}, every cell measured both with `sandbox motion_Close --disable all` and with the instrumented cc1 (tools/gcc-2.7.2/cc1, BB2_ALLOC_DEBUG=1) reading the per-allocno n_refs / live_length / priority / assigned hardreg table. Scripts tmp/grind/motion_Close/s3/allocsweep3.py (spotted the tie) and tmp/grind/motion_Close/s3/tiebreak.py (the decisive 2x2).
- result: {count declared first, no carry}: p 7/7 pri 20000, count 8/8 pri 30000, count takes $s0, score 17. {p declared first, no carry}: identical priorities, count still takes $s0, score 17. {count declared first, guard in p}: p 9/9 pri 30000 TIED with count 8/8 pri 30000, count wins on the lower allocno, score 17, byte-identical output. {p declared first, guard in p}: same tie, p wins on the lower allocno and takes $s0 - score 16, with the target's loop-body registers reproduced exactly (lw from $s0, addiu $s0,$s0,4, addiu $s1,$s1,-1, bnez $s1) and the inner guard branching on $s1 as the target does. Both halves are load-bearing; neither alone moves anything. New honest floor 16.
- verdict: CONFIRMED

## [s3] The role flip can be had without its prologue cost by LOWERING count's allocno priority instead of raising p's.
- mechanism: The tie-break needs only parity, so removing one of count's 8 references - the outer `if (count != 0)` guard test, staged instead through a separate short-lived local - should drop count to 7 refs and tie p's 7, letting declaration order win $s0 for p with no guard carried in p, and hence with the guard load left in a caller-saved temp instead of being dragged into callee-saved $s0.
- probe: Three declaration/body-order arrangements of the staged-guard-local form (u1_staged_guard_local, u2_staged_guard_local_countfirst, u4_staged_countinit_inside), each measured with sandbox and with the instrumented cc1: tmp/grind/motion_Close/s3/rolesweep.py + rolesweep.log.
- result: count does drop to 7 refs, but its live_length drops 8 -> 6 at the same time, so its priority RISES from 30000 to 23333; p's live_length is simultaneously stretched to 9 by the extra local, dropping p from 20000 to 15555. The gap widens instead of closing, no tie forms, the tie-break never engages, and the extra local costs 3 more points of positional residual - score 22 in all three arrangements with identical allocno tables. General rule extracted for future sessions: references and live_length shrink together, so an allocno's priority can never be lowered by deleting references; the only workable direction on this function is adding references to the allocno you want to win.
- verdict: KILLED

## [s3] With the roles flipped, restoring the target's emission order (the p address pair before the count pair) is compatible with keeping the roles.
- mechanism: The target emits `lui/addiu s0` (= p) at insns 8-9 and `lui/addiu s1` (= count) at 10-11. Session 2's count-before-p statement order was derived while count owned $s0 and is backwards once p owns it, so assigning p first should fix the two materialisations' positions at no cost.
- probe: The p-before-count body order applied on top of the guard-carrying, p-declared-first form, measured twice with sandbox + instrumented cc1: t5_pdecl_pguard_ptrorder in tiebreak.py and w5_pfirst_count_inside in ordersweep.py.
- result: Assigning p first moves count's birth one statement later: count's live_length falls 8 -> 7 and its priority RISES 30000 -> 34285, while p's live_length grows 9 -> 10 and its priority FALLS 30000 -> 27000. The tie is destroyed, count takes $s0 back, the roles revert, and the score returns to 20. Emission order and register roles are coupled through live_length - the statement order that fixes the positions is the one that destroys the priority tie fixing the roles. This coupling is the central problem of frontier F4.
- verdict: KILLED

## [s3] Some further structural spelling of the loop, disjoint from the 27 already banked, can move the floor.
- mechanism: If any residual is reachable from C it must come from a spelling that changes GCC's emission order, reference counts, or live ranges - session 2's premise, re-tested with 18 forms chosen to be disjoint from the s1/s2 banked list and aimed at the two buckets s2 never probed directly (the $v0-vs-$t0 temp and the beqz delay slot).
- probe: tmp/grind/motion_Close/s3/sweep3.py (12 forms: guard carried in count / in p / in a hoisted f, p assigned inside the inner guard, count as s16, count as u8, `*p++` combined with `--count`, guard folded into an && condition chain, f hoisted and re-read at the loop tail, decrement before the call, nested block declarations with initialisers, and a u32* walk casting at the call) plus tmp/grind/motion_Close/s3/ordersweep.py (6 body-order / loop-shape forms on top of the role-flipped body).
- result: In the session-2 family everything scored 17 or worse: count as s16 23, count as u8 21, p assigned inside the inner guard 20, &&-chain guard 20, f hoisted and re-read 20, decrement before the call 19. In the role-flipped family, post-increment+pre-decrement, f hoisted out of the loop, an early-return guard, a scalar-cast guard test and a plain `while` are ALL byte-identical at 16. The loop's C spelling is confirmed a free variable in BOTH families; the residual lives in the prologue/guard region and the frame, not in the loop.
- verdict: KILLED

## [s4] The target's $s0=p / $s1=count role split can be reached WITHOUT the session-3 type-punning guard carry, by giving p a strict allocno priority win instead of a manufactured tie.
- mechanism: global.c's allocno_compare ranks by floor_log2(n_refs)*n_refs/live_length, and flow.c weights REG_N_REFS by LOOP DEPTH. A single-level `do { ... } while (0);` wrap emits a NOTE_INSN_LOOP_BEG/END pair, so every reference inside the wrap is counted at depth 1 (worth 2). Wrapping the statement where p is already born therefore raises p's weighted n_refs 7 -> 8 while leaving its live_length at 7 (the wrap adds no new live range), moving p from 20000 to 34285 and past count's unchanged 8/8 = 30000. Session 3's F4 note predicted exactly this shape ("a ref inside p's existing range is worth strictly more than one that extends it") but assumed any such reference would have to be byte-free and therefore forbidden; the wrap is neither byte-free in the RTL sense nor forbidden — do-while(0) is an explicitly sanctioned device for ANY codegen effect including RA (.claude/rules/do-while-zero-exception.md:23-24).
- probe: A permuter campaign on the policy-clean 17-point chassis (tmp/grind/motion_Close/s4/wsA, hand-built byte-verified workspace, base score 468, ~11k iterations) proposed the wrap as output-463-1; every proposal was then re-measured with `sandbox motion_Close --disable all` plus the instrumented cc1 (BB2_ALLOC_DEBUG=1) in tmp/grind/motion_Close/s4/sweep4.py, and a 12-form follow-up (sweep5.py) minimised the device and probed wrap extent, wrap nesting, statement order and declaration order.
- result: `do { p = &D_8008D070; } while (0);` alone (sweep5 d4_dw0_p_only) scores 16 with p 8 refs / live_length 7 / pri 34285 in $s0 and count 8/8 / 30000 in $s1 — the target's roles, the target's loop-body registers (lw from $s0, addiu $s0,$s0,4, addiu $s1,$s1,-1, bnez $s1) and the inner guard on $s1. It scores 16 in BOTH declaration orders (d0/d2), i.e. it does not depend on the allocno tie-break at all, and it leaves the guard load in caller-saved $v0 so the prologue is not degraded the way session 3's form degraded it. This confirms frontier F4's probe 2 and makes the session-3 ruling question moot for the 16-point family.
- verdict: CONFIRMED

## [s4] With the do-while(0) priority win in place, the target's emission order (p pair before count pair) becomes compatible with keeping the roles.
- mechanism: The win now comes from a weighted reference count rather than a tie, so the hope was that it would be robust to moving p's assignment earlier — which is what fixes the remaining 2-point materialisation-order bucket (the target emits lui/addiu $s0 at insns 8-9 and lui/addiu $s1 at 10-11; our build emits them in the opposite order).
- probe: p assigned before count inside the wrap, measured in both declaration orders with sandbox + instrumented cc1 (sweep5 d1_dw0_pfirst, d3_dw0_pdecl_pfirst).
- result: Identical to session 3's tie-based failure. p's birth moves one statement earlier so its live_length grows 7 -> 8 and its priority falls 34285 -> 30000; count's birth moves one statement later so its live_length shrinks 8 -> 7 and its priority rises 30000 -> 38571. count retakes $s0, the loop body reverts to the wrong registers, score 16 -> 20 in both declaration orders. Emission order and register roles remain coupled through live_length; the coupling is a property of the function, not of the mechanism used to win the register. To break it, p would need roughly 11 weighted references against count's 38571 (p 11/8 = 41250), or count's live range must be lengthened without adding a count reference (frontier F4b).
- verdict: KILLED

## [s4] Nesting the do-while(0) wrap buys further priority separation and a lower score.
- mechanism: Nesting multiplies the loop-depth weight, so a doubly-wrapped reference should count 4 times rather than 2 and widen p's lead over count.
- probe: `do { do { count = ...; p = ...; } while (0); } while (0);` measured with sandbox + instrumented cc1 (sweep5 d5_dw0_nested2).
- result: Both allocnos are lifted together — p 9 refs / len 7 = 38571, count 10/8 = 37500 — the ordering is unchanged and the score stays at 16. The weight multiplier applies to every reference in the wrapped region, so it cannot separate two allocnos that both live there. Useful corollary: prerequisite 3 of the do-while(0) rule (a written justification that a single level was measured insufficient) never needs to be invoked on this function, because a single level is not insufficient and nesting is not better.
- verdict: KILLED

## [s4] The session-3 role-flipped (tie-pun) form is a distinct search basin worth its own permuter campaign.
- mechanism: It scores 16 against the clean chassis' 17 and has the target's loop registers, so a campaign seeded there should explore a neighbourhood the clean chassis cannot reach.
- probe: Campaign wsB seeded from the s3 form (tmp/grind/motion_Close/s4/wsB), ~4k iterations, 61 outputs, harvested by inspection of the output-* scores and the best source.
- result: The permuter's weighted base score for that chassis is 835 — far WORSE than the clean chassis' 468, because the permuter charges 60 per reordering and 100 per insertion/deletion while the engine charges per differing instruction. Every improving mutation the campaign found deletes the type pun; its best output (468) is exactly the clean chassis' base score and keeps only an rvalue cast inside the comparison. The basin drains into the clean family, so seeding there is wasted search. Secondary lesson banked: permuter score is NOT a proxy for engine distance on this function and must never be used to rank chassis.
- verdict: KILLED

## [s4] The target's $s0=p / $s1=count role split can be reached WITHOUT session 3's type-punning guard carry, by giving p a STRICT allocno priority win instead of a manufactured tie.
- mechanism: global.c's allocno_compare ranks by floor_log2(n_refs)*n_refs/live_length and flow.c weights REG_N_REFS by LOOP DEPTH. A single-level do{...}while(0) emits a NOTE_INSN_LOOP_BEG/END pair, so a reference inside the wrap counts at depth 1 (worth 2). Wrapping the statement where p is ALREADY born raises p's weighted n_refs 7->8 while leaving live_length at 7 (no new live range), moving p 20000 -> 34285, past count's unchanged 8/8 = 30000. Session 3's F4 note predicted this shape but assumed any such reference must be byte-free and therefore forbidden; do-while(0) is explicitly sanctioned for ANY codegen effect incl. register allocation (.claude/rules/do-while-zero-exception.md:23-24), single-level wraps are not exhaustion-gated, and the inline FAKE annotation is present.
- probe: Permuter campaign on the policy-clean 17-point chassis (tmp/grind/motion_Close/s4/wsA, hand-built byte-verified workspace, base score 468, ~11k iterations) proposed the wrap as output-463-1; every proposal re-measured with `sandbox motion_Close --disable all` + instrumented cc1 (BB2_ALLOC_DEBUG=1) in sweep4.py, then a 12-form follow-up (sweep5.py) minimised the device and swept wrap extent, nesting, statement order and declaration order.
- result: `do { p = &D_8008D070; } while (0);` alone (sweep5 d4_dw0_p_only) scores 16 with p 8 refs / live_length 7 / pri 34285 in $s0 and count 8/8 / 30000 in $s1 — the target's roles, the target's loop-body registers (lw from $s0, addiu $s0,$s0,4, addiu $s1,$s1,-1, bnez $s1) and the inner guard on $s1. It scores 16 in BOTH declaration orders (d0/d2), so unlike the s3 form it does not depend on the allocno tie-break, and the guard load stays in caller-saved $v0 so the prologue is not degraded to pay for the flip (session 3's form dragged it into $s0 and paid sw s0 + sw s1 in the beqz delay slot). Frontier F4 probe 2 CONFIRMED.
- verdict: CONFIRMED

## [s4] With the do-while(0) priority win in place, the target's emission order (p pair before count pair) becomes compatible with keeping the roles.
- mechanism: The win now comes from a weighted reference count rather than an exact tie, so it might be robust to moving p's assignment earlier — which is what would close the remaining 2-point materialisation-order bucket (target emits lui/addiu $s0 at insns 8-9 and lui/addiu $s1 at 10-11; our build emits them in the opposite order).
- probe: p assigned before count inside the wrap, measured in both declaration orders with sandbox + instrumented cc1 (sweep5 d1_dw0_pfirst, d3_dw0_pdecl_pfirst).
- result: Identical to session 3's tie-based failure: p's birth moves one statement earlier so live_length 7->8 and priority 34285->30000; count's birth moves one statement later so live_length 8->7 and priority 30000->38571. count retakes $s0, the loop body reverts to the wrong registers, score 16 -> 20 in BOTH declaration orders. Emission order and register roles are coupled through live_length as a property of the function, not of the mechanism used to win the register. Breaking it needs p at ~11 weighted refs (11/8 = 41250 > 38571) or count's live range lengthened without adding a count reference (F4b). Banked as rejected/dw0-pfirst-order-inverts-priority.c.
- verdict: KILLED

## [s4] Nesting the do-while(0) wrap buys further priority separation and a lower score.
- mechanism: Nesting multiplies the loop-depth weight, so a doubly-wrapped reference should count 4x rather than 2x and widen p's lead over count.
- probe: `do { do { count = ...; p = ...; } while (0); } while (0);` measured with sandbox + instrumented cc1 (sweep5 d5_dw0_nested2).
- result: Both allocnos lift together — p 9 refs / len 7 = 38571, count 10/8 = 37500 — ordering unchanged, score still 16. The multiplier applies to every reference in the wrapped region, so it cannot separate two allocnos that both live there. Corollary: prerequisite 3 of the do-while(0) rule (written justification that a single level was measured insufficient) never needs to be invoked on this function — a single level is sufficient and nesting is not better.
- verdict: KILLED

## [s4] The session-3 role-flipped (tie-pun) form is a distinct search basin worth its own permuter campaign.
- mechanism: It scores 16 against the clean chassis' 17 and already carries the target's loop registers, so a campaign seeded there should explore a neighbourhood the clean chassis cannot reach.
- probe: Campaign wsB seeded from the s3 form (tmp/grind/motion_Close/s4/wsB), ~4k iterations, 61 outputs, harvested by inspecting output-* scores and the best source.
- result: The permuter's weighted base score for that chassis is 835 — far WORSE than the clean chassis' 468, because the permuter charges 60 per reordering and 100 per ins/del while the engine charges per differing instruction. Every improving mutation deletes the type pun; the best output (468) is exactly the clean chassis' BASE score and keeps only an rvalue cast inside the comparison. The basin drains into the clean family, so seeding there is wasted search. Secondary lesson: permuter score is NOT a proxy for engine distance on this function and must not be used to rank chassis.
- verdict: KILLED

## [s4] A permuter workspace cannot be built for motion_Close because asm/funcs/motion_Close.s does not exist (the function is C-routed).
- mechanism: The usual import.py / mar_perm_workspace.sh path assembles target.o from asm/funcs/<func>.s; without that file the standard recipe aborts, which is also why tools/scan_hand_coded.py --single motion_Close aborts (noted by sessions 1-3 as a blocker).
- probe: Hand-transcribed the target region from `objdump -dr build/src/ings2.o`, assembled it to target.o, and had tmp/grind/motion_Close/s4/setup.sh VERIFY the normalized disassembly (32 lines incl. relocation records) against that region before building the workspace; compile.sh is the honest pipeline (cc1 -mel + prologue_fix + maspsx + multu_pad, no regfix/asmfix).
- result: Verified byte-identical on the first corrected comparison and used for all three campaigns. A minimal 5-declaration TU also reproduces ings2.c's full-file codegen for this function exactly (25 insns, same sequence), so the permuter did not have to process all 814 lines. The recipe is reusable for any C-routed function lacking asm/funcs/*.s — and it also unblocks F1's next probe (extract the .s, then run scan_hand_coded for a MEASURED signal tier).
- verdict: KILLED

## [s5b] H-F5: the guard-load / jalr temp can be moved from $v0 to $t0 — KILLED

STATEMENT. Frontier F5 held that the 4 points spent on $v0-vs-$t0 (guard lui/lw,
body lw, jalr) were a local-alloc decision that some C form could steer.

PROBE. `cc1 -O2 -G0 -funsigned-char -mcpu=3000 -mips1 -mno-abicalls -fno-builtin
-w -mel -da` on the floor-13 chassis (artifacts tmp/grind/motion_Close/s5b/f13.c.lreg,
f13.c.greg), then reading the allocator scan order in the frozen toolchain source.

RESULT. Both temps are block-local pseudos (74, 75) and both get hard reg 2.
local-alloc.c:2249-2262 and global.c:1057-1062/1203-1209 scan hard regs ascending
because the MIPS backend defines no REG_ALLOC_ORDER, so $v0 is the first
allocatable GR_REGS register and wins whenever free. $t0 = hard reg 8 requires
regs 2-7 all excluded, i.e. six live call-clobbered values over both ranges;
the target's instruction stream contains none (no-arg, void call), so any C
form manufacturing them emits instructions the target does not have.

VERDICT: KILLED, structurally. Do not re-probe. See
rejected/f5-t0-unreachable-alloc-scan-order.c.

## [s5b] H-PERM3: a permuter campaign on the floor-13 chassis finds a sub-13 form — KILLED for this basin

PROBE. tools/permuter_campaign.py launch/wait/harvest on tmp/grind/motion_Close/s5b/wsA
(the floor-13 chassis, base permuter score 413, -j 6), three in-turn wait windows,
55,660 iterations / 1421 s, harvested with --stop.

RESULT. Two novel finds, best permuter 259. Re-measured with the engine both are
worthless: the 259 find (a dead volatile local) scores 13 with two EXTRA emitted
instructions and is a frame-coercion cheat besides; the 370 find re-treads the
already-dead guard-staging family. Zero engine-gradient movement from 55k
iterations on this basin.

VERDICT: KILLED for this chassis. Combined with the F5 and H1 kills the basin is
explained: 11 of the 13 residual points are structurally unreachable, so there is
no gradient for a randomizer to descend. A future permuter session should not
reseed this chassis; only the 2 open points (beqz delay slot, prologue save order)
are worth a directed attack, and they are worth at most 2.

## [s5] The floor-13 form described only in a comment line of rejected/pfirst-wrap-depth-1-and-2-insufficient.c (p assigned first, three-level do-while(0) wrap around p's initialiser) really does measure 13 on the honest gradient.
- mechanism: flow.c weights REG_N_REFS by loop depth and each do{}while(0) emits a NOTE_INSN_LOOP_BEG/END pair, so three levels of nesting lift p to 10 weighted refs / live_length 8 = allocno priority 37500, past count's 34285, giving p $s0 while KEEPING the target's p-then-count address-materialisation order. Depth 1 and 2 measured insufficient (both score 20); depths 4-6 buy nothing.
- probe: Applied the reconstructed body to src/ings2.c and ran `sandbox motion_Close --disable all`.
- result: score 13, target_insns 26, build_insns 25 - confirmed, and re-confirmed at the end of the session with the same body still in src/. Banked into memory/grind/motion_Close/candidate.c with full mechanism + policy write-up (the file previously held session 4's floor-16 form).
- verdict: CONFIRMED

## [s5] Frontier F5: the D_800A2668 guard load and the indirect-call target, which land in $v0 in every form measured across five sessions, can be steered to the target's $t0 by some C form (worth 4 of the 13 remaining points).
- mechanism: F5 assumed this was a steerable local-alloc.c decision over call-clobbered registers that no allocno-priority lever had yet touched. It is not steerable: local-alloc.c:2249-2262 and global.c:1057-1062/1203-1209 both scan hard registers in ASCENDING NUMERIC ORDER (`#ifdef REG_ALLOC_ORDER ... #else int regno = i; #endif`) and REG_ALLOC_ORDER is undefined in the MIPS backend, so $v0 (hard reg 2) is the first allocatable GR_REGS register and is taken whenever free. Reaching $t0 (hard reg 8) requires hard regs 2,3,4,5,6,7 all excluded over BOTH live ranges, i.e. six simultaneously live call-clobbered values - and the target's byte stream contains no value in $v0/$v1/$a0-$a3 anywhere, because the call takes no arguments and returns void. Manufacturing those values emits instructions the target does not contain, adding distance faster than it removes it.
- probe: cc1 -da on the floor-13 chassis; read f13.c.lreg (Register 74 used 2 times across 4 insns in block 0; Register 75 used 4 times across 4 insns in block 2 - both block-local, so local-alloc owns them) and f13.c.greg (`;; Register dispositions: 72 in 17  73 in 16  74 in 2  75 in 2`); then grepped the frozen toolchain for REG_ALLOC_ORDER and read both allocators' find_reg scans.
- result: KILLED. Both temps are block-local pseudos assigned hard reg 2 by first-free ascending scan; no C form can exclude regs 2-7 without emitting instructions absent from the target. Banked as rejected/f5-t0-unreachable-alloc-scan-order.c.
- verdict: KILLED

## [s5] A permuter campaign seeded on the floor-13 chassis finds a sub-13 form.
- mechanism: Directed randomization on the closest known chassis is the standard way to surface a lever no hand-derivation reached; the s4 campaign on the floor-17 chassis is what produced the do-while(0) family in the first place.
- probe: tools/permuter_campaign.py launch --dir tmp/grind/motion_Close/s5b/wsA --label pfirst_wrap3_floor13 -j 6 (base permuter score 413), three in-turn `wait` windows, then harvest --stop. 55,660 iterations over 1421 s.
- result: KILLED for this basin. Exactly two novel finds. `output-259-1` (permuter 259, the best drop of the session) adds `volatile unsigned int new_var; new_var = 0;` - re-measured with the engine it scores 13, IDENTICAL to the chassis, with build_insns rising 25 -> 27; it is also an unused-local frame-coercion cheat outside the written-never-read carve-out (the target contains no such dead store), so it was rejected in-session and never surfaced as a candidate (rejected/volatile-dead-local-engine-neutral.c). `output-370-1` (permuter 370) re-treads the already-dead guard-staging family. Zero engine-gradient movement, which the F5+H1 kills explain: 11 of 13 residual points are structurally unreachable, so there is no gradient for a randomizer to descend.
- verdict: KILLED

## [s6] F7a — the target's ASCENDING prologue save order (s0,s1,ra) is reachable from pure C.
- mechanism: mips.c:save_restore_insns emits callee-saved stores with `for (regno = GP_REG_LAST; regno >= GP_REG_FIRST; regno--)` (source comment: "Save registers starting from high to low"), so cc1 always emits ra,s1,s0. The only pass that can reorder them is sched2, a per-BASIC-BLOCK list scheduler, and it reorders only under a dependence.
- probe: (1) read save_restore_insns in the frozen toolchain; (2) raw `cc1 -da` on the floor-13 chassis (tmp/grind/motion_Close/s6/f13.s, f13.c.sched2, f13.c.dbr); (3) corpus scan of all 1788 oracle-build functions for save-order shape (tmp/grind/motion_Close/s6/save_order_scan.py), then isolation of the 29 functions with motion_Close's exact {s0,s1,ra} save mask (asc_prologues.py).
- result: 126 of 606 multi-save functions DO emit ascending order, so the emission loop alone is not a kill — the reversal comes from a WAR anti-dependence: the prologue block also writes the saved register (nearly always `move sN,aM`, an incoming-argument copy), forcing `sw sN` ahead of it. Filtering the 29 same-mask functions for "no write to s0/s1 in the prologue block" leaves exactly TWO in the entire corpus — motion_Close and its cheat-carrying sibling func_80083794. motion_Close is void(void): it has no argument to copy, and its only s0/s1 writes are the two address materialisations that the TARGET places after the guard branch, i.e. in a different basic block, where GCC 2.7.2's block-local scheduler cannot see them. Hoisting them into the guard block was already measured dead (s3, score 19). KILLED.
- verdict: KILLED

## [s6] F7b — the target's EMPTY beqz delay slot (nop where our build puts `sw s0,16(sp)`) is reachable from pure C.
- mechanism: reorg.c:2941-3012 `fill_simple_delay_slots` scans backward from the branch, stopping only at a label or jump, and takes the first single insn that neither sets nor references the branch's resources. The target's guard branch tests $t0 and is preceded IN THE SAME BLOCK by `addiu sp,sp,-16` and the three saves — four eligible single insns — so reorg must fill it.
- probe: f13.c.dbr on the floor-13 chassis (the fill is visible as `(insn 116 (sequence[ (jump_insn 11 ...) (insn 109 sw s0,16(sp)) ]))`), plus a corpus scan for conditional branches immediately preceded by a callee-saved `sw` (tmp/grind/motion_Close/s6/prologue_scan.py) and inspection of every empty-slot case (empty_slot_cases.sh/.log).
- result: 118 such branches in the oracle build; only 6 have an empty slot, one of them being motion_Close. In all five others the preceding `sw` is itself already inside an EARLIER jal/branch delay slot, so the conditional branch begins a fresh basic block with nothing eligible before it (func_800278C0, func_80036E34, snd_LoadBgm, func_80074488; func_8002304C is prologue_config-rewritten). Zero compiled-C counterexamples: no function leaves the slot empty while an eligible insn precedes it in the block. The target's shape (four eligible insns + nop) is not compiler output. Also learned incidentally: a 2-insn `la` pseudo is ineligible for a slot, which is why the INNER guard's slot is empty in our build too and costs nothing. KILLED.
- verdict: KILLED

## [s6] F8 probe — motion_Close carries STRONG hand-coded signals (the canonical-asm endgame gate).
- mechanism: `tools/scan_hand_coded.py` scores 8 signals; the endgame-lock gate accepts only STRONG tiers driven by S1 (multu pacing), S2 (empty-body branches) or S6 (BIOS jumptable). Sessions 1-4 could never run it because the function is C-routed and asm/funcs/motion_Close.s does not exist.
- probe: synthesised asm/funcs/motion_Close.s from the session-4 byte-verified target.s, ran `--single motion_Close` and `--single func_80083794`, then deleted the synthesised file. Log: tmp/grind/motion_Close/s6/scan_hand_coded.log.
- result: motion_Close tier=LOW score=0/8 ("no strong hand-coded indicators"); S1/S2/S6 all negative, S3/S4 skipped as too short (26 < 40 insns). Sibling func_80083794 also LOW 0/8. The canonical-asm gate is a MEASURED FAILED GATE. Note this does not contradict the F1/F7 structural findings — scan_hand_coded's signal set simply does not include "prologue shape no compiler emits"; the corpus evidence is the stronger instrument here, and it is the thing an escalation must cite.
- verdict: KILLED (as a gate; the function scores LOW)

## [s6] F7a — the target's ASCENDING prologue save order (sw s0,4 / sw s1,8 / sw ra,12) is reachable from pure C.
- mechanism: mips.c:save_restore_insns emits callee-saved stores with `for (regno = GP_REG_LAST; regno >= GP_REG_FIRST; regno--)` (source comment: 'Save registers starting from high to low'), so cc1 always emits ra,s1,s0 — confirmed on the floor-13 chassis (tmp/grind/motion_Close/s6/f13.s: sw $31,24 / sw $17,20 / sw $16,16). The only pass that can reorder them is sched2, which is a per-BASIC-BLOCK list scheduler and therefore needs a dependence inside the prologue block.
- probe: Read save_restore_insns in the frozen toolchain; ran cc1 -da on the floor-13 chassis (f13.s, f13.c.sched2, f13.c.dbr); scanned all 1788 oracle-build functions for save-order shape (save_order_scan.py) and isolated the 29 with motion_Close's exact {s0,s1,ra} mask (asc_prologues.py).
- result: 126 of 606 multi-save functions DO emit ascending order, so the emission loop alone is NOT a kill (the naive reading is wrong and must not be re-asserted). The reversal comes from a WAR anti-dependence: the prologue block also writes the saved register — nearly always an incoming-argument copy `move sN,aM` — forcing `sw sN` ahead of it. Filtering the 29 same-mask functions for 'no write to s0/s1 in the prologue block' leaves EXACTLY TWO in the whole corpus: motion_Close and its cheat-carrying sibling func_80083794, i.e. zero compiled-C instances. motion_Close is void(void) so there is no argument copy, and its only s0/s1 writes are the two address materialisations that the TARGET ITSELF places after the guard branch (0xf88-0xf94) — a different basic block from the saves (0xf74-0xf7c) — where GCC 2.7.2's block-local scheduler cannot see them. Hoisting them into the guard block was already measured dead in s3 (score 19).
- verdict: KILLED

## [s6] F7b — the target's EMPTY beqz delay slot (nop where our build lands `sw s0,16(sp)`) is reachable from pure C.
- mechanism: reorg.c:2941-3012 fill_simple_delay_slots scans BACKWARD from the branch (`for (trial = prev_nonnote_insn (insn); ! stop_search_p (trial, 1); ...)`), stopping only at a label or jump, and takes the first single insn that neither sets nor references the branch's resources. The target's guard branch tests $t0 and is preceded in the SAME block by `addiu sp,sp,-16` plus the three saves — four eligible single insns, none touching $t0 — so reorg is obliged to fill it.
- probe: f13.c.dbr on the floor-13 chassis (the fill is explicit: `(insn 116 (sequence[ (jump_insn 11 ...) (insn 109 = sw s0,16(sp)) ]))`), plus a corpus scan for conditional branches immediately preceded by a callee-saved `sw` (prologue_scan.py) and inspection of every empty-slot case (empty_slot_cases.sh/.log).
- result: 118 such conditional branches in the oracle build; exactly 6 leave the slot empty and one of the 6 IS motion_Close. In all five others the preceding `sw` is itself already occupying an EARLIER jal/branch delay slot, so the conditional branch begins a fresh basic block with nothing eligible before it (func_800278C0, func_80036E34, snd_LoadBgm, func_80074488; func_8002304C is prologue_config-rewritten). Zero counterexamples: no compiled function leaves a conditional-branch delay slot empty while an eligible single insn precedes it in the block. Incidental: a 2-insn `la` pseudo is ineligible for a slot, which is why the INNER guard's slot is empty in our build too and costs nothing.
- verdict: KILLED

## [s6] F8 probe — motion_Close carries STRONG hand-coded signals (S1/S2/S6), the canonical-asm endgame gate.
- mechanism: tools/scan_hand_coded.py scores 8 signals and the endgame-lock gate accepts only STRONG tiers driven by S1 (multu pacing), S2 (empty-body branch) or S6 (BIOS jumptable). Sessions 1-4 could never run it because the function is C-routed and asm/funcs/motion_Close.s does not exist.
- probe: Synthesised asm/funcs/motion_Close.s from the session-4 byte-verified target.s, ran `python3 tools/scan_hand_coded.py --single motion_Close` and `--single func_80083794`, then DELETED the synthesised file so no repo dirt remains. Log: tmp/grind/motion_Close/s6/scan_hand_coded.log.
- result: motion_Close tier=LOW score=0/8, 'no strong hand-coded indicators' — S1, S2 and S6 all negative; S3/S4 skipped as too short (26 < 40 insns). Sibling func_80083794 (real asm/funcs file, 54 insns) also LOW 0/8. The canonical-asm gate is therefore a MEASURED FAILED GATE, not an open question. Note the tool's signal set does not include 'prologue shape no compiler emits', so the corpus evidence above is the stronger instrument and is what an escalation entry must cite.
- verdict: KILLED

## [s7] F9 — some C-level call construct can leave current_function_outgoing_args_size at 0, so H1's 16-byte outgoing-arg area is avoidable in pure C after all.
- mechanism: H1 was established in session 1 by a CENSUS ("no function in the oracle build has a sub-16 arg area except this crt0 pair, and both only because their jalr is inline asm"). A census is an argument about our build, not about the backend: it cannot rule out a C construct that simply never occurs in BB2. F9 asked the complementary question directly — enumerate every path in the frozen compiler that can emit a call insn, and check whether ANY of them can leave `current_function_outgoing_args_size` (cfoas) at zero, since `mips.c:compute_frame_size` computes `args_size = MIPS_STACK_ALIGN (cfoas)` and reads nothing else.
- probe: (a) Read the frozen toolchain. `REG_PARM_STACK_SPACE(FNDECL)` is `mips.h:1822` = `(MAX_ARGS_IN_REGISTERS*UNITS_PER_WORD) - FIRST_PARM_OFFSET(FNDECL)`; `MAX_ARGS_IN_REGISTERS` is 4 (`mips.h:1888`) and `FIRST_PARM_OFFSET` is the `#else` arm of an `#if 0` (`mips.h:1801-1812`), i.e. literally 0 — so the macro is the compile-time constant 16 for EVERY fndecl, prototyped or not. `MAYBE_REG_PARM_STACK_SPACE` and `FINAL_REG_PARM_STACK_SPACE` are undefined for MIPS (grepped `config/mips/`), so the two escape hatches in `calls.c` that could zero it are dead code; `OUTGOING_REG_PARM_STACK_SPACE` IS defined (`mips.h:1830`), so the `#ifndef` subtraction at `calls.c:1253` that would take it back off is also dead. What remains is `calls.c:1245-1247` `args_size.constant = MAX (args_size.constant, reg_parm_stack_space);` followed by `calls.c:1394-1401` `if (needed > cfoas) cfoas = needed;` — unconditional in the `args_size.var == 0` branch and not gated on `must_preallocate`. (b) Enumerate every `emit_call_insn` call site in the compiler (excluding emit-rtl.c's definition and genemit.c's generator): `calls.c:387/398/403` (emit_call_1, reached from both expand_call and emit_library_call, the latter applying the same MAX at `calls.c:2393-2401` and `2743-2751`); `integrate.c:1807`, `unroll.c:1948`, `loop.c:1801/1845` (all COPY an already-expanded call insn, and `integrate.c:1358` additionally propagates the inlinee's `OUTGOING_ARGS_SIZE` into the caller by MAX); `expr.c:8361/8382` inside `expand_builtin_apply`, which is the ONLY site in the compiler that emits a call without touching cfoas at all. (c) Measure it rather than assert it: 8 probe TUs compiled with the project's canonical cc1 flags (`-O2 -G0 -funsigned-char -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel`), reading the emitted `.frame ... args=` field. Scripts tmp/grind/motion_Close/s7/f9probe.sh and f9probe2.sh, output in tmp/grind/motion_Close/s7/out/.
- result: KILLED. Every ordinary C call shape emits `args= 16`: the motion_Close-shaped indirect call through a table (`.frame $sp,32,$31 # vars=0, regs=3/0, args=16`), a plain direct call (24/args=16), an UNPROTOTYPED call (24/args=16 — so the "a prototype gcc treats specially" idea in F9's own next-probe note is dead), a call to an `__attribute__((const))` function (24/args=16), a call inlined from a static callee (24/args=16, so integrate.c's copy path does not lose the accounting), and a call inside a loop (24/args=16). A leaf function with no call at all is the only ordinary shape with `args= 0` (`.frame $sp,0,$31`), which is exactly the point: the 16 appears the moment a call does. The single backend hole is real but useless — `__builtin_apply` emits `args= 0`, but it does so at a cost of `.frame $fp,72,$31 # vars= 56, regs= 3/0`: a frame POINTER, 56 bytes of vars, `$fp` in the save mask and 34 emitted instructions, against the target's 26 instructions, 16-byte frame, no frame pointer and a `{s0,s1,ra}` mask. It cannot produce the target's bytes, and `__builtin_apply` is a GCC extension that no 1998 PsyQ crt0 would contain. H1 is therefore upgraded from "no function in this build does this" to "the backend cannot do this": the 16-byte outgoing-argument area is a compile-time constant applied by a MAX on every call-expansion path, and there is no C spelling that avoids it while still emitting a call.
- verdict: KILLED

## [s7] The 13-point residual at the current floor is EXACTLY accounted for by the three killed mechanisms, with no unexplained instruction.
- mechanism: An escalation packet has to carry a table, not a narrative. Session 2 did this at floor 17 and session 6 asserted "13/13 explained" in prose; what was missing was the instruction-by-instruction pairing at the CURRENT floor.
- probe: Paired the 26 target instructions (tmp/grind/motion_Close/s5b/wsA/_tgt_mine.txt, byte-verified in s4 against `objdump -dr build/src/ings2.o`) against the 25 build instructions (_base.txt) with the candidate.c chassis in src/ings2.c, and classified every differing position. Table: tmp/grind/motion_Close/s7/residual_table.md.
- result: CONFIRMED, and the count is exact. 13 differing positions, measured score 13. Buckets: 5 positions are the $v0-vs-$t0 temp (insns 1, 2, 7, 15, 17) = frontier F5, killed s5b on the ascending first-free hard-reg scan; 6 positions plus the offset half of a 7th are frame size and save/restore offsets (3, 5, 21, 22, 23, 24, and the offset component of 4) = H1, killed s1 by census and s7 at the backend level; 3 positions are the ascending save ORDER and the empty beqz delay slot (the order component of 4, plus 6 and 8) = F7a/F7b, killed s6 with corpus measurements. Nothing is attributed to an unexplored direction.
- verdict: CONFIRMED

## [s7] F9 — some C-level call construct can leave current_function_outgoing_args_size at 0, so H1's 16-byte outgoing-argument area is avoidable in pure C after all.
- mechanism: H1 rested on a CENSUS of the oracle build (no call-making function has a sub-16 arg area except this crt0 pair, and both only because their jalr is inline asm). A census is an argument about OUR build, not about the backend: it cannot rule out a C construct that simply never occurs in BB2. mips.c:4466 (compute_frame_size) takes current_function_outgoing_args_size (cfoas) as the SOLE input to the frame's argument area, with one adjustment that only ever RAISES it (args_size==0 && calls_alloca -> 4*UNITS_PER_WORD). So the question reduces to: can any path that emits a call insn leave cfoas at zero?
- probe: (a) Read the frozen toolchain. REG_PARM_STACK_SPACE(FNDECL) (mips.h:1822) = MAX_ARGS_IN_REGISTERS(4, mips.h:1888) * UNITS_PER_WORD(4) - FIRST_PARM_OFFSET, and FIRST_PARM_OFFSET is the #else arm of an #if 0 block (mips.h:1801-1812), i.e. literally 0 — so the macro is the compile-time constant 16 for EVERY fndecl, prototyped or not. MAYBE_REG_PARM_STACK_SPACE and FINAL_REG_PARM_STACK_SPACE are undefined in config/mips/ (grepped), so the two escape hatches in calls.c that could zero it are dead code; OUTGOING_REG_PARM_STACK_SPACE IS defined (mips.h:1830), so the #ifndef-guarded subtraction at calls.c:1253 that would take the 16 back off is dead too. What survives is calls.c:1245-1247 'args_size.constant = MAX (args_size.constant, reg_parm_stack_space);' feeding calls.c:1394-1401 'if (needed > cfoas) cfoas = needed;', which is unconditional in the args_size.var==0 branch and NOT gated on must_preallocate. (b) Enumerate every emit_call_insn site in the compiler (excluding emit-rtl.c's definition and genemit.c's generator): calls.c:387/398/403 (emit_call_1, reached from both expand_call and emit_library_call, the latter applying the same MAX at calls.c:2393-2401 and 2743-2751); integrate.c:1807, unroll.c:1948, loop.c:1801/1845 (all COPY an already-expanded call insn, and integrate.c:1358 propagates the inlinee's OUTGOING_ARGS_SIZE into the caller by MAX); expr.c:8361/8382 inside expand_builtin_apply — the ONLY site in the compiler that never touches cfoas. (c) Measure rather than assert: 8 probe TUs compiled with the project's canonical cc1 flags (-O2 -G0 -funsigned-char -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel), reading the emitted '.frame ... args=' field. Scripts tmp/grind/motion_Close/s7/f9probe.sh and f9probe2.sh; emitted assembly in tmp/grind/motion_Close/s7/out/.
- result: Every ordinary C call shape emits args= 16: the motion_Close-shaped indirect call through a table (.frame $sp,32 vars=0 regs=3/0 args=16), a plain direct call (24/args=16), an UNPROTOTYPED call (24/args=16 — killing F9's own 'a call through a pointer with a prototype gcc treats specially' idea), a call to an __attribute__((const)) function (24/args=16), a call inlined from a static callee (24/args=16, so integrate.c's copy path does not lose the accounting), and a call inside a loop (24/args=16). A leaf function with no call is the only ordinary shape with args= 0 (.frame $sp,0), which is exactly the point: the 16 appears the moment a call does. Note every probe is already compiled with -mno-abicalls, the canonical build flag, so F9's '-mno-abicalls interactions' leg is measured dead too. The one real backend hole is __builtin_apply (expr.c:8361), which does emit args= 0 — and is useless: .frame $fp,72,$31 with vars=56, regs=3/0, $fp in the save mask and 34 emitted instructions (plus a dynamic arg-block push, an emit_block_move of the incoming argument area and a reload of $a0-$a3), against the target's 16-byte frame, no frame pointer, a {s0,s1,ra} mask and 26 instructions. It cannot produce the target's bytes, and it is a GCC extension no 1998 PsyQ crt0 would contain. Banked as rejected/f9-builtin-apply-only-zero-arg-area-but-frame-pointer.c.
- verdict: KILLED

## [s7] The 13-point residual at the current floor is EXACTLY accounted for by the three already-killed mechanisms, with no unexplained instruction.
- mechanism: An escalation packet has to carry a table, not a narrative. Session 2 produced a block-level attribution at floor 17 and session 6 asserted '13/13 explained' in prose; what was missing was the instruction-by-instruction pairing at the CURRENT floor of 13.
- probe: Applied memory/grind/motion_Close/candidate.c to src/ings2.c and re-measured (sandbox motion_Close --disable all -> score 13, target_insns 26, build_insns 25, this session). Then paired the 26 target instructions (tmp/grind/motion_Close/s5b/wsA/_tgt_mine.txt, byte-verified in s4 against objdump -dr build/src/ings2.o) against the 25 build instructions (_base.txt) and classified every differing position. Table: tmp/grind/motion_Close/s7/residual_table.md. src/ings2.c was restored to HEAD afterwards so the tree still builds byte-identical.
- result: 13 differing positions, measured score 13 — the attribution is exhaustive. Buckets: FIVE positions are the $v0-vs-$t0 temp (guard lui, guard lw, guard beq, loop lw, jalr = insns 1, 2, 7, 15, 17) = frontier F5, killed s5b on the ascending first-free hard-reg scan (no MIPS REG_ALLOC_ORDER); SIX positions plus the offset component of a seventh are frame size and save/restore offsets (3, 5, 21, 22, 23, 24 and half of 4) = H1, killed s1 by census and now s7 at the backend level; THREE positions are the ascending prologue save ORDER and the empty beqz delay slot (the order component of 4, plus 6 and 8) = F7a/F7b, killed s6 with corpus measurements (2 instances of the save shape in 1788 functions, both this cheat-carrying crt0 pair; 0 compiled-C counterexamples for the empty slot). Nothing is attributed to an unexplored direction.
- verdict: CONFIRMED

## [s8] F10 � a WHOLE-TU / file-level reshape (shared static helper, inlined body, different signature, different declaration order) moves one of the three surviving residual mechanisms.
- mechanism: Every form measured in sessions 1-7 varies the BODY of motion_Close inside the existing ings2.c. Two of the three surviving mechanisms (F7a's prologue-block WAR anti-dependence, F7b's delay-slot eligibility) are properties of the FUNCTION's shape as a whole - its signature, its callee-save set, its block structure - not of the loop's spelling, so a TU-level change was the one untried direction that was not a body-level respelling. motion_Close and func_80083794 are the same crt0 open/close pair and differ only in guard polarity and the D_800A2668 store, so the sibling is a free second data point on any TU-level change.
- probe: tmp/grind/motion_Close/s8/tusweep.py replaced the ENTIRE two-function region of src/ings2.c per variant (nine variants: control, three inlined-helper shapes, a non-inlined static helper, TU order swap, and three signature reshapes), each measured with `sandbox motion_Close --disable all` plus the instrumented cc1's per-allocno table (BB2_ALLOC_DEBUG=1). tmp/grind/motion_Close/s8/depthsweep.py then crossed the two inlined chassis with do-while(0) wrap depths 0/1/2/3 to test F4b on the new chassis.
- result: The TU-level axis is CODEGEN-INERT. v0_base13 13, v3_shared_inline_both 13 (BOTH functions rewritten as one parameterised `static __inline__ ctor_walk(void)` body), v7_order_swap 13 (motion_Close defined before its sibling) - and not merely equal in score: the allocno tables are IDENTICAL (p 10 refs / live_length 8 / pri 37500 -> $s0; count 8/7 / 34285 -> $s1) and so are the emitted instructions. The wrap-depth cross reproduces the direct-body ladder value for value on BOTH inlined chassis (depth 0: p 7/8 = 17500, score 20; depth 1: 8/8 = 30000, 20; depth 2: 9/8 = 33750, 20; depth 3: 10/8 = 37500, 13), so GCC 2.7.2 inlines before flow.c counts references and an inlined body is the same input as a written-out body to every pass that matters. Helper shapes that pass values as parameters are WORSE (helper taking count 16, helper taking (p, count) 20), and a plain non-inlined `static` helper is 23 (15 insns, frame 24 = 16 args + ra, the call moved out of motion_Close). F4b is not helped either: depth 3 remains necessary and minimal on every chassis. Banked as rejected/f10-tu-level-shape-is-codegen-inert.c.
- verdict: KILLED

## [s8] F7a - the target's ASCENDING prologue save order is reachable if motion_Close's prologue block is given the incoming-argument WAR anti-dependence that session 6 named as the missing ingredient.
- mechanism: The target saves `sw $s0,4($sp)` / `sw $s1,8($sp)` / `sw $ra,12($sp)` - ascending register number; our honest build emits `sw $ra,24` / `sw $s1,20` / `sw $s0,16` - descending. (The offset-to-register MAPPING is identical in both - ra highest, then s1, then s0 - only the store order in the stream differs.) Session 6 killed this with the finding that flipping the order needs a WAR anti-dependence inside the prologue basic block, in practice an incoming argument copy `move sN,aM`, which a void(void) function cannot have. That disproof was CONDITIONAL on the signature, and F10 named it as the one residual mechanism a TU-level reshape might reach.
- probe: tmp/grind/motion_Close/s8/paramsweep.py gave motion_Close incoming parameters on purpose (an ABI lie - never submittable, the function is a crt0 destructor-table walker called with no arguments - and used only to answer the mechanism question), and dumped the emitted prologue from the instrumented cc1 for each. Then the emission site itself was read in the frozen backend: `save_restore_insns`, tools/gcc-2.7.2/config/mips/mips.c:4680.
- result: DOUBLE KILL. (a) Empirically, w4_param_both_used - two genuine `move sN,aM` copies, each WAR-dependent on the save above it, both inside the prologue block, i.e. exactly session 6's missing ingredient - emits `sw s1,20 / move s1,a0 / sw s0,16 / move s0,a1 / beqz / sw ra,24`: the scheduler duly interleaves the copies and pushes `sw $ra` out of first position into the delay slot, but the s0/s1 saves REMAIN DESCENDING (score 21). w5_param_p_used (one copy) gives `sw s1,20 / move s1,a0 / sw ra,24 / beqz / sw s0,16`, score 19. Dead parameters (w1/w2/w3) are optimised away and score 13, identical to the void(void) control. (b) The reason is emission, not schedulability: mips.c:4680 is `for (regno = GP_REG_LAST; regno >= GP_REG_FIRST; regno--)` with gp_offset initialised to the TOP of the save area and decremented per store (mips.c:4710). The direction is unconditional - no target flag, no frame-pointer case, no ABI case selects the other one, and the comment at mips.c:4609-4612 states the high-to-low order is a deliberate debugger/epilogue convention. Saves are ALWAYS emitted ra, s1, s0; only a block-local scheduling move can reorder them, and the scheduler breaks ties between two structurally symmetric stores by program order, which IS the descending emission order. So the mechanism session 6 called NECESSARY is now also measured INSUFFICIENT, and F7a joins H1 at the backend-disproof tier: the ascending save order is not a property of any C input to this compiler. Banked as rejected/f7a-arg-copy-does-not-flip-save-order.c.
- verdict: KILLED

## [s8] F10 — a WHOLE-TU / file-level reshape (both functions written as one parameterised static helper, the body arriving by inlining, a different declaration order in the TU) moves one of the three surviving residual mechanisms (H1 frame, F5 $v0-vs-$t0, F7a/F7b prologue order + delay slot).
- mechanism: Every form measured in sessions 1-7 varies the BODY of motion_Close inside the existing ings2.c. Two of the three surviving mechanisms are properties of the FUNCTION's shape as a whole — its signature, its callee-save set, its block structure — not of the loop's spelling, so a TU-level change was the one untried direction that was not a body-level respelling. motion_Close and func_80083794 are the same crt0 open/close pair differing only in guard polarity and the D_800A2668 store, so the sibling is a free second data point on any TU-level change.
- probe: tmp/grind/motion_Close/s8/tusweep.py replaced the ENTIRE two-function region of src/ings2.c per variant (nine variants: control, three inlined-helper shapes, a non-inlined static helper, a TU order swap, three signature reshapes), measuring each with `sandbox motion_Close --disable all` plus the instrumented cc1 per-allocno table (BB2_ALLOC_DEBUG=1). tmp/grind/motion_Close/s8/depthsweep.py then crossed the two inlined chassis with do-while(0) wrap depths 0/1/2/3 to retest F4b on the new chassis.
- result: CODEGEN-INERT. v0_base13 13, v3_shared_inline_both 13 (BOTH functions rewritten as one parameterised `static __inline__ ctor_walk(void)` body), v7_order_swap 13 (motion_Close defined before its sibling) — and not merely score-equal: identical allocno tables (p 10 refs / live_length 8 / pri 37500 -> $s0; count 8/7 / 34285 -> $s1) and identical emitted instructions. The wrap-depth cross reproduces the direct-body ladder value for value on BOTH inlined chassis (depth 0: p 7/8 = 17500, 20; depth 1: 8/8 = 30000, 20; depth 2: 9/8 = 33750, 20; depth 3: 10/8 = 37500, 13), so GCC 2.7.2 inlines before flow.c counts references and an inlined body is the same input as a written-out body. Parameterised helpers are worse (helper taking count 16, helper taking (p, count) 20) and a plain non-inlined static helper is 23 (motion_Close down to 15 insns with .frame $sp,24 args=16 — the call and its 16-byte arg area simply move to the other function). F4b is not helped either: depth 3 remains necessary and minimal on every chassis. Banked as rejected/f10-tu-level-shape-is-codegen-inert.c.
- verdict: KILLED

## [s8] F7a — the target's ASCENDING prologue save order (sw $s0,4 / sw $s1,8 / sw $ra,12) is reachable if motion_Close's prologue block is given the incoming-argument WAR anti-dependence session 6 named as the missing ingredient.
- mechanism: Our honest build emits sw $ra,24 / sw $s1,20 / sw $s0,16 — descending register number. (The offset-to-register MAPPING is identical in target and build: ra highest, then s1, then s0; only the store ORDER in the stream differs, and the offsets differ solely by the H1 16-byte shift.) Session 6 killed the flip subject to motion_Close being void(void), which cannot carry a `move sN,aM` incoming-argument copy; F10 named that conditionality as the one opening a TU-level reshape might reach.
- probe: tmp/grind/motion_Close/s8/paramsweep.py gave motion_Close real incoming parameters on purpose (an ABI lie — the function is a crt0 destructor-table walker called with no arguments — measured only to answer the mechanism question) and dumped the emitted prologue from the instrumented cc1 for each of five cells. The emission site itself was then read in the frozen backend: save_restore_insns, tools/gcc-2.7.2/config/mips/mips.c:4680.
- result: DOUBLE KILL. (a) Empirically, w4_param_both_used — two genuine `move sN,aM` copies, each WAR-dependent on the save above it, both inside the prologue block, i.e. exactly the ingredient s6 named — emits `sw s1,20 / move s1,a0 / sw s0,16 / move s0,a1 / beqz / sw ra,24` (score 21): the scheduler interleaves the copies and pushes sw $ra out of first position into the delay slot, but the s0/s1 saves REMAIN DESCENDING. w5_param_p_used gives `sw s1,20 / move s1,a0 / sw ra,24 / beqz / sw s0,16` (19). Dead parameters (w1/w2/w3) are optimised away and score 13, identical to the void(void) control. (b) The reason is emission, not schedulability: mips.c:4680 is `for (regno = GP_REG_LAST; regno >= GP_REG_FIRST; regno--)` with gp_offset initialised to the top of the save area (mips.c:4623) and decremented per emitted store (mips.c:4710); the direction is unconditional — no target flag, no frame-pointer case, no ABI case selects the other one, and mips.c:4609-4612 states high-to-low is a deliberate debugger/epilogue convention. GP saves are ALWAYS emitted ra, s1, s0, and the scheduler breaks ties between two structurally symmetric stores by program order, which is that same descending order. The mechanism s6 called NECESSARY is now also measured INSUFFICIENT, so F7a joins H1 at the backend-disproof tier. Banked as rejected/f7a-arg-copy-does-not-flip-save-order.c.
- verdict: KILLED

## [s9] F11 - the DECLARATION of the three globals motion_Close touches is an input that can move one of the three surviving residual mechanisms.
- mechanism: Sessions 1-7 measured the BODY exhaustively and session 8 measured everything ABOVE the body (signature, inlining, TU order, helper parameterisation) codegen-inert, leaving exactly one input to cc1 nobody had varied: how D_800A2668 / D_8008D070 / D_00000000 are DECLARED. Array types, a struct wrapping the pair so one address materialisation could serve both, a const-qualified table, an `extern char` for the linker-provided count symbol, or a reordering of the three externs are all honest declaration changes, and any of them that changed the instruction count would force the s7 residual table to be re-derived rather than reused.
- probe: tmp/grind/motion_Close/s9/declsweep.py - ten declaration forms (base; array-typed table with `p = D_8008D070`; array-typed count; both arrays; `extern char D_00000000[]`; `extern void (*const D_8008D070)(void)` with p as `void (*const *)`; `extern u32 D_800A2668`; the three externs emitted in reverse order; `extern void *D_00000000`; array table + char count together), each applied to the extern lines AND to both users of the symbols (motion_Close and its sibling func_80083794), with the floor-13 body held fixed, each measured with `sandbox motion_Close --disable all` AND with the instrumented cc1 (BB2_ALLOC_DEBUG=1) reading the per-allocno table. No `asm("Sym")` alias-rename variant was written or measured - that is a forbidden family (.claude/rules/inline-asm-injection.md, alias renames section).
- result: ALL TEN score 13 with build_insns 25 and the byte-identical allocno table `ord=0 pseudo=73 hardreg=16 nrefs=10 livelen=8 pri=37500` (p -> $s0) / `ord=1 pseudo=72 hardreg=17 nrefs=8 livelen=7 pri=34285` (count -> $s1). Not one form moved a register, an instruction or a reference count. Mechanism for why this was structurally hopeless, recorded so no session re-opens it: under -G0 an extern object of ANY type is addressed by a `lui/addiu` %hi/%lo pair, so the declared type can only affect the pointer-arithmetic scale factor and array-name decay; p is incremented by a function-pointer width either way, and the count symbol's VALUE is never loaded (only its address is taken), so its declared type cannot even influence a load width. The two symbols live at unrelated addresses (0x8008D070 and 0x00000000), so no struct or array wrapping can collapse the two address materialisations into one. Banked as rejected/f11-global-declaration-forms-codegen-inert.c. CONSEQUENCE: with F10 (s8) and F11 (s9) both dead, EVERY input to cc1 outside the function body proper is now measured codegen-inert for this function.
- verdict: KILLED

## [s9] F4b - floor 13 is reachable with a do-while(0) wrap depth below 3, by changing the LOOP CONSTRUCT rather than the wrap.
- mechanism: The device that wins $s0 for p is loop-depth reference weighting - flow.c weights REG_N_REFS by loop depth and each do{}while(0) emits a NOTE_INSN_LOOP_BEG/END pair. On the do-while-LOOP chassis the walk's OWN references are also weighted by the real loop, so both allocnos start large (p 10 weighted refs, count 8) and p needs three wrap levels to clear count. A `goto` loop emits no loop notes for the walk, so every reference in the function is counted RAW (p 4, count 5) and the margin p must close is much smaller. This is a structural change to what flow.c counts, not a respelling of the same counts - precisely the `rederive` mandate.
- probe: tmp/grind/motion_Close/s9/declsweep.py measured goto-loop and while-loop chassis at wrap depths 0-3; tmp/grind/motion_Close/s9/gotosweep.py then crossed the goto chassis with {p-assigned-first, count-assigned-first} x {p-declared-first, count-declared-first} x {tail-tested, top-tested via `goto test;`} x depths 0-2 (18 cells); tmp/grind/motion_Close/s9/idiomsweep.py re-ran the depth ladder across five pointer/loop idioms (15 cells). All cells measured with sandbox + the instrumented cc1 per-allocno table.
- result: CONFIRMED - floor 13 at wrap depth 2. Ladder on the goto chassis with p assigned first: depth 0 p 4 refs / live_length 8 = 10000 -> $s1, count 5/7 = 14285 -> $s0, score 20; depth 1 p 5/8 = 12500, same roles, score 20; depth 2 p 6/8 = 15000 -> $s0, count 5/7 -> $s1, score 13. Declaration order is INERT at every depth (both orders identical), so unlike session 3's tie-punning form this win does not depend on global.c's allocno-number tie-break. The ladder is 20/20/13 across all five idioms (`*p++`, `p[0]` + separate bump, `--count`, decrement-before-call; the call-through `p[-1]()` idiom is worse throughout at 21/21/16), so depth 2 is a property of the function's reference structure, not of one body's spelling. A plain `while` loop needs depth 3 exactly as the do-while loop does (20/20/20/13), so the win comes specifically from the goto loop's absence of loop notes. candidate.c updated to the two-level form; the superseded three-level do-while-loop form is not disproven, merely dominated at equal score.
- verdict: CONFIRMED

## [s9] F4b can be pushed further - wrap depth 0 or 1 reaches floor 13 on some loop chassis.
- mechanism: If a chassis existed where count carried fewer raw references or p carried more without extending its live range, p could win $s0 with one wrap level or none, which would remove the last FAKE-annotated construct from the form entirely.
- probe: The same three sweeps (18 gotosweep cells + 15 idiomsweep cells + the 8 chassis/depth cells in declsweep), reading both the score and the allocno table for every depth-0 and depth-1 cell.
- result: KILLED for every chassis measured. The target's address-materialisation order requires p to be assigned FIRST, which puts count's birth one statement later: count then sits at 5 raw refs / live_length 7 = 14285 while p sits at 5/8 = 12500. p is behind on LIVE_LENGTH, not on references, and any added reference to p that is not inside p's existing range extends that range too - so p must reach 6 weighted references, i.e. two loop-note levels. The only measured way to drop count to 4 references is to delete the outer `if (count != 0)` guard by entering the walk through a top-tested `goto test;` loop; that does let p win $s0 at depth 1 (p 5/8 = 12500 vs count 4/7 = 11428) but it scores 14, because the target HAS that outer guard - trading one residual point for one wrap level is not an improvement. Banked as rejected/goto-chassis-wrap-depth-0-and-1-insufficient.c, which also serves as prerequisite 3 of .claude/rules/do-while-zero-exception.md for the two-level wrap now in candidate.c. The only direction left on F4b is lengthening count's live range without adding a count reference - an intervening real computation, which the target does not perform.
- verdict: KILLED

## [s9] F11 — the DECLARATION of the three globals motion_Close touches (D_800A2668, D_8008D070, D_00000000) is an input to cc1 that can move one of the three surviving residual mechanisms.
- mechanism: Sessions 1-7 measured the body exhaustively and session 8 measured everything above the body (signature, inlining, TU order, helper parameterisation) codegen-inert, leaving the declarations as the one untried input. Array types, a struct wrapping the pair so one address materialisation could serve both, a const-qualified table, an `extern char` for the linker-provided count symbol, or a reordered extern block are all honest declaration changes; any that changed the instruction count would force the s7 residual table to be re-derived rather than reused.
- probe: tmp/grind/motion_Close/s9/declsweep.py — ten declaration forms (base; array-typed table with `p = D_8008D070`; array-typed count; both arrays; `extern char D_00000000[]`; `extern void (*const D_8008D070)(void)` with p as `void (*const *)`; `extern u32 D_800A2668`; the three externs in reverse order; `extern void *D_00000000`; array table + char count together), each applied to the extern lines AND to both users of the symbols (motion_Close and its sibling func_80083794), with the floor-13 body held fixed, each measured with `sandbox motion_Close --disable all` AND with the instrumented cc1 (BB2_ALLOC_DEBUG=1) reading the per-allocno table. No asm("Sym") alias-rename variant was written or measured — forbidden family.
- result: All ten score 13 with build_insns 25 and the byte-identical allocno table (ord=0 pseudo=73 hardreg=16 nrefs=10 livelen=8 pri=37500 = p -> $s0; ord=1 pseudo=72 hardreg=17 nrefs=8 livelen=7 pri=34285 = count -> $s1). Not one form moved a register, an instruction or a reference count. Mechanism: under -G0 an extern of ANY type is addressed by a lui/addiu %hi/%lo pair, so the declared type can only affect the pointer-arithmetic scale factor and array-name decay; p is incremented by a function-pointer width either way, and the count symbol's VALUE is never loaded (only its address is taken) so its type cannot even influence a load width; and the two symbols are at unrelated addresses (0x8008D070 and 0x00000000), so no struct or array wrapping can collapse the two address materialisations. Banked as rejected/f11-global-declaration-forms-codegen-inert.c.
- verdict: KILLED

## [s9] F4b — floor 13 is reachable with a do-while(0) wrap depth below 3, by changing the LOOP CONSTRUCT rather than the wrap.
- mechanism: The device that wins $s0 for p is loop-depth reference weighting: flow.c weights REG_N_REFS by loop depth and each do{}while(0) emits a NOTE_INSN_LOOP_BEG/END pair. On the do-while-LOOP chassis the walk's own references are also weighted by the real loop, so both allocnos start large (p 10 weighted refs, count 8) and p needs three wrap levels to clear count. A `goto` loop emits no loop notes for the walk, so every reference is counted raw (p 4, count 5) and the margin p must close is far smaller. That is a structural change to what flow.c counts, not a respelling of the same counts.
- probe: declsweep.py measured goto-loop and while-loop chassis at wrap depths 0-3; gotosweep.py crossed the goto chassis with {p-assigned-first, count-assigned-first} x {p-declared-first, count-declared-first} x {tail-tested, top-tested via `goto test;`} x depths 0-2 (18 cells); idiomsweep.py re-ran the depth ladder across five pointer/loop idioms (15 cells). Every cell measured with sandbox + the instrumented cc1 per-allocno table.
- result: CONFIRMED — floor 13 at wrap depth 2. Goto chassis with p assigned first: depth 0 p 4 refs / live_length 8 = 10000 -> $s1, count 5/7 = 14285 -> $s0, score 20; depth 1 p 5/8 = 12500, same roles, score 20; depth 2 p 6/8 = 15000 -> $s0, count -> $s1, score 13. Declaration order is INERT at every depth, so unlike session 3's tie-punning form the win does not depend on global.c's allocno-number tie-break. The 20/20/13 ladder repeats across five idioms (`*p++`, `p[0]` + separate bump, `--count`, decrement-before-call; call-through `p[-1]()` is worse throughout at 21/21/16), so depth 2 is a property of the function's reference structure, not of one spelling. A plain `while` loop needs depth 3 exactly as the do-while loop does (20/20/20/13), confirming the win comes specifically from the goto loop's absence of loop notes. candidate.c updated to the two-level form.
- verdict: CONFIRMED

## [s9] F4b can be pushed all the way — wrap depth 0 or 1 reaches floor 13 on some loop chassis, removing the last FAKE-annotated construct entirely.
- mechanism: If a chassis existed where count carried fewer raw references, or p carried more without extending its live range, p could take $s0 with one wrap level or none.
- probe: The same three sweeps (18 gotosweep cells + 15 idiomsweep cells + the chassis/depth cells in declsweep), reading score and allocno table for every depth-0 and depth-1 cell.
- result: KILLED for every chassis measured. The target's address-materialisation order requires p to be assigned FIRST, which puts count's birth one statement later: count then sits at 5 raw refs / live_length 7 = 14285 while p sits at 5/8 = 12500. p is behind on LIVE_LENGTH, not on references, and any added reference to p outside its existing range extends the range too — so p must reach 6 weighted references, i.e. two loop-note levels. The only measured way to drop count to 4 references is deleting the outer `if (count != 0)` guard by entering through a top-tested `goto test;` loop; that lets p win at depth 1 (12500 vs 11428) but scores 14, because the target HAS that outer guard. Trading one residual point for one wrap level is not an improvement. Banked as rejected/goto-chassis-wrap-depth-0-and-1-insufficient.c, which doubles as prerequisite 3 of .claude/rules/do-while-zero-exception.md for the two-level wrap now in candidate.c.
- verdict: KILLED

## [s10] F12 — some other real C loop construct for the walk changes flow.c's reference accounting the way session 9's `goto` loop did, and reaches floor 13 at a do-while(0) wrap depth below 2.
- mechanism: Session 9's decisive finding was that the walk's loop CONSTRUCT changes the weighting of every reference in the function, because flow.c weights REG_N_REFS by loop depth and a `goto` loop emits no NOTE_INSN_LOOP_BEG/END pair for the walk. That moved the required wrap depth from 3 (do-while / while chassis) to 2 (goto chassis). F12 asked whether any construct not yet measured on this function — a `for` with the test in the middle clause, a `for(;;)`/`while(1)` with a `break` exit, a `continue`-based back edge, or an outer real loop around the walk — changes the accounting further. Each is honest C with real semantics rather than a wrapper device, so a win here would be a strictly lighter form than candidate.c's two-level wrap.
- probe: tmp/grind/motion_Close/s10/f12sweep.py — six loop chassis (A `for(;;)`+break, B `while(1)`+break, C `for(;;)`+continue, D `for (; count != 0; )`, E `for (i=0;i<1;i++)` around the goto walk, G the session-9 goto chassis as control) crossed with do-while(0) wrap depths 0/1/2, p assigned first and declared first in every cell (the target's address-materialisation order requires it). Every cell measured with `sandbox motion_Close --disable all` AND with the instrumented cc1 (tools/gcc-2.7.2/cc1, BB2_ALLOC_DEBUG=1) reading the per-allocno n_refs / live_length / priority / hardreg table. Log: tmp/grind/motion_Close/s10/f12sweep.log; per-cell assembly f12_*.s.
- result: KILLED, and cleanly. A, B, C and D score 20 at depths 0, 1 AND 2, with the BYTE-IDENTICAL allocno table at each depth — count 8 weighted refs / live_length 7 = 34285 against p 7/8 = 17500 (d0), 8/8 = 30000 (d1), 9/8 = 33750 (d2). That is exactly the do-while-LOOP family's ladder: `break` vs `continue` vs a middle-clause test vs a bottom test are codegen-inert to flow.c, because all four emit one NOTE_INSN_LOOP_BEG/END pair around the same body and therefore count the same references at the same weight. Only the `goto` chassis (G) escapes the loop note, and it alone reaches 13, at depth 2 (p 6/8 = 15000 vs count 5/7 = 14285), reproducing session 9 exactly. E — an outer once-through `for (i=0;i<1;i++)` — is worse on every axis: the induction variable becomes a third allocno (7 refs / live_length 8 -> $s2), the function grows 25 -> 30 emitted instructions, and the best cell is 18; it is also a construct the role prompt names as NOT sanctioned by the do-while(0) carve-out, so it was measured for mechanism only and can never be proposed. Banked as rejected/f12-loop-construct-surface-inert.c. CONSEQUENCE: the loop-construct surface joins F10 (whole-TU shape) and F11 (global declarations) as measured-inert, so every C-side input class that has ever been proposed for this function is now closed.
- verdict: KILLED

## [s10] The session-7 residual table, derived on the THREE-level do-while-loop chassis, still describes the CURRENT candidate (the session-9 two-level goto chassis) instruction for instruction.
- mechanism: An escalation packet must describe the form actually banked in candidate.c, not a superseded one. Both chassis score 13, but equal scores do not by themselves prove equal residual COMPOSITION: a different distribution of the same 13 points across the H1 / F5 / F7 buckets would invalidate the s7 table's claim that all 13 are backend-disproven and force a re-derivation.
- probe: Compared the emitted assembly of the current candidate chassis (tmp/grind/motion_Close/s10/f12_G_goto_tail_d2.s, produced by the instrumented cc1 during the F12 sweep) against tmp/grind/motion_Close/s5b/wsA/_base.txt, the byte-level build stream the s7 table was written from, and against _tgt_mine.txt (the byte-verified target).
- result: CONFIRMED — the two chassis emit the same stream. Current candidate: `lw $2,D_800A2668 / subu $sp,$sp,32 / sw $31,24 / sw $17,20 / beq $2,$0 (delay: sw $16,16) / la $16,D_8008D070 / la $17,D_00000000 / beq $17,$0 / lw $2,0($16) / addu $16,$16,4 / jal $31,$2 (delay: addu $17,$17,-1) / bne $17,$0 / lw $31,24 / lw $17,20 / lw $16,16 / addu $sp,$sp,32 / j $31`, with `.frame $sp,32,$31 # vars= 0, regs= 3/0, args= 16`. That is _base.txt position for position: the same 25 instructions, the same $v0 temp, the same descending save order, the same filled beqz delay slot, the same 32-byte frame with a 16-byte outgoing-argument area. The s7 residual table therefore transfers verbatim — 5 points F5 ($v0 vs $t0 at insns 1, 2, 7, 15, 17), 6 points plus half of a 7th H1 (frame size and the six save/restore offsets), 3 points F7a/F7b (ascending save order and the empty beqz delay slot) — and the packet does not need re-deriving.
- verdict: CONFIRMED

# ============================================================================
# SESSION 10 SYNTHESIS — FRONTIER RESET
# This block SUPERSEDES every earlier "LIVE FRONTIER" section in this file
# (the F1/F4 block near the top and the F4b/F8/F12 digest frontier). Read this
# and nothing else for the current state of the search.
# ============================================================================

## THE MERGED PICTURE (ten sessions, one paragraph)

motion_Close is a crt0-style destructor-table walk: guard a global, materialise
a table pointer and a count from two linker-provided symbols, then call each
entry and decrement. The honest pure-C form reproduces the target's control
flow, its operation sequence and even its two delay-slot fills exactly, at 25
emitted instructions against the target's 26. The floor fell 21 -> 20 (s1,
honest C at all) -> 17 (s2, statement order) -> 16 (s3/s4, winning $s0 for p)
-> 13 (s5, loop-note reference weighting) and has been FLAT at 13 for six
consecutive sessions across five distinct modalities. The 13 residual points are
paired instruction-by-instruction in tmp/grind/motion_Close/s7/residual_table.md
and every one of them belongs to a mechanism with a BACKEND-LEVEL disproof, not
a plateau:
  * H1 (~6.5 pts, frame size + six save/restore offsets) — REG_PARM_STACK_SPACE
    is the compile-time constant 16 for every fndecl (mips.h:1822 over
    mips.h:1888 and the #if 0 at mips.h:1801-1812), applied by a MAX on every
    call-expansion path (calls.c:1245-1247 feeding calls.c:1394-1401), and
    compute_frame_size (mips.c:4466) reads nothing else. The only cfoas-free
    emit_call_insn site in the whole compiler is expand_builtin_apply
    (expr.c:8361), which emits a frame pointer, a 72-byte frame and 34
    instructions. No C body containing a call can have the target's zero-byte
    outgoing-argument area.
  * F5 (5 pts, $v0 where the target uses $t0) — local-alloc.c:2249-2262 and
    global.c:1057-1062/1203-1209 scan hard registers in ascending numeric order
    because the MIPS backend defines no REG_ALLOC_ORDER, so $v0 wins whenever
    free; reaching $t0 needs six simultaneously live call-clobbered values,
    which the target's no-argument void call stream does not contain.
  * F7a (save order) — mips.c:4680 emits GP saves GP_REG_LAST -> GP_REG_FIRST
    unconditionally, and s8 measured that even a real incoming-argument WAR
    anti-dependence in the prologue block does not flip them.
  * F7b (empty beqz delay slot) — reorg.c must fill a slot when an eligible
    single insn precedes the branch in the block; the target's block holds four,
    and the corpus contains zero compiled-C counterexamples in 1788 functions.
Everything OUTSIDE the body is measured codegen-inert: whole-TU shape and
signature (F10, s8), global declarations (F11, s9), loop construct (F12, s10).
The canonical-asm endgame gate is a MEASURED FAILED gate (scan_hand_coded tier
LOW, 0/8, for motion_Close and for its sibling func_80083794 — s6), and no
coercion/spelling family with an in-hand SOTN-master precedent applies.

## THE FRONTIER, RESET TO THREE ENTRIES

### FR1 (terminal, and the only entry that matters) — motion_Close is a fully determined ESCALATION candidate awaiting the driver's routing.
**State.** Floor flat at 13 for six sessions over five modalities (structural,
permuter, forensics, rederive, synthesis). Residual 13/13 attributed, table
verified this session against the CURRENT candidate chassis. All three residual
mechanisms carry backend-level disproofs. All three non-body input axes are
measured dead. Both endgame-lock AND-gates FAIL: (1) scan_hand_coded returns
tier LOW 0/8 with S1/S2/S6 all negative, so the canonical-asm gate is a measured
failed gate, not an open question; (2) no coercion/spelling family with a
citable SOTN-master precedent (file+line or commit) applies to this residual —
the gap is frame layout and hard-register scan order, which no C-level device
can address.
**Next probe.** None on the C side. Sessions must NOT re-measure a dead axis.
When the driver assigns `escalation` modality, the packet is already written:
file the entry as `**OWNER-ESCALATION — RESOLVED BY STANDING RULING
(2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE**`, citing the s7 backend
disproof of H1, the s7 residual table, the s6 corpus disproofs of F7a/F7b, the
s5b scan-order disproof of F5, the s6 scan_hand_coded LOW tier, and the
s8/s9/s10 trio showing every non-body input to cc1 is inert. Both gates fail, so
nothing is pending on the owner and the entry is terminal.

### FR2 (dormant, cosmetic) — F4b: reach floor 13 with a do-while(0) wrap depth below 2.
**State.** Depth 2 on the goto chassis is the measured minimum and is minimal
across five pointer/loop idioms, both declaration orders, two inlined chassis,
ten global-declaration forms and now six loop constructs. The binding constraint
is arithmetic: with p assigned first (which the target's materialisation order
requires) count sits at 5 raw refs / live_length 7 = 14285 and p at 5/8 = 12500,
so p must reach 6 weighted references. Deleting a count reference means deleting
the outer guard the target has, which costs a residual point (14).
**Next probe.** Only worth time if a session is preparing a submission, which
cannot happen while H1 stands. The single untried direction is unchanged since
s8: lengthen count's live range WITHOUT adding a count reference — an
intervening real computation that the target also performs — and the target
performs none. Success criterion is score 13 at wrap depth <= 1, not a lower
score.

### FR3 (housekeeping) — keep the escalation packet true to the banked form.
**State.** Twice now the ledger's authoritative residual analysis has been
derived on a chassis that a later session superseded (s7's table on the
three-level do-while chassis; candidate.c replaced with the two-level goto
chassis in s9). This session verified the table still transfers, but the check
was ad hoc.
**Next probe.** Any session that changes candidate.c must re-emit the build
stream and diff it against tmp/grind/motion_Close/s5b/wsA/_base.txt before
reusing the s7 residual table. It is a one-command check
(`awk '/^motion_Close:/,/\.end/'` over the cc1 output) and it is the difference
between an escalation packet that describes the banked form and one that
describes a superseded one.

## [s10] F12 - some other real C loop construct for the walk changes flow.c's reference accounting the way session 9's goto loop did, and reaches floor 13 at a do-while(0) wrap depth below 2.
- mechanism: flow.c weights REG_N_REFS by loop depth and every loop construct emits a NOTE_INSN_LOOP_BEG/END pair, so the walk's own references are weighted; a goto loop emits no note and collapses every reference in the function to a raw count, which is why session 9 moved the required wrap depth from 3 to 2. If some other construct (for-with-break, while(1)-with-break, continue back edge, middle-clause for, or an outer real loop) changed the accounting further, p could win $s0 at depth 1 or 0 and the last device would leave the form.
- probe: tmp/grind/motion_Close/s10/f12sweep.py - six loop chassis x do-while(0) wrap depths 0/1/2 (18 cells), p assigned first and declared first in every cell, each measured with `sandbox motion_Close --disable all` AND with the instrumented cc1 (BB2_ALLOC_DEBUG=1) reading the per-allocno n_refs / live_length / priority / hardreg table. Log f12sweep.log, per-cell assembly f12_*.s.
- result: A for(;;)+break, B while(1)+break, C for(;;)+continue and D for(; count != 0; ) score 20 at depths 0, 1 AND 2 with the BYTE-IDENTICAL allocno table at each depth - count 8 weighted refs / live_length 7 = 34285 against p 7/8 = 17500 (d0), 8/8 = 30000 (d1), 9/8 = 33750 (d2), i.e. exactly the do-while-LOOP family's ladder, so break vs continue vs middle-clause vs bottom test are codegen-inert to flow.c. Only the goto chassis (control G) escapes the loop note and reaches 13, at depth 2 (p 6/8 = 15000 vs count 5/7 = 14285). The outer once-through for (i=0;i<1;i++) is worse on every axis - a third allocno in $s2 (7 refs / live_length 8), 30 emitted instructions against 25, best cell 18 - and is a construct the role prompt names as NOT sanctioned by the do-while(0) carve-out, so it was measured for mechanism only and can never be proposed. Banked as rejected/f12-loop-construct-surface-inert.c.
- verdict: KILLED

## [s10] The session-7 residual table, derived on the superseded three-level do-while-loop chassis, still describes the CURRENT candidate (the session-9 two-level goto chassis) instruction for instruction.
- mechanism: An escalation packet must describe the form actually banked in candidate.c. Both chassis score 13, but equal scores do not prove equal residual COMPOSITION - a different distribution of the same 13 points across the H1 / F5 / F7 buckets would invalidate the table's claim that all 13 points are backend-disproven and force a re-derivation before any escalation.
- probe: Diffed the current candidate chassis' emitted assembly (tmp/grind/motion_Close/s10/f12_G_goto_tail_d2.s, produced by the instrumented cc1 during the F12 sweep) against tmp/grind/motion_Close/s5b/wsA/_base.txt, the byte-level build stream the s7 table was written from, and against _tgt_mine.txt (the byte-verified target).
- result: Same stream, position for position: lw $2,D_800A2668 / subu $sp,$sp,32 / sw $31,24 / sw $17,20 / beq $2,$0 with sw $16,16 in the delay slot / la $16,D_8008D070 / la $17,D_00000000 / beq $17,$0 / lw $2,0($16) / addu $16,$16,4 / jal $31,$2 with addu $17,$17,-1 in the delay slot / bne $17,$0 / three restores / addu $sp,$sp,32 / j $31, under `.frame $sp,32,$31 # vars= 0, regs= 3/0, args= 16`. Same 25 instructions, same $v0 temp, same descending save order, same filled beqz delay slot, same 32-byte frame with a 16-byte outgoing-argument area. The s7 buckets (5 points F5, 6 plus half of a 7th H1, 3 F7a/F7b) transfer verbatim.
- verdict: CONFIRMED

## [s11] F13 — the GUARD and MATERIALISATION shape of the function (guard spelling, guard staging, declaration scope, call-temp scope, loop-body statement order, materialisation position) can move the floor on the session-9 goto chassis, because the s2/s3 verdicts that called this surface inert were measured in a different weighting regime.
- mechanism: flow.c weights REG_N_REFS by loop depth. Sessions 2/3 swept the guard surface on the do-while-LOOP chassis, where the walk's own references are loop-weighted and the two allocnos sit ~10% apart (p 10 refs / live_length 8 = 37500 vs count 8/7 = 34285). Session 9 replaced the chassis with a `goto` loop, which emits no NOTE_INSN_LOOP_BEG/END for the walk, so every reference in the function is counted RAW and the margin collapsed to 1.4% (p 5/8 = 12500 vs count 5/7 = 14285). A one-reference or one-insn lever that is worthless at a 10% margin can be decisive at 1.4%, so the s2/s3 conclusions do not transfer by construction — the same chassis-transfer error frontier FR3 exists to prevent. Re-measuring the surface in the raw-count regime is therefore a new probe, not a re-run of a dead axis.
- probe: tmp/grind/motion_Close/s11/f13sweep.py — 10 variants x do-while(0) wrap depths 0/1/2 = 30 cells, every cell measured with `sandbox motion_Close --disable all` AND with the instrumented cc1 (tools/gcc-2.7.2/cc1, BB2_ALLOC_DEBUG=1) reading the per-allocno n_refs / live_length / priority / assigned-hardreg table. Variants: V0 control, V1 early-return guard, V2 guard staged through a local, V3 count assigned first, V4 both locals declared inside the guard block, V5 declaration-with-initialiser at function scope, V6 call temp `f` declared at function scope, V7 `count--` before `f()`, V8 p materialised above the guard, M1 self-assign (measurement only). Log f13sweep.log, per-cell assembly f13_*.s.
- result: KILLED. Six variants (V0, V1, V2, V4, V6, V7) are BYTE-IDENTICAL to the control at every depth — identical allocno table (count 5 raw refs / live_length 7 = 14285; p 4+depth / 8 = 10000 / 12500 / 15000), identical 25 emitted instructions, identical 20 / 20 / 13 ladder. Guard spelling, guard staging, declaration SCOPE of the two locals, the call temp's scope and the loop body's statement order are all free variables of this function in the raw-count regime. The three non-inert variants are all worse: V3 count-first 17/16/16, V5 declaration-with-initialiser 19 at every depth (both materialisations hoisted above the guard and the wrap lands on the walk, weighting BOTH allocnos together — 9/13 refs vs 7/10 — exactly session 4's nested-wrap result), V8 p-above-guard 20/21/21 (p's live_length grows to 9-10 and the function emits 27 instructions instead of 25). Concrete demonstration that the re-measurement was warranted rather than redundant: `count--` before `f()` cost 19 points in the loop-note regime (s3 sweep3.py) and is completely FREE here (13, V7_dec_before_call_d2) — the verdict changed even though the floor did not. Banked as rejected/f13-guard-and-materialisation-shape-inert.c.
- verdict: KILLED

## [s11] FR2 / F4b — floor 13 is reachable at a do-while(0) wrap depth below 2 by giving up p-assigned-first, since count-assigned-first shortens p's live range and lets p win $s0 one wrap level cheaper.
- mechanism: In the raw-count regime the priority is floor_log2(n)*n/live_length. With p assigned first, p's live range starts one insn earlier than count's, so p is 4+depth refs / live_length 8 and count is 5 / 7 — p needs 6 weighted refs (depth 2) to clear 14285. Assigning count first inverts the live lengths (p 7, count 8), so at depth 1 p reaches 5/7 = 14285 against count's 5/8 = 12500 and takes $s0 with only ONE wrap level. The open question was whether the resulting materialisation-order penalty is smaller than the wrap level it buys.
- probe: cells V3_countfirst_assign at wrap depths 0/1/2 in tmp/grind/motion_Close/s11/f13sweep.py, sandbox + instrumented cc1.
- result: KILLED as an improvement, CONFIRMED as a mechanism. V3 at depth 1 does reach the target's register roles (p 5/7 = 14285 in $s0, count 5/8 = 12500 in $s1) at one wrap level — the first time the roles have been had below depth 2 on this chassis — but it scores 16, not 13, because the two address materialisations then emit count-pair-then-p-pair while the target emits p-pair-then-count-pair. Depth 2 does not recover it (still 16; p rises to 17142 but the order penalty is unchanged) and depth 0 is 17. The exchange rate is now measured exactly: one wrap level costs three residual points. FR2's success criterion is score 13 at depth <= 1, so this fails it; the depth-2 p-first form remains the minimum. Banked inside rejected/f13-guard-and-materialisation-shape-inert.c as the representative rejected member.
- verdict: KILLED

## [s11] A self-assign `p = p;` — a construct inside the FROZEN sanctioned "dead stores / self-assigns to LOCALS or PARAMS" family — can supply p's extra reference and reach floor 13 at wrap depth 0, removing the last FAKE-annotated construct from the candidate form.
- mechanism: The candidate's only non-obvious device is the two-level do-while(0) wrap, which exists purely to lift p's weighted reference count from 4 to 6. If a self-assign counted as a reference, one or two of them would do the same job inside a family that carries its own SOTN precedent. The competing claim — .claude/rules/duplicated-statement-into-arms.md's "dead stores measured INERT for this: flow deletes before counting" — was established on a different function and had never been measured here.
- probe: cell M1_self_assign at wrap depths 0/1/2 in tmp/grind/motion_Close/s11/f13sweep.py (MEASUREMENT ONLY — the cell was never a proposable form: no annotation, no exhaustion record, and it is dominated at equal score), sandbox + instrumented cc1.
- result: KILLED. The ladder is the control's VALUE FOR VALUE — depth 0: p 4/8 = 10000, count 5/7 = 14285, score 20; depth 1: p 5/8 = 12500, score 20; depth 2: p 6/8 = 15000, score 13 — with the same 25 emitted instructions. The self-assign never appears in the reference count at any depth: jump.c/cse delete the no-op set before flow.c's counter sees it, so it changes nothing upstream of the allocator either. The duplicated-statement-into-arms rule's claim therefore transfers to motion_Close: on this function a dead store is NOT a ref-lift device, the do-while(0) wrap is not substitutable by the cheaper-looking sanctioned family, and no future session should spend time on the substitution. Banked as rejected/m1-self-assign-does-not-lift-refs.c.
- verdict: KILLED

## FRONTIER AFTER SESSION 11 (structural)

Unchanged in substance from session 10, with one axis added to the dead list and
one arithmetic constant now measured rather than derived.

### FR1 (terminal) — motion_Close is a fully determined ESCALATION candidate awaiting the driver's routing.
**State.** Floor flat at 13 for SEVEN sessions across SIX modalities (structural,
permuter, forensics, rederive, synthesis, structural). The 13 residual points are
paired instruction-by-instruction in tmp/grind/motion_Close/s7/residual_table.md,
verified against the current chassis in s10, and all three mechanisms carry
backend-level disproofs: H1 (REG_PARM_STACK_SPACE is the compile-time constant 16
applied by MAX on every call-expansion path — mips.h:1822, calls.c:1245/1400,
mips.c:4466; the only cfoas-free site, __builtin_apply, emits a frame pointer, a
72-byte frame and 34 insns), F5 (ascending first-free hard-reg scan, no MIPS
REG_ALLOC_ORDER, so $v0 is never $t0), F7a (mips.c:4680 emits GP saves high-to-low
unconditionally and a real incoming-arg WAR anti-dependence does not reorder them)
and F7b (an empty beqz delay slot needs a block with no eligible single insn while
the target's holds four; 0/1788 compiled-C counterexamples). FIVE non-body input
axes are now measured dead — F10 whole-TU (s8), F11 declarations (s9), F12 loop
construct (s10), F13 guard/materialisation shape (s11) — plus the pointer/loop
idiom sweep (s9) and the dead-store substitution (s11).
**Next probe.** None on the C side; do NOT re-measure a dead axis. When the driver
assigns `escalation` modality, the packet is already written and both endgame-lock
gates FAIL, so under the owner's 2026-07-27 standing auto-ruling the terminal entry
is `OWNER-ESCALATION — RESOLVED BY STANDING RULING (2026-07-27): REFUSED /
OWNER-ACCEPTED INCOMPLETE`, citing the s7 backend disproof of H1, the s7 residual
table, the s5b scan-order disproof of F5, the s6 corpus disproofs of F7a/F7b, the
s6 scan_hand_coded LOW tier, and the s8/s9/s10/s11 quartet showing every input to
cc1 outside the body proper — and now every shape of the body outside the walk
itself — is inert.

### FR2 (dormant, cosmetic) — reach floor 13 with a do-while(0) wrap depth below 2.
**State.** Depth 2 on the goto chassis remains the measured minimum, now minimal
across five pointer/loop idioms, both declaration orders, two inlined chassis, ten
global-declaration forms, six loop constructs and ten guard/materialisation shapes.
Session 11 measured the exchange rate that makes it binding: count-assigned-first
DOES win p its register at depth 1 (p 5/7 = 14285 vs count 5/8 = 12500) but pays
three residual points of materialisation order, so 16 not 13. One wrap level costs
three points; there is no arrangement in which that trade is profitable.
**Next probe.** Only worth time if a session is preparing a submission, which
cannot happen while H1 stands. The single untried direction is unchanged since s8
and is now known to be the ONLY one: lengthen count's live range WITHOUT adding a
count reference and WITHOUT moving p's birth — an intervening real computation
that the target also performs — and the target performs none. The dead-store
substitution is closed (s11, M1). Success criterion is score 13 at wrap depth <= 1,
not a lower score.

### FR3 (housekeeping) — keep the escalation packet true to whatever form candidate.c actually holds.
**State.** Session 11 did NOT change candidate.c's form (the two-level goto
chassis is still the banked best, and the sweep's control cell re-measured it at
13 this session), so the s10 stream verification still stands and no re-diff was
required. The comment header carries a session-11 note.
**Next probe.** Unchanged: any session that changes candidate.c must re-emit the
build stream from the instrumented cc1 and diff it against
tmp/grind/motion_Close/s5b/wsA/_base.txt before reusing the s7 residual table.

## [s11] F13 - the GUARD and MATERIALISATION shape of the function (guard spelling, guard staging, declaration scope of the locals, call-temp scope, loop-body statement order, materialisation position) can move the floor on the session-9 goto chassis, because the s2/s3 verdicts that called this surface inert were measured in a different weighting regime.
- mechanism: flow.c weights REG_N_REFS by loop depth. Sessions 2/3 swept the guard surface on the do-while-LOOP chassis, where the walk's own references are loop-weighted and the two allocnos sit ~10% apart (p 10 refs / live_length 8 = 37500 vs count 8/7 = 34285). Session 9 replaced the chassis with a goto loop, which emits no NOTE_INSN_LOOP_BEG/END for the walk, so every reference is counted RAW and the margin collapsed to 1.4% (p 5/8 = 12500 vs count 5/7 = 14285). A one-reference or one-insn lever worthless at a 10% margin can be decisive at 1.4%, so the old verdicts do not transfer by construction - the same chassis-transfer error frontier FR3 exists to prevent.
- probe: tmp/grind/motion_Close/s11/f13sweep.py - 10 variants x do-while(0) wrap depths 0/1/2 = 30 cells, each measured with `sandbox motion_Close --disable all` AND with the instrumented cc1 (tools/gcc-2.7.2/cc1, BB2_ALLOC_DEBUG=1) reading the per-allocno n_refs / live_length / priority / assigned-hardreg table. Log f13sweep.log, per-cell assembly f13_*.s.
- result: Six variants (control, early-return guard, guard staged through a local, both locals declared inside the guard block, call temp `f` at function scope, count-- before f()) are BYTE-IDENTICAL at every depth: identical allocno table (count 5 raw refs / live_length 7 = 14285; p 4+depth / 8 = 10000 / 12500 / 15000), identical 25 emitted instructions, identical 20 / 20 / 13 ladder. The three non-inert variants are worse: count-assigned-first 17/16/16, declaration-with-initialiser 19 at every depth (both materialisations hoisted above the guard and the wrap then weights BOTH allocnos, 9/13 refs vs 7/10 - session 4's nested-wrap result reproduced), p-materialised-above-the-guard 20/21/21 (live_length 9-10, 27 emitted instructions). Concrete proof the re-measurement was warranted rather than redundant: `count--` before `f()` cost 19 points in the loop-note regime (s3) and is FREE here (13). Nothing in the sweep scored below 13. Banked as rejected/f13-guard-and-materialisation-shape-inert.c.
- verdict: KILLED

## [s11] FR2 / F4b - floor 13 is reachable at a do-while(0) wrap depth below 2 by giving up p-assigned-first, since count-assigned-first shortens p's live range and lets p win $s0 one wrap level cheaper.
- mechanism: Priority is floor_log2(n)*n/live_length. With p assigned first, p's range starts one insn earlier than count's, so p is 4+depth refs / live_length 8 against count 5/7 - p needs 6 weighted refs (depth 2) to clear 14285. Assigning count first inverts the live lengths (p 7, count 8), so at depth 1 p reaches 5/7 = 14285 against count's 5/8 = 12500 and takes $s0 with ONE wrap level. Open question: whether the resulting materialisation-order penalty is smaller than the wrap level it buys.
- probe: Cells V3_countfirst_assign at wrap depths 0/1/2 in tmp/grind/motion_Close/s11/f13sweep.py, sandbox + instrumented cc1.
- result: KILLED as an improvement, CONFIRMED as a mechanism. Depth 1 does reach the target's register roles (p 5/7 = 14285 in $s0, count 5/8 = 12500 in $s1) - the first time the roles have been had below depth 2 on this chassis - but scores 16, because the address materialisations then emit count-pair before p-pair while the target emits p-pair first. Depth 2 does not recover it (16; p rises to 17142, the order penalty is unchanged) and depth 0 is 17. Exchange rate now measured: one wrap level costs three residual points. FR2's criterion is 13 at depth <= 1, so this fails it and the depth-2 p-first form remains the minimum.
- verdict: KILLED

## [s11] A self-assign `p = p;` - a construct inside the FROZEN sanctioned 'dead stores / self-assigns to LOCALS or PARAMS' family - can supply p's extra reference and reach floor 13 at wrap depth 0, removing the last FAKE-annotated construct from the candidate form.
- mechanism: The candidate's only non-obvious device is the two-level do-while(0) wrap, which exists purely to lift p's weighted reference count from 4 to 6. If a self-assign counted as a reference, one or two would do the same job inside a family carrying its own SOTN precedent. The competing claim - .claude/rules/duplicated-statement-into-arms.md's 'dead stores measured INERT for this: flow deletes before counting' - was established on a different function and had never been measured here.
- probe: Cell M1_self_assign at wrap depths 0/1/2 in tmp/grind/motion_Close/s11/f13sweep.py. MEASUREMENT ONLY - never a proposable form (no annotation, no exhaustion record, dominated at equal score).
- result: KILLED. The ladder is the control's value for value - depth 0 p 4/8 = 10000 / score 20, depth 1 p 5/8 = 12500 / score 20, depth 2 p 6/8 = 15000 / score 13 - with the same 25 emitted instructions. The self-assign never appears in the reference count: jump.c/cse delete the no-op set before flow.c's counter sees it. The sanctioned dead-store family therefore cannot substitute for the do-while(0) wrap on this function. Banked as rejected/m1-self-assign-does-not-lift-refs.c.
- verdict: KILLED

## [s12] F14 — the do-while(0) wrap's PLACEMENT (which statements the note encloses), as opposed to its DEPTH, redistributes weighted references between the two allocnos and can win p its $s0 at a lower wrap depth.
- mechanism: flow.c's weighting is per-REFERENCE — `REG_N_REFS (regno) += loop_depth` for every reference inside a NOTE_INSN_LOOP_BEG/END pair — but every wrap measurement this function has ever had (s4 sweep5, s5b, s8, s9 gotosweep, s10 f12sweep, s11 f13sweep) placed the note on ONE statement, p's initialiser, and varied only the NESTING DEPTH around that site. The banked 20/20/13 ladder is therefore a ladder in depth-at-one-placement, not in the device's strength. p's four raw references are not all at the initialiser: one is the `*p` read and TWO are the `p++` set+use inside the walk, so a note around the bump should lift p by 2 per level rather than 1 and reach the required 6 weighted references at ONE level instead of two.
- probe: tmp/grind/motion_Close/s12/f14sweep.py — 7 placements x wrap depths 0/1/2 = 21 cells (wrap on p's initialiser [control], on `p++`, on `f = *p;`, on the read+bump pair, on the whole walk body, on count's initialiser [directional control], and split across the initialiser AND the bump), each measured with `sandbox motion_Close --disable all` AND with the instrumented cc1 (tools/gcc-2.7.2/cc1, BB2_ALLOC_DEBUG=1) reading the per-allocno n_refs / live_length / priority / assigned-hardreg table. Log f14sweep.log, per-cell assembly f14_*.s.
- result: CONFIRMED as a mechanism, and the per-reference weighting is now measured rather than inferred. Placement changes p's lift per wrap level exactly as the reference structure predicts: initialiser +1 (4/5/6 refs), `f = *p;` +1 (4/5/6), `p++` +2 (4/6/8), read+bump pair +3 (4/7/10), initialiser+bump split +3 (4/7/10). count is untouched by every in-walk p placement (5 raw refs / live_length 7 = 14285 in all 21 cells). The two directional controls behave as the mechanism requires: wrapping the WHOLE walk body lifts both allocnos together (p +3, count +2 per level — 7 vs 7 at depth 1, 10 vs 9 at depth 2) and never changes the ordering, 20 at every depth; wrapping count's initialiser lifts count alone (5/6/7) and is 20 at every depth. p wins $s0 at wrap depth ONE in five distinct placements — the first time the roles have been had at one level with p assigned first.
- verdict: CONFIRMED

## [s12] Every in-walk wrap that wins the roles at depth 1 nevertheless scores 15, so the placement axis buys nothing.
- mechanism: The naive in-walk cells all emit 26 instructions instead of 25 and score 15 instead of 13, which reads as "the note inside the loop costs more than the wrap level it saves". If that were a fixed consequence of the note landing inside the walk, F14 would be a mechanism with no product.
- probe: Stream diff of tmp/grind/motion_Close/s12/f14_W1_pinc_d1.s against f14_W0_control_d2.s (the banked chassis), then tmp/grind/motion_Close/s12/f14bsweep.py — 7 further in-walk arrangements x depths 1/2 (bump wrapped with `count--` moved ahead of the call; note enclosing bump AND call; read+bump pair with `count--` first; bump moved after the call; an EMPTY note between bump and call as an isolating diagnostic; initialiser+bump split with `count--` first), sandbox + instrumented cc1. Log f14bsweep.log, per-cell assembly f14b_*.s.
- result: KILLED. The 26th instruction is a maspsx load-delay nop with an identified cause and an identified fix. With the ORIGINAL statement order the note fences `addu $16,$16,4` out of its position between the load and the call; the scheduler fills the jal delay slot with it instead of with `addu $17,$17,-1`, leaving `lw $2,0($16)` feeding `jal $31,$2` back-to-back, and maspsx inserts the nop. Moving `count--` ahead of `f()` — measured FREE on this chassis in s11 F13 cell V7, and semantically identical since `count` is a local the callee cannot observe — gives the scheduler a second delay-slot candidate; it fills the jal slot with `addu $17,$17,-1` exactly as the target does, `addu $16,$16,4` stays between the lw and the jal, the nop disappears, and the cell scores 13 at 25 instructions. Three arrangements reach 13 at wrap depth 1: bump wrapped (X1, p 6/8 = 15000), read+bump pair wrapped (X3, 7/8 = 17500) and initialiser+bump split (X7, 7/8 = 17500). The isolating diagnostic X6 — an EMPTY `do { } while (0);` in the same in-walk position, adding no reference to either allocno — still emits 26 instructions at score 21, proving the fence is the NOTE and not the wrapped statement, so the statement order is what pays for it. Banked as rejected/f14-in-walk-wrap-costs-load-delay-nop.c.
- verdict: KILLED

## [s12] FR2 — floor 13 is reachable with a do-while(0) wrap depth below 2.
- mechanism: FR2's success criterion, open since session 8, is score 13 at wrap depth <= 1. Session 11 priced the only route it could see (count-assigned-first wins the roles at depth 1 but pays three residual points of materialisation order, 16) and concluded the trade is never profitable. That analysis held the wrap PLACEMENT fixed at p's initialiser; with the note moved onto the bump, p reaches 6 weighted references at one level with p still assigned FIRST, so no materialisation-order penalty is incurred at all.
- probe: Cell X1_dec_before_call at depths 1 and 2 in tmp/grind/motion_Close/s12/f14bsweep.py, sandbox + instrumented cc1, followed by the FR3-mandated stream re-verification: `awk '/^motion_Close:/,/\.end/'` over the cell's cc1 output diffed against tmp/grind/motion_Close/s10/f12_G_goto_tail_d2.s (the stream session 10 verified against tmp/grind/motion_Close/s5b/wsA/_base.txt, which the s7 residual table was written from).
- result: CONFIRMED — score 13, build_insns 25, at a SINGLE do-while(0) level (depth 2 is no better: also 13/25, so one level is the whole device). p 6 refs / live_length 8 = 15000 in $s0 against count 5/7 = 14285 in $s1. The stream is IDENTICAL to the s10-verified chassis except for the internal label NUMBER (.L136 vs .L144 — fewer loop notes are emitted, so the label counter lands lower); same 25 instructions, same registers, same `.frame $sp,32,$31 # vars= 0, regs= 3/0, args= 16`, same filled beqz delay slot, same descending save order, same $v0 temp. The s7 residual table therefore describes the new form verbatim and no re-derivation is owed. candidate.c now holds this form; the policy device is halved (one level, so do-while-zero-exception prerequisite 3 — the nested-wrap exhaustion record — no longer applies at all). NOTE for future integration: the label COUNT is unchanged but the label NUMBER is not, which is the [[global-label-drift-sibling-cheat]] surface. NOTE also that rejected/goto-chassis-wrap-depth-0-and-1-insufficient.c is now scoped rather than general: depth 1 is insufficient AT THE INITIALISER PLACEMENT, which is all that cell ever measured.
- verdict: CONFIRMED

## [s12] Wrap depth 0 — floor 13 with NO do-while(0) anywhere — is reachable on this chassis.
- mechanism: With FR2's depth-1 criterion met, the residual question is whether the device can be removed entirely. In the raw-count regime the priority is floor_log2(n)*n/live_length*10000 and the measured constants are p 4 raw refs / live_length 8 = 10000 against count 5/7 = 14285, with count holding the lower allocno number in the banked declaration order.
- probe: Closed-form enumeration over the priority formula, using the constants measured in all 35 cells of this session's two sweeps (every depth-0 cell in both sweeps, across 7 placements, reproduces p 4/8 = 10000 and count 5/7 = 14285 with score 20).
- result: KILLED. Flipping the roles at depth 0 requires one of exactly three things, and each emits instructions the target does not have. (a) A real extra p reference: the target's 26 instructions contain no p reference our 25 do not, so any such reference is a foreign construct. (b) p's live_length <= 5 (2*4/5 = 16000 > 14285): impossible, since p's live range spans the whole walk, and the only way to shorten it is to move p's birth after count's — the count-assigned-first form, measured at 16 (three residual points of materialisation order, s11 V3). (c) count's priority pushed to <= 10000, i.e. live_length >= 10 at 5 references (2*5/10 = 10000, a tie that p would then win on allocno number if declared first): needs at least three insns of intervening real computation between count's birth and its last use that the target does not perform. ONE wrap level is therefore the MINIMUM policy device for floor 13 on this chassis — a derived floor for the device, not merely an unbeaten measurement.
- verdict: KILLED

## [s12] F14 — the do-while(0) wrap's PLACEMENT (which statements the note encloses), as opposed to its DEPTH, redistributes weighted references between the two allocnos and can win p its $s0 at a lower wrap depth.
- mechanism: flow.c weights per REFERENCE (REG_N_REFS (regno) += loop_depth for every reference inside a NOTE_INSN_LOOP_BEG/END pair), but every wrap measurement this function has ever had (s4 sweep5, s5b, s8, s9, s10, s11) placed the note on ONE statement — p's initialiser — and varied only nesting depth. p's four raw references are not all there: one is the `*p` read and TWO are the `p++` set+use inside the walk, so a note on the bump should lift p by 2 per level and reach the required 6 weighted refs at ONE level.
- probe: tmp/grind/motion_Close/s12/f14sweep.py — 7 placements x wrap depths 0/1/2 = 21 cells (initialiser [control], `p++`, `f = *p;`, read+bump pair, whole walk body, count's initialiser [directional control], initialiser+bump split), each measured with `sandbox motion_Close --disable all` AND the instrumented cc1 (BB2_ALLOC_DEBUG=1) reading the per-allocno n_refs / live_length / priority / hardreg table.
- result: Lift per level tracks the reference count inside the note exactly: initialiser +1 (4/5/6), `f = *p;` +1 (4/5/6), `p++` +2 (4/6/8), read+bump pair +3 (4/7/10), split +3 (4/7/10). count is untouched by every in-walk p placement (5 raw refs / live_length 7 = 14285 in all 21 cells). Directional controls behave as required: whole-body wrap lifts BOTH (p +3, count +2 per level — 7 vs 7 at d1, 10 vs 9 at d2), ordering unchanged, 20 at every depth; count-initialiser wrap lifts count alone (5/6/7), 20 at every depth. p wins $s0 at depth ONE in five distinct in-walk placements with p still assigned first.
- verdict: CONFIRMED

## [s12] Every in-walk wrap that wins the roles at depth 1 nevertheless scores 15, so the placement axis buys nothing.
- mechanism: All naive in-walk cells emit 26 instructions instead of 25 and score 15 instead of 13 — which would make F14 a mechanism with no product if the extra instruction were a fixed consequence of the note landing inside the walk.
- probe: Stream diff of f14_W1_pinc_d1.s against f14_W0_control_d2.s, then tmp/grind/motion_Close/s12/f14bsweep.py — 7 further in-walk arrangements x depths 1/2 (bump wrapped with `count--` ahead of the call; note enclosing bump AND call; read+bump pair with `count--` first; bump after the call; an EMPTY note as isolating diagnostic; initialiser+bump split with `count--` first), sandbox + instrumented cc1.
- result: KILLED. The 26th instruction is a maspsx load-delay nop with an identified cause and fix: with the original statement order the note fences `addu $16,$16,4` out of its slot between the load and the call, the scheduler fills the jal delay slot with it instead of `addu $17,$17,-1`, so `lw $2,0($16)` feeds `jal $31,$2` back-to-back. Moving `count--` ahead of `f()` (measured FREE on this chassis in s11 F13 cell V7; semantically identical since count is a local the callee cannot observe) gives the scheduler a second delay-slot candidate, the target's loop schedule is restored and the cell scores 13 at 25 instructions. Three arrangements reach 13 at depth 1: bump (p 6/8 = 15000), read+bump pair (7/8 = 17500), initialiser+bump split (7/8 = 17500). Diagnostic X6 — an EMPTY do-while(0) in the same position, zero extra references — still emits 26 insns at score 21, proving the fence is the NOTE and the statement order is what pays for it. Banked as rejected/f14-in-walk-wrap-costs-load-delay-nop.c.
- verdict: KILLED

## [s12] FR2 — floor 13 is reachable with a do-while(0) wrap depth below 2.
- mechanism: FR2's criterion, open since s8, is score 13 at wrap depth <= 1. Session 11 priced the only route it could see (count-assigned-first wins the roles at depth 1 but pays three residual points of materialisation order, 16) while holding the PLACEMENT fixed at p's initialiser. With the note on the bump, p reaches 6 weighted refs at one level with p still assigned FIRST, so no materialisation-order penalty is incurred at all.
- probe: Cell X1_dec_before_call at depths 1 and 2 in f14bsweep.py, sandbox + instrumented cc1, followed by the FR3-mandated stream re-verification: awk '/^motion_Close:/,/\.end/' over the cell's cc1 output diffed against tmp/grind/motion_Close/s10/f12_G_goto_tail_d2.s (the stream s10 verified against s5b/wsA/_base.txt, which the s7 residual table was written from).
- result: CONFIRMED — score 13, build_insns 25, at a SINGLE do-while(0) level (depth 2 is also 13/25, so one level is the whole device): p 6 refs / live_length 8 = 15000 in $s0 against count 5/7 = 14285 in $s1. The stream is IDENTICAL to the s10-verified chassis except for the internal label NUMBER (.L136 vs .L144 — fewer loop notes, so the label counter lands lower): same 25 instructions, same registers, same `.frame $sp,32,$31 # vars= 0, regs= 3/0, args= 16`, same filled beqz delay slot, same descending save order, same $v0 temp. The s7 residual table describes the new form verbatim; candidate.c now holds it and do-while-zero-exception prerequisite 3 (nested-wrap exhaustion) no longer applies at all.
- verdict: CONFIRMED

## [s12] Wrap depth 0 — floor 13 with no do-while(0) anywhere — is reachable on this chassis.
- mechanism: With the depth-1 criterion met, the residual question is whether the device can be removed entirely. Priority is floor_log2(n)*n/live_length*10000; the measured depth-0 constants are p 4 raw refs / live_length 8 = 10000 against count 5/7 = 14285, with count holding the lower allocno number in the banked declaration order.
- probe: Closed-form enumeration over the priority formula using the constants measured in every depth-0 cell of both sweeps (7 placements; all reproduce p 4/8 = 10000, count 5/7 = 14285, score 20).
- result: KILLED. Flipping the roles at depth 0 requires exactly one of: (a) a real extra p reference — the target's 26 instructions contain no p reference our 25 lack, so any such reference is foreign; (b) p live_length <= 5 (2*4/5 = 16000) — impossible, p's range spans the walk and the only way to shorten it is to move p's birth after count's, i.e. the count-assigned-first form measured at 16; (c) count's priority pushed to <= 10000, i.e. live_length >= 10 at 5 references, needing three-plus insns of intervening real computation the target does not perform. One wrap level is the MINIMUM policy device for floor 13 on this chassis — a derived floor, not an unbeaten measurement.
- verdict: KILLED
