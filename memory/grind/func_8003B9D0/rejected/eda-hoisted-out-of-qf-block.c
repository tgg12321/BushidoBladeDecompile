/* REJECTED (session 2) — hoisting the `eda` pointer definition OUT of the
 * `qf & 0x30` block, in the hope that the block's entry branch would put the
 * definition in a PRIOR cse basic block (so cse would no longer know the base
 * pseudo's constant value and could not fold the +0x44C displacement).
 *
 * MEASURED: sandbox --disable all 6 -> 10 (WORSE); build_insns 188 -> 187.
 * WHY IT IS DEAD: the `if (qf & 0x30)` branch is an AROUND branch that
 * cse_end_of_basic_block extends across (tools/gcc-2.7.2/cse.c:8150-8183), so
 * no block boundary is created and the base pseudo's constant is still known;
 * meanwhile the lui/addiu materialisation moved OUT of the if-body, which
 * costs an extra always-executed pair vs target's inside-the-body placement.
 *
 *   ...
 *   u8 qf = q[3];
 *   s16 *eda = &D_80101EDA;      <-- hoisted here
 *   if (qf & 0x30) {
 *       saved_first = eda[0];
 *       saved_44c = eda[0x226];
 *       if (qf & 0x10) eda[0] = 0x32;
 *       if (q[3] & 0x20) eda[0x226] = 0x32;
 *       func_8003AFFC();
 *       eda[0] = saved_first;
 *       eda[0x226] = saved_44c;
 *   }
 *
 * Corollary for the next session: a cse block boundary between the base
 * definition and its first displaced use is still the right idea, but it must
 * be created by a construct that DEFEATS the extension (a BARRIER + join
 * label, i.e. an if/ELSE) — not by an ordinary `if` whose body the extension
 * simply walks through.
 */
