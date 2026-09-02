/*
 * REJECTED (s7, 2026-09-01) - K22. The s6 frontier's item 2 ("free HOISTS are
 * cheaper than free insns - ~21 free hoists ahead of the 0x91A2B3C5 movable
 * would need no extra insns at all") is FORECLOSED. The two channels are
 * MUTUALLY EXCLUSIVE.
 *
 * Arithmetic being attacked: loop.c:1719/1904 do `threshold -= 3` after every
 * movable actually moved, and the 0x91A2B3C5 movable is the SECOND movable in
 * the list (baseline .loop: `Insn 33: regno 78 (life 1) ... moved` then
 * `Insn 46: regno 84 (life 1) ... moved`), so it is tested at threshold 119
 * against insn_count 56. h extra hoists placed BEFORE it need `c > 63 - 3h`.
 *
 * The construct: k loop-carried counters with LARGE constant steps placed at
 * the TOP of the body, each post-loop-folded (`z0+...+zk - 3*sum`), so that
 * every step constant needs a lui/ori pair and therefore becomes an invariant
 * movable ahead of the magic, while the counter itself should be deleted by
 * biv elimination the way the s6 carrier was.
 *
 * MEASURED (tmp/grind/func_8003C714/s7/gen_hoist.py; baseline 56 insns/107 lines):
 *   k=1  : 58 insns, moved 9,  asm 116   (+9 lines)
 *   k=4  : 64 insns, moved 12, asm 137
 *   k=8  : 72 insns, moved 16, asm 165
 *   k=16 : 88 insns, moved 18, 6 not desirable,  asm 241
 *   k=22 : 100 insns, moved 14, 16 not desirable, asm 285
 * The hoists ARE created (moved 8 -> 9/12/16) but they are NOT free: about 9
 * emitted instructions per counter. The .loop dump names the mechanism in one
 * line - `Insn 49: possible biv, reg 74, const = (reg:SI 82)`. Once the step is
 * a REGISTER (which is exactly what makes the step constant hoistable), the biv
 * has a non-constant increment, loop.c cannot compute its final value, biv
 * elimination declines it ("biv 74 can be eliminated" never prints), and the
 * whole counter materialises.
 * Conversely the s6 free carrier uses small immediate steps, which fold into
 * `addiu` and create NO movable at all - zero hoists.
 * So: a step constant big enough to be hoisted destroys the elimination that
 * made the insns free; a step constant small enough to stay free creates no
 * hoist. There is no free-hoist family here.
 *
 * (Also worth recording: the ONE free hoist s6 observed - gen_biv k=2, moved
 * 8 -> 9 at 107 asm lines - is `Insn 145: regno 132 (life 1)`, the literal 21
 * of the `j != 21` exit test, at the BOTTOM of the loop. It is free only
 * because that biv SUBSTITUTES for the original counter, and being after the
 * magic in insn order it never reduces the threshold the magic is tested at.)
 *
 * The body below is k=4.
 */
void func_8003C714(void) {
    u8 buf[4];
    s32 *s0;
    s32 i;
    s32 z0;
    s32 z1;
    s32 z2;
    s32 z3;
    u8 *src;
    u8 *dst;

    s0 = func_80077D00();
    func_800372C0();
    gpu_InitDisplay();
    func_80060758();
    i = 0;
    z0 = 0;
    z1 = 0;
    z2 = 0;
    z3 = 0;
    do {
        src = (u8 *)&D_80106A58 + i * 8;
        dst = (u8 *)s0 + i * 4;
        z0 += 69905;
        z1 += 135448;
        z2 += 200991;
        z3 += 266534;
        dst[0x21] = *(s32 *)(src + 4) / 1800;
        dst[0x22] = (*(s32 *)(src + 4) / 30) % 60;
        dst[0x23] = ((*(s32 *)(src + 4) % 30) * 100) / 30;
        dst[0x24] = *src;
        i += 1;
    } while (i < 3);
    func_8001CD68(buf);
    *((u8 *)s0 + 0x2D) = *(u16 *)buf;
    *((u8 *)s0 + 0x2E) = buf[2];
    *((u8 *)s0 + 0x2F) = buf[3];
    *((u8 *)s0 + 0x30) = *(u16 *)&D_80101ED2;
    disp_SetFramebufferMode(1, 0, 0, 0);
    D_800A37B8 = z0 + z1 + z2 + z3 - 2018634;
    D_800A3834 = 0x1F;
    gpu_DisableDisplay();
}
