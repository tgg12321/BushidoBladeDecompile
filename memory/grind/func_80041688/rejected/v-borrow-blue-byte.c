/* s16 (2026-08-20) — borrow of the existing TRUE-arm local v as the
 * FALSE-arm blue-byte carrier, DEAD by measurement:
 *
 *   } else {
 *       r = *((u8 *)player + 0x18);
 *       g = *((u8 *)player + 0x19);
 *       v = *((u8 *)player + 0x1A);
 *       func_80041398(v | ((r << 16) | (g << 8)));
 *   }
 *
 * sandbox --disable all = 12, build_insns = 80 (baseline 8/82).
 * The multi-set carrier DOES kill the birthing launch-boost (mechanism
 * correct), but pseudo v allocates to $v0 (its TRUE-arm call-result home),
 * so the FALSE-arm final or becomes `or a0,a0,v0` — IDENTICAL to the TRUE
 * arm's tail — and jump2/find_cross_jump merges the now-identical suffix,
 * losing 2 insns vs target. The carrier must allocate to $v1; among the
 * function's existing multi-set locals only b itself (via the loop1
 * setcc staging whose def-use range is too short to conflict with p —
 * s6/s7 forensics) achieves that with zero RA perturbation. i is homed in
 * $a0, arg1 in $a1, p/q are pointers (type-unnatural). Hence candidate.c's
 * staged-guard form is the unique live spelling of this lever.
 */
