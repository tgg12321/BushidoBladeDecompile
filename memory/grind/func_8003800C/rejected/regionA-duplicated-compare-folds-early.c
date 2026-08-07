/* REJECTED (s14) — duplicated sum==chk compare into the continue arm.
 * Frontier #1 manual construction: give sum an 11th weighted reg_n_refs
 * via a duplicate compare (duplicated-statement-into-arms family) WITHOUT
 * bracketing sum=0's def, so sum=0 stays low-LUID / emitted first.
 *
 * FORM (plain sum=0, no do-while0):
 *     chk = *(s32*)((u8*)chkptr + 0x6C);
 *     if (sum == chk) { break; }
 *     if (sum != chk) { chkptr++; i++; offset += 0x24; }   // duplicate copy
 *
 * MEASURED: sandbox --disable all = 4, build_insns = 79 (BYTE-NEUTRAL —
 * the `sum != chk` compare was fully eliminated: it is the provable
 * complement of the break test, so jump1/cse folds it BEFORE flow.c runs
 * its reg_n_refs count). Result == plain sum=0 (inner a0<->a1 swap,
 * sum=$a1). The duplicate did NOT lift sum's ref count.
 *
 * WHY STRUCTURALLY INAPPLICABLE: sum is DEAD after its single compare
 * (target asm: exactly one `beq $a0,$v0`; no second sum use). The outer
 * loop's only split at sum-live scope is the break-vs-continue diamond,
 * whose two arms are COMPLEMENTARY — any compare placed in the continue
 * arm is the negation of the break test and folds at jump1 (before flow),
 * so no extra reg_n_refs reaches allocation. Unlike motion_SetMotion
 * (duplicated a REAL assignment across NON-complementary switch cases that
 * survive to jump2), damage has no multi-way split with a live-sum real
 * statement to duplicate. Also a cheat by spelling (dead re-compare, no
 * semantic purpose) even if it had worked.
 */
