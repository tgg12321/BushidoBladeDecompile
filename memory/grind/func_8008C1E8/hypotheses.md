# Hypothesis ledger — func_8008C1E8

## s1 (2026-08-18, recon) — floor 28 -> 23

### [s1-H1] Lost WIP edits were the floor gap — CONFIRMED (28 -> 25)
The 2026-08-06 WIP session's uncommitted src edits (documented floor 27) were
lost; src at session start still carried the `register s32 r_arg1 asm("s4")`
pin and the `__asm__("la %0, D_800F1AE2")` block (sandbox stripped 69 asm
insns). Re-applying the documented replacements — drop the pin, use `arg1`
directly, `volatile u16 *p_ae2 = &D_800F1AE2;` for the la — measured 25
(better than the documented 27; insn counts 159/159). All honest cheat
removal, banked into candidate.c.

### [s1-H2] `s1 = 0` belongs INSIDE the loop_flag[3] if-arm — CONFIRMED (25 -> 24)
Target idx 117 executes `addu $s1,$zero,$zero` in the `bne ... .L8008C410`
delay slot, and idx 118-119 immediately clobber s1 with `lui/addiu
%lo(D_800F1AF8)` on fall-through. So the zero is live ONLY on the taken
path: the original C is
`if ((...&0x80) != loop_flag[3]) { s1 = 0; goto loop_continue; }`
and reorg fills the delay slot from the branch-target thread. Our previous
C (`s1 = 0;` after the if) was semantically DIVERGENT from target (taken
path kept s1==s5) and its store was DCE'd. Probe: if-arm spelling measured
24. The unconditional-before-the-if spelling measured 26 (sched1 hoists the
store into the lhu load-delay slot instead — rejected/, do not re-try).

### [s1-H3] loop_continue read needs full address materialisation — CONFIRMED (24 -> 23)
`if (D_800F1AF4 != 0) goto outer_top;` (direct extern-volatile read) lets
combine fold %lo into the load (`lui; lw %lo(sym)(reg)`), target has full
`lui; addiu; lw 0(reg)`. Reading through a block-local
`volatile s32 *p_af4c = &D_800F1AF4;` reproduces the full form (same
spelling as the two return sites). Measured 23, build_insns 159 -> 160
(the +1 addiu now matches target's count at that site).

### [s1-H4] s0=0 at main_work head collapses the trampoline but regresses — KILLED (23 -> 31)
Moving `s0 = 0;` from before `goto main_work;` to the head of main_work
(hoping reorg steals it into the entry beqz delay slot) did collapse the
`j main_work; move s2,zero` pair (160 -> 159 insns) but scored 31. Same
failure family as the 2026-08-06 "hoist s0=0 above the flag test" negative
(27 -> 33). Conclusion: the entry cluster is COUPLED to the arg-home
problem — target's beqz delay slot is free for s0=0 only because
`move s4,a1` already happened at prologue position 3. Rejected/, banked.

## Live frontier after s1 (in priority order)

1. **Arg-home + s-register rotation (the dominant ~20-point cluster).**
   Target homes a1 -> s4 at insn 3 (with `sw s4` hoisted to prologue front);
   ours homes a1 -> s3 in the entry beqz delay slot. Full rotation:
   arg1 s3<->s4, outer counter s2<->s0, loop_flag copy s0<->s3,
   wait_val s4<->s2 (s1, s5 already match). Twin func_8008BF04 carries the
   IDENTICAL wall as regfix (`$19 <-> $20 @ 0-134`, `$16 <-> $17 @ 68-121`)
   — one mechanism, two functions. Next probe (forensics modality): -da greg
   dump via the instrumented cc1 (tools/gcc-2.7.2/cc1, BB2_ALLOC_DEBUG) —
   read allocno order/priorities for the four pseudos, find which C lever
   (ref counts, live-range shape, declaration order) flips the assignment to
   target's (counter->s0, wait_val->s2, loop_flag->s3, arg1->s4). Note
   target's assignment gives ARG1 the HIGHEST callee-save number = LOWEST
   allocation priority of the four; ours gives arg1 mid priority.
2. **Entry trampoline collapse (2 insns, `j main_work; move s2,zero`).**
   Only after (1): once arg1 homes in the prologue, the beqz delay frees and
   the `s0 = 0;` placement levers (goto-head steal, straight-line) should be
   re-swept — standalone attempts measured 31 (s1) and 33 (2026-08-06).
3. **Volatile ruling for D_800F1AEC block (COMPLETED-C gate, not a score
   item).** `volatile s32 *flag/loop_flag` is load-bearing (de-volatilize =
   27->39 measured 2026-08-06). Need the two-prong
   legitimate-volatile-interrupt-touched finding: identify the ISR mutating
   D_800F1AEC/+4/+8/+0xC (SPU/serial packet state; sibling grant comment at
   main.c:3430, func_8008C184 already COMPLETED-C with extern volatile on
   the same block). Also classify the p_af4/p_af8/p_af4c/p_ae2 pointer
   blocks (natural read spelling vs pointer-alias family needing FAKE
   annotation) BEFORE any candidate-ready.

## [s1] The lost 2026-08-06 WIP cheat-removals (drop s4 pin, replace la-asm with &D_800F1AE2) account for the 28-vs-27 floor gap
- mechanism: Honest C address-of reproduces the lui/addiu/lw materialisation the inline asm produced; the pin and asm were score-inert cheats being stripped by the sandbox
- probe: Re-applied the documented replacements to src/main.c, sandbox --disable all
- result: 28 -> 25, insn counts 159/159 (better than the documented 27)
- verdict: CONFIRMED

## [s1] Original C has `s1 = 0;` INSIDE the loop_flag[3] if-arm (taken-path-only), not after the if
- mechanism: Target executes addu s1,zero,zero in the bne delay slot and clobbers it immediately on fall-through (lui s1) => zero is live only on the taken path; reorg fills the delay slot from the branch-target thread. Old C was semantically divergent from target here
- probe: if (cond) { s1 = 0; goto loop_continue; } spelling; also probed the unconditional-before-if spelling
- result: if-arm: 25 -> 24. Unconditional: 26 (sched1 hoists the store into the lhu load-delay slot instead) — banked in rejected/
- verdict: CONFIRMED

## [s1] loop_continue's D_800F1AF4 test needs full lui/addiu/lw address materialisation
- mechanism: Direct extern-volatile global read lets combine fold %lo into the load; reading through a block-local volatile s32 * reproduces target's full form (same spelling as the two return sites)
- probe: volatile s32 *p_af4c = &D_800F1AF4; if (*p_af4c != 0) goto outer_top;
- result: 24 -> 23 (build_insns 159 -> 160, matching target's extra addiu at that site)
- verdict: CONFIRMED

## [s1] Placing s0=0 at main_work's head lets reorg steal it into the entry beqz delay slot and collapse the j/move trampoline
- mechanism: reorg fill from single-predecessor taken thread
- probe: Moved s0=0 from before `goto main_work;` to the block head
- result: Trampoline collapsed (160->159 insns) but score 23 -> 31; entry cluster is coupled to the arg-home (ours occupies the beqz delay with move s3,a1). Same family as the 2026-08-06 negative (27->33). Banked in rejected/
- verdict: KILLED
