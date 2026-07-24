/* REJECTED s2, score 10. Pass literal 0x80 (not constant_80) as the 3rd arg of
 * the FINAL func_800372F4 call, trying to SHORTEN constant_80's live range so
 * copy_end could tie/beat it in priority (a tie -> allocno-number tiebreak puts
 * copy_end, higher pseudo, last -> s5).
 * MEASURED: const_80(76) nrefs 3->2 BUT livelen STILL 76 (pri 263). GCC/CSE
 * still materializes 0x80 once and holds it in a callee-saved reg from entry to
 * the final call -> live range unchanged. const's livelen is STRUCTURALLY LOCKED
 * at ~76 (it is genuinely needed at both the first and final call). Therefore
 * copy_end (livelen capped ~38) can never tie or beat const -> never lowest
 * priority -> never s5. Score got worse (10).
 */
    v0 = ((s32 (*)())func_800372F4)(dest[3], dest[2], 0x80);
