/* REJECTED [s19, forensics, 2026-08-31] — the operand byte `val` staged across
 * the loop back edge (`val = *ptr` in the preheader and at the loop tail, arm
 * body reduced to `ptr++` + switch), to lengthen pseudo 76 (= val; the s5/s18
 * ledger mis-identified 76 as the stream byte c).
 * MEASURED: sandbox --disable all = 19, build_insns 68 == target 68 — shape
 * INTACT (s19/sandbox_D_val_staged.log), and the register dispositions are
 * byte-for-byte the same map as the 15-floor chassis {72:4,73:6,74:2,75:3,
 * 76:5,85:3,86:8,91:7}: the extra distance is scheduling, not allocation.
 * RA FORENSICS (s19/model_D_val_staged.json): livelen(76) 22 -> 31 only
 * (pri 47272 -> 34838; the s18 curve needs livelen >= 68, i.e. pri < 15483).
 * val dies in 12 places (one per switch arm), so back-edge staging can only
 * add the head+dispatch prefix to its live range, never the arm bodies.
 * SECOND-ORDER KILL banked here: the same edit lengthened the WALKER (73)
 * 62 -> 63 and pad (72) 62 -> 63, because the walker is live over the whole
 * loop — every lengthening lever demotes the walker in lockstep with its
 * target. That damping is why the priority channel is so stiff. */
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
    val = *ptr;
    while (c != 0) {
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
        val = *ptr;
    }
}
