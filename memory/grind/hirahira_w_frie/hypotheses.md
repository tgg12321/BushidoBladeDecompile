# Hypothesis ledger — hirahira_w_frie

## s2 (2026-07-17, structural)

1. KILLED — "sched1 orders a1's callee-save copy first naturally because the *offsets load feeds the entry branch (critical path)". Measured: fully clean form = 6 (a0 pair still first). Both chains tie at priority 4; rank_for_schedule (sched.c:2398) tie-breaks by INSN_LUID → expand_function_start's a0-first emission wins. The critical-path argument fails because the a0 chain (move+lw+addu) is exactly as deep as the a1 chain (move+lh+beq).

2. KILLED — "initializer/statement-order geometry can bias the copy-pair order". Measured: v1-as-first-decl-init = 6 (unchanged); inits-as-statements-after-read = 13 (worse — decl-position initializers for slots/count/dest are load-bearing for the s5/s6 prologue-region scheduling). Statement geometry cannot move the entry copies at all.

3. KILLED — "a semantically-true offsets walking cursor (single rename, forward role) flips the pair". Measured: 6, unchanged. The flip requires relocating the BASE (a0) copy, not the offsets copy.

4. CONFIRMED — "combine relocates a single-use param's entry copy to its rename-init position, flipping the pair without the reversed-pair shape". Measured: base-only rename `s32 *b = base;` = 2 (prologue pair fixed); + block-local `stop = -2` = 0. Mechanism: combine merges (move pA<-a0) + (move pb<-pA) placing the result at the later insn; a1's untouched entry copy then has lower LUID.

5. CONFIRMED — "the preheader pair (li s7,-2 before move s1,s5) requires an explicit constant statement before the walker init". Measured: clean+stop = 4 (exactly the 2-insn cluster closed vs clean 6). loop.c hoists the literal after the walker stmt; leaf-priority tie keeps emission order.
