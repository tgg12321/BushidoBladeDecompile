/* REJECTED (s4-rerun, enumerate, 2026-09-10) - WHY IT IS DEAD:
 * this is the ONLY closure form the Judge's standing constraint permits for the
 * sp+0x44..0x4F hole: a FIRST-DECLARATION `volatile u32 pad[N];`.  Measured, it produces the
 * target's frame SIZE (vars= 64, addiu sp,sp,-0x78, rect at sp+0x50) but the WRONG LAYOUT:
 * GCC 2.7.2 hands out increasing frame offsets in declaration order
 * (tools/gcc-2.7.2/function.c:724, `frame_offset += size;` under #ifndef FRAME_GROWS_DOWNWARD,
 * and MIPS leaves that macro undefined), so the first declaration owns the LOWEST slot and the
 * 0x2C descriptor is displaced from sp+0x18 to sp+0x20.  Every descriptor store then moves.
 * Spliced into src/text1b.c: sandbox --disable all = 45 (target 117 insns, build 119) - more
 * than twice the honest floor of 21.  Do not re-propose a first-declaration pad here.
 */
/* MIGRATION BANNER (s3, 2026-09-10): HEAD/main does NOT carry this body.  src/text1b.c:5948
 * carries `INCLUDE_ASM("asm/funcs", func_8006DD94);` per [[asm-until-matched]]; this file is
 * the in-progress candidate only.  Re-measured s3 by splicing it in with
 * tmp/grind/func_8006DD94/s2/splice.py: sandbox func_8006DD94 --disable all = 21 (117/117,
 * rules_dropped 0).  src/ was reverted to HEAD afterwards.  s3 closed the last two mechanisms
 * that could have produced the sp+0x44..0x4F bytes without declaring an object there:
 * register-pressure spill homes (they land ABOVE the rect - FRAME_GROWS_DOWNWARD is undefined
 * on MIPS, mips.h:1645, so function.c:724 hands out increasing offsets in allocation order and
 * the rect's expand_decl slot always precedes any reload slot) and alignment (BIGGEST_ALIGNMENT
 * is 64 bits, mips.h:1082, and stmt.c:3419 clamps every BLKmode automatic to it, so sp+0x48 is
 * the first legal slot after a descriptor ending at sp+0x44).
 */
/* func_8006DD94 - HONEST BEST FORM, sandbox 21, re-measured this session (s2, permuter).
 *
 * WHY THIS REPLACED THE PREVIOUS candidate.c (2026-09-10, session s2/permuter):
 * the previous candidate.c carried `u16 rects[2][4]` with row 0 never written and never
 * read.  That body was layer-1 FAILed (2026-09-10 06:36) and is now on this function's
 * BANNED CONSTRUCTS list, so it must never be used as a starting chassis again.  This file
 * is the layer-1-clean 0x2C-descriptor body (previously banked as
 * rejected/separate-rect-0x2C-score21.c): no pad, no dead local, no volatile, no FAKE
 * construct, no sanctioned-family claim.  It measures `sandbox func_8006DD94 --disable all`
 * = 21 (117/117, rules_dropped 0) and it is the chassis every future probe should start from.
 *
 * THE ENTIRE RESIDUAL IS ONE STACK SLOT.  The permuter workspace built this session
 * (tmp/grind/func_8006DD94/s2/mkws.sh -> tmp/perm_6dd94) shows base and target are both 117
 * instructions and differ ONLY in the frame: `addiu sp,sp,-112` vs `-120`, the seven register
 * saves, `addiu a1,sp,72` vs `80`, and the four rect `sh` at 72/74/76/78 vs 80/82/84/86.
 * The 0x2C descriptor fills sp+0x18..0x43; with nothing between it and the rect, the rect
 * (BLKmode, BIGGEST_ALIGNMENT) lands at sp+0x48 and cc1 prints `vars= 56`.  ONE additional
 * stack-homed local of ANY size declared before the rect moves it to sp+0x50 and prints
 * `vars= 64` - the target's frame.
 *
 * WHAT IS MEASURED DEAD (do not re-derive - full detail in hypotheses.md):
 *   - a wider shared descriptor type: killed three times; func_8006BB68 byte-matches on main
 *     with the 0x2C shape and zero hole, and so does func_800720FC.
 *   - a sibling that writes into its hole: none exists.  spmap.py mapped every $sp reference
 *     of all seven family members; func_80069F80 and func_8006A1A0 reserve 20 bytes above the
 *     descriptor and touch nothing above it at all - they have no rectangle whatsoever.
 *   - the phantom-frame-slot / HImode trigger, including the in-tree witness's verbatim
 *     spelling: seven probes, all `vars= 56`.
 *   - the permuter: 24k iterations on this chassis; every score-0 attractor it reaches is a
 *     `volatile` unused pad local in the interior position the Judge refused.
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
    volatile u32 pad[2];
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
