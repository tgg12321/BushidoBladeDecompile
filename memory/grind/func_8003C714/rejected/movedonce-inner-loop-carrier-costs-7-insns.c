/* REJECTED (s11, 2026-09-05) - moved_once carrier reproduces the TARGET'S EXACT
 * movable pattern but costs +7 emitted instructions.
 *
 * WHAT IT PROVES (new, and it CORRECTS s7's K20):
 *   loop.c:1609 `if (moved_once[regno]) insn_count *= 2;` mutates scan_loop's
 *   `insn_count` PERMANENTLY for every subsequent movable in the same loop.
 *   s7 attached moved_once to the 0x91A2B3C5 magic ITSELF and concluded the
 *   doubling is "self-defeating" because the inner-loop hoist inflates that
 *   movable's own lifetime. That conclusion does not hold when the carrier is a
 *   DIFFERENT, EARLIER movable: the magic's lifetime stays 1 and the doubling
 *   still applies to its test.
 *
 * MEASURED (.loop dump, tmp/grind/func_8003C714/s11/varE.loop):
 *   Loop from 25 to 174: 62 real insns.
 *   Insn  33: regno 79 (life  1) moved              <- &D_80106A58   (target hoists it)
 *   Insn 231: regno 82 (life  9) halved since already moved  moved   <- the carrier
 *   Insn  74: regno 87 (life  1) NOT DESIRABLE      <- 0x91A2B3C5    (target keeps in-loop)
 *   Insn  88: regno 94 (life 31) moved              <- 0x88888889    (target hoists it)
 *   That is the target's preheader, movable for movable.
 *   sandbox func_8003C714 --disable all = 18, build_insns 112 (baseline 105).
 *
 * COST: +7 emitted instructions for the inner loop (init / store / increment /
 * compare+branch / the hoisted const in the outer preheader). The target is 104
 * against our 105, so there is no room.
 *
 * The 0x12345678 spelling of the same carrier (varD.c) costs +8 (score 19).
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
            D_800A37B8 = 5;
            j += 1;
        } while (j < 2);
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
    D_800A37B8 = 0;
    D_800A3834 = 0x1F;
    gpu_DisableDisplay();
}
