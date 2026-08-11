/* REJECTED (session 2 re-measure of session-1 K1) — the
 * [[defeat-combine-symbol-fold]] "give the displaced address its own local"
 * lever, re-run AFTER region B was closed, because the session-1 ledger's F3
 * explicitly flagged regions A and B as possibly coupled through $s0 liveness
 * and asked for exactly this re-measure.
 *
 *   s16 *eda = &D_80101EDA;
 *   s16 *edb = eda + 0x226;
 *   saved_first = eda[0];
 *   saved_44c   = edb[0];
 *   if (qf & 0x10) eda[0] = 0x32;
 *   if (q[3] & 0x20) edb[0] = 0x32;
 *   func_8003AFFC();
 *   eda[0] = saved_first;
 *   edb[0] = saved_44c;
 *
 * MEASURED: sandbox --disable all 6 -> 6, build_insns 188 -> 188, region A
 * bytes unchanged (all three sites still `lui` + `%lo(sym+0x44C)`).
 *
 * CONCLUSION: K1 is dead INDEPENDENTLY of region B — the coupling the session-1
 * ledger hypothesised does not exist for this lever, and the axis is now closed
 * twice over.  Do not re-probe it a third time.  Mechanistically the reason is
 * clear from cse.c: cse folds the constant into `edb`'s own definition first
 * (`(set (reg) (plus (reg) 1100))` -> `(set (reg) (const (plus sym 1100)))`),
 * so by the time the bare-REG MEM `(mem (reg edb))` is processed the base is
 * still a known constant and the constant address wins downstream anyway.
 */
