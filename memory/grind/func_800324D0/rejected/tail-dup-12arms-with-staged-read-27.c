/* REJECTED [s22 2026-09-01] — the 12-arm loop-tail duplication that CLOSES this
 * function, but WITH the s5 staged tail (`cmd = *ptr; c = cmd;`) still in place.
 * Measured: sandbox --disable all = 27, build_insns 68 == target_insns 68
 * (tmp/grind/func_800324D0/s22/sandbox_p1.log). Byte-neutral like the winning
 * form, but the staging hands the duplicated reference lift to the command web
 * instead of the stream walker, so the walker never outranks the operand
 * carrier and the allocation lands FURTHER from target than the s21 body (27 vs
 * 15). KILL GENERALIZES: the s5 staged-value-reused-variable FAKE and the
 * reg_n_refs walker lift are ANTAGONISTIC on this chassis — the matching body
 * has no staged read at all. Drop the staging before duplicating.
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
            switch (cmd) {
                case 0: pad[0xA1] = val; cmd = *ptr; c = cmd; ptr++; continue;
                case 1: pad[0xA3] = val; cmd = *ptr; c = cmd; ptr++; continue;
                case 2: pad[0xA7] = val; cmd = *ptr; c = cmd; ptr++; continue;
                case 3: pad[0xA8] = val; cmd = *ptr; c = cmd; ptr++; continue;
                case 4: pad[0xA9] = val; cmd = *ptr; c = cmd; ptr++; continue;
                case 5: pad[0xA5] = val; cmd = *ptr; c = cmd; ptr++; continue;
                case 6: pad[0xA6] = val; cmd = *ptr; c = cmd; ptr++; continue;
                case 7: pad[0xA2] = val; cmd = *ptr; c = cmd; ptr++; continue;
                case 8: pad[0xA4] = val; cmd = *ptr; c = cmd; ptr++; continue;
                case 9: pad[0xAA] = val; cmd = *ptr; c = cmd; ptr++; continue;
                case 10: pad[0xAB] = val; cmd = *ptr; c = cmd; ptr++; continue;
                case 11: pad[0xAC] = val; cmd = *ptr; c = cmd; ptr++; continue;
            }
        }
        cmd = *ptr;
        c = cmd;
        ptr++;
    }
}
