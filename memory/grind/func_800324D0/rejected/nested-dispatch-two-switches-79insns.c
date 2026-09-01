/* REJECTED [s21, rederive, 2026-09-01] — NESTED DISPATCH (two 6-arm switches
 * behind an outer `if (cmd < 6)`). This is the s20 frontier's shape (b), the
 * ONLY unmeasured axis the ledger carried: "does splitting the twelve arm
 * stores across two dispatch regions split pseudo 76 into two carriers of
 * <= 15 weighted refs each?"
 * MEASURED: sandbox --disable all = 28, build_insns 79 vs target 68
 * (s21/sandbox_A_nested_dispatch.log) — two jump tables plus the outer
 * compare cost +11 instructions.
 * THE ANSWER TO THE FRONTIER QUESTION IS NO, AND FOR A REASON THAT KILLS THE
 * WHOLE AXIS: the RA model (s21/model_A_nested_dispatch.json) shows pseudo 76
 * UNSPLIT at nrefs 26 — exactly the base body's count — with only its live
 * length changed (22 -> 30, pri 47272 -> 34666). A NEW allocno 97 (the
 * half-select temp, nrefs 6 livelen 4 pri 30000) appears instead. The carrier
 * is one pseudo because `val` is ONE C variable; the number of dispatch
 * regions is irrelevant to reg_n_refs. Splitting the carrier would require TWO
 * distinct C variables holding the operand byte, which is the BANNED base/ff
 * overlapping-live-range family under a new name and has no semantic reading
 * (all twelve arms store the same loaded byte).
 */
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
            cmd = c - 0x80;
            val = *ptr;
            ptr++;
            if (cmd < 6) {
                switch (cmd) {
                case 0: pad[0xA1] = val; break;
                case 1: pad[0xA3] = val; break;
                case 2: pad[0xA7] = val; break;
                case 3: pad[0xA8] = val; break;
                case 4: pad[0xA9] = val; break;
                case 5: pad[0xA5] = val; break;
                }
            } else {
                switch (cmd) {
                case 6: pad[0xA6] = val; break;
                case 7: pad[0xA2] = val; break;
                case 8: pad[0xA4] = val; break;
                case 9: pad[0xAA] = val; break;
                case 10: pad[0xAB] = val; break;
                case 11: pad[0xAC] = val; break;
                }
            }
        }
        /* FAKE: loop-tail stream byte staged through the currently-dead cmd
         * (c = *ptr written as cmd = *ptr; c = cmd), mechanism: global.c
         * allocno census, lever-exhaustion: hypotheses.md s1-s4 */
        cmd = *ptr;
        c = cmd;
        ptr++;
    }
}
