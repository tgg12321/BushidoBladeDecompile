/* REJECTED (s9, rederive) - variant R16.
 * vars=0, insns=107. Pointer walk for the three mult reads, subscripts D_80099D3C[kidx+3..5] for the three tail halfwords. CSE rewrites every tail subscript relative to the live `src`, so no second symbol_ref use exists.
 * Measured with tmp/grind/func_80049A2C/s9/run.sh (real cpp + instrumented cc1,
 * full text1b.c TU). Target frame is .frame $sp,48 # vars= 8, regs= 5/0, args= 16.
 */
void func_80049A2C(s32 arg0, s32 arg1, s32 arg2) {
    u8 *new_var6;
    u8 *new_var5;
    s16 *new_var7;
    u8 temp_v1;
    u8 *new_var8;
    s32 kidx;
    s16 *p_anim;
    s16 new_var2;
    s16 *src;
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
    *((s16 *) (obj + 8)) = 0;
    *((s16 *) (obj + 0xA)) = 4;
    *((s16 *) (obj + 2)) = a1_val;
    kidx = (arg1 & 1) * 6;
    src = &D_80099D3C[kidx];
    *((s32 *) (obj + 0x4C)) = ((s32) ((*src) * (*((s16 *) (vehicle + 0x12))))) >> 12;
    src++;
    *((s32 *) (obj + 0x50)) = ((s32) ((*src) * (*((s16 *) (vehicle + 0x12))))) >> 12;
    src++;
    *((s32 *) (obj + 0x54)) = ((s32) ((*src) * (*((s16 *) (vehicle + 0x12))))) >> 12;
    src++;
    *((u16 *) (obj + 0x10)) = (u16) D_80099D3C[kidx + 3];
    *((u16 *) (obj + 0x12)) = (u16) D_80099D3C[kidx + 4];
    new_var2 = D_80099D3C[kidx + 5];
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
    *((s16 *) (obj + 8)) = 0;
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
