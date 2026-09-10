/* BEGIN func_8006DD94 */
typedef struct EnvB { s32 *header; s8 *table; s32 out; s32 pad0C; s32 semi; u32 ot_idx; s32 x; s32 y; s32 pad20, pad24; u8 has_color; u8 col_r; u8 col_g; u8 col_b; } EnvB;
extern s32 D_800A374C;
extern void func_8006D808(s32 *, s32 *, s32 *, s32, s32);
void func_8006DD94(s32 *arg0) {
    EnvB s;
    u16 rect[4];
    s16 i;
    s32 *q;
    s32 c;
    s32 hdr;
    s32 semi = 0;
    s32 sel = D_800A352C + 1;
    s32 env = D_800A34FC;
    s32 ph = D_800A3514;
    s32 prim = D_800A374C + 0x28;
    s32 outp = arg0[5];
    s32 otp = arg0[7];

    s.ot_idx = 0xA;
    q = *(s32 **)(arg0[1] + 0x3C);
    s.x = 0;
    s.semi = semi;

    for (i = 0; i < 3; i++) {
        s.has_color = 1;
        if (i == sel) {
            s.y = *(s16 *)(env + 0xE);
            c = ((rsin(((ph & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x80;
            s.col_r = s.col_g = s.col_b = c;
        } else {
            if (i == 0) { s.col_r = s.col_g = s.col_b = 0x80; }
            else { s.col_r = s.col_g = s.col_b = 0x40; }
            s.y = 0;
        }
        hdr = q[i + 8];
        s.header = (s32 *)hdr;
        s.table = (s8 *)(hdr + 0xC);
        s.out = outp;
        outp = func_8007352C((s32)&s);
        SetDrawMode(otp, 1, 0, func_8006E480((s32)s.header, semi), 0);
        AddPrim(prim, otp);
        otp += 0xC;
    }

    arg0[5] = outp;
    arg0[7] = otp;
    func_8006D808(&arg0[5], &arg0[7], q, s.ot_idx, -1);

    rect[2] = 0x96;
    rect[0] = 0xF5;
    rect[1] = 0x25;
    rect[3] = 1;
    func_80069898((GameObj *)arg0, rect, 0x11);
}
/* END func_8006DD94 */
