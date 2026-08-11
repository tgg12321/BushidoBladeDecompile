/* REJECTED (session 2, structural) — drop the pointer variable entirely and
 * write the six accesses as plain global reads/writes, which is the shape m2c
 * reconstructs from the target asm (`D_80101EDA.unk0` / `D_80101EDA.unk44C`).
 *
 *   saved_first = D_80101EDA;
 *   saved_44c   = D_80102326;
 *   if (qf & 0x10) D_80101EDA = 0x32;
 *   if (q[3] & 0x20) D_80102326 = 0x32;
 *   func_8003AFFC();
 *   D_80101EDA = saved_first;
 *   D_80102326 = saved_44c;
 *
 * MEASURED: sandbox --disable all 6 -> 23 (much WORSE); build_insns 188 -> 187.
 *
 * WHY IT IS DEAD, AND WHAT IT PROVES: with no pointer variable, expand emits
 * every access as `(mem (symbol_ref))` / `(mem (const (plus sym 1100)))` and
 * cc1 NEVER creates a shared base register for them — all six sites assemble
 * to a lui/%lo pair.  GCC 2.7.2 has no pass that hoists a repeated symbolic
 * address into a register (verified by reading cse.c: find_best_addr returns
 * immediately on CONSTANT_ADDRESS_P, cse.c:2656, and nothing in cse_insn /
 * cse_process_notes materialises a constant into a fresh pseudo).
 *
 * Therefore target's `la $s0, D_80101EDA` + `0($s0)` / `1100($s0)` addressing
 * CAN ONLY come from a pointer variable in the C source — which is what the
 * current candidate already has.  This kills the "m2c shows plain global
 * member accesses, so write them that way" reading of the m2c output: m2c
 * renders a register-based struct access as `SYM.unkNNN` regardless of whether
 * the C had a pointer, so its rendering is not evidence about the C shape here.
 */
