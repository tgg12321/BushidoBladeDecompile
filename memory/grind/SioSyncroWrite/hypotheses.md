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

## s2 (2026-08-18, structural) — floor 23 -> 1

### [s2-H5] The 4-pseudo callee-save rotation is caused by MISSING LOOP NOTES, not by an allocator wall — CONFIRMED (23 -> 8)
- mechanism: GCC 2.7.2 emits NOTE_INSN_LOOP_BEG/END only for front-end loop
  constructs; a label+goto loop is invisible to loop.c and to flow.c's
  `loop_depth` weighting of reg_n_refs. global.c allocno_compare ranks allocnos
  by floor_log2(n_refs)*n_refs/live_length and find_reg assigns callee-saves in
  ascending REG_ALLOC_ORDER, so priority order == s-number order. Unweighted
  refs put `st` first; depth-weighted refs put `retries` (depth 2) first,
  exactly as in target.
- probe: rewrote all three loops as real C loops — outer `while (D_800F1AF4 != 0)`,
  wait loop `while ((*(volatile u16*)(D_800A3044+4) & 5) != 5)`, second wait
  loop `do { ... } while ((...&0x80) == *p_af8)` — and DELETED the hand-written
  `wait_val` local (it is loop.c's hoist of the literal 5, not a source var).
- result: 23 -> 8, build_insns 160 -> 157. All six callee-save assignments now
  match target (retries->s0, i->s1, 5->s2, st->s3, arg1->s4, pkt_len->s5), and
  loop.c reproduces target's two preheader hoists (`addiu s2,zero,5` and
  `addu s3,v1,zero`) plus expand_end_loop's duplicated top exit test.
- verdict: CONFIRMED

### [s2-H6] The outer loop MUST be a real loop — a goto outer loop costs the inner loops their depth-2 weighting — CONFIRMED (8 -> 17 when reverted)
- mechanism: with the outer loop spelled as goto, the inner loops are depth 1
  instead of depth 2 and the priority ordering degrades again.
- probe: vB — inner loops real, outer loop reverted to the s1 goto form with the
  `p_af4c` block-local exit test.
- result: 17 (build_insns 158). Banked as
  rejected/s2-outer-loop-as-goto-loses-loop-note-17.c
- verdict: KILLED (the s1 `p_af4c` 3-insn trick is NOT recoverable this way)

### [s2-H7] `retries = 0;` above the early-return guard frees the arg-home to the prologue — CONFIRMED (8 -> 3)
- mechanism: reorg's fill_simple_delay_slots scans backwards from the entry
  `beqz` for a movable insn. With `retries = 0;` after the guard it is not in
  that block at all, so the only candidate is the parameter home `move sX,a1`
  and it is stolen into the slot; the prologue then never schedules the home
  early. Placing the zero before the test gives reorg the closer candidate,
  the home stays at prologue position 3 and its anti-dependency hoists
  `sw s4,0x20(sp)` to the front of the save block — target's exact entry.
- probe: moved `s0 = 0;` above `if (*flag != 0) return -1;`.
- result: 8 -> 3, build_insns 157 -> 158. NOTE: the identical edit measured
  27 -> 33 on the all-goto body (2026-08-06) and `s0=0` at main_work head
  measured 23 -> 31 (s1-H4). It is worth +30 ONLY once the loop notes exist —
  the s1 "entry cluster is coupled to the arg-home" prediction was right, and
  the coupling runs through the allocation, not through scheduling.
- verdict: CONFIRMED

### [s2-H8] The loop-invariant copy `st = flag` must be declared INSIDE the loop body — CONFIRMED (3 -> 1)
- mechanism: declared before the `while`, the copy is emitted before the
  duplicated top exit test and reorg steals it into that test's beqz delay slot,
  displacing `i = 0`. Declared inside the body it is a loop invariant that
  loop.c hoists into the preheader — i.e. AFTER the top test — which is where
  target has it (`beqz ...; addu s1,zero,zero` / `addu s3,v1,zero`).
- probe: moved `volatile s32 *loop_flag = flag;` inside the while body.
- result: 3 -> 1. Only one instruction now differs in the whole function.
- verdict: CONFIRMED

