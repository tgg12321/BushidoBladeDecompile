/* REJECTED — s5 (2026-07-21, permuter). Best form from a fresh-seed
 * whole-function permuter campaign run on the STRUCTURALLY DIFFERENT
 * no-FAKE floor-11 chassis (distinct RA basin from s4's floor-10 FAKE
 * chassis; base_score 5200 vs s4's 266). 17057 iterations, --stack-diffs.
 *
 * The campaign descended 5200 -> 207 and PLATEAUED FLAT at 207 from iter
 * ~12753 through 17057 (thousands of iters, zero novel improvement), then
 * the campaign was harvested + stopped. Never reached 0.
 *
 * The ONLY frame-growth lever the permuter surfaced in this distinct basin
 * is the SAME forbidden family s4 found: a volatile dead-frame local
 * (`volatile unsigned int pad;`) whose sole effect is to make GCC reserve
 * ~8 phantom frame bytes. This is a volatile-coercion / dead-vars frame
 * cheat (inline-asm-policy expanded catalog 2026-05-31; no-new-park-
 * categories "cheats by any spelling"): no semantic purpose, dead in the
 * emitted output, named `pad`. It reserves only ~8 bytes (not the needed
 * 32), so it plateaus at 207 and cannot close.
 *
 * Independent corroboration of s3 (structural 9-variant grid) and s4
 * (permuter on the floor-10 chassis): across TWO structurally-distinct
 * basins and ~26500 combined permuter iterations, the 32-byte phantom
 * frame is reachable ONLY via forbidden dead/volatile-local cheats,
 * never a legitimate pure-C frame-growth form. The permuter modality is
 * now measured DEAD on this function across both chassis basins.
 */
void AddTbpOfst_80047EE8(s32 arg0, s32 arg1)
{
    u32 *p;
    volatile unsigned int pad;   /* CHEAT — dead volatile frame local, forbidden */
    s32 saved;
    s16 new_var;
    s32 count;
    u32 v_off;
    unsigned int new_var2;
    p = (u32 *) arg0;
    saved = (s32) p;
    p = (u32 *) (((s32) p) + (((s32) (arg1 << 16)) >> 14));
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
