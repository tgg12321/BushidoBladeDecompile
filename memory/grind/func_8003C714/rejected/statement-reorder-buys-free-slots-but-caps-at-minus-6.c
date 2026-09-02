/*
 * REJECTED (s8, 2026-09-01, forensics) — the MOVABLE-ORDER dial is real, is FREE
 * (byte-count-neutral), is ordinary C, and is arithmetically two orders of
 * magnitude too small. It also changes the emitted instruction SEQUENCE, so it
 * can never match the target even if it were large enough.
 *
 * s7's K22 concluded "free hoists do not exist" — that is true for ADDING a
 * hoist. This session found the adjacent thing s7 did not test: REORDERING the
 * hoists that already exist. Plain statement reordering inside the loop body
 * moves the 0x91A2B3C5 movable later in loop.c's movable list at zero cost in
 * insn_count and zero cost in emitted instruction COUNT.
 *
 * MECHANISM. loop.c builds the movable list in insn order (scan_loop walks the
 * loop once, appending at loop.c:797-803). move_movables then walks that list in
 * order and does `threshold -= 3` after each successful move (loop.c:1719/1904),
 * so a movable's desirability test at loop.c:1631 sees
 *     threshold = 2 * (1 + n_non_fixed_regs) - 3 * (moves before it)
 *               = 122 - 3k
 * against `insn_count`. Reordering the four body statements changes which const
 * loads precede the magic, hence k.
 *
 * MEASURED (tmp/grind/func_8003C714/s8/sweep_ord.sh + gen_ord.py, one cc1 -dL
 * run per form; the loop-25 movable table):
 *
 *   order (statements)        | insn_count | magic slot | threshold | asm lines
 *   0x21,0x22,0x23,0x24 (cand)| 56         | 2nd (r84)  | 119       | 107
 *   0x22,0x23,0x21,0x24       | 56         | 3rd (r126) | 116       | 107
 *   0x23,0x22,0x21,0x24       | 56         | 3rd (r126) | 116       | 107
 *   0x24,0x22,0x23,0x21       | 56         | 3rd (r127) | 116       | 108
 *
 * WHY IT IS DEAD, in one line of arithmetic: this loop has exactly THREE
 * movables (regno 78 = the D_80106A58 address, regno 84 = 0x91A2B3C5, regno 91 =
 * 0x88888889), so the magic can be pushed to slot 3 and no further — a maximum
 * threshold reduction of 6, from 122 to 116. Declining the hoist needs
 * threshold < insn_count = 56, i.e. 22 moves ahead of it. `threshold` is a LOCAL
 * of scan_loop recomputed per loop (loop.c:532), so movables in the file's other
 * loops contribute nothing. The order dial's entire free range is -6 against a
 * -66 requirement.
 *
 * AND it is byte-divergent anyway: the emitted store sequence follows the source
 * statement order, so 0x22-before-0x21 emits the target's 8003C754.. quartet in
 * the wrong place. Diff of the `0x22,0x23,0x21,0x24` form against the candidate
 * shows the whole /1800 block relocated past the /30 block plus a reshuffled
 * `addu $8,$8,1`. The target's order is 0x21, 0x22, 0x23, 0x24 and the candidate
 * already has it.
 *
 * ALSO MEASURED AND DEAD, same session, same harness (sweep_form.sh,
 * gen_form.py): the LOOP FORM is insn_count-neutral. do-while / for / while
 * spellings of the same body all give `56 real insns`, the same 3 movables with
 * the magic 2nd, and 107 asm lines. The `for`/`while` forms widen the counted
 * range from 25..146 to 24..148 / 25..148 (count_loop_regs_set counts from
 * `loop_top ? loop_top : loop_start`, loop.c:592) but the extra luids are NOTEs,
 * not 'i'-class insns, so `count` at loop.c:3007 is unchanged. The loop_top
 * route adds zero.
 *
 * The rejected form (the best of the reorderings — byte-count-neutral, magic in
 * slot 3, still hoisted, still byte-divergent):
 */
void func_8003C714(void) {
    u8 buf[4];
    s32 *s0;
    s32 i;
    u8 *src;
    u8 *dst;

    s0 = func_80077D00();
    func_800372C0();
    gpu_InitDisplay();
    func_80060758();
    i = 0;
    do {
        src = (u8 *)&D_80106A58 + i * 8;
        dst = (u8 *)s0 + i * 4;
        dst[0x22] = (*(s32 *)(src + 4) / 30) % 60;
        dst[0x23] = ((*(s32 *)(src + 4) % 30) * 100) / 30;
        dst[0x21] = *(s32 *)(src + 4) / 1800;
        dst[0x24] = *src;
        i += 1;
    } while (i < 3);
    func_8001CD68(buf);
    *((u8 *)s0 + 0x2D) = *(u16 *)buf;
    *((u8 *)s0 + 0x2E) = buf[2];
    *((u8 *)s0 + 0x2F) = buf[3];
    *((u8 *)s0 + 0x30) = *(u16 *)&D_80101ED2;
    disp_SetFramebufferMode(1, 0, 0, 0);
    D_800A37B8 = 0;
    D_800A3834 = 0x1F;
    gpu_DisableDisplay();
}
