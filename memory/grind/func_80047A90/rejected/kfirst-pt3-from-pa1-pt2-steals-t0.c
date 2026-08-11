/* REJECTED s3 — sandbox 18 (vs floor 8 base).
 * k-first init order (k=1; pt2; pt1;) combined with pt3 = pa1 + 0x11 (shaving
 * pt1 from 5 to 4 refs, prio .244 -> .195). Hypothesis: sinking pt1 below k
 * (.233 in k-first position) lets k keep t0 while the emitted order matches
 * target's k=1-first. KILLED: pt2 (5 refs/41, .244) still allocates before k
 * and its find_reg walk hits t0 first (conflicts put a1/a2/a3 out of reach),
 * so pt2 steals t0. The k-first window needs BOTH pt1 and pt2 below k, and
 * pt2's 5th ref (the CSE'd addiu pt1,pt2,0x44 derivation) cannot be shaved
 * byte-neutrally. The working closure is the do-while(0) ref-weight wrap on
 * k = 1 (see candidate.c), not ref-shaving of the pointers. */
void func_80047A90(void) { /* body: 8-form with k-first + pt3 = pa1 + 0x11 */ }
