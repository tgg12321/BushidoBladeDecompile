/* REJECTED [s19, forensics, 2026-08-31] — the two head tests swapped
 * (`c < 0x80` first, `c != 0xFF` second) as a live-range lever on the
 * zero-extend temp (pseudo 85). MEASURED: the ra_solver model is IDENTICAL to
 * the 15-chassis baseline in every field — order [75,76,85,72,74,73,91,88],
 * nrefs/livelen/pri 75(10,4,75000) 76(26,22,47272) 85(8,7,34285) 72(37,62)
 * 74(8,9) 73(24,62,15483) (s19/model_F_test_reorder.json vs
 * s19/model_base.json). jump.c normalises the comparison order before flow
 * ever computes liveness, so head-test ORDER is not a live-range lever at all.
 * Not sandboxed: an RA-identical, insn-identical form cannot move the score. */
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
        if (c < 0x80) {
            ptr++;
        } else if (c != 0xFF) {
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
        } else {
            ptr += 6;
        }
        cmd = *ptr;
        c = cmd;
        ptr++;
    }
}
