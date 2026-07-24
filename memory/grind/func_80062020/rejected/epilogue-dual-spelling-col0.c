/* REJECTED — dual-spelling epilogue (col0 via symbol+index, cols 1,2 via pointer p).
 * This is the SAME family the audit-orch3a retro-audit FAILed (evidence.md).
 * It DOES reproduce target's split-addressing structure (col0 -> %hi(1198)+index -> %lo;
 * cols 1,2 -> 4(v0)/8(v0)) but it is a codegen steer: the terminator's three columns
 * are the same row, spelled two ways with no semantic purpose. And it STILL does not
 * close (score 12, not 0) because register alloc lands index in v0 (target wants v1)
 * and store order differs. Do NOT re-propose (both col0-first and col0-last variants).
 *
 * col0-first variant (measured s1, score 12):
 *   *(s32 *)((u8 *)&D_800F1198 + i12) = 0;   // col0 via symbol+index -> %hi/%lo split
 *   p = (s32 *)((u8 *)&D_800F1198 + i12);
 *   p[1] = 0;                                 // cols 1,2 via base pointer -> 4/8(v0)
 *   p[2] = 0;
 *
 * col0-last variant (== orch3a, retro-audit FAILed):
 *   p = (s32 *)((u8 *)&D_800F1198 + i12);
 *   p[2] = 0;
 *   p[1] = 0;
 *   *(s32 *)((u8 *)&D_800F1198 + i12) = 0;    // col0 re-spelled as full expr
 */
