extern u8 *D_800A38B4;
extern s32 (*g_anim_func_table[])(s16 *, s16 *);
extern MATRIX *MulMatrix0(MATRIX *, MATRIX *, MATRIX *);

void func_80049718(s32 arg0, s32 arg1, s32 *arg2, s16 *arg3) {
    int new_var2;
    SVECTOR rot;
    s16 *p_anim;
    s32 var_s5;
    s32 var_s3;
    u8 *vehicle;
    int new_var;
    u8 *obj;
    u8 *part;
    u8 *new_var3;
    u8 *ot;
    {
        s16 *tbl = D_800EF980;
        p_anim = tbl + arg0;
    }
    var_s3 = arg1;
    if ((*p_anim) < 0) {
        func_80052C10();
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
            g_anim_func_table[0]((s16 *) (obj + 0x10), (s16 *) (obj + 0x18));
            *((s32 *) (obj + 0x2C)) = arg2[0];
            *((s32 *) (obj + 0x30)) = arg2[1];
            *((s32 *) (obj + 0x34)) = arg2[2];
        } else {
            var_s3 = arg1 & 0x7FFF;
            new_var2 = var_s3 & 1;
            vehicle = (u8 *)func_8004153C(var_s3 >> 1);
            part = vehicle + ((new_var2 * 0x68) + 0x7E4);
            *((s32 *) (part + 0x4C)) = ((*((s32 *) (part + 0x4C))) * (*((s16 *) (vehicle + 0x12)))) >> 12;
            *((s32 *) (part + 0x50)) = ((*((s32 *) (part + 0x50))) * (*((s16 *) (vehicle + 0x12)))) >> 12;
            *((s32 *) (part + 0x54)) = ((*((s32 *) (part + 0x54))) * (*((s16 *) (vehicle + 0x12)))) >> 12;
            MulMatrix0((MATRIX *) (vehicle + 0x44), (MATRIX *) (part + 0x38), (MATRIX *) (obj + 0x18));
            rot.vx = *((s32 *) (part + 0x4C));
            rot.vy = *((s32 *) (part + 0x50));
            rot.vz = *((s32 *) (part + 0x54));
            ApplyMatrix((MATRIX *)(*((s32 *) (part + 0xC)) + 0x18), &rot, (VECTOR *) (obj + 0x2C));
            *((s32 *) (obj + 0x2C)) = (*((s32 *) (obj + 0x2C))) + (*((s32 *) ((*((u8 **) (part + 0xC))) + 0x2C)));
            *((s32 *) (obj + 0x30)) = (*((s32 *) (obj + 0x30))) + (*((s32 *) ((*((u8 **) (part + 0xC))) + 0x30)));
            *((s32 *) (obj + 0x34)) = (*((s32 *) (obj + 0x34))) + (*((s32 *) ((*((u8 **) (part + 0xC))) + 0x34)));
            var_s3 = var_s3 | 0x8000;
            *(MATRIX *)(part + 0x18) = *(MATRIX *)(obj + 0x18);
            var_s5 = *((s16 *) (vehicle + 0x1A84));
        }
        ot = (u8 *)D_800A3820;
        D_800A3820 = (s32)(ot + 4);
        *((u8 **) ot) = obj;
        obj += 0x68;
        if (var_s3 != 1) {
            s32 anim_v = D_800EF980[arg0];
            obj[0] = 3;
            obj[1] = 0;
            *((s32 *) (obj + 0x58)) = var_s5;
            new_var3 = (u8 *)D_800A3820;
            ot = new_var3;
            *((s32 *) (obj + 0xC)) = (s32) (obj - 0x68);
            *((s16 *) (obj + 6)) = 1;
            *((s16 *) (obj + 8)) = 0;
            *((s16 *) (obj + 0xA)) = 0;
            *((s16 *) (obj + 4)) = 6;
            *((s16 *) (obj + 2)) = (s16) ((anim_v * 2) + 1);
            D_800A3820 = (s32)(ot + 4);
            *((u8 **) ot) = obj;
            obj += 0x68;
        }
        D_800A38B4 = obj;
    }
}
