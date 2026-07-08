/* s43 rejected form / negative-mechanism bank:
 * Any hypothesis that expects the sched2 T-14 tie between insns 106 (a0-sll)
 * and 117 (v0-sll) to be CLASS-differentiated in rank_for_schedule
 * (sched.c:2419-2450) via insn_cost(117 for 120) > 1 is KILLED by the MIPS
 * pipeline definition: tools/gcc-2.7.2/config/mips/mips.md defines
 * function-units only for `memory` and `imuldiv`; the arith class (sll, addu,
 * ori, add, sub, and, or, xor, ...) has no define_function_unit, so
 * insn_cost falls to the default 1.
 *
 * Concretely: at T-14 with last_scheduled=120, both 117 (via cost==1 shortcut
 * at sched.c:2420) and 106 (no link) resolve to class=3. Ready-list qsort
 * falls through to INSN_LUID compare (sched.c:2455). Lower LUID sorts BEFORE
 * (117 emitted before 106 chronologically at T-15 vs T-14).
 *
 * Implication for candidate forms: NO C construct that changes the cost/
 * latency of 117->120 (or 120->122, or 128->111) via arith operand shape
 * can flip a sched2 tie in this window. The class axis is inert.
 *
 * The LUID lever remains the only sched2-tie-flip lever. LUID is controlled
 * by expand-time block-walk order, which the s2/s3 140-ordering sweep + the
 * y-family operand-swap probes fully closed. Ledger frontier P2
 * (saEft01Init exchange) is not affected — its bearing is a
 * cross-symbol/mirroring lever, not a sched2 tie-break.
 */
/* (no C body — this is a mechanism-KILL bank entry, not a candidate source form) */
