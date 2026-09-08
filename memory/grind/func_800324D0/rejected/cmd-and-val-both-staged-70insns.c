/* REJECTED [s19, forensics, 2026-08-31] — H (staged `cmd = c - 0x80`) plus D
 * (staged `val = *ptr`) together. MEASURED: sandbox --disable all = 37,
 * build_insns 70 != target 68 (s19/sandbox_I_cmd_val_staged.log).
 * RA FORENSICS (s19/model_I_cmd_val_staged.json): confirms the two staging
 * levers ADD on the priority axis (pri 75 = 10000, 76 = 31764, walker 15151)
 * and that their instruction costs also add (+1 each). val's ceiling is
 * unchanged: livelen(76) reaches only 34 against the required 68. */
void func_800324D0(u8 *pad) {
    u8 *ptr;
    u8 c;
    u32 cmd;
    u8 val;

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
    cmd = c - 0x80;
    val = *ptr;
    while (c != 0) {
        if (c == 0xFF) {
            ptr += 6;
        } else if (c < 0x80) {
            ptr++;
        } else {
            ptr++;
                switch (cmd) {
                    case 0: pad[0xA1] = val; break;
                    case 1: pad[0xA3] = val; break;
                    case 2: pad[0xA7] = val; break;
                    case 3: pad[0xA8] = val; break;
                    case 4: pad[0xA9] = val; break;
                    case 5: pad[0xA5] = val; break;
                    case 6: pad[0xA6] = val; break;
                    case 7: pad[0xA2] = val; break;
                    case 8: pad[0xA4] = val; break;
                    case 9: pad[0xAA] = val; break;
                    case 10: pad[0xAB] = val; break;
                    case 11: pad[0xAC] = val; break;
                }
        }
        c = *ptr;
        ptr++;
        cmd = c - 0x80;
        val = *ptr;
    }
}
