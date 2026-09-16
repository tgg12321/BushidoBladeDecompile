/* s71 (structural modality) — REJECTED.
 * Swapped declaration order: pt1[4] declared BEFORE pt0[4] (baseline:
 * pt0,pt1,hit0,hit1,work), all statements/hit0/hit1/work order unchanged.
 * MEASURED: sandbox func_80056CB8 --disable all: 38/204 -> 52/204
 * (build_insns UNCHANGED at 198 -- pure register/order shift, not an
 * insn-count regression). KILLED, instance-scoped, this chassis.
 */
