/* REJECTED s2 (2026-08-21) — split out2b + staged re-init between loops.
 *
 * Spelling: split model (loop2 uses out2b) with the between-loops def staged
 * through the dying loop1 variable:
 *     out2 = (s32 *)(((u8 *)pa4) + 0x20);
 *     out2b = out2;
 * Intent: flow counts out2 (pseudo 86) refs 3->5 and livelen 42->50 (solver
 * set #4-adjacent); combine then 2-insn-merges copy+def into the single
 * target addiu (byte-neutral).
 *
 * MEASURED: sandbox 16, build_insns 132, but 86 stayed at 3 refs / 42.
 * MECHANISM (dump-proven, .jump vs .cse): cse1 NORMALIZES the pair by
 * swapping destinations when the copy's source dies at the copy:
 *   pre-cse : insn164 (86 = 77+32), insn167 (87 = 86)
 *   post-cse: insn164 (87 = 77+32), insn167 (86 = 87)
 * The swapped copy (86 = 87) is then a dead store and flow deletes it
 * UNCOUNTED. Both staging directions (A=expr;B=A and B=expr;A=B) converge
 * to the same normal form, so no direction of this staging can ever lift
 * reg_n_refs on the dying variable. Generalizes: staging +refs onto a
 * pseudo that DIES at the copy is structurally impossible in GCC 2.7.2.
 */
