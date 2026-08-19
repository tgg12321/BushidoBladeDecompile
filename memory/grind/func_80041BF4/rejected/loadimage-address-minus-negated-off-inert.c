/* saTan4FireDisp (func_80041BF4) - GRIND candidate, s4 (2026-08-19).
 * sandbox --disable all == 11 (s3 banked 13, s2 17, s1 22, s0 29, HEAD form 41).
 * frame 88 / 135 insns, both exactly target's.
 *
 * Inherits every s1/s2/s3 structural finding unchanged (real do-while outer
 * loop; byte-pointer spelling of the D_80094DF0 table load; `tbl += 2` moved
 * to the tail of the loop body).  See the s3 header text preserved in
 * memory/grind/func_80041BF4/evidence.md for those three mechanisms in full.
 *
 * [s4] NEW: the trailing `if (func_8003E2A0() == 1)` test is closed by the
 * opaque constant-holder `one` (found by the s4 permuter campaign as
 * output-70-1, re-measured in the real chassis).  13 -> 11: our `li v1,1 /
 * bne v0,v1` becomes target's `li t0,1 / bne v0,t0`.  THREE ordinary-C
 * spellings of the same test are measured EXACTLY INERT at 13 (result named
 * in a block-local `rc`; named in a function-scope `rc`; Yoda `1 == f()`), so
 * the holder is currently the only measured form - it is a SANCTIONED-FAMILY
 * construct (constant-holder, .claude/rules/named-local-fake-exception.md)
 * carrying a /* FAKE *\/ annotation, and it MUST be re-vetted (layer-1
 * cheat-reviewer + Judge) before any candidate-ready submission.
 *
 * Residual 11 - ONE mechanism, unchanged in kind from s3 but with a CORRECTED
 * attribution (see hypotheses.md [s4]): `off` (pseudo 129) is given hard reg
 * $a1 because it is a direct register source of `(set (reg:SI 5 a1) (plus
 * (reg/v:SI 129) (reg:SI 139)))`.  BOTH allocators route it there
 * independently - local-alloc via qty_phys_sugg (combine_regs,
 * local-alloc.c:1857-1896) and global.c via hard_reg_preferences
 * (global.c:1728/1747, record_one_conflict).  s4 MEASURED that removing the
 * local-alloc route alone does NOT free $a1: two forms that make `off`
 * cross-block (so reg_qty < 0 and combine_regs bails at local-alloc.c:1826)
 * were verified with the instrumented cc1's BB2_SUGG_DEBUG hook to have NO
 * sugg=5 qty left in block 10, and global.c still put `off` in $a1 in both.
 * The remaining escape is therefore a CONFLICT, not a preference: $a1 must be
 * LIVE somewhere inside [born_index, dead_index) of `off` so that
 * find_free_reg's regs_live_at scan (local-alloc.c:2168-2171) excludes it.
 */
void func_80041BF4(s32 a0, s32 a1, s32 a2)
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
  int one;
  s16 rect[4];
  extern s32 func_800486FC(void);
  fp_ptr = (s32 *)func_8004153C(1);
  if (fp_ptr == 0) { return; }
  if ((*(((s16 *) fp_ptr) + 4)) != D_800A9A20) { return; }
  if (D_80094E08[*(((s16 *) fp_ptr) + 4)] == 0xFF) { return; }
  new_var = 5;
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
    { u8 *addr = ((u8 *)(&D_800A9A24)) - (-off);
    LoadImage((s32)rect, (s32)addr); }
    DrawSync(0);
    tbl += 2;
    func_80048A7C(rect[0], rect[1], 0x10, r, g, b);
  }
  outer++;
  } while (outer < 2);
  if (func_8003E2A0() == one) { func_8003E120(); }
}
