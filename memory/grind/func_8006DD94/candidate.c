/* func_8006DD94 - BEST HONEST FORM as of s4.  `sandbox --disable all` = 21 (117/117,
 * rules_dropped 0).  This body contains NO pad, NO dead local, NO FAKE construct and no
 * sanctioned-family claim: it is the plain 0x2C EnvA-shaped descriptor plus a real
 * `u16 rect[4]`, and every one of its stores is consumed.
 *
 * WHY THIS REPLACED THE OLD candidate.c (s4, 2026-09-10): the previous candidate.c was
 * s3's merged S_6DD94 frame-block struct with three unwritten INTERIOR words.  The Judge
 * FAILed that body at FINAL CALL (decisions.md 2026-09-10 05:59) and it is banked at
 * rejected/merged-struct-judge-fail-0559.c.  s4 then measured the honest cost of the whole
 * pad family: the equivalent dead-array spelling (`u16 rect0[4];` before the real rect)
 * produces the target's frame EXACTLY - cc1 prints `# vars= 64, regs= 7/0, args= 24` and
 * the rect stores land at sp+0x50..0x56 - yet `sandbox --disable all` still reports 21,
 * because the engine strips the unwritten array before scoring.  So the honest floor of
 * this function is 21, and the s3 "0" was a false 0 that only appeared because a struct
 * member evades the pad allowlist.  Do not chase pads: they are mechanically inert here.
 *
 * THE ENTIRE 21-INSN RESIDUAL IS ONE 8-BYTE FRAME DISPLACEMENT.  Target: descriptor at
 * sp+0x18 (0x2C bytes, ending 0x44), 12 untouched bytes sp+0x44..0x4F, `u16 rect[4]` at
 * sp+0x50, frame 120.  This body: descriptor at sp+0x18, rect at sp+0x48, frame 112.
 * Every differing insn is that displacement or its knock-on offsets.
 *
 * Byte-confirmed and NOT to be re-derived (s1/s2/s3): the 3-iteration loop shape, the u8
 * colour triple written through the chained assignment, the s16 counter, the rect store
 * order [2],[0],[1],[3], and the named `semi` local (deleting it costs 8 more insns -
 * rejected/no-local-literal-score8.c).
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
