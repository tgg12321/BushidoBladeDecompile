/* score 9 - passing the descriptor field itself (`s.semi` / `s.zero10`) as the second
 * argument instead of a local.  &s escapes to func_8007352C inside the loop, so the
 * member must be reloaded every iteration: an extra lw in the loop, and still no
 * callee-saved 0.  Worse than the literal (8).  Measured session 2.
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
        SetDrawMode(arg0[7], 1, 0, func_8006E480((s32)s.p0, s.zero10), 0);
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
