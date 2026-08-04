/*
 * REJECTED s2: loop-tail duplication placed in the store5 FALL-THROUGH arm
 * instead of the a0==0 arm. sandbox = 2 with build_insns 73 (target 72).
 *
 * Why dead: the store5 arm contains lhu 0x26C(a2) -> sh 0x90(a2). sched1
 * hoists the duplicate's `addiu a1,a1,1` into the lhu load-delay slot
 * (lhu; addiu; sh), so at jump2 time the block's suffix is only slti/bnez —
 * cross-jump merges just those two, leaving the addiu AND an inserted
 * `j` (with the sh pulled into its delay slot) as permanent extra bytes.
 * The RA flip itself DID work (19-class regression gone; registers all
 * correct). Lesson: the duplication arm must contain NO loads so the
 * duplicated addiu/slti/bnez suffix stays contiguous and merges whole.
 * The a0==0 arm (branch-only block) merges fully -> sandbox 0.
 *
 * (Delta vs candidate.c: tail duplicated after the 0x90 store, i.e.
 *  store5_21280: *(u16*)(a2+0x90) = *(u16*)(a2+0x26C);
 *                a1++; if (a1 < 3) goto loop2_21280; return;
 *  with `if (a0 == 0) goto next_21280;` left as a plain goto.)
 */
