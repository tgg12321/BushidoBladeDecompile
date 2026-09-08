/* REJECTED [s21, rederive, 2026-09-01] — head tests RE-NESTED: the payload
 * test (c < 0x80) becomes the outer branch and the 0xFF test moves inside its
 * else. Distinct from the s19 probe F order-swap (which jump.c normalised to a
 * bit-identical model): this changes the nesting, not the order.
 * MEASURED: sandbox --disable all = 17, build_insns 69 vs target 68
 * (s21/sandbox_B_nested_head.log). The RA model is bit-identical to the base
 * body (75:10/4/75000, 76:26/22/47272, 85:8/7/34285, 72:37/62/29838,
 * 74:8/9/26666, 73:24/62/15483) — the extra instruction buys nothing at all.
 * Head-branch topology is measured inert for the allocno census.
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
        if (c < 0x80) {
            ptr++;
        } else if (c == 0xFF) {
            ptr += 6;
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
        /* FAKE: loop-tail stream byte staged through the currently-dead cmd
         * (c = *ptr written as cmd = *ptr; c = cmd), mechanism: global.c
         * allocno census, lever-exhaustion: hypotheses.md s1-s4 */
        cmd = *ptr;
        c = cmd;
        ptr++;
    }
}
