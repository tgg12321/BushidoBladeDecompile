/* REJECTED — func_800174F4, session 1.
 * WHY DEAD: floor 25 -> 32 and build_insns 135 -> 133 (three instructions
 * LOST). Giving every role its own local (i = counter, n = loop limit,
 * tv = D_800A37A8[] table value, all split out of the single `h` variable)
 * destroys the reuse that target depends on: target keeps 0xF0/div-dividend,
 * loop-limit and table-value ALL in $s1, which only happens when they are
 * webs of ONE C variable. Splitting the counter alone is the win (floor 18 —
 * see hypotheses.md H2); splitting n or tv is the loss.
 * Do not re-propose.
 */
    s32 env;
    unsigned short h;
    s32 prim;
    s32 mask;
    s32 i;              /* <-- good (H2) */
    unsigned short n;   /* <-- kills it */
    unsigned short tv;  /* <-- kills it */

    /* ... case 1/2: i = 0; n = (rand() & 3) + 4; loop on i < n ...
       ... case 20:  div_result = h / (a1_val + 1);
                     tv = D_800A37A8[a0_val];
                     if ((new_val & 0xFF) == a1_val) tv |= 0x8000;
                     func_80060414(tv, (u8 *)prim, 0);            */
