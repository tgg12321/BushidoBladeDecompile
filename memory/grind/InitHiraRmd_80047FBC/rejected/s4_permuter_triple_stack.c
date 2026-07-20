/* s4 permuter closing form — sandbox score 0 achieved but REJECTED per
 * [[no-new-park-categories]] cheat-by-any-spelling vetting.
 *
 * Fresh-seed campaign (label=s4_chassis1, -j 6, --stop-on-zero) converged
 * to score 0 in ~15 min (single output-0-1). Byte-neutral vs target.
 *
 * The form stacks THREE forbidden-family constructs, each with no
 * semantic purpose (pure RA steering):
 *
 *   1. `s32 buf[8]; (void)buf;` — plain unused-array cheat (inherited from
 *      s1-s3 candidate; NOT the WRITTEN-never-read carve-out because C
 *      source has zero explicit stores into buf[]; target's sw v0,0x10(sp)
 *      is GCC-allocated from the discarded efc_buki_draw_zanzou return, not
 *      a source-level store).
 *   2. `int new_var3 = 16;` used as the shift count in `arg1 << new_var3`.
 *      Constant-holder scalar; natural spelling is `arg1 << 16`. No human
 *      programmer would extract a compile-time constant into a named local.
 *      Under [[named-local-fake-exception]] 2026-07-01 (constant-holder
 *      scalar carve-out) individually reviewable, but stacked here.
 *   3. `s32 new_var2; ...; new_var2 = sx_arg2;` used as a1v+new_var2 and
 *      a3v+new_var2 (2 of 4 call arg summands). Same-value local alias of
 *      a live local, mid-loop, with no observable behavior. Not covered by
 *      [[staged-value-reused-variable]] (that carve-out is for staging
 *      through an EXISTING currently-dead local; sx_arg2 is live here and
 *      new_var2 is a fresh local).
 *
 * All three fail the vetting checklist ([[no-new-park-categories]]):
 * - No semantic purpose (each construct is DCE'd or arithmetic no-op)
 * - No human would write it
 * - Justification purely references RA internals ("reg_n_refs lift",
 *   "priority tiebreaker")
 * - "Necessary only because permuter said so" — literally the smell test.
 *
 * Sandbox measurement (post-apply, before revert): score=0, 65/65 insns.
 * Bytes proven. Cheat-by-spelling classification confirmed by inspection.
 *
 * Killed hypothesis: "PERM_* directed permuter over base_addr/p init chain
 * + type variants can find a legit closing form for this residual." The
 * permuter's search space includes constant-hoisting and value-aliasing
 * mutations; when it converges, it converges to cheat forms because the
 * legitimate lever space here is genuinely narrow. Directed permuter alone
 * cannot resolve this residual.
 */
void InitHiraRmd_80047FBC(s32 arg0, s32 arg1, s16 arg2, s16 arg3)
{
    s32 buf[8];
    u32 *p;
    int new_var3;
    s32 base_addr;
    s32 count;
    s32 new_var2;
    s32 new_var;
    base_addr = arg0;
    new_var3 = 16;
    p = (u32 *)arg0;
    p = (u32 *)((s32)p + (((s32)(arg1 << new_var3)) >> 14));
    p = (u32 *)(base_addr + (((*p) >> 2) << 2));
    count = *(p++);
    if (count != 0) {
        s32 sx_arg2;
        s32 sx_arg3;
        count--;
        sx_arg2 = arg2;
        sx_arg3 = arg3;
        do {
            u32 word;
            s16 a1v;
            s16 a2v;
            s16 a3v;
            s16 v0v;
            word = *p;
            p = (u32 *)(((s32)p) + 4);
            a1v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a2v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            new_var = base_addr + (((u32)word >> 2) << 2);
            new_var2 = sx_arg2;
            a3v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            v0v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            efc_buki_draw_zanzou(new_var,
                          (s32)a1v + new_var2,
                          (s32)a2v + sx_arg3,
                          (s32)a3v + new_var2,
                          (s32)v0v + sx_arg3);
        } while ((count--) != 0);
    }
    (void)buf;
}
