/* REJECTED s17 - THIS FORM MEASURES SANDBOX score 0 (cheat-stripped, rules_dropped 0,
 74/74 insns) AND IS A CHEAT. A fabricated 14-argument extern is called in dead code
 after the function's return. GCC expands the call (setting
 current_function_outgoing_args_size to 56) and jump.c then deletes the unreachable
 block, so the outgoing-args block survives with no argument stores in it. It fails
 cheat-checklist T1 (no observable effect), T2 (no human writes an unreachable call to
 a function that does not exist in the game) and T5 (no sanctioned family covers
 dead-code-to-inflate-outgoing-args; first reach). NOT PROPOSED. Its value is the
 structural fact it proves - see hypotheses.md s17.
 */
extern void bb2_argprobe14(s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32);
void func_800480C0(s32 arg0, s32 arg1, s16 arg2, s16 arg3, s16 arg4, s16 arg5)
{
    u32 *p;
    s32 base_addr;
    s32 count;
    s32 new_var;
    base_addr = arg0;
    p = (u32 *)arg0;
    arg0 = 0; /* FAKE: breaks the $a0 == base_addr value association so cse2 cannot canonicalise the two base copies, mechanism: cse.c canonical-register substitution, lever-exhaustion: hypotheses.md s1-s3 */
    p = (u32 *)((s32)p + (((s32)(arg1 << 16)) >> 14));
    p = (u32 *)(base_addr + (((*p) >> 2) << 2));
    count = *(p++);
    if (count != 0) {
        s32 sx_arg2;
        s32 sx_arg3;
        s32 sx_arg4;
        s32 sx_arg5;
        count--;
        sx_arg2 = arg2;
        sx_arg3 = arg3;
        sx_arg4 = arg4;
        sx_arg5 = arg5;
        do {
            u32 word;
            s16 a1v;
            s16 a2v;
            s16 a3v;
            s16 v0v;
            word = *p;
            p = (u32 *)(((s32)p) + 4);
            a1v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a2v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            new_var = base_addr + (((u32)word >> 2) << 2);
            a3v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            v0v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            func_800482C8(new_var,
                          (s32)a1v + sx_arg2,
                          (s32)a2v + sx_arg3,
                          (s32)a3v + sx_arg4,
                          (s32)v0v + sx_arg5);
        } while ((count--) != 0);
    }
    return;
    bb2_argprobe14(1,2,3,4,5,6,7,8,9,10,11,12,13,14);
}
