/* REJECTED — permuter s4 floor10-frame campaign best find (score 202, NOT 0).
 *
 * The whole-function permuter campaign (9451 iters, fresh seed, --stack-diffs
 * so frame offsets score) on the floor-10 clean chassis lowered the weighted
 * score 266 -> 242 -> 202 by ONE lever only: inserting a volatile dead local
 * (`volatile long pad;` then widened to `volatile unsigned long long pad;`) to
 * reserve phantom frame bytes. It is a FORBIDDEN volatile-coercion / dead-vars
 * frame-reservation cheat (inline-asm-policy expanded catalog 2026-05-31;
 * dead-vars-local-array). It reserves only 8 bytes, so it plateaus at 202 and
 * never reaches the 32-byte target reservation; the next novel emitted was a
 * WORSE 207. The permuter found NO legitimate pure-C form that grows the frame.
 *
 * This CORROBORATES s3's structural finding (phantom-frame grid DEAD, v08
 * decisive control) from an independent random-search angle: the 32-byte
 * phantom frame is not reachable by any non-cheat C shape in this body.
 */
void AddTbpOfst_80047EE8(s32 arg0, s32 arg1)
{
    u32 *p;
    s32 saved;
    s16 new_var;
    s32 count;
    u32 v_off;
    volatile unsigned long long pad; /* CHEAT — volatile dead-frame reservation */
    unsigned int new_var2;
    p = (u32 *) arg0;
    saved = (s32) p;
    arg0 = 0; /* FAKE */
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
