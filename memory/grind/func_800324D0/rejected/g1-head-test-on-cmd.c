/* REJECTED s6 (brief-session 6, synthesis): head-web lengthening via head
 * tests on cmd. Preheader staged (cmd = ptr[4]; c = cmd;) + `if (cmd == 0xFF)`
 * as the first loop test. Goal: extend cmd head web's live range across the
 * loop-back edge to drop allocno 75 below val in allocno_compare.
 * MEASURED: sandbox --disable all = 30, build 70 vs target 68 — keeping cmd
 * live across the loop head costs 2 extra insns; shape breaks before the
 * census even shifts. Sibling G2 (`while (cmd != 0)` condition instead)
 * measured 32/69 — same failure mode. The head-web-lengthening axis is
 * shape-incompatible in every spelling. */
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

    cmd = ptr[4];
    c = cmd;
    ptr += 5;
    while (c != 0) {
        if (cmd == 0xFF) {          /* <-- the lengthener; breaks shape */
            ptr += 6;
        } else if (c < 0x80) {
            ptr++;
        } else {
            cmd = c - 0x80;
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
