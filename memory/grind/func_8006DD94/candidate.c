/* s6 UPDATE (synthesis, 2026-09-10; dispatched as "session 5"): body UNCHANGED - this is still
 * the layer-1-clean, FAKE-free 0x2C-descriptor chassis, re-measured `sandbox func_8006DD94
 * --disable all` = 21 (117/117, rules_dropped 0).  MIGRATION STATUS: HEAD/main carries
 * `INCLUDE_ASM("asm/funcs", func_8006DD94);` at src/text1b.c:5948; this file is the in-progress
 * candidate only, spliced and reverted again this session.
 * BUT THIS IS NO LONGER THE BEST FORM ON FILE.  s6 found the frozen family that the 2026-09-10
 * 07:42 Judge ruling did not enumerate - the OVERSIZED-LOCALS carve-out of
 * .claude/rules/dead-vars-local-array.md:39-95 (owner ruling 2026-07-13), in live use on main at
 * src/text1a_post.c:387-400 (func_80041BF4, `s16 rect[8]`) - and measured that all five of its
 * prerequisites hold here when the LIVE descriptor is the extended object:
 *   prong 1  target frame 0x78 - 0x20 saves - 0x18 args = 0x40 = 64-byte locals region vs this
 *            body's vars= 56 (the fully-written form yields frame 0x70 != 0x78);
 *   prong 2  extending the descriptor moves the rect to sp+0x50 (target); extending the other
 *            live object instead (u16 rect[8]) reaches the frame but keeps the rect at sp+0x48,
 *            score 5;
 *   prong 3  range measured: descriptor 0x34 -> sandbox 0, 0x38 -> sandbox 0 (byte-identical),
 *            0x30 -> sandbox 21;
 *   prong 4  this ledger (1,080 spellings, 56k permuter iterations, 4 class kills);
 *   prong 5  the Judge.
 * The 0x34 form measures sandbox 0 AND verify-oracle build_matches true (SHA1
 * 62efab4f73f992798c43e8c730aa43baa10bb4fa) on this chassis, and the sandbox does NOT strip it,
 * so it needs no engine allowlist row.  It is held, FAKE-annotated, at
 * memory/grind/func_8006DD94/pending-ruling-oversized-descriptor-0x34-oracle-match.c and was NOT
 * submitted: the trailing-member spelling is on this function's BANNED list, so s6 returned
 * `ruling-request` naming the grant that supersedes the ban.  Do not re-run spelling search.
 */
/* s5 UPDATE (enumerate, 2026-09-10; dispatched as "session 4"): body UNCHANGED - this is still
 * the layer-1-clean, FAKE-free 0x2C-descriptor chassis and it is still the best honest form.
 * MIGRATION STATUS: HEAD/main carries `INCLUDE_ASM("asm/funcs", func_8006DD94);` at
 * src/text1b.c:5948; this file is the in-progress candidate only, spliced and reverted again
 * this session.  Re-measured s5: `sandbox func_8006DD94 --disable all` = 21 (117/117,
 * rules_dropped 0).
 * s5 spent the enumerate modality in full and banked three results:
 *   (1) 973 spellings of the render loop's linear tail (fully-named form, inline axis x
 *       def-before-use ordering axis) - best 25, ZERO at the floor.  The best of them is this
 *       body with the loop temp at BLOCK scope instead of function-top scope, which costs 4
 *       (rejected/enum-loop-tail-best-blockscope-hv-score25.c).
 *   (2) 42 declaration orders (every placement of the two stack-homed objects among the seven
 *       top-level declarations) - perfectly bimodal, {21: 21, 42: 21}: descriptor-first is 21,
 *       rect-first is 42, and the five scalar declarations change nothing in any position
 *       (stmt.c:3357-3364 - a register-eligible automatic has no frame footprint).  The target
 *       needs a THIRD stack-homed object between the two, which is the Judge-FAILed question.
 *   (3) the last live frontier item is empty: a BLKmode keep-temp is the only slot mechanism
 *       ordered BEFORE a later expand_decl, and every callee here returns a scalar or void, so
 *       no honest struct-valued expression exists to create one.
 * Do NOT re-run spelling search on this chassis: 1,080 spellings are now measured across three
 * exhaustive sweeps and none is below 21.
 */
/* s4-RERUN UPDATE (enumerate, 2026-09-10): body UNCHANGED.  The previous s4 outcome was
 * DISCARDED by the driver for an invalid predicate_cite path; its findings were re-measured
 * and re-banked here.  MIGRATION STATUS: HEAD/main still carries
 * `INCLUDE_ASM("asm/funcs", func_8006DD94);` at src/text1b.c:5948 - this file is the
 * in-progress candidate only, spliced and reverted again this session.
 * Re-measured this session: sandbox func_8006DD94 --disable all = 21 (117/117, rules_dropped 0).
 * NEW this session: (a) the frozen pad family's FIRST-DECLARATION requirement is measured
 * incompatible with the target layout - `volatile u32 pad[2];` declared first gives the exact
 * target frame size but moves the descriptor from sp+0x18 to sp+0x20, sandbox 45
 * (rejected/first-decl-volatile-pad-displaces-descriptor-to-0x20-score45.c); only an INTERIOR
 * reservation reproduces sp+0x18 descriptor / sp+0x50 rect.  (b) the mandated systematic
 * spelling sweep of the rect block (65 spellings, swap axis included) has best 21, one at the
 * floor - the residual is not in that block's spelling space.
 */
/* s4 UPDATE (enumerate, 2026-09-10): this file is UNCHANGED as the layer-1-clean chassis
 * (sandbox 21, vars= 56, rect at sp+0x48, 112 body insns).  MIGRATION STATUS: HEAD/main
 * still carries `INCLUDE_ASM("asm/funcs", func_8006DD94);` at src/text1b.c:5948 - this
 * file is the in-progress candidate only and was spliced/reverted again this session.
 * s4 measured the residual to be exactly `vars= 56` vs the target's `vars= 64`, enumerated
 * 41 spellings for a construct that closes that 8-byte gap without adding an instruction,
 * and found that all six hits reserve the bytes with an object no instruction touches.
 * The byte-proven form is rejected/two-separate-rect-arrays-oracle-match-sandbox-21.c
 * (verify-oracle build_matches true, re-verified s4); it is held there pending a Judge
 * ruling on its construct class, which s4 requested.  A byte-neutral variant of THIS
 * chassis with the rect in a trailing nested block is tmp/grind/func_8006DD94/s4/b1_nested.c
 * - use it if a future probe needs a stack temp to be allocated below the rect.
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
