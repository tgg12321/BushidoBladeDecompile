/* score 1 (u8/u16) and 2 (s8/s16) - a narrow-typed zero-holder.  Measured session 2:
 * v1 u8 -> 1, v2 s16 -> 2, v3 u16 -> 1, v4 `u8 abr = 0;` -> 1, v5 s8 -> 2.  The frame
 * comes out right (the 7th callee-saved register is allocated) but the argument needs
 * a zero/sign-extension insn the target does not have.  Dead axis: SImode is required
 * at the call, and SImode works (candidate.c), so nothing narrow is ever needed.
 */
/* BEGIN func_8006DD94 */
typedef struct {
    s32 *p0;
    s32 *p1;
    s32 in_tex;
    s32 pad0C;
    s32 zero10;
    s32 arg2;
    s32 width;
    s32 zero1C;
    s32 pad20;
    s32 pad24;
    u8 has_color;
    u8 col_r;
    u8 col_g;
    u8 col_b;
    s32 pad2C;
    s32 pad30;
} EnvB;
extern s32 D_800A374C;
extern void func_8006D808(s32 *, s32 *, s32 *, s32, s32);
void func_8006DD94(s32 *arg0) {
    EnvB s;
    u16 rect[4];
    s16 i;
    s32 *q;
    s32 c;
    s32 p0;
    u8 abr;

    abr = 0;
    s.arg2 = 0xA;
    q = *(s32 **)(arg0[1] + 0x3C);
    s.width = 0;
    s.zero10 = 0;

    for (i = 0; i < 3; i++) {
        s.has_color = 1;
        if (i == D_800A352C + 1) {
            s.zero1C = *(s16 *)(D_800A34FC + 0xE);
            c = ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x80;
            s.col_r = s.col_g = s.col_b = c;
        } else {
            if (i == 0) {
                s.col_r = s.col_g = s.col_b = 0x80;
            } else {
                s.col_r = s.col_g = s.col_b = 0x40;
            }
            s.zero1C = 0;
        }
        p0 = q[i + 8];
        s.p0 = (s32 *)p0;
        s.p1 = (s32 *)(p0 + 0xC);
        s.in_tex = arg0[5];
        arg0[5] = func_8007352C((s32)&s);
        SetDrawMode(arg0[7], 1, 0, func_8006E480((s32)s.p0, abr), 0);
        AddPrim(D_800A374C + 0x28, arg0[7]);
        arg0[7] += 0xC;
    }

    func_8006D808(&arg0[5], &arg0[7], q, s.arg2, -1);

    rect[2] = 0x96;
    rect[0] = 0xF5;
    rect[1] = 0x25;
    rect[3] = 1;
    func_80069898((GameObj *)arg0, rect, 0x11);
}
/* END func_8006DD94 */
