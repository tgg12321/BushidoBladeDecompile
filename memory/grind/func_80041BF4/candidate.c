/* saTan4FireDisp - GRIND candidate, s3 (2026-08-19).
 * sandbox --disable all == 13 (s2 banked 17, s1 22, s0 29, HEAD form 41).
 * frame 88 / 135 insns, both exactly target's.
 *
 * THE ROTATION IS SOLVED (s1) and the callee-save allocation is target's in
 * full: a2 home->s2 (reused by outer), b->s3, yoff->s4, xoff->s5, g->s6,
 * r->s7, tbl->s0, idx->s1, fp_ptr->fp.  THE INNER-LOOP SCHEDULE IS SOLVED
 * (s3): every insn of the loop body is now in target's exact order.
 * Three structural changes carry all of that:
 *   1. [s1] outer loop written as a REAL `do { ... } while (outer < 2);`
 *      (was a goto loop). flow.c weights REG_N_REFS by loop_depth, and
 *      global.c allocno priority = floor_log2(refs)*refs/live_length*10000,
 *      so the real loop notes lift outer above b -> target's allocation
 *      order.  [s1] the `do { yoff = 0; } while (0);` wrap in the else arm
 *      must stay REMOVED (under a real outer loop it put yoff's else-def at
 *      depth 3 and flipped the rotation backwards).
 *   2. [s2] the table load is spelled as byte-pointer arithmetic
 *          tbl = *(s16 **)((u8 *) D_80094DF0 + (IDX << 2));
 *      instead of the array index `(s16 *) D_80094DF0[IDX]`.  The scale-4
 *      index forces the address sum out of EXPAND_SUM into a real add insn,
 *      leaving a standalone `(set rN (symbol_ref D_80094DF0))` that loop.c
 *      records as a movable and move_movables hoists into the outer-loop
 *      preheader (loop.c:1631) -- which separates the def from its MEM use
 *      across a basic-block boundary so combine can no longer fold the
 *      symbol into the load.  The byte-pointer spelling has no MULT node in
 *      the address tree, expand keeps the sum un-emitted, and the load
 *      prints target's `lui at,%hi; addu at,at,v0; lw s0,%lo(at)`.  22 -> 17.
 *   3. [s3] `tbl += 2;` moved from the middle of the loop body (between the
 *      rect[1] store and the rect[2] constant store) to AFTER `DrawSync(0)`,
 *      i.e. into the tail of the body just before the func_80048A7C call.
 *      MECHANISM (read from .sched/.greg, not inferred): the mid-body
 *      `(set tbl (plus tbl 4))` carried a REG_DEP_ANTI against the y-coordinate
 *      load `(set r133 (mem (plus tbl 2)))` that sat between the two halves of
 *      the LoadImage argument-setup chain.  sched1 could not lift the
 *      `addiu a0,sp,24 / la a1 / addu a1` block above that anti-dependence, so
 *      the arg setup landed mid-body while target has it at the very top of the
 *      body.  With the pointer bump moved past DrawSync the anti-dependence is
 *      out of the way, sched1 issues the whole arg-setup block first, and the
 *      body order becomes byte-for-byte target's.  17 -> 13.
 *
 * Residual 13 - ONE mechanism, entirely in local-alloc (measured, .lreg):
 *   `off` is pseudo 129, block-local to the loop body (block 10), born at the
 *   ashift and dead at `(set (reg:SI 5 a1) (plus (reg/v:SI 129) (reg:SI 139)))`.
 *   local-alloc.c:1240-1300 walks the source operands of that insn and calls
 *   combine_regs(operand, hard a1); combine_regs (local-alloc.c:1857-1880)
 *   sees a HARD setreg and records a1 in qty_phys_sugg[qty(129)], then
 *   find_free_reg's just_try_suggested pass honours it -> "Register 129 in 5".
 *   That steals a1, so reload -- which must rematerialize the three
 *   loop-hoisted invariants 136 (const 16), 137 (const 1) and 139
 *   (symbol_ref D_800A9A24), none of which global.c could allocate -- falls
 *   through to $t0 for all three.  Target instead has off in $v0 and the
 *   rematerialised symbol in $a1:
 *     target  lui a1,%hi / addiu a1,a1,%lo / addu a1,v0,a1
 *             ... li v0,16 / sh v0,28(sp) / li v0,1 / sh v0,30(sp)
 *             lhu v1,2(s0) ... addu v1,v1,s4 / sh v1,26(sp)
 *             sll v0,s1,5 (loop-bottom delay slot) ... li t0,1 / bne v0,t0
 *     ours    lui t0 / addiu t0 / addu a1,a1,t0
 *             li t0,16 / sh t0,28(sp) / li t0,1 / sh t0,30(sp)
 *             lhu v0,2(s0) ... addu v0,v0,s4 / sh v0,26(sp)
 *             sll a1,s1,5 ... li v1,1 / bne v0,v1
 *   The y-temp v0-vs-v1 rename and the trailing li v1,1 are downstream of the
 *   same a1 theft; every insn is otherwise identical and in identical order.
 *   The whole residual collapses if `off` can be denied hard reg a1.
 *
 *   [s3] MEASURED INERT against this residual (all 13, all 135 insns):
 *   every declaration permutation of the twelve locals (12 forms, s3 round 1
 *   -- declaration order is completely dead for this function); naming the
 *   source address in a block-local; commuting the address add; `idx * 32`
 *   and a literal `idx << 5` with new_var deleted; swapping rect[2]/rect[3];
 *   `&rect[0]` for `rect`; dropping the `sent` variable; `*tbl` for `tbl[0]`;
 *   u16* tbl with tbl[0]/tbl[1] reads; `off` hoisted to function scope;
 *   moving `tbl += 2` further (before DrawSync / after func_80048A7C).
 *   MEASURED WORSE: rect[1] stored before rect[0] (45/139), DrawSync before
 *   LoadImage (73/136), idx++ at the body end (16), coordinates named in
 *   block-locals (15), rect[2]/rect[3] after the LoadImage args (15), the
 *   `off` expression inlined into the call (15).
 */
void saTan4FireDisp(s32 a0, s32 a1, s32 a2)
{
  s32 *fp_ptr;
  s32 r;
  s32 g;
  s32 b;
  s32 outer;
  int new_var;
  s32 xoff;
  s32 yoff;
  s16 *tbl;
  s32 idx;
  s32 sent;
  s16 rect[4];
  extern s32 func_800486FC(void);
  fp_ptr = (s32 *)func_8004153C(1);
  if (fp_ptr == 0) { return; }
  if ((*(((s16 *) fp_ptr) + 4)) != D_800A9A20) { return; }
  if (D_80094E08[*(((s16 *) fp_ptr) + 4)] == 0xFF) { return; }
  new_var = 5;
  r = (a0 << 12) / 255;
  g = (a1 << 12) / 255;
  b = (a2 << 12) / 255;
  if (func_800486FC()) {
    b = func_8004881C(r, g, b);
    g = b;
    r = b;
  }
  outer = 0;
  do {
  if (outer == 0) {
    xoff = -0x140;
    yoff = 0xF0;
  } else {
    xoff = 0x80;
    yoff = 0;
  }
  tbl = *(s16 **)((u8 *) D_80094DF0 + (D_80094E08[*(((s16 *) fp_ptr) + 4)] << 2));
  idx = 0;
  while ((sent = tbl[0]) >= 0)
  {
    s32 off = idx << new_var;
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
  outer++;
  } while (outer < 2);
  if (func_8003E2A0() == 1) { func_8003E120(); }
}
