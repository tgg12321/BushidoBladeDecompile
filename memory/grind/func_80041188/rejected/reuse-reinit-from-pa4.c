/* REJECTED s3 (sandbox 15): the stptr-reuse form with the loop2 re-init
 * reading the CARRIER instead of out2:
 *
 *     stptr2 = saved + 0x750;
 *     stptr = (s32) (((u8 *) pa4) + 0x20);   // emits addiu s3,s7,32 == target
 *
 * The bytes of the re-init itself are exactly target's, but dropping the
 * out2 read removes out2's 4th flow-counted reference and its live
 * extension into the loop2 preamble: out2 falls from 4 refs/47 insns
 * (pri 1702, tied with tbl, tie breaks in our favor by allocno number)
 * to 3 refs/42 (pri 714), sinking below the carrier (6 refs/95 = 1263)
 * and a3 (808).  The carrier then takes s6 and the 3-cycle
 * {carrier, a3, out2} returns: sandbox 15 (vs 1 for the out2-read form).
 * Measured 2026-08-21, artifacts in tmp/grind/func_80041188/s3/.
 *
 * Session also measured (same chassis): swapping the two preamble2
 * statements (stptr = (s32) out2; BEFORE stptr2 = saved + 0x750;)
 * scores 10 -- the copy must be the LAST preamble2 statement so out2's
 * live length is 47 (pri exactly ties tbl's 1702); one insn shorter
 * (46 -> pri 1739 > 1702) makes out2 allocate BEFORE tbl and steal s5.
 */