### [s2-H9] The last instruction: the outer-loop exit test needs an UN-FOLDED address, and no natural spelling produces it — KILLED (4 spellings, all worse)
- mechanism: a named (REG_USERVAR_P) pointer local holding `&SYM` prevents
  combine folding the lo_sum into the MEM, giving target's 3-insn
  `lui; addiu; lw 0(r)`. That is how our three other 3-insn sites work (entry
  `*flag`, cleanup_B `*p_af4b`, the final `*p_af4`). The exit test is the only
  site where such a pointer cannot survive: loop.c's scan_loop will move any
  loop-invariant set out of the loop unless (per loop.c ~line 690) it is a
  non-uservar not used in the exit test, or set and used in the same basic
  block, or not guaranteed-executed — a pointer set anywhere sane in this body
  satisfies one of the move-me cases, and hoisted it costs a callee-save.
- probes / results (all banked in rejected/):
  * `volatile s32 *p_af4c;` assigned at the loop bottom, `} while (*p_af4c != 0);`
    -> 9, build_insns 161 (loop.c hoisted it — set and use in the same BB).
  * `volatile s32 *p_af4c = &D_800F1AF4;` declared before the do-while
    -> 11, build_insns 161 (live across the loop, buys a 7th callee-save).
  * ONE function-scope `volatile s32 *p_af4` shared by the exit test, cleanup_B
    and the final return (the reload-rematerialises-a-REG_EQUIV-constant theory)
    -> 22, build_insns 157. It gets a hard register and wrecks the allocation.
  * named VALUE local: `remaining = D_800F1AF4;` at the loop bottom,
    `} while (remaining != 0);` -> still 1 (the fold happens on the address, and
    naming the loaded value does not make the address a uservar).
- also measured: hand-written top guard + `do { } while (D_800F1AF4 != 0);`
  scores 1 exactly like the plain `while` — the residual is identical.
- verdict: KILLED for the pointer-idiom family. The remaining untried shapes are
  `for (;;)` / `while (1)` with a mid-loop `break` (which WOULD give the exit
  test its own basic-block-local pointer) — NOT proposed here: the frozen SOTN
  list names `while(1){...;break;}` as unsanctioned, and a bottom-exit loop has
  `do/while` as the simpler equivalent form, so it fails cheat-tests T1/T2.

## Live frontier after s2 (in priority order)

1. **The single missing `addiu` at the outer-loop exit test.** Everything else
   in the function matches. Next probes, in order: (a) forensics — `-dL`/`-dc`
   cc1 dumps around loop.c and combine to see exactly why the top duplicate and
   the bottom original diverge in target but not for us (they are byte-identical
   copies in our build, so target's two tests CANNOT both come from one C
   expression — the top is a folded direct global read, the bottom an un-folded
   one, which is strong evidence the original C has a hand-written top guard and
   a bottom test with a different spelling); (b) re-derive modality — look for a
   sibling PsyQ/Marionation routine with the same `while (countdown)` shape whose
   exit test compiles un-folded, and copy its spelling; (c) check whether the
   file-scope declaration of D_800F1AF4 (currently `extern volatile s32`) has a
   form that blocks the fold at the loop test but not at the top guard/store.
2. **COMPLETED-C gating work (not score work).** Before any candidate-ready:
   (a) the two-prong legitimate-volatile-interrupt-touched finding for the
   D_800F1AEC block — identify the SPU/serial ISR that mutates
   D_800F1AEC/+4/+8/+0xC (DeliverEvent 0xF000000B is the hook to trace);
   (b) classify the four block-local `volatile T *p = &GLOBAL;` reads
   (p_ae2, p_af8, p_af4b, p_af4) — natural read spelling vs a coercion family —
   and write self_vet.md.
3. **Twin func_8008BF04 (main.c:3337).** Same goto-loop spelling, same
   signature, carries regfix `$19<->$20 @ 0-134` + `$16<->$17 @ 68-121` — the
   identical rotation. Rewriting its loops as real C loops should retire those
   rules outright. Highest-value transfer of this finding.

