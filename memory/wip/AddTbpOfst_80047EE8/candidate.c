/* AddTbpOfst_80047EE8 (text1b.c) — CLEAN pure-C candidate (cheat_count 0).
 * sandbox --disable all = 15. The LOOP BODY is byte-identical to target; ALL
 * diffs are (a) frame size 0x28 vs target's 0x48 (a 32-byte UNUSED-frame gap)
 * + (b) the prologue scheduling cascade that follows the smaller frame + (c)
 * one redundant `move s2,s0` the committed cheat preserves via an asm barrier.
 *
 * The COMMITTED src/text1b.c form (NOT this) carries 3 cheats to match:
 *   - `s32 unused_slack[8]; (void)unused_slack;` (dead-vars-local-array,
 *     FORBIDDEN 2026-05-31, find_unused_local_arrays) — fabricates the 32 bytes
 *   - register pins on $16/$18
 *   - INLINE_MOVE_ALIASING `__asm__("move %0,%1")` barrier
 *
 * There is NO legitimate pure-C lever that produces 32 bytes of unused frame.
 * This is a PROJECT-POLICY decision, not a single-function grind. See notes.md. */
void AddTbpOfst_80047EE8(s32 arg0, s32 arg1)
{
    s32 cached;
    s32 saved;
    u32 *p;
    s16 new_var;
    s32 count;
    u32 v_off;
    unsigned int new_var2;
    cached = arg0;
    saved = cached;
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
            efc_buki_draw_zanzou(saved + (new_var2 << 2), new_var, a2v, a3v, v0v);
        }
        while ((count--) != 0);
    }
}
