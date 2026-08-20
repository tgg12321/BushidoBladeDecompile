/* REJECTED SEED (not a candidate form) — func_800481E8, s4-rerun 2026-08-20.
 * WHY IT IS DEAD: this is the DIRECTED HYBRID permuter seed (PERM_GENERAL over the
 * loop-local declaration set and over each halfword stream read, PERM_LINESWAP over
 * the whole loop body) that spans BOTH previously-campaigned chassis at once.
 * Base score 266 (frame-only diffs, 56/56 insns). A 46,124-iteration / 28-minute
 * 8-job campaign over this seed produced ZERO finds — the directive space contains
 * no mutation that changes frame size, so it cannot reach the vars=32 delta.
 * Do not re-seed this space. Campaign telemetry label: directed-hybrid-permgeneral.
 */
typedef signed char s8; typedef unsigned char u8;
typedef short s16; typedef unsigned short u16;
typedef int s32; typedef unsigned int u32;
extern void func_800482C8();
void func_800481E8(s32 arg0, s32 arg1)
{
    u32 *p;
    u32 *base;
    s32 count;
    s32 a0_for_call;
    p = (u32 *)arg0;
    base = p;
    arg0 = 0;
    p = (u32 *)((s32)p + (((s32)(arg1 << 16)) >> 14));
    p = (u32 *)((s32)base + (((*p) >> 2) << 2));
    count = *(p++);
    if (count != 0) {
        count--;
        do {
            u32 word;
            PERM_GENERAL(s16 a1v; s16 a2v; s16 a3v; u16 v0v;,s32 a1v; s32 a2v; s32 a3v; s32 v0v;)
            unsigned int new_var2;
            PERM_LINESWAP(
            word = *p;
            p = (u32 *)(((s32)p) + 4);
            a1v = PERM_GENERAL((s16)(*((u16 *)p)),*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a2v = PERM_GENERAL((s16)(*((u16 *)p)),*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a3v = PERM_GENERAL((s16)(*((u16 *)p)),*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            v0v = *((u16 *)p);
            new_var2 = word >> 2;
            a0_for_call = (s32)base + (new_var2 << 2);
            p = (u32 *)(((s32)p) + 2);
            )
            if ((s32)(s16)a3v < 0x280) {
                v0v += 1;
            }
            func_800482C8(a0_for_call,
                          (s32)(s16)a1v,
                          (s32)(s16)a2v,
                          (s32)(s16)a3v,
                          (s32)(s16)v0v);
        } while ((count--) != 0);
    }
}
