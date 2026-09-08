/* REJECTED [s19, forensics, 2026-08-31] — THE 68-INSN SPELLING OF THE
 * LOOP-CARRIED cmd. H (staged `cmd = c - 0x80` in the preheader and at the loop
 * tail) plus the two head tests re-expressed on the biased value, which is
 * exact unsigned algebra: c == 0xFF <=> cmd == 0x7F, and c < 0x80 <=>
 * cmd > 0x7F (for c >= 0x80, cmd = c - 0x80 <= 0x7F; for c < 0x80 the u32
 * subtraction wraps above 0xFFFFFF80). The loop test stays on c.
 * MEASURED: sandbox --disable all = 33, build_insns 68 == target_insns 68,
 * rules_dropped 0 (s19/sandbox_L_staged_tests_on_cmd.log). Shape INTACT — this
 * answers the s19 frontier question "is there a 68-insn spelling that makes the
 * cmd subtraction loop-carried?" with YES.
 * RA FORENSICS (s19/model_L_staged_tests_on_cmd.json): the allocno set drops to
 * SEVEN (the zero_extend temp is gone), order [76,72,75,74,73,89,86] with
 * 76(26,21) pri 49523 | 72(37,64) 28906 | 75(11,14) 23571 | 74(6,6) 20000 |
 * 73(24,64) 15000. The walker is 5th of 7, i.e. re-expressing the tests on cmd
 * costs pseudo 75 its H demotion (H: nrefs 7, livelen 15, pri 9333 -> here
 * nrefs 11, livelen 14, pri 23571) because the head tests add references to
 * cmd. Kill: the instruction that H spends is precisely what buys 75's low
 * reference count; paying for it with head tests on cmd hands the references
 * straight back. */
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
    while (c != 0) {
        if (cmd == 0x7F) {
            ptr += 6;
        } else if (cmd > 0x7F) {
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
