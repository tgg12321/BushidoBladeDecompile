/* REJECTED s18 (synthesis, DIAGNOSTIC ONLY): rejected/psyq-rtpt-pair-reorg-slot-residual.c with the vertex pointer as a
 * file-scope global register variable in $16 -- a register pin (completion-bar wall), never shippable. Raw port cc1 and
 * original cc1psx agree byte-for-byte (tmp/grind/func_8004A76C/s18/port_vs_psx.txt empty). With the $s0 ABI granted, the
 * body matches the target except: mfc2 seats $2/$3/$7 (greg "75 in 2 76 in 3 77 in 7") vs target $t0/$t1/$t2; addu a1,12
 * before the mfc2s (target after); jr slot holds sh $7 (target nop). So even the banned pin leaves three GCC-unreachable
 * residuals. tmp/grind/func_8004A76C/s18/g_s0.s */
/* REJECTED s7 (forensics): closest raw-cc1 form so far (tmp/grind/func_8004A76C/s7/p_f.s). PsyQ 4.0
 * INLINE_C.H pair gte_rtpt (nop,nop,rtpt; :489) at entry + gte_rtpt_b (no nop; :814) in the loop
 * reproduces the target nop pad; sxy += 3 above the mfc2 reads makes reorg steal n-- into the beqz slot.
 * Residual vs target: $a3 base (target unsaved $s0, s1 class kill), mfc2 temps $2/$3/$8 (target
 * $t0-$t2, s6 class kill global.c:1057), addiu sxy before the mfc2s (target after), jr slot filled
 * with the last sh (target nop). Last two: reorg.c fill_simple_delay_slots backward scan (s7 class kill).
 * Asserts a false $a3 ABI; not shippable. */
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
/* DIAGNOSTIC ONLY (wall: global register variable = register pin, never shippable) */
register SVECTOR *v asm("$16");
void func_8004A76C(s32 n, s32 *sxy, s16 *sz) {
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
    z1 = gte_mfc2(17);
    z2 = gte_mfc2(18);
    z3 = gte_mfc2(19);
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
