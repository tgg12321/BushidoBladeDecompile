void func_80027640(s32 arg0)
{
    volatile s32 sp10;
    volatile s32 _g14;
    volatile s32 sp18;
    volatile s32 _g1C;
    int new_var;
    volatile s32 sp20;
    volatile s32 _g24;
    volatile s32 sp28;
    volatile s32 _g2C;
    s32 temp_s2;
    s32 v0_v;
    s32 var_a3;
    s32 v1_v;
    u8 cnt;
    s32 *stage_ptr;
    s32 s0_addr;
    temp_s2 = *((s16 *) (arg0 + 4));
    stage_ptr = (s32 *)stage_GetDataPtr();
    cnt = *((u8 *) (arg0 + 0x34C));
    if (((0, cnt)) < 0x40U) {
        *((u8 *) (arg0 + 0x34C)) = cnt + 1;
    }
    var_a3 = 0x3C - (((*((u8 *) (arg0 + 0x34C))) - 1) * 4);
    if (var_a3 < 0xA) {
        var_a3 = 0xA;
    }
    if (D_800A36A4 == 3) {
        v1_v = 0x2EE0;
        if ((*((s32 *) ((*((s32 *) arg0)) + 0xF4))) >= 0x3E9) {
            v1_v = -0x2710;
        }
        sp20 = v1_v;
        {
            s32 a2_v = 0x1770;
            if ((*((s32 *) ((*((s32 *) arg0)) + 0xFC))) > 0) {
                a2_v = -0x1770;
            }
            sp28 = a2_v;
            {
                s32 cnv = 0x64 - var_a3;
                sp10 = ((v1_v * var_a3) + ((*((s32 *) (arg0 + 0xF4))) * cnv)) / 100;
                v0_v = ((a2_v * var_a3) + ((*((s32 *) (arg0 + 0xFC))) * cnv)) / 100;
            }
        }
    } else {
        s32 *p = (s32 *) (((s32) stage_ptr) + (((D_800A36A4 * 0xC) + (temp_s2 * 3)) * 2));
        sp10 = *((s16 *) p);
        v0_v = *((s16 *) (((s32) p) + 4));
    }
    sp18 = v0_v;
    {
        s32 dx = sp10 - (*((s32 *) (arg0 + 0xF4)));
        sp10 = dx;
        sp18 = v0_v - (*((s32 *) (arg0 + 0xFC)));
        *((s32 *) (arg0 + 0xF4)) = (*((s32 *) (arg0 + 0xF4))) + dx;
        *((s32 *) (arg0 + 0xFC)) = (*((s32 *) (arg0 + 0xFC))) + sp18;
        *((s32 *) (arg0 + 0xD8)) = (*((s32 *) (arg0 + 0xD8))) + sp10;
        *((s32 *) (arg0 + 0xE0)) = (*((s32 *) (arg0 + 0xE0))) + sp18;
        s0_addr = arg0 + 0x5E;
        *((s32 *) (arg0 + 0xB8)) = (*((s32 *) (arg0 + 0xB8))) + sp10;
        new_var = (*((s32 *) (arg0 + 0xC0))) + sp18;
        *((s32 *) (arg0 + 0x104)) = 0;
        *((s32 *) (arg0 + 0x108)) = 0;
        *((s32 *) (arg0 + 0x10C)) = 0;
        *((s32 *) (arg0 + 0x134)) = 0;
        *((s32 *) (arg0 + 0x138)) = 0;
        *((s32 *) (arg0 + 0x13C)) = 0;
        *((s32 *) (arg0 + 0xC0)) = new_var;
    }
    {
        void *r1 = func_80021424(arg0, *((u16 *) (*((s32 *) (arg0 + 0x50)))), s0_addr);
        void *r2 = func_80021424(arg0, *((u16 *) (((s32) r1) + 0x3A)), s0_addr);
        func_80021A98((s16) temp_s2, r2, *((s16 *) (arg0 + 0x5E)));
    }
    func_80032854(*((s16 *) (arg0 + 4)), 0x30, (s32 *)(arg0 + 0xF4), 0);
}
