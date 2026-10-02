/* PsyQ LIBGPU.H DR_MOVE (VRAM-to-VRAM copy primitive). */
typedef struct {
    OTag tag;
    u32 code[5];
} DR_MOVE; /* 0x18 */

/* One VRAM-scroll channel at D_800EF848 (0x134 bytes each). */
typedef struct {
    s32 phase;          /* +0x000 */
    DR_MOVE move[2][6]; /* +0x004: one bank per frame parity, two packets per level */
    s16 ctl[7];         /* +0x124: filled from D_80099C34 by func_80048F58 */
} MoveChannel;

extern MoveChannel D_800EF848[];
extern u16 D_80099C34[];
extern void func_80052C10(void);
void func_80048F58(s32 a0, s32 a1) {
    s32 i;
    u16 *src;
    s16 *dst;
    MoveChannel *base;
    if (a1 > 0) {
        func_80052C10();
    }
    base = &D_800EF848[a1];
    base->phase = 0;
    src = (u16 *)(D_80099C34 + a0 * 7);
    dst = base->ctl;
    i = 0;
    do {
        *dst = *src;
        src++;
        i++;
        dst++;
    } while (i < 7);
}

extern void SetDrawMove(DR_MOVE *, RECT *, u32, u32);
void func_80048FFC(s32 arg0) {
    RECT rect;
    s16 *ctl;
    s32 x;
    s32 y;
    s32 xf;
    s32 yf;
    s32 x2;
    s32 y2;
    s32 x2f;
    s32 y2f;
    s32 h;
    s32 end;
    s32 i;
    MoveChannel *ch;
    DR_MOVE *p;
    s32 phase;

    ch = &D_800EF848[arg0];
    phase = ch->phase;
    ctl = ch->ctl;
    p = ch->move[D_800A36AC & 1];
    x = ctl[0] & ~0x3F;
    y = ctl[1] & ~0xFF;
    xf = ctl[0] % 64;
    yf = ctl[1] % 256;
    h = ctl[2];
    end = ctl[3];
    x2 = ctl[4] & ~0x3F;
    y2 = ctl[5] & ~0xFF;
    x2f = ctl[4] % 64;
    y2f = ctl[5] % 256;
    i = 0;
    do {
        s32 nx = x2 + x2f;
        s32 ny = y2 + y2f;
        s32 cy = y + yf;
        s32 dy = end - phase;
        s32 old;
        rect.x = x + xf;
        rect.y = cy;
        rect.w = h;
        rect.h = dy;
        SetDrawMove(p, &rect, nx, ny + phase);
        old = phase;
        phase >>= 1;
        xf >>= 1;
        yf >>= 1;
        x2f >>= 1;
        y2f >>= 1;
        h >>= 1;
        end >>= 1;
        i++;
        p->tag.addr = ((OTag *)D_800A378C)[0xFFF].addr;
        ((OTag *)D_800A378C)[0xFFF].addr = (u32)p;
        p++;
        rect.y = cy + dy;
        rect.h = old;
        SetDrawMove(p, &rect, nx, ny);
        p->tag.addr = ((OTag *)D_800A378C)[0xFFF].addr;
        ((OTag *)D_800A378C)[0xFFF].addr = (u32)p;
        p++;
    } while (i < 3);
    ch->phase += ctl[6];
    if (ch->phase >= ctl[3]) {
        ch->phase %= ctl[3];
    }
}