## [s2] The dominant 4-pseudo callee-save rotation is caused by the body being written as label+goto loops, which emit no NOTE_INSN_LOOP_BEG/END, so loop.c never runs on this function and flow.c never loop-depth-weights reg_n_refs for global.c's allocno_compare.
- mechanism: allocno_compare ranks allocnos by floor_log2(n_refs)*n_refs/live_length and find_reg assigns callee-saves in ascending REG_ALLOC_ORDER, so priority order equals s-number order; flow.c adds loop_depth to reg_n_refs per reference. Unweighted refs rank the loop-state pointer first; depth-2-weighted refs rank the retry counter first, exactly as in target.
- probe: Rewrote all three loops as real C loops (outer while, wait while, second wait do-while) and deleted the hand-written wait_val local; sandbox func_8008C1E8 --disable all.
- result: 23 -> 8, build_insns 160 -> 157. All six callee-save assignments now match target exactly (retries->s0, i->s1, const-5->s2, st->s3, arg1->s4, pkt_len->s5); loop.c reproduces target's two preheader hoists (addiu s2,zero,5 and addu s3,v1,zero) and expand_end_loop's duplicated top exit test.
- verdict: CONFIRMED

## [s2] The outer loop specifically must be a real loop; reverting it to a goto loop (to keep s1's 3-insn p_af4c exit test) costs the inner loops their depth-2 weighting.
- mechanism: With no outer NOTE_INSN_LOOP_BEG the inner loops are depth 1, and the allocno priority ordering degrades back toward the s1 rotation.
- probe: vB: inner loops real, outer loop reverted to the s1 goto form with the p_af4c block-local exit test.
- result: 17 (build_insns 158). Banked as rejected/s2-outer-loop-as-goto-loses-loop-note-17.c
- verdict: KILLED

## [s2] Placing the retry-counter zero above the early-return guard frees the parameter home to stay in the prologue and reproduces target's entry shape.
- mechanism: reorg's fill_simple_delay_slots scans backward from the entry beqz; with the zero placed after the guard the only movable candidate is the a1 parameter home, which gets stolen into the slot. With the zero before the test reorg takes it instead, the home stays at prologue position 3, and its anti-dependency hoists sw s4,0x20(sp) to the front of the save block.
- probe: Moved s0 = 0; above if (*flag != 0) return -1;
- result: 8 -> 3, build_insns 157 -> 158. Entry prologue, beqz delay slot and the inline j / li v0,-1 return now match target. The identical edit measured 27 -> 33 on the all-goto body (2026-08-06), so it is worth +30 only once the loop notes exist, confirming s1's 'entry cluster is coupled to the arg-home' prediction.
- verdict: CONFIRMED

## [s2] The loop-invariant copy of the state pointer must be declared INSIDE the outer loop body, not before it.
- mechanism: Declared before the while, the copy is emitted before the duplicated top exit test and reorg steals it into that test's beqz delay slot, displacing i = 0. Declared inside the body it is a loop invariant that loop.c hoists into the preheader, i.e. after the top test, which is target's exact placement.
- probe: Moved volatile s32 *loop_flag = flag; inside the while body.
- result: 3 -> 1. One instruction now differs in the entire function.
- verdict: CONFIRMED

## [s2] The last missing instruction (target's un-folded lui/addiu/lw at the outer-loop exit test) can be produced by the same named-pointer idiom that already yields the un-folded form at the function's three other read sites.
- mechanism: A named REG_USERVAR_P pointer local holding &SYM blocks combine from folding the lo_sum into the MEM. But loop.c's scan_loop moves any loop-invariant set out of the loop unless it is a non-uservar not used in the exit test, or set and used in the same basic block, or not guaranteed-executed; every sane placement in this body hits a move-me case, and once hoisted the pointer costs a 7th callee-save.
- probe: Four spellings measured: pointer assigned at the loop bottom and used in the do-while condition; pointer declared before the do-while; one function-scope pointer shared by the exit test, cleanup_B and the final return (the reload/REG_EQUIV-rematerialisation theory); and a named VALUE local remaining = D_800F1AF4; with while (remaining != 0).
- result: 9 (161 insns), 11 (161 insns), 22 (157 insns), and 1 (unchanged, because naming the loaded value does not make the address a uservar). Also measured: hand-written top guard plus do-while scores 1 with an identical residual. All banked in rejected/.
- verdict: KILLED

## s3 (2026-08-18, structural) — floor 1 -> 1; the exit-test pointer family is now closed from both ends

### [s3-H1] A named pointer local can hold the exit-test address inside the loop if its set is in a different basic block from its use — CONFIRMED as a mechanism, KILLED as a match (1 -> 6)
- mechanism: tools/gcc-2.7.2/loop.c:688-701. scan_loop moves a loop-invariant
  set only if ONE of (1) `reg_in_basic_block_p` (set + all uses in one BB),
  (2) `! REG_USERVAR_P && ! REG_LOOP_TEST_P`, (3) `! maybe_never &&
  ! loop_reg_used_before_p`. A named C pointer fails (2) by construction, so
  putting its assignment in a different BB from the load (and past a label, so
  `maybe_never` is 1) defeats all three and the `la` stays in the loop.
