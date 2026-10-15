/* REJECTED s6 (solver diagnostic, never a candidate): vertex ptr as file-scope global to free $a3; n-- in the test makes a move $2,$4 copy; z temps got $7,$8,$9 (global.c first-fit), target $t0-$t2. Raw cc1 output: tmp/grind/func_8004A76C/s6/probe_tail.s */
#include "common.h"
#include "gte.h"
/* s6 solver diagnostic (NOT a candidate): vertex pointer moved off the
 * argument registers (file-scope pointer) so $a3 is free as in the target;
 * branch tests n before the decrement (target: beqz a0 / delay addiu a0,-1).
 * Purpose: read which hard regs global.c gives the three mfc2 temps. */
#define gte_ldv3c(p)                                                           \
    __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)\n\tlwc2 $2, 8(%0)\n\t" \
                      "lwc2 $3, 12(%0)\n\tlwc2 $4, 16(%0)\n\tlwc2 $5, 20(%0)"  \
                      :: "r"(p))
#define gte_stsxy3c(p)                                                         \
    __asm__ volatile ("swc2 $12, 0(%0)\n\tswc2 $13, 4(%0)\n\tswc2 $14, 8(%0)"  \
                      :: "r"(p))
extern SVECTOR *D_vtx;
void func_8004A76C(s32 n, s32 *sxy, s16 *sz) {
    s32 z1, z2, z3;
    SVECTOR *v = D_vtx;

    n--;
    gte_ldv3c(v);
    gte_rtpt();
    v += 3;
    goto mid;
loop:
    gte_rtpt();
    sz += 3;
    sz[-3] = z1;
    sz[-2] = z2;
    sz[-1] = z3;
mid:
    gte_stsxy3c(sxy);
    z1 = gte_mfc2(17);
    z2 = gte_mfc2(18);
    z3 = gte_mfc2(19);
    sxy += 3;
    if (n-- != 0) {
        gte_ldv3c(v);
        v += 3;
        goto loop;
    }
    sz[0] = z1;
    sz[1] = z2;
    sz[2] = z3;
    D_vtx = v;
}
