/* DEAD (s4b, measured): staging the two header words through locals the
   function ALREADY owns - which is what staged-value-reused-variable.md bound 2
   actually sanctions - does NOT close. s6/s4 as their own staging carriers:
   246 insns, diffs=8 (probe10 b1/b2/b4). s3 + a value local: 248 insns,
   diffs=40 (probe10 b3). Only two FRESH twice-written carriers reach 0, which
   is why this session returned ruling-request instead of claiming the family. */
case 3:
    s1 = s2;
    s6 = (s32 *)s0[s3 - 2];
    s4 = (s32 *)s0[s3 - 1];
    s6 = &s0[(u32)s6 >> 2];        /* 246 insns, diffs=8 */
    s4 = &s0[(u32)s4 >> 2];
    D_8009947A = 1;
    break;
