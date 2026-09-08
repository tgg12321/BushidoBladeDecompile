/* REJECTED (s2, 2026-08-20): moving the payload arm's `ptr++` to after the
 * switch made the score WORSE — sandbox 30, build_insns 66 vs target 68
 * (GCC merged the arm increment with the shared-tail increment and the loop
 * shape broke). Do not re-propose. The `val = *ptr; ptr++;` order before the
 * switch is load-bearing for the 68-insn shape.
 */
void func_800324D0(u8 *pad) {
    u8 *ptr; u8 c; u32 cmd; u8 val;
    ptr = *(u8 **)(pad + 0x58);
    c = 0xFF;
    pad[0xA1] = c; pad[0xA3] = c; pad[0xA2] = c; pad[0xA4] = c;
    pad[0xAA] = 0; pad[0xA7] = 0; pad[0xA8] = 0; pad[0xA5] = 0;
    pad[0xA6] = c; pad[0xAB] = c; pad[0xAC] = c;
    c = ptr[4]; ptr += 5;
    if (c == 0) return;
    do {
        if (c == 0xFF) { ptr += 6; }
        else if (c < 0x80) { ptr++; }
        else {
            cmd = c - 0x80;
            val = *ptr;
            if (cmd < 12) {
                switch (cmd) { /* ... 12 sb cases ... */ }
            }
            ptr++;   /* <-- the killed move */
        }
        c = *ptr; ptr++;
    } while (c != 0);
}
