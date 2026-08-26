/* REJECTED [s9, solver modality] - counter-split partition P2 ("n = count + 1"
 * as a fresh loop-carried local, so the counter's 8 global.c allocno refs are
 * split across two pseudos with disjoint live ranges).
 *
 * WHY IT IS DEAD (measured, not argued): sandbox --disable all = 13, 33/33 insns
 * (byte-neutral - the two split pseudos do not conflict and both land in $s0, so
 * no copy is emitted), and the instrumented-cc1 ALLOCDBG shows the split SURVIVED
 * to global alloc yet did not help:
 *     pseudo 79 (n, loop-carried) nrefs=4 livelen=7  pri=11428 -> $s0
 *     pseudo 75 (count)           nrefs=4 livelen=8  pri=10000 -> $s0
 *     pseudo 74 (pointer)         nrefs=5 livelen=17 pri= 5882 -> $s1
 * Both halves still outrank the pointer's byte-forced 5882, so the pointer is
 * still allocated SECOND and still gets $s1. This is the empirical confirmation
 * of the s9 foreclosure: global.c priority is refs/live_length, so every cut that
 * lowers a counter pseudo's ref count shortens its live range by the same span -
 * refs and live_length fall together and the quotient never drops below the
 * pointer's. See memory/grind/func_80037A20/evidence.md [s9].
 */
s32 func_80037A20(s32 arg0, s32 arg1)
{
  s32 *var_s0;
  s32 var_s1;
  s32 sp10[8];
  s32 v0_val;
  var_s0 = (s32 *)&D_80102810;
  func_80079A30(sp10, (s32) (&g_str_memcard_fmt), arg0, arg1);
  var_s1 = 0;
  if (bios_firstfile_B(sp10, var_s0) != 0)
  {
    s32 n = var_s1 + 1;
    loop:
    var_s0 = (s32 *) (((u8 *) var_s0) + 0x28);
    v0_val = bios_nextfile_B(var_s0);
    n += 1;
    if (v0_val) goto loop;
    var_s1 = n - 1;
  }
  D_800A38C8 = var_s1;
  return var_s1;
}
