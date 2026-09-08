/* REJECTED [s19, forensics, 2026-08-31] — `cmd = c - 0x80` hoisted to the top
 * of the loop body (before the 0xFF / <0x80 tests), to lengthen the cmd ARM web
 * (pseudo 75) per the s18 relief curve.
 * MEASURED: sandbox --disable all = 28, build_insns 68 == target 68 (shape
 * intact), tmp/grind/func_800324D0/s19/sandbox_A_cmd_early.log.
 * RA FORENSICS (s19/model_A_cmd_early.json): the lever WORKS but is far too
 * weak and it back-fires on a second allocno. livelen(75) 4 -> 9 (pri 75000 ->
 * 33333, the curve needs >= 20 i.e. pri < 15483), while the zero-extend temp
 * (pseudo 85 = zero_extend(c), NOT the "cmd arm web" the s5/s18 ledger called
 * it) SHORTENS 7 -> 4 because its last consumer moved earlier, so pri(85) rises
 * 34285 -> 60000 and it becomes the new order head. Allocation order
 * [85,76,75,72,74,73,91,87]: the walker is still allocated LAST of the six.
 * Kill: in-body hoisting is a sub-2x live-length lever and the two short webs
 * are anti-correlated — shortening one lengthens the other. */
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
    while (c != 0) {
        cmd = c - 0x80;
        if (c == 0xFF) {
            ptr += 6;
        } else if (c < 0x80) {
            ptr++;
        } else {
            val = *ptr;
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
        cmd = *ptr;
        c = cmd;
        ptr++;
    }
}
