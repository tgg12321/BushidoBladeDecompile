/* REJECTED (s2, 2026-07-13) -- the judge's prescribed "clean fix".
 *
 *   include/code6cac.h:95:  extern s16 D_800A36A4;  ->  extern u16 D_800A36A4;
 *   src/code6cac_b.c:3317:  switch ((s16)(D_800A36A4 - 2)) { ... }   // no cast
 *
 * DISPROVEN on two independent axes -- do NOT re-propose:
 *
 * 1. BINARY EVIDENCE (s1 census over asm/funcs). The global is read with `lh`
 *    in ~13 functions (camera_SetMatrix_8001DA8C, cpu_side_move_dir,
 *    func_80021D10, func_80021DB0, func_80022224, func_80022408,
 *    func_80032C50 x2, func_80058580, mario_test_Exec, mk_leaf_newpos,
 *    se_data_set x3) and with `lhu` in only 3 (func_80033498, func_8003AE5C,
 *    se_data_set's 4th read). A u16 declaration makes every PLAIN read an lhu,
 *    so the retype flips all 13 lh sites and breaks already-COMPLETED-C
 *    functions. The s16 declaration is pinned by the bytes.
 *
 * 2. USE-SITE SEMANTICS (s2 census over src/, all 19 sites). Nothing requires
 *    unsigned semantics: every read is a small non-negative stage index
 *    (multiplied by 0x18/0xC/3, used as an array subscript) and the stores are
 *    small constants -- and src/code6cac_b.c:4019 stores through an EXPLICIT
 *    `D_800A36A4 = (s16)v1;` signed cast. Under
 *    [[header-type-correction-from-use-sites]] the retype fails prong (a):
 *    no use site exhibits unsigned-specific semantics that is dead under s16.
 *    It also fails prong (c)/(d) -- the fix would have to ADD `(s16)` casts at
 *    the 13 lh sites rather than eliminate casts.
 *
 * Consequence: the lhu at this site cannot be obtained by fixing the
 * declaration. It requires a use-site unsigned view -- which is the exact
 * construct the judge banned. See the s2 ruling_question.
 */
