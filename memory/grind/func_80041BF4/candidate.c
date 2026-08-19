/* func_80041BF4 (saTan4FireDisp) - GRIND candidate, s10 (2026-08-19).
 * sandbox --disable all == 3 at 136 insns, frame 88 / vars=24 (target: 135
 * insns, frame 88 / vars=24).  Floor history: s9 11, s8 11, s7 11, s4 11,
 * s3 13, s2 17, s1 22, s0 29, HEAD form 41.
 *
 * [s10] SYNTHESIS session.  The function changed BASIN.  Everything below is
 * measured this session; nothing is inferred.
 *
 * 1. THE 2x2 BASIN MATRIX IS NOW COMPLETE (scores with the trailing `one`
 *    holder present, insns in brackets):
 *        inner\outer      real do-while        goto-spelled
 *        real while        11 [135]             23 [135]
 *        goto-spelled      48 [136]             40 [136]
 *    and the decisive reading is that NOTE_INSN_LOOP_BEG around the INNER
 *    loop is simultaneously REQUIRED and FATAL:
 *      - REQUIRED, because flow.c weights REG_N_REFS by loop_depth and that
 *        weighting is what orders global.c's allocnos; without inner-loop
 *        notes xoff/yoff/r/g/fp_ptr rotate away from target's $s5/$s4/$s7/
 *        $s6/$fp.
 *      - FATAL, because with the notes loop.c's move_movables hoists the
 *        (set reg (symbol_ref "D_800A9A24")) out of the loop, global.c then
 *        never allocates it, and reload rematerialises it into $t0 while
 *        local-alloc hands $a1 to `off` - the entire 11-instruction residual
 *        of the while basin (s3-s9's whole subject).
 *    The n1 form (outer goto, inner while) proves the hoist needs only ONE
 *    enclosing loop: its residual is the while basin's 11 diffs PLUS the
 *    outer/b callee-save swap, and nothing else.
 *
 * 2. THE REQUIRED HALF IS BUYABLE WITHOUT LOOP NOTES.  In the goto basin the
 *    ONLY allocno out of place is xoff: global.c's allocation order is
 *    `... 82 83 79 78 81 [77 76 75] 80 ...` where target's (= the while
 *    basin's) is `... 82 83 79 78 81 80 ... 77 76 75`, i.e. pseudo 80 (xoff)
 *    falls from 6th to last.  s9's banked goto form had already bought yoff
 *    back with a `do { yoff = 0; } while (0);` wrap (43 -> 40); wrapping the
 *    xoff def the same way buys xoff back too: 40 -> 25, and at that point
 *    EVERY REGISTER IN THE FUNCTION MATCHES TARGET - callee-saves, the inner
 *    loop's $a1/$v0/$v1, and the trailing $t0.  Measured: wrap xoff only 43,
 *    wrap yoff only 40, wrap both 25 (either arm works: wrapping both defs in
 *    the THEN arm is also 25).
 *
 * 3. THE REMAINING 22 OF THOSE 25 WERE PURE FRAME OFFSETS.  cc1 prints the
 *    gradient directly (`.frame $sp,N,$31 # vars= X`): the goto basin had
 *    vars=16, target has vars=24.  Declaring the LoadImage rect as
 *    `s16 rect[8]` (only rect[0..3] ever stored) makes vars=24 / frame 88 and
 *    takes the score 25 -> 3.  Frame math from the TARGET BYTES ALONE:
 *    88 = ALIGN8(vars) + ALIGN8(args=24) + gp_regs=40  =>  vars in (16,24];
 *    the locals region writes only the 8 bytes of the rect, so the original
 *    declared a locals object strictly larger than the bytes it writes and no
 *    fully-written locals set can produce frame 88 (a 4-element rect gives
 *    ALIGN8(8+8)+24+40 = 80).  A separate dead pad local is NOT used: prong 2
 *    of the oversized-locals carve-out prefers extending the LIVE object, and
 *    rect is live (its address is passed to LoadImage).  Range: an 8-byte
 *    fixed phantom already exists in this frame, so vars in (16,24] admits
 *    rect[5] through rect[8] byte-identically (ALIGN8) - [8] is chosen.
 *    NOTE: p1/p2 (a separate `s16 pad[4]` / `s16 pad[2]` declared AFTER rect)
 *    also reach vars=24 but score 25, because GCC 2.7.2 assigns locals in
 *    REVERSE declaration order and the pad steals rect's sp+24 slot.
 *
 * 4. WHAT IS LEFT IS EXACTLY ONE INSTRUCTION (3 scored diffs):
 *        idx  target                    ours
 *         82  -                         sll  v0,s1,0x5     (extra, body top)
 *        108  lhu v1,0(s0)              nop
 *        110  sll v0,s1,0x5  (bgez ds)  move v1,v0  (bgez ds)
 *    cse2 substitutes the body's `lhu 0(tbl)` with the value of the test
 *    block's `lh 0(tbl)` (legal: only the low 16 bits survive, the result is
 *    `sh`-stored), dbr then fills the bgez delay slot with that move instead
 *    of the `sll`, the sll is displaced to the top of the body, and the
 *    load-delay `nop` after the `lh` is the net +1.  Target performs no such
 *    substitution: it emits BOTH loads in the test block.  s10 measured four
 *    spellings inert against it - including writing the x read INTO the test
 *    block explicitly (`px = *(u16 *)tbl;` immediately before the `if`, with
 *    and without the preheader copy), which is target's own block layout and
 *    still collapses to `move v1,v0`.  Moving the `tbl += 2` bump above the
 *    reads is 11.
 *
 * UNVETTED CONSTRUCTS IN THIS FORM - none has been through layer-1 or the
 * Judge, and this form must NOT be submitted as-is:
 *   (a) `s16 rect[8]` - oversized-locals carve-out
 *       (.claude/rules/dead-vars-local-array.md, owner ruling 2026-07-13).
 *       The frame-math proof (prong 1) and the live-object preference
 *       (prong 2) are satisfied above; prongs 3-5 (range annotation wording,
 *       lever-exhaustion sign-off, review) are not yet discharged.
 *   (b) the two `do { ... } while (0);` wraps - sanctioned family
 *       (.claude/rules/do-while-zero-exception.md) but the SCOPE sentence of
 *       that rule must be quoted and matched before it is spent; this use is
 *       an allocno-priority (flow.c loop_depth ref weighting) effect, not the
 *       LABEL_OUTSIDE_LOOP_P / reorg.c interaction the role brief names.
 *   (c) `int one = 1;` - constant-holder
 *       (.claude/rules/named-local-fake-exception.md), carried since s4 and
 *       still worth exactly 2.  s10 measured SEVEN more ordinary-C spellings
 *       of the trailing test exactly inert (subtract-and-compare-zero,
 *       xor-and-compare-zero, double-negated !=, switch/case, and naming the
 *       CALL RESULT in a reused `idx` and in a reused `outer`), on top of
 *       s4's three - ten total.  The divergence is reload's spill-register
 *       choice for the rematerialised literal, not the shape of the test.
 */
