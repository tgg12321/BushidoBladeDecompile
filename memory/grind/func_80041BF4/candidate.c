/* saTan4FireDisp (func_80041BF4) - GRIND candidate, s8 (2026-08-19).
 * sandbox --disable all == 11 at 135/135 insns, frame 88 (floor unchanged;
 * s7 11, s4 11, s3 13, s2 17, s1 22, s0 29, HEAD form 41).
 *
 * [s8] REDERIVE session. The floor did not move, but the FORM is strictly
 * simpler than s4-s7's: two constructs carried since s0 are now MEASURED
 * EXACTLY INERT and have been deleted.
 *   - `int new_var; new_var = 5;` - an UNANNOTATED opaque constant-holder used
 *     as the shift amount in `idx << new_var` - is worth ZERO. Literal
 *     `idx << 5` scores 11 at 135 insns; `idx * 32` also scores 11 at 135.
 *     It was carried on the assumption it was load-bearing. It is not, and it
 *     was a layer-1 liability (a constant-holder with no FAKE annotation and
 *     no lever-exhaustion record). Removed.
 *   - `s32 sent;` with `while ((sent = tbl[0]) >= 0)` is worth ZERO: plain
 *     `while (tbl[0] >= 0)` scores 11 at 135 insns. Removed.
 * The ONLY remaining non-ordinary construct is the `one` constant-holder for
 * the trailing `func_8003E2A0() == 1` test. s8 re-measured it as still worth
 * exactly 2 (11 with, 13 without) and, independently, still worth exactly 2 in
 * the goto-spelled chassis (43 with, 45 without) - so it is NOT a downstream
 * artifact of the loop residual, it is a genuinely separate 2-instruction
 * divergence ($v1 vs target $t0 holding the literal 1). It remains UNVETTED
 * and its FAKE prerequisites are still unmet (the ladder is not spent -
 * synthesis is untried). Do not spend it.
 *
 * [s8] Frontier item 1 - "rederive the LoadImage source address so that no
 * bare (set reg (symbol_ref)) insn is born at all" - is KILLED, and killed
 * mechanically: EVERY spelling that turns the address into a SCALED
 * pointer/array access makes GCC 2.7.2 cc1 SEGFAULT (exit 139) on this
 * translation unit. Measured on three spellings:
 *     (u16 *)&D_800A9A24 + (idx << 4)
 *     ((u8 (*)[32])&D_800A9A24)[idx - 1]
 *     &((u16 (*)[16])&D_800A9A24)[idx - 1][0]
 * Flag bisection on the identical preprocessed input: -fno-strength-reduce is
 * the ONLY flag that avoids the crash; -fno-schedule-insns, -fno-schedule-insns2
 * and -fno-rerun-cse-after-loop all still segfault. The crash is therefore in
 * loop.c strength reduction, and since CC_FLAGS is frozen the array /
 * typed-stride rederive family is UNAVAILABLE, not merely worse. It is also
 * unwanted on the evidence: target strength-reduces nothing here - it re-emits
 * `sll $v0,$s1,5` every iteration and re-materialises the symbol every
 * iteration - which is the signature of an UNSCALED byte-pointer add, exactly
 * the form kept below. A PsyQ-idiomatic `struct { s16 x,y,w,h; }` rect (with
 * `&rect` or `&rect.x`) is also exactly inert (11 at 135).
 *
 * Residual 11 - unchanged root cause, now read directly out of the .greg dump
 * rather than inferred: the symbol pseudo (reg 140) is hoisted by loop.c out
 * of BOTH loops to before the OUTER loop - at insn 327, alongside the two
 * REG_EQUIV constants 137 (0x10) and 138 (1), ahead of NOTE_INSN_LOOP_BEG 129
 * - so global.c records `140 conflicts: ... 2 4 5 6 7 29`, i.e. it conflicts
 * with $a0-$a3 and can never take $a1. `off` (reg 130) stays block-local,
 * local-alloc hands it $a1 from the hard-reg suggestion on
 * (set (reg:SI 5 a1) (plus (reg/v:SI 130) (reg:SI 140))) at insn 229, and the
 * symbol is then re-materialised into $t0. Target's allocation is the mirror
 * image: symbol in $a1 (reloaded straight into the destination), off in $v0.
 * All eleven differing instructions are that one swap plus its cascade
 * (y into $v1 not $v0; the 0x10/1 rect constants into $v0 not $t0).
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
     spellings of the test measured inert at 13). NOT VETTED - the modality ladder is
     NOT spent (synthesis untried); this construct must not be submitted as-is. */
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
  while (tbl[0] >= 0)
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
  outer++;
  } while (outer < 2);
  if (func_8003E2A0() == one) { func_8003E120(); }
}
