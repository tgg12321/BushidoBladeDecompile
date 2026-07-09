/* REJECTED s8 probe 1: rederive idx_1495 = idx_1494 + 1 (honest pointer
 * arithmetic form matching prologue asm `addiu $s4, $s2, 0x1`).
 * Result: masked=15 (+13 vs h5 baseline of 2).
 * Reason: h5 basin depends on the cross-symbol tbl_125c-routed spelling
 * for its s-reg allocation web. Replacing it with the honest form
 * regresses file-scope RA (WIP notes.md warned about this coupling).
 * The forbidden cross-symbol spelling is currently load-bearing.
 * The remaining basin work MUST find a way to satisfy the prologue asm's
 * `addiu s4,s2,1` shape WITHOUT breaking the s-reg web h5 depends on. */
idx_1495 = idx_1494 + 1;
