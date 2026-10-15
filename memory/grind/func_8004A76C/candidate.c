/*
 * func_8004A76C — no C candidate exists.
 *
 * asm/funcs/func_8004A76C.s is a 39-insn hand software-pipelined GTE
 * (cop2) perspective-transform loop (rtpt + mfc2 of GTE result regs,
 * lwc2/swc2 vertex I/O). GTE ops have no C form (AGENTS.md inline-asm
 * policy). scan_hand_coded --single: tier=STRONG 1/8 (S7, unsaved $s0).
 * The lone caller func_8004A4E0 hands $s0 live across the call in a way
 * GCC cannot emit (its own canonical audit); the loop body's rtpt/entry-j
 * pipelining shape is hand-scheduled, not GCC-schedulable. Closed by
 * docs/audits/INLINE-ASM-AUDIT-2026-10-01.md item A6 ("CLOSED, no
 * change"); re-confirmed memory/grind/func_8004A76C/hypotheses.md s1 H1
 * (class kill). src/main/3AB48.c:30 stays
 * INCLUDE_ASM("asm/funcs", func_8004A76C);
 */
/*
 * s5 (synthesis): two honest-C syntheses measured and banked under
 * rejected/ (33 natural loop, 30 hand-pipelined goto form) -- both move the
 * vertex pointer from the target's unsaved $s0 to $a3, a false ABI claim
 * against the hand-asm caller, and the sandbox strips gte_rtpt() (and local
 * ldv3c/stsxy3c) as unpinned engine/gtemacro.py units, so neither score is a
 * shippable floor. The stub above stays the candidate.
 */
/*
 * s7 (forensics): raw instrumented cc1 -da probes (tmp/grind/func_8004A76C/s7/)
 * narrowed the residual of the closest form
 * (rejected/psyq-rtpt-pair-reorg-slot-residual.c) to four classes: $s0 base,
 * $t0-$t2 mfc2 seats, and two reorg.c delay-slot facts (beqz slot holds the
 * a0 decrement while the eligible `addiu a1` sits right before the branch;
 * `jr ra` slot left as nop after an eligible sh). The nop pad before the first
 * rtpt alone is reachable (PsyQ gte_rtpt + gte_rtpt_b). The stub stays.
 */
