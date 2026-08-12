/* REJECTED — MoveImage, session 1.
 * WHY DEAD: guard RE-SPELLING has ZERO effect. All three spellings below
 * compiled BYTE-IDENTICAL to the two-plain-ifs baseline: 47 insns, score 21,
 * `beqz $v0,<epilogue> / li $v0,-1` emitted twice. cc1 normalizes if / goto /
 * `||` guard forms to the same RTL before jump.c ever runs, so the
 * jump.c:1764 "conditional jump jumping over an unconditional jump" collapse
 * fires identically for all of them.
 * The 2-instruction shortfall is fixed by BODY statement order instead — see
 * hypotheses.md H2 and candidate.c. Do not re-propose any of these.
 */

/* (A) inverted second guard, body inside the if */
s32 MoveImage_A(s32 *arg0, s16 arg1, s16 arg2) {
    /* ... */
    if (((s16 *)arg0)[2] == 0) {
        return -1;
    }
    if (((s16 *)arg0)[3] != 0) {
        /* body ... */
        return 0 /* fn(...) */;
    }
    return -1;
}

/* (B) mixed exit forms — `goto err` on the first guard, trailing err block */
s32 MoveImage_B(s32 *arg0, s16 arg1, s16 arg2) {
    if (((s16 *)arg0)[2] == 0) {
        goto err;
    }
    if (((s16 *)arg0)[3] != 0) {
        /* body ... */
        return 0 /* fn(...) */;
    }
    return -1;
err:
    return -1;
}

/* (C) short-circuit `||` — kept in the final form only because it is the
 * natural PsyQ spelling, NOT because it changed anything on its own. */
s32 MoveImage_C(s32 *arg0, s16 arg1, s16 arg2) {
    if (((s16 *)arg0)[2] == 0 || ((s16 *)arg0)[3] == 0) {
        return -1;
    }
    /* body ... */
    return 0 /* fn(...) */;
}
