/* func_800324D0 — BEST CLEAN FORM (s2, 2026-08-20): sandbox --disable all = 27,
 * build_insns 68 == target 68. Pin-free, zero constructs, ordinary C.
 *
 * This is the Judge-directed baseline (layer-1 ruling 2026-08-20 15:48,
 * docs/grind/decisions.md:9542): the s1 base/ff block-0 split that closed to 0
 * was FAILed as a Test-3 GCC-internals cheat and is BANNED in any spelling
 * (banked at rejected/layer1-fail-0820-1548.c). Do not re-derive it.
 *
 * The whole 27 is one 3-cycle register rotation across ~27 insns (s1 objdump
 * proof, evidence.md): walker ours $a2 / target $v1, cmd ours $a1 / target $a2,
 * val ours $v1 / target $a1. Schedule, shape, andi, sltiu, frame all match.
 * The single sufficient closing condition is a walker allocno preference for
 * hard reg 3 (s1 BB2_FINDREG_DEBUG ground truth) — but every known honest
 * planting route is measured dead or banned; see hypotheses.md frontier.
 */
void func_800324D0(u8 *pad) {
    u8 *ptr;
    u8 c;
    u32 cmd;
    u8 val;

    ptr = *(u8 **)(pad + 0x58);
    c = 0xFF;
    pad[0xA1] = c;
    pad[0xA3] = c;
    pad[0xA2] = c;
    pad[0xA4] = c;
    pad[0xAA] = 0;
    pad[0xA7] = 0;
    pad[0xA8] = 0;
    pad[0xA5] = 0;
    pad[0xA6] = c;
    pad[0xAB] = c;
    pad[0xAC] = c;

    c = ptr[4];
    ptr += 5;
    if (c == 0) return;

    do {
        cmd = c;
        if (cmd == 0xFF) {
            ptr += 6;
        } else if (cmd < 0x80) {
            ptr++;
        } else {
            cmd -= 0x80;
            val = *ptr;
            ptr++;
            if (cmd < 12) {
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
        }
        c = *ptr;
        ptr++;
    } while (c != 0);
}
