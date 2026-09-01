/* REJECTED [s21, rederive, 2026-09-01] — FLAT, not worse: `for` form with the
 * stream advance in the third clause (`for (c = ptr[4], ptr += 5; c != 0;
 * c = cmd, ptr++)`), the staged tail read as the loop body's last statement.
 * MEASURED: sandbox --disable all = 15, build_insns 68 == target 68
 * (s21/sandbox_C_for_form.log) — identical score to candidate.c, and the RA
 * model is BIT-IDENTICAL to the base body's (same order [75,76,85,72,74,73,
 * 91,86], same nrefs/livelen/pri for all eight allocnos).
 * Banked as a NEGATIVE for the loop-form axis: while-form vs for-form-with-
 * step is invisible to GCC 2.7.2's allocno census, so loop syntax is not a
 * lever on this function.
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

    for (c = ptr[4], ptr += 5; c != 0; c = cmd, ptr++) {
        if (c == 0xFF) {
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
    }
}
