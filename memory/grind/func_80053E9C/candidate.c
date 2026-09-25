extern s32 D_800A33F0;
extern s32 D_800A33F4;
extern s32 func_80052754(s32, s32, s32);
extern void func_80052C4C(s16 *, s32, s32, s32);
extern void func_80052CD4(s32 *, s32 *, s16);

#define WORK8(offset)  (*(s8 *)((u8 *)D_800A33F4 + (offset)))
#define WORK16(offset) (*(s16 *)((u8 *)D_800A33F4 + (offset)))
#define WORK32(offset) (*(s32 *)((u8 *)D_800A33F4 + (offset)))

s32 func_80053E9C(s32 arg0, s32 arg1) {
    s32 data;
    s32 count;
    s32 sign;
    s32 value;
    s32 normalized;
    s32 x0;
    s32 y0;
    s32 z0;
    s32 x1;
    s32 y1;
    s32 z1;
    s32 packed;
    s32 distance;
    s32 cross;
    s32 xEnd;
    s32 yEnd;
    u16 offset;

    if (arg0 < 0 || arg1 < 0 || arg0 >= 32 || arg1 >= 32) {
        return 0;
    }

    offset = *(u16 *)(D_800A33F0 + ((arg1 * 32 + arg0) * 2));
    WORK32(0xE0) = offset;
    if (offset == 0xFFFF) {
        return 0;
    }

    x0 = arg0 * 0x7D0 - 0x7D00;
    z0 = arg1 * 0x7D0 - 0x7D00;
    WORK16(0x4E) = WORK32(0x0C);
    WORK16(0x4C) = WORK32(0x08) - x0;
    WORK16(0x56) = WORK32(0x1C);
    WORK16(0x50) = WORK32(0x10) - z0;
    WORK16(0x58) = WORK32(0x20) - z0;
    WORK16(0x54) = WORK32(0x18) - x0;

    data = D_800A33F0 + offset;
    count = *(s16 *)data;
    count--;
    data += 2;
    if (count != -1) do {
        WORK32(0xD0) = *(s16 *)data;
        data += 2;
        WORK32(0xD4) = *(s16 *)data;
        data += 2;
        WORK32(0xD8) = *(s16 *)data;
        data += 2;
        WORK32(0xDC) = *(u16 *)data;
        packed = (*(s16 *)(data + 2) << 16) | *(u16 *)data;
        data += 4;

        WORK32(0xE4) = WORK32(0xD0) * WORK16(0x4C)
                     + WORK32(0xD4) * WORK16(0x4E)
                     + WORK32(0xD8) * WORK16(0x50) + packed;
        WORK32(0xDC) = packed;
        WORK32(0xE8) = WORK32(0xD0) * WORK16(0x54)
                     + WORK32(0xD4) * WORK16(0x56)
                     + WORK32(0xD8) * WORK16(0x58) + WORK32(0xDC);

        sign = 1;
        if (WORK32(0xE4) < 0) {
            sign = -1;
        }
        value = WORK32(0xE4);
        if (value < 0) {
            value = -value;
        }
        normalized = sign * (value >> 14);
        WORK32(0xE4) = normalized;

        sign = 1;
        if (WORK32(0xE8) < 0) {
            sign = -1;
        }
        value = WORK32(0xE8);
        if (value < 0) {
            value = -value;
        }
        normalized = sign * (value >> 14);
        WORK32(0xE8) = normalized;

        x0 = WORK32(0xE4);
        y0 = WORK32(0xE8);
        distance = x0 - y0;
        if (x0 >= 0 && y0 < 0) {
            WORK32(0xE0) = distance;
            x1 = ((WORK16(0x54) - WORK16(0x4C)) * WORK32(0xE4)) / distance
               + WORK16(0x4C);
            y1 = ((WORK16(0x56) - WORK16(0x4E)) * WORK32(0xE4)) / WORK32(0xE0)
               + WORK16(0x4E);
            z1 = ((WORK16(0x58) - WORK16(0x50)) * WORK32(0xE4)) / WORK32(0xE0)
               + WORK16(0x50);
            WORK32(0xA8) = x1;
            WORK32(0xAC) = y1;
            WORK32(0xB0) = z1;
            func_80052C4C((s16 *)data, WORK32(0xA8), WORK32(0xAC), z1);

            data += 18;
            value = *(u16 *)data;
            data += 2;
            {
            s32 vertexCount;
            s32 nextY;
            s16 header;
            header = value;
            vertexCount = header;
            vertexCount &= 0xFF;
            WORK32(0xCC) = header >> 8;
            vertexCount--;
            WORK32(0xB4) = *(s16 *)data;
            data += 2;
            nextY = *(s16 *)data;
            data += 2;
            WORK32(0xE0) = 1;
            WORK32(0xB8) = nextY;
            func_80052CD4((s32 *)((u8 *)D_800A33F4 + 0xC4),
                          (s32 *)((u8 *)D_800A33F4 + 0xC8), nextY);

            if (vertexCount != -1) {
            xEnd = WORK32(0xC4);
            yEnd = WORK32(0xC8);
            do {
                WORK32(0xBC) = *(s16 *)data;
                data += 2;
                WORK32(0xC0) = *(s16 *)data;
                cross = (xEnd - WORK32(0xB4))
                      * (WORK32(0xC0) - WORK32(0xB8))
                      - (yEnd - WORK32(0xB8))
                      * (WORK32(0xBC) - WORK32(0xB4));
                data += 2;
                if (cross > 0) {
                    WORK32(0xE0) = 0;
                    break;
                }
                vertexCount--;
                WORK32(0xB4) = WORK32(0xBC);
                WORK32(0xB8) = WORK32(0xC0);
            } while (vertexCount != -1);
            }
            if (vertexCount > 0) {
                data += vertexCount * 4;
            }
            if (WORK32(0xE0) != 0) {
                WORK32(0xE0) = func_80052754(WORK32(0xA8) - WORK16(0x4C),
                                             WORK32(0xAC) - WORK16(0x4E),
                                             WORK32(0xB0) - WORK16(0x50));
                if (WORK32(0xE0) < WORK32(0)) {
                    WORK16(0x48) = arg0;
                    WORK16(0x4A) = arg1;
                    WORK32(0x38) = WORK32(0xA8);
                    WORK32(0x3C) = WORK32(0xAC);
                    WORK32(0x40) = WORK32(0xB0);
                    WORK32(0x28) = WORK32(0xD0);
                    WORK32(0x2C) = WORK32(0xD4);
                    WORK32(0x30) = WORK32(0xD8);
                    WORK32(0x34) = WORK32(0xDC);
                    WORK32(0) = WORK32(0xE0);
                    WORK16(4) = WORK32(0xCC);
                }
            }
            }
        } else {
            data += 18;
            {
            s32 vertexCount;
            vertexCount = *(s16 *)data;
            data += 2;
            vertexCount &= 0xFF;
            data += (vertexCount + 1) * 4;
            }
        }
        count--;
    } while (count != -1);
    return WORK32(0) != 0x7FFFFFFF;
}

#undef WORK8
#undef WORK16
#undef WORK32
