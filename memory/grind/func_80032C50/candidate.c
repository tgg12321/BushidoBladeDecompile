void func_80032C50(s32 obj, s32 kind) {
    s32 pos[3];
    s32 base;
    s32 base2;
    s32 row;
    s32 row2;
    s32 tri;
    s32 tri2;

    if (D_800A38DC == 3 && *(s16 *)(obj + 4) == 1 &&
        ((u32)(kind - 7) < 2 || (u32)(kind - 9) < 2 || (u32)(kind - 11) < 2 ||
         (u32)(kind - 13) < 2 || (u32)(kind - 15) < 2 || kind == 17)) {
        base2 = 0;
        base = 0;
    } else {
        base = *(s16 *)(obj + 4) * 40;
        base2 = *(s16 *)(*(u8 **)obj + 4) * 40;
    }

    if (D_800A38DC == 3 &&
        ((u32)(kind - 0x15) < 2 || (u32)(kind - 0x17) < 2 || (u32)(kind - 0x19) < 2 ||
         kind == 0x26 || (u32)(kind - 0x36) < 2 || (u32)(kind - 0x38) < 2 ||
         (u32)(kind - 0x3A) < 2 || kind == 0x47)) {
        if (*(s16 *)(obj + 4) == 1) {
            base += D_8008EBCC[D_800A384C];
        } else {
            base2 += D_8008EBCC[D_800A384C];
        }
    }

    row = base + *(u8 *)(obj + 0xB2) * 4;
    tri = *(u8 *)(obj + 0xB2) * 3;
    row2 = base2 + (*(u8 **)obj)[0xB2] * 4;
    tri2 = (*(u8 **)obj)[0xB2] * 3;

    switch (kind) {
    case 0:
        if (D_800A36A4 == 11) {
            if (*(s32 *)(obj + 0x1B0) - 600 < *(s32 *)(obj + 0x19C)) {
                pos[0] = *(s32 *)(obj + 0x198);
                pos[1] = *(s32 *)(obj + 0x1B0);
                pos[2] = *(s32 *)(obj + 0x1A0);
                func_80061A3C(pos, *(s16 *)(obj + 0x1BA),
                              D_8008EBE0[D_8008E5A8[*(s16 *)(obj + 0xC)]],
                              *(s16 *)(obj + 4));
                if ((0x60 >> *(u8 *)(obj + 0xB1)) & 1) {
                    func_80032854(*(s16 *)(obj + 4), 9, (u8 *)pos, 0);
                }
            }
        } else if (D_800A36A4 == 14) {
            if (((0x60 >> *(u8 *)(obj + 0xB1)) & 1) &&
                *(s32 *)(obj + 0x1B0) - 600 < *(s32 *)(obj + 0x19C)) {
                pos[0] = *(s32 *)(obj + 0x198);
                pos[1] = *(s32 *)(obj + 0x1B0);
                pos[2] = *(s32 *)(obj + 0x1A0);
                func_80032854(*(s16 *)(obj + 4), 9, (u8 *)pos, 0);
            }
        }
        break;
    case 1:
        if (D_800A36A4 == 11) {
            if (*(s32 *)(obj + 0x1B4) - 600 < *(s32 *)(obj + 0x1A8)) {
                pos[0] = *(s32 *)(obj + 0x1A4);
                pos[1] = *(s32 *)(obj + 0x1B4);
                pos[2] = *(s32 *)(obj + 0x1AC);
                func_80061A3C(pos, *(s16 *)(obj + 0x1C2),
                              D_8008EBE0[D_8008E5A8[*(s16 *)(obj + 0xC)]],
                              *(s16 *)(obj + 4));
                if ((0x60 >> *(u8 *)(obj + 0xB1)) & 1) {
                    func_80032854(*(s16 *)(obj + 4), 9, (u8 *)pos, 0);
                }
            }
        } else if (D_800A36A4 == 14) {
            if (((0x60 >> *(u8 *)(obj + 0xB1)) & 1) &&
                *(s32 *)(obj + 0x1B4) - 600 < *(s32 *)(obj + 0x1A8)) {
                pos[0] = *(s32 *)(obj + 0x1A4);
                pos[1] = *(s32 *)(obj + 0x1B4);
                pos[2] = *(s32 *)(obj + 0x1AC);
                func_80032854(*(s16 *)(obj + 4), 9, (u8 *)pos, 0);
            }
        }
        break;
    case 2:
        func_80032854(*(s16 *)(obj + 4), 10, (u8 *)(obj + 0x180), 0);
        break;
    case 3:
        func_80032854(*(s16 *)(obj + 4), 10, (u8 *)(obj + 0x18C), 0);
        break;
    case 4:
        func_80032854(*(s16 *)(obj + 4), 10, (u8 *)(obj + 0x174), 0);
        break;
    case 7:  func_800325E0(base + 0x31, (s32 *)(obj + 0xF4)); break;
    case 8:  func_800325E0(base + 0x32, (s32 *)(obj + 0xF4)); break;
    case 9:  func_800325E0(base + 0x33, (s32 *)(obj + 0xF4)); break;
    case 10: func_800325E0(base + 0x37, (s32 *)(obj + 0xF4)); break;
    case 11: func_800325E0(base + 0x38, (s32 *)(obj + 0xF4)); break;
    case 12: func_800325E0(base + 0x39, (s32 *)(obj + 0xF4)); break;
    case 13: func_800325E0(base + 0x3D, (s32 *)(obj + 0xF4)); break;
    case 14: func_800325E0(base + 0x3E, (s32 *)(obj + 0xF4)); break;
    case 15: func_800325E0(base + 0x3D, (s32 *)(obj + 0xF4)); break;
    case 16: func_800325E0(base + 0x3F, (s32 *)(obj + 0xF4)); break;
    case 17: func_800325E0(base + 0x40, (s32 *)(obj + 0xF4)); break;
    case 18: func_800325E0(base + 0x22, (s32 *)(obj + 0xF4)); break;
    case 19: func_800325E0(base + 0x23, (s32 *)(obj + 0xF4)); break;
    case 20: func_800325E0(base + 0x24, (s32 *)(obj + 0xF4)); break;
    case 21: func_800325E0(base + 0x25, (s32 *)(obj + 0xF4)); break;
    case 22: func_800325E0(base + 0x26, (s32 *)(obj + 0xF4)); break;
    case 23: func_800325E0(base + 0x27, (s32 *)(obj + 0xF4)); break;
    case 24: func_800325E0(base + 0x28, (s32 *)(obj + 0xF4)); break;
    case 25: func_800325E0(base + 0x29, (s32 *)(obj + 0xF4)); break;
    case 26: func_800325E0(base + 0x2A, (s32 *)(obj + 0xF4)); break;
    case 27: func_800325E0(row + 0x41, (s32 *)(obj + 0xF4)); break;
    case 28: func_800325E0(row + 0x42, (s32 *)(obj + 0xF4)); break;
    case 29: func_800325E0(row + 0x43, (s32 *)(obj + 0xF4)); break;
    case 30: func_800325E0(row + 0x44, (s32 *)(obj + 0xF4)); break;
    case 31: func_800325E0(tri + 0x71, (s32 *)(obj + 0xF4)); break;
    case 32: func_800325E0(tri + 0x72, (s32 *)(obj + 0xF4)); break;
    case 33: func_800325E0(tri + 0x73, (s32 *)(obj + 0xF4)); break;
    case 34: func_800325E0(0x77, (s32 *)(obj + 0xF4)); break;
    case 35: func_800325E0(0x7A, (s32 *)(obj + 0xF4)); break;
    case 36: func_800325E0(base + 0x3A, (s32 *)(obj + 0xF4)); break;
    case 37: func_800325E0(base + 0x36, (s32 *)(obj + 0xF4)); break;
    case 38: func_800325E0(base + 0x2B, (s32 *)(obj + 0xF4)); break;
    case 39: func_800325E0(base + 0x2C, (s32 *)(obj + 0xF4)); break;
    case 40: func_800325E0(base2 + 0x31, (s32 *)(*(u8 **)obj + 0xF4)); break;
    case 41: func_800325E0(base2 + 0x32, (s32 *)(*(u8 **)obj + 0xF4)); break;
    case 42: func_800325E0(base2 + 0x33, (s32 *)(*(u8 **)obj + 0xF4)); break;
    case 43: func_800325E0(base2 + 0x37, (s32 *)(*(u8 **)obj + 0xF4)); break;
    case 44: func_800325E0(base2 + 0x38, (s32 *)(*(u8 **)obj + 0xF4)); break;
    case 45: func_800325E0(base2 + 0x39, (s32 *)(*(u8 **)obj + 0xF4)); break;
    case 46: func_800325E0(base2 + 0x3D, (s32 *)(*(u8 **)obj + 0xF4)); break;
    case 47: func_800325E0(base2 + 0x3E, (s32 *)(*(u8 **)obj + 0xF4)); break;
    case 48: func_800325E0(base2 + 0x3D, (s32 *)(*(u8 **)obj + 0xF4)); break;
    case 49: func_800325E0(base2 + 0x3F, (s32 *)(*(u8 **)obj + 0xF4)); break;
    case 50: func_800325E0(base2 + 0x40, (s32 *)(*(u8 **)obj + 0xF4)); break;
    case 51: func_800325E0(base2 + 0x22, (s32 *)(*(u8 **)obj + 0xF4)); break;
    case 52: func_800325E0(base2 + 0x23, (s32 *)(*(u8 **)obj + 0xF4)); break;
    case 53: func_800325E0(base2 + 0x24, (s32 *)(*(u8 **)obj + 0xF4)); break;
    case 54: func_800325E0(base2 + 0x25, (s32 *)(*(u8 **)obj + 0xF4)); break;
    case 55: func_800325E0(base2 + 0x26, (s32 *)(*(u8 **)obj + 0xF4)); break;
    case 56: func_800325E0(base2 + 0x27, (s32 *)(*(u8 **)obj + 0xF4)); break;
    case 57: func_800325E0(base2 + 0x28, (s32 *)(*(u8 **)obj + 0xF4)); break;
    case 58: func_800325E0(base2 + 0x29, (s32 *)(*(u8 **)obj + 0xF4)); break;
    case 59: func_800325E0(base2 + 0x2A, (s32 *)(*(u8 **)obj + 0xF4)); break;
    case 60: func_800325E0(row2 + 0x41, (s32 *)(*(u8 **)obj + 0xF4)); break;
    case 61: func_800325E0(row2 + 0x42, (s32 *)(*(u8 **)obj + 0xF4)); break;
    case 62: func_800325E0(row2 + 0x43, (s32 *)(*(u8 **)obj + 0xF4)); break;
    case 63: func_800325E0(row2 + 0x44, (s32 *)(*(u8 **)obj + 0xF4)); break;
    case 64: func_800325E0(tri2 + 0x71, (s32 *)(*(u8 **)obj + 0xF4)); break;
    case 65: func_800325E0(tri2 + 0x72, (s32 *)(*(u8 **)obj + 0xF4)); break;
    case 66: func_800325E0(tri2 + 0x73, (s32 *)(*(u8 **)obj + 0xF4)); break;
    case 67: func_800325E0(0x77, (s32 *)(*(u8 **)obj + 0xF4)); break;
    case 68: func_800325E0(0x7A, (s32 *)(*(u8 **)obj + 0xF4)); break;
    case 69: func_800325E0(base2 + 0x3A, (s32 *)(*(u8 **)obj + 0xF4)); break;
    case 70: func_800325E0(base2 + 0x36, (s32 *)(*(u8 **)obj + 0xF4)); break;
    case 71: func_800325E0(base2 + 0x2B, (s32 *)(*(u8 **)obj + 0xF4)); break;
    case 72: func_800325E0(base2 + 0x2C, (s32 *)(*(u8 **)obj + 0xF4)); break;
    }
}
