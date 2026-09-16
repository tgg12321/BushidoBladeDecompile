/* REJECTED (s3): hoisting the shared 0xFF constant written to D_800F4E18
 * and D_800F4E2E each per-voice iteration into a single named local read
 * twice, in place of the repeated literal 0xFF.
 *
 * Measured: sandbox --disable all regresses 19 -> 21 with this local
 * present; reverting restores 19 exactly. Killed as an instance on the
 * floor-19 candidate.c chassis (memory/grind/_SsVmInit/hypotheses.md s3 H10).
 *
 * The change (applied inside the `if (maxVoice != 0) { ... }` block, just
 * before the per-voice do-while loop):
 *
 *     s16 ff = 0xFF;
 *     ...
 *     *(s16 *)((u8 *)&D_800F4E18 + offset) = ff;   // was: = 0xFF;
 *     ...
 *     *(s16 *)((u8 *)&D_800F4E2E + offset) = ff;   // was: = 0xFF;
 *
 * Do not re-propose this exact form without a new producer mechanism —
 * GCC 2.7.2 allocates the fresh named local differently than the
 * twice-repeated immediate operand, and the resulting allocation measured
 * farther from target, not closer.
 */
