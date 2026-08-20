/* PARTIAL WIN - session s8 variant F2. NOT a candidate; NOT rejected.
 *
 * This is the FIRST ordinary-C form of func_80049A2C that reproduces target's
 * frame exactly:  .frame $sp,48,$31  # vars= 8, regs= 6/0, args= 16, extra= 0
 * (target: addiu $sp,-0x30, five saves at 0x18..0x28). No dead local, no pad,
 * no register pin, no inline asm - the 8 bytes come from reload1.c:2404
 * alter_reg reserving a slot for a pseudo that combine.c:10836 orphaned onto a
 * codegen-free `(use (reg))`.
 *
 * COST (why it is not the candidate): sandbox --disable all = 50, 129 insns vs
 * target 126. The second index expression on D_80099D3C forces a SIXTH
 * callee-saved register (regs= 6/0 instead of target's 5/0), which adds the
 * extra sw/lw pair plus one more instruction. The frame total is unaffected by
 * the 6th save (compute_frame_size rounds gp_reg_size to 8, so 5 and 6 saves
 * both contribute 24) - that is why the frame is still exactly 48.
 *
 * NEXT SESSION: the existence question is closed; this is now a REGISTER
 * PRESSURE problem. Find a spelling of "two distinct variable indices into
 * D_80099D3C" whose second index does not need a live register across the
 * remaining stores, or move the trigger onto a symbol whose base register is
 * already live (D_800EF980 / D_80099CC8).
 */
void func_80049A2C(s32 arg0, s32 arg1, s32 arg2) {
    u8 *new_var6;
    u8 *new_var5;
    s16 *new_var7;
    u8 temp_v1;
    u8 *new_var8;
    s16 *p_anim;
    s16 new_var2;
    s16 *src;
    int new_var3;
    s32 kidx;
    u8 *obj;
    u8 *vehicle;
    s16 a1_val;
    u8 *ot;

    new_var6 = D_80099CC8;
    {
        u8 *p = new_var6 + (arg0 * 2);
        temp_v1 = p[arg2];
    }
    if (temp_v1 == 0xFF) {
        return;
    }
    new_var3 = 8;
    new_var8 = (u8 *) D_800EF980;
    p_anim = (s16 *) (new_var8 + (temp_v1 * 2));
    if ((*p_anim) < 0) {
        InitFadePanel();
    }
    vehicle = (u8 *) func_8004153C(arg1 >> 1);
    obj = D_800A38B4;
    obj[0] = 0;
    obj[1] = 0;
    a1_val = (*p_anim) * 2;
    *((s16 *) (obj + 4)) = 6;
    *((s16 *) (obj + new_var3)) = 0;
    *((s16 *) (obj + 0xA)) = 4;
    *((s16 *) (obj + 2)) = a1_val;
    kidx = (arg1 & 1) * 6;
    *((s32 *) (obj + 0x4C)) = ((s32) (D_80099D3C[kidx] * (*((s16 *) (vehicle + 0x12))))) >> 12;
    src = &D_80099D3C[kidx + 1];
    *((s32 *) (obj + 0x50)) = ((s32) ((*src) * (*((s16 *) (vehicle + 0x12))))) >> 12;
    src++;
    *((s32 *) (obj + 0x54)) = ((s32) ((*src) * (*((s16 *) (vehicle + 0x12))))) >> 12;
    src++;
    *((u16 *) (obj + 0x10)) = (u16) (*src);
    src++;
    *((u16 *) (obj + 0x12)) = (u16) (*src);
    new_var2 = src[1];
    *((s32 *) (obj + 0xC)) = (s32) (vehicle + 0x50C);
    *((s16 *) (obj + 6)) = 0;
    *((u16 *) (obj + 0x14)) = (u16) new_var2;
    func_800417D0((s32 *) obj);
    ot = D_800A3820;
    D_800A3820 = ot + 4;
    *((u8 **) ot) = obj;
    obj += 0x68;
    new_var5 = obj + 0xA;
    a1_val = (*p_anim) * 2;
    obj[0] = 3;
    *((s32 *) (obj + 0xC)) = (s32) (obj - 0x68);
    obj[1] = 0;
    new_var7 = (s16 *) (obj + 6);
    *((s16 *) (obj + new_var3)) = 0;
    *new_var7 = 1;
    *((s16 *) new_var5) = 0;
    *((s16 *) (obj + 4)) = 6;
    *((s16 *) (obj + 2)) = (s16) (a1_val + 1);
    *((s32 *) (obj + 0x58)) = (s32) (*((s16 *) (vehicle + 0x1A84)));
    ot = D_800A3820;
    D_800A3820 = ot + 4;
    *((u8 **) ot) = obj;
    D_800A38B4 = obj + 0x68;
}
