/* REJECTED (grind session 2, structural) — hoisting the rect[1] read above the
 * D_8009BF28 store.
 *
 * WHY IT IS DEAD: it always costs the H2 delay-slot steal. Every spelling that
 * puts BOTH rect reads ahead of the BF28 store measured build_insns 48 (one
 * instruction short of target's 49), i.e. the guard collapsed back to the
 * `beqz` form and the `j <epi> / addiu $v0,-1` pair was lost:
 *
 *   A1 both walked ahead (src = *rect++; wh = *rect;)          20 / 48
 *   A2 both walked, pointer advanced twice                      20 / 48
 *   A3 only wh = rect[1] hoisted, rect[0] left inline           14 / 48
 *   A4 both ahead, stores ordered BF28 / BF2C / BF24            20 / 48
 *   A5 wh read between the BF28 and BF24 stores                 10 / 48
 *   A7 both ahead + BF24 store through the walked pointer       20 / 48
 *   A8 wh read first via a backwards-walking pointer            20 / 48
 *   A9 both reads hoisted above the bf24 pointer materialization 20 / 48
 *   W4 both reads into plain locals before the BF28 store       20 / 48
 *   W5 rect[0] ahead, BF24 store moved last                     20 / 48
 *
 * This matters because the sched2 dump says the ONLY way to lower the
 * remaining residual by shortening insn 80's (the BF2C store's) dependence
 * chain is to make the rect[1] load depth-1, which is exactly this hoist.
 * The lever and the constraint are mutually exclusive: at most ONE of the two
 * rect reads can sit ahead of the BF28 store while the `sll` keeps the head of
 * the post-guard block. Do not re-derive this.
 */
s32 MoveImage(s32 *arg0, s16 arg1, s16 arg2) {
    s32 *p;
    s32 (*fn)();
    s32 packed;
    s32 *bf24;
    s32 *rect;
    s32 src;
    s32 wh;

    func_8007B3A8(&D_80015F74, (s32)arg0);
    if (((s16 *)arg0)[2] == 0 || ((s16 *)arg0)[3] == 0) {
        return -1;
    }
    packed = ((s32)arg2 << 16) | ((u32)arg1 & 0xFFFF);
    bf24 = &D_8009BF24;
    rect = arg0;
    src = *rect++;
    wh = *rect;              /* <-- the fatal hoist: costs build_insns 49 -> 48 */
    D_8009BF28 = packed;
    *bf24 = src;
    D_8009BF2C = wh;
    p = (s32 *)g_gpu_dev_table;
    fn = (s32 (*)())p[2];
    return fn(p[6], (s32)bf24 - 8, 0x14, 0);
}
