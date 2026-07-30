/* mk_g2l — MATCHING FORM, session 2 (grinder).  honest sandbox floor: 0
 * (`sandbox mk_g2l --disable all` => score 0, target_insns 51, build_insns 51).
 *
 * IN PLACE in src/code6cac_b.c as of session 2.  The 10 regfix rules at
 * regfix.txt:242-257 are now dead paperwork and should be retired by the
 * operator (`retire mk_g2l`) — this session may not touch regfix.txt.
 *
 * WHAT CLOSED IT (two independent levers, both plain structural C):
 *
 * 1. NO SHARED ARGUMENT VARIABLE — write the call at each site instead of
 *    funnelling every path through one `id` local and a shared `do_call:`
 *    label.  Session 1 + the first half of session 2 got stuck at floor 7
 *    because a shared arg local forces GCC to materialise a PSEUDO for the
 *    call's first argument.  That pseudo legitimately prefers hard reg $a0,
 *    and `global.c:797-840 expand_preferences()` then IORs its {a0,a1}
 *    preference straight into the call-path obj pseudo (the shared load
 *    `id = *(s16 *)(obj + 4)` is a single_set whose DEST is that pseudo and
 *    which carries a REG_DEAD note for obj, and the two do not conflict).
 *    The obj therefore took $a0; target wants $v0.  With the call duplicated
 *    into each arm and the field-4 read written inline, the load combines
 *    directly into the argument HARD reg, so `reg_allocno[SET_DEST]` is -1,
 *    expand_preferences skips the insn, no preference is merged, and the obj
 *    falls back to the alloc-order first-free register — $v0, exactly target.
 *    jump2's cross-jumping re-merges the duplicated `lh $a0,4($vN)` / `jal`
 *    suffixes after allocation, reproducing target's shared .L80027970 /
 *    .L80027974 tails at 51 instructions.
 *
 * 2. NO PARAM-ALIAS LOCALS — `stack_a2` / `stack_v1` are used directly.  The
 *    `u8 *arg_a2 = stack_a2; s32 arg_v1 = stack_v1;` copies (load-bearing in
 *    the OLD shared-arg structure, where dropping them cost a point) are what
 *    inverted the two entry-block stack loads: with the aliases the scheduler
 *    emitted `lw 0x2C(sp)` before `lw 0x28(sp)`; target is the other way
 *    round.  Dropping the a2 alias alone is already enough (also 0); dropping
 *    both is the cleaner spelling and is what is committed here.
 *
 * CHEAT AUDIT: no dead stores, no self-assigns, no register pins, no inline
 * asm, no volatile, no parameter reuse, no alias renames, no unused
 * declarations, no do-while(0), no goto.  Every statement has semantic
 * purpose; `chk_obj` is a genuine common subexpression of the four field
 * reads in the check region.  The duplicated call sites are the ordinary
 * early-return spelling a human would write for a five-way dispatch that ends
 * in the same call — the arms differ in their command constant, which is the
 * whole point of the function.
 */
void mk_g2l(s32 a0, s32 *ptr, s32 cmd, s32 a3, u8 *stack_a2, s32 stack_v1) {
    s32 chk_obj;

    if (a0 == 1) {
        return;
    }

    if (stack_v1 != 0) {
        func_80032854(*(s16 *)(*ptr + 0x4), 0x2B, stack_a2, (s16 *)0);
        return;
    }

    chk_obj = *ptr;
    {
        s16 f86 = *(s16 *)(chk_obj + 0x86);
        s16 f8E = *(s16 *)(chk_obj + 0x8E);

        if (f86 == f8E) {
            func_80032854(*(s16 *)(chk_obj + 0x4), 0x28, stack_a2, (s16 *)0);
            return;
        }

        {
            s16 f88 = *(s16 *)(chk_obj + 0x88);
            if (f86 == f88 && a3 != 0) {
                func_80032854(*(s16 *)(chk_obj + 0x4), 0x27, stack_a2, (s16 *)0);
                return;
            }
        }
    }

    if (cmd == 0) {
        func_80032854(*(s16 *)(*ptr + 0x4), 0x22, stack_a2, (s16 *)0);
        return;
    }

    if (cmd < 6) {
        func_80032854(*(s16 *)(*ptr + 0x4), 0x23, stack_a2, (s16 *)0);
        return;
    }

    func_80032854(*(s16 *)(*ptr + 0x4), 0x24, stack_a2, (s16 *)0);
}
