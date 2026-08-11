/* BEST ADAPTED BODY - src/display.c - func_8007D6D8 == LIBGPU/SYS `_exeque`
 * Reference: sotn-decomp src/main/psxsdk/libgpu/sys.c:797-857.
 * BB2 is an older library revision: the busy-chcr early-out returns 1 BEFORE taking
 * the interrupt mask; queue slots are a 0x60-stride triple of parallel arrays
 * (D_80103680 / D_80103684 / D_80103688) rather than SOTN's struct array; there is no
 * null-func _reset/printf arm; and the tail drain-callback is gated on
 * D_8009BE7C/D_8009BE80 instead of CheckCallback().
 * The 0x60 stride and the parallel-array idiom were read off BB2's own already-
 * decompiled producer func_8007D3F8 (_addque2), which writes the same three slots.
 *
 * MEASURED: 69   (baseline 187 = INCLUDE_ASM).  build 153 / target 187.
 * RESIDUAL (34 insns short): the target re-LOADS the queue-out index D_8009BF7C at
 * every single slot access; plain C common-subexpression-eliminates all of those
 * reloads into one. BB2's already-committed sibling func_8007D3F8 spells this with
 * `*(volatile s32 *)&D_8009BF78`, which is a volatile-coercion cheat under current
 * policy, so it was deliberately NOT used here - the honest reference-C number is 69.
 *
 * Replaces:  INCLUDE_ASM("asm/funcs", func_8007D6D8);
 * Also needs: the forward decl at display.c:816 changed from `void func_8007D6D8();`
 * to `s32 func_8007D6D8();`, and a new `extern s32 D_8009BF84;`.
 */

#define QSLOT_FN(i)  (*(s32 (**)(s32 *, s32))((s32)&D_80103680 + (i) * 0x60))
#define QSLOT_ARG(i) (*(s32 **)((s32)&D_80103684 + (i) * 0x60))
#define QSLOT_CNT(i) (*(s32 *)((s32)&D_80103688 + (i) * 0x60))

s32 func_8007D6D8(void) {
    if (*D_8009BF54 & 0x01000000) {
        return 1;
    }
    D_8009BF84 = SetIntrMask(0);
    if (D_8009BF78 == D_8009BF7C) {
        goto out;
    }
    if (*D_8009BF54 & 0x01000000) {
        goto out;
    }
loop:
    if (((D_8009BF7C + 1) & 0x3F) == D_8009BF78) {
        if (D_8009BE80 == 0) {
            DMACallback(2, 0);
        }
    }
    while (!(*D_8009BF48 & 0x04000000)) {
    }
    QSLOT_FN(D_8009BF7C)(QSLOT_ARG(D_8009BF7C), QSLOT_CNT(D_8009BF7C));
    D_8009BF68 = QSLOT_FN(D_8009BF7C);
    D_8009BF6C = QSLOT_ARG(D_8009BF7C);
    D_8009BF70 = QSLOT_CNT(D_8009BF7C);
    D_8009BF7C = (D_8009BF7C + 1) & 0x3F;
    if (D_8009BF78 == D_8009BF7C) {
        goto out;
    }
    if (!(*D_8009BF54 & 0x01000000)) {
        goto loop;
    }
out:
    SetIntrMask(D_8009BF84);
    if (D_8009BF78 == D_8009BF7C && !(*D_8009BF54 & 0x01000000) &&
        D_8009BE7C != 0 && D_8009BE80 != 0) {
        D_8009BE7C = 0;
        ((void (*)(void))D_8009BE80)();
    }
    return (D_8009BF78 - D_8009BF7C) & 0x3F;
}
