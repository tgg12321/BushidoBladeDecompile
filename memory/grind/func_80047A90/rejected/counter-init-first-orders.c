/* REJECTED session 1: loop-2 init statement orders other than pt2;pt1;k-last.
 *   k = 1; pt2 = ...; pt1 = ...;   -> sandbox 21 (pt2->t1, pt1->a3, k->t2, a3off->t0)
 *   pt2 = ...; k = 1; pt1 = ...;   -> sandbox 18
 *   pt2 = ...; pt1 = ...; k = 1;   -> sandbox 14  (WINNER, in candidate.c)
 * Mechanism: earlier pointer inits lengthen pt1/pt2 live ranges, sinking their
 * global.c priority below a3off so they fall through to t1/t2 (target).
 * NB target's emitted order IS k-first (insn 30) — that order must come from
 * scheduling or another source shape, NOT from source k-first (measured wrong regs).
 * Do not re-propose k-first/k-middle source orders for this pointer-spelling family. */
