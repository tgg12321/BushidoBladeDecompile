/* s3 P4 — KILLED, score 7 (probe-1 leak shape).
 * val = {(s32)(D_800F1164+3) hoisted before the re-test, 0x10016 in arm 1},
 * arm 2 stores val; repack natural. Live code (arm 2 consumes it), placement
 * mirrors target's delay-slot addiu a0,v1,1.
 * cse dump fact (tmp/grind/func_80061C00/s3/fn.cse): the D+3 value ALREADY
 * has a canonical pseudo — reg 95, born at the (D_800F1164+2)[1] lbu address
 * materialization in the check block. cse rewrote val's set to `val = reg95`
 * and substituted arm-2's store use to reg95 directly -> val's set dead ->
 * flow deleted -> const-only local -> leak (const->$v1, arg1->$a3).
 * GENERALIZED H10 LAW: the check-region D+3/D+2 values have canonical
 * anonymous pseudos (92, 95); ANY C-level named carrier of them is
 * substituted away regardless of placement. The check-region bytes
 * (incl. the delay-slot addiu) are already correct without help.
 */
void func_80061C00_p4_shape(void);
