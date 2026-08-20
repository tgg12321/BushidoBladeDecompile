void func_800485EC(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5)
{
    register s32 s1 asm("s1") = arg0;
    register s32 s0 asm("s0") = arg1;
    register s32 s2 asm("s2") = arg2;
    register s32 s3 asm("s3") = arg3;
    s32 v1;
    s32 flags;
    v1 = *((u8 *) s1);
    if (v1 != 0x10) {
        return;
    }
    s1 += 4;
    flags = *((s32 *) s1);
    s1 += 4;
    *((s16 *) (s0 + 0)) = (s16)(flags & 7);
    if (flags & 8) {
        register u32 size asm("v0");
        v1 = s1;
        size = *((u32 *) v1);
        *((u16 *) (s0 + 0xA)) = arg4;
        *((u16 *) (s0 + 0xC)) = arg5;
        s1 = v1 + ((size >> 2) << 2);
        v1 += 8;
        *((u16 *) (s0 + 0x10)) = *((u16 *) (v1 + 2));
        *((s16 *) (s0 + 0xE)) = (s16)*((s32 *)v1);
        *((s32 *) (s0 + 0x1C)) = v1 + 4;
        *((s16 *) (s0 + 0x14)) = GetClut(*((s16 *) (s0 + 0xA)), *((s16 *) (s0 + 0xC)));
    } else {
        *((s16 *) (s0 + 0x14)) = 0;
    }
    v1 = s1 + 8;
    asm volatile("" : "=r"(v1) : "0"(v1));
    *((s16 *) (s0 + 2)) = (s16)s2;
    *((s16 *) (s0 + 4)) = (s16)s3;
    *((u16 *) (s0 + 8)) = *((u16 *) (v1 + 2));
    *((s16 *) (s0 + 6)) = (s16)*((s32 *)v1);
    *((s32 *) (s0 + 0x18)) = v1 + 4;
    *((s16 *) (s0 + 0x12)) = GetTPage(*((s16 *) (s0 + 0)), 0, (*((s16 *) (s0 + 2))) & 0xFFC0, (*((s16 *) (s0 + 4))) & 0xFF00);
}
