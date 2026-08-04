/*
 * REJECTED (s1, 2026-08-03): ANY real-loop spelling of loop2 (do-while
 * measured here; `for` measured by WIP at 34) is dead.
 *
 * Variant A — do{...}while with all six invariant inits INSIDE the body:
 *   floor 35, build 73 insns. loop.c DID hoist the counter-init first and
 *   the cheap reg/const inits after it (a1, t4, t3, t2, t1, li-5 order),
 *   but the mode/t0 GLOBAL LOADS did NOT hoist (lh/lbu stayed in-loop:
 *   invariant_p rejects MEMs due to may-alias with the sh-through-a2
 *   stores; GCC 2.7.2 has no alias analysis), plus RA scrambled and an
 *   extra `andi vX,val,0xffff` truncation appeared.
 *
 * Variant B — do{...}while with decls OUTSIDE (floor-2 layout, only the
 * goto loop converted): floor 34, build 73. Loop notes made loop.c hoist
 * the bare constant 5 from `nibble != 5` into a register (`li t1,5` in
 * the preamble) and RA scrambled (pointer moved to $v1, t1 to $t0 etc.).
 *
 * KILL REASON (structural, applies to every note-carrying form): target
 * materializes 5 INSIDE the loop (`addiu $v0,$zero,5` at 0x8002134C) and
 * loads mode/t0 exactly once in the preamble. With loop notes, loop.c
 * always hoists the 5; without them it never relocates anything. So the
 * original loop2 was compiled WITHOUT loop.c LICM => goto-shaped source.
 * Do not re-try for/while/do spellings of loop2.
 */
