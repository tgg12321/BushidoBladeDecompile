/*
 * REJECTED (s7, 2026-09-01) - K20. The loop.c:1609 `moved_once` insn_count
 * DOUBLING is real but SELF-DEFEATING, and this form measures why.
 *
 * The idea (s1 frontier item 1, never measured until now): loop_optimize
 * (loop.c:425) scans loops last-first, so an INNER loop is scanned before its
 * outer loop, and `moved_once` is allocated ONCE for the whole function
 * (loop.c:344) - it persists across loop scans. If the 0x91A2B3C5 movable is
 * hoisted out of an inner loop (into the inner preheader, still inside the
 * outer loop), then when the OUTER loop is scanned the same regno hits
 * `if (moved_once[regno]) insn_count *= 2;` (loop.c:1609-1613), which would
 * halve the +64 insn_count requirement to +4.
 *
 * MEASURED (tmp/grind/func_8003C714/s7/dumps/inner{1,2}.i.loop):
 *   inner k=1: outer "Loop from 25 to 168: 59 real insns."
 *              "Insn 225: regno 85 (life 14) ... halved since already moved  moved to 229"
 *   inner k=2: outer "Loop from 25 to 169: 60 real insns."
 *              "Insn 226: regno 85 (life 15) ... halved since already moved  moved to 230"
 * The doubling FIRES (59 -> 118, 60 -> 120) and the movable is STILL MOVED,
 * because the same inner-loop hoist that sets moved_once also moves the SET
 * away from its USE: m->lifetime goes 1 -> 14/15. The desirability test is
 * `threshold * savings * m->lifetime >= insn_count` (loop.c:1631), so the
 * left-hand side grows 14-15x while the right-hand side grows only 2x
 * (119 * 1 * 15 = 1785 >= 120). The 2x gain is bought with a >=14x loss.
 *
 * This forecloses the doubling channel for ANY spelling: the only way to set
 * moved_once for this pseudo is to hoist it out of an enclosing inner loop,
 * and that hoist necessarily separates the const load from its single `mult`
 * use by the whole inner-loop entry, inflating lifetime far past the factor of
 * two the doubling buys. (Independently, a trip-count-1 inner loop is a banned
 * respelling of the do-while(0) carve-out, so the form was never admissible
 * either - but it is now dead on the MECHANISM, not merely on policy.)
 */
void func_8003C714(void) {
    u8 buf[4];
    s32 *s0;
    s32 i;
    s32 j;
    u8 *src;
    u8 *dst;

    s0 = func_80077D00();
    func_800372C0();
    gpu_InitDisplay();
    func_80060758();
    i = 0;
    do {
        src = (u8 *)&D_80106A58 + i * 8;
        dst = (u8 *)s0 + i * 4;
        j = 0;
        do {
            dst[0x21] = *(s32 *)(src + 4) / 1800;
            j += 1;
        } while (j < 2);
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
    D_800A37B8 = 0;
    D_800A3834 = 0x1F;
    gpu_DisableDisplay();
}
