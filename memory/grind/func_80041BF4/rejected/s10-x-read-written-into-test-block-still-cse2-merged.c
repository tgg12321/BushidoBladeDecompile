/* s10 REJECTED (exactly inert, 3 at 136).  Frontier attack on the last
 * instruction: write the body's x read INTO the loop-test block explicitly
 * (`px = *(u16 *)tbl;` immediately before `if (tbl[0] >= 0) goto again;`),
 * which is TARGET'S OWN block layout (`lh v0,0(s0); lhu v1,0(s0); bgez`).
 * cse2 still substitutes the value of the test's `lh` for the `lhu` and emits
 * `move v1,v0`.  Variants: with the preheader copy of the read (3), without it
 * (3), and with `px` typed s32 (26 at 137 - the s32 form adds a real
 * sign-extension). */
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
  u16 px;
  int one;
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
    do { xoff = 0x80; } while (0);
    do { yoff = 0; } while (0);
  }
  tbl = *(s16 **)((u8 *) D_80094DF0 + (D_80094E08[*(((s16 *) fp_ptr) + 4)] << 2));
  idx = 0;
  px = *((u16 *) tbl);
  goto test;
  again:
  {
    s32 off = idx << 5;
    idx++;
    rect[0] = px + xoff;
    rect[1] = (*(((u16 *) tbl) + 1)) + yoff;
    rect[2] = 0x10;
    rect[3] = 1;
    LoadImage((s32)rect, (s32)((u8 *)&D_800A9A24 + off));
    DrawSync(0);
    tbl += 2;
    func_80048A7C(rect[0], rect[1], 0x10, r, g, b);
  }
  test:
  px = *((u16 *) tbl);
  if (tbl[0] >= 0) goto again;
  outer++;
  }
  if (outer < 2) goto oloop;
  if (func_8003E2A0() == one) { func_8003E120(); }
}
