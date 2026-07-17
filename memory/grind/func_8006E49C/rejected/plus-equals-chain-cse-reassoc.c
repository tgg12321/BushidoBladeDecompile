/* REJECTED (s2): t += delta chain (attempt to keep t single-death so
 * local-alloc takes it with the $v0 return copy-suggestion, evicting the
 * const). s2 artifact probe4.c/.s.
 *
 * Measured: cse re-associates (set t (plus t d)) back to (set t (plus a0 K))
 * ONLY while arg0 is unchanged since t's last a0-based def (probe4: t+=0x438
 * became addu $3,$4,25080 = a0-based). Across an arg0 advance the a0
 * equivalence is invalidated and the insn STAYS t-relative (addu $3,$3,8600
 * vs target addiu v0,a0,0x1B58 — WRONG immediates/operands). And where cse
 * DOES re-associate, the rewritten insn no longer reads t, so flow (which
 * runs after cse) counts the per-segment deaths anyway — t never becomes
 * single-death. Both halves of the idea fail; bytes force a0-based defs,
 * a0-based defs force multi-death, multi-death forces global-alloc.
 *
 * Corollary banked as evidence: target bytes IMPLY the original's temp
 * pseudo was also multi-death/global-alloc'd, so the original evicted the
 * const from $v0 some other way (unknown; see hypotheses.md frontier).
 */
