/* Rejected: declare all locals uninitialized, then assign them in the
 * intended LUID order (i = a0 BEFORE v1 = a0<<4). Score rose 2 -> 11.
 *
 * MECHANISM: GCC 2.7.2's global.c allocno-priority uses ASSIGNMENT LUID,
 * not declaration LUID. Splitting decl from init just moves the LUID cost
 * to the assignment site; the same RA rotation as H1 (i-before-v1) fires
 * (a0->$21, s4->$18, s5->$20 + CSE rewrite of sll operand to $16).
 *
 * WHAT IT PROVES: RA and sched2 use the SAME LUID axis for the pseudos of
 * `i` and `v1`. Reordering the assignments perturbs both simultaneously;
 * there is no C-source structural lever that changes ONLY the sched2 LUID
 * of the a0->s0 (move16) pseudo without also changing its RA allocno
 * priority. The closing lever must therefore either:
 *   (a) create the move16 pseudo INSIDE the s4 tree so its LUID is baked
 *       into the s4 sub-expression (see cse-fold-anon-shift), or
 *   (b) express move16 as a comma/assignment sub-expression at a specific
 *       tree position that changes only the sched2 LUID, or
 *   (c) find a form that eliminates the move16 pseudo entirely (a0 stays
 *       in $18 and $16=$18 is the scheduler's copy-choice).
 */
void func_80045294(s32 a0, s32 a1) {
    s32 sum = 0;
    s32 v1;
    s32 s4;
    s32 i = a0;
    s32 count;
    s32 s5;
    v1 = a0 << 4;
    s4 = *(s32 *)((u8 *)&D_800EED14 + v1);
    count = D_800A33AC;
    s5 = s4 + a1;
    /* ... body unchanged ... */
}
