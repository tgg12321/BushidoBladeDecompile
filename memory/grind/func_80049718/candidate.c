/* func_80049718 (text1b.c) — CLEAN candidate, floor 4 (HEAD 11). ZERO cheat-asm,
 * ZERO regfix (the 2 stripped rules are now inert). Layer-1 cheat-reviewer PASSED
 * both changes below.
 *
 * Two pure-C levers (cumulative from HEAD 11 -> 4):
 *  1) (prior session) removed no-op do{...}while(0) + if(0){} perturbers (11->6).
 *  2) (s1) block-2 store reorder: `obj+4=6` moved to follow obj+8/obj+0xA zero
 *     stores — matches target's late `sh v0,4(s2)` schedule (6->4). Independent
 *     stores to distinct offsets; behavior-identical.
 *
 * REMAINING residual (floor 4) = CLUSTER A, prologue `p_anim = &D_800EF980[arg0]`.
 *   Mechanism (RTL .greg, tmp/grind/func_80049718/s1/text1b.i.greg:7783-7800):
 *   our fork emits insn18 `(reg v0)=ashift s6,1` (INDEX) BEFORE insn20
 *   `(reg v1)=symbol_ref D_800EF980` (BASE), so local-alloc gives index->$v0,
 *   base->$v1, and `addu s0,v0,v1`. Target wants base->$v0, index->$v1,
 *   `addu s0,v1,v0`. Cause: `fold` canonicalizes the address PLUS so the
 *   symbol_ref (address constant) is operand-2 -> index materializes first.
 *   A gated 4->0 close exists (named base-pointer alias) — see
 *   candidate_gated_alias.c; it is pointer-alias-family and requires documented
 *   lever-exhaustion + /* FAKE */ per the cheat-reviewer, NOT yet satisfied. */
void func_80049718(s32 arg0, s32 arg1, s32 *arg2, s16 *arg3) {
    int new_var2;
    s16 sp10[3];
    s16 *p_anim;
    s32 var_s5;
    s32 var_s3;
    u8 *vehicle;
    int new_var;
    u8 *obj;
    u8 *part;
    u8 *new_var3;
    u8 *ot;
    p_anim = &D_800EF980[arg0];
    var_s3 = arg1;
    if ((*p_anim) < 0) {
        InitFadePanel();
    }
    obj = D_800A38B4;
    var_s5 = 0;
    obj[0] = 0;
    obj[1] = 0;
    new_var = (*p_anim) * 2;
    *((s16 *) (obj + 4)) = 6;
    *((s16 *) (obj + 8)) = 0;
    *((s32 *) (obj + 0xC)) = 0;
    *((s16 *) (obj + 0xA)) = 4;
    *((s16 *) (obj + 2)) = (s16) new_var;
    if (arg1 != 0) {
        if (arg1 == 1) {
            u8 *p_arg3 = (u8 *) arg3;
            *((u16 *) (obj + 0x10)) = *((u16 *) (p_arg3 + 0));
            *((u16 *) (obj + 0x12)) = *((u16 *) (p_arg3 + 2));
            *((u16 *) (obj + 0x14)) = *((u16 *) (p_arg3 + 4));
            g_anim_func_table((s16 *) (obj + 0x10), (s16 *) (obj + 0x18));
            *((s32 *) (obj + 0x2C)) = arg2[0];
            *((s32 *) (obj + 0x30)) = arg2[1];
            *((s32 *) (obj + 0x34)) = arg2[2];
        } else {
            var_s3 = arg1 & 0x7FFF;
            new_var2 = var_s3 & 1;
            vehicle = func_8004153C(var_s3 >> 1);
            part = vehicle + ((new_var2 * 0x68) + 0x7E4);
            *((s32 *) (part + 0x4C)) = ((*((s32 *) (part + 0x4C))) * (*((s16 *) (vehicle + 0x12)))) >> 12;
            *((s32 *) (part + 0x50)) = ((*((s32 *) (part + 0x50))) * (*((s16 *) (vehicle + 0x12)))) >> 12;
            *((s32 *) (part + 0x54)) = ((*((s32 *) (part + 0x54))) * (*((s16 *) (vehicle + 0x12)))) >> 12;
            p_anim = vehicle + 0x44;
            func_8007E4DC((s16 *) p_anim, (s16 *) (part + 0x38), (s16 *) (obj + 0x18));
            sp10[0] = (s16) (*((s32 *) (part + 0x4C)));
            sp10[1] = (s16) (*((s32 *) (part + 0x50)));
            sp10[2] = (s16) (*((s32 *) (part + 0x54)));
            func_8007ED6C((*((s32 *) (part + 0xC))) + 0x18, sp10, (s32 *) (obj + 0x2C));
            *((s32 *) (obj + 0x2C)) = (*((s32 *) (obj + 0x2C))) + (*((s32 *) ((*((u8 **) (part + 0xC))) + 0x2C)));
            *((s32 *) (obj + 0x30)) = (*((s32 *) (obj + 0x30))) + (*((s32 *) ((*((u8 **) (part + 0xC))) + 0x30)));
            *((s32 *) (obj + 0x34)) = (*((s32 *) (obj + 0x34))) + (*((s32 *) ((*((u8 **) (part + 0xC))) + 0x34)));
            var_s3 = var_s3 | 0x8000;
            *((_struct_copy_func49718 *) (part + 0x18)) = *((_struct_copy_func49718 *) (obj + 0x18));
            var_s5 = *((s16 *) (vehicle + 0x1A84));
        }
        ot = D_800A3820;
        D_800A3820 = ot + 4;
        *((u8 **) ot) = obj;
        obj += 0x68;
        if (var_s3 != 1) {
            s32 anim_v = D_800EF980[arg0];
            obj[0] = 3;
            obj[1] = 0;
            *((s32 *) (obj + 0x58)) = var_s5;
            new_var3 = D_800A3820;
            ot = new_var3;
            *((s32 *) (obj + 0xC)) = (s32) (obj - 0x68);
            *((s16 *) (obj + 6)) = 1;
            *((s16 *) (obj + 8)) = 0;
            *((s16 *) (obj + 0xA)) = 0;
            *((s16 *) (obj + 4)) = 6;
            *((s16 *) (obj + 2)) = (s16) ((anim_v * 2) + 1);
            D_800A3820 = ot + 4;
            *((u8 **) ot) = obj;
            obj += 0x68;
        }
        D_800A38B4 = obj;
    }
}
