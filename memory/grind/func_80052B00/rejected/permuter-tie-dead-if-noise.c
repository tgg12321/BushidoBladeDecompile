/* REJECTED — func_80052B00, grind session 4 (permuter, 2026-08-01).
 *
 * WHY IT IS DEAD: it is not an improvement and it is a cheat shape.
 *
 * This is the ONLY output either session-4 permuter campaign produced across
 * 117,314 iterations on two structurally distinct chassis. It scored 140 — an
 * exact TIE with the base chassis (`struct-fields-direct-rvalue-fused8`,
 * tmp/grind/func_80052B00/s4/ws2/base.c), not an improvement — and it emits
 * byte-identical code to that base: the trailing `if (new_var)` block is dead
 * and GCC deletes it entirely.
 *
 * Independently of the score, the construct is forbidden: `new_var` is an
 * uninitialized local read and then dead-stored, i.e. a dead-local /
 * constant-holder shape ([[dead-vars-local-array]], [[inline-asm-policy]]'s
 * coercion catalog). It is score-inert under the cheat-invisible sandbox
 * besides. It is banked here purely so the permuter's single "find" is on the
 * record as noise rather than as an unexplored lead.
 *
 * Full campaign data: memory/grind/func_80052B00/evidence.md §Session 4,
 * hypotheses.md H8, tmp/grind/func_80052B00/s4/ws2/campaign.log.
 */
typedef signed int s32;
typedef struct {
    s32 r0;
    s32 r1;
    s32 r2;
    s32 r3;
    s32 r4;
    s32 r5;
    s32 r6;
    s32 r7;
} GteCtl;

void func_80052B00(GteCtl *m) {
    int new_var;
    __asm__ volatile (
        "ctc2 %0, $0\n\tctc2 %1, $1\n\tctc2 %2, $2\n\tctc2 %3, $3\n\t"
        "ctc2 %4, $4\n\tctc2 %5, $5\n\tctc2 %6, $6\n\tctc2 %7, $7"
        :: "r"(m->r0), "r"(m->r1), "r"(m->r2), "r"(m->r3),
           "r"(m->r4), "r"(m->r5), "r"(m->r6), "r"(m->r7));
    if (new_var) {
        new_var = 1;
    }
}
