/* s32 PERM_GENERAL fresh-seed novel find (output-40-10, weighted=40 = g3 basin).
 * Mutation: hoist &D_800F19C0 into fn-scope `void **new_var2;` (added as extra
 * decl), assigned in prologue then used as `pp = (void**)new_var2;`, AND
 * pass `status` (existing fn-scope local) as debug_printf's final arg after
 * `status = arg5;` set. Composite mutation: pp-source-indirection + arg5
 * output-alias. Score class weighted=40 => masked=6 (g3 basin) per s22/s23
 * correlation. Novel spelling but same class — g3 family, arg5-chain routed
 * through fn-scope carrier that alters fn-scope pseudo priority arithmetic. */
void **new_var2;
new_var2 = &D_800F19C0;
{
    s32 arg5;
    s32 t0;
    void **pp;
    t0 = idx_1494[0];
    v0 = idx_1494[1];
    pp = (void **)new_var2;
    t0 *= 4;
    t0 = (s32)((u8 *)tbl_125c + t0);
    v0 <<= 2;
    arg5 = *(s32 *)(v0 + (s32)tbl_125c);
    status = arg5; /* PERM alias-tail: pass status not arg5 */
    debug_printf(&D_800161C8, *pp, D_800A11DC[D_800A11D5], *(s32 *)t0, status);
}