- probe: `volatile s32 *p_af4c;` in the function declaration block, assigned
  `p_af4c = &D_800F1AF4;` (a) immediately before `if (i == pkt_len)` and
  (b) immediately after the inner wait loop; loop rewritten as
  `if (D_800F1AF4 != 0) do { ... } while (*p_af4c != 0);`.
- result: BOTH 6, build_insns 158. Disassembly (tmp/grind/SioSyncroWrite/s3/
  vA2.fn.txt, offsets 0x5bc4/0x5bc8) shows the `la` present inside the loop as
  predicted — but materialised into **s2, a callee-save**, because the pointer
  is live across the `if (i == pkt_len)` block which contains the
  DeliverEvent/callback calls. It displaces loop.c's const-5 preheader hoist.
  Target's address register is $v0, dead one insn later.
- verdict: KILLED. Banked as
  rejected/s3-loop-body-pointer-before-iflen-takes-callee-save-6.c and
  rejected/s3-loop-body-pointer-after-waitloop-takes-callee-save-6.c

### [s3-H2] THE STRUCTURAL IMPOSSIBILITY (the session's main result)
Target's exit test needs the address (a) materialised inside the loop AND
(b) in a caller-save temp dead one instruction later, i.e. set adjacent to the
load. "Set adjacent to the load" IS loop.c case (1) — `reg_in_basic_block_p` —
which always makes the set movable and gets it hoisted (s2 measured 9 / 11 / 22
for those placements). "Not hoisted" requires the set to be in a different basic
block, which forces the pointer live across the call-carrying if-block and
therefore into a callee-save (s3-H1: 6). The two requirements are mutually
exclusive for ANY C pointer variable in this loop shape. No further pointer
spelling can succeed; stop proposing them.

### [s3-H3] Target's three un-folded sites come from ONE pointer that lost its hard register (reload REG_EQUIV rematerialisation) — PLAUSIBLE, not reachable from C (4 configurations, all worse)
- mechanism: a single `&D_800F1AF4` pseudo hoisted by loop.c, denied a hard reg
  by global.c's find_reg (a 7th callee-save is not worth its save/restore cost),
  and rematerialised from its REG_EQUIV symbol_ref at each use — which is the
  only story that explains target having un-folded reads at cleanup_B
  (0x8008C244), the exit test (0x8008C410) and the final return (0x8008C428)
  while the top guard (0x8008C2A4) stays folded. Supporting style evidence: every
  reference to D_800F1AEC in the verbatim Sony COMB object
  (asm/funcs/_comb_control.s:84/258/356/549/670/723) is the un-folded form, i.e.
  Sony's LIBCOMB C holds block bases in pointer variables.
- probe: four ref-count / live-range configurations of one shared pointer, all
  on the neutral `if (guard) do { } while (*pc != 0);` chassis, trying to make
  global.c decline the callee-save.
