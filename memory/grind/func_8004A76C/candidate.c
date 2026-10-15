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
