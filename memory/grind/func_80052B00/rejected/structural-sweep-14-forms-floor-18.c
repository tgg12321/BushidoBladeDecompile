/* func_80052B00 — REJECTED FORM CLASS: the whole pure-C structural axis.
 * Grind session 2 (2026-08-01), modality = structural.
 *
 * WHY THIS FILE EXISTS. Session 1 established the floor at 18 by reading the
 * compiler source (reorg.c stop_search_p) and reasoning about GCC's allocator.
 * Session 2 replaced that reasoning with measurement: FOURTEEN distinct C
 * spellings of this body were compiled and scored with the cheat-invisible
 * sandbox (`sandbox func_80052B00 --disable all`). None went below 18, and in
 * every single one the `jr $ra` delay slot came out as `nop`.
 *
 * Representative best form (score 18 — ties the floor, contains no pins and no
 * cheat of any kind; this is the honest pure-C body of the function):
 *
 *     void func_80052B00(s32 *matrix) {
 *         s32 t0 = matrix[0];
 *         s32 t1 = matrix[1];
 *         s32 t2 = matrix[2];
 *         s32 t3 = matrix[3];
 *         s32 t4 = matrix[4];
 *         s32 t5 = matrix[5];
 *         s32 t6 = matrix[6];
 *         s32 t7 = matrix[7];
 *         __asm__ volatile ("ctc2 %0, $0" :: "r"(t0));
 *         __asm__ volatile ("ctc2 %0, $1" :: "r"(t1));
 *         __asm__ volatile ("ctc2 %0, $2" :: "r"(t2));
 *         __asm__ volatile ("ctc2 %0, $3" :: "r"(t3));
 *         __asm__ volatile ("ctc2 %0, $4" :: "r"(t4));
 *         __asm__ volatile ("ctc2 %0, $5" :: "r"(t5));
 *         __asm__ volatile ("ctc2 %0, $6" :: "r"(t6));
 *         __asm__ volatile ("ctc2 %0, $7" :: "r"(t7));
 *     }
 *
 * MEASURED SWEEP (scripts: tmp/grind/func_80052B00/s2/sweep.py + sweep2.py;
 * raw output: sweep_results.txt + sweep2_results.txt). Baseline = 18.
 *
 *   round 1 (whole-body spellings)
 *     plain-locals ............ 18   (the body shown above)
 *     walking-pointer ......... 18   (s32 *p = matrix; s32 tN = *p++;)
 *     reverse-load-order ...... 18   (load matrix[7] first, matrix[0] last)
 *     base-copy-handle ........ 18   (second pointer handle, p[N] loads)
 *     direct-operand .......... 25   (matrix[N] inline in the asm operand)
 *     struct-block-copy ....... 43   (typedef struct of 8 s32, block copy)
 *     local-array ............. 46   (s32 m[8]; m[N] = matrix[N];)
 *     interleaved-pairs ....... (void: C89-illegal, decls after statements —
 *                                re-run correctly in round 2)
 *
 *   round 2 (ordering / statement re-association / reorg-perturbing forms;
 *            each row also records the emitted tail instructions)
 *     ctc2-reverse-write ...... 18   tail: ctc2 t1,$0 | jr ra | nop
 *     last-write-own-block .... 18   tail: ctc2 t1,$7 | jr ra | nop
 *     do-while-zero-probe ..... 18   tail: ctc2 t1,$7 | jr ra | nop
 *     trailing-return ......... 18   tail: ctc2 t1,$7 | jr ra | nop
 *     interleaved-pairs-c89 ... 21   tail: ctc2 v0,$7 | jr ra | nop
 *     interleaved-single ...... 25   tail: ctc2 a0,$7 | jr ra | nop
 *
 * WHAT THIS KILLS.
 *  1. The structural axis itself. Declaration order, load order, walking
 *     pointers, extra pointer handles, block scoping, statement interleaving,
 *     type/aggregate reshaping, cop2 write order, and a trailing return all
 *     leave the floor at exactly 18 or make it worse. There is no gradient in
 *     this axis — 4 distinct spellings tie the floor, 6 are strictly worse,
 *     none improve it.
 *  2. H1 (session 1's delay-slot impossibility) is now EMPIRICALLY confirmed,
 *     not merely read out of reorg.c: 14/14 forms emit `jr $ra; nop`. The
 *     do-while(0) probe is the strongest of these — NOTE_INSN_LOOP_BEG is the
 *     one documented lever that perturbs reorg.c's delay-slot behaviour
 *     ([[do-while-zero-exception]]), and it does not move the slot either. (It
 *     was run as a PROBE only; it is not a proposed form and its prerequisites
 *     were never in play, since it produced no benefit.)
 *
 * DO NOT RE-RUN ANY OF THESE. The residual 18 = 8 register renames (target
 * wants $t0..$t7; GCC's leaf caller-save pool gives $v0,$v1,$a1..$t1) + the
 * deferred matrix[0] load + one load-delay nop + the unfilled jr delay slot.
 * The only construct that reaches the target's register set is a
 * `register asm("$N")` pin, which is forbidden cheat-asm and score-inert.
 */
