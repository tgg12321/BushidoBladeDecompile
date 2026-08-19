/* func_80041BF4 - GOTO-BASIN best form, banked s9 (2026-08-19).
 * sandbox --disable all == 40 at 136 insns, frame 80 (target 135 insns, frame 88).
 * This is NOT the candidate (the while/do-while basin candidate.c is 11 @ 135);
 * it is the alternate basin the ledger frontier item 2 names as the winning
 * chassis, banked here so it survives tmp/ cleanup.
 *
 * Why this basin exists: goto-spelling BOTH loops removes every loop note, so
 * loop.c's scan_loop never sees a loop and the (set reg (symbol_ref "D_800A9A24"))
 * insn is never hoisted.  Consequence, measured s7 and re-measured s9: the entire
 * inner-loop body is target's register-for-register (symbol in $a1 reloaded straight
 * into the argument, `off` in $v0) - the exact allocation the while basin cannot
 * reach.  Its whole residual is OUTSIDE the loop body.
 *
 * [s9] Residual of this form, measured:
 *   - frame 80 vs target 88 (8 bytes = 2 words fewer phantom local slots).
 *     Adding two extra LIVE s32 locals does NOT move it (still 80, still 136) -
 *     the phantom-slot lever does not apply here.
 *   - callee-save permutation: this form has xoff=$s8 yoff=$s7 r=$s5 g=$s4
 *     fp_ptr=$s6; target has xoff=$s5 yoff=$s4 r=$s7 g=$s6 fp_ptr=$fp.
 *     DECLARATION ORDER IS COMPLETELY INERT here (three permutations + rect-first
 *     all score exactly 43) - s1/s3's declaration-order rotation levers, which
 *     worked in the while basin, do NOT work in this basin.
 *   - +1 instruction: cse2 merges the loop test's `lh $v0,0($s0)` with the next
 *     iteration's `lhu $v1,0($s0)` into `lh` + `move $v1,$v0`, dbr then fills the
 *     bgez delay slot with that move instead of `sll $v0,$s1,5`, and a load-delay
 *     `nop` after the `lh` is the net extra insn.  Target emits BOTH `lh` and
 *     `lhu` of the same address and puts the `sll` in the delay slot.
 *   - the trailing `== 1` divergence, independent of the loop (see evidence E-s8-5).
 *
 * The ONLY lever found so far that moves this basin: restoring the
 * `do { yoff = 0; } while (0);` wrap in the else arm - 43 -> 40.  It is applied
 * below and is sanctioned by .claude/rules/do-while-zero-exception.md ONLY for a
 * LABEL_OUTSIDE_LOOP_P / reorg.c interaction, which has NOT been demonstrated for
 * this basin - do not submit this form without establishing that.
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
  s16 rect[4];
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
    xoff = 0x80;
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
