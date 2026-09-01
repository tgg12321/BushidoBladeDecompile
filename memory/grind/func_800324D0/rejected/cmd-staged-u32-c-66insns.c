/* REJECTED [s19, forensics, 2026-08-31] — H with `u32 c` instead of `u8 c`,
 * probing whether the u8->u32 promotion loss (which s3's merged-c-cmd measured
 * at -1 insn) could pay for H's +1 preheader instruction and land back on 68.
 * MEASURED: sandbox --disable all = 28, build_insns 66 != target 68
 * (s19/sandbox_K_cmd_staged_u32c.log). The widening removes THREE insns from
 * the H shape, not one, so it overshoots 68 in the other direction. The
 * insn-count ledger for the staged-subtraction family is therefore
 * {u8 c: 69, u32 c: 66} with no spelling in between measured. */
void func_800324D0(u8 *pad) {
    u8 *ptr;
    u32 c;
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
    while (c != 0) {
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
        c = *ptr;
        ptr++;
        cmd = c - 0x80;
    }
}
