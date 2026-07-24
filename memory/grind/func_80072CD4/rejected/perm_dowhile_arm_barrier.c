/* REJECTED (s4 permuter, floor-4 chassis best = permuter-score 25, NOT a match).
 * The permuter wrapped the ENTIRE arg0<4 inner-if block PLUS the `@4 = fc_const`
 * store in an empty `do { ... } while (0);`. This is a scheduling-barrier cheat
 * (empty do-while used purely to fence the store schedule), NOT the sanctioned
 * do-while-zero exception (that carve-out is scoped to the reorg.c
 * LABEL_OUTSIDE_LOOP_P / NE-invert peephole; here the effect is store-order
 * scheduling, outside the sanction — same family as the already-rejected
 * plus4_first_dowhile.c). AND it does not even reach a byte match (score 25,
 * ~5 register diffs remain). Dead on both counts: cheat + non-match.
 *
 * Mechanism: the do-while NOTE_INSN_LOOP_BEG/END fences perturb sched1's
 * placement of the @4/@0xC merge stores, but the fence is a no-semantic-purpose
 * wrapper, and it trades the 2-store reorder for register diffs (honestly worse).
 */
s32 func_80072CD4(s32 arg0, GameObj *arg1)
{
  int fc_const;
  initPolyG4(arg1);
  gpu_SetSemiTransp(arg1, 0);
  if (arg0 < 4)
  {
    do {
      fc_const = 0xFC;
      if ((*((s32 *) (((s32) D_800A35C4) + 8))) & 4) {
        *((u8 *) (((s32) arg1) + 5)) = 0xC3;
        *((u8 *) (((s32) arg1) + 6)) = 0x1E;
        *((u8 *) (((s32) arg1) + 0xD)) = 0xC8;
        *((u8 *) (((s32) arg1) + 0xE)) = 0x32;
      } else {
        *((u8 *) (((s32) arg1) + 5)) = 0xC3;
        *((u8 *) (((s32) arg1) + 6)) = 0x50;
        *((u8 *) (((s32) arg1) + 0xD)) = 0xDC;
        *((u8 *) (((s32) arg1) + 0xE)) = 0x46;
      }
      *((u8 *) (((s32) arg1) + 4)) = fc_const;
    } while (0);
    *((u8 *) (((s32) arg1) + 0xC)) = fc_const;
    *((u8 *) (((s32) arg1) + 0x14)) = 0xFC;
    *((u8 *) (((s32) arg1) + 0x15)) = 0x82;
    *((u8 *) (((s32) arg1) + 0x1C)) = 0x32;
    *((u8 *) (((s32) arg1) + 0x1D)) = 0x28;
    *((u8 *) (((s32) arg1) + 0x16)) = 0;
    *((u8 *) (((s32) arg1) + 0x1E)) = 0xA;
  }
  /* else arm unchanged from candidate.c */
  ot_Link(D_800A374C + 0x60, arg1);
  return (s32) (((u8 *) arg1) + 0x24);
}