void func_80041BF4(s32 a0, s32 a1, s32 a2)
{
  s32 *fp_ptr;
  s32 r;
  s32 g;
  s32 b;
  s32 outer;
  s32 xoff;
  s32 yoff;
  s16 *tbl;
  s32 idx;
  int one;
  /* FAKE: oversized locals object (rect[4] is the live RECT; rect[4..7] is the
     unwritten tail), mechanism: mips.c compute_frame_size / get_frame_size -
     ALIGN8(vars)+ALIGN8(args)+gp_regs; target frame 88 = ALIGN8(vars)+24+40
     forces vars in (16,24] while the locals region stores only the 8 rect
     bytes, lever-exhaustion: memory/grind/func_80041BF4/hypotheses.md [s10]
     (frame sweep: separate pad locals, function-scope off, s16/s32/u16 coord
     temps, named width temp - all measured, none reaches vars=24 with rect at
     sp+24).  n.b.! ALIGN8 makes the declared size recoverable only as a
     RANGE: rect[5]..rect[8] are byte-identical here; [8] chosen.  Family:
     .claude/rules/dead-vars-local-array.md OVERSIZED-LOCALS carve-out
     (owner ruling 2026-07-13).  NOT VETTED - do not submit as-is. */
  s16 rect[8];
  extern s32 func_800486FC(void);
  fp_ptr = (s32 *)func_8004153C(1);
  if (fp_ptr == 0) { return; }
  if ((*(((s16 *) fp_ptr) + 4)) != D_800A9A20) { return; }
  if (D_80094E08[*(((s16 *) fp_ptr) + 4)] == 0xFF) { return; }
  r = (a0 << 12) / 255;
  /* FAKE: opaque constant-holder `one` for the trailing `== 1` test, mechanism:
     local-alloc.c block_alloc/find_free_reg - the literal 1 is rematerialized by
     reload into $v1, while a live pseudo carrying it is allocated $t0 as target does,
     lever-exhaustion: memory/grind/func_80041BF4/hypotheses.md [s4] (three ordinary-C
     spellings of the test measured inert at 13) */
  one = 1;
  g = (a1 << 12) / 255;
  b = (a2 << 12) / 255;
  if (func_800486FC()) {
    b = func_8004881C(r, g, b);
    g = b;
    r = b;
  }
  outer = 0;
  oloop:
  {
  if (outer == 0) {
    xoff = -0x140;
    yoff = 0xF0;
  } else {
    /* FAKE: do-while(0) wraps on the else-arm offset defs, mechanism: flow.c
       weights REG_N_REFS by loop_depth, so the wrap lifts each def's weighted
       ref count and restores global.c's allocno order to target's
       (xoff -> $s5, yoff -> $s4); without both wraps this goto-spelled basin
       scores 40/43 instead of 25, lever-exhaustion:
       memory/grind/func_80041BF4/hypotheses.md [s9] (declaration order is
       completely inert in this basin - three permutations + rect-first all
       score 43) and [s10] (arm swap and intra-arm assignment order both
       inert/worse).  Family: .claude/rules/do-while-zero-exception.md - SCOPE
       NOT YET VERIFIED for a ref-weighting effect.  NOT VETTED. */
    do { xoff = 0x80; } while (0);
    do { yoff = 0; } while (0);
  }
  tbl = *(s16 **)((u8 *) D_80094DF0 + (D_80094E08[*(((s16 *) fp_ptr) + 4)] << 2));
  idx = 0;
  goto test;
  again:
  {
    s32 off = idx << 5;
    idx++;
    rect[0] = (*((u16 *) tbl)) + xoff;
    rect[1] = (*(((u16 *) tbl) + 1)) + yoff;
    rect[2] = 0x10;
    rect[3] = 1;
    LoadImage((s32)rect, (s32)((u8 *)&D_800A9A24 + off));
    DrawSync(0);
    tbl += 2;
    func_80048A7C(rect[0], rect[1], 0x10, r, g, b);
  }
  test:
  if (tbl[0] >= 0) goto again;
  outer++;
  }
  if (outer < 2) goto oloop;
  if (func_8003E2A0() == one) { func_8003E120(); }
}
