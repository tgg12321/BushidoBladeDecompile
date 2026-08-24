s32 func_80017D84(void *a0) {
    register u8 *obj_base asm("a3");
    register void *saved_a0 asm("t0");
    s32 i;

    saved_a0 = a0;
    obj_base = &g_file_data_buf[0];
    for (i = 0; i < 8; i++) {
        if (*(s32 *)obj_base == 0) break;
        obj_base += 0x34;
    }
    if (i == 8) return -1;
    if (D_800A30E8 < i) {
        D_800A30E8 = i;
    }
    *(u16 *)(obj_base + 4) = *(u16 *)saved_a0;
    *(u32 *)obj_base = *(u32 *)((u8 *)saved_a0 + 4);
    {
        register u32 *ptr asm("v0");
        ptr = *(u32 **)((u8 *)saved_a0 + 0xC);
        {
            register s32 r0 asm("v1"), r1 asm("a0"), r2 asm("a1"), r3 asm("a2");
            r0 = ptr[0]; r1 = ptr[1]; r2 = ptr[2]; r3 = ptr[3];
            *(s32 *)(obj_base + 0x14) = r0;
            *(s32 *)(obj_base + 0x18) = r1;
            *(s32 *)(obj_base + 0x1C) = r2;
            *(s32 *)(obj_base + 0x20) = r3;
            r0 = ptr[4]; r1 = ptr[5]; r2 = ptr[6]; r3 = ptr[7];
            *(s32 *)(obj_base + 0x24) = r0;
            *(s32 *)(obj_base + 0x28) = r1;
            *(s32 *)(obj_base + 0x2C) = r2;
            *(s32 *)(obj_base + 0x30) = r3;
        }
        asm volatile("" ::: "memory");
    }
    {
        register s32 val1 asm("v1");
        register void *tmp_a0 asm("a0") = saved_a0;
        register u8 *tmp_a1 asm("a1");
        s32 val0;
        val0 = *(s16 *)((u8 *)saved_a0 + 2);
        *(s32 *)(obj_base + 8) = val0;
        asm volatile("" : "=r"(saved_a0) : "0"(saved_a0));
        val1 = *(volatile s32 *)((u8 *)saved_a0 + 0x10);
        val0 = *(s16 *)(obj_base + 4);
        asm volatile("move %0, %1
	sh $0, 6(%1)" : "=r"(tmp_a1) : "r"(obj_base), "r"(val1), "r"(val0));
        val0 <<= 6;
        *(s32 *)(obj_base + 0xC) = val1;
        val1 += val0;
        *(s32 *)(obj_base + 0x10) = val1;
        func_80017A44(tmp_a0, tmp_a1);
        return i;
    }
}
