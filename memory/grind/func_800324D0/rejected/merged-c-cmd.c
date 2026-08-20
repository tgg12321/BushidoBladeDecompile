/* REJECTED (s3 structural, 2026-08-20): c and cmd merged into ONE u32
 * variable (0xFF holder = cmd, no `cmd = c` copy at the loop head).
 * sandbox --disable all = 37, build_insns 67 (target 68): the andi that the
 * target carries — produced by the u8→u32 promotion at the `cmd = c` copy —
 * disappears, so the build is one insn SHORT and the shape breaks.
 * Confirms the distinct c ($v0) / cmd ($a2) carrier pair is load-bearing.
 */
void func_800324D0(u8 *pad) {
    u8 *ptr;
    u32 cmd;   /* both roles */
    u8 val;

    ptr = *(u8 **)(pad + 0x58);
    cmd = 0xFF;
    pad[0xA1] = cmd; /* ... same 11 init stores ... */
    /* loop: if (cmd == 0xFF) ... else if (cmd < 0x80) ... else { cmd -= 0x80;
     * val = *ptr; ptr++; switch (cmd) ... }  cmd = *ptr; ptr++; while (cmd); */
}
