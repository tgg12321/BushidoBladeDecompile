void func_8003F824(u8 *arg0, s32 arg1)
{
    register u8 *arg0_s4 asm("s4") = arg0;
    register s32 arg1_s7 asm("s7") = arg1;
    register u8 *s3 asm("s3");
    s16 *new_var;
    register s16 *s1 asm("s1");
    register u8 *s0 asm("s0");
    s32 s5;
    u8 *s2;
    s32 cur;
    u8 *dst;
    s32 flag;
    s32 ret;
    u8 *new_var2;
    u8 *new_var7;
    s32 new_var5;
    s16 new_var3;

    s3 = *((u8 **)(arg0_s4 + 0x24));
    if (s3 == 0) goto done;
    s1 = *((s16 **)(arg0_s4 + 0x28));
    s0 = s3 + 0x468;
    if (*s1 == -3) {
        *((s32 *)(arg0_s4 + 0x24)) = 0;
        goto done;
    }
    *((u16 *)(s3 + 0x0)) = 0;
    *((u16 *)(s3 + 0x2)) = 0;
    *((u16 *)(s3 + 0x4)) = 0;
    *((u16 *)(s3 + 0x6)) = 0;
    new_var3 = (s16)(*((u16 *)s1));
    if (*s1 == -3) goto check_arg1;
    s5 = 0;
    new_var5 = -2;
outer_loop:
    if (new_var3 == new_var5) goto past_marker;
    new_var = (s16 *)s3;
    if (*((s16 *)s3) >= 5) {
        func_80052C10();
    }
    cur = (*new_var) * 0xD0 + 8;
    s2 = s3 + cur;
    new_var2 = *((u8 **)(arg0_s4 + 0x1A34 + s5 * 4));
    *((u8 **)(s2 + 0x18)) = s0;
    s0 = new_var2;
    *((u8 **)(s2 + 0x14)) = s0;
    ret = (s32)func_8003FA24((s32 *)s2);
    dst = s3 + (s32)(*((s16 *)s3)) * 16;
    new_var7 = s2;
    do {
        s32 t0 = *((s32 *)(new_var7 + 0x4));
        s32 t1 = *((s32 *)(new_var7 + 0x8));
        s32 t2 = *((s32 *)(new_var7 + 0xC));
        s32 t3 = *((s32 *)(new_var7 + 0x10));

        *((s32 *)(dst + 0x418)) = t0;
        *((s32 *)(dst + 0x41C)) = t1;
        *((s32 *)(dst + 0x420)) = t2;
        *((s32 *)(dst + 0x424)) = t3;

        *s0 = 0xD;
        (*((u16 *)(s3 + 0x0)))++;
        flag = *((s16 *)s1);
        s0 = (u8 *)ret;
        if (flag < 0) goto check_ctrl;
        s1++;
walk_nonneg:
        cur = *((s16 *)s1);
        s1++;
        if (cur >= 0) goto walk_nonneg;
        s1--;
    } while (0);
check_ctrl:
    if (*((s16 *)s1) == -1) {
        s1++;
        func_8003FECC((s32 *)arg0_s4, (s32 *)new_var7, s1);
    }
    flag = *((s16 *)s1);
skip_marker:
    if (flag == new_var5) goto past_marker;
    s1++;
walk_nonmark:
    if (*((s16 *)s1) != new_var5) { s1++; goto walk_nonmark; }
past_marker:
    s1++;
    new_var3 = *((s16 *)s1);
    s5 = new_var3;
    if (s5 != -3) { s5++; goto outer_loop; }
check_arg1:
    if (arg1_s7 != 0) {
        func_80045A28(
            (s32)(*((s16 *)(arg0_s4 + 0x4))),
            (s32)(s0 - *((u8 **)(arg0_s4 + 0x1C))));
    }
done:
    return;
}
