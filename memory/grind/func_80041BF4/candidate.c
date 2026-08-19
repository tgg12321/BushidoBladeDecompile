/* saTan4FireDisp - GRIND candidate, s2 (2026-08-19).
 * sandbox --disable all == 17 (s1 banked 22, earlier 29, HEAD form 41).
 * frame 88 / 135 insns, both exactly target's.
 *
 * THE ROTATION IS SOLVED (s1) and the callee-save allocation is target's in
 * full: a2 home->s2 (reused by outer), b->s3, yoff->s4, xoff->s5, g->s6,
 * r->s7, tbl->s0, idx->s1, fp_ptr->fp.  Two structural changes carry it:
 *   1. [s1] outer loop written as a REAL `do { ... } while (outer < 2);`
 *      (was a goto loop). flow.c weights REG_N_REFS by loop_depth, and
 *      global.c allocno priority = floor_log2(refs)*refs/live_length*10000,
 *      so the real loop notes lift outer above b -> target's allocation
 *      order.  [s1] the `do { yoff = 0; } while (0);` wrap in the else arm
 *      must stay REMOVED (under a real outer loop it put yoff's else-def at
 *      depth 3 and flipped the rotation backwards).
 *   2. [s2] the table load is spelled as byte-pointer arithmetic
 *          tbl = *(s16 **)((u8 *) D_80094DF0 + (IDX << 2));
 *      instead of the array index `(s16 *) D_80094DF0[IDX]`.  MECHANISM
 *      (read from dumps, not inferred): with the array-index spelling
 *      expand emits a standalone `(set rN (symbol_ref D_80094DF0))` plus an
 *      explicit `(set rM (plus r_scaled rN))` add, because the scale-4 index
 *      forces the sum out of EXPAND_SUM into a real insn.  loop.c's
 *      scan_loop then records rN as a loop movable and move_movables hoists
 *      it into the outer-loop preheader (loop.c:1631,
 *      `threshold * savings * lifetime >= insn_count`; measured
 *      "Insn 163: regno 124 (life 2), move-insn savings 1  moved to 325"),
 *      which separates the def from its use across a basic-block boundary
 *      so combine can no longer fold the symbol into the MEM.  Result:
 *      `lui t0,%hi; addiu t0,t0,%lo; addu v0,v0,t0; lw s0,0(v0)`.
 *      With the byte-pointer spelling the address tree has no MULT node, so
 *      expand keeps it as an un-emitted sum and the load stays
 *      `(mem (plus (reg) (symbol_ref)))` -> combine/final print target's
 *      `lui at,%hi; addu at,at,v0; lw s0,%lo(at)`.  The matched sibling
 *      func_80041AC8 in the same TU gets the same target form from the
 *      array-index spelling only because it is NOT in a loop: nothing
 *      hoists, so combine folds it in-block.  22 -> 17.
 *
 * Residual 17 - ONE family, the same LICM mechanism one level in:
 *   loop.c still hoists three inner-loop invariants out of the inner loop
 *   and then out of the outer loop (measured, .loop dump):
 *     regno 136 = (const_int 16)   -> rect[2] = 0x10
 *     regno 137 = (const_int 1)    -> rect[3] = 1
 *     regno 139 = symbol_ref D_800A9A24  -> the LoadImage source address
 *   reload rematerializes all three at their uses (they carry REG_EQUIV),
 *   so the instruction COUNT is unchanged (135/135) but the scratch chosen
 *   is $t0 where target uses $v0/$a1, and the LoadImage arg-setup block
 *   lands mid-body instead of at the top of the loop body.  Concretely:
 *     target  addiu a0,sp,24 / lui a1,%hi / addiu a1,a1,%lo / addu a1,v0,a1
 *             ... li v0,16 / sh v0,28(sp) / li v0,1 / sh v0,30(sp)
 *             lhu v1,2(s0) ... sll v0,s1,5      (loop-bottom delay slot)
 *     ours    ... addiu a0,sp,24 / lui t0 / addiu t0 / addu a1,a1,t0
 *             li t0,16 / sh t0,28(sp) / li t0,1 / sh t0,30(sp)
 *             lhu v0,2(s0) ... sll a1,s1,5
 *   plus the two riders `li v1,1 / bne v0,v1` (target `li t0,1 / bne v0,t0`)
 *   and `sll a1,s1,5` (target `sll v0,s1,5`).
 *   [s2] MEASURED INERT against this residual (all 135 insns, all still 17):
 *   inner-body statement re-association (rect[2]/rect[3] before the
 *   coordinate stores is WORSE - 46/139), naming the LoadImage source in a
 *   local, `(s32)&D_800A9A24 + off` without the u8* cast, moving `tbl += 2`,
 *   folding `idx++` into the shift, assigning the computed address back into
 *   `off`, commuting both coordinate adds, naming either coordinate read in
 *   a block-local, and swapping the rect[2]/rect[3] order.
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
    tbl += 2;
    rect[2] = 0x10;
    rect[3] = 1;
    LoadImage((s32)rect, (s32)((u8 *)&D_800A9A24 + off));
    DrawSync(0);
    func_80048A7C(rect[0], rect[1], 0x10, r, g, b);
  }
  outer++;
  } while (outer < 2);
  if (func_8003E2A0() == 1) { func_8003E120(); }
}
