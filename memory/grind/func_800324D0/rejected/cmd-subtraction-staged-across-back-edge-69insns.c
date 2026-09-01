/* REJECTED [s19, forensics, 2026-08-31] — THE MOST INFORMATIVE NEGATIVE OF THE
 * SESSION. `cmd = c - 0x80` computed in the PREHEADER and at the LOOP TAIL, so
 * the biased command value is loop-carried (live across the back edge).
 * MEASURED: sandbox --disable all = 30, build_insns 69 != target 68
 * (s19/sandbox_H_cmd_staged.log) — shape broken by exactly one instruction.
 * RA FORENSICS (s19/model_H_cmd_staged.json) — this is the first spelling in
 * 19 sessions in which the walker OUT-RANKS the cmd web:
 *   pseudo 85 (zero_extend(c)) DISAPPEARS (folded into the staged subtraction);
 *   pseudo 75 (cmd) nrefs 10 -> 7, livelen 4 -> 15, pri 75000 -> 9333;
 *   pseudo 73 (walker) pri 14769 > pri(75) 9333  ==> order [76,86,72,74,73,75,...]
 *   and 75 lands in $a2 (6), which is its TARGET seat.
 * So the priority inversion the ledger has called "arithmetically dead" since
 * s1 is NOT dead: back-edge staging demotes a short web by ~8x. It costs
 * exactly one preheader instruction, and the 68-insn budget has no slack.
 * The open frontier is a 68-insn spelling of the same loop-carried value. */
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
