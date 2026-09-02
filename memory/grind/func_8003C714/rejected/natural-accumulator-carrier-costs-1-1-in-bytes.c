/*
 * REJECTED (s7, 2026-09-01) - K21. The s6 frontier's LIVE item ("find an
 * ORDINARY-C spelling of >= 64 extra loop-carried RTL insns that die inside
 * loop_optimize") is FORECLOSED: the free channel is open ONLY to
 * constant-step bivs, and a NATURAL carrier is by definition data-dependent,
 * hence not a biv, hence not final-value-replaceable, hence 1:1 in bytes.
 *
 * MEASURED (tmp/grind/func_8003C714/s7/gen_acc.py via sweep.sh; baseline = 56
 * real insns / 107 asm lines):
 *   acc += *(s32*)(src+4), used after the loop  : 58 insns, 112 lines (+2/+5)
 *   ... same but DEAD after the loop            : 56 insns, 107 lines (+0/+0)
 *   4x acc += mem, used after                   : 61 insns, 115 lines (+5/+8)
 *   acc = acc*31 + mem (checksum), used after   : 60 insns, 114 lines (+4/+7)
 *   16x acc += mem, used after                  : 74 insns, 128 lines (+18/+21)
 *   8x acc = acc*31 + mem, used after           : 82 insns, 136 lines (+26/+29)
 *   CONTROL acc += 1, post-use `acc - 3`        : 57 insns, 107 lines (+1/+0)
 *
 * Two laws fall out, and together they close the axis:
 *  (a) DEAD data-dependent carrier -> insn_count +0. It never reaches
 *      count_loop_regs_set; cse1 / delete_dead_from_cse (toplev.c:2865-2866)
 *      remove it first. This is the K10/K15/K16 result, re-confirmed.
 *  (b) LIVE data-dependent carrier -> insn_count +N and asm +1.12N..2.5N.
 *      strength_reduce can only delete a carrier whose exit value it can FOLD,
 *      i.e. a biv with a CONSTANT increment (loop.c final-value replacement).
 *      A running sum / checksum / anything reading loop data has a
 *      non-constant increment, is never a biv, and must therefore be COMPUTED
 *      - every counted insn survives to the assembler.
 * Reaching insn_count >= 120 (the loop.c:1631 flip point at threshold 119)
 * with a natural carrier therefore costs about +72 emitted instructions on a
 * 104-instruction target (build would be ~177 instructions). The s6
 * reachability proof used the ONLY free spelling there is: an artificial
 * small-immediate-step biv whose sole use is post-loop - dead code by
 * construction, inadmissible under the frozen family list.
 *
 * The body below is the 16x memory-sum accumulator (+18 insn_count, +21 asm).
 */
void func_8003C714(void) {
    u8 buf[4];
    s32 *s0;
    s32 i;
    s32 acc;
    u8 *src;
    u8 *dst;

    s0 = func_80077D00();
    func_800372C0();
    gpu_InitDisplay();
    func_80060758();
    i = 0;
    acc = 0;
    do {
        src = (u8 *)&D_80106A58 + i * 8;
        dst = (u8 *)s0 + i * 4;
        dst[0x21] = *(s32 *)(src + 4) / 1800;
        dst[0x22] = (*(s32 *)(src + 4) / 30) % 60;
        dst[0x23] = ((*(s32 *)(src + 4) % 30) * 100) / 30;
        dst[0x24] = *src;
        acc += *(s32 *)(src + 0);
        acc += *(s32 *)(src + 4);
        acc += *(s32 *)(src + 0);
        acc += *(s32 *)(src + 4);
        acc += *(s32 *)(src + 0);
        acc += *(s32 *)(src + 4);
        acc += *(s32 *)(src + 0);
        acc += *(s32 *)(src + 4);
        acc += *(s32 *)(src + 0);
        acc += *(s32 *)(src + 4);
        acc += *(s32 *)(src + 0);
        acc += *(s32 *)(src + 4);
        acc += *(s32 *)(src + 0);
        acc += *(s32 *)(src + 4);
        acc += *(s32 *)(src + 0);
        acc += *(s32 *)(src + 4);
        i += 1;
    } while (i < 3);
    func_8001CD68(buf);
    *((u8 *)s0 + 0x2D) = *(u16 *)buf;
    *((u8 *)s0 + 0x2E) = buf[2];
    *((u8 *)s0 + 0x2F) = buf[3];
    *((u8 *)s0 + 0x30) = *(u16 *)&D_80101ED2;
    disp_SetFramebufferMode(1, 0, 0, 0);
    D_800A37B8 = acc;
    D_800A3834 = 0x1F;
    gpu_DisableDisplay();
}
