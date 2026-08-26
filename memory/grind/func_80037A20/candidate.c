/* candidate [s9, solver modality] - honest, pin-free, cheat-free.
 * sandbox --disable all = 8  (was 13 for s1..s8; 33/33 insns).  FLOOR DROPPED.
 *
 * HEAD is INCLUDE_ASM (asm-until-matched, a7892ba2); paste this body over the
 * INCLUDE_ASM line before any sandbox re-measure.
 *
 * WHAT CHANGED vs the s1..s8 candidate: the directory pointer is spelled as a
 * BASE pointer (var_s0, handed to firstfile) plus a WALKING pointer (p) that the
 * loop advances - the natural way to walk a DIRENTRY array - and the loop is a
 * do/while instead of a goto.  Both are ordinary C; no coercion construct, no
 * FAKE annotation, nothing from any sanctioned-exception family.
 *
 * WHY IT WORKS (instrumented cc1, BB2_ALLOC_DEBUG via tools/ra_solver/extract.py):
 *   before (one pointer local): ctr  p75 nrefs=8  len=14 pri=17142 -> $s0   [WRONG]
 *                               ptr  p74 nrefs=5  len=17 pri= 5882 -> $s1   [WRONG]
 *   after  (base + walking):    p    p75 nrefs=7  len= 6 pri=23333 -> $s0   [target]
 *                               ctr  p76 nrefs=10 len=16 pri=18750 -> $s1   [target]
 *                               base p74 nrefs=3  len=12 pri= 2500 -> $s0   [target]
 * Splitting the pointer creates a SHORT, REFERENCE-DENSE loop-carried allocno
 * whose global.c priority (refs/live_length) finally out-ranks the counter's, so
 * the pointer family is allocated FIRST and takes $s0, the counter is forced to
 * $s1, and the base pointer - which dies at the copy and therefore does not
 * conflict with the walking pointer - re-uses $s0.  The copy `p = var_s0` is a
 * no-op move once both land in $s0 and is deleted by final.c, so the insn count
 * stays at 33.  Twelve of the thirteen s1..s8 diffs (the whole s0<->s1 rename)
 * are gone.
 *
 * REMAINING 8 diffs, both PRE-RA, for the next session:
 *   (1) placement of `la D_80102810`: target emits the lui/addiu pair in the
 *       ENTRY basic block (before the sprintf jal); ours emits it in the block
 *       after the jal, which shifts the whole prologue schedule.  Attribute the
 *       pass from tmp/grind/func_80037A20/dumps (.cse/.loop/.combine) before
 *       hypothesising.
 *   (2) the entry increment: ours `li $s1,1` (cse.c FIRST-pass REG_WAS_0
 *       const-prop of the dominating var_s1 = 0), target `addiu $s1,$s1,1`.
 */
s32 func_80037A20(s32 arg0, s32 arg1)
{
  s32 *var_s0;
  s32 *p;
  s32 var_s1;
  s32 sp10[8];
  s32 v0_val;
  var_s0 = (s32 *)&D_80102810;
  func_80079A30(sp10, (s32) (&g_str_memcard_fmt), arg0, arg1);
  var_s1 = 0;
  if (bios_firstfile_B(sp10, var_s0) != 0)
  {
    var_s1++;
    p = var_s0;
    do {
      p = (s32 *) (((u8 *) p) + 0x28);
      v0_val = bios_nextfile_B(p);
      var_s1 += 1;
    } while (v0_val);
    var_s1 -= 1;
  }
  D_800A38C8 = var_s1;
  return var_s1;
}
