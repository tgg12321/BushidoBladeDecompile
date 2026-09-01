/* REJECTED [s21, rederive, 2026-09-01] — operand read hoisted OUT of the
 * payload branch to the top of the loop body. Purpose: measure the honest
 * CEILING of reg_live_length(76), the single quantity the target disposition
 * needs at >= 68.
 * MEASURED: sandbox --disable all = 16, build_insns 68 == target 68
 * (s21/sandbox_D_val_hoisted_head.log). RA model: 76 goes 22 -> 26 livelen,
 * pri 47272 -> 40000; 85 goes 7 -> 6 livelen, pri 34285 -> 40000 (a net
 * PROMOTION of the zext temp). Nowhere near the 68 the target needs.
 * It is also semantically wrong: it reads the operand byte on the 0xFF and
 * short-command paths, which the target bytes prove the original does not do.
 * Banked purely as the livelen(76) ceiling probe.
 */
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
        val = *ptr;
        if (c == 0xFF) {
            ptr += 6;
        } else if (c < 0x80) {
            ptr++;
        } else {
            cmd = c - 0x80;
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
