/* REJECTED (s18, forensics, 2026-09-03) — NOT a proposal; MECHANISM EVIDENCE ONLY.
 *
 * WHY THESE ARE DEAD AS SUBMISSIONS: every one of the five bodies below reaches the
 * target terminator-epilogue arrangement DISP8 | DISP4 | LOSUM0 exactly (measured, 27
 * insns in the s16struct chassis, tmp/grind/func_80062020/s18/results18.txt) — and every
 * one of them buys its second value-consumption with DEAD CODE: a dead scalar local, a
 * dead re-store into the function's existing local `t`, or an empty-bodied `if`.  The
 * empty-bodied `if (cond) { }` dead-read is in the forbidden-family catalog verbatim; the
 * others are strictly more coercive than the plain ascending chain they are competing
 * with, and none of them should ever be submitted for this function under any family
 * label.  They are banked because of what they PROVE, not because they are candidates.
 *
 * WHAT THEY PROVE.  Until s18, every form ever measured to reach the target arrangement
 * was a chained assignment, so the ledger could not distinguish "the chain is an authored
 * address-materialization trick" (the assertion five layer-1 FAILs rest on) from "the two
 * addressing forms are a compiler law the author cannot select".  These five bodies are
 * syntactically and semantically unrelated to a chain and to each other, and they all land
 * on the identical arrangement — while two *chain* forms (a cast-wrapped chain, and a chain
 * whose +0 store is written first) do NOT reach it.  The dumps then name the two passes:
 * RTL EXPAND chooses DISP vs LOSUM per store from store_field's want_value
 * (tools/gcc-2.7.2/expr.c:3453-3464, visible in the .rtl dump before any optimiser runs),
 * and COMBINE re-folds any DISP whose address pseudo has a single use back into the symbol
 * (tools/gcc-2.7.2/combine.c:1458, `added_sets_2 = ! dead_or_set_p (i3, i2dest)`).
 * Full write-up: tmp/grind/func_80062020/s18/forensics_s18.md.
 *
 * Declaration used for all five (the banked s15 aggregate merge, include/game.h):
 *     typedef struct { s32 unk0; s32 unk4; s32 unk8; } Unk800F1198Record;
 *     extern Unk800F1198Record D_800F1198[];
 * Only the epilogue after `end:` varies; the copy loop is the banked body's, unchanged.
 */

/* --- N01: OR into a dead scalar local ------------------------------ REACHES (dead z) */
/*  s32 z;                                                                              */
/*  end: z = (D_800F1198[i].unk8 = 0) | (D_800F1198[i].unk4 = 0);                       */
/*       D_800F1198[i].unk0 = 0;                                                        */

/* --- N02: same, consumed into the function's already-dead local t --- REACHES (dead t) */
/*  end: t = (D_800F1198[i].unk8 = 0) | (D_800F1198[i].unk4 = 0);                       */
/*       D_800F1198[i].unk0 = 0;                                                        */

/* --- N03: two sequential stagings into t, no operator -------------- REACHES (dead t) */
/*  end: t = (D_800F1198[i].unk8 = 0);                                                  */
/*       t = (D_800F1198[i].unk4 = 0);                                                  */
/*       D_800F1198[i].unk0 = 0;                                                        */

/* --- N05: empty-bodied comparison ------- REACHES (forbidden family: empty-if dead-read) */
/*  end: if ((D_800F1198[i].unk8 = 0) != (D_800F1198[i].unk4 = 0)) { }                  */
/*       D_800F1198[i].unk0 = 0;                                                        */

/* --- N07: additive identity into a dead local ---------------------- REACHES (dead z) */
/*  s32 z;                                                                              */
/*  end: z = (D_800F1198[i].unk8 = 0) + (D_800F1198[i].unk4 = 0) + 0;                   */
/*       D_800F1198[i].unk0 = 0;                                                        */

/* --- and the informative MISSES, same chassis ---------------------------------------- */
/*  N04  D_800F1198[i].unk0 = ((D_800F1198[i].unk8 = 0), (D_800F1198[i].unk4 = 0));      */
/*         -> LOSUM8 | LOSUM4 | LOSUM0   (comma discards the +8 value)                   */
/*  N06  D_800F1198[i].unk0 = (s32)(D_800F1198[i].unk4 = (s32)(D_800F1198[i].unk8 = 0)); */
/*         -> LOSUM8 | LOSUM4 | LOSUM0   (the cast breaks consumption at expand)         */
/*  N08  z = (D_800F1198[i].unk8 = 0); unk4 = 0; unk0 = 0;                               */
/*         -> DISP8 at expand, re-folded to LOSUM8 by COMBINE (single use)               */
/*  N09  D_800F1198[i].unk0 = 0; D_800F1198[i].unk4 = D_800F1198[i].unk8 = 0;            */
/*         -> DISP8 at expand, re-folded to LOSUM8 by COMBINE; +0 no longer last         */
/*  N10  unk8 = 0; unk4 = 0; z = (D_800F1198[i].unk0 = 0);                               */
/*         -> LOSUM8 | LOSUM4 | LOSUM0   (the wrong store is the consumed one)           */
