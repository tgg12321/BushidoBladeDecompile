/* REJECTED [s16 2026-08-31] — "shorten the walker's live range by sinking its
 * load below the 11 pad stores".  MEASURED FLAT: sandbox --disable all = 15,
 * target_insns 68 == build_insns 68, rules_dropped 0
 * (tmp/grind/func_800324D0/s16/sandbox_p1.log).
 *
 * WHY IT IS DEAD, and why no restatement of it can work: the RA model extracted
 * from THIS source is BYTE-IDENTICAL to the candidate's
 * (tmp/grind/func_800324D0/s16/model_p1_identical_to_candidate.json vs
 * model.json) — same allocation order [75,76,85,72,74,73,91,86], same
 * priorities [75000,47272,34285,29838,26666,15483,333,326], same livelen 62 for
 * the walker (pseudo 73), same dispositions.  cc1 re-establishes the walker's
 * live range regardless of where the C statement sits: the `lw` has no
 * dependence on the stores, so flow.c's liveness (and the scheduler) put it back
 * at the top.  The live-length term of allocno_compare's priority
 * (floor_log2(refs)*refs/livelen) is therefore NOT source-order-controllable for
 * this pointer, which kills the whole "raise the walker's priority by making it
 * live over less of the function" family, not just this spelling.
 */
void func_800324D0(u8 *pad) {
    u8 *ptr;
    u8 c;
    u32 cmd;
    u8 val;

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

    ptr = *(u8 **)(pad + 0x58);
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
            {
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
        /* FAKE: loop-tail stream byte staged through the currently-dead cmd
         * (c = *ptr written as cmd = *ptr; c = cmd), mechanism: global.c
         * allocno census - the extra cmd set splits cmd into head/arm webs and
         * re-seeds find_reg (val/byte/holders land in target regs; floor 27->15,
         * 68/68), lever-exhaustion: memory/grind/func_800324D0/hypotheses.md
         * s1-s4 + two dry permuter campaigns (evidence.md s4/s5) */
        cmd = *ptr;
        c = cmd;
        ptr++;
    }
}
