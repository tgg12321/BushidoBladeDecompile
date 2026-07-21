/* s2 REJECTED — pure spelling V2: decl-initializers + const-qualified saved.
 * u32 *p = (u32*)arg0; const s32 saved = (s32)p; on the floor-10 chassis
 * (single-walker + live first precompute), NO FAKE store.
 * sandbox --disable all = 11 (FAKE form = 10). const is discarded at RTL by
 * cse2 canonical-register substitution — same mechanism sibling s8 proved
 * (const-qualified form byte-identical to non-const). DEAD.
 */
void AddTbpOfst_80047EE8(s32 arg0, s32 arg1)
{
    u32 *p = (u32 *) arg0;
    const s32 saved = (s32) p;
    s16 new_var;
    s32 count;
    u32 v_off;
    unsigned int new_var2;
    p = (u32 *) ((s32) p + (((s32) (arg1 << 16)) >> 14));
    v_off = *p;
    p = (u32 *) (saved + ((v_off >> 2) << 2));
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
            s32 first;
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
            first = saved + (new_var2 << 2);
            v0v = (s16) (*((u16 *) p));
            p = (u32 *) (((s32) p) + 2);
            efc_buki_draw_zanzou(first, new_var, a2v, a3v, v0v);
        }
        while ((count--) != 0);
    }
}
