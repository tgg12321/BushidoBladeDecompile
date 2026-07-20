void InitHiraRmd_80047FBC(s32 arg0, s32 arg1, s16 arg2, s16 arg3)
{
    /* Frame: the 32-byte dead-vars padding (frame=80, vars=32) emerges
     * from the declared `s32 buf[8]` local — GCC 2.7.2 reserves frame for
     * declared locals regardless of later DCE. Sibling cluster
     * (AddTbpOfst_80047EE8, InitHiraRmd_800480C0, func_800481E8) shares
     * this idiom — see feedback_dead_vars_local_array_idiom.md.
     *
     * 100% pure C: no regfix, no inline asm, no register pins. Two
     * non-obvious structural levers reproduce target's exact register
     * allocation (both isolated via decomp-permuter whole-function search):
     *
     * 1. Prologue staging ($s0=arg0; $s4=$s0; $s0+=a1). A plain `base=p`
     *    copy lets copy-prop fold base==p==arg0 and emit `$s4=$a0` directly
     *    (1 insn short). Precomputing the efc first arg into `new_var`
     *    mid-loop shifts whole-function register allocation enough that GCC
     *    stages arg0 through $s0 first, then copies $s4=$s0.
     *
     * 2. Second pointer ($s0=$s4+v0). After the staging, GCC keeps arg0
     *    live in $a0 and emits `$s0=$a0+v0`. The dead `arg0 = 0;` (DCE'd to
     *    zero instructions, since arg0 is unused afterward) breaks GCC's
     *    $a0==arg0 value association, so this reference binds to $s4 (base)
     *    — matching target. Replaces 7 regfix rules incl. 2 lost_codegen
     *    `addu $sN,$sM,$zero` insert cheats. */
    s32 buf[8];
    u32 *p;
    u32 *base;
    s32 count;
    s32 new_var;
    p = (u32 *)arg0;
    base = p;
    arg0 = 0;
    p = (u32 *)((s32)p + (((s32)(arg1 << 16)) >> 14));
    p = (u32 *)((s32)base + (((*p) >> 2) << 2));
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
            new_var = (s32)base + (((u32)word >> 2) << 2);
            a3v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            v0v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            efc_buki_draw_zanzou(new_var,
                          (s32)a1v + sx_arg2,
                          (s32)a2v + sx_arg3,
                          (s32)a3v + sx_arg2,
                          (s32)v0v + sx_arg3);
        } while ((count--) != 0);
    }
    (void)buf;
}
