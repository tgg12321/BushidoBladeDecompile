/* REJECTED (s6-rederive, 2026-08-11) — the array-ISH spellings that still FOLD.
 *
 * Session 6 found that declaring the object as an incomplete array
 * (`extern s16 D_80101EDA[];`) and writing plain array references closes
 * region A (candidate.c, sandbox 0).  These four neighbours of that form were
 * measured in the same mini-TU sweep (tmp/grind/func_8003B9D0/s6/, exact build
 * flags, checked for `1100($reg)` vs `D_80101EDA+1100`) and ALL still fold —
 * i.e. the lever is the DECLARED TYPE OF THE OBJECT, not "anything that looks
 * like an array".  Do not re-propose any of them.
 *
 *   mini_07_arrptr    — array declaration, but the accesses go through a
 *                       pointer local:            FOLDS (all 3 sites)
 *       extern s16 D_80101EDA[];
 *       s16 *eda = D_80101EDA;  ... eda[0] ... eda[0x226] ...
 *
 *   mini_10_ptrtoarr  — scalar declaration + pointer-to-array cast:  FOLDS
 *       s16 (*eda)[0x226] = (s16 (*)[0x226])&D_80101EDA;
 *       ... eda[0][0] ... eda[1][0] ...
 *
 *   mini_11_structarr — scalar declaration + array-of-struct index:  FOLDS
 *       typedef struct { s16 f; u8 pad[1098]; } Slot;
 *       Slot *s = (Slot *)&D_80101EDA;  ... s[0].f ... s[1].f ...
 *
 *   mini_12_castarr   — scalar declaration + cast to pointer-to-incomplete-
 *                       array, dereferenced:      DIFFERENT, still wrong
 *       #define EDA (*(s16 (*)[])&D_80101EDA)
 *       ... EDA[0] ... EDA[0x226] ...
 *       (emits `la $16,D_80101EDA+1100` + `0($16)` for the FAR sites and a
 *        constant address for the NEAR ones — the mirror image of target.)
 *
 * Corollary for future sessions: any form that reaches the object through a
 * POINTER VALUE (local, cast, struct pointer) re-introduces a base pseudo whose
 * cse quantity carries `(symbol_ref)` as its constant, and find_best_addr folds
 * the displaced address again.  The array reference is the only spelling that
 * never creates that pseudo.
 */
