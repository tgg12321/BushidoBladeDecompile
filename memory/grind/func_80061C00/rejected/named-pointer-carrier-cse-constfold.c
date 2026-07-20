/* s2 variant F — REJECTED, score 31 (+4 insns).
 * t = sp20[1]-repack + (s32)(D_800F1164 + 2) pointer carrier, derefs via
 * ((u8 *)t)[k], goal: unboost the sp20[1] load symmetrically with val's.
 * cc1 cse CONSTANT-PROPAGATED the symbol value through the named variable:
 * every deref folded to absolute %hi/%lo(D_800F1164+3) addressing (lui + lbu
 * off(at/v0)), destroying the shared base-register shape and adding insns.
 * The anonymous compound spelling (D_800F1164 + 2)[k] is LOAD-BEARING: it is
 * what makes cse materialize ONE base pseudo and index it.
 * Kill lesson: never name a constant-address pointer into a C variable in
 * this function; cse folds it away.
 */
