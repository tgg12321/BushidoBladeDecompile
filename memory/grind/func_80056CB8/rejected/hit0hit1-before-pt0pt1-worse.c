/* s71 (structural modality) — REJECTED.
 * Swapped declaration order: hit0[4]/hit1[4] declared BEFORE pt0[4]/pt1[4]
 * (baseline order is pt0,pt1,hit0,hit1,work). All statements unchanged.
 * MEASURED: sandbox func_80056CB8 --disable all: 38/204 -> 85/204
 * (build_insns 198 -> 201). Substantially worse on both axes.
 * This is a genuinely untried inter-array declaration-order axis (s24 only
 * tried "all five arrays vs the three scalars" as one block, never a
 * pairwise array/array reorder). KILLED, instance-scoped, this chassis.
 */
