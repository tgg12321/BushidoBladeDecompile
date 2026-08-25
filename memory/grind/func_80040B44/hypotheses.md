# Hypothesis ledger — func_80040B44

## STATUS AFTER s2 (structural): SOLVED — honest floor 0, frontier empty

`sandbox func_80040B44 --disable all` = **score 0**, `build_insns` 93 ==
`target_insns` 93, measured in s2 with `candidate.c` in place in
`src/text1a_pre.c`. Zero regfix/asmfix reliance, zero inline asm, zero FAKE
annotations, zero sanctioned-family claims. The self-vet is in `self_vet.md`.
The only outstanding work is the operator-side retirement of the 15 now-dead
regfix rules (`engine retire func_80040B44`) — a surface a grind session may not
touch. There is no remaining hypothesis to test; if a later session is dispatched
here, the correct first action is to re-measure the floor with `candidate.c`
applied, not to reopen anything below.

## Resolved hypotheses

### H1-ORDER — per-site addend order closes the 5 commutative addu diffs
VERDICT: **KILLED AS STATED, CONFIRMED IN CORRECTED FORM.** The mechanism s1
proposed (GCC preserves source addend order for PLUS when neither operand is
constant) is false for POINTER arithmetic: `pointer_int_sum()` in the C front end
(c-typeck.c) rewrites every pointer+integer sum to pointer-first before `fold()`
or `expand_expr` sees it. Rewriting the five sites while staying in the pointer
domain measured floor 32 -> 32, no emitted change at any site
(`rejected/h1-addend-order-in-pointer-domain-no-effect.c`). The same five sites
rewritten in the INTEGER domain (`a3 * 0x68 + (s32)t7`, `a3 * 4 + (s32)&seen[0]`,
`(t2 << 2) + (s32)arg0 + 0x1A34`) preserve source order and reproduce target's
index-first `addu` at all five: floor 32 -> **27**. Ordinary C, no family claimed.

### H2-FRESH — fresh per-loop locals fix the a0/a1/v0 init-loop seats
VERDICT: **PARTIALLY KILLED, CORRECTED FORM CONFIRMED.** Fully fresh locals per
loop (fresh counter AND fresh pointer, the m2c partition) measured 27 -> 24: it
fixes loop 2's pointer seat but leaves both counters mis-seated
(`rejected/h2-fresh-counter-per-init-loop.c`). Target holds $a0 for BOTH loop
counters, which means ONE counter variable spans both loops while the POINTERS
are fresh. Shared `i` + fresh `p1`/`p2` measured 24 -> **15**.
Trap banked: `i = 0x11;` placed before `s32 *p1;` inside the block is C89
declaration-after-statement, which this cc1 accepts SILENTLY and miscompiles —
the init-loop stores disappeared and the score fell to a meaningless 23 on a
shrunken body. Declare first; never trust a score drop that shrinks `build_insns`.

### H3-DEADDEC — honest producer for the dead `t3 -= 2` (target insn 89)
VERDICT: **RESOLVED — the premise was wrong.** The insn is not a lost source
statement at all, so branches (a)/(b)/(c) of s1's probe order are all moot, and
in particular the dead-store-FAKE fallback (c) was never needed and must not be
reached for by a future session.
Pass attribution (dumps, not inference): a source-level `t3 = t3 - 2;` is present
in the RTL through `.cse2` and absent in `.flow` — flow.c's `insn_dead_p`
deletion removes it, and no dead-store spelling survives that.
The actual producer is `reorg.c`. Writing the walking-pointer advance at the TOP
of the loop body (`t3 += 2; a3 = *t3; t3 += 2; ...body...; a0_val = *t3;`) with NO
preheader advance and NO post-loop compensation makes the delay-slot filler steal
the loop-top advance into the `bne` delay slot, replicate it in the preheader, and
emit the exit-path `addiu t3,t3,-2` compensation itself. Floor 15 -> **2**, and
`build_insns` closed 92 -> 93. The forbidden lost-codegen regfix @175 existed only
to fake compiler output.
GENERAL RULE for the pipeline: an exit-path instruction that exactly undoes a
delay-slot instruction is reorg.c loop-top-steal compensation. Look for a loop
whose advance sits at the top of the body; do not reach for a dead-store family.

