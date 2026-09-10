/* PENDING-RULING FORM (func_8006DD94 s5 synthesis, 2026-09-10).
 * NOT submitted as candidate-ready: the trailing-member spelling of the oversized
 * descriptor is on this function's BANNED list (layer-1 FAIL 2026-09-10 05:42, Judge
 * FAIL 05:59/07:42), so a candidate-ready declaring it would be discarded by the
 * driver before review.  s5 returned `ruling-request` naming the grant that the
 * 07:42 ruling did not enumerate: the OVERSIZED-LOCALS carve-out of
 * .claude/rules/dead-vars-local-array.md (owner ruling 2026-07-13), which is in live
 * use on main at src/text1a_post.c:387-400.
 * MEASURED THIS SESSION on the current chassis (HEAD = INCLUDE_ASM at src/text1b.c:5948):
 *   sandbox func_8006DD94 --disable all = 0 (117/117, rules_dropped 0)  [probe B_desc34]
 *   verify-oracle = ok true, build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa,
 *                   build_matches true - the full 606,208-byte executable is
 *                   byte-identical to the original with this body in place.
 * Note the sandbox does NOT strip this form (the struct is live and partially
 * written), so unlike the two-separate-RECT form it needs NO engine allowlist row:
 * if the carve-out is granted this body is directly submittable.
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
    /* FAKE: oversized locals object - `s` is the LIVE descriptor whose address is
       passed to func_8007352C every iteration; pad2C/pad30 are its unwritten tail.
       mechanism: mips.c compute_frame_size / get_frame_size -
       frame = ALIGN8(vars) + ALIGN8(args) + ALIGN8(gp_regs).  Frame-math proof from
       the TARGET BYTES ALONE: target frame is 0x78 with seven callee-saves
       ($s0-$s5,$ra at sp+0x58..0x70 => ALIGN8(28) = 0x20) and a 0x18 outgoing-args
       area (the 5-arg func_8006D808 call stores at sp+0x10), so the locals region is
       0x78 - 0x20 - 0x18 = 0x40 = 64 bytes, while the only stores into it are the
       0x2C-byte descriptor at sp+0x18..0x43 and the 8-byte rect at sp+0x50..0x57
       (52 bytes; sp+0x44..0x4F is never read, written or addressed anywhere in
       asm/funcs/func_8006DD94.s).  The fully-written form (EnvB = 0x2C + u16 rect[4])
       measures vars= 56 => ALIGN8(56)+0x18+0x20 = 0x70 != 0x78, so no fully-written
       locals set can produce the target frame.
       n.b.! the rect's slot is 8-aligned (stmt.c:3419 clamps a BLKmode automatic to
       BIGGEST_ALIGNMENT = 64 bits, mips.h:1082), so the declared descriptor size is
       recoverable only as a RANGE: 0x34 (pad2C, pad30) and 0x38 (pad2C, pad30, pad34)
       are byte-identical (both sandbox 0, s5 probes B_desc34/C_desc38); 0x30 (pad2C
       alone) puts the rect back at sp+0x48 and scores 21 (probe E_desc30).  0x34 is
       chosen as the smallest member of the range.
       Family: .claude/rules/dead-vars-local-array.md OVERSIZED-LOCALS carve-out
       (owner ruling 2026-07-13); prong 2 is satisfied by extending the LIVE object -
       `s`'s address is passed to func_8007352C - rather than adding a dead pad, and
       extending the OTHER live object instead (u16 rect[8], the func_80041BF4
       exemplar's exact shape) is measured wrong here: it reaches the target frame but
       leaves the rect base at sp+0x48 and scores 5 (probe D_rect8).
       In-tree precedent for this carve-out: src/text1a_post.c:387-400 (func_80041BF4,
       `s16 rect[8]`, accepted on main).
       Lever-exhaustion: memory/grind/func_8006DD94/hypotheses.md - 5 sessions,
       1,080 enumerated spellings (973 loop-tail + 65 rect-block + 42 declaration
       orders), 56k permuter iterations over 2 campaigns, 4 class kills (spill homes
       cannot land below the rect, function.c:724; alignment capped, stmt.c:3419;
       no BLKmode keep-temp carrier; declaration-order space has exactly 2 points). */
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
