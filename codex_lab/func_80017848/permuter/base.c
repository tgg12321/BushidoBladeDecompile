typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

extern s32 math_Distance3D(s32 *a0, s32 *a1);

s32 func_80017848(u8 *ctx, s32 arg1, s32 slot_a, s32 slot_b) {
    register u8 *ctxp asm("s2") = ctx;
    register s32 saved_arg1 asm("s5") = arg1;
    register s32 sa asm("s4") = slot_a;
    register s32 sb asm("s3") = slot_b;
    register u8 *slot_a_ptr asm("s0");
    register u8 *slot_b_ptr asm("s1");
    u8 *slots;
    u8 *links;
    s32 i;
    s32 dist;

    if (sa == sb) {
        return 0;
    }

    slots = *(u8 **)(ctxp + 0xC);
    if (*(s32 *)(slots + (sa << 6) + 0x18) >= 0) {
        if (*(s32 *)(slots + (sb << 6) + 0x18) >= 0) {
            return 0;
        }
    }

    slot_a_ptr = slots + (sa << 6);
    if (*(s32 *)(slot_a_ptr + 0x1C) > 0) {
        i = 0;
        do {
            links = *(u8 **)(ctxp + 0x10);
            if (*(u16 *)(links + (*(u8 *)(slot_a_ptr + 0x24 + i) << 4) + 4) == sb) {
                return 0;
            }
            i++;
        } while (i < *(s32 *)(slot_a_ptr + 0x1C));
    }

    if (*(s32 *)(slot_a_ptr + 0x20) > 0) {
        i = 0;
        do {
            links = *(u8 **)(ctxp + 0x10);
            if (*(s16 *)(links + (*(u8 *)(slot_a_ptr + 0x2C + i) << 4) + 6) == sb) {
                return 0;
            }
            i++;
        } while (i < *(s32 *)(slot_a_ptr + 0x20));
    }

    slot_a_ptr = slots + (sa << 6);
    slot_b_ptr = slots + (sb << 6);
    dist = math_Distance3D((s32 *)slot_a_ptr, (s32 *)slot_b_ptr);
    i = *(s16 *)(ctxp + 0x6);
    links = *(u8 **)(ctxp + 0x10) + (i << 4);
    *(s32 *)(links + 0x0) = dist;
    *(s32 *)(links + 0x8) = dist * 3;
    *(s32 *)(links + 0xC) = saved_arg1;
    *(s32 *)(links + 0x4) = (sa << 16) | sb;

    i = *(s32 *)(slot_a_ptr + 0x1C);
    *(s32 *)(slot_a_ptr + 0x1C) = i + 1;
    *(u8 *)(slot_a_ptr + 0x24 + i) = *(u16 *)(ctxp + 0x6);

    i = *(s32 *)(slot_b_ptr + 0x20);
    *(s32 *)(slot_b_ptr + 0x20) = i + 1;
    *(u8 *)(slot_b_ptr + 0x2C + i) = *(u16 *)(ctxp + 0x6);

    *(u16 *)(ctxp + 0x6) = *(u16 *)(ctxp + 0x6) + 1;
    return 1;
}

