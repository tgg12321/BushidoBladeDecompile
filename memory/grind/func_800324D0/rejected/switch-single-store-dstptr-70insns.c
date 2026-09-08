void func_800324D0(u8 *pad) {
    u8 *ptr;
    u8 c;
    u32 cmd;
    u8 val;
    u8 *dst;

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
            dst = pad;
            switch (cmd) {
                case 0: dst = &pad[0xA1]; break;
                case 1: dst = &pad[0xA3]; break;
                case 2: dst = &pad[0xA7]; break;
                case 3: dst = &pad[0xA8]; break;
                case 4: dst = &pad[0xA9]; break;
                case 5: dst = &pad[0xA5]; break;
                case 6: dst = &pad[0xA6]; break;
                case 7: dst = &pad[0xA2]; break;
                case 8: dst = &pad[0xA4]; break;
                case 9: dst = &pad[0xAA]; break;
                case 10: dst = &pad[0xAB]; break;
                case 11: dst = &pad[0xAC]; break;
            }
            *dst = val;
        }
        c = *ptr;
        ptr++;
    }
}
