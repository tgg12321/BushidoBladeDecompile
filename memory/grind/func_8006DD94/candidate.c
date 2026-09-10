/* func_8006DD94 - BEST FORM, session 2 (recon).  sandbox --disable all = 0 AND
 * verify-oracle build_sha1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa this session.
 * ORDINARY C - no FAKE construct, no pointer-clothed zero-holder.
 *
 * What changed vs session 1's candidate: s1 believed an `s32` zero could not carry the
 * loop-invariant 0 into a callee-saved register ("cse folds it, score 8") and therefore
 * reached for `s32 *clut; clut = NULL;`.  That row is DISPROVEN.  Re-measured this
 * session on BOTH chassis: `s32 semi; semi = 0;` -> 0 and `s32 semi = 0;` -> 0.  The
 * mode of the local was never the mechanism; having a NAMED LOCAL rather than the
 * literal at the call is.  Deleting the local (literal 0 at the call) is the only thing
 * that costs the 8 insns - measured this session, rejected/no-local-literal-score8.c.
 *
 * `semi` is semantically truthful, not a zero-holder: it is the semi-transparency mode.
 * It is consumed twice, both times as a semi-transparency operand:
 *   - `s.semi` is EnvA offset 0x10, which func_8007352C hands to SetSemiTrans
 *     (src/text1b.c:6790);
 *   - func_8006E480 (src/text1b.c:6106) computes (a0[0] & 0xFE1F) + (a0[1] << 7) + a1,
 *     i.e. a getTPage word whose bits 5-6 (the abr / semi-transparency field, cleared by
 *     the 0xFE1F mask) come from that second argument.
 * Mode 0 = no semi-transparency, consistently, in both consumers.
 *
 * The EnvB typedef is the EnvA layout (src/text1b.c:6710) widened to 0x34 and MUST stay
 * function-local: widening the shared EnvA/S69E18 regressed COMPLETED-C func_8006BB68
 * from 0 to 17 (s1).  0x34 is what puts vars=64 / rect at sp+0x50 (s1).
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
    s32  pad2C, pad30;
} EnvB;
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
