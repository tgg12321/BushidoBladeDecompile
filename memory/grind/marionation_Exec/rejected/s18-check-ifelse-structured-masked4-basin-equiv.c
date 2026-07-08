/* s18 v02 basin-equivalent - masked 4, 178 build_insns.
 * Rewrote check1/check2/tail from goto+label chain to structured if/else
 * (if(check){...return;} else{if(check){...return;} if(a0==0)goto loop;
 * return 0;}). Ties vT40 exactly. NOVEL masked-4 spelling: proves the
 * check-region control-flow structure (goto+label vs structured if/else)
 * is byte-inert - jump.c collapses the CFG identically. Basin membership
 * count is now 12 known distinct masked-4 spellings (vT40 + s11 u10/w03/w10
 * + s12 v08/w05/w08/z01/z02/z07 + s17 a4/a5 + s18 v02). */
