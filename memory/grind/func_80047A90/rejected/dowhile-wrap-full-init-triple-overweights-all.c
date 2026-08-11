/* REJECTED s3 — sandbox 18 (vs floor 8 base).
 * do { pt2 = ...; pt1 = ...; k = 1; } while (0); around the WHOLE loop-2 init
 * triple (k-last order). Hypothesis: note boundaries alone might flip the k=1
 * emission order. KILLED: flow.c's loop_depth ref weighting double-counts the
 * init refs of ALL THREE pseudos (pt2/pt1/k each +1 weighted ref), lifting
 * pt1/pt2 priorities above the working window and breaking the loop-2 register
 * set wholesale. The lever must be SELECTIVE: wrapping only k = 1 lifts only k
 * (the discovery that produced the 6-floor candidate). */
void func_80047A90(void) { /* body: 8-form with do-while(0) around all three inits */ }
