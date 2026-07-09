/* s21 PROBE C: v0 = v0 + v0; v0 = v0 + v0; on arg5 side.
   masked=2 INERT — addsi3 chain folded to SLL by cse/combine's shift-recognition
   (fold_rtx converts (plus x x) -> (ashift x 1)). Basin preserved bit-identical.
*/
    v0 = idx_1494[1];
    v0 = v0 + v0; v0 = v0 + v0;
    arg5 = *(s32 *)(v0 + (s32)tbl_125c);
