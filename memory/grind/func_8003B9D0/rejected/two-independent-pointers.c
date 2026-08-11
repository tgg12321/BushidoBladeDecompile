/* REJECTED (session 2, structural) — give the far halfword its OWN pointer at
 * its OWN splat symbol, so that BOTH pointers are used at displacement 0 and
 * neither access can be hit by cse's displaced-address symbol fold.
 *
 *   s16 *eda = &D_80101EDA;
 *   s16 *edb = &D_80102326;          <-- 0x80101EDA + 0x44C, same halfword
 *   saved_first = eda[0];
 *   saved_44c   = edb[0];
 *   if (qf & 0x10) eda[0] = 0x32;
 *   if (q[3] & 0x20) edb[0] = 0x32;
 *   func_8003AFFC();
 *   eda[0] = saved_first;
 *   edb[0] = saved_44c;
 *
 * MEASURED: sandbox --disable all 6 -> 17 (much WORSE); build_insns 188 -> 189.
 *
 * WHY IT IS DEAD: the second `la` (lui+addiu materialisation of D_80102326) is
 * two always-executed instructions, and holding two live symbol-address
 * pointers across the func_8003AFFC() call forces a second callee-saved
 * register, which perturbs the whole surrounding allocation.  Target holds
 * exactly ONE base register ($s0) and reaches the far halfword with a
 * register+displacement `1100($s0)`, so any form that materialises a second
 * base is structurally further from target, not closer.  The offset-0-for-both
 * trick therefore does not generalise from K2's "the fold follows the non-zero
 * displacement" finding: removing the displacement removes the shared base too.
 */
