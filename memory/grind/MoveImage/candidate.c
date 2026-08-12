/* MoveImage — session 5 (permuter modality).
 *
 * ############################################################################
 * # THIS FORM MEASURES sandbox MoveImage --disable all == 0 (build_insns 49  #
 * # == target 49) ON THE REAL WHOLE-TU BUILD — the function is byte-closed.  #
 * # IT IS **NOT** APPROVED.  The closing lever is a TYPE-QUALIFIER CLAIM on  #
 * # a global (`extern s32 *const g_gpu_dev_table;`) and I could not place it #
 * # in a sanctioned SOTN family, so session 5 returned `ruling-request`      #
 * # instead of `candidate-ready`.  DO NOT submit this as a candidate, and do #
 * # NOT re-derive it as if it were sanctioned, until the owner has ruled.    #
 * # The sanctioned-construct fallback is candidate_sanctioned_floor2.c       #
 * # (floor 2) and candidate_alt_plain_arg.c (floor 4, residual moved).       #
 * ############################################################################
 *
 * WHAT CHANGED vs candidate_alt_plain_arg.c (which measures 4 / 49):
 *   the file-scope declaration in src/display.c
 *       -  extern s32 g_gpu_dev_table;
 *       +  extern s32 *const g_gpu_dev_table;
 *   Nothing else.  The function body is the session-3 plain-argument base
 *   (walking rect pointer + `q = p + 6; fn(*q, ...)`), unchanged.
 *
 * MECHANISM (measured, session 3 + 5):
 *   The sole residual on the plain-argument base was the dev-table
 *   `lui/lw %hi/%lo(g_gpu_dev_table)` pair emitted 4 slots LATE, because in
 *   that build the dev-table load acquires a true memory dependence on the
 *   packet store (`mv2.prio`: insn=83 pred=75 kind=0, final_pri=2) that
 *   target's build does not have.  A const-qualified declaration makes the
 *   load RTX_UNCHANGING_P, and sched.c:828 makes an unchanging read never
 *   conflict with any store — the false edge disappears, priority(83) goes
 *   back to 1, and the pair moves into target's slots.  Score 4 -> 0.
 *
 * COLLATERAL: none measured.  With the const declaration in place the four
 * matched siblings that share the declaration/table still measure 0:
 *   ClearImage 0/37, ClearImage2 0/39, LoadImage 0/25, StoreImage 0/25.
 *
 * SEMANTIC BASIS FOR THE CLAIM (this is what the owner must rule on):
 *   g_gpu_dev_table (0x8009BE6C) is READ-ONLY across the entire executable.
 *   `grep 8009BE6C asm/funcs/*.s` -> 17 files, every reference a `lw`, ZERO
 *   stores; no `g_gpu_dev_table = ...` in any src/*.c.  So "this pointer is
 *   never reassigned" is a true statement about the program, not a fiction.
 *   AGAINST: the construct has no observable effect on emitted behaviour
 *   (cheat test T1), its mechanism is named in terms of a GCC pass
 *   (sched.c RTX_UNCHANGING_P, test T3), and the nearest catalog family is
 *   "volatile-coercion by plain extern", of which const is the mirror image
 *   (test T5).  The nearest SANCTIONING rule is
 *   .claude/rules/header-type-correction-from-use-sites.md, but its prong (b)
 *   ("the OLD type required functionally necessary compensating casts") is
 *   about signedness/width, not qualifiers, so it cannot be cited verbatim.
 *   That unresolvable tension is exactly why this is a ruling-request.
 */
extern s32 *const g_gpu_dev_table;   /* <-- THE LEVER (file scope, display.c) */
extern u8 D_80015F74;
extern s32 D_8009BF24;
extern s32 D_8009BF28;
extern s32 D_8009BF2C;

s32 MoveImage(s32 *arg0, s16 arg1, s16 arg2) {
    s32 *p;
    s32 *q;
    s32 (*fn)();
    s32 packed;
    s32 *bf24;
    s32 *rect;
    s32 src;

    func_8007B3A8(&D_80015F74, (s32)arg0);
    if (((s16 *)arg0)[2] == 0 || ((s16 *)arg0)[3] == 0) {
        return -1;
    }
    packed = ((s32)arg2 << 16) | ((u32)arg1 & 0xFFFF);
    bf24 = &D_8009BF24;
    rect = arg0;
    src = *rect++;
    D_8009BF28 = packed;
    *bf24 = src;
    D_8009BF2C = *rect;
    p = (s32 *)g_gpu_dev_table;
    fn = (s32 (*)())p[2];
    q = p + 6;
    return fn(*q, (s32)bf24 - 8, 0x14, 0);
}
