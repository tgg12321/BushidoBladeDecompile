/* PsyQ LIBSPU spu.c: `_spu_FwriteByIO` (static) — verbatim-linked Sony object
   (census 2026-07-09).  C refs: Xeeynamo/psyz decomp/src/libspu/spu.c:111 and
   sotn-decomp psxsdk/libspu/spu.c (_spu_writeByIO).

   `_spu_RXX` (0x800A2CDC) holds the SPU register-file base (0x1F801C00), so
   `_spu_RXX + 0x1A6` is the SPU transfer/control register block at 0x1F801DA6:
   transfer address, data FIFO, SPUCNT, transfer control, SPUSTAT — five
   consecutive 16-bit hardware registers.  Sony's own libspu reaches them
   through `union SpuUnion *_spu_RXX` with the SPUR()/SPUW() field macros; the
   struct below is that same register block, and every access in this function
   goes through it, exactly as the original source does.

   Applying this body also deletes the redundant forward declaration
   `extern void _spu_FwriteByIO(s32, s32);` (HEAD src/main.c:1898); both call
   sites (src/main.c:1719, :1905) pass s32 values and compile unchanged. */
typedef struct {
    u16 trans_addr;  /* 0x1DA6 */
    u16 trans_fifo;  /* 0x1DA8 */
    u16 spucnt;      /* 0x1DAA */
    u16 trans_ctrl;  /* 0x1DAC */
    u16 spustat;     /* 0x1DAE */
} SpuCtrlRegs;

#define SPU_CTRL ((volatile SpuCtrlRegs *)(_spu_RXX + 0x1A6))

void _spu_FwriteByIO(u8 *addr, u32 size) {
    u16 spustat;
    s32 num;
    u16 *cur;
    s32 i;
    u32 j;
    u16 cnt;

    cur = (u16 *)addr;
    spustat = SPU_CTRL->spustat & 0x7FF;
    SPU_CTRL->trans_addr = _spu_tsa;
    _spu_Fw1ts();
    while (size != 0) {
        num = (size > 0x40) ? 0x40 : size;
        for (i = 0; i < num; i += 2) {
            SPU_CTRL->trans_fifo = *cur++;
        }
        cnt = SPU_CTRL->spucnt;
        cnt &= ~0x30;
        cnt |= 0x10;
        SPU_CTRL->spucnt = cnt;
        _spu_Fw1ts();
        if (SPU_CTRL->spustat & 0x400) {
            j = 0;
            do {
                if (++j > 0xF00) {
                    printf(&D_800163D8, &D_800163F8);
                    break;
                }
            } while (SPU_CTRL->spustat & 0x400);
        }
        _spu_Fw1ts();
        _spu_Fw1ts();
        size -= num;
    }
    cnt = SPU_CTRL->spucnt;
    j = 0;
    cnt &= ~0x30;
    SPU_CTRL->spucnt = cnt;
    if ((SPU_CTRL->spustat & 0x7FF) != spustat) {
        do {
            if (++j > 0xF00) {
                printf(&D_800163D8, &D_8001640C);
                break;
            }
        } while ((SPU_CTRL->spustat & 0x7FF) != spustat);
    }
}
