/* REJECTED s2 (2026-07-21) — frontier F1: struct-typed device-table dispatch.
 * Both spellings measured sandbox 6 (NEUTRAL — identical to the untyped
 * v0[11] cast; no pseudo-landscape change at the call boundary reaches the
 * sched.c/RA cascade). Kills F1 entirely: the dispatch-load surface type
 * (untyped word array, typed struct member, with or without intermediate
 * local) does not affect the post-call dataflow shape.
 *
 * Form A (typed local): score 6
 * Form C (direct member call, no local): score 6
 */

typedef struct {
    u32 pad[11];
    void (*otc)(u32 *, s32);
} GpuDevice;

u32 *func_8007B844(u32 *ot, s32 n) {
    u32 mask;
    if (g_gpu_debug_level >= 2) {
        g_gpu_debug_func(&D_80015F98, ot, n);
    }
    /* Form A */
    {
        GpuDevice *dev = (GpuDevice *)g_gpu_dev_table;
        dev->otc(ot, n);
    }
    /* Form C (alternative measured spelling):
     * ((GpuDevice *)g_gpu_dev_table)->otc(ot, n);
     */
    mask = 0xFFFFFF;
    mask = ((u32)&g_gpu_ot_end) & mask;
    *ot = mask;
    return ot;
}
