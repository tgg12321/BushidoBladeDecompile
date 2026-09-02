/*
 * REJECTED (s8, 2026-09-01, forensics) — the ADMISSION axis (s7 frontier item 2)
 * is CLOSED. No ordinary-C spelling stops `(set (reg N) (const_int 0x91A2B3C5))`
 * from being admitted to loop.c's movable list.
 *
 * WHY IT IS DEAD (mechanism, read out of tools/gcc-2.7.2/loop.c this session):
 * scan_loop's admission gate is exactly three tests, and a compiler-generated
 * CONST_INT set passes all three unconditionally:
 *
 *  1. loop.c:649  `&& ! may_not_optimize[REGNO (SET_DEST (set))]`
 *     may_not_optimize is written ONLY by count_loop_regs_set (loop.c:3037 and
 *     loop.c:3044): the SAME pseudo set in two basic blocks of the loop, or set
 *     twice with a use in between, or carrying an explicit (clobber (reg)).
 *     Two source-level `/1800` divisions in two basic blocks produce two
 *     DISTINCT pseudos, so n_times_set stays 1 for each and neither is flagged.
 *     Worse: loop.c's movable MATCHING then merges them and makes the hoist MORE
 *     desirable — measured below, `twobb` gives `regno 86 (life 2), savings 2`
 *     plus `Insn 73: regno 92 (life 1), done move-insn matches 54`.
 *
 *  2. loop.c:695-700  the uservar / basic-block test. Its second disjunct is
 *     `(! REG_USERVAR_P (SET_DEST (set)) && ! REG_LOOP_TEST_P (SET_DEST (set)))`,
 *     which is TRUE for every compiler temp. The magic pseudo is created by
 *     expand_divmod, is never a user variable and is never the loop-test reg, so
 *     this test can never reject it — regardless of maybe_never, of where in the
 *     loop the division sits, or of whether the reg is used before the set.
 *
 *  3. loop.c:715  `! ((maybe_never || call_passed) && may_trap_p (src))`.
 *     src is a CONST_INT (move_insn is set from the REG_EQUAL note at
 *     loop.c:665), and may_trap_p (const_int) is 0. Never fires.
 *
 * m->cond and m->global — the two struct fields s7 named as escape hatches —
 * are NOT admission gates at all:
 *   * m->global (loop.c:790) is consumed only for m->savemode on PARTIAL
 *     movables (loop.c:888); the desirability test at loop.c:1631 never reads it.
 *   * m->cond (loop.c:789) is `(tem|tem1|tem2) > 1`, i.e. invariant_p returned 2.
 *     invariant_p of a CONST_INT returns 1, always. m->cond is structurally 0
 *     for this movable and the re-check at loop.c:1586 passes trivially.
 *
 * MEASURED (tmp/grind/func_8003C714/s8/sweep_adm.sh, gen_adm.py; one cc1 -dL run
 * per form; `moved` counts the whole file, the loop-25 movable table is the
 * relevant one). Baseline `base` = candidate.c: 56 real insns, 3 movables
 * (regno 78 life 1 / regno 84 life 1 = the 0x91A2B3C5 magic / regno 91 life 31 =
 * the 0x88888889 /30 magic), 107 asm lines.
 *
 *   form      | loop range   | insn_count | movables | magic still moved? | asm
 *   base      | 25..146      | 56         | 3        | YES (2nd, life 1)  | 107
 *   cond      | 25..153      | 58         | 4        | YES (3rd, life 1)  | 109
 *   twobb     | 28..173      | 66         | 4+match  | YES (savings 2!)   | 123
 *   preloop   | 37..158      | 56         | 3        | YES (2nd, life 1)  | 120
 *   postloop  | 25..146      | 56         | 3        | YES (2nd, life 1)  | 118
 *   twosame   | 28..163      | 63         | 3        | YES (2nd, life 8)  | 119
 *
 * The five probe bodies are in tmp/grind/func_8003C714/s8/dumps/body_<form>.c:
 *   cond      — the division wrapped in `if (i != 5) { ... }` so the magic's set
 *               sits in a conditionally-executed basic block (maybe_never route).
 *   twobb     — a second `/1800` in the `else` arm (two-basic-block route).
 *   preloop   — a `/1800` before the loop whose value is stored after it
 *               (m->global via regno_first_uid < INSN_LUID (loop_start)).
 *   postloop  — a `/1800` after the loop (m->global via regno_last_uid > end).
 *   twosame   — a second `/1800` in the SAME basic block (n_times_set / consec).
 *
 * Every one of them still hoists the magic, and every one of them costs bytes
 * (+2 to +16 asm lines against a 104-instruction target). Nothing here is
 * submittable and nothing here is worth re-spelling.
 *
 * Representative body (the `twobb` form — the strongest of the five, and the one
 * that proves the mechanism backfires):
 */
void func_8003C714(void) {
    u8 buf[4];
    s32 *s0;
    s32 i;
    u8 *src;
    u8 *dst;
    s32 w;

    s0 = func_80077D00();
    func_800372C0();
    gpu_InitDisplay();
    func_80060758();
    w = 0;
    i = 0;
    do {
        src = (u8 *)&D_80106A58 + i * 8;
        dst = (u8 *)s0 + i * 4;
        if (i != 5) {
            dst[0x21] = *(s32 *)(src + 4) / 1800;
        } else {
            w = *(s32 *)(src) / 1800;
        }
        dst[0x22] = (*(s32 *)(src + 4) / 30) % 60;
        dst[0x23] = ((*(s32 *)(src + 4) % 30) * 100) / 30;
        dst[0x24] = *src;
        i += 1;
    } while (i < 3);
    func_8001CD68(buf);
    *((u8 *)s0 + 0x2D) = *(u16 *)buf;
    *((u8 *)s0 + 0x2E) = buf[2];
    *((u8 *)s0 + 0x2F) = buf[3];
    *((u8 *)s0 + 0x30) = *(u16 *)&D_80101ED2;
    disp_SetFramebufferMode(1, 0, 0, 0);
    D_800A37B8 = w;
    D_800A3834 = 0x1F;
    gpu_DisableDisplay();
}
