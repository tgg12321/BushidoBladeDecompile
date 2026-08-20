s32 _addque2(s32 (*arg0)(s32 *, s32), s32 *arg1, s32 arg2, s32 arg3) {
    s32 *var_a3;
    s32 temp_a0;
    s32 temp_a1;
    s32 var_a2;
    s32 var_v0;
    s32 var_v0_2;

    set_alarm();
    goto check_top;
err_loop:
    if (get_alarm() != 0) {
        return -1;
    }
    _exeque();
check_top:
    if (((D_8009BF78 + 1) & 0x3F) == D_8009BF7C) {
        goto err_loop;
    }
    {
        D_8009BF80 = SetIntrMask(0);
        D_8009BE7C = 1;
        if ((D_8009BE75 == 0) || ((D_8009BF78 == D_8009BF7C) && !(*D_8009BF54 & 0x01000000) && (D_8009BE80 == 0))) {
            do {
            } while (!(*D_8009BF48 & 0x04000000));
            arg0(arg1, arg3);
            D_8009BF68 = arg0;
            D_8009BF6C = arg1;
            D_8009BF70 = arg3;
            SetIntrMask(D_8009BF80);
            return 0;
        }
        DMACallback(2, _exeque);
        var_a2 = 0;
        if (arg2 != 0) {
            s32 v_shift;
            var_a3 = arg1;
            var_v0_2 = arg2;
loop_13:
            if (var_v0_2 < 0) {
                var_v0_2 += 3;
            }
            v_shift = var_v0_2 >> 2;
            temp_a0 = var_a2 * 4;
            if (var_a2 < v_shift) {
                temp_a1 = *var_a3;
                var_a3 += 1;
                var_a2 += 1;
                *(s32 *)(temp_a0 + ((D_8009BF78 * 0x60) + (s32)&D_8010368C)) = temp_a1;
                var_v0_2 = arg2;
                goto loop_13;
            }
            *(s32 **)((s32)&D_80103684 + (*(volatile s32 *)&D_8009BF78 * 0x60)) = (s32 *)((*(volatile s32 *)&D_8009BF78 * 0x60) + (s32)&D_8010368C);
        } else {
            *(s32 **)((s32)&D_80103684 + (D_8009BF78 * 0x60)) = arg1;
        }
        *(s32 *)((s32)&D_80103688 + (*(volatile s32 *)&D_8009BF78 * 0x60)) = arg3;
        *(s32 (**)(s32 *, s32))((s32)&D_80103680 + (*(volatile s32 *)&D_8009BF78 * 0x60)) = arg0;
        D_8009BF78 = (D_8009BF78 + 1) & 0x3F;
        SetIntrMask(D_8009BF80);
        _exeque();
        var_v0 = (D_8009BF78 - D_8009BF7C) & 0x3F;
        return var_v0;
    }
}
