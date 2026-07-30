/* mk_g2l - REJECTED (session 2). Best SHARED-ARGUMENT spelling. Floor 7.
 *
 * Why it is dead: every path funnels the call's first argument through one C
 * variable (`id`) reaching a shared `do_call:` label, which forces GCC to
 * materialise a PSEUDO for that argument.  The pseudo legitimately prefers
 * hard reg $a0, and global.c:797-840 expand_preferences() then IORs its
 * {a0,a1} preference into the call-path obj pseudo, because the shared load
 * `id = *(s16 *)(v0_obj + 0x4)` is a single_set whose DEST is that pseudo and
 * which carries a REG_DEAD note for v0_obj, and the two allocnos do not
 * conflict.  The obj therefore takes $a0; target wants $v0.
 *
 * This form is otherwise PERFECT - the greg dump shows every other allocno
 * already on target's register (73 ptr->$t0, 74 cmd->$a1, 75 a3->$a3,
 * 77 arg_v1->$v1, 79 chk_obj->$v1, 80 id->$a0, 81 arg_a2->$a2).  That is
 * exactly why it is banked: it is the ceiling of the whole shared-arg family,
 * and it proves the family's ceiling is 7, not 0.  The match (see
 * ../candidate.c) required deleting the argument variable entirely so the
 * field-4 load writes the argument HARD register and the merge never fires.
 *
 * Do not re-propose any shared-arg-variable spelling (dead `a0` parameter,
 * fresh `id`, `argv`, staged holder, ...) - see hypotheses.md K3.
 */
void mk_g2l(s32 a0, s32 *ptr, s32 cmd, s32 a3, u8 *stack_a2, s32 stack_v1) {
    s32 v0_obj;
    s32 chk_obj;
    s32 id;
    u8 *arg_a2 = stack_a2;
    s32 arg_v1 = stack_v1;

    if (a0 == 1) {
        return;
    }

    if (arg_v1 != 0) {
        v0_obj = *ptr;
        cmd = 0x2B;
        goto call_with_field4;
    }

    chk_obj = *ptr;
    {
        s16 f8E;

        id = *(s16 *)(chk_obj + 0x86);
        f8E = *(s16 *)(chk_obj + 0x8E);

        if (id == f8E) {
            id = *(s16 *)(chk_obj + 0x4);
            cmd = 0x28;
            goto do_call;
        }

        {
            s16 f88 = *(s16 *)(chk_obj + 0x88);
            if (id == f88 && a3 != 0) {
                id = *(s16 *)(chk_obj + 0x4);
                cmd = 0x27;
                goto do_call;
            }
        }
    }

    if (cmd == 0) {
        v0_obj = *ptr;
        cmd = 0x22;
        goto call_with_field4;
    }

    if (cmd < 6) {
        cmd = 0x23;
        v0_obj = *ptr;
        goto call_with_field4;
    }

    v0_obj = *ptr;
    cmd = 0x24;

call_with_field4:
    id = *(s16 *)(v0_obj + 0x4);

do_call:
    func_80032854(id, cmd, arg_a2, (s16 *)0);
}
