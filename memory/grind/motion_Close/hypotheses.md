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
