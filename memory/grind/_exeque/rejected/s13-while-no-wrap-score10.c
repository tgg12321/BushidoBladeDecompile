extern s32 D_8009BF84;
s32 _exeque(void) {
    if (*D_8009BF54 & 0x01000000) {
        return 1;
    }
    D_8009BF84 = SetIntrMask(0);
    while (_qin != _qout && !(*D_8009BF54 & 0x01000000)) {
            if (((_qout + 1) & 0x3F) == _qin && g_gpu_ctx.drawsync_cb == 0) {
                DMACallback(2, 0);
            }
            while (!(*D_8009BF48 & 0x04000000)) {
            }
            _que[_qout].func(_que[_qout].arg, _que[_qout].count);
            _qlog[0] = (s32)_que[_qout].func;
            D_8009BF6C = _que[_qout].arg;
            D_8009BF70 = _que[_qout].count;
            _qout = (_qout + 1) & 0x3F;
    }
    SetIntrMask(D_8009BF84);
    if (_qin == _qout && !(*D_8009BF54 & 0x01000000) && g_gpu_ctx.unk08 != 0 && g_gpu_ctx.drawsync_cb != 0) {
        g_gpu_ctx.unk08 = 0;
        ((void (*)(void))g_gpu_ctx.drawsync_cb)();
    }
    return (_qin - _qout) & 0x3F;
}
