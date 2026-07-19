/* s11 rejected: combined declarator for v1 and s4 on one `s32` statement.
 * Form: `s32 v1 = a0 << 4, s4 = *(s32*)((u8*)&D_800EED14 + v1);` in place
 * of the two separate `s32 v1 = ...;` / `s32 s4 = ...;` decls.
 * Measured NEUTRAL (score=2, 83/83). GCC 2.7.2 lowers a
 * comma-separated declarator list to the same tree_LUID sequence as two
 * separate single-declarator statements -- v1's assignment LUID precedes
 * s4's identically to the baseline, no sched2 tie movement, no CSE
 * boundary change.  Confirms declarator-list packing is a free axis and
 * cannot shift the sll/move16 tie.
 */
void func_80045294(s32 a0, s32 a1) {
    s32 sum = 0;
    s32 v1 = a0 << 4, s4 = *(s32 *)((u8 *)&D_800EED14 + v1);
    s32 i = a0;
    s32 count = D_800A33AC;
    s32 s5 = s4 + a1;
    /* ... unchanged body ... */
}
