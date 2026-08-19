/* func_80041BF4 (saTan4FireDisp) - GRIND candidate, s11 (2026-08-19).
 * sandbox --disable all == 0 at 135 insns (target: 135), frame 88 / vars=24.
 * Floor history: s10 3, s9 11, s8 11, s7 11, s6 11, s5 11, s4 11, s3 13,
 * s2 17, s1 22, s0 29, HEAD form 41.  THIS IS A BYTE MATCH.
 *
 * [s11] STRUCTURAL session.  The last instruction fell to a pass-attributed
 * cse.c mechanism, not to a spelling search.
 *
 * 1. THE DIVERGENCE WAS cse1, NOT cse2.  Reading tools/gcc-2.7.2/cse.c
 *    cse_end_of_basic_block (lines 8008-8190) explains s10's residual exactly:
 *    the block that starts at the `test:` label ends in a CONDITIONAL jump to
 *    `again:`, and cse EXTENDS the block across that branch when three things
 *    hold - flag_cse_follow_jumps (on at -O2), LABEL_NUSES(again) == 1, and
 *    the insn found by scanning back from `again:` (skipping NOTEs that are
 *    not LOOP_END/SETJMP and CODE_LABELs with zero uses) is a BARRIER.  Our
 *    entry `goto test;` supplies exactly that barrier.  So cse walked
 *    test-then-body, recorded `(mem:HI (reg tbl))` from the loop test's `lh`,
 *    and substituted the body's `lhu` with it.  Confirmed in the dumps: the
 *    substitution is already present in text1a_post.cse (insn 208 uses
 *    `(subreg:SI (reg:HI 142) 0)`, reg 142 being the test block's load), while
 *    text1a_post.jump still has the body's own load in reg 126.
 *
 * 2. THE SUBSTITUTION IS FATAL WHEREVER THE COPY LANDS, because the copy
 *    DEPENDS on the `lh`.  Measured this session: with the x read moved into
 *    the test block after the condition load the merge becomes `andi v1,v0,
 *    0xffff` (still 136 - the andi takes the bgez delay slot and the load
 *    delay after `lh` stays a nop).  Target fills that load delay with an
 *    INDEPENDENT insn - its own second load `lhu v1,0(s0)` - so any form that
 *    reaches the test block through a value derived from the `lh` is one insn
 *    short by construction.
 *
 * 3. THE FIX IS TO LET combine REBUILD THE SIGN-EXTENDING LOAD.  Read the
 *    halfword ONCE, unsigned, in the loop condition, and test its sign with a
 *    cast:
 *        test:
 *        x = *(u16 *) tbl;
 *        if ((s16) x >= 0) goto again;
 *    cse now has a single load to record (nothing to merge), and combine.c
 *    folds `(ashiftrt (ashift (zero_extend (mem:HI ...)) 16) 16)` back into
 *    `(sign_extend (mem:HI ...))`, i.e. it MANUFACTURES the second memory
 *    reference AFTER cse can no longer unify the two.  The emitted test block
 *    becomes target's `lh v0,0(s0); lhu v1,0(s0); bgez v0,...; sll v0,s1,5` -
 *    the lhu fills the load delay and the sll fills the branch delay.  135
 *    instructions, score 0.  This is also ordinary, readable C: x is a real
 *    consumed value (it is rect[0]'s x coordinate) and the loop terminator is
 *    genuinely the sign bit of that same halfword.
 *
 * 4. A SECOND, INDEPENDENT WAY TO KILL THE MERGE was measured and is banked
 *    as a rejected form (it does not reach 0 on its own): wrapping the
 *    preheader's `goto test;` in `do { ... } while (0);` puts a
 *    NOTE_INSN_LOOP_END immediately before `again:`, which makes cse's
 *    backward scan stop on the note instead of the barrier, so the block is
 *    never extended.  Useful as a general lever for this family; recorded in
 *    hypotheses.md.
 *
 * 5. THE THREE FAKE CONSTRUCTS THAT SURVIVE were each re-measured against
 *    ordinary-C substitutes IN THE NEW BASIN this session (the old
 *    measurements were chassis-relative and were not trusted):
 *      - `int one = 1;`  removing it costs 12; six ordinary spellings 9..20.
 *      - the single do-while(0) wrap  removing it costs 18; six natural
 *        geometries 18..33.  s10 needed TWO wraps; this basin needs ONE.
 *      - `s16 rect[8]`  rect[4] costs 22; rect[5..8] are all 0 (the range),
 *        rect[9] costs 22.
 *    Full per-variant numbers are in hypotheses.md [s11] and the self-vet.
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
  s32 x;
  /* FAKE: opaque constant-holder for the trailing `== 1` test, mechanism:
     local-alloc.c block_alloc / find_free_reg - the bare literal is
     rematerialized by reload into $v1, while a live pseudo carrying it is
     allocated $t0 exactly as target does, lever-exhaustion:
     memory/grind/func_80041BF4/hypotheses.md [s4] and [s10] (ten ordinary-C
     spellings of the test measured inert) plus [s11] (six more re-measured in
     the NEW basin: subtract-compare-zero 12, double-negated 12, switch/case 12,
     named call result in a fresh local 12, in reused `x` 12, in reused `idx`
     20, in reused `outer` 9 - none reaches 0).  Family:
     .claude/rules/named-local-fake-exception.md (owner ruling 2026-07-01). */
  int one;
  /* FAKE: oversized locals object - rect[0..3] is the live LoadImage RECT and
     rect[4..7] is the unwritten tail, mechanism: mips.c compute_frame_size /
     get_frame_size - ALIGN8(vars) + ALIGN8(args) + gp_regs.  Frame-math proof
     from the TARGET BYTES ALONE: target frame is 88 with ten callee-saves
     ($s0-$s7,$fp,$ra = 40 bytes) and a 24-byte outgoing-args area (the 6-arg
     func_80048A7C call), so the locals region is 88-40-24 = 24 bytes while the
     only stores into it are the 8 bytes of the RECT at sp+0x18.  The
     fully-written form (rect[4]) measures frame 80, so no fully-written locals
     set can produce target's 88.  n.b.! ALIGN8 plus this frame's fixed 8-byte
     phantom slot make the declared size recoverable only as a RANGE: rect[5],
     rect[6], rect[7] and rect[8] are all byte-identical here (all measured 0
     this session; rect[4] 22 and rect[9] 22) - [8] is chosen.  Family:
     .claude/rules/dead-vars-local-array.md OVERSIZED-LOCALS carve-out (owner
     ruling 2026-07-13); prong 2 is satisfied by extending the LIVE object -
     rect's address is passed to LoadImage - rather than adding a dead pad.
     Lever-exhaustion: memory/grind/func_80041BF4/hypotheses.md [s10] frame
     sweep + [s11] re-measured in the new basin. */
  s16 rect[8];
  extern s32 func_800486FC(void);
  fp_ptr = (s32 *)func_8004153C(1);
  if (fp_ptr == 0) { return; }
  if ((*(((s16 *) fp_ptr) + 4)) != D_800A9A20) { return; }
  if (D_80094E08[*(((s16 *) fp_ptr) + 4)] == 0xFF) { return; }
  r = (a0 << 12) / 255;
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
    /* FAKE: single-level do-while(0) wrap on the else-arm offset defs;
       observed effect - it lifts xoff's and yoff's weighted reference counts
       (flow.c weights REG_N_REFS by loop_depth) so global.c's allocno order
       matches target's and the two offsets land in $s5/$s4; without the wrap
       this form scores 18 instead of 0.  Natural geometry was tried first:
       arm swap (20), defs hoisted above the if (20), ternary (33), block-local
       and nested-block declaration scopes (18), duplicated real statement into
       both arms (18), plain assignment (18) - all measured this session.
       Family: .claude/rules/do-while-zero-exception.md (owner ruling
       2026-07-06, sanctioned for ANY codegen effect incl. register
       allocation). */
    do { xoff = 0x80; yoff = 0; } while (0);
  }
  tbl = *(s16 **)((u8 *) D_80094DF0 + (D_80094E08[*(((s16 *) fp_ptr) + 4)] << 2));
  idx = 0;
  goto test;
  again:
  {
    s32 off = idx << 5;
    idx++;
    rect[0] = x + xoff;
    rect[1] = (*(((u16 *) tbl) + 1)) + yoff;
    rect[2] = 0x10;
    rect[3] = 1;
    LoadImage((s32)rect, (s32)((u8 *)&D_800A9A24 + off));
    DrawSync(0);
    tbl += 2;
    func_80048A7C(rect[0], rect[1], 0x10, r, g, b);
  }
  test:
  x = *(u16 *) tbl;
  if ((s16) x >= 0) goto again;
  outer++;
  }
  if (outer < 2) goto oloop;
  if (func_8003E2A0() == one) { func_8003E120(); }
}
