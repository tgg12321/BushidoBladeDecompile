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

## LIVE FRONTIER

### F1 — motion_Close's original source was hand-written assembly (crt0 runtime)
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

### F2 — pin down whether the residual is EXACTLY the nine regfix rules
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

### F3 — the s0/s1 role + v0-vs-t0 temp assignment (LOW VALUE, do after F1/F2)
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
