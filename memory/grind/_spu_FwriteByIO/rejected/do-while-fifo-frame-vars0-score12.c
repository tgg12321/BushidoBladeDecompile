/* PsyQ 4.0 LIBSPU spu.c: _spu_FwriteByIO (static) — verbatim-linked Sony
   object (census 2026-07-09); C ref: Xeeynamo/psyz decomp/src/libspu/spu.c:111
   and sotn-decomp psxsdk/libspu/spu.c (_spu_writeByIO). */
void _spu_FwriteByIO(u8 *addr, u32 size) {
    u16 spustat;
    s32 num;
    u16 *cur;
    s32 i;
    u32 j;
    u16 cnt;
    s32 b;

    cur = (u16 *)addr;
    spustat = *(volatile u16 *)(_spu_RXX + 0x1AE) & 0x7FF;
    *(volatile u16 *)(_spu_RXX + 0x1A6) = _spu_tsa;
    _spu_Fw1ts();
    while (size != 0) {
        num = (size > 0x40) ? 0x40 : size;
        i = 0;
        if (num > 0) {
            b = _spu_RXX;
            do {
                *(volatile u16 *)(b + 0x1A8) = *cur++;
                i += 2;
            } while (i < num);
        }
        cnt = *(volatile u16 *)(_spu_RXX + 0x1AA);
        cnt &= ~0x30;
        cnt |= 0x10;
        *(volatile u16 *)(_spu_RXX + 0x1AA) = cnt;
        _spu_Fw1ts();
        if (*(volatile u16 *)(_spu_RXX + 0x1AE) & 0x400) {
            j = 0;
            do {
                if (++j > 0xF00) {
                    printf(&D_800163D8, &D_800163F8);
                    break;
                }
            } while (*(volatile u16 *)(_spu_RXX + 0x1AE) & 0x400);
        }
        _spu_Fw1ts();
        _spu_Fw1ts();
        size -= num;
    }
    cnt = *(volatile u16 *)(_spu_RXX + 0x1AA);
    j = 0;
    cnt &= ~0x30;
    *(volatile u16 *)(_spu_RXX + 0x1AA) = cnt;
    if ((*(volatile u16 *)(_spu_RXX + 0x1AE) & 0x7FF) != spustat) {
        do {
            if (++j > 0xF00) {
                printf(&D_800163D8, &D_8001640C);
                break;
            }
        } while ((*(volatile u16 *)(_spu_RXX + 0x1AE) & 0x7FF) != spustat);
    }
}
