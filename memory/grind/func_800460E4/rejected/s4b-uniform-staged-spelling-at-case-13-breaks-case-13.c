/* DEAD (s4b, measured): applying the staged case-3 spelling UNIFORMLY at
   case 13 as well (to remove the "two constructs apart in the same function"
   divergence that banned_constructs #6 complained about) breaks case 13:
   248 insns, diffs=13 (probe8 z3); with the FAKE s1 chain also removed, 44
   (z4). Case 13's target block schedules its two header loads adjacently on
   its own - it has an extra load and a call in the same block - so it needs
   and keeps the plain ALIGN4 spelling. The divergence is byte-forced, not
   chosen. */
case 13:
    s1 = s2;
    { u32 hidx = s0[s3 - 2]; u32 lidx = s0[s3 - 1];
      hidx >>= 2; s6 = &s0[hidx];
      lidx >>= 2; s4 = &s0[lidx]; }      /* 248 insns, diffs=13 */
    func_80044010(PTR_OFF(s0, ALIGN4(s0[5])), 8);
    D_8009947A = 1;
    break;
