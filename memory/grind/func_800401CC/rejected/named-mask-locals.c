/* REJECTED — named mask locals to steer the $6/$7 assignment. No effect
 * (still 7, masks unchanged), in BOTH variants:
 *   (a) single `s32 addr_mask; ... addr_mask = 0xFFFFFF;` before the packet
 *       statements, used in both & sites;
 *   (b) `addr_mask = 0xFFFFFF; cmd_mask = 0xFF000000;` init FFFFFF-first.
 * QTYDBG measurement: allocation follows local-alloc qty_compare_1 priority
 * (floor_log2(refs)*refs*size/(death-birth), shorter-lived wins; tie -> lower
 * qty number). The FFFFFF value is born 4 luids earlier in every variant
 * (sched1 hoists its li+ori chain first, chain-length priority) and the two
 * tail ANDs are always re-packed adjacent by sched1, so FF000000 is always
 * ~2 luids shorter-lived and always allocated first -> always takes $6.
 * Birth order / first-encounter order is NOT the lever. The v0-split of stmt2
 * (early or late placement) is killed by the same re-packing.
 */