- result: exit test + final return = 16 (159 insns); exit test only = 11 (161);
  all three sites declared before the loop = 19 (157); all three declared at the
  function top = 22 (157, reproducing s2's banked figure exactly). The pointer
  wins a hard register in every configuration.
- verdict: KILLED for C-level control. Whether the allocno is spilled is a
  global.c cost decision with no C lever. The next probe for this instruction is
  FORENSICS on .greg/.lreg, not another spelling.

### [s3-H4] The do-while chassis with a hand-written top guard is score-neutral — CONFIRMED (1 -> 1)
- probe: `if (D_800F1AF4 != 0) do { ... } while (D_800F1AF4 != 0);` replacing the
  plain `while`.
- result: 1, build_insns 158 — identical residual. Re-confirms s2's note; the
  chassis is free and remains available as a carrier for future exit-test ideas.
- verdict: CONFIRMED (neutral)

### [s3-H5] The D_800F1AEC volatile can move from the pointer type to the file-scope declaration at zero cost — CONFIRMED (1 -> 1, sibling 0 -> 0)
- mechanism: the body currently spells `volatile s32 *flag = &D_800F1AEC;` over
  an `extern s32 D_800F1AEC;`, i.e. it ADDS a qualifier through a pointer type —
  which reads as volatile-coercion-by-cast. If the volatile is original
  semantics, the natural spelling is the file-scope `extern volatile s32`, which
  is what the legitimate-volatile-interrupt-touched carve-out grants (and what
  the four sibling words of the same struct already have).
- probe: sed the declaration at src/main.c:3431 to `extern volatile s32
  D_800F1AEC;` and score both consumers of the symbol.
- result: SioSyncroWrite 1 -> 1 (158/159); the already-matched sibling
  SioAnsyncWrite 0 -> 0 (25/25). Score-neutral, no regression on a COMPLETED
  function. Reverted in-session (the allowlist entry needs a layer-2
  cheat-reviewer + operator commit-audit block a grind session cannot produce).
- verdict: CONFIRMED. Two-prong evidence is complete: IRQ writer HandleSio
  `sw $zero,0x0($a0)` @0x8008CCD4 (asm/funcs/_comb_control.s, $a0 = &D_800F1AEC
  materialised @0x8008CC78); use-site constructs = double-read-across-
  sequence-point (`st[1] += 1; st[1];`) + IRQ-mutated-loop-bound (the +8
  countdown drives the outer loop). Draft allowlist entry in evidence.md.

## Live frontier after s3 (in priority order)

1. **The single missing `addiu` — now a FORENSICS question, not a spelling one.**
   Every C-level spelling of the address materialisation is dead (s2: 4, s3: 6).
   The next probe is to read the allocation dumps: `pwsh tools/grinder/dump.ps1
   SioSyncroWrite`, then the SioSyncroWrite slice of main.lreg / main.greg, for
   the shared-pointer configuration (rejected/s3-shared-pointer-three-sites-
   do-while-19.c) — find the allocno for the `&D_800F1AF4` pseudo, read its
   priority and which hard reg find_reg gave it, and determine what would have
   to differ for it to be spilled with its REG_EQUIV intact. If nothing in the
   C can change that, this function is a candidate for an endgame-lock
   escalation on ONE instruction — but only the driver may declare exhaustion,
   and the forensics and re-derive modalities are both still untried.
2. **COMPLETED-C gating work.** The D_800F1AEC two-prong finding is now DONE
   (evidence.md, [s3-H5]); what remains is (a) an operator/layer-2 filing of the
   volatile_extern_allowlist.txt entry so the body can use the natural
   declaration, and (b) self_vet.md covering the four block-local
   `volatile T *p = &GLOBAL;` reads — note p_ae2 / p_af8 / p_af4b / p_af4 all
   point at globals that are ALREADY `extern volatile`, so they add no qualifier
   and are pure address-materialisation spellings.
3. **Twin SioSyncroRead (src/main.c:3337).** Still goto-loop spelled, still
   carrying regfix `$19<->$20 @ 0-134` + `$16<->$17 @ 68-121` — the identical
   rotation the s2 loop-note rewrite dissolves here. Highest-value transfer of
   this ledger's findings; out of scope for a SioSyncroWrite session.

## [s3] A named pointer local can keep the exit-test address materialisation inside the loop if its assignment sits in a different basic block from its use.
- mechanism: tools/gcc-2.7.2/loop.c:688-701 — scan_loop moves a loop-invariant set only if ONE of (1) reg_in_basic_block_p (set and all uses in one basic block), (2) !REG_USERVAR_P && !REG_LOOP_TEST_P, (3) !maybe_never && !loop_reg_used_before_p. A named C pointer fails (2) by construction, so a cross-basic-block assignment past a label defeats all three and the la survives in the loop.
- probe: volatile s32 *p_af4c; declared in the function declaration block, assigned p_af4c = &D_800F1AF4; (a) immediately before if (i == pkt_len) and (b) immediately after the inner wait loop, with the loop rewritten as if (D_800F1AF4 != 0) do { ... } while (*p_af4c != 0); sandbox --disable all plus objdump of the sandbox object.
- result: Both spellings scored 6 (build_insns 158). The la IS present inside the loop, exactly as predicted (tmp/grind/SioSyncroWrite/s3/vA2.fn.txt, offsets 0x5bc4/0x5bc8: lui s2,%hi(D_800F1AF4); addiu s2,s2,%lo) — but it materialises into s2, a CALLEE-SAVE, because the pointer is live across the if (i == pkt_len) block which contains the DeliverEvent/callback calls. It displaces loop.c's const-5 preheader hoist. Target's address register is $v0, dead one instruction later.
- verdict: KILLED

## [s3] No C pointer variable can produce target's exit-test form, because the two properties target requires are mutually exclusive under loop.c's movable test.
- mechanism: Target needs the address (a) materialised inside the loop and (b) in a caller-save temp dead one instruction later, i.e. set ADJACENT to the load. Set-adjacent-to-load is exactly loop.c case (1) reg_in_basic_block_p, which always makes the set movable and gets it hoisted. Not-hoisted requires a different basic block, which forces the pointer live across the call-carrying if-block and hence into a callee-save.
- probe: Combination of s2's three same-basic-block placements (9 / 11 / 22) and s3's two different-basic-block placements (6 / 6), read against the loop.c source rather than inferred.
- result: Every same-BB placement is hoisted; every different-BB placement costs a callee-save. The family is closed from both ends. Six distinct pointer spellings are now banked in memory/grind/SioSyncroWrite/rejected/.
- verdict: CONFIRMED

## [s3] Target's three un-folded D_800F1AF4 reads come from ONE shared pointer that lost its hard register and is rematerialised by reload from its REG_EQUIV symbol_ref.
- mechanism: A single &D_800F1AF4 pseudo hoisted by loop.c, denied a hard reg by global.c find_reg (a 7th callee-save is not worth its save/restore cost), rematerialised at each use. This is the only story that explains target having un-folded reads at cleanup_B (0x8008C244), the exit test (0x8008C410) and the final return (0x8008C428) while the top guard (0x8008C2A4) stays folded as a hand-written direct global read. Style support: every reference to D_800F1AEC in the verbatim Sony COMB object (asm/funcs/_comb_control.s:84/258/356/549/670/723) is the un-folded form, i.e. Sony's LIBCOMB C holds block bases in pointer variables.
- probe: Four ref-count / live-range configurations of one shared pointer on the neutral hand-guard + do-while chassis, aiming to make global.c decline the callee-save.
- result: exit test + final return (2 refs) = 16 (159 insns); exit test only (1 ref) = 11 (161); all three sites declared before the loop = 19 (157); all three declared at function top = 22 (157, reproducing s2's banked figure exactly, so the chassis is consistent). The pointer wins a hard register in every configuration. Whether the allocno is spilled is a global.c cost decision with no C-level lever.
- verdict: KILLED

## [s3] A hand-written top guard plus a do-while is score-neutral versus the plain while, so it is a free carrier for exit-test experiments.
- mechanism: expand_end_loop's duplicated top exit test and a hand-written if-guard produce the same folded 2-insn read; the loop notes that drive the allocation are emitted for do-while exactly as for while.
- probe: if (D_800F1AF4 != 0) do { ... } while (D_800F1AF4 != 0); replacing the plain while.
- result: 1, build_insns 158 — identical residual. Re-confirms s2's note.
- verdict: CONFIRMED

## [s3] The volatile on the D_800F1AEC packet-state block can move from the pointer type to the file-scope declaration at zero score cost, removing the body's only volatile-coercion-by-pointer-type surface.
- mechanism: The body spells volatile s32 *flag = &D_800F1AEC; over an extern s32 D_800F1AEC;, i.e. it ADDS a qualifier through a pointer type. If the volatile is original semantics the natural spelling is extern volatile s32, which is what .claude/rules/legitimate-volatile-interrupt-touched.md grants and what the four sibling words of the same struct already carry (volatile_extern_allowlist.txt:41-45).
- probe: Changed src/main.c:3431 to extern volatile s32 D_800F1AEC; and scored BOTH consumers of the symbol, then reverted. Separately traced the IRQ writer through asm/funcs/_comb_control.s.
- result: SioSyncroWrite 1 -> 1 (158/159) and the already-matched sibling SioAnsyncWrite 0 -> 0 (25/25) — score-neutral, no regression on a COMPLETED function. Two-prong evidence complete: HandleSio materialises $a0 = &D_800F1AEC at 0x8008CC78 (_comb_control.s:549-550) and stores sw $zero,0x0($a0) at 0x8008CCD4, clearing D_800F1AEC itself; use-site constructs are double-read-across-sequence-point (st[1] += 1; st[1];) and IRQ-mutated-loop-bound (the +8 countdown drives the outer loop). Reverted in-session because filing the allowlist entry needs the layer-2 cheat-reviewer plus commit-audit block that a grind session cannot produce; the draft entry text is banked in evidence.md.
- verdict: CONFIRMED
