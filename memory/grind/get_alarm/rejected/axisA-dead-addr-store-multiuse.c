/* REJECTED (s7 forensics) — cheat-by-any-spelling; do NOT re-propose.
 *
 * Probe: give &D_8009BF68 a SECOND use inside func_8007DC9C so combine keeps the
 * address materialized in a pseudo (the 3-insn lui;addiu;lw 0(reg) target form).
 *
 *   D_8009BF88 = (s32)&D_8009BF68[0];   // dead store, overwritten below by temp_v0
 *   debug_printf(&D_80016044, D_8009BF68[0], D_8009BF6C, D_8009BF70);
 *
 * RESULT: objdump of the sandbox .o shows EXACTLY target asm lines 43-45:
 *   1e84 lui   v0,%hi(D_8009BF68)
 *   1e8c addiu v0,v0,%lo(D_8009BF68)
 *   1e90 lw    a1,0(v0)
 * i.e. the axis-A materialization is reproduced. sandbox score 9 -> 8, build_insns
 * 90 -> 93 (the dead sw s3,%lo(D_8009BF68) at 195c is NOT DCE'd — a store to a
 * game global cannot be proven dead across the intervening debug_printf/
 * motion_make_table calls, so it persists, +2 insns).
 *
 * WHY IT IS A CHEAT (no-new-park-categories, cheats-by-any-spelling):
 *  - No semantic purpose: stores an address into a game-state global that holds a
 *    motion table pointer, immediately overwritten; a human would never write it.
 *  - Dead-in-intent: exists ONLY to give the address pseudo a 2nd use so combine's
 *    multi-use retention (combine.c:1458 added_sets_2) keeps it materialized.
 *  - Justification references GCC internals (combine fold suppression), not logic.
 *
 * FORENSIC VALUE: proves axis-A's 3-insn form requires a 2nd use of &D_8009BF68 in
 * func_8007DC9C's OWN insn chain. combine_instructions runs per-function
 * (toplev.c:3004, inside rest_of_compilation), so a use of &D_8009BF68 in sibling
 * func_8007D3F8 is invisible to func_8007DC9C's combine pass -> matching the sibling
 * CANNOT close axis A. The "wait for sibling" frontier probe is mechanically
 * impossible; the only single-function reproduction is this coercion.
 */
