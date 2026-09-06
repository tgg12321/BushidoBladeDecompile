/* _addque2 — CANDIDATE (session 1, 2026-09-06) — sandbox 0/184 with the volatile
 * queue LIVE (--keep-cheat-asm), full verify-oracle ok (SHA1 62efab4f...), honest
 * (detector-stripped, un-granted) floor 18. BLOCKED ONLY by an integration handoff:
 *   1. volatile_extern_allowlist.txt entry for D_80103680 (Ruling 4, commit c80d976e —
 *      same class as the D_8009BF68 / D_8009BF78 grants at allowlist lines 75-76).
 *   2. aggregate-merge prong (d): move `typedef struct GpuQueueItem` + the extern to
 *      include/gpu.h (TU-local here only because include/ is outside session scope).
 *   3. prong (c) amendment 2026-09-03: rows D_80103684 / D_80103688 / D_8010368C stay in
 *      undefined_syms_auto.txt + named_syms.txt (asm/funcs/_exeque.s still names them)
 *      suffixed "alias of D_80103680+N; retire with _exeque".
 * DECLARATION CHANGES in src/display.c that are part of the recipe (all measured):
 *   - line 720  `extern s32 *D_8009BF48;`  -> `extern volatile s32 *D_8009BF48;`
 *       (GPU_STATUS 0x1F801814 pointee; 29 -> 22; the later site :747 was already volatile;
 *        .loop dump showed loop.c hoisting the read out of the spin-wait otherwise)
 *   - lines 732 + 795 `extern s32 *D_8009BF54;` -> `extern volatile s32 *D_8009BF54;`
 *       (DMA2_CHCR 0x1F8010A8 pointee; 22 -> 20; _reset still byte-identical: oracle ok)
 *   - the four `extern s32 D_80103680/84/88/8C;` per-word scalars REMOVED; the
 *     `extern u8 D_80103680[];` view removed; _reset's memset takes `(u8 *)D_80103680`.
 *   - `extern volatile GpuQueueItem D_80103680[64];` (20 -> 2; the SOTN original is
 *     `static volatile struct QueueItem D_80037F54[0x40]`, sys.c:95 of the
 *     sotn-decomp clone at C:/Users/Trenton/Desktop/sotn-decomp)
 *   - copy loop spelled `arg[i]` not `*p++` (2 -> 0; loop.c giv init order).
 * Reference shape: sotn-decomp src/main/psxsdk/libgpu/sys.c:744 `_addque2`.
 * Exact verified diff: tmp/grind/_addque2/s1/candidate-diff.txt (also copied to
 * memory/grind/_addque2/s1-candidate-diff.txt).
 */
/* ADDQUE2-BEGIN */
/* LIBGPU/SYS `_addque2` — reference sotn-decomp src/main/psxsdk/libgpu/sys.c:744
 * (older library revision: per-store re-index of the volatile queue head,
 * 0x60-byte slots = func / arg / count / 21 data words). */
typedef struct GpuQueueItem {
    /* 0x00 */ s32 (*func)(s32 *, s32);
    /* 0x04 */ s32 *arg;
    /* 0x08 */ s32 count;
    /* 0x0C */ s32 data[21];
} GpuQueueItem; /* size 0x60 */
extern volatile GpuQueueItem D_80103680[64]; /* g_gpu_packet_queue_base: 64 x 0x60 */

s32 _addque2(s32 (*func)(s32 *, s32), s32 *arg, s32 len, s32 count) {
    s32 i;

    set_alarm();
    while (((D_8009BF78 + 1) & 0x3F) == D_8009BF7C) {
        if (get_alarm() != 0) {
            return -1;
        }
        _exeque();
    }
    D_8009BF80 = SetIntrMask(0);
    D_8009BE7C = 1;
    if (D_8009BE75 == 0 ||
        (D_8009BF78 == D_8009BF7C && !(*D_8009BF54 & 0x01000000) && D_8009BE80 == 0)) {
        while (!(*D_8009BF48 & 0x04000000)) {
        }
        func(arg, count);
        D_8009BF68[0] = (s32)func;
        D_8009BF6C = (s32)arg;
        D_8009BF70 = count;
        SetIntrMask(D_8009BF80);
        return 0;
    }
    DMACallback(2, _exeque);
    if (len != 0) {
        for (i = 0; i < len / 4; i++) {
            D_80103680[D_8009BF78].data[i] = arg[i];
        }
        D_80103680[D_8009BF78].arg = D_80103680[D_8009BF78].data;
    } else {
        D_80103680[D_8009BF78].arg = arg;
    }
    D_80103680[D_8009BF78].count = count;
    D_80103680[D_8009BF78].func = func;
    D_8009BF78 = (D_8009BF78 + 1) & 0x3F;
    SetIntrMask(D_8009BF80);
    _exeque();
    return (D_8009BF78 - D_8009BF7C) & 0x3F;
}
/* ADDQUE2-END */
