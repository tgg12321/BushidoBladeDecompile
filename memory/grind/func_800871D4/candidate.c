/* CANDIDATE - func_800871D4 (_SsVmKeyOffNow) - session 3 (2026-08-26)
   sandbox --disable all: score = 3   (target_insns=52, build_insns=53)
   Previous floor was 10 (s0-s2). This is the first form to reproduce BOTH
   `andi $v1,$a0,0xFFFF` instructions AND the target's $a1/$a2 seats.

   THE TWO MECHANISMS (both measured this session; see evidence.md):

   (1) The redundant `& 0xFFFF` survives combine.c ONLY when the pseudo holding
       the raw `lhu` result has MORE THAN ONE USE. can_combine_p refuses to
       substitute a load into an AND when the load's destination is still live
       afterwards, so simplify_and_const_int / nonzero_bits never gets the
       chance to prove the value 16-bit-clean and fold the mask to identity.
       Caching the load in `temp_a0` and masking it TWICE - once up front, and
       once as the semantically-required restore in the else arm, which is a
       LIVE store because var_v1 is modified in place by `-= 0x10` - gives
       temp_a0 two uses, and both `andi`s survive. That alone moved the floor
       10 -> 6 with build_insns == target_insns == 52 and a structurally exact
       instruction stream. (s1's rejected m2c-shape form died because it wrote
       `1 << (var_v1 - 0x10)` instead of modifying var_v1 in place, which made
       the else-arm restore dead and let DCE remove it.)

   (2) The residual at floor 6 was a pure global.c allocno-priority tie. The
       then-arm mask (pseudo 75) and the else-arm mask (pseudo 76) are
       symmetric; global.c sorts allocnos by
       floor_log2(n_refs)*n_refs*size/live_length, so pseudo 75 (3 refs /
       19 insns, pri .158) sorted ahead of 76 (3 refs / 21, pri .143) and took
       $a1, whereas the target wants 76 in $a1 and 75 in $a2. Hoisting
       `var_a2 = 0` out of the else arm up to the declaration extends pseudo
       75's live range across block 0, which flips the sort order from
       `74 73 75 76` to `74 73 76 75` and yields the target's seats exactly
       (verified via tools/ra_solver/extract.py dispositions 75->6, 76->5).

   REMAINING COST (the whole of the score-3 residual): the hoisted
   `var_a2 = 0` is a block-0 instruction, and GCC's sched2 sinks it to just
   before the branch, where reorg.c claims it for the `beqz` delay slot. The
   target's delay slot holds `addiu $v0,$zero,1` instead, so the constant 1 is
   no longer materialised once for both arms and gets emitted twice
   (`li v0,1` in each arm) => build_insns 53 vs 52.
   The next session must reach the SAME allocno-priority flip WITHOUT adding
   any block-0 instruction. ra_solver's remaining 1-atom vectors for that are
   `refs_up pseudo 76: 3->4` and `refs_down pseudo 75: 3->2`. */
void func_800871D4(s32 a0_arg)
{
  u32 temp_a0;
  u32 var_v1;
  s32 var_a2 = 0;
  s32 var_a1;
  s32 temp_v0;
  u16 temp_v1_d8;
  u16 temp_a0_da;
  u16 temp_v0_1b10;
  u16 temp_v0_1b12;

  temp_a0 = D_8010280A;
  var_v1 = temp_a0 & 0xFFFF;
  if (var_v1 < 0x10U)
  {
    var_a2 = 1 << var_v1;
    var_a1 = 0;
  }
  else
  {
    var_v1 -= 0x10;
    var_a1 = 1 << var_v1;
    var_v1 = temp_a0 & 0xFFFF;
  }
  temp_v0 = ((((var_v1 * 8) - var_v1) * 4) - var_v1) * 2;
  *((s8 *)((u8 *)&D_800F4E35 + temp_v0)) = 0;
  temp_v1_d8 = D_801078D8;
  temp_a0_da = D_801078DA;
  *((s16 *)((u8 *)&D_800F4E1C + temp_v0)) = 0;
  *((s16 *)((u8 *)&D_800F4E18 + temp_v0)) = 0;
  temp_v0_1b10 = D_800F1B10;
  temp_v1_d8 = temp_v1_d8 | var_a2;
  D_801078D8 = temp_v1_d8;
  D_800F1B10 = temp_v0_1b10 & ~temp_v1_d8;
  temp_v0_1b12 = D_800F1B12;
  temp_a0_da = temp_a0_da | var_a1;
  D_801078DA = temp_a0_da;
  D_800F1B12 = temp_v0_1b12 & ~temp_a0_da;
}
