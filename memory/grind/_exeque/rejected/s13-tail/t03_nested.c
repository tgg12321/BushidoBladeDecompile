/* PsyQ 4.0 LIBGPU SYS: _exeque â€” verbatim-linked Sony object (census 2026-07-09); C ref:
 * sotn-decomp src/main/psxsdk/libgpu/sys.c:797 is an older revision (null-func reset path,
 * CheckCallback tail) and was not adopted. Drains the packet queue; when it is empty and a
 * draw is pending, clears the pending flag and calls the DrawSyncCallback. */
s32 _exeque(void) {
    if (*D_8009BF54 & 0x01000000) {
        return 1;
    }
    D_8009BF84 = SetIntrMask(0);
    while (_qin != _qout && !(*D_8009BF54 & 0x01000000)) {
        if (((_qout + 1) & 0x3F) == _qin && g_gpu_ctx.drawsync_cb == 0) {
            DMACallback(2, NULL);
        }
        while (!(*D_8009BF48 & 0x04000000)) {
        }
        _que[_qout].func(_que[_qout].arg, _que[_qout].count);
        _qlog[0] = (s32)_que[_qout].func;
        D_8009BF6C = _que[_qout].arg;
        /* FAKE: do-while(0) â€” its loop notes keep this log store between the arg log store
         * and the _qout advance; without it sched sinks both log stores to the loop test
         * (score 10; memory/grind/_exeque/evidence.md [s13]) */
        do {
            D_8009BF70 = _que[_qout].count;
        } while (0);
        _qout = (_qout + 1) & 0x3F;
    }
    SetIntrMask(D_8009BF84);
    if (_qin == _qout && !(*D_8009BF54 & 0x01000000)) {
        if (g_gpu_ctx.unk08 != 0) {
            if (g_gpu_ctx.drawsync_cb != 0) {
                g_gpu_ctx.unk08 = 0;
                ((void (*)(void))g_gpu_ctx.drawsync_cb)();
            }
        }
    }
    return (_qin - _qout) & 0x3F;
}
