/* s10 WAYPOINT/KILLED as a standalone (score 9). Region B index form with a SINGLE
 * dominating k=0 after the CopyBlock merge (no do-while(0)):
 *   if (!(bit)) { CopyBlock; }  k = 0;  do { ...base+k*4... } while(k<0x16);
 * RESULT: the single immediate const-0 biv init FOLDS the giv inits to plain
 * `move a0,t1; move a2,a0` (consts-first, target structure) BUT swaps the k/a2p
 * registers: k=$a2, a2p=$a1 (target wants k=$a1, a2p=$a2). Folding removes k's use
 * in the giv-init sll, dropping k's ref priority so the a2p giv wins $a1.
 * FIX (-> score 2): wrap the k=0 in do-while(0) — the loop-depth ref bump restores
 * k's priority to win $a1 while keeping the fold. See candidate.c Region B.
 * (Two-branch if/else k=0 keeps correct regs but does NOT fold -> bloated init, score 6.)
 */
