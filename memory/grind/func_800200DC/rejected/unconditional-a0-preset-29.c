/* REJECTED s2: score 29 (vs floor 6). m2c shows var_a0 = 0x12C assigned
 * BEFORE the disc>=0 test, so tried:
 *   s32 a0 = 300;
 *   if (disc >= 0) { ... }   // no else
 * The unconditional preset moves the li into the disc block at sched1 and
 * wrecks the schedule. The else-arm { a0 = 300; } + cross-jump form is the
 * one that byte-matches (m2c's shape is post-reorg appearance, not source). */
