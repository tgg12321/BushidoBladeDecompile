/* s21 PROBE A: v0 = v0 * 4 on arg5 side (route through expand_mult).
   masked=2 INERT — expand_mult with outer SET target=v0-pseudo folds identically
   to `v0 <<= 2` in-place. No fresh single-set dest on arg5 side; no LAUNCH shift.
*/
    v0 = idx_1494[1];
    v0 = v0 * 4;
    arg5 = *(s32 *)(v0 + (s32)tbl_125c);
