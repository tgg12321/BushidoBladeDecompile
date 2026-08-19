/* s10 REJECTED: 59 at 122 insns, vars=8.  Naming the two coordinates in s16
 * block-locals (also s32, also naming the raw u16 reads) collapses 14
 * instructions out of the loop and drops a callee-save register entirely.
 * Measured in the frame hunt (looking for +8 bytes of vars); it moves vars the
 * WRONG way. */
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
    s16 x = (*((u16 *) tbl)) + xoff;
    s16 y = (*(((u16 *) tbl) + 1)) + yoff;
    rect[0] = x;
    rect[1] = y;
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
