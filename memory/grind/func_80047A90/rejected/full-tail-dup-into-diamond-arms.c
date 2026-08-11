/* REJECTED session 2: duplicating the ENTIRE post-diamond tail
 * (*pa1 += v1; if (k==8) {...} else {...}) into BOTH arms of the a0<0 diamond,
 * hoping cross-jump would re-merge to target's own `j .L80047B68` shared-tail
 * shape while lifting pa2/a3off to 10 refs (tier-3 prio ~.86-.88).
 *
 * Sandbox 24 with 81/84 insns: STRUCTURE BREAKS. Cross-jump/jump-opt merged the
 * duplicated diamond differently (3 insns lost, likely the then-arm's j+delay
 * pattern collapsed). Byte-neutrality prerequisite of the
 * duplicated-statement-into-arms family FAILS for this surface. Do not re-propose
 * duplication across the a0<0 diamond; the only byte-neutral duplication surface
 * in this function is the if (k==8) arm pair (candidate.c, floor 8).
 */
void func_80047A90(void) { /* body: candidate.c's loop2 with the whole tail
    (v1 assign; *pa1 += v1; if(k==8){store;incs}else{incs}) written out in both
    arms of if (a0 < 0) */ }
