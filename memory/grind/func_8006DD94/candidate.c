/* func_8006DD94 - BEST FORM, session s3 (recon).  sandbox --disable all = 0 (117/117,
 * rules_dropped 0) AND verify-oracle build_sha1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa,
 * both measured THIS session with this exact body in src/text1b.c.
 *
 * RULING PENDING - do not submit this body as candidate-ready until the pipeline rules on
 * the three unwritten INTERIOR words (sp44/sp48/sp4C).  See the s3 ruling_question.
 *
 * What changed vs the layer-1-FAILed s2 body: s2 declared a function-local `EnvB` that was
 * the 0x2C EnvA layout plus two TRAILING unwritten words, with the rect as a separate
 * `u16 rect[4]` local.  Layer-1 FAILed exactly those two trailing words (banned construct).
 * This body instead models the function's whole stack-locals block as ONE struct - the
 * descriptor sub-range at offset 0 (passed to func_8007352C as &s.header, the same address
 * the s2 body passed) and the screen rect as a real member at struct offset 0x38 => sp+0x50.
 * The three unknown words now sit BETWEEN two used members instead of at the end.
 *
 * Why any filler is needed at all (measured, s3): GCC 2.7.2 8-aligns every stack slot, so a
 * 0x2C descriptor followed by a separate `u16 rect[4]` puts the rect at sp+0x48 and the frame
 * at 112; the target has the rect at sp+0x50 and the frame at 120.  The whole residual is
 * that one 8-byte displacement (score 21 without it - rejected/separate-rect-0x2C-score21.c).
 * s3 swept every scalar spelling that could plausibly leave a phantom slot in that hole
 * (s16 pair feeding an HImode bitwise expr, s16 scalar, s64, declaration reordering,
 * scalars-before-rect): NONE of them reserves a byte.  Only a declared aggregate does, and a
 * WRITTEN one materialises stores the target does not have.
 *
 * In-file precedent for unwritten interior members in already-accepted C:
 *   - `S_69AE4` (src/text1b.c:5424-5426), the stack-block struct of COMPLETED-C
 *     func_80069AE4, whose members sp24/sp38/sp3C are never written and are followed by the
 *     written member sp40; its descriptor is likewise passed as `&s.sp18`
 *     (src/text1b.c:5498).  The sp<offset> member-naming convention here is taken from it.
 *   - `EnvA` (src/text1b.c:6654-6669), the descriptor type of COMPLETED-C func_8007352C,
 *     which ships pad0C/pad20/pad24 - interior members no BB2 caller writes.
 *
 * Everything else is inherited byte-confirmed from s1/s2 and must NOT be re-derived: the
 * 3-iteration loop shape, the u8 colour triple written via the chained assignment, the s16
 * counter, the rect store order, and the named `semi` local (deleting it costs 8 insns -
 * rejected/no-local-literal-score8.c).
 */
/* BEGIN func_8006DD94 */
typedef struct S_6DD94 {
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
    s32  sp44, sp48, sp4C;
    u16  rect[4];
} S_6DD94;
extern s32 D_800A374C;
extern void func_8006D808(s32 *, s32 *, s32 *, s32, s32);
void func_8006DD94(s32 *arg0) {
    S_6DD94 s;
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
        arg0[5] = func_8007352C((s32)&s.header);
        SetDrawMode(arg0[7], 1, 0, func_8006E480((s32)s.header, semi), 0);
        AddPrim(D_800A374C + 0x28, arg0[7]);
        arg0[7] += 0xC;
    }

    func_8006D808(&arg0[5], &arg0[7], q, s.ot_idx, -1);

    s.rect[2] = 0x96;
    s.rect[0] = 0xF5;
    s.rect[1] = 0x25;
    s.rect[3] = 1;
    func_80069898((GameObj *)arg0, s.rect, 0x11);
}
/* END func_8006DD94 */
