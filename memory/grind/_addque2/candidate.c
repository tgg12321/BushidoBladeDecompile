/* _addque2 candidate -- s1 (2026-09-06, recon; grant executed). Honest sandbox 0 (184/184),
 * verify-oracle ok (build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa) with the
 * granted-path edits live:
 *   include/gpu.h              GpuQueueItem typedef + `extern volatile GpuQueueItem D_80103680[64];`
 *   volatile_extern_allowlist  D_80103680 Ruling 4 grant line
 *   undefined_syms_auto.txt    alias-suffix rows for D_80103684/88/8C (retire with _exeque)
 *   named_syms.txt             same alias suffix on the three census rows
 *   src/display.c              body below + `volatile s32 *` pointee on D_8009BF48 (:720)
 *                              and D_8009BF54 (:732/:795), four per-word externs removed,
 *                              `(u8 *)` cast on _reset's memset.
 * Full diff: memory/grind/_addque2/s1-granted-path-diff.txt. */
/* ADDQUE2-BEGIN */
/* LIBGPU/SYS `_addque2` — reference sotn-decomp src/main/psxsdk/libgpu/sys.c:744
 * (older library revision: per-store re-index of the volatile queue head,
 * 0x60-byte slots = func / arg / count / 21 data words). */
/* GpuQueueItem + `extern volatile GpuQueueItem D_80103680[64];` live in
 * include/gpu.h (aggregate-merge prong (d): header-canonical). */

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