### H4-SCHED — the 3-insn preheader order (`li t8,0xffff` before `addiu t4`)
VERDICT: **RESOLVED.** Not a scheduling question at all. `loop.c`'s
`move_movables` emits hoisted loop invariants immediately before
`NOTE_INSN_LOOP_BEG` — i.e. always at the very END of the preheader (confirmed in
the `.loop` dump: the `li 0xffff` is insn uid 290, sitting between the `t4`
initializer and the LOOP_BEG note). No ordering of preheader SOURCE statements can
place an explicit initializer after it, so an explicit `t4` can never match
(`rejected/p3-explicit-t4-walking-pointer.c`, floor 2).
The resolution is that `t4` is not a source variable: it is invariantly
`t5 + 0x58`, so the else arm is written against `t5` (`*(s16 *)((u8 *)t5 + 2)`,
`*(s32 *)((u8 *)t5 + 0x58)`, with `t4 += 0x68` dropped because `t5 += 0x68`
already runs) and loop.c's strength reduction creates `t4` as a giv. A giv
initializer is emitted AFTER the movable hoist, which is exactly target's order.
Floor 2 -> **0**.

### H5-ONE — is `s32 one = 1;` a rule-era holder that can just be deleted?
VERDICT: **CONFIRMED — deleted, byte-neutral.** Important nuance for the pipeline:
under the intermediate chassis the deletion was a REGRESSION (floor 6 -> 7,
because loop.c then hoists `li 1` to the end of the preheader alongside the
`li 0xffff`; `rejected/p2-no-one-local-both-constants-hoisted.c`). Only after H4
resolved did deleting it measure byte-NEUTRAL, and `*a1 = 1;` is the shipped form.
A suspect holder that "measures necessary" is only necessary RELATIVE to the
current chassis — re-test every one after each structural change.

### t2/t3 main-loop seat swap (s1 secondary, rule @157 class)
VERDICT: **RESOLVED, and the model is banked for reuse.** GCC 2.7.2
`global.c allocno_compare` ranks allocnos by
`floor_log2(reg_n_refs) * reg_n_refs / reg_live_length`; `reg_n_refs` is
loop-depth weighted (1 outside any loop, 2 inside one); MIPS defines no
`REG_ALLOC_ORDER`, so assignment is strict first-fit by ascending hard regno among
non-conflicting registers. Measured here: walking pointer 16 refs / 67 length ->
4*16/67 = .955 beat the `a0_val & 0xFFFF` temp at 10 / 34 -> 3*10/34 = .882, so
the pointer was allocated first and first-fit took $t2 where target wants $t3
(`rejected/h2c-single-pointer-var-16-refs-wrong-t2-t3-seat.c`, floor 15).
`floor_log2` is a step function, so 16 sits exactly on a cliff: one fewer weighted
ref (15) collapses the priority to .67 and reverses the order. H3's loop-top
advance sheds two and the seats land correctly for free.
CRITICAL COROLLARY (cost a full probe, `rejected/p1-split-preloop-pointer-seats-a2.c`,
floor 6): splitting the value across TWO pseudos also reverses the priority order
but can NEVER make them share a hard register. `global.c expand_preferences` only
unions preference SETS, and both are empty unless a pseudo is copied to/from a
hard reg, so the short-lived pseudo is allocated last and first-fit hands it the
lowest free regno ($a2 here) while target keeps the value in one register
throughout. When target computes a value into one register and keeps it there, the
C must use ONE variable — tune the REF COUNT, never the variable count.

## [s1] The migration byte-coupling is positional: INCLUDE_ASM(macro-inc) emission lands at the TU's .text start, not the function's mid-TU source position
- mechanism: file-scope asm include emitted ahead of compiled functions; B44 is 10th of 15 in text1a_pre.c
- probe: reproduced migration state exactly (sha e29f91cc == recorded); image diff: one 2788B range 0x800401CC-0x80040CB0 with B44's prologue at TU start and func_800401CC's signature at +372B; 11 scattered 1-2B diffs are %hi/%lo addends to shifted sibling symbols; tree restored, oracle MATCH re-verified
- result: coupling fully characterized; COMPLETED-C (compiled C in source position) has no coupling — solving the function is the clean exit
- verdict: CONFIRMED

## [s1] The 32-insn honest diff decomposes into 5 classes: a0/a1 init-loop seats, t2/t3 main-loop seats, 5 commutative addu operand orders, one 3-insn preheader schedule, and one DCE'd dead decrement (addiu t3,t3,-2)
- mechanism: side-by-side disassembly of honest sandbox build vs oracle bytes
- probe: tmp/grind/func_80040B44/s1/sidebyside.txt (93 insns, 41 marked incl. branch-addend false positives, 32 scoring)
- result: map banked in evidence.md with per-class rule attribution (rules 157-177)
- verdict: CONFIRMED — and every one of the five classes was closed in s2
