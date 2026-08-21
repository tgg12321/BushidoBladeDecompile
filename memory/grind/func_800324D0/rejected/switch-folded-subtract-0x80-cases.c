/* REJECTED [s9, rederive/forensics, brief-session 8] — switch-folded
 * subtraction spellings (m2c-suggested axis). m2c renders the target's
 * dispatch as `switch (temp_a2) { case 0x80: ... }` (it cannot distinguish
 * source spellings; casesi normalizes both). Two spellings measured on the
 * 15-floor staged-tail chassis, BOTH regress to 27, build 68 == target 68:
 *
 *   R1: arm = `val = *ptr; ptr++; switch (c) { case 0x80: ... }`
 *       (cmd arm web deleted entirely)                      -> 27, 68/68
 *   R2: arm = `cmd = c; val = *ptr; ptr++; switch (cmd)
 *       { case 0x80: ... }`                                 -> 27, 68/68
 *
 * Kill mechanism: folding the -0x80 into the switch makes GCC's casesi emit
 * the subtraction into a fresh die-at-def scratch pseudo instead of the cmd
 * arm web; the arm web (allocno 85) disappears or shrinks to a copy, the s4
 * census (75/76/85 split) collapses back to the s1 census, and the full 27
 * rotation returns. The source-level `cmd = c - 0x80` carrying the
 * SUBTRACTED value in the cmd web is load-bearing for the 15 floor.
 * (R1 body below; R2 = R1 plus `cmd = c;` before the switch on cmd.)
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
        if (c == 0xFF) {
            ptr += 6;
        } else if (c < 0x80) {
            ptr++;
        } else {
            val = *ptr;
            ptr++;
            {
                switch (c) {
                    case 0x80: pad[0xA1] = val; break;
                    case 0x81: pad[0xA3] = val; break;
                    case 0x82: pad[0xA7] = val; break;
                    case 0x83: pad[0xA8] = val; break;
                    case 0x84: pad[0xA9] = val; break;
                    case 0x85: pad[0xA5] = val; break;
                    case 0x86: pad[0xA6] = val; break;
                    case 0x87: pad[0xA2] = val; break;
                    case 0x88: pad[0xA4] = val; break;
                    case 0x89: pad[0xAA] = val; break;
                    case 0x8A: pad[0xAB] = val; break;
                    case 0x8B: pad[0xAC] = val; break;
                }
            }
        }
        cmd = *ptr;
        c = cmd;
        ptr++;
    }
}
