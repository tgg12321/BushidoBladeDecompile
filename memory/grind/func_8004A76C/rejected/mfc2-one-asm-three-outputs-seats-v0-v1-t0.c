/* REJECTED s10 (forensics): p_f with the three mfc2 reads fused into one asm with three "=r" outputs.
 * Port cc1 and original cc1psx both seat them $2/$3/$8 (target $t0-$t2); identical output to p_f.
 * Diagnostic only; asserts the same false $a3 ABI as p_f. tmp/grind/func_8004A76C/s10/v5_one_asm.{s,psx.s} */
#include "common.h"
#include "gte.h"
/* REJECTED s5 (synthesis): hand-pipelined goto-into-second-half C mirroring the target CFG,
 * local verbatim-style gte_ldv3c/gte_stsxy3c, vertex ptr as 4th arg. sandbox = 30
 * (tmp/grind/func_8004A76C/s5/sandbox_pipelined.txt), 5 source-level hunks: ALL lwc2/swc2/rtpt asm
 * stripped (multi-insn local macros + rtpt .word are not pinned gtemacro units), so the score measures
 * only the GPR skeleton; the sh/mfc2/branch skeleton itself lines up. Still asserts a false $a3 ABI. */
/* s5 synthesis probe 2: software-pipelined C (goto into the loop's second
 * half, rtpt for N+1 issued before N's sz stores), PsyQ inline_c.h
 * gte_ldv3c / gte_stsxy3c spellings (single base, constant offsets), vertex
 * pointer as ordinary 4th arg (target: unsaved $s0). */
#define gte_ldv3c(p)                                                           \
    __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)\n\tlwc2 $2, 8(%0)\n\t" \
                      "lwc2 $3, 12(%0)\n\tlwc2 $4, 16(%0)\n\tlwc2 $5, 20(%0)"  \
                      :: "r"(p))
#define gte_stsxy3c(p)                                                         \
    __asm__ volatile ("swc2 $12, 0(%0)\n\tswc2 $13, 4(%0)\n\tswc2 $14, 8(%0)"  \
                      :: "r"(p))
/* PsyQ 4.0 INLINE_C.H:489 gte_rtpt (nop,nop,rtpt) and :814/:825 gte_rtpt_b (no nop) */
#define gte_rtpt_nop() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A280030")
void func_8004A76C(s32 n, s32 *sxy, s16 *sz, SVECTOR *v) {
    s32 z1, z2, z3;

    n--;
    gte_ldv3c(v);
    gte_rtpt_nop();
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
    sxy += 3;
    __asm__ volatile ("mfc2 %0, $17
	mfc2 %1, $18
	mfc2 %2, $19" : "=r"(z1), "=r"(z2), "=r"(z3));
    if (n != 0) {
        n--;
        gte_ldv3c(v);
        v += 3;
        goto loop;
    }
    sz[0] = z1;
    sz[1] = z2;
    sz[2] = z3;
}
