void func_80047EE8(s32 arg0, s32 arg1)
{
    register s32 cached asm("$16");
    register s32 saved asm("$18");
    s32 unused_slack[8];
    u32 *p;
    s16 new_var;
    s32 count;
    u32 v_off;
    unsigned int new_var2;
    (void)unused_slack;
    cached = arg0;
    /* INLINE_MOVE_ALIASING: pure-C alternatives failed.
     *   - technique=plain_copy: `saved = cached;` got DCE'd or rematerialized
     *     to $a0 since GCC tracks values not vars; lost the addu instruction
     *   - technique=saved_eq_arg0: `saved = arg0;` same DCE issue, lost insn
     *   - technique=decl_init: `register s32 saved asm("$18") = arg0;`
     *     same outcome, GCC treats as alias of $a0
     *   - technique=volatile_saved: `volatile s32 saved = arg0;` introduced
     *     sw+lw spill (wrong shape, frame=80 with vars=40)
     * Per feedback_inline_move_aliasing.md, single-insn escape valve to
     * keep the redundant addu $s2,$s0,$0 in the emission.
     */
    __asm__ volatile("move %0, %1" : "=r"(saved) : "r"(cached));
    arg1 <<= 16;
    arg1 >>= 14;
    cached = cached + arg1;
    v_off = *(u32 *)cached;
    cached = saved + ((v_off >> 2) << 2);
    p = (u32 *) cached;
    count = *(p++);
    if (count != 0)
    {
        count--;
        do
        {
            u32 word;
            s16 a1v;
            s16 a2v;
            s16 a3v;
            s16 v0v;
            word = *p;
            p = (u32 *) (((s32) p) + 4);
            a1v = (s16) (*((u16 *) p));
            p = (u32 *) (((s32) p) + 2);
            a2v = (s16) (*((u16 *) p));
            p = (u32 *) (((s32) p) + 2);
            a3v = (s16) (*((u16 *) p));
            p = (u32 *) (((s32) p) + 2);
            new_var = a1v;
            new_var2 = word >> 2;
            v0v = (s16) (*((u16 *) p));
            p = (u32 *) (((s32) p) + 2);
            func_800482C8(saved + (new_var2 << 2), new_var, a2v, a3v, v0v);
        }
        while ((count--) != 0);
    }
}
