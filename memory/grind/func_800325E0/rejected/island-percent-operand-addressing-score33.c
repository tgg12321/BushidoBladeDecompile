void func_800325E0(s32 arg0, s32 *arg1) {
    s32 sp_tmp;
    s32 dx, dy, dz;
    u32 dist_volume;
    s32 distance_scale;
    s16 listener_angle;
    s32 projected_pan;
    u32 pan_sign;
    s32 pan_L;
    s32 pan_R;
    s32 L_scaled, R_scaled;

    dx = *(s32 *)((u8 *)D_800A36B4 + 0x20) - arg1[0];
    dy = *(s32 *)((u8 *)D_800A36B4 + 0x24) - arg1[1];
    dz = *(s32 *)((u8 *)D_800A36B4 + 0x28) - arg1[2];

    if (((u32)(dx + 0x9C40) > 0x13880U) || ((u32)(dz + 0x9C40) > 0x13880U)) {
        dist_volume = 0x9C40;
    } else {
        u32 dist_sq = (u32)((dx * dx) + (dy * dy) + (dz * dz));
        if (dist_sq < 0x400U) {
            dist_volume = (u32)(u8)(*(((u8 *)&D_8008D118) + dist_sq)) >> 3;
        } else {
            u32 clz;
            /* Hand-written GTE leading-zero-count block (LZCS in, LZCR out) —
             * canonical inline asm, identical to the user-authorized block in
             * the matched sibling func_800274BC (src/code6cac_b.c:292). */
            __asm__ volatile(
                "addu   $t4, %1, $zero\n"
                "mtc2   $t4, $30\n"        /* LZCS <- dist_sq */
                "nop\n"
                "nop\n"
                "addu   $t4, %2, $zero\n"  /* &sp_tmp */
                "swc2   $31, 0($t4)\n"     /* sp_tmp <- LZCR */
                : "=m"(sp_tmp)
                : "r"(dist_sq), "r"(&sp_tmp)
                : "$12");
            clz = sp_tmp;
            {
                u32 v0_m = (u32)-2;
                u32 v1_m;
                u32 idx;
                u32 hi;
                v0_m &= clz;
                v1_m = 0x16 - v0_m;
                idx = dist_sq >> v1_m;
                v1_m = v1_m >> 1;
                hi = (u32)((u8)(*((&D_8008D118) + idx)));
                dist_volume = (hi << 16) >> (0x13 - v1_m);
            }
        }
    }

    distance_scale = (s32)((0x9C40 - dist_volume) << 11) / 32000;
    if (distance_scale < 0) {
        distance_scale = 0;
    }

    listener_angle = *(s16 *)((u8 *)D_800A36B4 + 0x12);
    projected_pan = (s32)((dx * (s32)*((&Judge) + (((listener_angle + 0x400) & 0xFFF))))
                       + (dz * (s32)*((&Judge) + ((listener_angle & 0xFFF))))) >> 12;
    pan_sign = ~(u32)projected_pan >> 31;
    if (projected_pan < 0) {
        projected_pan = -projected_pan;
    }

    pan_L = (s32)((0x7530 - projected_pan) << 12) / 30000;
    pan_R = (s32)((0x2710 - projected_pan) << 12) / 10000;
    if (pan_L < 0) pan_L = 0;
    if (pan_R < 0) pan_R = 0;

    {
        s32 tmp = pan_L;
        if (pan_sign != 0) {
            pan_L = pan_R;
            pan_R = tmp;
        }
    }

    L_scaled = (s32)(distance_scale * pan_L) >> 16;
    R_scaled = (s32)(distance_scale * pan_R) >> 16;
    if (L_scaled >= 0x80) L_scaled = 0x7F;
    if (R_scaled >= 0x80) R_scaled = 0x7F;

    func_8005C650(arg0, L_scaled, R_scaled);
}
