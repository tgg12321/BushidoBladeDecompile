void func_800324D0(u8 *pad) {
    u8 *ptr;
    u8 c;
    u32 cmd;
    u8 val;
    u32 off;

    ptr = *(u8 **)(pad + 0x58);
    pad[0xA1] = 0xFF;
    pad[0xA3] = 0xFF;
    pad[0xA2] = 0xFF;
    pad[0xA4] = 0xFF;
    pad[0xAA] = 0;
    pad[0xA7] = 0;
    pad[0xA8] = 0;
    pad[0xA5] = 0;
    pad[0xA6] = 0xFF;
    pad[0xAB] = 0xFF;
    pad[0xAC] = 0xFF;

    c = ptr[4];
    ptr += 5;
    while (c != 0) {
        if (c == 0xFF) {
            ptr += 6;
        } else if (c < 0x80) {
            ptr++;
        } else {
            cmd = c - 0x80;
            val = *ptr;
            ptr++;
            off = 0;
            switch (cmd) {
                case 0: off = 0xA1; break;
                case 1: off = 0xA3; break;
                case 2: off = 0xA7; break;
                case 3: off = 0xA8; break;
                case 4: off = 0xA9; break;
                case 5: off = 0xA5; break;
                case 6: off = 0xA6; break;
                case 7: off = 0xA2; break;
                case 8: off = 0xA4; break;
                case 9: off = 0xAA; break;
                case 10: off = 0xAB; break;
                case 11: off = 0xAC; break;
            }
            pad[off] = val;
        }
        c = *ptr;
        ptr++;
    }
}
