/* func_8002BC68 — SANDBOX DISTANCE 0 form (s1, 2026-07-28) — DO NOT APPLY
 * WITHOUT OWNER RULING. Identical to candidate.c except the GTE asm block's
 * clobber list is extended to : "$12", "$13", "$14", "$15".
 *
 * Mechanism (bytes-proven): the 2-insn residual (mfhi $13 vs $24) is a
 * RELOAD-emitted hi->GP move; reload1.c order_regs_for_reload() puts every
 * hard reg explicitly mentioned in the RTL into bad_spill_regs; with only
 * "$12" clobbered, spill_regs[0]=$13; with $13-$15 also mentioned,
 * spill_regs[0]=$24 = target. No pure-C construct can put hard regs into
 * bad_spill_regs, so the original TU provably mentioned $13-$15 at asm level
 * in this function. Layer-1 cheat-reviewer FAILed the construct as a
 * register-pin-by-clobber-spelling absent independent evidence of the
 * original island's t4-t7 footprint. Owner ruling requested (see outcome s1
 * + docs/grind ruling question). Same construct closes func_8002BEA0's
 * mfhi rules (its regfix carries the identical $13->$24 subst pair).
 */
/* diff vs candidate.c: line `: "$12");`  -->  `: "$12", "$13", "$14", "$15");` */
