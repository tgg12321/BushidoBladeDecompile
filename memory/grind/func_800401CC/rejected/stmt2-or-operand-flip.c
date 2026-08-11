/* REJECTED — stmt2 OR-operand flip. Measured 7 -> 12.
 * `ot[0x3FFC/4] = ((s32)pkt & 0xFFFFFF) | (ot[0x3FFC/4] & 0xFF000000);`
 * GCC reuses the FIRST or-operand's register as the destination, so the or
 * lands in pkt's reg ($4) where target has it in the ot-word reg ($v0), and
 * the addiu/sw placement reshuffles. Target's or-dest proves the original
 * operand order is ot-first. The mask $6/$7 assignment did NOT flip either
 * (QTYDBG: FF000000 still shorter-lived at local-alloc). Do not re-propose;
 * same applies to the stmt1 flip (7 -> 9, load order perturbed).
 */
