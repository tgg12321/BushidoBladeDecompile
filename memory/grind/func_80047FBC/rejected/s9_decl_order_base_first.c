/* s9 rederive-modality: declaration order swap (base declared BEFORE p)
 * with unchanged init chain (`p = arg0; base = p;`).
 *
 * Hypothesis: cse2's canon_reg canonicalization might pick the lower-LUID
 * regno as class root; declaring base first might give reg 79 (base) a
 * lower luid than reg 78 (p), inverting the fold direction so insn #36's
 * substitution goes reg 72→reg 79 (= $s4) instead of reg 78→reg 72 (= $a0).
 *
 * Result: sandbox --disable all = 1 (unchanged). GCC assigns pseudo-regnos
 * by first-USE order (LUID), NOT declaration order. Declaring `base` before
 * `p` does not change reg 78/79 assignment because `p` is used first
 * (`p = (u32*)arg0` precedes `base = p`). cse2 still forms the same
 * {reg 72, 78, 79} equivalence class with reg 72 as root.
 *
 * KILLED — declaration order is inert. Same finding as s3's init-order
 * swap, extended to pure declaration-order without init-order change.
 */
void InitHiraRmd_80047FBC(s32 arg0, s32 arg1, s16 arg2, s16 arg3)
{
    s32 buf[8];
    u32 *base;   /* declared FIRST */
    u32 *p;
    s32 count;
    s32 new_var;
    p = (u32 *)arg0;
    base = p;
    arg0 = 0;
    /* ...rest unchanged from s3 candidate... */
    (void)base; (void)p; (void)buf; (void)count; (void)new_var;
}
