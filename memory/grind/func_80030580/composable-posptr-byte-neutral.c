s32 *func_80030580(s32 *arg0, s32 arg1) {
    u8 *obj;
    u8 *src = (u8 *)arg0;
    s16 *tbl;
    s32 i;
    Vec3i *pos, *vel;

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
    pos = (Vec3i *)(obj + 0x2C);
    vel = (Vec3i *)(obj + 0x44);
    pos->x = *(s32 *)(src + 0xF4);
    pos->y = *(s32 *)(src + 0xF8) - *(s16 *)(src + 0x1A) / 32;
    pos->z = *(s32 *)(src + 0xFC);
    tbl = &D_8008E194 + arg1 * 7;
    vel->x = ((&Judge)[*(u16 *)(src + 0x1CA) & 0xFFF] * tbl[2]) >> 12;
    vel->y = tbl[3];
    vel->z = ((&Judge)[(*(s16 *)(src + 0x1CA) + 0x400) & 0xFFF] * tbl[2]) >> 12;
    pos->x += vel->x;
    pos->y += vel->y;
    pos->z += vel->z;
    pos->x += vel->x / 2;
    pos->y += vel->y / 2;
    pos->z += vel->z / 2;
    *(Vec3i *)(obj + 0x38) = *pos;
    *(s16 *)(obj + 0x54) = 0;
    *(u16 *)(obj + 0x56) = *(u16 *)(src + 0x1CA);
    *(s16 *)(obj + 0x58) = 0;
    if (tbl[0] == 1) {
        *(s16 *)(obj + 0x5C) = 0;
        *(u16 *)(obj + 0x5E) = *(u16 *)(tbl + 4);
        *(s16 *)(obj + 0x60) = 0;
    } else if (tbl[0] == 2) {
        *(u16 *)(obj + 0x5C) = *(u16 *)(tbl + 4);
        *(s16 *)(obj + 0x5E) = 0;
        *(s16 *)(obj + 0x60) = 0;
    } else if (tbl[0] == 3) {
        *(s16 *)(obj + 0x5C) = 0;
        *(u16 *)(obj + 0x5E) = *(u16 *)(tbl + 4);
        *(s16 *)(obj + 0x60) = 0;
    } else {
        *(s16 *)(obj + 0x5C) = 0;
        *(s16 *)(obj + 0x5E) = 0;
        *(s16 *)(obj + 0x60) = 0;
    }
    *(s32 *)(obj + 0x50) = 1;
    *(u8 *)(obj + 5) = 0;
    *(s16 *)obj = 0;
    return (s32 *)obj;
}
