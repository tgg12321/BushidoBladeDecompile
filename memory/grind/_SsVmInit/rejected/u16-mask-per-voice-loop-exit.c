/* REJECTED s2: masking the per-voice loop's exit compare and/or shift
 * amount to u16 (to match target's andi $v0,$s0,0xFFFF; sltu; bnez /
 * andi $a1,$s0,0xFFFF shapes at the loop's tail) made the score WORSE,
 * not better: 19 -> 21 with the exit-compare mask alone or combined with
 * the shift-amount mask. Measured both combinations; both regressed.
 * Diff vs the CONFIRMED candidate.c (only the two lines below changed):
 *
 *   offset = 1;
 *   buf[0] = offset << (u16)i;      // was: buf[0] = offset << i;
 *   ...
 *   } while ((u16)i < maxVoice);    // was: } while (i < maxVoice);
 *
 * KILLED (instance): this specific spelling, on the s2 HEAD chassis
 * (candidate.c as committed this session), no FAKE constructs present.
 * Do not re-propose this exact cast placement without a new mechanism
 * theory backed by a fresh .greg/.sched dump read.
 */
