void func_80017FA0(s32 *a0) {
    s32 _frame;
    s32 temp;
    s32 *ptr;

    temp = a0[3];
    if (temp == 0) {
        goto end;
    }
    ptr = (s32 *)temp;

    {
        s32 val = ptr[0] << 7;
        asm volatile("" : "=r"(val) : "0"(val));
        *(volatile s32 *)0x1F8000B8 = val;
    }

    {
        s32 i = 0;
        if (ptr[1] > 0) {
            s32 *p68 = ptr;
            volatile s32 *ac_base = (volatile s32 *)0x1F800000;
            s32 sp_off = 0;
            do {
                s32 j;
                s32 data_off;
                s32 sp_inner;

                j = 0;
                data_off = i << 5;
                sp_inner = sp_off;
                do {
                    s32 *dp = (s32 *)((u8 *)ptr + data_off);
                    *(volatile s32 *)(0x1F800064 + sp_inner) = dp[2] << 2;
                    data_off += 0x10;
                    *(volatile s32 *)(0x1F800068 + sp_inner) = dp[3] << 2;
                    j++;
                    *(volatile s32 *)(0x1F80006C + sp_inner) = dp[4] << 2;
                    sp_inner += 0xC;
                } while (j < 2);

                ac_base[0x2B] = *(s32 *)((u8 *)p68 + 0x68) << 2;
                p68 = (s32 *)((u8 *)p68 + 4);
                sp_off += 0x18;
                i++;
                ac_base++;
            } while (i < ptr[1]);
        }
    }

    *(volatile s32 *)0x1F800060 = ((s32 *)a0[3])[1];

end:
    asm volatile("" : : "m"(_frame));
}
