void _spu_FwriteByIO(u8 *addr, u32 size) {
    u16 spustat;
    s32 num_to_trans;
    u16 *cur_pos;
    s32 spustat_cur;
    s32 i;
    u16 cnt;
    u32 timeout;

    cur_pos = (u16 *)addr;
    spustat = *(volatile u16 *)(_spu_RXX + 0x1AE) & 0x7FF;
    *(volatile u16 *)(_spu_RXX + 0x1A6) = _spu_tsa;
    _spu_Fw1ts();
    while (size > 0) {
        num_to_trans = (size > 0x40) ? 0x40 : size;
        for (i = 0; i < num_to_trans; i += 2) {
            *(volatile u16 *)(_spu_RXX + 0x1A8) = *cur_pos++;
        }
        cnt = *(volatile u16 *)(_spu_RXX + 0x1AA);
        cnt &= ~0x30;
        cnt |= 0x10;
        *(volatile u16 *)(_spu_RXX + 0x1AA) = cnt;
        _spu_Fw1ts();
        timeout = 0;
        while (*(volatile u16 *)(_spu_RXX + 0x1AE) & 0x400) {
            timeout++;
            if (timeout > 0xF00) {
                printf(&D_800163D8, &D_800163F8);
                break;
            }
        }
        _spu_Fw1ts();
        _spu_Fw1ts();
        size -= num_to_trans;
    }
    cnt = *(volatile u16 *)(_spu_RXX + 0x1AA);
    cnt &= ~0x30;
    *(volatile u16 *)(_spu_RXX + 0x1AA) = cnt;
    timeout = 0;
    spustat_cur = *(volatile u16 *)(_spu_RXX + 0x1AE) & 0x7FF;
    while (spustat_cur != spustat) {
        timeout++;
        if (timeout > 0xF00) {
            printf(&D_800163D8, &D_8001640C);
            return;
        }
        spustat_cur = *(volatile u16 *)(_spu_RXX + 0x1AE) & 0x7FF;
    }
}
