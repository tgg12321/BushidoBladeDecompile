typedef int s32; typedef unsigned int u32; typedef unsigned short u16; typedef unsigned char u8;
extern s32 D_800A2CDC;
extern s32 D_800A2D2C;
extern volatile s32 D_800A2D14;
extern s32 *D_800A2CE0; extern s32 *D_800A2CE4; extern s32 *D_800A2CE8;
extern void spu_WriteReg16(void); extern void spu_ReadReg(void);
extern void bios_DeliverEvent(s32, s32);
static void _spu_FiDMA(void) {
    u32 i;
    s32 b;
    if (D_800A2D2C == 0) { spu_WriteReg16(); }
    b = D_800A2CDC;
    *(volatile u16 *)(b + 0x1AA) = *(volatile u16 *)(b + 0x1AA) & 0xFFCF;
    i = 0;
    while (*(volatile u16 *)(b + 0x1AA) & 0x30) {
        if (++i > 0xF00) { break; }
    }
    if (D_800A2D14 != 0) {
        bios_DeliverEvent(1, 2);
    } else {
        bios_DeliverEvent(0xF0000009, 0x20);
    }
}
static void _spu_Fr_(s32 arg0, u16 arg1, s32 arg2) {
    *(volatile u16 *)(D_800A2CDC + 0x1A6) = arg1;
    spu_WriteReg16();
    *(volatile u16 *)(D_800A2CDC + 0x1AA) = *(volatile u16 *)(D_800A2CDC + 0x1AA) | 0x30;
    spu_WriteReg16();
    spu_ReadReg();
    *D_800A2CE0 = arg0;
    *D_800A2CE4 = (arg2 << 16) | 0x10;
    D_800A2D2C = 1;
    *D_800A2CE8 = 0x1000200;
}
void anchor(void) { _spu_FiDMA(); _spu_Fr_(0,0,0); }
