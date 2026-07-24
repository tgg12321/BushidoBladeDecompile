/* func_80072CD4 — REJECTED (s3): the "clean cross-block var_v0" form.
 * sandbox --disable all = 13, build_insns 78 (1 SHORT of target 79).
 * This is target's OWN structure (@0xE is a native merge store reading a
 * cross-block var_v0 whose li sits in the arm; @4/@0xC emitted before @0xE),
 * so it gets the merge store ORDER right. It fails ONLY on register allocation:
 * var_v0 lands in $v1 (target: $v0) and fc_const in $a0 (target: $v1).
 *
 * ROOT CAUSE (s3, proven via cc1 -da sched1 dump, tmp/grind/func_80072CD4/s3):
 *   sched1 is a BOTTOM-UP list scheduler. In each arm the byte-store constants
 *   (0xC3/0x1E/0xC8) form li/sb pairs; when a store is scheduled, its own li
 *   gets a 7f000001 "launch" priority and is placed immediately above it
 *   (source order preserved). var_v0's li (a LONE constant load whose only use,
 *   the @0xE store, is in the NEXT block) has NO in-arm successor, so it never
 *   earns the launch boost. It is ready at every step but loses the
 *   `potential_hazard` tiebreak to the stores (sched.c:2683-2699, sb's memory
 *   unit outranks a li), so it is picked LAST bottom-up == placed FIRST in the
 *   arm (hoisted to the arm top). Hoisted => var_v0 is live ACROSS the three
 *   $v0 byte constants => conflicts with $v0 => RA forces it to $v1. (greg:
 *   reg75 conflicts 2; 74 in $a0, 75 in $v1.)
 *
 * WHY NO CLEAN FIX (s3 measured kills, all on this base):
 *   - var_v0 typed int (SI) instead of u8 (QI): 13 (no change).
 *   - @0xE written FIRST in merge (shorten range): 13 (conflict is in the ARM,
 *     not the merge — @0xE position is irrelevant).
 *   - arm byte-stores reordered / var_v0-first-in-arm: 17 (worse; drift).
 *   The pin the target relies on requires an IN-ARM consumer of var_v0. The
 *   only byte-neutral one is an in-arm `sb var_v0,0xE` — but that IS the per-arm
 *   form (candidate.c, floor 4), whose merged sb cross-jumps to the merge HEAD,
 *   putting @0xE before @4/@0xC (wrong order). Every arm value (0xC3/0x1E/0xC8/
 *   0x50/0xDC) is a distinct compile-time constant != var_v0 (0x32/0x46), so
 *   NO byte-neutral in-arm value-reuse consumer exists to pin the li otherwise.
 *
 * The two clean structural attractors are therefore (per-arm=4) and
 * (cross-block=13); neither reaches 0. A natural 0 needs the sched1 hoist
 * defeated (no clean structural lever found) OR the @4/@0xC store-order
 * duplication into arms (rejected/dup4_0xc_into_arms.c — reviewer FAIL,
 * owner-ruling question).
 */
s32 func_80072CD4(s32 arg0, GameObj *arg1)
{
  int fc_const;
  int var_v0;
  initPolyG4(arg1);
  gpu_SetSemiTransp(arg1, 0);
  if (arg0 < 4)
  {
    fc_const = 0xFC;
    if ((*((s32 *) (((s32) D_800A35C4) + 8))) & 4)
    {
      *((u8 *) (((s32) arg1) + 5)) = 0xC3;
      *((u8 *) (((s32) arg1) + 6)) = 0x1E;
      *((u8 *) (((s32) arg1) + 0xD)) = 0xC8;
      var_v0 = 0x32;
    }
    else
    {
      *((u8 *) (((s32) arg1) + 5)) = 0xC3;
      *((u8 *) (((s32) arg1) + 6)) = 0x50;
      *((u8 *) (((s32) arg1) + 0xD)) = 0xDC;
      var_v0 = 0x46;
    }
    *((u8 *) (((s32) arg1) + 4)) = fc_const;
    *((u8 *) (((s32) arg1) + 0xC)) = fc_const;
    *((u8 *) (((s32) arg1) + 0xE)) = var_v0;
    *((u8 *) (((s32) arg1) + 0x14)) = 0xFC;
    *((u8 *) (((s32) arg1) + 0x15)) = 0x82;
    *((u8 *) (((s32) arg1) + 0x1C)) = 0x32;
    *((u8 *) (((s32) arg1) + 0x1D)) = 0x28;
    *((u8 *) (((s32) arg1) + 0x16)) = 0;
    *((u8 *) (((s32) arg1) + 0x1E)) = 0xA;
  }
  else
  {
    *((u8 *) (((s32) arg1) + 4)) = 0x10;
    *((u8 *) (((s32) arg1) + 5)) = 0x30;
    *((u8 *) (((s32) arg1) + 6)) = 0x60;
    *((u8 *) (((s32) arg1) + 0xC)) = 0x18;
    *((u8 *) (((s32) arg1) + 0xD)) = 0;
    *((u8 *) (((s32) arg1) + 0xE)) = 0x40;
    *((u8 *) (((s32) arg1) + 0x14)) = 0x30;
    *((u8 *) (((s32) arg1) + 0x15)) = 0;
    *((u8 *) (((s32) arg1) + 0x16)) = 0x60;
    *((u8 *) (((s32) arg1) + 0x1C)) = 0;
    *((u8 *) (((s32) arg1) + 0x1D)) = 0;
    *((u8 *) (((s32) arg1) + 0x1E)) = 0;
  }
  ot_Link(D_800A374C + 0x60, arg1);
  return (s32) (((u8 *) arg1) + 0x24);
}
