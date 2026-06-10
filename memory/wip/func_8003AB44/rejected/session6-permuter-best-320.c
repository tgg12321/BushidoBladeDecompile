/* REJECTED — permuter session 6 best find (score 320 / floor 540 base)
 *
 * Found by 33,307 directed-permuter iters on candidate.c base (2026-06-10).
 * Best legitimate permuter score: 540 (= candidate.c unchanged). Best find
 * with mutations: 320, via TWO mutations both of which fail the cheats-by-
 * any-spelling lens per [[no-new-park-categories]] / [[review-discipline-
 * before-commit]] / [[do-while-zero-exception]]:
 *
 *   1. CASE 1: `do { D_800A38AC = 2; } while (0);` wrap around the state
 *      transition store. The do-while-zero exception is sanctioned ONLY for
 *      the LABEL_OUTSIDE_LOOP_P / reorg.c invert-jump peephole interaction
 *      [[do-while-zero-exception]]. Here the gap is NOT that mechanism — it
 *      is (a) case 2 polarity (reorg.c eager-fill of `addiu $a0,3` over
 *      `move v0,0` in a bnez delay slot), and (b) case 5/6 register choice
 *      ($v1 vs $v0 — flow.c live-set at the multi-entry jtbl join, per
 *      session 4's ALLOCDBG diagnostic). The wrap bends a DIFFERENT GCC
 *      pass and reopens the slippery-slope the rule exists to close.
 *
 *   2. CASE 2: `D_800A37D8 = 0;` added before `goto done;`. This is an
 *      emitted store with no semantic justification — D_800A37D8 was
 *      already set to 0 in case 1 of a prior call (state machine reaches
 *      state 2 only after state 1 ran the reset). Adding a duplicate reset
 *      to influence GCC's RTL flow analysis is a dead-store coercion in
 *      the family of forbidden Lever D / dead-conditional-store /
 *      goto-end-prologue-delay-slot patterns. The store appears in the
 *      emitted output (it is NOT DCE'd — would be a +1-insn regression in
 *      isolation), but its presence is what flips the permuter's score.
 *      A real programmer would not write this duplicate reset.
 *
 * Combined effect: the permuter scored 320 because mutation 1 changed the
 * case 1 branch structure enough to ripple through case 2's surrounding
 * RTL, and mutation 2 displaced the case 2 dispatch's available-expr live
 * set. Neither shift retired any of the 5 baseline regfix rules — the
 * masked-Levenshtein floor 6 was unchanged. The permuter's weighted
 * scoring metric ≠ the sandbox masked-Levenshtein metric ([[scoring-
 * systems]]); 320 is a noise-reduction artifact, not a structural lever.
 *
 * Cheat-reviewer verdict: FAIL (both mutations). Preserved here only as
 * a "do not re-derive this" marker for future agents.
 */

typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
extern s32 D_800A37B8;
extern u8 D_800A38AC;
extern s32 D_800A37D8;
extern s32 D_800A38A0;
extern u8 D_80102794;
extern u8 D_800A3916;
extern void func_8003A308(void);
extern s32 func_8008C464(s32, s32, s32);
extern void func_8003A39C(void);
extern void func_8003A360(void);
extern void gpu_SetDispMask(s32);

s32 func_8003AB44(void) {
    D_800A37B8++;
    switch (D_800A38AC) {
    case 0:
        D_800A38AC = 1;
        return 0;
    case 1:
        func_8003A308();
        D_800A37D8 = 0;
        if (D_800A38A0 == 0) {
            gpu_SetDispMask(1);
            do { D_800A38AC = 2; } while (0);    /* CHEAT — do-while-zero out of sanctioned scope */
            return 0;
        }
        D_800A38AC = 3;
        return 0;
    case 2:
        if (D_80102794 & 0x10) {
            goto fail;
        }
        if (func_8008C464(3, 1, 0) == 0) {
            return 0;
        }
        D_800A37D8 = 0;                          /* CHEAT — dead-store coercion */
        goto done;
    case 3:
        if (func_8008C464(3, 1, 0) != 0) {
            goto retry;
        }
    done:
        func_8008C464(3, 0, 0);
        D_800A38AC = 4;
        return 0;
    retry:
        D_800A37D8++;
        if (D_800A37D8 < 4) {
            return 0;
        }
    fail:
        func_8003A39C();
        return -1;
    case 4:
        gpu_SetDispMask(0);
    case 5:
    case 6:
        D_800A38AC++;
        return 0;
    case 7:
        D_800A3916 = 1;
        func_8003A360();
        return 1;
    }
    return 0;
}
