/* two-separate-rect-arrays  -  NOT disproven: this form BUILDS THE ORACLE.
 *
 * s4 (enumerate, 2026-09-10) RE-VERIFIED this exact body on the current HEAD chassis:
 *   sandbox func_8006DD94 --disable all  -> score 21 (117/117, rules_dropped 0)
 *   verify-oracle                        -> ok true, build_matches true,
 *                                           build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa
 * i.e. the whole 606,208-byte SLUS_006.63 is byte-identical to the original with this
 * body in src/text1b.c.  The 21 comes from engine/volatile_cheats.py stripping the
 * untouched `u16 rect0[4]` out of the SCORED object file only.
 *
 * It lives in rejected/ ONLY because its construct class (a declared-but-untouched
 * local array) has not been ruled on by the Judge for this function.  It is NOT on the
 * BANNED CONSTRUCTS list: the banned forms are the MERGED `u16 rects[2][4]` and the
 * function-local EnvB struct's trailing pad members, both of which respell the hole
 * inside a live declaration.  This form declares a separate sibling object.
 *
 * s4's 41-spelling enumeration (evidence.md s4) showed this is one of only six spellings
 * out of 41 that reproduce the target frame (vars= 64, rect at sp+0x50, body n= 112) and
 * that all six reserve via an object no instruction touches - so no expression-level
 * spelling replaces it.
 *
 * DO NOT submit as candidate-ready without a Judge ruling on the construct class.
 */
/* BEGIN func_8006DD94 */
typedef struct EnvB {
    s32 *header;
    s8  *table;
    s32  out;
    s32  pad0C;
    s32  semi;
    u32  ot_idx;
    s32  x;
    s32  y;
    s32  pad20, pad24;
    u8   has_color;
    u8   col_r;
    u8   col_g;
    u8   col_b;
} EnvB;
extern s32 D_800A374C;
extern void func_8006D808(s32 *, s32 *, s32 *, s32, s32);
void func_8006DD94(s32 *arg0) {
    EnvB s;
    u16 rect0[4];
    u16 rect[4];
    s16 i;
    s32 *q;
    s32 c;
    s32 hdr;
    s32 semi = 0;

    s.ot_idx = 0xA;
    q = *(s32 **)(arg0[1] + 0x3C);
    s.x = 0;
    s.semi = semi;

    for (i = 0; i < 3; i++) {
        s.has_color = 1;
        if (i == D_800A352C + 1) {
            s.y = *(s16 *)(D_800A34FC + 0xE);
            c = ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x80;
            s.col_r = s.col_g = s.col_b = c;
        } else {
            if (i == 0) {
                s.col_r = s.col_g = s.col_b = 0x80;
            } else {
                s.col_r = s.col_g = s.col_b = 0x40;
            }
            s.y = 0;
        }
        hdr = q[i + 8];
        s.header = (s32 *)hdr;
        s.table = (s8 *)(hdr + 0xC);
        s.out = arg0[5];
        arg0[5] = func_8007352C((s32)&s);
        SetDrawMode(arg0[7], 1, 0, func_8006E480((s32)s.header, semi), 0);
        AddPrim(D_800A374C + 0x28, arg0[7]);
        arg0[7] += 0xC;
    }

    func_8006D808(&arg0[5], &arg0[7], q, s.ot_idx, -1);

    rect[2] = 0x96;
    rect[0] = 0xF5;
    rect[1] = 0x25;
    rect[3] = 1;
    func_80069898((GameObj *)arg0, rect, 0x11);
}
/* END func_8006DD94 */