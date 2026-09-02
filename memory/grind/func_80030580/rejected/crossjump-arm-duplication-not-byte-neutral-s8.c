/* REJECTED s8(forensics): FRONTIER-3 post-reload cross-jump route - duplicating real statements into the four tbl[0] arms so jump2 tail-merges them after reload has sized the frame. Measured: duplicating the existing obj+0x50/obj+5/obj+0 tail into all four arms costs bodydiff=17 (jump2 does NOT re-merge it to zero), and a Judge-folded re-store duplicated into 2/3/4 arms costs bodydiff=35..53 while buying only ONE extra slot (vars=16) no matter how many copies. The route cannot be byte-neutral: cross-jumping dedups copies, it never deletes the last one. Representative shape below (cj_jt4: vars=16 bodydiff=44). */
s32 *func_80030580(s32 *arg0, s32 arg1) {
    u8 *obj;
    u8 *src = (u8 *)arg0;
    s16 *tbl;
    s32 i;

    obj = (u8 *)&D_80106A78;
    for (i = 0; i < 12; i++, obj += 0x64) {
        if (*(s16 *)(obj + 2) == -1 && *(u8 *)(obj + 0xA) == 0xFF) break;
    }
    *(u8 *)(obj + 0xA) = i;
    *(s16 *)(obj + 2) = arg1;
    *(u8 *)(obj + 7) = 0;
    *(u8 *)(obj + 8) = 0;
    *(u8 *)(obj + 4) = 1;
    *(u8 *)(obj + 6) = *(u16 *)(src + 4);
    *(s32 *)(obj + 0x2C) = *(s32 *)(src + 0xF4);
    *(s32 *)(obj + 0x30) = *(s32 *)(src + 0xF8) - *(s16 *)(src + 0x1A) / 32;
    *(s32 *)(obj + 0x34) = *(s32 *)(src + 0xFC);
    tbl = &D_8008E194 + arg1 * 7;
    *(s32 *)(obj + 0x44) = ((&Judge)[*(u16 *)(src + 0x1CA) & 0xFFF] * tbl[2]) >> 12;
    *(s32 *)(obj + 0x48) = tbl[3];
    *(s32 *)(obj + 0x4C) = ((&Judge)[(*(s16 *)(src + 0x1CA) + 0x400) & 0xFFF] * tbl[2]) >> 12;
    *(s32 *)(obj + 0x2C) += *(s32 *)(obj + 0x44);
    *(s32 *)(obj + 0x30) += *(s32 *)(obj + 0x48);
    *(s32 *)(obj + 0x34) += *(s32 *)(obj + 0x4C);
    *(s32 *)(obj + 0x2C) += *(s32 *)(obj + 0x44) / 2;
    *(s32 *)(obj + 0x30) += *(s32 *)(obj + 0x48) / 2;
    *(s32 *)(obj + 0x34) += *(s32 *)(obj + 0x4C) / 2;
    *(Vec3i *)(obj + 0x38) = *(Vec3i *)(obj + 0x2C);
    *(s16 *)(obj + 0x54) = 0;
    *(u16 *)(obj + 0x56) = *(u16 *)(src + 0x1CA);
    *(s16 *)(obj + 0x58) = 0;
    if (tbl[0] == 1) {
        *(s16 *)(obj + 0x5C) = 0;
        *(u16 *)(obj + 0x5E) = *(u16 *)(tbl + 4);
        *(s16 *)(obj + 0x60) = 0;
        *(s32 *)(obj + 0x4C) = ((&Judge)[(*(s16 *)(src + 0x1CA) + 0x400) & 0xFFF] * tbl[2]) >> 12;
        *(s32 *)(obj + 0x50) = 1;
        *(u8 *)(obj + 5) = 0;
        *(s16 *)obj = 0;
    } else if (tbl[0] == 2) {
        *(u16 *)(obj + 0x5C) = *(u16 *)(tbl + 4);
        *(s16 *)(obj + 0x5E) = 0;
        *(s16 *)(obj + 0x60) = 0;
        *(s32 *)(obj + 0x4C) = ((&Judge)[(*(s16 *)(src + 0x1CA) + 0x400) & 0xFFF] * tbl[2]) >> 12;
        *(s32 *)(obj + 0x50) = 1;
        *(u8 *)(obj + 5) = 0;
        *(s16 *)obj = 0;
    } else if (tbl[0] == 3) {
        *(s16 *)(obj + 0x5C) = 0;
        *(u16 *)(obj + 0x5E) = *(u16 *)(tbl + 4);
        *(s16 *)(obj + 0x60) = 0;
        *(s32 *)(obj + 0x4C) = ((&Judge)[(*(s16 *)(src + 0x1CA) + 0x400) & 0xFFF] * tbl[2]) >> 12;
        *(s32 *)(obj + 0x50) = 1;
        *(u8 *)(obj + 5) = 0;
        *(s16 *)obj = 0;
    } else {
        *(s16 *)(obj + 0x5C) = 0;
        *(s16 *)(obj + 0x5E) = 0;
        *(s16 *)(obj + 0x60) = 0;
        *(s32 *)(obj + 0x4C) = ((&Judge)[(*(s16 *)(src + 0x1CA) + 0x400) & 0xFFF] * tbl[2]) >> 12;
        *(s32 *)(obj + 0x50) = 1;
        *(u8 *)(obj + 5) = 0;
        *(s16 *)obj = 0;
    }
    return (s32 *)obj;
}
