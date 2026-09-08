/* REJECTED [s22 2026-09-07, rederive] - duplicated-tail chassis with `u32 c`.
  * Purpose: the -msoft-float residual is a loop.c:1631 hoist that needs the loop
  * insn_count at or below 58, and the duplicated-tail RA fix needs K >= 7 arms
  * (61 insns); `u32 c` sheds the zero-extends and was the cheapest candidate for
  * the missing 3 instructions.
  * MEASURED: sandbox --disable all = 9 at K = 7, 8, 9 and 12, and build_insns 67
  * != target_insns 68 (s22 gen3 sweep) - it sheds a REAL instruction the target
  * keeps, so it can never match regardless of the hoist. K <= 6 measures 28-31.
  * Dead on the insn-count leg, not on the hoist leg. */
void func_800324D0(u8 *pad) {
    u8 *ptr;
    u32 c;
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
            val = *ptr;
            ptr++;
            switch (c - 0x80) {
                case 0: pad[0xA1] = val; c = *ptr; ptr++; continue;
                case 1: pad[0xA3] = val; c = *ptr; ptr++; continue;
                case 2: pad[0xA7] = val; c = *ptr; ptr++; continue;
                case 3: pad[0xA8] = val; c = *ptr; ptr++; continue;
                case 4: pad[0xA9] = val; c = *ptr; ptr++; continue;
                case 5: pad[0xA5] = val; c = *ptr; ptr++; continue;
                case 6: pad[0xA6] = val; c = *ptr; ptr++; continue;
                case 7: pad[0xA2] = val; c = *ptr; ptr++; continue;
                case 8: pad[0xA4] = val; c = *ptr; ptr++; continue;
                case 9: pad[0xAA] = val; c = *ptr; ptr++; continue;
                case 10: pad[0xAB] = val; c = *ptr; ptr++; continue;
                case 11: pad[0xAC] = val; c = *ptr; ptr++; continue;
            }
        }
        c = *ptr;
        ptr++;
    }
}
