void func_800481E8(s32 arg0, s32 arg1)
{
    register s32 cached asm("$16");
    s32 saved;
    u32 *p;
    s32 count;
    /* INLINE_MOVE_ALIASING: pure-C alternatives failed.
     *   - technique=natural_chain: GCC's CSE collapses `cached = arg0; saved = cached`
     *     to `move $s2, $a0` directly, eliding the intermediate $s0 hold. Verified
     *     via dc.sh dump-text: idx 2 emits `addu $18, $4, $zero` (1 instruction
     *     short of target). Even cc1psx (real PsyQ GCC 2.7.2.SN.1) does the same.
     *   - technique=volatile_temp: `volatile s32 vol = arg0` produces sw+lw spill
     *     pattern (wrong shape: memory access not reg-reg addu chain).
     *   - technique=address_take: `s32 *p = &saved` introduces sp-relative load,
     *     wrong shape.
     *   - technique=decl_order: reordering decls doesn't affect collapsed prologue.
     *   - technique=for_loop_restructure: alternate loop shapes (for/while/goto)
     *     unchanged codegen.
     *
     * Per feedback_inline_move_aliasing.md, single-insn escape valve materializes
     * arg0 into $s0 via reg-reg move. After this, `saved = cached` (plain C var)
     * naturally chains $s2 = $s0 because $s0 holds the asm-opaque value GCC can't
     * fold. Then `cached = saved + offset` updates $s0 as the working pointer
     * (target's `addu $s0, $s0, $a1` shape).
     */
    __asm__ volatile("move %0, %1" : "=r"(cached) : "r"(arg0));
    saved = cached;
    cached = saved + (((s32)(arg1 << 16)) >> 14);
    p = (u32 *)cached;
    cached = saved + (((*p) >> 2) << 2);
    p = (u32 *)cached;
    count = *(p++);
    if (count != 0) {
        count--;
        do {
            u32 word;
            s16 a1v;
            s16 a2v;
            s16 a3v;
            u16 v0v;
            s32 a0_for_call;
            unsigned int new_var2;
            word = *p;
            p = (u32 *)(((s32)p) + 4);
            a1v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a2v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a3v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            v0v = *((u16 *)p);
            new_var2 = word >> 2;
            a0_for_call = saved + (new_var2 << 2);  /* compute early (target sched) */
            p = (u32 *)(((s32)p) + 2);  /* always advance (target delay slot) */
            if ((s32)a3v < 0x280) {
                v0v += 1;
            }
            func_800482C8(a0_for_call,
                          (s32)a1v,
                          (s32)a2v,
                          (s32)a3v,
                          (s32)(s16)v0v);
        } while ((count--) != 0);
    }
}
