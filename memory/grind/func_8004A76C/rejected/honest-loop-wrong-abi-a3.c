/* REJECTED s5 (synthesis): honest natural GTE loop, vertex ptr moved to a 4th arg ($a3).
 * sandbox --disable all = 33 (tmp/grind/func_8004A76C/s5/sandbox_honest.txt), 3 source-level hunks.
 * Dead because: (1) target reads vertices via unsaved $s0 set by hand-asm caller func_8004A4E0 -> this
 * signature asserts a false ABI (completion-bar honest code); (2) not pipelined; (3) gte.h gte_rtpt()
 * .word is stripped by the sandbox (not a pinned engine/gtemacro.py unit), so 33 scores a program with no rtpt. */
/* s5 synthesis probe: the most natural honest C for func_8004A76C's semantics,
 * with the vertex pointer as an ordinary 4th argument (target takes it in an
 * unsaved $s0) and the GTE ops as canonical gte.h macro islands. */
void func_8004A76C(s32 n, s32 *sxy, s16 *sz, SVECTOR *v) {
    s32 z1, z2, z3;

    do {
        gte_ldv3(&v[0], &v[1], &v[2]);
        gte_rtpt();
        v += 3;
        gte_stsxy3(&sxy[0], &sxy[1], &sxy[2]);
        z1 = gte_mfc2(17);
        z2 = gte_mfc2(18);
        z3 = gte_mfc2(19);
        sxy += 3;
        sz[0] = z1;
        sz[1] = z2;
        sz[2] = z3;
        sz += 3;
    } while (--n != 0);
}
