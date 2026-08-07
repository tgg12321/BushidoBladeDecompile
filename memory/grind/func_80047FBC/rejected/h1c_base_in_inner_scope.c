/* REJECTED s2: base declared in inner scope, after p is derived from arg0.
 * Sandbox score 8, build 63 insns. Wrapping base's scope tightly makes GCC
 * allocate it differently: emits `move s0,a0; addu s0,s0,a1` (no s4 setup at
 * all in the prologue path), then insn #18 becomes something like using $a0
 * still, and the entire loop-body allocation shifts. Net: strictly worse
 * than the flat H1b form. Killed the hypothesis that "shorter base lifetime
 * = better allocation." */
void InitHiraRmd_80047FBC(s32 arg0, s32 arg1, s16 arg2, s16 arg3)
{
    s32 buf[8];
    u32 *p;
    s32 count;
    s32 new_var;
    p = (u32 *)arg0;
    p = (u32 *)((s32)p + (((s32)(arg1 << 16)) >> 14));
    {
        u32 *base = (u32 *)arg0;
        p = (u32 *)((s32)base + (((*p) >> 2) << 2));
        count = *(p++);
        if (count != 0) {
            s32 sx_arg2 = arg2;
            s32 sx_arg3 = arg3;
            count--;
            do {
                u32 word;
                s16 a1v; s16 a2v; s16 a3v; s16 v0v;
                word = *p; p = (u32*)((s32)p + 4);
                a1v = (s16)*(u16*)p; p = (u32*)((s32)p + 2);
                a2v = (s16)*(u16*)p; p = (u32*)((s32)p + 2);
                new_var = (s32)base + (((u32)word >> 2) << 2);
                a3v = (s16)*(u16*)p; p = (u32*)((s32)p + 2);
                v0v = (s16)*(u16*)p; p = (u32*)((s32)p + 2);
                efc_buki_draw_zanzou(new_var,
                    (s32)a1v + sx_arg2, (s32)a2v + sx_arg3,
                    (s32)a3v + sx_arg2, (s32)v0v + sx_arg3);
            } while ((count--) != 0);
        }
    }
    (void)buf;
}
