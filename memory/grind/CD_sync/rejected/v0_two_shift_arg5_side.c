/* s21 PROBE B: v0 <<= 1; v0 <<= 1; on arg5 side.
   masked=2 INERT — two-shift folded to single SLL by cse/combine; extra SET on
   p_v0 does not shift qty priority; RTL identical to `v0 <<= 2`.
*/
    v0 = idx_1494[1];
    v0 <<= 1; v0 <<= 1;
    arg5 = *(s32 *)(v0 + (s32)tbl_125c);
