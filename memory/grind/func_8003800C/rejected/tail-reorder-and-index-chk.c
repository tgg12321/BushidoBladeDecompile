/* REJECTED — two zero-cost-intended context perturbations, both WORSE. (s3, 2026-07-22)
 *
 * Goal: flip Region A (sum/j a0/a1 swap) by shifting whole-function LUID context
 * WITHOUT the offset+=j instruction cost. Neither worked — both disturbed the
 * already-matched outer-loop registers (base=t1, i=a2, chkptr=t0, offset=a3).
 *
 * (1) TAIL REORDER offset-first: `offset += 0x24; chkptr++; i++;` (vs
 *     `chkptr++; i++; offset += 0x24;`)  ->  score 10 (from 8). The three
 *     increments are independent; reordering the source changes sched1's tie
 *     placement in the outer-loop tail and cascades. KILLED.
 *
 * (2) INDEX-BASED CHK (drop the chkptr walking-pointer IV): replace
 *     `chkptr = (s32*)base; ... if (sum == *(s32*)((u8*)chkptr+0x6C)); ... chkptr++;`
 *     with `if (sum == ((s32*)base)[i + 0x1B])`  ->  score 14 (from 8).
 *     Converting the chkptr induction variable to i-indexing forces GCC to
 *     recompute base + i*4 each iteration (sll/addu), diverging from target's
 *     matched `addiu $t0,$t0,4` walking pointer. KILLED.
 *
 * Conclusion: the matched outer regs (t1/a2/t0/a3) are load-bearing; any outer
 * IV restructure that touches them cascades worse. offset+=j is the ONLY manual
 * lever found that flips Region A, and it is not match-viable (see the sibling
 * rejected file). Zero-cost manual flip of Region A is confirmed unavailable.
 */
